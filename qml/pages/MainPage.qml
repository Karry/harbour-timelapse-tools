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

    ComboBox {
        id: cameraComboBox
        width: parent.width

        property bool initialized: false
        property string frontFaceLabel: qsTr("FrontFace")
        property string backFaceLabel: qsTr("BackFace")

        CameraModel {
            id: cameraModel
        }

        label: qsTr("Camera")
        menu: ContextMenu {
            Repeater {
                width: parent.width
                model: cameraModel
                delegate: MenuItem {
                    text: model.backend + ": " + model.name +
                          (model.position != "Unspecified" ? " (" + qsTr(model.position) +")"  : "")
                }
            }
        }

        onCurrentItemChanged: {

        }
        Component.onCompleted: {
            initialized = true;
            console.log("camera model: " + cameraModel.rowCount());
        }
        onPressAndHold: {
            // improve default ComboBox UX :-)
            clicked(mouse);
        }
    }

}
