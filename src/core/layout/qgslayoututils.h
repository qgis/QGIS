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
     * target map. If the \a dpi argument is not specified then the dpi will be taken from the destinatation
     * painter device.
     * \see createRenderContextForLayout()
     */
    static QgsRenderContext createRenderContextForMap( QgsLayoutItemMap *map, QPainter *painter, double dpi = -1 );

    /**
     * Creates a render context suitable for the specified \a layout and \a painter destination.
     * This method returns a new QgsRenderContext which matches the scale and settings from the layout's
     * QgsLayout::referenceMap().
     * \see createRenderContextForMap()
     */
    static QgsRenderContext createRenderContextForLayout( QgsLayout *layout, QPainter *painter );

};

#endif //QGSLAYOUTUTILS_H
