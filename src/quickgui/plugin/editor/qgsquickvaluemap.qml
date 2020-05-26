/***************************************************************************
 qgsquickvaluemap.qml
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
import QtGraphicalEffects 1.0
import QgsQuick 0.1 as QgsQuick

/**
 * Value Map for QGIS Attribute Form
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
    // Reversed to model's key-value map. It is used to find index according current value
    property var reverseConfig: ({})
    property var currentValue: value

    comboStyle: customStyle.fields
    textRole: 'display'
    height: parent.height
    model: ListModel {
      id: listModel
    }

    Component.onCompleted: {
      if( config['map'] )
      {
        if( config['map'].length )
        {
          //it's a list (>=QGIS3.0)
          for(var i=0; i<config['map'].length; i++)
          {
            var currentMap = config['map'][i]
            var currentKey = Object.keys(currentMap)[0]
            listModel.append( { display: currentKey } )
            reverseConfig[currentMap[currentKey]] = currentKey;
          }
        }
        else
        {
          //it's a map (<=QGIS2.18)
          var currentMap = config['map'].length ? config['map'][currentIndex] : config['map']
          var currentKey = Object.keys(currentMap)[0]
          for(var key in config['map']) {
            listModel.append( { display: key } )
            reverseConfig[config['map'][key]] = key;
          }
        }
      }
      currentIndex = find(reverseConfig[value])
    }

    onCurrentTextChanged: {
      var currentMap = config['map'].length ? config['map'][currentIndex] : config['map']
      valueChanged(currentMap[currentText], false)
    }

    // Workaround to get a signal when the value has changed
    onCurrentValueChanged: {
      currentIndex = find(reverseConfig[value])
    }

  }
}
