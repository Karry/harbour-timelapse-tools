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

#include <CameraModel.h>

#include <QmlCameraDevice.h>
#include <QTextStream>

#include <TimeLapse/timelapse.h>
#include <TimeLapse/pipeline_cpt.h>
#include <TimeLapse/pipeline_cpt_v4l.h>
#include <TimeLapse/pipeline_cpt_gphoto2.h>
#include <TimeLapse/pipeline_write_frame.h>
#include <TimeLapse/pipeline_cpt_qcamera.h>


CameraModel::CameraModel(QObject *parent):
  QAbstractListModel(parent)
{
  devices = listDevices();
  QTextStream verboseOutput(stdout);
  if (devices.isEmpty()) {
    verboseOutput << QCoreApplication::translate("main", "No compatible capture device found");
  } else {
    verboseOutput << "Found devices: " << Qt::endl;
    for (QSharedPointer<timelapse::CaptureDevice> d : devices) {
      connect(d.data(), &timelapse::CaptureDevice::update, this, &CameraModel::onCameraUpdate);
      verboseOutput << "  " << d->toString() << Qt::endl;
    }
  }
}

void CameraModel::onCameraUpdate() {
  QObject *cam=sender();
  int i=0;
  for (QSharedPointer<timelapse::CaptureDevice> d : devices) {
    if (d.data() == cam) {
      emit dataChanged(index(i), index(0));
      return;
    }
    i++;
  }
}

int CameraModel::rowCount(const QModelIndex &) const {
  return devices.size();
}

QVariant CameraModel::data(const QModelIndex &index, int role) const {
  if(index.row() < 0 || index.row() >= (int)devices.size()) {
    return QVariant();
  }

  const auto &dev = devices[index.row()];
  switch(role){
    case NameRole: return dev->name();
    case BackendRole: return dev->backend();
    case DeviceRole: return dev->device();
    case PositionRole: return QmlCameraDevice::positionString(dev->position());
    case ResolutionRole: return dev->resolution();
    case CameraObjectRole: return QVariant::fromValue(new QmlCameraDevice(dev));
  }
  return QVariant();
}

QHash<int, QByteArray> CameraModel::roleNames() const {
  QHash<int, QByteArray> roles=QAbstractItemModel::roleNames();

  roles[NameRole]="name";
  roles[BackendRole]="backend";
  roles[DeviceRole]="device";
  roles[PositionRole]="position";
  roles[ResolutionRole]="resolution";
  roles[CameraObjectRole]="cameraObject";

  return roles;
}

Qt::ItemFlags CameraModel::flags(const QModelIndex &index) const {
  if(!index.isValid()) {
    return Qt::ItemIsEnabled;
  }

  return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QList<QSharedPointer<timelapse::CaptureDevice>> CameraModel::listDevices() {
  QList<QSharedPointer<timelapse::CaptureDevice>> result;

  QTextStream verboseOutput(stdout);
  QTextStream err(stderr);

  QList<timelapse::V4LDevice> v4lDevices = timelapse::V4LDevice::listDevices(&verboseOutput);
  for (const timelapse::V4LDevice &v4lDev: v4lDevices) {
    result.push_back(QSharedPointer<timelapse::CaptureDevice>(new timelapse::V4LDevice(v4lDev)));
  }

  try {
    QList<timelapse::Gphoto2Device> gp2devices = timelapse::Gphoto2Device::listDevices(&verboseOutput, &err);
    for (const timelapse::Gphoto2Device &gp2Dev: gp2devices) {
      result.push_back(QSharedPointer<timelapse::Gphoto2Device>(new timelapse::Gphoto2Device(gp2Dev)));
    }
  } catch (std::exception &e) {
    err << "Can't get Gphoto2 devices. " << QString::fromUtf8(e.what()) << Qt::endl;
  }

  {
    // Sailfish OS did not provide list of Qt cameras until we load at least default one...
    QCamera camera;
    camera.load();
  }

  QList<timelapse::QCameraDevice> qCamDevices = timelapse::QCameraDevice::listDevices(&verboseOutput);
  for (const timelapse::QCameraDevice &qCamDev: qCamDevices) {
    result.push_back(QSharedPointer<timelapse::QCameraDevice>(new timelapse::QCameraDevice(qCamDev)));
  }

  return result;
}
