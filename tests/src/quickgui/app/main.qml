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

  QgsQuick.MapCanvas {
    id: mapCanvas

    height: parent.height
    width: parent.width

    mapSettings.project: __project
    mapSettings.layers: __layers
  }
  
  QgsQuick.ElevationProfileCanvas {
    id: elevationProfileCanvas
    
    height: parent.height
    width: parent.width
    
    project: __project
    crs: mapCanvas.mapSettings.destinationCrs
  }
}
