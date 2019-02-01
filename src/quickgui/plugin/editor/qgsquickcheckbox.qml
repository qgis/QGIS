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

  height: childrenRect.height
  anchors {
    right: parent.right
    left: parent.left
  }
  id: fieldItem

  CheckBox {
    property var currentValue: value
    height: customStyle.height
    id: checkBox
    leftPadding: 0
    checked: value == config['CheckedState']

    indicator: Rectangle {
                implicitWidth: customStyle.height
                implicitHeight: customStyle.height
                radius: customStyle.cornerRadius
                border.color: checkBox.activeFocus ? customStyle.fontColor : "grey"
                border.width: 1
                Rectangle {
                    visible: checkBox.checked
                    color: customStyle.fontColor
                    radius: customStyle.cornerRadius
                    anchors.margins: 4
                    anchors.fill: parent
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: checkBox.currentValue = !checkBox.currentValue
                }
        }
    onCheckedChanged: {
      valueChanged( checked ? config['CheckedState'] : config['UncheckedState'], false )
      forceActiveFocus()
    }

    // Workaround to get a signal when the value has changed
    onCurrentValueChanged: {
      checked = currentValue == config['CheckedState']
    }
  }
}
