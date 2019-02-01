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

  id: fieldItem
  anchors {
    left: parent.left
    right: parent.right
    rightMargin: 10 * QgsQuick.Utils.dp
  }

  height: Math.max(image.height, button.height)

  QgsQuick.PhotoCapture {
    id: photoCapturePanel
    visible: false
    height: window.height
    width: window.width
    edge: Qt.RightEdge
    imageButtonSize: button.height
  }

  Image {
    property var currentValue: value

    id: image
    width: customStyle.height * 3
    autoTransform: true
    fillMode: Image.PreserveAspectFit
    visible: currentValue

    Component.onCompleted: image.source = getSource()

    function getSource() {
      if (image.status === Image.Error)
        return ""
      else if (image.currentValue && QgsQuick.Utils.fileExists(homePath + "/" + image.currentValue))
        return homePath + "/" + image.currentValue
      else
        return ""
    }
  }

  Image {
      id: icon
      source: image.status === Image.Error ? QgsQuick.Utils.getThemeIcon("ic_broken_image_black") : QgsQuick.Utils.getThemeIcon("ic_photo_notavailable_white")
      width: button.width
      height: button.height
      sourceSize.width: width
      sourceSize.height: height
      fillMode: Image.PreserveAspectFit
      visible: !image.currentValue
  }

  ColorOverlay {
      anchors.fill: icon
      source: icon
      color: customStyle.fontColor
  }

  Button {
    id: button
    visible: enabled
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
