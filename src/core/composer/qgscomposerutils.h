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

#include "qgscomposition.h" //for page size and orientation enums
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
    static void rotate( const double angle, double& x, double& y );

    /**Calculates the largest scaled version of originalRect which fits within boundsRect, when it is rotated by
     * a specified amount.
     * @param originalRect QRectF to be rotated and scaled
     * @param boundsRect QRectF specifying the bounds which the rotated and scaled rectangle must fit within
     * @param rotation the rotation in degrees to be applied to the rectangle
     * @returns largest scaled version of the rectangle possible
    */
    static QRectF largestRotatedRectWithinBounds( const QRectF originalRect, const QRectF boundsRect, const double rotation );

    /**Returns the size in mm corresponding to a font point size
     * @param pointSize font size in points
     * @see mmToPoints
    */
    static double pointsToMM( const double pointSize );

    /**Returns the size in mm corresponding to a font point size
     * @param mmSize font size in mm
     * @see pointsToMM
    */
    static double mmToPoints( const double mmSize );

    /**Resizes a QRectF relative to a resized bounding rectangle.
     * @param rectToResize QRectF to resize, contained within boundsBefore. The
     * rectangle is linearly scaled to retain its relative position and size within
     * boundsAfter.
     * @param boundsBefore QRectF of bounds before resize
     * @param boundsAfter QRectF of bounds after resize
    */
    static void relativeResizeRect( QRectF &rectToResize, const QRectF &boundsBefore, const QRectF &boundsAfter );

    /**Returns a scaled position given a before and after range*/
    static double relativePosition( const double position, const double beforeMin, const double beforeMax, const double afterMin, const double afterMax );

    /*Decodes a string representing a paper orientation*/
    static QgsComposition::PaperOrientation decodePaperOrientation( const QString orientationString, bool &ok );

    /*Decodes a string representing a preset page size*/
    static bool decodePresetPaperSize( const QString presetString, double &width, double &height );

    /**Reads all data defined properties from xml
     * @param itemElem dom element containing data defined properties
     * @param dataDefinedNames map of data defined property to name used within xml
     * @param dataDefinedProperties map of data defined properties to QgsDataDefined in which to store properties from xml
     * @note this method was added in version 2.5
    */
    static void readDataDefinedPropertyMap( const QDomElement &itemElem,
                                            QMap< QgsComposerObject::DataDefinedProperty, QString >* dataDefinedNames,
                                            QMap< QgsComposerObject::DataDefinedProperty, QgsDataDefined* >* dataDefinedProperties
                                          );

    /**Reads a single data defined property from xml DOM element
     * @param property data defined property to read
     * @param ddElem dom element containing settings for data defined property
     * @param dataDefinedProperties map of data defined properties to QgsDataDefined in which to store properties from xml
     * @note this method was added in version 2.5
    */
    static void readDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property, const QDomElement &ddElem,
                                         QMap< QgsComposerObject::DataDefinedProperty, QgsDataDefined* >* dataDefinedProperties );


    /**Writes data defined properties to xml
     * @param itemElem DOM element in which to store data defined properties
     * @param doc DOM document
     * @param dataDefinedNames map of data defined property to name used within xml
     * @param dataDefinedProperties map of data defined properties to QgsDataDefined for storing in xml
     * @note this method was added in version 2.5
    */
    static void writeDataDefinedPropertyMap( QDomElement &itemElem, QDomDocument &doc,
        const QMap< QgsComposerObject::DataDefinedProperty, QString >* dataDefinedNames,
        const QMap< QgsComposerObject::DataDefinedProperty, QgsDataDefined* >* dataDefinedProperties );

};

#endif
