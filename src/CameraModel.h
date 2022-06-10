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
#include <QAbstractListModel>
#include <QtMultimedia/QCamera>

class CameraModel: public QAbstractListModel {
  Q_OBJECT

public slots :
  void onCameraUpdate();

public:
  explicit CameraModel(QObject *parent=nullptr);
  CameraModel(const CameraModel&) = delete;
  CameraModel(CameraModel &&) = delete;
  ~CameraModel() = default;
  CameraModel& operator=(const CameraModel&) = delete;
  CameraModel& operator=(CameraModel&&) = delete;

  enum Roles {
    NameRole = Qt::UserRole,
    BackendRole = Qt::UserRole + 1,
    DeviceRole = Qt::UserRole + 2,
    PositionRole = Qt::UserRole + 3,
    ResolutionRole = Qt::UserRole + 4,
    CameraObjectRole = Qt::UserRole + 5
  };
  Q_ENUM(Roles)

  Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;
  Q_INVOKABLE Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
  QList <QSharedPointer<timelapse::CaptureDevice>> listDevices();

private:
  QList <QSharedPointer<timelapse::CaptureDevice>> devices;
};
