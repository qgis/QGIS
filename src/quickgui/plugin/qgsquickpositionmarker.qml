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
import QgsQuick 0.1 as QgsQuick

/**
 * \brief Graphical representation of physical location on the map.
 *
 * Source position and accuracy taken from PositionKit is drawn as a marker on the map.
 * Marker is grayed out when position is not available. When PositionKit support reading of the accuracy,
 * the circle is drawn around the marker. PositionKit must be connected, for example GPS position source from QgsQuickPositionKit.
 */
Item {
  id: positionMarker
  property int size: 48 * QgsQuick.Utils.dp

  /**
   * Utils for handling position.
   */
  property QgsQuick.PositionKit positionKit

  /**
   * Color of the marker when position is known.
   */
  property color baseColor: "darkblue"
  /**
   * Color of the marker when position is unknown (e.g. GPS signal lost).
   */
  property color unavailableColor: "gray"

  /**
   * Whether circle representing accuracy of the position should be rendered.
   */
  property bool withAccuracy: true

  /**
   * Icon for position marker.
   */
  property var markerIcon: QgsQuick.Utils.getThemeIcon("ic_navigation_black")

  /**
   * Source position accuracy circle-shaped indicator around positionMarker.
   */
  Rectangle {
    id: accuracyIndicator
    visible: withAccuracy &&
             positionKit.hasPosition &&
             (positionKit.accuracy > 0) &&
             (accuracyIndicator.width > positionMarker.size / 2.0)
    x: positionKit.screenPosition.x - width/2
    y: positionKit.screenPosition.y - height/2
    width:positionKit.screenAccuracy
    height: accuracyIndicator.width
    color: baseColor
    border.color: "black"
    border.width: 3 * QgsQuick.Utils.dp
    radius: width*0.5
    opacity: 0.1
  }

  /**
   * Position marker.
   */
  Rectangle {
    id: navigationMarker
    property int borderWidth: 2 * QgsQuick.Utils.dp
    width: positionMarker.size + 20 * QgsQuick.Utils.dp
    height: width
    color: "white"
    border.color: baseColor
    border.width: borderWidth
    radius: width*0.5
    antialiasing: true
    x: positionKit.screenPosition.x - width/2
    y: positionKit.screenPosition.y - height/2

    Image {
      id: navigation
      source: positionMarker.markerIcon
      fillMode: Image.PreserveAspectFit
      rotation: positionKit.direction
      anchors.centerIn: parent
      width: positionMarker.size
      height: width
    }

    /**
     * Makes positionMarker (navigation) grey if position is unknown.
     */
    ColorOverlay {
      anchors.fill: navigation
      source: navigation
      color: positionKit.hasPosition ? baseColor : unavailableColor
      rotation: positionKit.direction
      visible: !(positionKit.hasPosition)
    }
  }
}

