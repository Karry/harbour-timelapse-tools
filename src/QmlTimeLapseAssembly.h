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
#include <TimeLapse/capture.h>

#include <QmlCameraDevice.h>

#include <QObject>
#include <QStringList>
#include <QTextStream>

#include <cstdio>

class QmlTimeLapseAssembly: public QObject {
  Q_OBJECT

public:
  enum Deflicker {
    NoDeflicker = 0,
    Average = 1,
    MovingAverage = 2
  };
  Q_ENUM(Deflicker)

  enum Profile {
    HDx264_Low = 0,
    HDx264_High = 1,
    HDx265 = 2,
    UHDx265 = 3
  };
  Q_ENUM(Profile)

  Q_PROPERTY(QString source READ getSource WRITE setSource)
  Q_PROPERTY(QString dir READ getDir WRITE setDir)
  Q_PROPERTY(QString name READ getName WRITE setName)

public slots:
  void start();

signals:
  void error(QString message);
  void progress(QString message);
  void finish();

public:
  QmlTimeLapseAssembly();
  QmlTimeLapseAssembly(const QmlTimeLapseAssembly&) = delete;
  QmlTimeLapseAssembly(QmlTimeLapseAssembly&&) = delete;
  ~QmlTimeLapseAssembly() override = default;
  QmlTimeLapseAssembly& operator=(const QmlTimeLapseAssembly&) = delete;
  QmlTimeLapseAssembly& operator=(QmlTimeLapseAssembly&&) = delete;

  QString getSource() const {
    return _source;
  }
  void setSource(const QString &s) {
    _source = s;
  }
  QString getDir() const {
    return _dir;
  }
  void setDir(const QString &s) {
    _dir = s;
  }
  QString getName() const {
    return _name;
  }
  void setName(const QString &n) {
    _name = n;
  }

  QStringList getVideoDirectories() const;

  static void setVideoDirectories(const QStringList &l);

private:
  QString _source;
  QString _dir;
  QString _name;
  Deflicker _deflicker = Deflicker::NoDeflicker;
  Profile _profile = Profile::HDx264_High;
  qreal _fps=25.0;

  /** length of output video in seconds.
   * if length < 0, then length will be count of inputs images / fps
   */
  qreal _length=0;

  /* It is useful when time interval between images is not fixed.
   * Input image to output video frame mapping will be computed from image
   * timestamp (EXIF metadata will be used).
   */
  bool _noStrictInterval=false;
  bool _blendFrames=false;

  QTextStream err;
  QTextStream verboseOutput;
};
