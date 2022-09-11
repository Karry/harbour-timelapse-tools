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

#include <TimeLapseItem.h>

#include <QObject>
#include <QAbstractListModel>
#include <QDir>
#include <QFileSystemWatcher>
#include <QDateTime>

class TimeLapseModel: public QAbstractListModel {
Q_OBJECT

public slots :
  void onDirectoryChanged(const QString &path);
  void deleteTimeLapse(int row);
  void rename(int row, QString newName);

public:
  explicit TimeLapseModel(QObject *parent=nullptr);
  TimeLapseModel(const TimeLapseModel&) = delete;
  TimeLapseModel(TimeLapseModel &&) = delete;
  ~TimeLapseModel() = default;
  TimeLapseModel& operator=(const TimeLapseModel&) = delete;
  TimeLapseModel& operator=(TimeLapseModel&&) = delete;

  enum Roles {
    NameRole = Qt::UserRole,
    PathRole = Qt::UserRole +1,
    CreationRole = Qt::UserRole + 2,
    DurationRole = Qt::UserRole + 3,
    CameraRole = Qt::UserRole + 4
  };
  Q_ENUM(Roles)

  Q_INVOKABLE int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;
  Q_INVOKABLE Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
  void update();

private:
  QFileSystemWatcher dirWatcher;
  QList <TimeLapseItem> timelapses;
};
