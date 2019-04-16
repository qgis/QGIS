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
  property real iconSize:  customStyle.fields.height * 0.75
  property real textMargin: QgsQuick.Utils.dp * 10

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
      name: "broken"
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
          fieldItem.state = "broken"
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
        else {
          fieldItem.state = "notAvailable"
          return homePath + "/" + image.currentValue
        }
      }
    }
  }

  Button {
    id: deleteButton
    visible: !readOnly && fieldItem.state === "valid"
    width: fieldItem.iconSize
    height: width
    padding: 0

    anchors.right: imageContainer.right
    anchors.bottom: imageContainer.bottom

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
    id: buttonsContainer
    anchors.centerIn: imageContainer
    anchors.fill: imageContainer
    anchors.margins: 10
    visible: fieldItem.state !== "valid"

    QgsQuick.IconTextItem {
      id: photoButton
      iconSize: fieldItem.iconSize
      fontColor: customStyle.fields.fontColor
      fontPixelSize: fieldItem.iconSize/2
      iconSource: fieldItem.cameraIcon
      labelText: qsTr("Take a Photo")

      visible: !readOnly && fieldItem.state !== " valid"
      height: fieldItem.iconSize
      anchors.horizontalCenter: parent.horizontalCenter

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
      iconSize: fieldItem.iconSize
      fontColor: customStyle.fields.fontColor
      fontPixelSize: fieldItem.iconSize/2
      iconSource: fieldItem.galleryIcon
      labelText: qsTr("Add From a Gallery")

      visible: !readOnly && fieldItem.state !== " valid"
      height: fieldItem.iconSize
      anchors.top: photoButton.bottom
      anchors.horizontalCenter: parent.horizontalCenter

      MouseArea {
        anchors.fill: parent
        onClicked: externalResourceHandler.chooseImage(fieldItem)
      }
    }

    QgsQuick.IconTextItem {
      id: infoItem
      iconSize: fieldItem.iconSize/2
      fontColor: customStyle.fields.fontColor
      iconSource: fieldItem.brokenImageIcon
      labelText: qsTr("Image is broken: ") + image.currentValue

      visible: fieldItem.state === "broken" || fieldItem.state === "notAvailable"
      height: fieldItem.iconSize/2
      anchors.bottom: parent.bottom
      anchors.horizontalCenter: parent.horizontalCenter

    }
  }

}
