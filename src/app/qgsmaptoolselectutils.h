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
/* $Id$ */

#ifndef QGSMAPTOOLSELECTUTILS_H
#define QGSMAPTOOLSELECTUTILS_H

#include <Qt>
#include <QRect>
#include <QPoint>

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
    Selects the features within currently selected layer.
    @param canvas The map canvas used to get the current selected vector layer and
    for any required geometry transformations
    @param selectGeometry The geometry to select the layers features. This geometry
    must be in terms of the canvas coordinate system.
    @param doContains Features will only be selected if contained within the
    selection rubber band.
    @param doDifference Take the symmetric difference of the the current selected
    features and the new features found within the provided selectGeometry.
    @param singleSelect Only selects the closest feature to the selectGeometry.
  */
  void setSelectFeatures( QgsMapCanvas* canvas,
                          QgsGeometry* selectGeometry,
                          bool doContains = true,
                          bool doDifference = false,
                          bool singleSelect = false );

  /**
    Select the features within currently selected layer.
    @param canvas The map canvas used to get the current selected vector layer and
    for any required geometry transformations
    @param selectGeometry The geometry to select the layers features. This geometry
    must be in terms of the canvas coordinate system.
    @param e MouseEvents are used to determine the current selection
    operations (add, subtract, contains)
  */
  void setSelectFeatures( QgsMapCanvas* canvas, QgsGeometry* selectGeometry, QMouseEvent * e );

  /**
    Get the current selected canvas map layer. Returns NULL if it is not a vector layer
    @param canvas The map canvas used for getting the current layer
    @return QgsVectorLayer The layer
  */
  QgsVectorLayer* getCurrentVectorLayer( QgsMapCanvas* canvas );

  /**
  Expands a rectangle to a minimum size for selection based on the vector layer type
  @param selectRect The QRect to expand
  @param vlayer The vector layer layer
  @param vlayer The point to expand the rectangle around
  */
  void expandSelectRectangle( QRect& selectRect, QgsVectorLayer* vlayer, const QPoint point );

  /**
  Sets a QgsRubberband to rectangle in map units using a rectangle defined in device coords
  @param canvas The map canvas used to transform the rectangle into map units
  @param selectRect The input rectangle in device coords
  @param rubberBand The rubberband that will be set in map units using the input rectangle
  */
  void setRubberBand( QgsMapCanvas* canvas, QRect& selectRect, QgsRubberBand* rubberBand );
}

#endif
