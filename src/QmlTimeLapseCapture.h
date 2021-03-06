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

#pragma once

#include <TimeLapse/pipeline_cpt.h>
#include <TimeLapse/capture.h>

#include <QmlCameraDevice.h>

#include <QObject>
#include <QStringList>
#include <QTextStream>

#include <cstdio>

class QmlTimeLapseCapture: public timelapse::TimeLapseCapture {
  Q_OBJECT

  Q_PROPERTY(QStringList recordDirectories READ getRecordDirectories)

  Q_PROPERTY(QmlCameraDevice* camera READ getCamera WRITE setCamera)

  Q_PROPERTY(QString dir READ getDir  WRITE setDir)
  Q_PROPERTY(QString baseDir READ getBaseDir  WRITE setBaseDir)

  Q_PROPERTY(bool adaptiveShutterSpeed READ getAdaptiveShutterSpeed WRITE setAdaptiveShutterSpeed)
  Q_PROPERTY(QString shutterSpeedStr READ getShutterSpeedStr WRITE setShutterSpeedStr)
  Q_PROPERTY(QString maxShutterSpeedStr READ getMaxShutterSpeedStr WRITE setMaxShutterSpeedStr)

public slots :
  void start() override;

public:
  QmlTimeLapseCapture();
  QmlTimeLapseCapture(const QmlTimeLapseCapture&) = delete;
  QmlTimeLapseCapture(QmlTimeLapseCapture&&) = delete;
  ~QmlTimeLapseCapture() override = default;
  QmlTimeLapseCapture& operator=(const QmlTimeLapseCapture&) = delete;
  QmlTimeLapseCapture& operator=(QmlTimeLapseCapture&&) = delete;

  QmlCameraDevice* getCamera() {
    return new QmlCameraDevice(getDevice());
  }

  void setCamera(QmlCameraDevice *camera) {
    if (camera!=nullptr) {
      setDevice(camera->timelapseDevice());
    }
  }

  QString getDir() {
    return _dir;
  }

  void setDir(const QString &dir) {
    _dir=dir;
  }

  QString getBaseDir() {
    return _baseDir;
  }

  void setBaseDir(const QString &baseDir) {
    _baseDir=baseDir;
  }

  bool getAdaptiveShutterSpeed() {
    return _adaptiveShutterSpeed;
  }

  void setAdaptiveShutterSpeed(bool adaptiveShutterSpeed) {
    _adaptiveShutterSpeed=adaptiveShutterSpeed;
  }

  QString getShutterSpeedStr() {
    return getMinShutterSpeed().toString();
  }

  void setShutterSpeedStr(const QString &shutterSpeed) {
    setMinShutterSpeed(ShutterSpeedChoice(shutterSpeed));
  }

  QString getMaxShutterSpeedStr() {
    return getMaxShutterSpeed().toString();
  }

  void setMaxShutterSpeedStr(const QString &maxShutterSpeed) {
    setMaxShutterSpeed(ShutterSpeedChoice(maxShutterSpeed));
  }

  QStringList getRecordDirectories() const;

  static void setRecordDirectories(const QStringList &l);

private:
  QTextStream err;
  QTextStream verboseOutput;
  bool _adaptiveShutterSpeed=false;
  QString _dir;
  QString _baseDir;
};
