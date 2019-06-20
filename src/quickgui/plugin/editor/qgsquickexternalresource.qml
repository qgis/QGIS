/***************************************************************************
 qgsquickexternalresource.qml
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

import QtQuick 2.5
import QtQuick.Controls 2.0
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.3
import QgsQuick 0.1 as QgsQuick

/**
 * External Resource (Photo capture) for QGIS Attribute Form
 * Requires various global properties set to function, see qgsquickfeatureform Loader section
 * Do not use directly from Application QML
 * The widget is interactive which allows interactions even in readOnly state (e.g showing preview), but no edit!
 */
Item {
  signal valueChanged(var value, bool isNull)

  property var image: image
  property var cameraIcon: customStyle.icons.camera
  property var deleteIcon: customStyle.icons.remove
  property var galleryIcon: customStyle.icons.gallery
  property var brokenImageIcon: customStyle.icons.brokenImage
  property var notAvailableImageIcon: customStyle.icons.notAvailable
  property real iconSize:  customStyle.fields.height
  property real textMargin: QgsQuick.Utils.dp * 10
  // Meant to be use with the save callback - stores image source
  property string sourceToDelete

  function callbackOnSave() {
    externalResourceHandler.onFormSave(fieldItem)
  }
  function callbackOnCancel() {
    externalResourceHandler.onFormCanceled(fieldItem)
  }

  id: fieldItem
  enabled: true // its interactive widget
  height: customStyle.fields.height * 3
  anchors {
    left: parent.left
    right: parent.right
    rightMargin: 10 * QgsQuick.Utils.dp
  }

  states: [
    State {
      name: "valid"
    },
    State {
      name: "notSet"
    },
    State {
      name: "notAvailable"
    }
  ]

  QgsQuick.PhotoCapture {
    id: photoCapturePanel
    visible: false
    height: window.height
    width: window.width
    edge: Qt.RightEdge
    imageButtonSize: fieldItem.iconSize
  }

  Rectangle {
    id: imageContainer
    width: parent.width
    height: parent.height
    color: customStyle.fields.backgroundColor
    radius: customStyle.fields.cornerRadius

    Image {
      property var currentValue: value

      id: image
      height: imageContainer.height
      sourceSize.height: imageContainer.height
      autoTransform: true
      fillMode: Image.PreserveAspectFit
      visible: fieldItem.state === "valid"
      anchors.verticalCenter: parent.verticalCenter
      anchors.horizontalCenter: parent.horizontalCenter

      MouseArea {
        anchors.fill: parent
        onClicked: externalResourceHandler.previewImage(homePath + "/" + image.currentValue)
      }

      onCurrentValueChanged: {
        image.source = image.getSource()
      }

      Component.onCompleted: image.source = getSource()

      function getSource() {
        if (image.status === Image.Error) {
          fieldItem.state = "notAvailable"
          return ""
        }
        else if (image.currentValue && QgsQuick.Utils.fileExists(homePath + "/" + image.currentValue)) {
          fieldItem.state = "valid"
          return homePath + "/" + image.currentValue
        }
        else if (!image.currentValue) {
          fieldItem.state = "notSet"
          return ""
        }
        fieldItem.state = "notAvailable"
        return homePath + "/" + image.currentValue
      }
    }
  }

  Button {
    id: deleteButton
    visible: !readOnly && fieldItem.state !== "notSet"
    width: buttonsContainer.itemHeight
    height: width
    padding: 0

    anchors.right: imageContainer.right
    anchors.bottom: imageContainer.bottom
    anchors.margins: buttonsContainer.itemHeight/4

    onClicked: externalResourceHandler.removeImage(fieldItem, homePath + "/" + image.currentValue)

    background: Image {
      id: deleteIcon
      source: fieldItem.deleteIcon
      width: deleteButton.width
      height: deleteButton.height
      sourceSize.width: width
      sourceSize.height: height
      fillMode: Image.PreserveAspectFit
    }

    ColorOverlay {
      anchors.fill: deleteIcon
      source: deleteIcon
      color: customStyle.fields.attentionColor
    }
  }

  Item {
    property real itemHeight: fieldItem.iconSize/2

    id: buttonsContainer
    anchors.centerIn: imageContainer
    anchors.fill: imageContainer
    anchors.margins: fieldItem.textMargin
    visible: fieldItem.state === "notSet"

    anchors.horizontalCenter: parent.horizontalCenter
    anchors.verticalCenter: parent.verticalCenter

    ColumnLayout {
      width: parent.width
      height: photoButton.height * 2
      anchors.horizontalCenter: parent.horizontalCenter
      anchors.verticalCenter: parent.verticalCenter

      QgsQuick.IconTextItem {
        id: photoButton
        iconSize: buttonsContainer.itemHeight
        fontColor: customStyle.fields.fontColor
        fontPixelSize: customStyle.fields.fontPixelSize
        iconSource: fieldItem.cameraIcon
        labelText: qsTr("Take a photo")

        visible: !readOnly && fieldItem.state !== " valid"
        height: buttonsContainer.itemHeight * 1.5
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

        MouseArea {
          anchors.fill: parent
          onClicked: {
            photoCapturePanel.visible = true
            photoCapturePanel.targetDir = homePath
            photoCapturePanel.fieldItem = fieldItem
          }
        }
      }

      QgsQuick.IconTextItem {
        id: browseButton
        iconSize: buttonsContainer.itemHeight
        fontColor: customStyle.fields.fontColor
        fontPixelSize: customStyle.fields.fontPixelSize
        iconSource: fieldItem.galleryIcon
        labelText: qsTr("Add from gallery")

        visible: !readOnly && fieldItem.state !== " valid"
        height: buttonsContainer.itemHeight * 1.5
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

        MouseArea {
          anchors.fill: parent
          onClicked: externalResourceHandler.chooseImage(fieldItem)
        }
      }
    }
  }

  Text {
    id: text
    height: parent.height
    width: imageContainer.width - 2* fieldItem.textMargin
    wrapMode: Text.WordWrap
    minimumPixelSize: 50 * QgsQuick.Utils.dp
    text: qsTr("Image is not available: ") + image.currentValue
    font.pixelSize: buttonsContainer.itemHeight * 0.75
    color: customStyle.fields.fontColor
    anchors.leftMargin: buttonsContainer.itemHeight + fieldItem.textMargin
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
    elide: Text.ElideRight
    visible: fieldItem.state === "notAvailable"
  }

}
