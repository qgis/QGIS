/***************************************************************************
 qgsquickvaluerelation.qml
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

import QtQuick 2.7
import QtQuick.Controls 2.2
import QtGraphicalEffects 1.0
import QgsQuick 0.1 as QgsQuick

/**
 * Value Relation for QGIS Attribute Form
 * Requires various global properties set to function, see qgsquickfeatureform Loader section
 * Do not use directly from Application QML
 */
Item {
  signal valueChanged(var value, bool isNull)

  id: fieldItem
  enabled: !readOnly
  height: customStyle.fields.height
  anchors {
    left: parent.left
    right: parent.right
    rightMargin: 10 * QgsQuick.Utils.dp
  }

  QgsQuick.EditorWidgetComboBox {
    // Value relation cache map
    property var currentMap
    // Reversed to currentMap. It is used to find key (currentValue) according value (currentText)
    property var reversedMap: ({})
    property var currentValue: value

    comboStyle: customStyle.fields
    textRole: 'text'
    height: parent.height
    model: ListModel {
      id: listModel
    }

    Component.onCompleted: {
      currentMap = QgsQuick.Utils.createValueRelationCache(config)
      var valueInKeys = false
      var keys = Object.keys(currentMap)
      for(var i=0; i< keys.length; i++)
      {
        var currentKey = keys[i]
        if (value == currentKey) valueInKeys = true
        var valueText = currentMap[currentKey]
        listModel.append( { text: valueText } )
        reversedMap[valueText] = currentKey;
      }
      model = listModel
      currentIndex = valueInKeys ? find(currentMap[value]) : -1
    }

    onCurrentTextChanged: {
      valueChanged(reversedMap[currentText], false)
    }

    // Workaround to get a signal when the value has changed
    onCurrentValueChanged: {
      currentIndex = currentMap ? find(currentMap[value]) : -1
    }

  }
}
