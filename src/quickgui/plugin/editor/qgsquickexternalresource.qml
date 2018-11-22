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
  anchors.left: parent.left
  anchors.right: parent.right

  height: Math.max(image.height, button.height)

  QgsQuick.PhotoCapture {
    id: photoCapturePanel
    visible: false
    height: window.height
    width: window.width
    edge: Qt.RightEdge
  }

  Image {
    property var currentValue: value

    id: image
    width: 200 * QgsQuick.Utils.dp
    autoTransform: true
    fillMode: Image.PreserveAspectFit

    Component.onCompleted: image.source = getSource()

    function getSource() {
      if (image.status === Image.Error)
        return QgsQuick.Utils.getThemeIcon("ic_broken_image_black")
      else if (image.currentValue && QgsQuick.Utils.fileExists(homePath + "/" + image.currentValue))
        return homePath + "/" + image.currentValue
      else
        return QgsQuick.Utils.getThemeIcon("ic_photo_notavailable_white")
    }
  }

  Button {
    id: button
    visible: fieldItem.enabled
    width: 45 * QgsQuick.Utils.dp
    height: 45 * QgsQuick.Utils.dp

    anchors.right: parent.right
    anchors.bottom: parent.bottom

    onClicked: {
      photoCapturePanel.visible = true
      photoCapturePanel.targetDir = homePath
      photoCapturePanel.fieldItem = fieldItem
    }

    background: Image {
      source: QgsQuick.Utils.getThemeIcon("ic_camera_alt_border")
      width: button.width
      height: button.height
    }
  }
}
