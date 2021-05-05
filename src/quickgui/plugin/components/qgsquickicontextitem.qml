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
import QtQuick.Layouts 1.3
import QgsQuick 0.1 as QgsQuick

Item {
  property real iconSize
  property color fontColor
  property real fontPointSize: root.iconSize * 0.75
  property string iconSource
  property string labelText

  id: root

  ColumnLayout {
    anchors.fill: parent
    spacing: 2 * QgsQuick.Utils.dp

    Item {
      id: iconContainer

      Layout.fillHeight: true
      Layout.preferredHeight: root.height / 2
      Layout.preferredWidth: root.width

      Image {
        id: icon

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom

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
    }

    Item {
      id: textContainer

      Layout.fillHeight: true
      Layout.preferredHeight: root.height / 2
      Layout.preferredWidth: root.width

      Text {
        id: text

        text: root.labelText
        font.pointSize: root.fontPointSize
        width: parent.width
        color: root.fontColor
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
        maximumLineCount: 3
      }
    }
  }
}
