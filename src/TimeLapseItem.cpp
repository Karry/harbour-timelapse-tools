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

#include "TimeLapseItem.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

QDateTime birthTime(QDir dir) {
#if QT_VERSION < QT_VERSION_CHECK(5,10,0)
  return QFileInfo(dir.path()).created();
#else
  return QFileInfo(dir.path()).birthTime();
#endif
}

const char *TimeLapseItem::FILE_METADATA = "metadata.json";

TimeLapseItem::TimeLapseItem(const QDir &dir):
  dir(dir) {

  // metadata
  bool metadata=false;
  if (dir.exists(TimeLapseItem::FILE_METADATA)){
    QFile jsonFile(dir.filePath(TimeLapseItem::FILE_METADATA));
    jsonFile.open(QFile::OpenModeFlag::ReadOnly);
    QJsonDocument doc = QJsonDocument::fromJson(jsonFile.readAll());
    QJsonObject metadataObject = doc.object();
    if (metadataObject.contains("name") &&
        metadataObject.contains("creation") &&
        metadataObject.contains("duration") &&
        metadataObject.contains("camera")) {
      name = metadataObject["name"].toString();
#if QT_VERSION > QT_VERSION_CHECK(5, 8, 0) /* For compatibility with QT 5.6 */
      creation.setSecsSinceEpoch(metadataObject["creation"].toDouble());
#else
      creation.setTime_t(metadataObject["creation"].toDouble());
#endif
      duration = std::chrono::milliseconds(metadataObject["duration"].toInt());
      camera = metadataObject["camera"].toString();
      metadata=true;
    }
  }
  if (!metadata) {
    name = dir.dirName();
  }
}

bool TimeLapseItem::writeMetadata() const {
  QJsonObject metadata;
  metadata["name"] = getName();
#if QT_VERSION > QT_VERSION_CHECK(5, 8, 0) /* For compatibility with QT 5.6 */
  metadata["creation"] = (double)getCreation().toSecsSinceEpoch();
#else
  metadata["creation"] = (double)getCreation().toTime_t();
#endif
  metadata["duration"] = int(getDuration().count());
  metadata["camera"] = camera;

  QJsonDocument doc(metadata);
  QFile metadataFile(dir.filePath(TimeLapseItem::FILE_METADATA));
  metadataFile.open(QFile::OpenModeFlag::WriteOnly);
  metadataFile.write(doc.toJson());
  metadataFile.close();
  if (metadataFile.error() != QFile::FileError::NoError){
    qWarning() << "Failed to write metadata:" << metadataFile.errorString();
    return false;
  }
  return true;
}
