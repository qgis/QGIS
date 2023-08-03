/***************************************************************************
    qgspainting.h
    ---------------------
    begin                : July 2016
    copyright            : (C) 2016 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPAINTING_H
#define QGSPAINTING_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgis_sip.h"

#include <QPainter>

class QTransform;

/**
 * \ingroup core
 * \brief Contains miscellaneous painting utility functions.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsPainting
{
  public:

    /**
     * Returns a QPainter::CompositionMode corresponding to a Qgis::BlendMode.
     *
     * \see getBlendModeEnum
     */
    static QPainter::CompositionMode getCompositionMode( Qgis::BlendMode blendMode );

    /**
     * Returns a Qgis::BlendMode corresponding to a QPainter::CompositionMode.
     *
     * \see getCompositionMode()
     */
    static Qgis::BlendMode getBlendModeEnum( QPainter::CompositionMode blendMode );

    /**
     * Returns TRUE if \a mode is a clipping blend mode.
     *
     * \since QGIS 3.30
     */
    static bool isClippingMode( Qgis::BlendMode mode );

    /**
     * Calculates the QTransform which maps the triangle defined by the points (\a inX1, \a inY1), (\a inY2, \a inY2), (\a inX3, \a inY3)
     * to the triangle defined by (\a outX1, \a outY1), (\a outY2, \a outY2), (\a outX3, \a outY3).
     *
     * \param inX1 source triangle vertex 1 x-coordinate
     * \param inY1 source triangle vertex 1 y-coordinate
     * \param inX2 source triangle vertex 2 x-coordinate
     * \param inY2 source triangle vertex 2 y-coordinate
     * \param inX3 source triangle vertex 3 x-coordinate
     * \param inY3 source triangle vertex 3 y-coordinate
     * \param outX1 destination triangle vertex 1 x-coordinate
     * \param outY1 destination triangle vertex 1 y-coordinate
     * \param outX2 destination triangle vertex 2 x-coordinate
     * \param outY2 destination triangle vertex 2 y-coordinate
     * \param outX3 destination triangle vertex 3 x-coordinate
     * \param outY3 destination triangle vertex 3 y-coordinate
     * \param ok will be set to TRUE if the transform could be determined.
     *
     * \returns Calculated transform (if possible)
     */
    static QTransform triangleToTriangleTransform( double inX1, double inY1, double inX2, double inY2, double inX3, double inY3, double outX1, double outY1, double outX2, double outY2, double outX3, double outY3, bool &ok SIP_OUT );

};

#endif // QGSPAINTING_H
