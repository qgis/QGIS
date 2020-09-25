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
  property var fieldName: field.name

  function itemSelected( index ) {
    combobox.itemClicked( index )
  }

  function openCombobox() {
    combobox.popup.open()
  }

  id: fieldItem
  enabled: !readOnly
  height: customStyle.fields.height
  anchors {
    left: parent.left
    right: parent.right
    rightMargin: 10 * QgsQuick.Utils.dp
  }

  QgsQuick.EditorWidgetComboBox {
    id: combobox
    property var currentEditorValue: value

    comboStyle: customStyle.fields
    textRole: 'FeatureTitle'
    height: parent.height

    model: QgsQuick.FeaturesListModel {
      id: vrModel

      // recalculate index when model changes
      onModelReset: {
        combobox.currentIndex = vrModel.rowFromAttribute( QgsQuick.FeaturesListModel.KeyColumn, value )
      }
    }

    Component.onCompleted: {
        vrModel.setupValueRelation( config )
        currentIndex = vrModel.rowFromAttribute( QgsQuick.FeaturesListModel.KeyColumn, value )
    }

    onPressedChanged: {
      if( pressed )
      {
        customWidget.valueRelationOpened( fieldItem, vrModel )
        pressed = false // we close combobox and let custom handler react, it can open combobox via openCombobox()
      }
    }

    // Called when user makes selection in the combo box
    onItemClicked: {
        currentIndex = vrModel.rowFromAttribute( QgsQuick.FeaturesListModel.FeatureId, index )
        valueChanged( vrModel.keyFromAttribute( QgsQuick.FeaturesListModel.FeatureId, index ), false )
    }

    // Called when the same form is used for a different feature
    onCurrentEditorValueChanged: {
        currentIndex = vrModel.rowFromAttribute( QgsQuick.FeaturesListModel.KeyColumn, value );
    }
  }
}
