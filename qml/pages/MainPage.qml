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

import harbour.timelapsetools 1.0

Page {
    id: mainPage

    SilicaListView {
        id: listView

        anchors{
            fill: parent
            //left: parent.left
            //right: parent.right
            //bottom: parent.bottom
            //top: newButtonColumn.bottom
        }

        clip: true
        spacing: Theme.paddingMedium

        header: Rectangle {
            id: newButtonColumn

            //anchors.margins: Theme.horizontalPageMargin
            //topPadding: Theme.paddingLarge
            //bottomPadding: Theme.paddingLarge
            //padding: Theme.horizontalPageMargin
            //rightPadding: Theme.horizontalPageMargin

            color: "transparent"
            width: parent.width
            height: newTimeLapseButton.height + 2*Theme.paddingLarge
            //spacing: Theme.paddingLarge

            Button {
                id: newTimeLapseButton
                y: Theme.paddingLarge
                anchors {
                    horizontalCenter: parent.horizontalCenter
                    //margins: Theme.paddingLarge
                }
                text: qsTr("New TimeLapse")
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("NewTimeLapsePage.qml"),{})
                }
            }
        }


        VerticalScrollDecorator {}
        TimeLapseModel {
            id: timeLapseModel
        }

        model: timeLapseModel
        delegate:  ListItem {
            id: timeLapseItem
            width: listView.width
            //height: timeLapseName.height
            Label {
                id: timeLapseName
                x: Theme.paddingMedium
                text: model.name
            }

            Label{
                id: dateLabel
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: Theme.paddingMedium
                text: Qt.formatDateTime(birthTime)
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.secondaryColor
                visible: Qt.formatDateTime(birthTime) != ""
            }

            onClicked: {
                pageStack.push(Qt.resolvedUrl("Assembly.qml"),
                               {
                                   name: model.name,
                                   path: model.path
                               })
            }

            menu: ContextMenu {
                MenuItem {
                    //: Context menu for downloading map
                    text: qsTr("Delete")
                    onClicked: {
                        Remorse.itemAction(timeLapseItem,
                                           //: label for remorse timer when canceling the download
                                           qsTr("Deleting"),
                                           function() {
                                               console.log("about delete timelapse on row " + model.index);
                                               timeLapseModel.deleteTimeLapse(model.index);
                                           });
                    }
                }
            }
        }
    }
}
