/***************************************************************************
  qgsquickpositionmarker.qml
  --------------------------------------
  Date                 : Dec 2017
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

import QtQuick 2.3
import QtQuick.Controls 2.2
import QtQml 2.2
import QtGraphicalEffects 1.0
import QgisQuick 0.1 as QgsQuick

Item {
  id: positionMarker
  property int size: 48 * QgsQuick.Utils.dp

  property QgsQuick.MapSettings mapSettings // required to be connected from parent!
  property QgsQuick.PositionKit positionKit: QgsQuick.PositionKit {id: positionKit}

  property var simulatePositionLongLatRad // use in debug mode to simulate movement around some GPS location
  // longitude, latitude, and radius, all in degrees WSG84
  // e.g. simulatePositionLongLatRad = [60, 10, 0.02]

  property point screenPosition  // in pixels
  property color baseColor: "darkblue"
  property color unavailableColor: "gray"
  property alias mapPosition: wgs84toMapCrs.projectedPosition // in map coordinates
  property alias gpsPosition: positionKit.position // in WGS84 coordinates
  property alias gpsAccuracy: positionKit.accuracy // in meters

  property var gpsPositionLabel: {
    if (positionKit.hasPosition) {
      QgsQuick.Utils.qgsPointToString(positionKit.position, 3) // e.g -2.243, 45.441
    } else {
      "GPS signal lost"
    }
  }
  property var gpsAccuracyLabel: {
    if (positionMarker.withAccuracy) {
      if (positionKit.hasPosition && positionKit.accuracy > 0) {
        QgsQuick.Utils.distanceToString(positionKit.accuracy, 0) // e.g 1 km or 15 m or 500 mm
      } else {
        "GPS accuracy N/A"
      }
    } else {
      ""
    }
  }
  property var withAccuracy: true
  property var markerIcon: QgsQuick.Utils.getThemeIcon("ic_navigation_black")

  onMapSettingsChanged: update_location()

  Connections {
    target: mapSettings
    onVisibleExtentChanged: update_location()
  }

  function update_location() {
    if (mapSettings)
      screenPosition = mapSettings.coordinateToScreen(wgs84toMapCrs.projectedPosition)
  }

  onSimulatePositionLongLatRadChanged: {
    if (simulatePositionLongLatRad) {
      var longitude = simulatePositionLongLatRad[0]
      var latitude = simulatePositionLongLatRad[1]
      var radius = simulatePositionLongLatRad[2]
      console.log("Use simulated position around longlat: " + longitude + ", " + latitude + ", radius " + radius )
      positionKit.use_simulated_location(longitude, latitude, radius);
    } else {
      positionKit.use_gps_location();
    }
  }

  QgsQuick.CoordinateTransformer {
    id: wgs84toMapCrs
    sourceCrs: QgsQuick.Utils.coordinateReferenceSystemFromEpsgId(4326)
    destinationCrs: mapSettings.destinationCrs
    sourcePosition: positionKit.position
    mapSettings: positionMarker.mapSettings
    onProjectedPositionChanged: update_location()
  }

  // GPS accuracy circle-shaped indicator around positionMarker
  Rectangle {
    id: accuracyIndicator
    visible: mapSettings &&
             withAccuracy &&
             positionKit.hasPosition &&
             (positionKit.accuracy > 0) &&
             (accuracyIndicator.width > positionMarker.size / 2.0)
    x: positionMarker.screenPosition.x - width/2
    y: positionMarker.screenPosition.y - height/2
    width: {
      if (positionKit.accuracy > 0) {
        var scpm = QgsQuick.Utils.screenUnitsToMeters(positionMarker.mapSettings, 1) // scpm is how much meters is 1 pixel
        if (scpm > 0)
          2 * ( positionKit.accuracy / scpm )
        else
          2
      }
      else 2
    }
    height: accuracyIndicator.width
    color: baseColor
    border.color: "black"
    border.width: 3
    radius: width*0.5
    opacity: 0.1
  }

  Rectangle {
          id: navigationMarker
          property int borderWidth: 2
          width: positionMarker.size + 20
          height: width
          color: "white"
          border.color: baseColor
          border.width: borderWidth
          radius: width*0.5
          antialiasing: true
          x: positionMarker.screenPosition.x - width/2
          y: positionMarker.screenPosition.y - height/2

          Image {
            id: navigation
            source: positionMarker.markerIcon
            fillMode: Image.PreserveAspectFit
            rotation: positionKit.direction
            anchors.centerIn: parent
            width: positionMarker.size
            height: width
          }

          // Makes positionMarker (navigation) grey if gps signal is lost.
          ColorOverlay {
            anchors.fill: navigation
            source: navigation
            color: positionKit.hasPosition ? baseColor : unavailableColor
            rotation: positionKit.direction
            visible: !(positionKit.hasPosition && baseColor == "black")
          }
      }

}
