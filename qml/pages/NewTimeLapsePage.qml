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

import QtQuick 2.5
import Sailfish.Silica 1.0
import Sailfish.Share 1.0
import Nemo.KeepAlive 1.2
import Nemo.Notifications 1.0
import Nemo.DBus 2.0

import QtGraphicalEffects 1.0
import QtMultimedia 5.6

import harbour.timelapsetools 1.0

import "../custom"
import "../custom/Utils.js" as Utils

Page {
    id: newTimeLapsePage

    allowedOrientations: Orientation.Landscape
    property var camera: null

    property int settingWidth: Math.min(newTimeLapsePage.width / 3, 1200)

    MediaPlayer {
        id: emptyPlayer
        autoPlay: true
    }

    VideoOutput {
        id: viewFinder

        x: newTimeLapsePage.settingWidth
        y: Theme.fontSizeLarge + Theme.paddingLarge
        width: parent.width - x
        height: parent.height - y

        fillMode: PreserveAspectFit
        //autoOrientation: true
        orientation: 90
        /*
        function updateOrientation() {
            switch (newTimeLapsePage.orientation) {
            case Orientation.Portrait:
                viewFinder.orientation = 90;
                break;
            case Orientation.Landscape:
                viewFinder.orientation = 90;
                break;
            case Orientation.PortraitInverted:
                viewFinder.orientation = 180;
                break;
            case Orientation.LandscapeInverted:
                viewFinder.orientation = 270;
                break;
            default:
                viewFinder.orientation = 0;
            }
        }
        */
    }

    states: [
        State { name: "Generic" },
        State { name: "Exposure" },
        State { name: "Focus" }
    ]
    state: "Generic"

    onStateChanged: {
        console.log("State changed: "+ state);
    }

    Row {
        id: sectionRow
        y: Theme.paddingSmall
        x: Math.min((parent.width - sectionRow.width) /2, Theme.horizontalPageMargin *3)

        spacing: Theme.paddingLarge
        Label {
            text: qsTr("Generic")
            font.pixelSize: Theme.fontSizeLarge
            color: newTimeLapsePage.state == "Generic" ? Theme.highlightColor : Theme.primaryColor
            MouseArea {
                anchors.fill: parent
                onClicked: { newTimeLapsePage.state = "Generic" }
            }
        }
        Label {
            text: qsTr("Exposure")
            font.pixelSize: Theme.fontSizeLarge
            color: newTimeLapsePage.state == "Exposure" ? Theme.highlightColor : Theme.primaryColor
            MouseArea {
                anchors.fill: parent
                onClicked: { newTimeLapsePage.state = "Exposure" }
            }
        }
        Label {
            text: qsTr("Focus")
            font.pixelSize: Theme.fontSizeLarge
            color: newTimeLapsePage.state == "Focus" ? Theme.highlightColor : Theme.primaryColor
            MouseArea {
                anchors.fill: parent
                onClicked: { newTimeLapsePage.state = "Focus" }
            }
        }
    }

    CameraModel {
        id: cameraModel
    }
    TimeLapseCapture {
        id: timelapseCapture
    }

    Column {
        anchors {
            top: sectionRow.bottom
            left: parent.left
            bottom: parent.bottom
        }
        //width: Math.min(parent.width / 2, Math.max(1200, parent.width))
        width: newTimeLapsePage.settingWidth

        visible: newTimeLapsePage.state == "Generic"

        ComboBox {
            id: cameraComboBox
            width: parent.width

            property bool initialized: false
            property string frontFaceLabel: qsTr("FrontFace")
            property string backFaceLabel: qsTr("BackFace")

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

            Timer {
                id: startCameraDelay
                interval: 10
                repeat: false
                onTriggered: {
                    if (newTimeLapsePage.camera!=null) {
                        newTimeLapsePage.camera.start();
                    }
                }
            }

            function setupCamera() {
                if (newTimeLapsePage.camera!=null) {
                    newTimeLapsePage.camera.stop();
                }
                var idx = cameraModel.index(cameraComboBox.currentIndex, 0);
                newTimeLapsePage.camera = cameraModel.data(idx, CameraModel.CameraObjectRole);
                if (newTimeLapsePage.camera.mediaObject != null) {
                    viewFinder.visible = true;
                    if (cameraModel.data(idx, CameraModel.PositionRole) == "FrontFace") {
                        viewFinder.orientation = 270;
                    } else {
                        viewFinder.orientation = 90;
                    }
                    viewFinder.source = newTimeLapsePage.camera
                    startCameraDelay.start();
                } else {
                    viewFinder.source = emptyPlayer;
                    //viewFinder.visible = false;
                }
            }

            onCurrentItemChanged: {
                if (!initialized){
                    return;
                }
                setupCamera()
            }
            Component.onCompleted: {
                initialized = true;
                console.log("camera model: " + cameraModel.rowCount());
                setupCamera();
            }
            onPressAndHold: {
                // improve default ComboBox UX :-)
                clicked(mouse);
            }
        }
        Label {
            text: qsTr("Device: %1").arg(newTimeLapsePage.camera.device)
            visible: newTimeLapsePage.camera.device != ""
            x: Theme.horizontalPageMargin
        }
        Label {
            text: qsTr("Resolution: %1x%2")
                .arg(newTimeLapsePage.camera.resolution.width)
                .arg(newTimeLapsePage.camera.resolution.height)
            visible: newTimeLapsePage.camera.resolution.width > 0 && newTimeLapsePage.camera.resolution.height > 0
            x: Theme.horizontalPageMargin
        }
        TextField {
            label: qsTr("Capture interval [ms]")
            text: "1000"
            validator: RegExpValidator { regExp: /^[0-9]{3,}$/ }
            inputMethodHints: Qt.ImhDigitsOnly
        }
        ComboBox {
            id: destinationDirectoryComboBox

            property bool initialized: false
            property string selected: updateDirectory
            property ListModel directories: ListModel {}

            label: qsTr("Directory")
            menu: ContextMenu {
                id: contextMenu
                Repeater {
                    model: destinationDirectoryComboBox.directories
                    MenuItem {
                        text: Utils.humanDirectory(dir)
                    }
                }
            }

            onCurrentItemChanged: {
                if (!initialized){
                    console.log("NOT initialized");
                    return;
                }
                var dirs=timelapseCapture.recordDirectories;
                //var dirs=[]; //mapDownloadsModel.getLookupDirectories();
                selected = dirs[currentIndex];
                //selected = directories[currentIndex].dir
                console.log("changed, currentIndex=" + destinationDirectoryComboBox.currentIndex + " selected: " + selected);
            }
            Component.onCompleted: {
                var dirs=timelapseCapture.recordDirectories;
                //var dirs=[]; //mapDownloadsModel.getLookupDirectories();
                for (var i in dirs){
                    var dir = dirs[i];
                    if (selected==""){
                        selected=dir;
                    }
                    console.log("Dir: "+dir);
                    directories.append({"dir": dir});
                }
                initialized = true;
                console.log("initialized, currentIndex=" + destinationDirectoryComboBox.currentIndex);
            }
        }
        TextField {
            id: imageCntField
            label: qsTr("Image count (0 is unlimited)")
            text: "0"
            validator: RegExpValidator { regExp: /^[0-9]{3,}$/ }
            inputMethodHints: Qt.ImhDigitsOnly
        }
        Label {
            property string captureDuration: "?"
            property string videoDuration: "?"
            text: qsTr("Capture duration: %1, video duration: %2 (30 fps)")
                .arg().arg()
            visible: imageCntField!="0"
            x: Theme.horizontalPageMargin
            font.pixelSize: Theme.fontSizeSmall
        }
    }

    onOrientationChanged: {
        console.log("Orientation: " + orientation);
        //viewFinder.updateOrientation();
    }

}
