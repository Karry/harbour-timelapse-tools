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

QStringList QmlCameraDevice::getShutterSpeedOptions() {
  QStringList result;
  bool bulb=false;
  int64_t maxTime=0;
  for (auto &choice: dev->getShutterSpeedChoices()) {
    if (choice.isBulb()) {
      bulb=true;
    } else {
      result << choice.toString();
      maxTime=std::max(maxTime, choice.toSecond());
    }
  }
  if (bulb) {
    for (int64_t bulbTime=maxTime + (maxTime < 10 ? 1 : 5);
         bulbTime <= 300;
         bulbTime += (bulbTime < 10 ? 1 : (bulbTime < 60 ? 5 : 10))) {
      result << QString("Bulb:%1").arg(bulbTime);
    }
  }
  return result;
}

QStringList QmlCameraDevice::getApertureOptions() {
  return dev->getApertureChoices();
}

QStringList QmlCameraDevice::getIsoOptions() {
  return dev->getIsoChoices();
}

QStringList QmlCameraDevice::getFocusModeOptions() {
  return dev->getFocusModeChoices();
}

QString QmlCameraDevice::getShutterSpeed() {
  return dev->currentShutterSpeed().toString();
}

QString QmlCameraDevice::getAperture() {
  return dev->currentAperture();
}

QString QmlCameraDevice::getIso() {
  return dev->currentIso();
}

QString QmlCameraDevice::getFocusMode() {
  return dev->currentFocusMode();
}

void QmlCameraDevice::setShutterSpeed(const QString &shutterSpeed) {
  dev->setShutterSpeed(timelapse::ShutterSpeedChoice(shutterSpeed));
}

void QmlCameraDevice::setAperture(const QString &aperture) {
  dev->setAperture(aperture);
}

void QmlCameraDevice::setIso(const QString &iso) {
  dev->setIso(iso);
}

void QmlCameraDevice::setFocusMode(const QString &focusMode) {
  dev->setFocusMode(focusMode);
}

QString QmlCameraDevice::getFocusPointMode() {
  return dev->currentFocusPointMode();
}

void QmlCameraDevice::setFocusPointMode(const QString &mode) {
  dev->setFocusPointMode(mode);
}

QStringList QmlCameraDevice::getFocusPointModeOptions() {
  return dev->getFocusPointModeChoices();
}

QPointF QmlCameraDevice::getCustomFocusPoint() {
  return dev->customFocusPoint();
}

void QmlCameraDevice::setCustomFocusPoint(const QPointF &p) {
  qDebug() << "CustomFocusPoint" << p;
  dev->setCustomFocusPoint(p);
}

bool QmlCameraDevice::getFocusLockSupport() {
  return dev->focusLockSupported();
}

bool QmlCameraDevice::getPersistentFocusLock() {
  return dev->persistentFocusLock();
}

void QmlCameraDevice::setPersistentFocusLock(bool b) {
  dev->setPersistentFocusLock(b);
}

QString QmlCameraDevice::positionString(QCamera::Position position) {
  switch (position) {
    case QCamera::BackFace: return "BackFace";
    case QCamera::FrontFace: return "FrontFace";
    default: return "Unspecified";
  }
}
