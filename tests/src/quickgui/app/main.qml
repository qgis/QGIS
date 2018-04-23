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
import QgisQuick 0.1 as QgsQuick
import "."

ApplicationWindow {
    id: window
    visible: true
    visibility: "Maximized"
    title: qsTr("QGIS Quick Test App")

    // Some info
    Button {
        id: logbutton
        text: "Log"
        onClicked: logPanel.visible = true
        z: 1
    }

    Label {
        text: positionMarker.gpsPositionLabel
        z: 1
        x: logbutton.width + 10
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
            if (res.valid)
            {
                featurePanel.show_panel(
                            res.layer,
                            res.feature,
                            "Edit" )
            }
        }
    }

    Item {
        anchors.fill: mapCanvas
        transform: QgsQuick.MapTransform {
            mapSettings: mapCanvas.mapSettings
        }

        QgsQuick.FeatureHighlight {
            color: "red"
            width: 20
            model: featurePanel.visible ? featurePanel.currentFeatureModel : null
            mapSettings: mapCanvas.mapSettings
        }

        z: 1   // make sure items from here are on top of the Z-order
    }


    Drawer {
        id: logPanel
        visible: false
        modal: true
        interactive: true
        dragMargin: 0 // prevents opening the drawer by dragging.
        height: window.height
        width: QgsQuick.Utils.dp * 700
        edge: Qt.RightEdge
        z: 2   // make sure items from here are on top of the Z-order

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

    QgsQuick.PositionMarker {
        id: positionMarker
        mapSettings: mapCanvas.mapSettings
        simulatePositionLongLatRad: __use_simulated_position ? [-97.36, 36.93, 2] : undefined
    }

    QgsQuick.ScaleBar {
        id: scaleBar
        y: window.height - height
        height: 50
        mapSettings: mapCanvas.mapSettings
        preferredWidth: 115 * QgsQuick.Utils.dp
        z: 1
    }

    FeaturePanel {
        id: featurePanel
        height: window.height
        width: QgsQuick.Utils.dp * 700
        edge: Qt.RightEdge
        mapSettings: mapCanvas.mapSettings
        project: __project
    }

}
