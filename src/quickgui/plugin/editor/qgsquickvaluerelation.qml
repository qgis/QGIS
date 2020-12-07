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
  signal valueChanged( var value, bool isNull )

  property var fieldName: field.name
  property bool allowMultipleValues: config['AllowMulti']
  property var widgetValue: value
  property var currentFeatureLayerPair: featurePair
  property string widgetType: ""
  property bool isReadOnly: readOnly

  property var model: QgsQuick.FeaturesListModel {
    id: vrModel
  }

  id: fieldItem

  enabled: !readOnly
  height: customStyle.fields.height
  anchors {
    left: parent.left
    right: parent.right
    rightMargin: 10 * QgsQuick.Utils.dp
  }

  onCurrentFeatureLayerPairChanged: {
    vrModel.setupValueRelation( config )
    vrModel.currentFeature = currentFeatureLayerPair.feature
    if ( widgetType === "" )
      widgetType = customWidget.getTypeOfWidget( fieldItem, vrModel )
    updateField()
  }

  /**
    * setValue sets value for value relation field
    * function accepts feature id as either a single value or an array of values
    * if array is passed, ids are converted to Key model column and used as multivalues
    */
  function setValue( featureIds, isNull = false ) {

    if ( Array.isArray(featureIds) && allowMultipleValues )
    {
      vrModel.searchExpression = "" // to be sure search is empty
      // construct JSON-like value list of key column
      // { val1, val2, val3, ... }

      let keys = featureIds.map( id => vrModel.attributeFromValue( QgsQuick.FeaturesListModel.FeatureId, id, QgsQuick.FeaturesListModel.KeyColumn ) )
      let valueList = '{' + keys.join(',') + '}'

      valueChanged(valueList, isNull)
    }

    else {
      valueChanged(
            vrModel.attributeFromValue(
              QgsQuick.FeaturesListModel.FeatureId,
              featureIds,
              QgsQuick.FeaturesListModel.KeyColumn
              ),
            isNull)
    }
  }

  // Called when data in different fields are changed.
  function dataUpdated( feature ) {
    if ( !isReadOnly ) {
      vrModel.currentFeature = feature
      updateField()
      combobox.popup.close()
    }
  }

  /**
    * in order to get values as
    */
  function getCurrentValueAsFeatureId() {
    if ( allowMultipleValues && widgetValue != null && widgetValue.startsWith('{') )
    {
      let arr = vrModel.convertMultivalueFormat( widgetValue, QgsQuick.FeaturesListModel.FeatureId )
      return Object.values(arr)
    }

    return undefined
  }

  /**
    * updateField function updates visible value of current field
    * if value to be set is undefined, -1, empty string or similar, it also resets current value
    */
  function updateField() {
    if ( widgetValue == null || widgetValue === ""  || widgetValue === -1 ) {
      textField.clear()
      combobox.currentIndex = -1
      return
    }
    let reset = false

    if ( widgetType === "textfield" ) {
      if ( allowMultipleValues && widgetValue.startsWith('{') )
      {
        let strings = vrModel.convertMultivalueFormat( widgetValue )
        textField.text = strings.join(", ")
        if ( !strings || strings.length === 0 )
          reset = true
      }
      else {
        let text = vrModel.attributeFromValue( QgsQuick.FeaturesListModel.KeyColumn, widgetValue, QgsQuick.FeaturesListModel.FeatureTitle )
        textField.text = text || ""
        if ( !text )
          reset = true
      }
    }
    else if ( widgetType === "combobox" ) {
      let index = vrModel.rowFromAttribute( QgsQuick.FeaturesListModel.KeyColumn, widgetValue )
      combobox.currentIndex = index
      if ( index < 0 )
        reset = true
    }

    if ( reset && !isReadOnly )
      setValue( -1, true )
  }

  /**
    * onWidgetValueChanged signal updates value of either custom valueRelation widget or combobox widget
    */
  onWidgetValueChanged: updateField()

  onWidgetTypeChanged: {
    if ( widgetType === "combobox" ) {
      textField.visible = false
      combobox.visible = true
    }
    else if ( widgetType === "textfield" ) {
      textField.visible = true
      combobox.visible = false
    }
  }

  Item {
    id: textFieldContainer
    anchors.fill: parent

    TextField {
      id: textField
      anchors.fill: parent
      readOnly: true
      font.pixelSize: customStyle.fields.fontPixelSize
      color: customStyle.fields.fontColor
      topPadding: 10 * QgsQuick.Utils.dp
      bottomPadding: 10 * QgsQuick.Utils.dp

      MouseArea {
        anchors.fill: parent
        propagateComposedEvents: false
        onClicked: {
          customWidget.valueRelationOpened( fieldItem, vrModel )
          mouse.accepted = true
        }
      }

      Image {
        id: rightArrow
        source: QgsQuick.Utils.getThemeIcon("ic_angle_right")
        width: 15
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        height: 30
        smooth: true
        visible: false
      }
      ColorOverlay {
        anchors.fill: rightArrow
        source: rightArrow
        color: isReadOnly ? customStyle.toolbutton.backgroundColorInvalid : customStyle.toolbutton.activeButtonColor
      }

      background: Rectangle {
        anchors.fill: parent
        border.color: textField.activeFocus ? customStyle.fields.activeColor : customStyle.fields.normalColor
        border.width: textField.activeFocus ? 2 : 1
        color: customStyle.fields.backgroundColor
        radius: customStyle.fields.cornerRadius
      }
    }
  }

  QgsQuick.EditorWidgetComboBox {
    id: combobox

    comboStyle: customStyle.fields
    textRole: 'FeatureTitle'
    height: parent.height
    model: vrModel

    Component.onCompleted: {
      currentIndex = vrModel.rowFromAttribute( QgsQuick.FeaturesListModel.KeyColumn, value )
    }

    /**
     * Called when user makes selection in the combo box.
     * No need to set currentIndex manually since it is done in onWidgetValueChanged update function
     */
    onItemClicked: {
      valueChanged( vrModel.attributeFromValue( QgsQuick.FeaturesListModel.FeatureId, index, QgsQuick.FeaturesListModel.KeyColumn ), false )
    }
  }
}
