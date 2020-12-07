/***************************************************************************
  qgsquickscalebar.qml
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

Item {
  id: scaleBar

  /**
   * The mapSettings property contains configuration for rendering of the map.
   *
   * Must be connected before usage from QgsQuick.MapCanvas::mapSettings
   */
  property alias mapSettings: scaleBarKit.mapSettings
  /**
   * Preferred width of scalebar in pixels on the screen. Defaults set to 300.
   *
   * barWidth is calculated to be close to preferred width, but has "nice" textual
   * representation
   */
  property alias preferredWidth: scaleBarKit.preferredWidth
  /**
   * Preferred system of measurement for the resulting distance. Default is metric system
   */
  property alias systemOfMeasurement: scaleBarKit.systemOfMeasurement
  /**
   * Kit for all calculation of width and text of the scalebar
   *
   * See also QgsQuickMapCanvasMap::mapSettings
   */
  property QgsQuick.ScaleBarKit scaleBarKit: QgsQuick.ScaleBarKit {
    id: scaleBarKit
  }
  /**
   * Reserved text width.
   */
  property int textWidth: fontMetrics.averageCharacterWidth * 8
  /**
   * Text color
   */
  property color barColor: "white"
  /**
   * Background color
   */
  property color barBackgroundColor: "grey"
  /**
   * Opacity
   */
  property double barOpacity: 0.8
  /**
   * Textual representation of the bar length (e.g 800 m)
   */
  property string barText: scaleBarKit.distance + " " + scaleBarKit.units
  /**
   * Calculated width of bar in pixels
   */
  property int barWidth: scaleBarKit.width
  /**
   * Size of scalebar line (height of bar)
   */
  property int lineWidth: 5 * QgsQuick.Utils.dp

  width: textWidth + barWidth

  MouseArea {
    anchors.fill: background
    onClicked: {
      animation.restart()
    }
  }

  NumberAnimation {
    id: animation
    target: scaleBar
    property: "barWidth"
    to: 200
    duration: 1000
  }

  Rectangle {
    id: background
    color: scaleBar.barBackgroundColor
    opacity: scaleBar.barOpacity
    width: parent.width
    height: parent.height
  }

  FontMetrics {
    id: fontMetrics
    font: text.font
  }

  Row {
    opacity: 1
    spacing: 0

    Text {
      id: text
      width: textWidth
      height: scaleBar.height
      text: barText
      color: barColor
      font.pixelSize: scaleBar.height - 2 * scaleBar.lineWidth
      horizontalAlignment: Text.AlignHCenter
      verticalAlignment: Text.AlignVCenter
    }

    Rectangle {
      id: leftBar
      width: scaleBar.lineWidth
      height: scaleBar.height - 20 * QgsQuick.Utils.dp
      y: (scaleBar.height - leftBar.height) / 2
      color: barColor
      opacity: 1
    }

    Rectangle {
      width: scaleBar.width - text.width - 15 * QgsQuick.Utils.dp
      height: scaleBar.lineWidth
      y: (scaleBar.height - scaleBar.lineWidth) / 2
      color: barColor
    }

    Rectangle {
      id: rightBar
      width: scaleBar.lineWidth
      height: scaleBar.height - 20 * QgsQuick.Utils.dp
      y: (scaleBar.height - leftBar.height) / 2
      color: barColor
    }
  }
}
