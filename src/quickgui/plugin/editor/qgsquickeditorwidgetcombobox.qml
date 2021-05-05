/***************************************************************************
 qgsquickcombobox.qml
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
 * ComboBox for QGIS Attribute Form - used by QgsQuickValueMap and QgsQuickValueRelation
 * Do not use directly from Application QML
 */
ComboBox {
  id: comboBox

  property var comboStyle
  property bool readOnly: false
  property real iconSize
  signal itemClicked( var index )

  anchors { left: parent.left; right: parent.right }

  MouseArea {
    anchors.fill: parent
    propagateComposedEvents: true

    onClicked: mouse.accepted = false
    onPressed: { forceActiveFocus(); mouse.accepted = false; }
    onReleased: mouse.accepted = false;
    onDoubleClicked: mouse.accepted = false;
    onPositionChanged: mouse.accepted = false;
    onPressAndHold: mouse.accepted = false;
  }

  // [hidpi fixes]
  delegate: ItemDelegate {
    width: comboBox.width
    height: comboBox.height * 0.8
    text: model.display
    font.weight: comboBox.currentIndex === index ? Font.DemiBold : Font.Normal
    font.pointSize: customStyle.fields.fontPointSize
    highlighted: comboBox.highlightedIndex === index
    leftPadding: customStyle.fields.sideMargin
    onClicked: comboBox.itemClicked( model.FeatureId ? model.FeatureId : index )
  }

  contentItem: Text {
    height: comboBox.height * 0.8
    text: comboBox.displayText
    font.pointSize: customStyle.fields.fontPointSize
    horizontalAlignment: Text.AlignLeft
    verticalAlignment: Text.AlignVCenter
    elide: Text.ElideRight
    leftPadding: customStyle.fields.sideMargin
    color: comboStyle.fontColor
  }

  background: Item {
    implicitWidth: 120 * QgsQuick.Utils.dp
    implicitHeight: comboBox.height * 0.8

    Rectangle {
      anchors.fill: parent
      id: backgroundRect
      border.color: comboBox.pressed ? comboStyle.activeColor : comboStyle.normalColor
      border.width: comboBox.visualFocus ? 2 : 1
      color: comboStyle.backgroundColor
      radius: comboStyle.cornerRadius
    }
  }
  // [/hidpi fixes]

  indicator: Item {
    anchors.right: parent.right
    anchors.verticalCenter: parent.verticalCenter

    Image {
      id: comboboxIndicatorIcon
      source: customStyle.icons.combobox
      height: iconSize ? iconSize : parent.height * 0.4
      width: height / 2
      anchors.right: parent.right
      anchors.rightMargin: customStyle.fields.sideMargin
      anchors.verticalCenter: parent.verticalCenter
      fillMode: Image.PreserveAspectFit
      autoTransform: true
      visible: false
    }

    ColorOverlay {
      anchors.fill: comboboxIndicatorIcon
      source: comboboxIndicatorIcon
      color: readOnly ? customStyle.toolbutton.backgroundColorInvalid : customStyle.toolbutton.activeButtonColor
    }
  }
}
