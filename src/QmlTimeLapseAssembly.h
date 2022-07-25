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
#include <QThread>

#include <cstdio>

enum Deflicker {
  NoDeflicker = 0,
  Average = 1,
  MovingAverage = 2
};

enum Profile {
  HDx264_Low = 0,
  HDx264_High = 1,
  HDx265 = 2,
  UHDx265 = 3
};

struct AssemblyParams {
  QString source;
  QStringList fileSuffixes;
  QString dir;
  QString name;
  Deflicker deflicker = Deflicker::NoDeflicker;
  int wmaCount = 10;
  bool deflickerDebugView = false;
  Profile profile = Profile::HDx264_High;
  qreal fps=25.0;

  /** length of output video in seconds.
   * if length < 0, then length will be count of inputs images / fps
   */
  qreal length=-1;

  /* It is useful when time interval between images is not fixed.
   * Input image to output video frame mapping will be computed from image
   * timestamp (EXIF metadata will be used).
   */
  bool noStrictInterval=false;
  bool blendFrames=false;
  bool blendBeforeResize=false;
  bool adaptiveResize=true;
};

class AssemblyProcess : public QObject {
  Q_OBJECT

signals:
  void error(QString msg);
  void done();

public slots:
  void init();
  void start(const AssemblyParams params);

public:
  explicit AssemblyProcess(QThread *thread);
  ~AssemblyProcess() override;

private:
  QThread *thread;

  QTextStream err;
  QTextStream verboseOutput;

  timelapse::Pipeline *pipeline=nullptr;
  QTemporaryDir *tempDir=nullptr;

};

class QmlTimeLapseAssembly: public QObject {
  Q_OBJECT

public:
  Q_ENUM(Deflicker)
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
  void startRequest(const AssemblyParams &params);

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
    return params.source;
  }
  void setSource(const QString &s);

  QString getDir() const {
    return params.dir;
  }
  void setDir(const QString &s) {
    if (s != params.dir) {
      params.dir = s;
      emit dirChanged(params.dir);
    }
  }
  QString getName() const {
    return params.name;
  }
  void setName(const QString &n) {
    if (n != params.name) {
      params.name = n;
      emit nameChanged(params.name);
    }
  }

  int getInputImgCnt() const {
    return inputImgCnt;
  }

  Deflicker getDeflicker() const {
    return params.deflicker;
  }
  void setDeflicker(Deflicker deflicker) {
    if (params.deflicker != deflicker) {
      params.deflicker = deflicker;
      emit deflickerChanged(params.deflicker);
    }
  }
  int getDeflickerWmaCount() const {
    return params.wmaCount;
  }
  void setDeflickerWmaCount(int c) {
    if (c != params.wmaCount) {
      params.wmaCount = c;
      emit deflickerWmaCountChanged(params.wmaCount);
    }
  }
  Profile getProfile() const {
    return params.profile;
  }
  void setProfile(Profile profile) {
    if (params.profile != profile) {
      params.profile = profile;
      emit profileChanged(params.profile);
    }
  }
  qreal getFps() const {
    return params.fps;
  }
  void setFps(qreal fps) {
    if (fps != params.fps) {
      params.fps = fps;
      emit fpsChanged(params.fps);
    }
  }
  qreal getLength() const {
    return params.length;
  }
  void setLength(qreal length) {
    if (length != params.length) {
      params.length = length;
      emit lengthChanged(params.length);
    }
  }
  bool getNoStrictInterval() const {
    return params.noStrictInterval;
  }
  void setNoStrictInterval(bool noStrictInterval) {
    if (params.noStrictInterval != noStrictInterval) {
      params.noStrictInterval = noStrictInterval;
      emit noStrictIntervalChanged(params.noStrictInterval);
    }
  }
  bool getBlendFrames() const {
    return params.blendFrames;
  }
  void setBlendFrames(bool blendFrames) {
    if (params.blendFrames != blendFrames) {
      params.blendFrames = blendFrames;
      emit blendFramesChanged(params.blendFrames);
    }
  }

  bool getProcessing() const {
    return process!=nullptr;
  }

  QStringList getVideoDirectories() const;

  static void setVideoDirectories(const QStringList &l);

private:
  AssemblyParams params;
  AssemblyProcess *process=nullptr;

  int inputImgCnt = 0;
};
