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

#include <QTextStream>

#include <cassert>

static QStringList videoDirectories;

QmlTimeLapseAssembly::QmlTimeLapseAssembly():
  err(stderr), verboseOutput(stdout)
{
  if (!videoDirectories.empty()) {
    _dir = videoDirectories[0];
  }
  // TODO: configurable, support raw images...
  fileSuffixes << "jpeg";
  fileSuffixes << "jpg";
}

void QmlTimeLapseAssembly::setSource(const QString &s) {
  if (s == _source) {
    return;
  }
  _source = s;
  QDir d(_source);
  QFileInfoList l = d.entryInfoList(QDir::Files, QDir::Name);
  inputImgCnt=0;
  for (QFileInfo i2 : l) {
    if ((i2.isFile() || i2.isSymLink()) &&
        (fileSuffixes.isEmpty() || fileSuffixes.contains(i2.completeSuffix(), Qt::CaseInsensitive))) {
      inputImgCnt++;
    }
  }
  emit inputImgCntChanged(inputImgCnt);
  emit sourceChanged(_source);
}

void QmlTimeLapseAssembly::onError(const QString &msg) {
  emit error(msg);
  cleanup();
}

void QmlTimeLapseAssembly::cleanup() {
  if (pipeline != nullptr) {
    pipeline->deleteLater();
    pipeline = nullptr;
  }
  if (_tempDir != nullptr) {
    delete _tempDir;
    _tempDir = nullptr;
  }
  _processing = false;
  emit processingChanged();
}

void QmlTimeLapseAssembly::start() {
  // TODO: unpack ffmpeg
  // cd .cache/cz.karry.timelapse/TimeLapseTools/
  // tar -xf /usr/share/harbour-timelapse-tools/bin/ffmpeg.tar
  // chmod +x .cache/cz.karry.timelapse/TimeLapseTools/ffmpeg
  using namespace timelapse;

  // check temp dir
  _tempDir = new QTemporaryDir(_source + QDir::separator() + ".tmp");
  if (!_tempDir->isValid()) {
    emit error("Can't create temp directory");
    return;
  }

  qDebug() << "Tmp dir: " << QDir::tempPath();
  _tempDir->setAutoRemove(true);
  qDebug() << "Using temp directory " << _tempDir->path();

  int _width=-1;
  int _height=-1;
  QString _bitrate;
  QString _codec;
  switch (_profile) {
    case HDx264_Low:
      _width = 1920;
      _height = 1080;
      _bitrate = "20000k";
      _codec = "libx264";
      break;
    case HDx264_High:
      _width = 1920;
      _height = 1080;
      _bitrate = "40000k";
      _codec = "libx264";
      break;
    case HDx265:
      _width = 1920;
      _height = 1080;
      _bitrate = "40000k";
      _codec = "libx265";
      break;
    case UHDx265:
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
  inputArguments << _source;
  pipeline = Pipeline::createWithFileSource(inputArguments, fileSuffixes, false, &verboseOutput, &err);

  if (_deflicker == Deflicker::Average || _deflicker == Deflicker::MovingAverage) {
    *pipeline << new ComputeLuminance(&verboseOutput);
  }

  if (_length < 0) {
    *pipeline << new OneToOneFrameMapping();
  } else if (_noStrictInterval) {
    *pipeline << new ImageMetadataReader(&verboseOutput, &err);
    *pipeline << new VariableIntervalFrameMapping(&verboseOutput, &err, _length, _fps);
  } else {
    *pipeline << new ConstIntervalFrameMapping(&verboseOutput, &err, _length, _fps);
  }

  if (_deflicker == Deflicker::Average || _deflicker == Deflicker::MovingAverage) {
    if (_deflicker == Deflicker::MovingAverage) {
      *pipeline << new WMALuminance(&verboseOutput, wmaCount);
    } else {
      *pipeline << new ComputeAverageLuminance(&verboseOutput);
    }
    *pipeline << new AdjustLuminance(&verboseOutput, deflickerDebugView);
  }

  if (_blendFrames) {
    if (_blendBeforeResize) {
      *pipeline << new BlendFramePrepare(&verboseOutput);
      *pipeline << new ResizeFrame(&verboseOutput, _width, _height, _adaptiveResize);
    } else {
      *pipeline << new ResizeFrame(&verboseOutput, _width, _height, _adaptiveResize);
      *pipeline << new BlendFramePrepare(&verboseOutput);
    }
  } else {
    *pipeline << new ResizeFrame(&verboseOutput, _width, _height, _adaptiveResize);
    *pipeline << new FramePrepare(&verboseOutput);
  }
  *pipeline << new WriteFrame(QDir(_tempDir->path()), &verboseOutput, false);

  *pipeline << new VideoAssembly(QDir(_tempDir->path()), &verboseOutput, &err, false,
                                 QFileInfo(_dir + QDir::separator() + _name + ".mp4"),
                                 _width, _height, _fps, _bitrate, _codec,
                                 // TODO: evaluate the binary path
                                 "/home/defaultuser/.cache/cz.karry.timelapse/TimeLapseTools/ffmpeg");

  connect(pipeline, &Pipeline::done, this, &QmlTimeLapseAssembly::cleanup);
  connect(pipeline, &Pipeline::error, this, &QmlTimeLapseAssembly::onError);

  // startup pipeline
  _processing = true;
  emit processingChanged();
  emit pipeline->process();
}

QStringList QmlTimeLapseAssembly::getVideoDirectories() const {
  return videoDirectories;
}

void QmlTimeLapseAssembly::setVideoDirectories(const QStringList &l) {
  videoDirectories=l;
}
