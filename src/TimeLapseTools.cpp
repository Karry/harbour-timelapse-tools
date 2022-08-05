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

#include <harbour-timelapse-tools/private/Version.h>
#include <IconProvider.h>
#include <TimeLapseTools.h>
#include <CameraModel.h>
#include <QmlTimeLapseAssembly.h>
#include <QmlTimeLapseCapture.h>
#include <TimeLapseModel.h>

#include <Arguments.h>

// SFOS
#include <sailfishapp/sailfishapp.h>

// Qt includes
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QStandardPaths>
#include <QQmlContext>
#include <QFileInfo>
#include <QtCore/QtGlobal>
#include <QSettings>
#include <QTextStream>

#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <QStorageInfo>
#include <QTranslator>

#include <magick/magick.h>
#include <magick/image.h>

#include <sstream>

#ifndef TIMELAPSE_TOOLS_VERSION_STRING
static_assert(false, "TIMELAPSE_TOOLS_VERSION_STRING should be defined by build system");
#endif

std::string osPrettyName(){
  QSettings osRelease("/etc/os-release", QSettings::IniFormat);
  QVariant prettyName=osRelease.value("PRETTY_NAME");
  if (prettyName.isValid()){
    return prettyName.toString().toStdString();
  } else {
    return "Unknown OS";
  }
}

std::string versionStrings(){
  std::stringstream ss;
  ss << "harbour-timelapse-tools"
     << " " << TIMELAPSE_TOOLS_VERSION_STRING
     << " (Qt " << qVersion() << ", " << osPrettyName() << ")";

  return ss.str();
}

Q_DECL_EXPORT int main(int argc, char* argv[]) {
#ifdef Q_WS_X11
  QCoreApplication::setAttribute(Qt::AA_X11InitThreads);
#endif

  QGuiApplication *app = SailfishApp::application(argc, argv);

  app->setOrganizationDomain("timelapse.karry.cz");
  app->setOrganizationName("cz.karry.timelapse"); // needed for Sailjail
  app->setApplicationName("TimeLapseTools");
  app->setApplicationVersion(TIMELAPSE_TOOLS_VERSION_STRING);

  Arguments args;
  {
    ArgParser argParser(app, argc, argv);

    osmscout::CmdLineParseResult argResult = argParser.Parse();
    if (argResult.HasError()) {
      std::cerr << "ERROR: " << argResult.GetErrorDescription() << std::endl;
      std::cout << argParser.GetHelp() << std::endl;
      return 1;
    }

    args = argParser.GetArguments();
    if (args.help) {
      std::cout << argParser.GetHelp() << std::endl;
      return 0;
    }
    if (args.version) {
      std::cout << versionStrings() << std::endl;
      return 0;
    }
  }

  std::cout << "Starting " << versionStrings() << std::endl;

#ifdef QT_QML_DEBUG
  qWarning() << "Starting QML debugger on port 1234.";
  qQmlEnableDebuggingHelper.startTcpDebugServer(1234);
#endif

  // setup c++ locale
  try {
    std::locale::global(std::locale(""));
  } catch (const std::runtime_error& e) {
    std::cerr << "Cannot set locale: \"" << e.what() << "\"" << std::endl;
  }

  // install translator
  QTranslator translator;
  QLocale locale;
  if (translator.load(locale.name(), SailfishApp::pathTo("translations").toLocalFile())) {
    qDebug() << "Install translator for locale " << locale << "/" << locale.name();
    app->installTranslator(&translator);
  }else{
    qWarning() << "Can't load translator for locale" << locale << "/" << locale.name() <<
               "(" << SailfishApp::pathTo("translations").toLocalFile() << ")";
  }

  timelapse::registerQtMetaTypes();

  qmlRegisterType<CameraModel>("harbour.timelapsetools", 1, 0, "CameraModel");
  qmlRegisterType<QmlTimeLapseAssembly>("harbour.timelapsetools", 1, 0, "TimeLapseAssembly");
  qmlRegisterType<QmlTimeLapseCapture>("harbour.timelapsetools", 1, 0, "TimeLapseCapture");
  qmlRegisterType<TimeLapseModel>("harbour.timelapsetools", 1, 0, "TimeLapseModel");

  qRegisterMetaType<QmlTimeLapseAssembly::AssemblyParams>("AssemblyParams");

  {
    QStringList videoDirectories;
    videoDirectories << QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);

    QString pictureDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QStringList storeDirectories;
    storeDirectories << pictureDir + QDir::separator() + ".timelapse";

    for (const QStorageInfo &storage : QStorageInfo::mountedVolumes()) {

      QString mountPoint = storage.rootPath();

      // Sailfish OS specific mount point base for SD cards!
      if (storage.isValid() &&
          storage.isReady() &&
          mountPoint.startsWith("/run/media/")) {

        qDebug() << "Found storage:" << mountPoint;
        storeDirectories << mountPoint + QDir::separator() + "Pictures" + QDir::separator() + ".timelapse";
        videoDirectories << mountPoint + QDir::separator() + "Videos";
      }
    }
    QmlTimeLapseCapture::setRecordDirectories(storeDirectories);
    QmlTimeLapseAssembly::setVideoDirectories(videoDirectories);
  }

  // setup ImageMagick, see https://imagemagick.org/script/resources.php
  // TODO: it is working?
  qputenv("MAGICK_CONFIGURE_PATH", "/usr/share/harbour-timelapse-tools/etc/ImageMagick-6");
  qputenv("MAGICK_HOME", "/usr/share/harbour-timelapse-tools");
  Magick::InitializeMagick(*argv);

  QScopedPointer<QQuickView> view(SailfishApp::createView());
  view->rootContext()->setContextProperty("TimeLapseToolsVersionString", TIMELAPSE_TOOLS_VERSION_STRING);
  view->engine()->addImageProvider(QLatin1String("harbour-osmscout"), new IconProvider());
  view->setSource(SailfishApp::pathTo("qml/main.qml"));
  view->showFullScreen();

  int result=app->exec();

  Magick::TerminateMagick();
  return result;
}

