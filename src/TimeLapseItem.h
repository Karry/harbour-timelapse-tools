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

#include <QDir>
#include <QDateTime>

#include <chrono>

class TimeLapseItem {
public:
  explicit TimeLapseItem(const QDir &dir);
  TimeLapseItem(const TimeLapseItem&) = default;
  TimeLapseItem(TimeLapseItem&&) = default;
  ~TimeLapseItem() = default;
  TimeLapseItem& operator=(const TimeLapseItem&) = default;
  TimeLapseItem& operator=(TimeLapseItem&&) = default;

  QDir getDir() const {
    return dir;
  }

  QString getName() const {
    return name;
  }

  void setName(const QString n){
    name=n;
  }

  QDateTime getCreation() const {
    return creation;
  }

  void setCreation(const QDateTime& dt) {
    creation = dt;
  }

  std::chrono::milliseconds getDuration() const {
    return duration;
  }

  void setDuration(const std::chrono::milliseconds &d) {
    duration = d;
  }

  QString getCamera() const {
    return camera;
  }

  void setCamera(const QString c) {
    camera = c;
  }

  bool writeMetadata() const;

private:
  static const char* FILE_METADATA;

  QString name;
  QDir dir;
  QDateTime creation;
  std::chrono::milliseconds duration;
  QString camera;
};
