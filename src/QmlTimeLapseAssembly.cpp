/*
  TimeLapse tools for SFOS
  Copyright (C) 2022  Lukáš Karas

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <QmlTimeLapseAssembly.h>

#include <TimeLapse/pipeline_deflicker.h>
#include <TimeLapse/pipeline_frame_mapping.h>
#include <TimeLapse/pipeline_frame_prepare.h>
#include <TimeLapse/pipeline_resize_frame.h>
#include <TimeLapse/pipeline_video_assembly.h>
#include <TimeLapse/pipeline_write_frame.h>

#include <QProcess>
#include <QTextStream>
#include <QStandardPaths>

#include <istream> // have to be before tar.hpp
#include <tar/tar.hpp>

#include <cassert>
#include <algorithm>

static QStringList videoDirectories;

AssemblyProcess::AssemblyProcess(QThread *t):
  thread(t)
{}

AssemblyProcess::~AssemblyProcess() {
  if (pipeline != nullptr) {
    pipeline->deleteLater();
    pipeline = nullptr;
  }
  if (resources != nullptr) {
    resources->deleteLater(); // needs to be deleted after pipeline
  }

  assert(thread != nullptr);
  thread->quit();
  thread = nullptr;
}

void AssemblyProcess::init() {

}

PipelineResources::PipelineResources(const QString &templateName):
  dir(templateName), err(stderr), verboseOutput(stdout)
{}

void AssemblyProcess::start(const QmlTimeLapseAssembly::AssemblyParams params) {
  using namespace timelapse;

  QFileInfo output(params.dir + QDir::separator() + params.name + ".mp4");
  if (output.exists()) {
    emit error(tr("Output file already exists"));
    return;
  }

  // unpack ffmpeg
  // cd .cache/cz.karry.timelapse/TimeLapseTools/
  // tar -xf /usr/share/harbour-timelapse-tools/bin/ffmpeg.tar
  QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
  QString ffmpegBinary = cacheDir + QDir::separator() + "ffmpeg";
  QString ffmpegArchive = "/usr/share/harbour-timelapse-tools/bin/ffmpeg.tar";
  if (QFileInfo ffmpegInfo = QFileInfo(ffmpegBinary);
      !ffmpegInfo.exists() || ffmpegInfo.created() < QFileInfo(ffmpegArchive).created()) {
    try {
      emit progress(tr("Unpacking ffmpeg"));
      if (ffmpegInfo.exists()) {
        if (!QFile(ffmpegBinary).remove()) {
          qWarning() << "Failed to remove " << ffmpegBinary;
        }
      }
      tar::tar_reader tar(ffmpegArchive.toStdString());
      std::istream &is = tar.get("ffmpeg");
      std::ofstream os(ffmpegBinary.toStdString().c_str(), std::ios_base::out | std::ios_base::binary);
      std::copy(istreambuf_iterator<char>(is), istreambuf_iterator<char>(), ostreambuf_iterator<char>(os));
      os.close();
      if (!is.good() || !os.good()) {
        throw std::runtime_error("IO error");
      }
    } catch  (const std::exception &e) {
      qWarning() << "Failed to unpack ffmpeg:" << e.what();
      emit error(tr("Failed to unpack ffmpeg: %1").arg(e.what()));
      return;
    }
    QFile(ffmpegBinary).setPermissions(QFile::ExeGroup | QFile::ExeOther | QFile::ExeUser);
  }

  // check temp dir
  resources = new PipelineResources(params.source + QDir::separator() + ".tmp");
  if (!resources->dir.isValid()) {
    emit error("Can't create temp directory");
    return;
  }

  qDebug() << "Tmp dir: " << QDir::tempPath();
  resources->dir.setAutoRemove(true);
  qDebug() << "Using temp directory " << resources->dir.path();

  int width=-1;
  int height=-1;
  QString bitrate;
  QString codec;
  QString pixelFormat;
  switch (params.profile) {
    case QmlTimeLapseAssembly::HDx264_Low:
      width = 1920;
      height = 1080;
      bitrate = "20000k";
      codec = "libx264";
      pixelFormat = "yuv420p";
      break;
    case QmlTimeLapseAssembly::HDx264_High:
      width = 1920;
      height = 1080;
      bitrate = "40000k";
      codec = "libx264";
      pixelFormat = "yuv420p";
      break;
    case QmlTimeLapseAssembly::HDx265:
      width = 1920;
      height = 1080;
      bitrate = "40000k";
      codec = "libx265";
      break;
    case QmlTimeLapseAssembly::UHDx265:
      width = 3840;
      height = 2160;
      bitrate = "60000k";
      codec = "libx265";
      break;
    default:
      assert(false);
  }

  // build processing pipeline
  QStringList inputArguments;
  inputArguments << params.source;
  pipeline = Pipeline::createWithFileSource(inputArguments, params.fileSuffixes, false, &resources->verboseOutput, &resources->err);

  if (params.deflicker == QmlTimeLapseAssembly::Deflicker::Average || params.deflicker == QmlTimeLapseAssembly::Deflicker::MovingAverage) {
    *pipeline << new ComputeLuminance(&resources->verboseOutput);
  }

  qDebug() << "explicit video length:" << params.length;

  if (params.length < 0) {
    *pipeline << new OneToOneFrameMapping();
  } else if (params.noStrictInterval) {
    *pipeline << new ImageMetadataReader(&resources->verboseOutput, &resources->err);
    *pipeline << new VariableIntervalFrameMapping(&resources->verboseOutput, &resources->err, params.length, params.fps);
  } else {
    *pipeline << new ConstIntervalFrameMapping(&resources->verboseOutput, &resources->err, params.length, params.fps);
  }

  if (params.deflicker == QmlTimeLapseAssembly::Deflicker::Average ||
      params.deflicker == QmlTimeLapseAssembly::Deflicker::MovingAverage) {
    if (params.deflicker == QmlTimeLapseAssembly::Deflicker::MovingAverage) {
      *pipeline << new WMALuminance(&resources->verboseOutput, params.wmaCount);
    } else {
      *pipeline << new ComputeAverageLuminance(&resources->verboseOutput);
    }
    *pipeline << new AdjustLuminance(&resources->verboseOutput, params.deflickerDebugView);
  }

  if (params.blendFrames) {
    if (params.blendBeforeResize) {
      *pipeline << new BlendFramePrepare(&resources->verboseOutput, params.frameCount());
      *pipeline << new ResizeFrame(&resources->verboseOutput, width, height, params.adaptiveResize);
    } else {
      *pipeline << new ResizeFrame(&resources->verboseOutput, width, height, params.adaptiveResize);
      *pipeline << new BlendFramePrepare(&resources->verboseOutput, params.frameCount());
    }
  } else {
    *pipeline << new ResizeFrame(&resources->verboseOutput, width, height, params.adaptiveResize);
    *pipeline << new FramePrepare(&resources->verboseOutput, params.frameCount());
  }
  *pipeline << new WriteFrame(QDir(resources->dir.path()), &resources->verboseOutput, false);

  VideoAssembly *assembly = new VideoAssembly(QDir(resources->dir.path()), &resources->verboseOutput, &resources->err, false,
                                              output,
                                              width, height, params.fps, bitrate, codec,
                                              ffmpegBinary, pixelFormat);
  connect(assembly, &VideoAssembly::started, this, &AssemblyProcess::onFFmpegStarted);
  *pipeline << assembly;

  connect(pipeline, &Pipeline::done, this, &AssemblyProcess::onDone);
  connect(pipeline, &Pipeline::done, this, &AssemblyProcess::done);
  connect(pipeline, &Pipeline::error, this, &AssemblyProcess::error);
  connect(pipeline, &Pipeline::imageLoaded, this, &AssemblyProcess::onImageLoaded);
  emit pipeline->process();
}

void AssemblyProcess::onFFmpegStarted() {
  emit progress(tr("Assembling video with ffmpeg"));
}

void AssemblyProcess::onDone() {
  emit progress(tr("Done"));
}

void AssemblyProcess::onImageLoaded(int stage, int cnt) {
  emit progress(tr("Stage %1, processing image %2").arg(stage+1).arg(cnt+1));
}

QmlTimeLapseAssembly::QmlTimeLapseAssembly()
{
  if (!videoDirectories.empty()) {
    params.dir = videoDirectories[0];
  }
  // TODO: configurable, support raw images...
  params.fileSuffixes << "jpeg";
  params.fileSuffixes << "jpg";
}

QmlTimeLapseAssembly::~QmlTimeLapseAssembly() {
  if (process!=nullptr) {
    onError(tr("Interrupted"));
  }
}

void QmlTimeLapseAssembly::setSource(const QString &s) {
  if (s == params.source) {
    return;
  }
  params.source = s;
  QDir d(params.source);
  QFileInfoList l = d.entryInfoList(QDir::Files, QDir::Name);
  inputImgCnt=0;
  for (QFileInfo i2 : l) {
    if ((i2.isFile() || i2.isSymLink()) &&
        (params.fileSuffixes.isEmpty() || params.fileSuffixes.contains(i2.completeSuffix(), Qt::CaseInsensitive))) {
      inputImgCnt++;
    }
  }
  emit inputImgCntChanged(inputImgCnt);
  emit sourceChanged(params.source);
}

void QmlTimeLapseAssembly::onError(const QString &msg) {
  emit error(msg);
  cleanup();
}

void QmlTimeLapseAssembly::onProgress(QString msg) {
  _progressMessage = msg;
  emit progressMessageChanged(msg);
}

void QmlTimeLapseAssembly::cleanup() {
  if (process!=nullptr) {
    process->deleteLater();
    process = nullptr;
  }
  emit processingChanged();
}

void QmlTimeLapseAssembly::start() {
  QThread *thread=new QThread();
  thread->setObjectName("time-lapse-assembly");
  // TODO: join the thread before application termination
  QObject::connect(thread, &QThread::finished,
                   thread, &QThread::deleteLater);
  process = new AssemblyProcess(thread);
  process->moveToThread(thread);
  connect(thread, &QThread::started,
          process, &AssemblyProcess::init);
  thread->start();

  connect(this, &QmlTimeLapseAssembly::startRequest,
          process, &AssemblyProcess::start,
          Qt::QueuedConnection);

  connect(process, &AssemblyProcess::done,
          this, &QmlTimeLapseAssembly::cleanup,
          Qt::QueuedConnection);

  connect(process, &AssemblyProcess::error,
          this, &QmlTimeLapseAssembly::onError,
          Qt::QueuedConnection);

  connect(process, &AssemblyProcess::progress,
          this, &QmlTimeLapseAssembly::onProgress,
          Qt::QueuedConnection);

  emit startRequest(params);
  emit processingChanged();
}

void QmlTimeLapseAssembly::cancel() {
  if (process!=nullptr) {
    process->deleteLater();
    process = nullptr;
    emit processingChanged();
  }
}

QStringList QmlTimeLapseAssembly::getVideoDirectories() const {
  return videoDirectories;
}

void QmlTimeLapseAssembly::setVideoDirectories(const QStringList &l) {
  videoDirectories=l;
}
