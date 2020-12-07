/***************************************************************************
 qgsquickicontextitem.qml
  --------------------------------------
  Date                 : 2019
  Copyright            : (C) 2019 by Viktor Sklencar
  Email                : viktor.sklencar@lutraconsulting.co.uk
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

import QtQuick 2.5
import QtGraphicalEffects 1.0
import QgsQuick 0.1 as QgsQuick

Item {
  property real iconSize
  property color fontColor
  property real fontPixelSize: root.iconSize * 0.75
  property string iconSource
  property string labelText

  id: root
  width: root.iconSize + text.width
  height: root.iconSize

  Image {
    id: icon
    source: root.iconSource
    width: root.iconSize
    height: root.iconSize
    sourceSize.width: width
    sourceSize.height: height
    fillMode: Image.PreserveAspectFit
  }

  ColorOverlay {
    anchors.fill: icon
    source: icon
    color: root.fontColor
  }

  Text {
    id: text
    height: root.iconSize
    text: root.labelText
    font.pixelSize: root.fontPixelSize
    color: root.fontColor
    anchors.leftMargin: root.iconSize + fieldItem.textMargin
    x: root.iconSize + fieldItem.textMargin
    horizontalAlignment: Text.AlignRight
    verticalAlignment: Text.AlignVCenter
  }
}
