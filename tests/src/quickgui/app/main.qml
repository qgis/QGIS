/***************************************************************************
  main.qml
  --------------------------------------
  Date                 : Nov 2017
  Copyright            : (C) 2017 by Peter Petrik
  Email                : zilolv at gmail dot com
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
import QgsQuick 0.1 as QgsQuick
import "."

ApplicationWindow {
  id: window
  visible: true
  visibility: "Maximized"
  title: qsTr("QGIS Quick Test App")

  QgsQuick.MapCanvas {
    id: mapCanvas

    height: parent.height
    width: parent.width

    mapSettings.project: __project
    mapSettings.layers: __layers

    QgsQuick.IdentifyKit {
      id: identifyKit
      mapSettings: mapCanvas.mapSettings
    }

    onClicked: {
      var screenPoint = Qt.point(mouse.x, mouse.y)
      var res = identifyKit.identifyOne(screenPoint);
      if (QgsQuick.Utils.hasValidGeometry(res.layer, res.feature)) {
        featureModel.feature = res
      }
    }
  }

  QgsQuick.FeatureModel {
    id: featureModel
  }

  Item {
    anchors.fill: mapCanvas
    transform: QgsQuick.MapTransform {
      mapSettings: mapCanvas.mapSettings
    }

    QgsQuick.FeatureHighlight {
      color: "red"
      width: 20
      model: featureModel
      mapSettings: mapCanvas.mapSettings
    }

    z: 1   // make sure items from here are on top of the Z-order
  }

  Drawer {
    id: logPanel
    visible: true
    modal: true
    interactive: true
    height: window.height
    width: QgsQuick.Utils.dp * 700
    edge: Qt.RightEdge
    z: 2 // make sure items from here are on top of the Z-order

    background: Rectangle {
      color: "white"
    }

    QgsQuick.MessageLog {
      id: messageLog
      width: parent.width
      height: parent.height
      model: QgsQuick.MessageLogModel {}
      visible: true
    }
  }

  QgsQuick.ScaleBar {
    id: scaleBar
    y: window.height - height
    height: 50
    mapSettings: mapCanvas.mapSettings
    preferredWidth: 115 * QgsQuick.Utils.dp
    z: 1
  }
}
