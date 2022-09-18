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

#include <QObject>
#include <QTimer>

class BatteryMonitor: public QObject {
  Q_OBJECT

  Q_PROPERTY(int level READ getLevel NOTIFY updated)
public slots:
  void update();

signals:
  void updated();

public:
  BatteryMonitor();
  BatteryMonitor(const BatteryMonitor &) = delete;
  BatteryMonitor(BatteryMonitor&&) = delete;
  BatteryMonitor& operator=(const BatteryMonitor&) = delete;
  BatteryMonitor& operator=(BatteryMonitor&&) = delete;

  int getLevel() const {
    return level;
  }

private:
  QTimer timer;
  QString batteryRoot;
  int level=-1;
};
