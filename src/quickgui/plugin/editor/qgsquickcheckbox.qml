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

import QtQuick 2.7
import QtQuick.Controls 2.2
import QgsQuick 0.1 as QgsQuick

/**
 * Checkbox for QGIS Attribute Form
 * Requires various global properties set to function, see qgsquickfeatureform Loader section
 * Do not use directly from Application QML
 */
Item {
  id: fieldItem

  property var checkedState: getConfigValue(config['CheckedState'], true)
  property var uncheckedState: getConfigValue(config['UncheckedState'], false)
  property string booleanEnum: "1" // QMetaType::Bool Enum of Qvariant::Type
  property bool isReadOnly: readOnly

  signal valueChanged( var value, bool isNull )

  function getConfigValue(configValue, defaultValue) {
    if (!configValue && field.type + "" === fieldItem.booleanEnum) {
      return defaultValue
    } else return configValue
  }

  enabled: !readOnly
  height: childrenRect.height
  anchors {
    right: parent.right
    left: parent.left
  }

  Rectangle {
    id: fieldContainer
    height: customStyle.fields.height
    color: customStyle.fields.backgroundColor
    radius: customStyle.fields.cornerRadius
    anchors { right: parent.right; left: parent.left }

    MouseArea {
      anchors.fill: parent
      onClicked: switchComp.toggle()
    }

    Text {
      text: switchComp.checked ? fieldItem.checkedState : fieldItem.uncheckedState
      font.pointSize: customStyle.fields.fontPointSize
      color: customStyle.fields.fontColor
      horizontalAlignment: Text.AlignLeft
      verticalAlignment: Text.AlignVCenter
      anchors.left: parent.left
      anchors.verticalCenter: parent.verticalCenter
      leftPadding: customStyle.fields.sideMargin
    }

    QgsQuick.SwitchComponent {
      id: switchComp

      property var currentValue: value

      isReadOnly: fieldItem.isReadOnly
      bgndColorActive: customStyle.toolbutton.activeButtonColor
      bgndColorInactive: customStyle.toolbutton.backgroundColorInvalid

      anchors.right: parent.right
      anchors.verticalCenter: parent.verticalCenter
      anchors.rightMargin: customStyle.fields.sideMargin

      implicitHeight: fieldContainer.height * 0.6

      checked: value === fieldItem.checkedState

      onSwitchChecked: {
        valueChanged( isChecked ? fieldItem.checkedState : fieldItem.uncheckedState, false )
      }

      // Workaround to get a signal when the value has changed
      onCurrentValueChanged: {
        switchComp.checked = currentValue === fieldItem.checkedState
      }
    }
  }
}
