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

.pragma library

function humanDirectory(directory){
    return directory
        .replace(/^\/home\/[^/]*$/i, qsTr("Home"))
        .replace(/^\/home\/[^/]*\/Documents$/i, qsTr("Documents"))
        .replace(/^\/media\/sdcard\/[^/]*$/i, qsTr("SD card"))
        .replace(/^\/run\/media\/[^/]*\/[^/]*$/i, qsTr("SD card"))
        .replace(/^\/home\/[^/]*\//i, "[" + qsTr("Home") + "] ")
        .replace(/^\/media\/sdcard\/[^/]*\//i, "[" + qsTr("SD card") + "] ")
        .replace(/^\/run\/media\/[^/]*\/[^/]*\//i, "[" + qsTr("SD card") + "] ");
}

function humanDuration(seconds){
    var hours   = Math.floor(seconds / 3600);
    var minutes = Math.floor((seconds - (hours * 3600)) / 60);

    if (hours   < 10) {hours   = "0"+hours;}
    if (minutes < 10) {minutes = "0"+minutes;}
    return hours+':'+minutes;
}

function humanDurationLong(seconds){
    var hours   = Math.floor(seconds / 3600);
    var rest    = seconds - (hours * 3600);
    var minutes = Math.floor(rest / 60);
    var sec     = Math.floor(rest - (minutes * 60));

    if (hours   < 10) {hours   = "0"+hours;}
    if (minutes < 10) {minutes = "0"+minutes;}
    if (sec     < 10) {sec = "0"+sec;}
    return hours+':'+minutes+':'+sec;
}
