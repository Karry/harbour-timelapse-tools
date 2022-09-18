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

#include <BatteryMonitor.h>

#include <QDir>
#include <QDebug>
#include <QFile>

BatteryMonitor::BatteryMonitor() {
  QDir battery("/sys/class/power_supply");
  QStringList filters;
  filters << "*battery";
  battery.setNameFilters(filters);

  const QStringList files = battery.entryList();
  QStringListIterator iterator(files);

  if (iterator.hasNext()) {
    batteryRoot = battery.absoluteFilePath(iterator.next());
    qDebug() << "Found battery root: " << batteryRoot;

    update();
    connect(&timer, &QTimer::timeout, this, &BatteryMonitor::update);
    timer.start(10000);
  }
}

void BatteryMonitor::update() {
  QFile capacityFile(batteryRoot + "/capacity");
  if (capacityFile.open(QFile::ReadOnly)){
    QByteArray content = capacityFile.readAll();
    level = QString(content).toInt();
    capacityFile.close();

    emit updated();
  } else {
    qWarning() << "Cannot read " << capacityFile.fileName();
  }
}
