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

#include <QmlTimeLapseCapture.h>

#include <TimeLapse/pipeline_cpt.h>

#include <TimeLapseItem.h>

#include <QUuid>

static QStringList recordDirectories;

QmlTimeLapseCapture::QmlTimeLapseCapture():
  timelapse::TimeLapseCapture(&err, &verboseOutput), // err and verboseOutput are not initialised in TimeLapseCapture constructor
  err(stderr), verboseOutput(stdout)
{
  connect(this, &TimeLapseCapture::done, this, &QmlTimeLapseCapture::onDone);
}

QmlTimeLapseCapture::~QmlTimeLapseCapture() {
  onDone();
}

void QmlTimeLapseCapture::onDone() {
  if (timer.isValid()) {
    TimeLapseItem metadata(getOutput());
    metadata.setDuration(std::chrono::milliseconds(timer.elapsed()));
    metadata.writeMetadata();

    timer.invalidate();
  }
}

void QmlTimeLapseCapture::start() {
  if (_adaptiveShutterSpeed) {
    setShutterSpeedChangeThreshold(2);
  }

#if QT_VERSION >= QT_VERSION_CHECK(5,7,0)
  _dirName = QUuid::createUuid().toString(QUuid::WithoutBraces) + "_";
#else
  _dirName = QUuid::createUuid().toString().remove('{').remove('}') + "_";
#endif

  for (QChar qch: _name) {
    char ch = qch.toLatin1();
    _dirName += (ch == 0 || ch == '/' || ch == '*' || ch == ' ' || ch == '\t') ?
                '_' : ch;
  }
  emit dirNameChanged();
  setOutput(QDir(_baseDir + QDir::separator() + _dirName));
  timelapse::TimeLapseCapture::start();

  TimeLapseItem metadata(getOutput());
  metadata.setName(_name);
  metadata.setCamera(getCameraDescription());
  metadata.setCreation(QDateTime::currentDateTime());
  metadata.writeMetadata();

  timer.start();
}

QString QmlTimeLapseCapture::getCameraDescription() {
  if (const QSharedPointer<timelapse::CaptureDevice> dev = getDevice(); dev) {
    return dev->backend() + ": " + dev->name();
  } else {
    return "";
  }
}

QStringList QmlTimeLapseCapture::getRecordDirectories() const {
  return recordDirectories;
}

void QmlTimeLapseCapture::setRecordDirectories(const QStringList &l) {
  recordDirectories=l;
}
