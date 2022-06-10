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

#include <QmlCameraDevice.h>
#include <QDebug>

#include <TimeLapse/pipeline_cpt_qcamera.h>

QmlCameraDevice::QmlCameraDevice(const QSharedPointer<timelapse::CaptureDevice>& dev):
  dev(dev) {

  connect(dev.data(), &timelapse::CaptureDevice::update, this, &QmlCameraDevice::update);
  connect(dev.data(), &timelapse::CaptureDevice::update, this, &QmlCameraDevice::onUpdate);
}

void QmlCameraDevice::onUpdate() {
  qDebug() << "updated resolution: " << dev->resolution();
}

void QmlCameraDevice::start() {
  dev->start();
}

void QmlCameraDevice::stop() {
  dev->stop();
}

QString QmlCameraDevice::positionString(QCamera::Position position) {
  switch (position) {
    case QCamera::BackFace: return "BackFace";
    case QCamera::FrontFace: return "FrontFace";
    default: return "Unspecified";
  }
}
