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

QDateTime birthTime(QDir dir) {
#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
  return QFileInfo(dir.path()).created();
#else
  return QFileInfo(dir.path()).birthTime();
#endif
}

bool dirComparator(const Dir &d1, const Dir &d2) {
  return d1.birthTime > d2.birthTime;
}

void TimeLapseModel::update() {
  beginResetModel();

  timelapses.clear();
  dirWatcher.removePaths(dirWatcher.directories());
  QmlTimeLapseCapture capture;
  dirWatcher.addPaths(capture.getRecordDirectories());
  for (const QString &baseDir: capture.getRecordDirectories()) {
    QDirIterator dirIt(baseDir, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::FollowSymlinks);
    while (dirIt.hasNext()) {
      dirIt.next();
      QFileInfo fInfo(dirIt.filePath());
      if (fInfo.isDir()) {
        qDebug() << baseDir << "subdir:" << dirIt.filePath() << QDir(fInfo.filePath()).dirName();
        QDir dir(fInfo.filePath());
        timelapses << Dir{dir, birthTime(dir)};
      }
    }
    std::sort(timelapses.begin(), timelapses.end(), dirComparator);
  }
  qDebug() << "found" << timelapses.size() << "timelapses";

  endResetModel();
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
    case NameRole: return dir.dir.dirName();
    case PathRole: return dir.dir.path();
    case BirthTimeRole: return dir.birthTime;
  }
  return QVariant();
}

void TimeLapseModel::deleteTimeLapse(int row) {
  if(row < 0 || row >= (int)timelapses.size()) {
    return;
  }

  QDir dir = timelapses[row].dir;
  qDebug() << "Removing" << dir;
  dir.removeRecursively();
}

void TimeLapseModel::rename(int row, QString newName) {
  if(row < 0 || row >= (int)timelapses.size()) {
    return;
  }

  QDir dir = timelapses[row].dir;
  QDir parent = dir;
  parent.cdUp();
  qDebug() << "renaming" << dir.path() << "to" << (parent.path() + QDir::separator() + newName);
  if (!dir.rename(dir.path(), parent.path() + QDir::separator() + newName)) {
    qWarning() << "Renaming fails.";
  }
}

QHash<int, QByteArray> TimeLapseModel::roleNames() const {
  QHash<int, QByteArray> roles=QAbstractItemModel::roleNames();

  roles[NameRole]="name";
  roles[PathRole]="path";
  roles[BirthTimeRole]="birthTime";

  return roles;
}

Qt::ItemFlags TimeLapseModel::flags(const QModelIndex &index) const {
  if(!index.isValid()) {
    return Qt::ItemIsEnabled;
  }

  return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
