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
    property real height: 30 * QgsQuick.Utils.dp
  }

  property QtObject tabs: QtObject {
    property color normalColor: "#4CAF50"
    property color activeColor: "#1B5E20"
    property color disabledColor: "#999999"
    property real height: 48 * QgsQuick.Utils.dp
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
}
