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

    RemorsePopup { id: remorse }

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
        MouseArea {
            id: focusMouseArea
            anchors.fill: parent
            onClicked: {
                if (camera!=null && (camera.focusMode === "Auto" || camera.focusMode === "Continuous" || camera.focusMode === "Macro")) {
                    // var viewFinderPoint = Qt.point(mouse.x, mouse.y);
                    var viewFinderPoint = Qt.point(mouse.y, focusMouseArea.width - mouse.x);
                    console.log("viewFinderPoint: " + viewFinderPoint + " (" + focusMouseArea.width + " x " + focusMouseArea.height + ")");
                    camera.focusPointMode = "Custom";
                    camera.customFocusPoint = viewFinder.mapPointToSourceNormalized(viewFinderPoint);
                    focusCircle.x = mouse.x - focusCircle.width / 2;
                    focusCircle.y = mouse.y - focusCircle.height / 2;
                    console.log("focus circle: " + focusCircle.x + " x " + focusCircle.y + "");
                }
            }
        }
        Rectangle {
            id: focusCircle
            height: Theme.itemSizeMedium
            width: height
            radius: width / 2
            x: parent.width / 2 - focusCircle.width / 2
            y: parent.height / 2 - focusCircle.width / 2
            border.width: 4
            border.color: "white"
            color: "transparent"
            visible: camera.focusPointMode == "Custom" && (camera.focusMode === "Auto" || camera.focusMode === "Continuous" || camera.focusMode === "Macro")
        }
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



    SilicaFlickable {
        id: flickable
        anchors {
            top: sectionRow.bottom
            left: parent.left
            bottom: parent.bottom
        }
        width: newTimeLapsePage.settingWidth
        //contentHeight: content.height + header.height + 2*Theme.paddingLarge

        VerticalScrollDecorator {}

        Column {
            //width: Math.min(parent.width / 2, Math.max(1200, parent.width))
            width: parent.width

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
                id: nameField
                label: qsTr("Name")
                text: Qt.formatDateTime(new Date())
                validator: RegExpValidator { regExp: /[^\/*:]{1,}/ }
            }
            TextField {
                id: intervalField
                label: qsTr("Capture interval [ms]")
                text: "1000"
                validator: RegExpValidator { regExp: /^[0-9]{3,}$/ }
                inputMethodHints: Qt.ImhDigitsOnly
                onTextChanged: {
                    durationLabel.update()
                }
            }
            ComboBox {
                id: destinationDirectoryComboBox

                property bool initialized: false
                property string selected: ""
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
                validator: RegExpValidator { regExp: /^[0-9]{1,}$/ }
                inputMethodHints: Qt.ImhDigitsOnly
                onTextChanged: {
                    durationLabel.update()
                }
            }
            Label {
                id: durationLabel
                function update() {
                    captureDuration = Utils.humanDurationLong((parseInt(intervalField.text, 10)/1000)
                                                              * parseInt(imageCntField.text, 10))
                    videoDuration = Utils.humanDurationLong(parseInt(imageCntField.text, 10) / 30)
                }

                property string captureDuration: "?"
                property string videoDuration: "?"
                text: qsTr("Capture duration: %1, \n" +
                           "video duration: %2 @ 30 fps")
                    .arg(captureDuration).arg(videoDuration)
                width: parent.width - (Theme.horizontalPageMargin * Theme.paddingSmall)
                wrapMode: Text.WordWrap
                visible: imageCntField.text!="0"
                x: Theme.horizontalPageMargin
                font.pixelSize: Theme.fontSizeTiny
                color: Theme.secondaryColor
            }
        }

        Column {
            width: parent.width
            visible: newTimeLapsePage.state == "Exposure"

            ComboBox {
                id: isoComboBox
                width: parent.width
                visible: camera!=null && camera.isoOptions.length > 0

                property bool initialized: false

                label: qsTr("ISO")
                menu: ContextMenu {
                    Repeater {
                        width: parent.width
                        model: camera != null ? camera.isoOptions : []
                        delegate: MenuItem {
                            text: modelData
                        }
                    }
                }
                function update() {
                    isoComboBox.initialized=false;
                    isoComboBox.currentIndex=0;
                    if (camera!=null) {
                        for (var row=0; row < camera.isoOptions.length; row++){
                            if (camera.iso==camera.isoOptions[row]){
                                isoComboBox.currentIndex=row;
                            }
                        }
                    }
                    isoComboBox.initialized=true;
                }
                onCurrentItemChanged: {
                    if (!initialized){
                        console.log("NOT initialized");
                        return;
                    }
                    camera.iso=camera.isoOptions[currentIndex];
                }
            }
            ComboBox {
                id: apertureComboBox
                width: parent.width
                visible: camera!=null && camera.apertureOptions.length > 0

                property bool initialized: false

                label: qsTr("Aperture")
                menu: ContextMenu {
                    Repeater {
                        width: parent.width
                        model: camera != null ? camera.apertureOptions : []
                        delegate: MenuItem {
                            text: modelData
                        }
                    }
                }
                function update() {
                    apertureComboBox.initialized=false;
                    apertureComboBox.currentIndex=0;
                    if (camera!=null) {
                        for (var row=0; row < camera.apertureOptions.length; row++){
                            if (camera.aperture==camera.apertureOptions[row]){
                                apertureComboBox.currentIndex=row;
                            }
                        }
                    }
                    apertureComboBox.initialized=false;
                }
                onCurrentItemChanged: {
                    if (!initialized){
                        console.log("NOT initialized");
                        return;
                    }
                    camera.aperture=camera.apertureOptions[currentIndex];
                }
            }
            TextSwitch{
                id: adaptiveShutterSpeedSwitch
                width: parent.width
                visible: camera!=null && camera.shutterSpeedOptions.length > 0

                text: qsTr("Adaptive shutter-speed")
            }
            ComboBox {
                id: shutterSpeedComboBox
                width: parent.width
                visible: camera!=null && camera.shutterSpeedOptions.length > 0

                property bool initialized: false

                label: adaptiveShutterSpeedSwitch.checked ? qsTr("Min. shutter speed") : qsTr("Shutter speed")
                menu: ContextMenu {
                    Repeater {
                        width: parent.width
                        model: camera != null ? camera.shutterSpeedOptions : []
                        delegate: MenuItem {
                            text: modelData
                        }
                    }
                }
                function update() {
                    shutterSpeedComboBox.initialized=false;
                    shutterSpeedComboBox.currentIndex=0;
                    if (camera!=null) {
                        for (var row=0; row < camera.shutterSpeedOptions.length; row++){
                            if (camera.shutterSpeed==camera.shutterSpeedOptions[row]){
                                shutterSpeedComboBox.currentIndex=row;
                            }
                        }
                    }
                    shutterSpeedComboBox.initialized=true;
                }
            }
            ComboBox {
                id: maxShutterSpeedComboBox
                width: parent.width
                visible: camera!=null && camera.shutterSpeedOptions.length > 0 && adaptiveShutterSpeedSwitch.checked

                property bool initialized: false

                label: qsTr("Max. shutter speed")
                menu: ContextMenu {
                    Repeater {
                        width: parent.width
                        model: camera != null ? camera.shutterSpeedOptions : []
                        delegate: MenuItem {
                            text: modelData
                        }
                    }
                }
                function update() {
                    shutterSpeedComboBox.initialized=false;
                    shutterSpeedComboBox.currentIndex=0;
                    if (camera!=null) {
                        for (var row=0; row < camera.shutterSpeedOptions.length; row++){
                            if (camera.shutterSpeed==camera.shutterSpeedOptions[row]){
                                shutterSpeedComboBox.currentIndex=row;
                            }
                        }
                    }
                    shutterSpeedComboBox.initialized=true;
                }
            }
            Label {
                text: qsTr("Camera didn't support ISO, aperture and shutter speed setting.")
                width: parent.width - (Theme.horizontalPageMargin * Theme.paddingSmall)
                wrapMode: Text.WordWrap
                visible: !shutterSpeedComboBox.visible && !apertureComboBox.visible && !isoComboBox.visible
                x: Theme.horizontalPageMargin
                font.pixelSize: Theme.fontSizeTiny
                color: Theme.secondaryColor
            }
        }

        Column {
            width: parent.width
            visible: newTimeLapsePage.state == "Focus"

            ComboBox {
                id: focusModeComboBox
                width: parent.width
                visible: camera!=null && camera.focusModeOptions.length > 0

                property bool initialized: false

                label: qsTr("Focus mode")
                menu: ContextMenu {
                    Repeater {
                        width: parent.width
                        model: camera != null ? camera.focusModeOptions : []
                        delegate: MenuItem {
                            text: modelData
                        }
                    }
                }
                function update(){
                    focusModeComboBox.initialized=false;
                    focusModeComboBox.currentIndex=0;
                    if (camera!=null) {
                        for (var row=0; row < camera.focusModeOptions.length; row++){
                            console.log("camera.focusMode: " + camera.focusMode + " camera.focusModeOptions[" + row + "]: " + camera.focusModeOptions[row]);
                            if (camera.focusMode===camera.focusModeOptions[row]){
                                focusModeComboBox.currentIndex=row;
                                console.log("focusModeComboBox.currentIndex match : " + focusModeComboBox.currentIndex);
                            }
                        }
                    }
                    console.log("focusModeComboBox.currentIndex: " + focusModeComboBox.currentIndex);
                    focusModeComboBox.initialized=true;
                }

                onCurrentItemChanged: {
                    if (!initialized){
                        console.log("NOT initialized");
                        return;
                    }
                    camera.focusMode=camera.focusModeOptions[currentIndex];
                }
            }
            ComboBox {
                id: focusPointModeComboBox
                width: parent.width
                visible: camera!=null && camera.focusPointModeOptions.length > 0 &&
                         (camera.focusMode === "Auto" || camera.focusMode === "Continuous" || camera.focusMode === "Macro")

                property bool initialized: false

                label: qsTr("Focus point mode")
                menu: ContextMenu {
                    Repeater {
                        width: parent.width
                        model: camera != null ? camera.focusPointModeOptions : []
                        delegate: MenuItem {
                            text: modelData
                        }
                    }
                }
                function update() {
                    focusPointModeComboBox.initialized=false;
                    focusPointModeComboBox.currentIndex=0;
                    if (camera!=null) {
                        for (var row=0; row < camera.focusPointModeOptions.length; row++){
                            console.log("camera.focusPointMode: " + camera.focusPointMode + " camera.focusPointModeOptions[" + row + "]: " + camera.focusPointModeOptions[row]);
                            if (camera.focusPointMode==camera.focusPointModeOptions[row]){
                                focusPointModeComboBox.currentIndex=row;
                            }
                        }
                    }
                    focusPointModeComboBox.initialized=true;
                }
                onCurrentItemChanged: {
                    if (!initialized){
                        console.log("NOT initialized");
                        return;
                    }
                    camera.focusPointMode=camera.focusPointModeOptions[currentIndex];
                }
            }
            TextSwitch{
                id: persistentFocusLockSwitch
                width: parent.width
                visible: camera!=null && camera.focusLockSupport
                checked: camera!=null && camera.persistentFocusLock

                onCheckedChanged: {
                    if (camera!=null) {
                        camera.persistentFocusLock = checked;
                    }
                }

                text: qsTr("Persistent focus lock")
            }
            Label {
                text: qsTr("Camera didn't support focus setting.")
                width: parent.width - (Theme.horizontalPageMargin * Theme.paddingSmall)
                wrapMode: Text.WordWrap
                visible: !focusModeComboBox.visible && !focusPointModeComboBox.visible
                x: Theme.horizontalPageMargin
                font.pixelSize: Theme.fontSizeTiny
                color: Theme.secondaryColor
            }
        }
    }


    Timer {
        id: updateDelay
        interval: 10
        repeat: false
        function update() {
            focusPointModeComboBox.update();
            focusModeComboBox.update();
            maxShutterSpeedComboBox.update();
            shutterSpeedComboBox.update();
            apertureComboBox.update();
            isoComboBox.update();
        }

        onTriggered: {
            if (newTimeLapsePage.camera!=null) {
                update();
            }
        }
    }
    Connections {
        target: newTimeLapsePage
        onCameraChanged: {
            updateDelay.start();
        }
    }
    Connections {
        target: camera
        onUpdate: {
            updateDelay.start();
        }
    }

    onOrientationChanged: {
        console.log("Orientation: " + orientation);
        //viewFinder.updateOrientation();
    }

    Rectangle {
        id: captureCircle
        height: Theme.itemSizeMedium
        width: height
        radius: width / 2

        anchors{
            right: parent.right
            bottom: parent.bottom
            rightMargin: Theme.horizontalPageMargin
            bottomMargin: Theme.paddingMedium
        }

        border.width: 5
        border.color: "white"
        color: "red"
        visible: camera != null

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (nameField.text=="") {
                    remorse.execute(qsTr("Empty name!"), function() { }, 10 * 1000);
                    return;
                }

                pageStack.replace(Qt.resolvedUrl("Capture.qml"),
                                  {
                                      name: nameField.text,
                                      camera: newTimeLapsePage.camera,
                                      interval: parseInt(intervalField.text, 10),
                                      baseDir: destinationDirectoryComboBox.selected,
                                      count: parseInt(imageCntField.text, 10),
                                      adaptiveShutterSpeed: adaptiveShutterSpeedSwitch.checked,
                                      shutterSpeed: camera.shutterSpeedOptions[shutterSpeedComboBox.currentIndex],
                                      maxShutterSpeed: camera.shutterSpeedOptions[maxShutterSpeedComboBox.currentIndex]
                                  });
            }
        }
    }

}
