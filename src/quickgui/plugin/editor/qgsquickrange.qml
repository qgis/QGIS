/***************************************************************************
 qgsquickrange.qml
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

import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QgsQuick 0.1 as QgsQuick

Item {
  signal valueChanged(var value, bool isNull)

  property var currentValue: value
  property real customMargin: 10 * QgsQuick.Utils.dp

  id: fieldItem
  enabled: !readOnly
  height: customStyle.fields.height

  anchors {
    left: parent.left
    right: parent.right
    rightMargin: fieldItem.customMargin
  }

  // background
  Rectangle {
    anchors.fill: parent
    border.color: customStyle.fields.normalColor
    border.width: 1 * QgsQuick.Utils.dp
    color: customStyle.fields.backgroundColor
    radius: customStyle.fields.cornerRadius
  }

  Row {
    id: rowLayout
    anchors.fill: parent
    anchors.rightMargin: fieldItem.customMargin
    anchors.leftMargin: fieldItem.customMargin

    Text {
      id: valueLabel
      width: rowLayout.width/3
      height: fieldItem.height
      elide: Text.ElideRight
      text: Number(control.value).toFixed(config["Precision"]).toLocaleString(Qt.locale())
      verticalAlignment: Text.AlignVCenter
      horizontalAlignment: Text.AlignLeft
      font.pixelSize: customStyle.fields.fontPixelSize
      color: customStyle.fields.fontColor
      topPadding: fieldItem.customMargin
      bottomPadding: fieldItem.customMargin
    }

    Slider {
      id: control
      value: fieldItem.currentValue
      width: parent.width - valueLabel.width
      height: fieldItem.height
      implicitWidth: width
      from: config["Min"]
      to: config["Max"]
      stepSize: config["Step"] ? config["Step"] : 1

      onValueChanged: {
        fieldItem.valueChanged(control.value, false)
      }

      background: Rectangle {
        x: control.leftPadding
        y: control.topPadding + control.availableHeight / 2 - height / 2
        implicitWidth: control.width
        implicitHeight: control.height * 0.1
        width: control.availableWidth
        height: implicitHeight
        radius: 2 * QgsQuick.Utils.dp
        color:  fieldItem.enabled ? customStyle.fields.fontColor : customStyle.fields.backgroundColorInactive
      }
    }
  }

}
