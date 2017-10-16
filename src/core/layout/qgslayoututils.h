/***************************************************************************
                             qgslayoututils.h
                             -------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTUTILS_H
#define QGSLAYOUTUTILS_H

#include "qgis_core.h"

class QgsRenderContext;
class QgsLayout;
class QgsLayoutItemMap;
class QPainter;
class QRectF;

/**
 * \ingroup core
 * Utilities for layouts.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutUtils
{
  public:

    /**
     * Ensures that an \a angle (in degrees) is in the range 0 <= angle < 360.
     * If \a allowNegative is true then angles between (-360, 360) are allowed. If false,
     * angles are converted to positive angles in the range [0, 360).
     */
    static double normalizedAngle( const double angle, const bool allowNegative = false );

    /**
     * Creates a render context suitable for the specified layout \a map and \a painter destination.
     * This method returns a new QgsRenderContext which matches the scale and settings of the
     * target map. If the \a dpi argument is not specified then the dpi will be taken from the destination
     * painter device.
     * \see createRenderContextForLayout()
     */
    static QgsRenderContext createRenderContextForMap( QgsLayoutItemMap *map, QPainter *painter, double dpi = -1 );

    /**
     * Creates a render context suitable for the specified \a layout and \a painter destination.
     * This method returns a new QgsRenderContext which matches the scale and settings from the layout's
     * QgsLayout::referenceMap().
     * If the \a dpi argument is not specified then the dpi will be taken from the destination
     * painter device.
     * \see createRenderContextForMap()
     */
    static QgsRenderContext createRenderContextForLayout( QgsLayout *layout, QPainter *painter, double dpi = -1 );

    /**
     * Resizes a QRectF relative to a resized bounding rectangle.
     * \param rectToResize QRectF to resize, contained within boundsBefore. The
     * rectangle is linearly scaled to retain its relative position and size within
     * boundsAfter.
     * \param boundsBefore QRectF of bounds before resize
     * \param boundsAfter QRectF of bounds after resize
     */
    static void relativeResizeRect( QRectF &rectToResize, const QRectF &boundsBefore, const QRectF &boundsAfter );

    /**
     * Returns a scaled position given a before and after range
     * \param position initial position within before range to scale
     * \param beforeMin minimum value in before range
     * \param beforeMax maximum value in before range
     * \param afterMin minimum value in after range
     * \param afterMax maximum value in after range
     * \returns position scaled to range specified by afterMin and afterMax
     */
    static double relativePosition( const double position, const double beforeMin, const double beforeMax, const double afterMin, const double afterMax );


    /**
     * Returns true if the render to the specified \a painter is a preview render,
     * i.e. is being rendered inside a QGraphicsView widget as opposed to a destination
     * device (such as an image).
     */
    static bool isPreviewRender( QPainter *painter );

};

#endif //QGSLAYOUTUTILS_H
