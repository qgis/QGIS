/***************************************************************************
                             qgsshapegenerator.h
                             ----------------
    begin                : March 2021
    copyright            : (C) 2021 Nyall Dawson
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

#ifndef QGSSHAPEGENERATOR_H
#define QGSSHAPEGENERATOR_H

#define SIP_NO_FILE
#include "qgis_core.h"

#include <QPolygonF>

class QgsPointXY;
class QPainterPath;

/**
 * \ingroup core
 * \class QgsShapeGenerator
 * \brief Contains utility functions for generating shapes.
 *
 * \note Not available in Python bindings
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsShapeGenerator
{
  public:

    /**
     * Generates a "balloon"/"talking bubble" style shape (as a QPolygonF).
     *
     * The \a origin point indicates the starting point for the pointing wedge portion of the balloon.
     *
     * The \a rect argument specifies the rectangular bounds of the main body of the balloon.
     *
     * The \a wedgeWidth argument gives the width of the pointing wedge portion of the balloon at the
     * position where it joins with the balloon's main body.
     */
    static QPolygonF createBalloon( const QgsPointXY &origin, const QRectF &rect, double wedgeWidth );

    /**
     * Generates a "balloon"/"talking bubble" style shape (as a painter path).
     *
     * The \a origin point indicates the starting point for the pointing wedge portion of the balloon.
     *
     * The \a rect argument specifies the rectangular bounds of the main body of the balloon.
     *
     * The \a wedgeWidth argument gives the width of the pointing wedge portion of the balloon at the
     * position where it joins with the balloon's main body.
     *
     * The \a cornerRadius argument gives the radius for rounding corners on the main bubble rectangle.
     */
    static QPainterPath createBalloon( const QgsPointXY &origin, const QRectF &rect, double wedgeWidth, double cornerRadius );
};

#endif // QGSSHAPEGENERATOR_H
