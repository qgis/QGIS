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

import QtQuick 2.0
import QtQuick.Controls 2.2
import QgsQuick 0.1 as QgsQuick

/**
 * Checkbox for QGIS Attribute Form
 * Requires various global properties set to function, see qgsquickfeatureform Loader section
 * Do not use directly from Application QML
 */
Item {
  signal valueChanged( var value, bool isNull )

  /**
   * Handling missing config value for un/checked state for boolean field
   */
  property var checkedState: getConfigValue(config['CheckedState'], true)
  property var uncheckedState: getConfigValue(config['UncheckedState'], false)

  id: fieldItem
  enabled: !readOnly
  height: childrenRect.height
  anchors {
    right: parent.right
    left: parent.left
  }

  function getConfigValue(configValue, defaultValue) {
     if (typeof value === "boolean" && !configValue) {
         return defaultValue
     } else return configValue
  }

  CheckBox {
    property var currentValue: value
    height: customStyle.fields.height
    id: checkBox
    leftPadding: 0
    checked: value === fieldItem.checkedState

    indicator: Rectangle {
                implicitWidth: customStyle.fields.height
                implicitHeight: customStyle.fields.height
                radius: customStyle.fields.cornerRadius
                border.color: checkBox.activeFocus ? customStyle.fields.fontColor : "grey"
                border.width: 1
                Rectangle {
                    visible: checkBox.checked
                    color: customStyle.fields.fontColor
                    radius: customStyle.fields.cornerRadius
                    anchors.margins: 4
                    anchors.fill: parent
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: checkBox.checked = !checkBox.checked
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
