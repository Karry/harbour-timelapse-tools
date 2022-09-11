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
import Nemo.KeepAlive 1.2
import Nemo.Notifications 1.0
import Nemo.DBus 2.0

import QtGraphicalEffects 1.0
import QtMultimedia 5.6

import harbour.timelapsetools 1.0

import "../custom"
import "../custom/Utils.js" as Utils

Page {
    id: capturePage

    property string name: Utils.defaultName()
    property var camera: null
    property int interval: 1000
    property string baseDir: ""
    property int count: 0
    property bool adaptiveShutterSpeed: false
    property string shutterSpeed: "1/100"
    property string maxShutterSpeed: "1/100"

    TimeLapseCapture {
        id: capture

        camera: capturePage.camera
        dir: capturePage.name
        interval: capturePage.interval
        baseDir: capturePage.baseDir
        count: capturePage.count
        adaptiveShutterSpeed: capturePage.adaptiveShutterSpeed
        shutterSpeedStr: capturePage.shutterSpeed
        maxShutterSpeedStr: capturePage.maxShutterSpeed

        Component.onCompleted: {
            capture.start();
        }
        onImageCaptured: {
            console.log("captured to file: " + file);
        }
        onDone: {
            console.log("done");
        }
        onError: {
            console.log("error: " + msg);
        }
    }

    SilicaFlickable{
        id: flickable
        anchors.fill: parent
        contentHeight: content.height + Theme.paddingMedium

        VerticalScrollDecorator {}

        Column {
            id: content
            x: Theme.paddingMedium
            width: parent.width - 2*Theme.paddingMedium

            SectionHeader{ text: qsTr("Capturing details") }

            DetailItem {
                label: qsTr("Folder")
                value: Utils.humanDirectory(capture.baseDir)
            }
            DetailItem {
                label: qsTr("Name")
                value: capture.dir
            }
            DetailItem {
                label: qsTr("Picture count")
                value: capture.capturedCount
            }
        }
    }
}
