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

import harbour.timelapsetools 1.0

import "../custom"
import "../custom/Utils.js" as Utils

Page {
    id: assemblyPage
    property string name
    property string path

    RemorsePopup { id: remorse }

    TimeLapseAssembly {
        id: assembly
        source: assemblyPage.path
        dir: destinationDirectoryComboBox.selected
        name: nameField.text
        deflicker: deflickerComboBox.selected
        profile: profileComboBox.selected
        noStrictInterval: noStrictIntervalSwitch.checked
        blendFrames: blendSwitch.checked
        stabilize: stabilizeSwitch.checked
        onError: {
            remorse.execute(qsTranslate("message", message), function() { }, 10 * 1000);
            processDialog.rejectRequested = true;
            processDialog.reject();
        }
    }

    function pageStatus(status){
        // https://sailfishos.org/develop/docs/silica/qml-sailfishsilica-sailfish-silica-page.html/
        if (status == PageStatus.Inactive){
            return "Inactive";
        }
        if (status == PageStatus.Activating){
            return "Activating";
        }
        if (status == PageStatus.Active){
            return "Active";
        }
        if (status == PageStatus.Deactivating){
            return "Deactivating";
        }

        return "Unknown";
    }

    Dialog{
        id: processDialog
        acceptDestinationAction: PageStackAction.Pop
        property bool rejectRequested: false;

        onStatusChanged: {
            console.log("Process dialog status: " + pageStatus(processDialog.status));
            if (processDialog.status == PageStatus.Active && rejectRequested) {
                processDialog.reject();
            }
        }
        onAccepted: {
            console.log("accepted");
        }
        onRejected: {
            console.log("rejected");
            assembly.cancel();
        }
        canAccept: !assembly.processing

        DialogHeader {
            id: processDialogHeader
            title: qsTr("Assembling")
            acceptText : qsTr("Done")
            cancelText : qsTr("Cancel")
        }

        BusyIndicator {
            id: busyIndicator
            running: assembly.processing
            size: BusyIndicatorSize.Large
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: processDialogHeader.bottom
        }

        Column {
            id: details
            anchors.top: busyIndicator.bottom
            width: parent.width - 2*Theme.paddingMedium

            DetailItem {
                label: qsTr("Source")
                value: Utils.humanDirectory(assembly.source)
            }
            DetailItem {
                label: qsTr("Output dir")
                value: Utils.humanDirectory(assembly.dir)
            }
            DetailItem {
                label: qsTr("Video name")
                value: assembly.name
            }
            DetailItem {
                label: qsTr("Input images")
                value: assembly.inputImageCount
            }
            DetailItem {
                label: qsTr("Progress")
                value: assembly.progressMessage
            }
        }
    }

    SilicaFlickable {
        id: flickable
        anchors.fill: parent
        contentHeight: content.height + 2*Theme.paddingLarge

        VerticalScrollDecorator {}

        Column {
            id: content
            width: parent.width

            PageHeader { title: qsTr("Assembly time lapse %1").arg(assemblyPage.name) }

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
                    var dirs=assembly.videoDirectories;
                    selected = dirs[currentIndex];
                    console.log("changed, currentIndex=" + destinationDirectoryComboBox.currentIndex + " selected: " + selected);
                }
                Component.onCompleted: {
                    var dirs=assembly.videoDirectories;
                    for (var i in dirs){
                        var dir = dirs[i];
                        if (selected==""){
                            selected=dir;
                        }
                        console.log("Dir: "+dir);
                        directories.append({"dir": dir});
                    }
                    initialized = true;
                    console.log("destinationDirectoryComboBox initialized, currentIndex=" + destinationDirectoryComboBox.currentIndex);
                }
            }
            TextField {
                id: nameField
                label: qsTr("Name")
                text: assemblyPage.name
            }
            ComboBox {
                id: profileComboBox

                property bool initialized: false
                property var selected: TimeLapseAssembly.HDx264_High
                property ListModel profiles: ListModel {}
                property var profileValues: []

                label: qsTr("Profile")
                menu: ContextMenu {
                    id: profileContextMenu
                    Repeater {
                        model: profileComboBox.profiles
                        MenuItem {
                            text: Utils.humanDirectory(name)
                        }
                    }
                }

                onCurrentItemChanged: {
                    if (!initialized){
                        console.log("NOT initialized");
                        return;
                    }
                    console.log("profileValues[" + currentIndex + "]: " + profileValues[currentIndex])
                    selected = profileValues[currentIndex];
                    console.log("changed, currentIndex=" + profileComboBox.currentIndex + " selected: " + selected);
                }
                Component.onCompleted: {
                    profiles.append({"name": "HD h264 @ 20 Mbps"});
                    profiles.append({"name": "HD h264 @ 40 Mbps"});
                    profiles.append({"name": "HD h265 @ 40 Mbps"});
                    profiles.append({"name": "4k h265 @ 60 Mbps"});
                    profileValues.push(TimeLapseAssembly.HDx264_Low);
                    profileValues.push(TimeLapseAssembly.HDx264_High);
                    profileValues.push(TimeLapseAssembly.HDx265);
                    profileValues.push(TimeLapseAssembly.UHDx265);
                    profileComboBox.currentIndex = 1;
                    initialized = true;
                    console.log("profileComboBox initialized, currentIndex=" + profileComboBox.currentIndex);
                }
            }
            TextField {
                id: fpsField
                label: qsTr("FPS")
                text: "0"
                validator: RegExpValidator { regExp: /^[0-9]{1,}$/ }
                inputMethodHints: Qt.ImhDigitsOnly
                property bool initialized: false
                Component.onCompleted: {
                    fpsField.text = assembly.fps;
                    initialized = true;
                }
                onTextChanged: {
                    if (initialized) {
                        assembly.fps = parseInt(fpsField.text, 10)
                    }
                }
            }
            TextSwitch{
                id: automaticLengthSwitch
                width: parent.width

                text: qsTr("Automatic video lenght")
                checked: true

                onCheckedChanged: {
                    if (checked) {
                        lengthField.text = "-1";
                    } else {
                        lengthField.text = Math.round(assembly.inputImageCount / assembly.fps);
                    }
                }
            }
            Label {
                text: qsTr("Time lapse has %1 pictures, video length: %2")
                    .arg(assembly.inputImageCount).arg(Utils.humanDurationLong(assembly.inputImageCount / assembly.fps))
                width: parent.width - (Theme.horizontalPageMargin * Theme.paddingSmall)
                wrapMode: Text.WordWrap
                visible: automaticLengthSwitch.checked
                x: Theme.horizontalPageMargin
                font.pixelSize: Theme.fontSizeTiny
                color: Theme.secondaryColor
            }
            TextField {
                id: lengthField
                label: qsTr("Duration [seconds]")
                text: "-1"
                visible: !automaticLengthSwitch.checked
                validator: RegExpValidator { regExp: /^[0-9]{1,}$/ }
                inputMethodHints: Qt.ImhDigitsOnly
                property bool initialized: false
                Component.onCompleted: {
                    initialized = true;
                }
                onTextChanged: {
                    if (initialized) {
                        assembly.length = parseInt(lengthField.text, 10)
                    }
                }
            }
            TextSwitch{
                id: noStrictIntervalSwitch
                width: parent.width

                text: qsTr("Dynamic frame interval")
                description: qsTr("Intervals between pictures in video are determited from real capture time.")
                visible: !automaticLengthSwitch.checked
                checked: false
            }
            TextSwitch{
                id: blendSwitch
                width: parent.width

                text: qsTr("Blend frames")
                visible: !automaticLengthSwitch.checked
                checked: false
            }
            ComboBox {
                id: deflickerComboBox

                property bool initialized: false
                property var selected: TimeLapseAssembly.NoDeflicker
                property ListModel profiles: ListModel {}
                property var profileValues: []

                label: qsTr("Deflickering")
                menu: ContextMenu {
                    id: deflickerContextMenu
                    Repeater {
                        model: deflickerComboBox.profiles
                        MenuItem {
                            text: Utils.humanDirectory(name)
                        }
                    }
                }

                onCurrentItemChanged: {
                    if (!initialized){
                        console.log("NOT initialized");
                        return;
                    }
                    console.log("profileValues[" + currentIndex + "]: " + profileValues[currentIndex])
                    selected = profileValues[currentIndex];
                    console.log("changed, currentIndex=" + deflickerComboBox.currentIndex + " selected: " + selected);
                }
                Component.onCompleted: {
                    profiles.append({"name": qsTr("Disable")});
                    profiles.append({"name": qsTr("Average all images")});
                    profiles.append({"name": qsTr("Moving average")});
                    profileValues.push(TimeLapseAssembly.NoDeflicker);
                    profileValues.push(TimeLapseAssembly.Average);
                    profileValues.push(TimeLapseAssembly.MovingAverage);
                    initialized = true;
                    console.log("deflickerComboBox initialized, currentIndex=" + deflickerComboBox.currentIndex);
                }
            }
            TextField {
                id: wmaCountField
                label: qsTr("Deflicker window size")
                text: "10"
                validator: RegExpValidator { regExp: /^[0-9]{1,}$/ }
                inputMethodHints: Qt.ImhDigitsOnly
                visible: deflickerComboBox.selected == TimeLapseAssembly.MovingAverage
                property bool initialized: false
                Component.onCompleted: {
                    fpsField.text = assembly.deflickerWmaCount;
                    initialized = true;
                }
                onTextChanged: {
                    if (initialized) {
                        assembly.deflickerWmaCount = parseInt(fpsField.text, 10)
                    }
                }
            }
            TextSwitch{
                id: stabilizeSwitch
                width: parent.width

                text: qsTr("Stabilize")
                description: qsTr("Stabilize video movements by ffmpeg vid.stab.")
                checked: false
            }
            Button {
                id: startButton
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
                text: qsTr("Assembly")
                onClicked: {
                    if (!assembly.processing) {
                        assembly.start()
                        processDialog.rejectRequested = false;
                        processDialog.open();
                    }
                }
            }
        }
    }
}
