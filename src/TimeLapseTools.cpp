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

#ifdef QT_QML_DEBUG
#include <QtQuick>
#endif

#include <sstream>
#include "IconProvider.h"

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

  QScopedPointer<QQuickView> view(SailfishApp::createView());
  view->rootContext()->setContextProperty("TimeLapseToolsVersionString", TIMELAPSE_TOOLS_VERSION_STRING);
  view->engine()->addImageProvider(QLatin1String("harbour-osmscout"), new IconProvider());
  view->setSource(SailfishApp::pathTo("qml/main.qml"));
  view->showFullScreen();
  int result=app->exec();

  return result;
}

