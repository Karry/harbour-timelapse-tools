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
  thread(t), err(stderr), verboseOutput(stdout)
{}

AssemblyProcess::~AssemblyProcess() {
  if (pipeline != nullptr) {
    pipeline->deleteLater();
    pipeline = nullptr;
  }
  if (tempDir != nullptr) {
    delete tempDir;
    tempDir = nullptr;
  }

  assert(thread != nullptr);
  thread->quit();
  thread = nullptr;
}

void AssemblyProcess::init() {

}

void AssemblyProcess::start(const QmlTimeLapseAssembly::AssemblyParams params) {
  using namespace timelapse;

  QFileInfo output(params.dir + QDir::separator() + params.name + ".mp4");
  if (output.exists()) {
    emit error(tr("Output file exists already."));
    return;
  }

  // unpack ffmpeg
  // cd .cache/cz.karry.timelapse/TimeLapseTools/
  // tar -xf /usr/share/harbour-timelapse-tools/bin/ffmpeg.tar
  QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
  QString ffmpegBinary = cacheDir + QDir::separator() + "ffmpeg";
  if (!QFileInfo(ffmpegBinary).exists()) {
    try {
      tar::tar_reader tar("/usr/share/harbour-timelapse-tools/bin/ffmpeg.tar");
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
  tempDir = new QTemporaryDir(params.source + QDir::separator() + ".tmp");
  if (!tempDir->isValid()) {
    emit error("Can't create temp directory");
    return;
  }

  qDebug() << "Tmp dir: " << QDir::tempPath();
  tempDir->setAutoRemove(true);
  qDebug() << "Using temp directory " << tempDir->path();

  int _width=-1;
  int _height=-1;
  QString _bitrate;
  QString _codec;
  switch (params.profile) {
    case QmlTimeLapseAssembly::HDx264_Low:
      _width = 1920;
      _height = 1080;
      _bitrate = "20000k";
      _codec = "libx264";
      break;
    case QmlTimeLapseAssembly::HDx264_High:
      _width = 1920;
      _height = 1080;
      _bitrate = "40000k";
      _codec = "libx264";
      break;
    case QmlTimeLapseAssembly::HDx265:
      _width = 1920;
      _height = 1080;
      _bitrate = "40000k";
      _codec = "libx265";
      break;
    case QmlTimeLapseAssembly::UHDx265:
      _width = 3840;
      _height = 2160;
      _bitrate = "60000k";
      _codec = "libx265";
      break;
    default:
      assert(false);
  }

  // build processing pipeline
  QStringList inputArguments;
  inputArguments << params.source;
  pipeline = Pipeline::createWithFileSource(inputArguments, params.fileSuffixes, false, &verboseOutput, &err);

  if (params.deflicker == QmlTimeLapseAssembly::Deflicker::Average || params.deflicker == QmlTimeLapseAssembly::Deflicker::MovingAverage) {
    *pipeline << new ComputeLuminance(&verboseOutput);
  }

  if (params.length < 0) {
    *pipeline << new OneToOneFrameMapping();
  } else if (params.noStrictInterval) {
    *pipeline << new ImageMetadataReader(&verboseOutput, &err);
    *pipeline << new VariableIntervalFrameMapping(&verboseOutput, &err, params.length, params.fps);
  } else {
    *pipeline << new ConstIntervalFrameMapping(&verboseOutput, &err, params.length, params.fps);
  }

  if (params.deflicker == QmlTimeLapseAssembly::Deflicker::Average ||
      params.deflicker == QmlTimeLapseAssembly::Deflicker::MovingAverage) {
    if (params.deflicker == QmlTimeLapseAssembly::Deflicker::MovingAverage) {
      *pipeline << new WMALuminance(&verboseOutput, params.wmaCount);
    } else {
      *pipeline << new ComputeAverageLuminance(&verboseOutput);
    }
    *pipeline << new AdjustLuminance(&verboseOutput, params.deflickerDebugView);
  }

  if (params.blendFrames) {
    if (params.blendBeforeResize) {
      *pipeline << new BlendFramePrepare(&verboseOutput);
      *pipeline << new ResizeFrame(&verboseOutput, _width, _height, params.adaptiveResize);
    } else {
      *pipeline << new ResizeFrame(&verboseOutput, _width, _height, params.adaptiveResize);
      *pipeline << new BlendFramePrepare(&verboseOutput);
    }
  } else {
    *pipeline << new ResizeFrame(&verboseOutput, _width, _height, params.adaptiveResize);
    *pipeline << new FramePrepare(&verboseOutput);
  }
  *pipeline << new WriteFrame(QDir(tempDir->path()), &verboseOutput, false);

  *pipeline << new VideoAssembly(QDir(tempDir->path()), &verboseOutput, &err, false,
                                 output,
                                 _width, _height, params.fps, _bitrate, _codec,
                                 ffmpegBinary);

  connect(pipeline, &Pipeline::done, this, &AssemblyProcess::done);
  connect(pipeline, &Pipeline::error, this, &AssemblyProcess::error);
  emit pipeline->process();
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

  emit startRequest(params);
  emit processingChanged();
}

QStringList QmlTimeLapseAssembly::getVideoDirectories() const {
  return videoDirectories;
}

void QmlTimeLapseAssembly::setVideoDirectories(const QStringList &l) {
  videoDirectories=l;
}
