/***************************************************************************
                         qgscomposerutils.h
                             -------------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson
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
#ifndef QGSCOMPOSERUTILS_H
#define QGSCOMPOSERUTILS_H

#include <QPointF>
#include <QRectF>

class QPainter;

/** \ingroup MapComposer
 * Utilities for compositions.
 */
class CORE_EXPORT QgsComposerUtils
{
  public:

    /**Draws an arrow head on to a QPainter.
     * @param p destination painter
     * @param x x-coordinate of arrow center
     * @param y y-coordinate of arrow center
     * @param angle angle in degrees which arrow should point toward, measured
     * clockwise from pointing vertical upward
     * @param arrowHeadWidth size of arrow head
    */
    static void drawArrowHead( QPainter* p, const double x, const double y, const double angle, const double arrowHeadWidth );

    /**Calculates the angle of the line from p1 to p2 (counter clockwise,
     * starting from a line from north to south)
     * @param p1 start point of line
     * @param p2 end point of line
     * @returns angle in degrees, clockwise from south
    */
    static double angle( const QPointF& p1, const QPointF& p2 );

    /**Rotates a point / vector around the origin.
     * @param angle rotation angle in degrees, counterclockwise
     * @param x in/out: x coordinate before / after the rotation
     * @param y in/out: y cooreinate before / after the rotation
    */
    static void rotate( double angle, double& x, double& y );

    /**Calculates the largest scaled version of originalRect which fits within boundsRect, when it is rotated by
     * a specified amount.
     * @param originalRect QRectF to be rotated and scaled
     * @param boundsRect QRectF specifying the bounds which the rotated and scaled rectangle must fit within
     * @param rotation the rotation in degrees to be applied to the rectangle
     * @returns largest scaled version of the rectangle possible
    */
    static QRectF largestRotatedRectWithinBounds( const QRectF originalRect, const QRectF boundsRect, const double rotation );
};

#endif
