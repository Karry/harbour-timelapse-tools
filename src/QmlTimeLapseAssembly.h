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
#include <TimeLapse/pipeline.h>

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


  Q_PROPERTY(QStringList videoDirectories READ getVideoDirectories)

  Q_PROPERTY(QString source READ getSource WRITE setSource NOTIFY sourceChanged)
  Q_PROPERTY(QString dir READ getDir WRITE setDir NOTIFY dirChanged)
  Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
  Q_PROPERTY(int inputImageCount READ getInputImgCnt NOTIFY inputImgCntChanged)
  Q_PROPERTY(Deflicker deflicker READ getDeflicker WRITE setDeflicker NOTIFY deflickerChanged)
  Q_PROPERTY(int deflickerWmaCount READ getDeflickerWmaCount WRITE setDeflickerWmaCount NOTIFY deflickerWmaCountChanged)
  Q_PROPERTY(Profile profile READ getProfile WRITE setProfile NOTIFY profileChanged)
  Q_PROPERTY(qreal fps READ getFps WRITE setFps NOTIFY fpsChanged)
  Q_PROPERTY(qreal length READ getLength WRITE setLength NOTIFY lengthChanged)
  Q_PROPERTY(bool noStrictInterval READ getNoStrictInterval WRITE setNoStrictInterval NOTIFY noStrictIntervalChanged)
  Q_PROPERTY(bool blendFrames READ getBlendFrames WRITE setBlendFrames NOTIFY blendFramesChanged)
  Q_PROPERTY(bool processing READ getProcessing NOTIFY processingChanged)

public slots:
  void start();
  void cleanup();
  void onError(const QString &msg);

signals:
  void error(QString message);
  void progress(QString message);
  void finish();
  void processingChanged();

  void sourceChanged(QString);
  void dirChanged(QString);
  void nameChanged(QString);
  void inputImgCntChanged(int cnt);
  void deflickerChanged(Deflicker);
  void deflickerWmaCountChanged(int);
  void profileChanged(Profile);
  void fpsChanged(qreal);
  void lengthChanged(qreal);
  void noStrictIntervalChanged(bool);
  void blendFramesChanged(bool);

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
  void setSource(const QString &s);

  QString getDir() const {
    return _dir;
  }
  void setDir(const QString &s) {
    if (s != _dir) {
      _dir = s;
      emit dirChanged(_dir);
    }
  }
  QString getName() const {
    return _name;
  }
  void setName(const QString &n) {
    if (n != _name) {
      _name = n;
      emit nameChanged(_name);
    }
  }

  int getInputImgCnt() const {
    return inputImgCnt;
  }

  Deflicker getDeflicker() const {
    return _deflicker;
  }
  void setDeflicker(Deflicker deflicker) {
    if (_deflicker != deflicker) {
      _deflicker = deflicker;
      emit deflickerChanged(_deflicker);
    }
  }
  int getDeflickerWmaCount() const {
    return wmaCount;
  }
  void setDeflickerWmaCount(int c) {
    if (c != wmaCount) {
      wmaCount = c;
      emit deflickerWmaCountChanged(wmaCount);
    }
  }
  Profile getProfile() const {
    return _profile;
  }
  void setProfile(Profile profile) {
    if (_profile != profile) {
      _profile = profile;
      emit profileChanged(_profile);
    }
  }
  qreal getFps() const {
    return _fps;
  }
  void setFps(qreal fps) {
    if (fps != _fps) {
      _fps = fps;
      emit fpsChanged(_fps);
    }
  }
  qreal getLength() const {
    return _length;
  }
  void setLength(qreal length) {
    if (length != _length) {
      _length = length;
      emit lengthChanged(_length);
    }
  }
  bool getNoStrictInterval() const {
    return _noStrictInterval;
  }
  void setNoStrictInterval(bool noStrictInterval) {
    if (_noStrictInterval != noStrictInterval) {
      _noStrictInterval = noStrictInterval;
      emit noStrictIntervalChanged(_noStrictInterval);
    }
  }
  bool getBlendFrames() const {
    return _blendFrames;
  }
  void setBlendFrames(bool blendFrames) {
    if (_blendFrames != blendFrames) {
      _blendFrames = blendFrames;
      emit blendFramesChanged(_blendFrames);
    }
  }

  bool getProcessing() const {
    return _processing;
  }

  QStringList getVideoDirectories() const;

  static void setVideoDirectories(const QStringList &l);

private:
  QString _source;
  QStringList fileSuffixes;
  QString _dir;
  QString _name;
  Deflicker _deflicker = Deflicker::NoDeflicker;
  int wmaCount = 10;
  bool deflickerDebugView = false;
  Profile _profile = Profile::HDx264_High;
  qreal _fps=25.0;

  /** length of output video in seconds.
   * if length < 0, then length will be count of inputs images / fps
   */
  qreal _length=-1;

  /* It is useful when time interval between images is not fixed.
   * Input image to output video frame mapping will be computed from image
   * timestamp (EXIF metadata will be used).
   */
  bool _noStrictInterval=false;
  bool _blendFrames=false;
  bool _blendBeforeResize=false;
  bool _adaptiveResize=true;

  QTextStream err;
  QTextStream verboseOutput;

  int inputImgCnt = 0;

  bool _processing = false;
  timelapse::Pipeline *pipeline=nullptr;
  QTemporaryDir *_tempDir=nullptr;
};
