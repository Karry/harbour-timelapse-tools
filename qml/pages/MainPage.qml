/*
 TimeLapse tools for Sailfish OS
 Copyright (C) 2022  Lukas Karas

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

import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.Share 1.0
import Nemo.KeepAlive 1.2
import Nemo.Notifications 1.0
import Nemo.DBus 2.0

import QtGraphicalEffects 1.0

import harbour.timelapsetools 1.0

Page {
    id: mainPage

    Column{
        anchors.margins: Theme.horizontalPageMargin
        y: Theme.paddingLarge
        width: parent.width
        spacing: Theme.paddingLarge

        Button {
            anchors {
                horizontalCenter: parent.horizontalCenter
            }
            text: qsTr("New TimeLapse")
            onClicked: {
                pageStack.push(Qt.resolvedUrl("NewTimeLapsePage.qml"),{})
            }
        }
    }

}
