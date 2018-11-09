/***************************************************************************
  main.qml
  --------
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
  title: "QGIS Quick Test App"

  // Some info
  Button {
    id: logbutton
    text: "Log"
    onClicked: logPanel.visible = true
    z: 1
  }

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
      var screenPoint = Qt.point( mouse.x, mouse.y );

      var res = identifyKit.identifyOne(screenPoint);
      highlight.featureLayerPair = res
      if (res.valid)
        featurePanel.show_panel(res, "Edit" )
    }
  }

  QgsQuick.FeatureHighlight {
    anchors.fill: mapCanvas
    id: highlight
    color: "red"
    mapSettings: mapCanvas.mapSettings
    z: 1
  }

  /** Message Log */
  Drawer {
    id: logPanel
    visible: false
    modal: true
    interactive: true
    height: window.height
    width: 700 * QgsQuick.Utils.dp
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
    }
  }

  /** Scale Bar in metric units*/
  QgsQuick.ScaleBar {
    id: scaleBar
    y: window.height - height
    height: 50 * QgsQuick.Utils.dp
    mapSettings: mapCanvas.mapSettings
    preferredWidth: 115 * QgsQuick.Utils.dp
    z: 1
  }

  /** Scale Bar in imperial units*/
  QgsQuick.ScaleBar {
    id: scaleBarImperialUnits
    y: window.height - height
    x: window.width/2
    height: scaleBar.height
    mapSettings: mapCanvas.mapSettings
    preferredWidth: scaleBar.preferredWidth
    systemOfMeasurement: QgsQuick.QgsUnitTypes.ImperialSystem
    z: 1
  }

  /** Position Kit and Marker */
  QgsQuick.PositionKit {
    id: positionKit
    mapSettings: mapCanvas.mapSettings
    simulatePositionLongLatRad: __use_simulated_position ? [-97.36, 36.93, 2] : undefined
  }

  QgsQuick.PositionMarker {
    id: positionMarker
    positionKit: positionKit
    z: 2
  }

  Label {
    id: gpsPositionLabel
    text: {
      var label = "Signal Lost"
      if ( positionKit.hasPosition )
        label = QgsQuick.Utils.formatPoint( positionKit.position )
        if (positionKit.accuracy > 0)
          label += " (" + QgsQuick.Utils.formatDistance( positionKit.accuracy, positionKit.accuracyUnits, 0 ) + ")"
      label;
    }
    height: scaleBar.height
    x: window.width - width
    font.pixelSize: 22 * QgsQuick.Utils.dp
    font.italic: true
    color: "steelblue"
    z: 1
  }

  /** Coordinate transformater */
  QgsQuick.CoordinateTransformer {
    id: coordinateTransformer
    sourcePosition: positionKit.position
    sourceCrs: positionKit.positionCRS()
    destinationCrs: QgsQuick.Utils.coordinateReferenceSystemFromEpsgId( 3857 ) //web mercator
    transformContext: mapCanvas.mapSettings.transformContext()
  }

  Label {
    id: webPositionLabel
    text: {
      if ( positionKit.hasPosition )
         QgsQuick.Utils.formatPoint( coordinateTransformer.projectedPosition ) + " (web mercator)"
    }
    height: scaleBar.height
    x: window.width - width
    y: gpsPositionLabel.height + 2 * QgsQuick.Utils.dp
    font.pixelSize: 22 * QgsQuick.Utils.dp
    font.italic: true
    color: "steelblue"
    z: 1
  }

  FeaturePanel {
    id: featurePanel
    height: window.height
    width: 700 * QgsQuick.Utils.dp
    edge: Qt.RightEdge
    mapSettings: mapCanvas.mapSettings
    project: __project
    visible: false
  }
}
