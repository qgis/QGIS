/***************************************************************************
 qgsquickcheckbox.qml
  --------------------------------------
  Date                 : 2017
  Copyright            : (C) 2017 by Matthias Kuhn
  Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

import QtQuick 2.6
import QtQuick.Controls 2.2
import QgsQuick 0.1 as QgsQuick

/**
 * Checkbox for QGIS Attribute Form
 * Requires various global properties set to function, see qgsquickfeatureform Loader section
 * Do not use directly from Application QML
 */
Item {
  signal valueChanged( var value, bool isNull )

  property var checkedState: getConfigValue(config['CheckedState'], true)
  property var uncheckedState: getConfigValue(config['UncheckedState'], false)
  property string booleanEnum: "1" // QMetaType::Bool Enum of Qvariant::Type

  id: fieldItem
  enabled: !readOnly
  height: childrenRect.height
  anchors {
    right: parent.right
    left: parent.left
    rightMargin: 10 * QgsQuick.Utils.dp
  }

  function getConfigValue(configValue, defaultValue) {
    if (!configValue && field.type + "" === fieldItem.booleanEnum) {
      return defaultValue
    } else return configValue
  }

  Rectangle {
    id: fieldContainer
    height: customStyle.fields.height
    color: customStyle.fields.backgroundColor
    radius: customStyle.fields.cornerRadius
    anchors { right: parent.right; left: parent.left }

    Text {
      text: checkBox.checked ? fieldItem.checkedState : fieldItem.uncheckedState
      font.pixelSize: customStyle.fields.fontPixelSize
      color: customStyle.fields.fontColor
      horizontalAlignment: Text.AlignLeft
      verticalAlignment: Text.AlignVCenter
      anchors.left: parent.left
      anchors.verticalCenter: parent.verticalCenter
      leftPadding: 6 * QgsQuick.Utils.dp
    }

    CheckBox {
      property var currentValue: value
      height: customStyle.fields.height/2
      width: height * 2
      anchors.right: parent.right
      anchors.rightMargin: fieldItem.anchors.rightMargin
      anchors.verticalCenter: parent.verticalCenter
      id: checkBox
      leftPadding: 0
      checked: value === fieldItem.checkedState

      indicator: Rectangle {
        implicitWidth: parent.width
        implicitHeight: parent.height
        x: checkBox.leftPadding
        y: parent.height / 2 - height / 2
        radius: parent.height/2
        color: checkBox.checked ? customStyle.fields.fontColor : "#ffffff"
        border.color: checkBox.checked ? customStyle.fields.fontColor : customStyle.fields.normalColor

        Rectangle {
          x: checkBox.checked ? parent.width - width : 0
          width: parent.height
          height: parent.height
          radius: parent.height/2
          color: "#ffffff"
          border.color: checkBox.checked ? customStyle.fields.fontColor : customStyle.fields.normalColor
        }
      }

      onCheckedChanged: {
        valueChanged( checked ? fieldItem.checkedState : fieldItem.uncheckedState, false )
        forceActiveFocus()
      }

      // Workaround to get a signal when the value has changed
      onCurrentValueChanged: {
        checked = currentValue === fieldItem.checkedState
      }
    }
  }
}
