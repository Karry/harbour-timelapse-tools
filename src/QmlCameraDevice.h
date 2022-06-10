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

#include <QObject>
#include <QtMultimedia/QAbstractVideoSurface>

/**
 * QML Abstraction for TimeLapse camera device.
 * To support v4l and gPhoto2 cameras, we cannot use Qt Camera api easily.
 */
class QmlCameraDevice: public QObject {
  Q_OBJECT

  // https://doc.qt.io/qt-5/qml-qtmultimedia-videooutput.html#source-prop
  // NemoVideoTextureBackend is failing with videoSurface
  Q_PROPERTY(QMediaObject* mediaObject READ getViewFinderMedia)

  Q_PROPERTY(QString name READ getName NOTIFY update)
  Q_PROPERTY(QString backend READ getBackend NOTIFY update)
  Q_PROPERTY(QString device READ getDevice NOTIFY update)
  Q_PROPERTY(QString position READ getPosition NOTIFY update)
  Q_PROPERTY(QSize resolution READ getResolution NOTIFY update)

signals:
  void update();

public slots :
  void start();
  void stop();

  void onUpdate();

public:
  explicit QmlCameraDevice(const QSharedPointer<timelapse::CaptureDevice>& dev);
  ~QmlCameraDevice() = default;

  QMediaObject* getViewFinderMedia() {
    return dev->viewfinder();
  }
  QString getName() const {
    return dev->name();
  }
  QString getBackend() const {
    return dev->backend();
  }
  QString getDevice() const {
    return dev->device();
  }
  QString getPosition() const {
    return positionString(dev->position());
  }
  QSize getResolution() const {
    return dev->resolution();
  }

  static QString positionString(QCamera::Position position);

private:
  QSharedPointer<timelapse::CaptureDevice> dev;
};
