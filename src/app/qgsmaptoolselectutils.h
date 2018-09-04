/***************************************************************************
qgsmaptoolselectutils.h  -  Utility methods to help with select map tools
---------------------
begin                : May 2010
copyright            : (C) 2010 by Jeremy Palmer
email                : jpalmer at linz dot govt dot nz
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSMAPTOOLSELECTUTILS_H
#define QGSMAPTOOLSELECTUTILS_H

#include <Qt>
#include <QRect>
#include <QPoint>

#include "qgsvectorlayer.h"

class QMouseEvent;
class QgsMapCanvas;
class QgsVectorLayer;
class QgsGeometry;
class QgsRubberBand;

/**
  Namespace containing methods which are useful for the select maptool widgets
 */
namespace QgsMapToolSelectUtils
{

  /**
   * Calculates a list of features matching a selection geometry and flags.
   * \param canvas the map canvas used to get the current selected vector layer and
    for any required geometry transformations
   * \param selectGeometry the geometry to select the layers features. This geometry
    must be in terms of the canvas coordinate system.
   * \param doContains features will only be selected if fully contained within
    the selection rubber band (otherwise intersection is enough).
   * \param singleSelect only selects the closest feature to the selectGeometry.
   * \returns list of features which match search geometry and parameters
   * \since QGIS 2.16
   */
  QgsFeatureIds getMatchingFeatures( QgsMapCanvas *canvas, const QgsGeometry &selectGeometry, bool doContains, bool singleSelect );

  /**
    Selects the features within currently selected layer.
    \param canvas the map canvas used to get the current selected vector layer and
    for any required geometry transformations
    \param selectGeometry the geometry to select the layers features. This geometry
    must be in terms of the canvas coordinate system.
    \param selectBehavior behavior of select (ie replace selection, add to selection)
    \param doContains features will only be selected if fully contained within
    the selection rubber band (otherwise intersection is enough).
    \param singleSelect only selects the closest feature to the selectGeometry.
    \since QGIS 2.16
  */
  void setSelectedFeatures( QgsMapCanvas *canvas,
                            const QgsGeometry &selectGeometry,
                            QgsVectorLayer::SelectBehavior selectBehavior = QgsVectorLayer::SetSelection,
                            bool doContains = true,
                            bool singleSelect = false );

  /**
    Selects multiple matching features from within currently selected layer.
    \param canvas the map canvas used to get the current selected vector layer and
    for any required geometry transformations
    \param selectGeometry the geometry to select the layers features. This geometry
    must be in terms of the canvas coordinate system.
    \param modifiers Keyboard modifiers are used to determine the current selection
    operations (add, subtract, contains)
    \since QGIS 2.16
    \see selectSingleFeature()
  */
  void selectMultipleFeatures( QgsMapCanvas *canvas, const QgsGeometry &selectGeometry, const Qt::KeyboardModifiers &modifiers );

  /**
    Selects a single feature from within currently selected layer.
    \param canvas the map canvas used to get the current selected vector layer and
    for any required geometry transformations
    \param selectGeometry the geometry to select the layers features. This geometry
    must be in terms of the canvas coordinate system.
    \param modifiers Keyboard modifiers are used to determine the current selection
    operations (add, subtract, contains)
    \see selectMultipleFeatures()
  */
  void selectSingleFeature( QgsMapCanvas *canvas, const QgsGeometry &selectGeometry, const Qt::KeyboardModifiers &modifiers );

  /**
    Get the current selected canvas map layer. Returns nullptr if it is not a vector layer
    \param canvas The map canvas used for getting the current layer
    \returns QgsVectorLayer The layer
  */
  QgsVectorLayer *getCurrentVectorLayer( QgsMapCanvas *canvas );

  /**
  Expands a point to a rectangle with minimum size for selection based on the vector layer type
  \param point The point to expand the rectangle around (in map coordinates)
  \param canvas The map canvas used to transform between canvas and map units
  \param vlayer The vector layer layer
  \returns Expanded rectangle in map units
  */
  QgsRectangle expandSelectRectangle( QgsPointXY mapPoint, QgsMapCanvas *canvas, QgsVectorLayer *vlayer );

  /**
  Sets a QgsRubberband to rectangle in map units using a rectangle defined in device coords
  \param canvas The map canvas used to transform the rectangle into map units
  \param selectRect The input rectangle in device coords
  \param rubberBand The rubberband that will be set in map units using the input rectangle
  */
  void setRubberBand( QgsMapCanvas *canvas, QRect &selectRect, QgsRubberBand *rubberBand );
}

#endif
