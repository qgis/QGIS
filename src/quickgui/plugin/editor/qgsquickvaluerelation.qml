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

    property var currentValue: value

    comboStyle: customStyle.fields
    textRole: 'display'
    height: parent.height

    model: QgsQuick.ValueRelationListModel {
        id: vrModel
    }

    Component.onCompleted: {
        vrModel.populate(config)
        currentIndex = vrModel.rowForKey(value);
    }

    // Called when user makes selection in the combo box
    onCurrentIndexChanged: {
      valueChanged(vrModel.keyForRow(currentIndex), false)
    }

    // Called when the same form is used for a different feature
    onCurrentValueChanged: {
        currentIndex = vrModel.rowForKey(value);
    }

  }
}
