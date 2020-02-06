/***************************************************************************
 qgsquickfeatureformstyling.qml
  --------------------------------------
  Date                 : January 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

import QtQuick 2.0

import QgsQuick 0.1 as QgsQuick

QtObject {
  property color backgroundColor: "white"
  property real backgroundOpacity: 1

  property QtObject group: QtObject {
    property color backgroundColor: "lightGray"
    property color marginColor: "black"
    property real leftMargin: 1 * QgsQuick.Utils.dp
    property real rightMargin: 1 * QgsQuick.Utils.dp
    property real topMargin: 1 * QgsQuick.Utils.dp
    property real bottomMargin: 1 * QgsQuick.Utils.dp
    property real height: 64 * QgsQuick.Utils.dp
    property color fontColor: "black"
    property int spacing: 10 * QgsQuick.Utils.dp
    property int fontPixelSize: 24 * QgsQuick.Utils.dp
  }

  property QtObject tabs: QtObject {
    property color normalColor: "#4CAF50"
    property color activeColor: "#1B5E20"
    property color disabledColor: "#FFFFFF"
    property color backgroundColor: "#999999"
    property color normalBackgroundColor: "#FFFFFF"
    property color activeBackgroundColor: "#4CAF50"
    property color disabledBackgroundColor: "#999999"
    property real height: 48 * QgsQuick.Utils.dp
    property real buttonHeight: height * 0.8
    property real spacing: 0
  }

  property QtObject constraint: QtObject {
    property color validColor: "black"
    property color invalidColor: "#c0392b"
    property color descriptionColor: "#e67e22"
  }

  property QtObject toolbutton: QtObject {
    property color backgroundColor: "transparent"
    property color backgroundColorInvalid: "#bdc3c7"
    property real size: 80 * QgsQuick.Utils.dp
  }

    property QtObject fields: QtObject {
      property color fontColor: "black"
      property color backgroundColor: "lightGray"
      property color backgroundColorInactive: "lightGray"
      property color activeColor: "#1B5E20"
      property color attentionColor: "red"
      property color normalColor: "#4CAF50"
      property real height: 48 * QgsQuick.Utils.dp
      property real cornerRadius: 0 * QgsQuick.Utils.dp
      property int fontPixelSize: 48 * QgsQuick.Utils.dp

    }

  property QtObject icons: QtObject {
    property var camera: QgsQuick.Utils.getThemeIcon("ic_camera")
    property var remove: QgsQuick.Utils.getThemeIcon("ic_delete_forever_white")
    property var gallery: QgsQuick.Utils.getThemeIcon("ic_gallery")
    property var brokenImage: QgsQuick.Utils.getThemeIcon("ic_broken_image_black")
    property var notAvailable: QgsQuick.Utils.getThemeIcon("ic_photo_notavailable_white")
    property var today: QgsQuick.Utils.getThemeIcon("ic_today")
    property var back: QgsQuick.Utils.getThemeIcon("ic_back")
  }

}
