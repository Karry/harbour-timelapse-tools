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

#include <TimeLapseModel.h>
#include <QmlTimeLapseCapture.h>

#include <QDirIterator>

TimeLapseModel::TimeLapseModel(QObject *parent):
  QAbstractListModel(parent)
{
  update();
  connect(&dirWatcher, &QFileSystemWatcher::directoryChanged, this, &TimeLapseModel::onDirectoryChanged);
}

void TimeLapseModel::onDirectoryChanged(const QString &) {
  update();
}

bool timelapseComparator(const TimeLapseItem &d1, const TimeLapseItem &d2) {
  return d1.getCreation() > d2.getCreation();
}

void TimeLapseModel::update() {
  dirWatcher.removePaths(dirWatcher.directories());
  QmlTimeLapseCapture capture;
  dirWatcher.addPaths(capture.getRecordDirectories());

  QList<TimeLapseItem> freshList;
  for (const QString &baseDir: capture.getRecordDirectories()) {
    QDirIterator dirIt(baseDir, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::FollowSymlinks);
    while (dirIt.hasNext()) {
      dirIt.next();
      QFileInfo fInfo(dirIt.filePath());
      if (fInfo.isDir()) {
        qDebug() << baseDir << "subdir:" << dirIt.filePath() << QDir(fInfo.filePath()).dirName();
        QDir dir(fInfo.filePath());
        freshList << TimeLapseItem(dir);
      }
    }
    // std::sort(freshList.begin(), freshList.end(), timelapseComparator);
  }
  qDebug() << "found" << freshList.size() << "timelapses";

  // update model

  // remove disappearing
  QSet<QString> freshSet;
  for (const auto &i: freshList) {
    freshSet.insert(i.getDir().absolutePath());
  }

  for (int i=0; i<timelapses.size();) {
    if (freshSet.contains(timelapses.at(i).getDir().absolutePath())) {
      i++;
    } else {
      beginRemoveRows(QModelIndex(), i, i);
      timelapses.removeAt(i);
      endRemoveRows();
    }
  }

  // add new rows
  QSet<QString> existingSet;
  for (const auto &i: timelapses) {
    existingSet.insert(i.getDir().absolutePath());
  }

  for (int i=0; i<freshList.size(); i++) {
    const auto &entry=freshList.at(i);
    if (!existingSet.contains(entry.getDir().absolutePath())) {
      // find new place
      int j=0;
      for (; j<timelapses.size(); j++) {
        if (timelapseComparator(entry, timelapses.at(j))) {
          break;
        }
      }
      beginInsertRows(QModelIndex(), j, j);
      timelapses.insert(j, entry);
      endInsertRows();
    }
  }
}

int TimeLapseModel::rowCount(const QModelIndex &) const {
  return timelapses.size();
}

QVariant TimeLapseModel::data(const QModelIndex &index, int role) const {
  if(index.row() < 0 || index.row() >= (int)timelapses.size()) {
    return QVariant();
  }

  const auto &dir = timelapses[index.row()];
  switch(role){
    case NameRole: return dir.getName();
    case PathRole: return dir.getDir().path();
    case CreationRole: return dir.getCreation();
    case DurationRole: return int(dir.getDuration().count());
    case CameraRole: return dir.getCamera();
  }
  return QVariant();
}

void TimeLapseModel::deleteTimeLapse(int row) {
  if(row < 0 || row >= (int)timelapses.size()) {
    return;
  }

  QDir dir = timelapses[row].getDir();
  qDebug() << "Removing" << dir;
  dir.removeRecursively();
}

void TimeLapseModel::rename(int row, QString newName) {
  if(row < 0 || row >= (int)timelapses.size()) {
    return;
  }

  timelapses[row].setName(newName);
  if (!timelapses[row].writeMetadata()) {
    qWarning() << "Renaming fails.";
  }
}

QHash<int, QByteArray> TimeLapseModel::roleNames() const {
  QHash<int, QByteArray> roles=QAbstractItemModel::roleNames();

  roles[NameRole]="name";
  roles[PathRole]="path";
  roles[CreationRole]="creation";
  roles[DurationRole]="duration";
  roles[CameraRole]="camera";

  return roles;
}

Qt::ItemFlags TimeLapseModel::flags(const QModelIndex &index) const {
  if(!index.isValid()) {
    return Qt::ItemIsEnabled;
  }

  return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
