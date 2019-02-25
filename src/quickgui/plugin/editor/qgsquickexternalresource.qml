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
 */
Item {
  signal valueChanged(var value, bool isNull)

  property var image: image
  property var brokenImageSource: QgsQuick.Utils.getThemeIcon("ic_broken_image_black")
  property var notavailableImageSource: QgsQuick.Utils.getThemeIcon("ic_photo_notavailable_white")

  id: fieldItem
  height: image.hasValidSource? customStyle.height * 3 : customStyle.height
  anchors {
    left: parent.left
    right: parent.right
    rightMargin: 10 * QgsQuick.Utils.dp
  }

  QgsQuick.PhotoCapture {
    id: photoCapturePanel
    visible: false
    height: window.height
    width: window.width
    edge: Qt.RightEdge
    imageButtonSize: customStyle.height
  }

  Image {
    property var currentValue: value
    property bool hasValidSource: false

    id: image
    height: fieldItem.height
    sourceSize.height: height
    autoTransform: true
    fillMode: Image.PreserveAspectFit
    visible: hasValidSource

    MouseArea {
        anchors.fill: parent
        onClicked: externalResourceHandler.previewImage(homePath + "/" + image.currentValue)
    }

    onCurrentValueChanged: {
        image.source = image.getSource()
    }

    onSourceChanged: {
        hasValidSource = (image.source ===  fieldItem.brokenImageSource ||
                          image.source === fieldItem.notavailableImageSource) ? false : true
    }

    Component.onCompleted: image.source = getSource()

    function getSource() {
         if (image.status === Image.Error)
           return fieldItem.brokenImageSource
         else if (image.currentValue && QgsQuick.Utils.fileExists(homePath + "/" + image.currentValue))
           return homePath + "/" + image.currentValue
         else
           return fieldItem.notavailableImageSource
       }
     }

  ColorOverlay {
      anchors.fill: image
      source: image
      color: customStyle.fontColor
      visible: !image.hasValidSource
  }

  Button {
    id: deleteButton
    visible: fieldItem.enabled && image.hasValidSource
    width: customStyle.height
    height: width
    padding: 0

    anchors.right: imageBrowserButton.left
    anchors.bottom: parent.bottom
    anchors.verticalCenter: parent.verticalCenter

    onClicked: externalResourceHandler.removeImage(fieldItem, homePath + "/" + image.currentValue)

    background: Image {
      id: deleteIcon
      source: QgsQuick.Utils.getThemeIcon("ic_delete_forever_white")
      width: deleteButton.width
      height: deleteButton.height
      sourceSize.width: width
      sourceSize.height: height
      fillMode: Image.PreserveAspectFit
    }

    ColorOverlay {
        anchors.fill: deleteIcon
        source: deleteIcon
        color: customStyle.fontColor
    }
  }

  Button {
    id: imageBrowserButton
    visible: fieldItem.enabled
    width: customStyle.height
    height: width
    padding: 0

    anchors.right: button.left
    anchors.bottom: parent.bottom
    anchors.verticalCenter: parent.verticalCenter

    onClicked:externalResourceHandler.chooseImage(fieldItem)

    background: Image {
      id: browseIcon
      source: QgsQuick.Utils.getThemeIcon("ic_gallery")
      width: imageBrowserButton.width
      height: imageBrowserButton.height
      sourceSize.width: width
      sourceSize.height: height
      fillMode: Image.PreserveAspectFit
    }

    ColorOverlay {
        anchors.fill: browseIcon
        source: browseIcon
        color: customStyle.fontColor
    }
  }

  Button {
    id: button
    visible: fieldItem.enabled
    width: customStyle.height
    height: width
    padding: 0

    anchors.right: parent.right
    anchors.bottom: parent.bottom
    anchors.verticalCenter: parent.verticalCenter

    onClicked: {
      photoCapturePanel.visible = true
      photoCapturePanel.targetDir = homePath
      photoCapturePanel.fieldItem = fieldItem
    }

    background: Image {
      id: cameraIcon
      source: QgsQuick.Utils.getThemeIcon("ic_camera")
      width: button.width
      height: button.height
      sourceSize.width: width
      sourceSize.height: height
      fillMode: Image.PreserveAspectFit
    }

    ColorOverlay {
        anchors.fill: cameraIcon
        source: cameraIcon
        color: customStyle.fontColor
    }
  }
}
