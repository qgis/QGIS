/***************************************************************************
 qgsquickmapcanvas.qml
  --------------------------------------
  Date                 : 10.12.2014
  Copyright            : (C) 2014 by Matthias Kuhn
  Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQml 2.14

import QgsQuick 0.1 as QgsQuick

Item {
  id: root

  /**
   * The mapSettings property contains configuration for rendering of the map.
   *
   * It should be used as a primary source of map settings (and project) for
   * all other components in the application.
   *
   * This is a readonly property.
   *
   * See also QgsQuickMapCanvasMap::mapSettings
   */
  property alias mapSettings: mapCanvasWrapper.mapSettings

  /**
   * The isRendering property is set to true while a rendering job is pending for this map canvas map.
   * It can be used to show a notification icon about an ongoing rendering job.
   *
   * This is a readonly property.
   *
   * See also QgsQuickMapCanvasMap::mapSettings
   */
  property alias isRendering: mapCanvasWrapper.isRendering

  /**
   * When the incrementalRendering property is set to true, the automatic refresh of map canvas during rendering is allowed.
   */
  property alias incrementalRendering: mapCanvasWrapper.incrementalRendering

  //! Consider mouse as a touchscreen device. If disabled, the mouse will act as a stylus pen.
  property bool mouseAsTouchScreen: false

  //! This signal is emitted independently of double tap / click
  signal clicked(var point)

  //! This signal is only emitted if there is no double tap/click coming after a short delay
//  signal confirmedClicked(var point) // TODO: ideally get rid of this one

  //! Signal emitted when user holds pointer on map
  signal longPressed(var point)

  //! Emitted when a release happens after a long press
  signal longPressReleased()

  //! Emitted when user does some interaction with map canvas (pan, zoom)
  signal userInteractedWithMap()

  /**
   * Freezes the map canvas refreshes.
   *
   * In case of repeated geometry changes (animated resizes, pinch, pan...)
   * triggering refreshes all the time can cause severe performance impacts.
   *
   * If freeze is called, an internal counter is incremented and only when the
   * counter is 0, refreshes will happen.
   * It is therefore important to call freeze() and unfreeze() exactly the same
   * number of times.
   */
  function freeze(id) {
    mapCanvasWrapper.__freezecount[id] = true
    mapCanvasWrapper.freeze = true

    userInteractedWithMap()
  }

  function unfreeze(id) {
    delete mapCanvasWrapper.__freezecount[id]
    mapCanvasWrapper.freeze = Object.keys(mapCanvasWrapper.__freezecount).length !== 0
  }

  function zoomIn(point) {
    mapCanvasWrapper.zoom(point, 0.67)
  }

  function zoomOut(point) {
    mapCanvasWrapper.zoom(point, 1.5)
  }

  QgsQuick.MapCanvasMap {
    id: mapCanvasWrapper

    width: root.width
    height: root.height

    property var __freezecount: ({})

    freeze: false
  }

  // Mouse, stylus and other pointer devices handler
  TapHandler {
    id: stylusClick

    property bool longPressActive: false

    enabled: !mouseAsTouchScreen
    acceptedDevices: PointerDevice.AllDevices & ~PointerDevice.TouchScreen

    onSingleTapped: {
      root.clicked(point.position)
    }

    onLongPressed: {
      root.longPressed(point.position)
      longPressActive = true
    }

    onPressedChanged: {
      if (longPressActive)
        root.longPressReleased()
      longPressActive = false
    }
  }

  // Map actions - select, long press, double tap - with fingers
  // Extra gesture - tap and hold - will forward grabPermissions to grabHandler to zoom in/out
  TapHandler {
    id: tapHandler

    property bool longPressActive: false
    property bool doublePressed: false
    property var timer: Timer {
      property var tapPoint

      interval: 350
      repeat: false

      onTriggered: {
        root.clicked(tapPoint)
      }
    }

    acceptedDevices: mouseAsTouchScreen ? PointerDevice.AllDevices : PointerDevice.TouchScreen

    onSingleTapped: {
      if(point.modifiers === Qt.RightButton)
      {
        mapCanvasWrapper.zoom(point.position, 1.25)
      }
      else
      {
        timer.tapPoint = point.position
        timer.restart()
      }
    }

    onDoubleTapped: {
      mapCanvasWrapper.zoom(point.position, 0.8)
    }

    onLongPressed: {
      root.longPressed(point.position)
      longPressActive = true
    }

    onPressedChanged: {
      if ( pressed && timer.running )
      {
        timer.stop()
        doublePressed = true
        dragHandler.grabPermissions = PointerHandler.CanTakeOverFromItems | PointerHandler.CanTakeOverFromHandlersOfDifferentType | PointerHandler.ApprovesTakeOverByAnything
      }
      else
      {
        doublePressed = false
        dragHandler.grabPermissions = PointerHandler.ApprovesTakeOverByHandlersOfSameType | PointerHandler.ApprovesTakeOverByHandlersOfDifferentType | PointerHandler.ApprovesTakeOverByItems
      }

      if (longPressActive)
        root.longPressReleased()
      longPressActive = false
    }
  }

  // Map panning with fingers and an extra gesture to zoom in/out after double tap (tap and hold)
  DragHandler {
    id: dragHandler

    target: null
    grabPermissions: PointerHandler.ApprovesTakeOverByHandlersOfSameType | PointerHandler.ApprovesTakeOverByHandlersOfDifferentType | PointerHandler.ApprovesTakeOverByItems

    property var oldPos
    property real oldTranslationY

    property bool isZooming: false
    property point zoomCenter

    onActiveChanged: {
      if ( active )
      {
        if ( tapHandler.doublePressed )
        {
          oldTranslationY = 0;
          zoomCenter = centroid.position;
          isZooming = true;
          freeze('zoom');
        }
        else
        {
          freeze('pan');
        }
      }
      else
      {
        unfreeze(isZooming ? 'zoom' : 'pan');
        isZooming = false;
      }
    }

    onCentroidChanged: {
      var oldPos1 = oldPos;
      oldPos = centroid.position;
      if ( active )
      {
        if ( isZooming )
        {
          mapCanvasWrapper.zoom(zoomCenter, Math.pow(0.8, (translation.y - oldTranslationY)/60))
          oldTranslationY = translation.y
        }
        else
        {
          mapCanvasWrapper.pan(centroid.position, oldPos1)
        }
      }
    }
  }

  // Mouse or stylus map zooming with action buttons
  DragHandler {
    target: null
    acceptedDevices: PointerDevice.Stylus | PointerDevice.Mouse
    grabPermissions: PointerHandler.TakeOverForbidden
    acceptedButtons: Qt.MiddleButton | Qt.RightButton

    property real oldTranslationY
    property point zoomCenter

    onActiveChanged: {
      if (active)
      {
        oldTranslationY = 0
        zoomCenter = centroid.position
      }

      if ( active )
        freeze('zoom')
      else
        unfreeze('zoom')
    }

    onTranslationChanged: {
      if (active)
      {
        mapCanvasWrapper.zoom(zoomCenter, Math.pow(0.8, (oldTranslationY - translation.y)/60))
      }

      oldTranslationY = translation.y
    }
  }

  // Two fingers pinch zooming
  PinchHandler {
    id: pinch
    target: null
    acceptedDevices: PointerDevice.TouchScreen | PointerDevice.TouchPad
    grabPermissions: PointerHandler.TakeOverForbidden

    property var oldPos
    property real oldScale: 1.0

    onActiveChanged: {
      if ( active ) {
        freeze('pinch')
        oldScale = 1.0
        oldPos = centroid.position
      } else {
        unfreeze('pinch')
      }
    }

    onCentroidChanged: {
      var oldPos1 = oldPos
      oldPos = centroid.position
      if ( active )
      {
        mapCanvasWrapper.pan(centroid.position, oldPos1)
      }
    }

    onActiveScaleChanged: {
      if ( oldScale !== 1 )
      {
        mapCanvasWrapper.zoom( pinch.centroid.position, oldScale / pinch.activeScale )
        mapCanvasWrapper.pan( pinch.centroid.position, oldPos )

      }
      oldScale = pinch.activeScale
    }
  }

  // Mouse wheel zooming
  WheelHandler {
    target: null
    grabPermissions: PointerHandler.CanTakeOverFromHandlersOfDifferentType | PointerHandler.ApprovesTakeOverByItems

    onWheel: {
      if (event.angleDelta.y > 0)
      {
        zoomIn(point.position)
      }
      else
      {
        zoomOut(point.position)
      }

      userInteractedWithMap()
    }
  }
}
