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

    /** Draws an arrow head on to a QPainter.
     * @param p destination painter
     * @param x x-coordinate of arrow center
     * @param y y-coordinate of arrow center
     * @param angle angle in degrees which arrow should point toward, measured
     * clockwise from pointing vertical upward
     * @param arrowHeadWidth size of arrow head
    */
    static void drawArrowHead( QPainter* p, const double x, const double y, const double angle, const double arrowHeadWidth );

    /** Calculates the angle of the line from p1 to p2 (counter clockwise,
     * starting from a line from north to south)
     * @param p1 start point of line
     * @param p2 end point of line
     * @returns angle in degrees, clockwise from south
    */
    static double angle( const QPointF& p1, const QPointF& p2 );

    /** Rotates a point / vector around the origin.
     * @param angle rotation angle in degrees, counterclockwise
     * @param x in/out: x coordinate before / after the rotation
     * @param y in/out: y cooreinate before / after the rotation
    */
    static void rotate( const double angle, double& x, double& y );

    /** Ensures that an angle is in the range 0 <= angle < 360
     * @param angle angle in degrees
     * @returns equivalent angle within the range [0, 360)
     * @see snappedAngle
    */
    static double normalizedAngle( const double angle );

    /** Snaps an angle to its closest 45 degree angle
     * @param angle angle in degrees
     * @returns angle snapped to 0, 45/90/135/180/225/270 or 315 degrees
    */
    static double snappedAngle( const double angle );

    /** Calculates the largest scaled version of originalRect which fits within boundsRect, when it is rotated by
     * a specified amount.
     * @param originalRect QRectF to be rotated and scaled
     * @param boundsRect QRectF specifying the bounds which the rotated and scaled rectangle must fit within
     * @param rotation the rotation in degrees to be applied to the rectangle
     * @returns largest scaled version of the rectangle possible
    */
    static QRectF largestRotatedRectWithinBounds( const QRectF originalRect, const QRectF boundsRect, const double rotation );

    /** Returns the size in mm corresponding to a font point size
     * @param pointSize font size in points
     * @see mmToPoints
    */
    static double pointsToMM( const double pointSize );

    /** Returns the size in mm corresponding to a font point size
     * @param mmSize font size in mm
     * @see pointsToMM
    */
    static double mmToPoints( const double mmSize );

    /** Resizes a QRectF relative to a resized bounding rectangle.
     * @param rectToResize QRectF to resize, contained within boundsBefore. The
     * rectangle is linearly scaled to retain its relative position and size within
     * boundsAfter.
     * @param boundsBefore QRectF of bounds before resize
     * @param boundsAfter QRectF of bounds after resize
    */
    static void relativeResizeRect( QRectF &rectToResize, const QRectF &boundsBefore, const QRectF &boundsAfter );

    /** Returns a scaled position given a before and after range
     * @param position initial position within before range to scale
     * @param beforeMin minimum value in before range
     * @param beforeMax maximum value in before range
     * @param afterMin minimum value in after range
     * @param afterMax maximum value in after range
     * @returns position scaled to range specified by afterMin and afterMax
    */
    static double relativePosition( const double position, const double beforeMin, const double beforeMax, const double afterMin, const double afterMax );

    /** Decodes a string representing a paper orientation
     * @param orientationString string to decode
     * @param ok will be true if string could be decoded
     * @returns decoded paper orientation
    */
    static QgsComposition::PaperOrientation decodePaperOrientation( const QString orientationString, bool &ok );

    /** Decodes a string representing a preset page size
     * @param presetString string to decode
     * @param width double for decoded paper width
     * @param height double for decoded paper height
     * @returns true if string could be decoded successfully
    */
    static bool decodePresetPaperSize( const QString presetString, double &width, double &height );

    /** Reads all data defined properties from xml
     * @param itemElem dom element containing data defined properties
     * @param dataDefinedNames map of data defined property to name used within xml
     * @param dataDefinedProperties map of data defined properties to QgsDataDefined in which to store properties from xml
     * @note this method was added in version 2.5
     * @see readDataDefinedProperty
     * @see writeDataDefinedPropertyMap
    */
    static void readDataDefinedPropertyMap( const QDomElement &itemElem,
                                            QMap< QgsComposerObject::DataDefinedProperty, QString >* dataDefinedNames,
                                            QMap< QgsComposerObject::DataDefinedProperty, QgsDataDefined* >* dataDefinedProperties
                                          );

    /** Reads a single data defined property from xml DOM element
     * @param property data defined property to read
     * @param ddElem dom element containing settings for data defined property
     * @param dataDefinedProperties map of data defined properties to QgsDataDefined in which to store properties from xml
     * @note this method was added in version 2.5
     * @see readDataDefinedPropertyMap
    */
    static void readDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property, const QDomElement &ddElem,
                                         QMap< QgsComposerObject::DataDefinedProperty, QgsDataDefined* >* dataDefinedProperties );

    /** Writes data defined properties to xml
     * @param itemElem DOM element in which to store data defined properties
     * @param doc DOM document
     * @param dataDefinedNames map of data defined property to name used within xml
     * @param dataDefinedProperties map of data defined properties to QgsDataDefined for storing in xml
     * @note this method was added in version 2.5
     * @see readDataDefinedPropertyMap
    */
    static void writeDataDefinedPropertyMap( QDomElement &itemElem, QDomDocument &doc,
        const QMap< QgsComposerObject::DataDefinedProperty, QString >* dataDefinedNames,
        const QMap< QgsComposerObject::DataDefinedProperty, QgsDataDefined* >* dataDefinedProperties );

    /** Returns a font where size is set in pixels and the size has been upscaled with FONT_WORKAROUND_SCALE
     * to workaround QT font rendering bugs
     * @param font source font with size set in points
     * @returns font with size set in pixels
     * @note added in version 2.5
    */
    static QFont scaledFontPixelSize( const QFont& font );

    /** Calculate font ascent in millimeters, including workarounds for QT font rendering issues
     * @param font input font
     * @returns font ascent in millimeters
     * @note added in version 2.5
     * @see fontDescentMM
     * @see fontHeightMM
     * @see fontHeightCharacterMM
     * @see textWidthMM
     */
    static double fontAscentMM( const QFont& font );

    /** Calculate font descent in millimeters, including workarounds for QT font rendering issues
     * @param font input font
     * @returns font descent in millimeters
     * @note added in version 2.5
     * @see fontAscentMM
     * @see fontHeightMM
     * @see fontHeightCharacterMM
     * @see textWidthMM
     */
    static double fontDescentMM( const QFont& font );

    /** Calculate font height in millimeters, including workarounds for QT font rendering issues
     * The font height is the font ascent + descent + 1 (for the baseline).
     * @param font input font
     * @returns font height in millimeters
     * @note added in version 2.5
     * @see fontAscentMM
     * @see fontDescentMM
     * @see fontHeightCharacterMM
     * @see textWidthMM
     */
    static double fontHeightMM( const QFont& font );

    /** Calculate font height in millimeters of a single character, including workarounds for QT font
     * rendering issues
     * @param font input font
     * @param character character to calculate height for
     * @returns character height in millimeters
     * @note added in version 2.5
     * @see fontAscentMM
     * @see fontDescentMM
     * @see fontHeightMM
     * @see textWidthMM
     */
    static double fontHeightCharacterMM( const QFont& font, const QChar& character );

    /** Calculate font width in millimeters for a string, including workarounds for QT font
     * rendering issues
     * @param font input font
     * @param text string to calculate width of
     * @returns string width in millimeters
     * @note added in version 2.5
     * @see fontAscentMM
     * @see fontDescentMM
     * @see fontHeightMM
     * @see fontHeightCharacterMM
     * @see textHeightMM
     */
    static double textWidthMM( const QFont& font, const QString& text );

    /** Calculate font height in millimeters for a string, including workarounds for QT font
     * rendering issues. Note that this method uses a non-standard measure of text height,
     * where only the font ascent is considered for the first line of text.
     * @param font input font
     * @param text string to calculate height of
     * @param multiLineHeight line spacing factor
     * @returns string height in millimeters
     * @note added in version 2.12
     * @see textWidthMM
     */
    static double textHeightMM( const QFont& font, const QString& text, double multiLineHeight = 1.0 );

    /** Draws text on a painter at a specific position, taking care of composer specific issues (calculation to pixel,
     * scaling of font and painter to work around Qt font bugs)
     * @param painter destination QPainter
     * @param pos position to draw text
     * @param text string to draw
     * @param font font to use for drawing text
     * @param color color to draw text
     * @note added in version 2.5
     */
    static void drawText( QPainter* painter, const QPointF& pos, const QString& text, const QFont& font, const QColor& color = QColor() );

    /** Draws text on a painter within a rectangle, taking care of composer specific issues (calculation to pixel,
     * scaling of font and painter to work around Qt font bugs)
     * @param painter destination QPainter
     * @param rect rectangle to draw into
     * @param text string to draw
     * @param font font to use for drawing text
     * @param color color to draw text
     * @param halignment optional horizontal alignment
     * @param valignment optional vertical alignment
     * @param flags allows for passing Qt::TextFlags to control appearance of rendered text
     * @note added in version 2.5
     */
    static void drawText( QPainter* painter, const QRectF& rect, const QString& text, const QFont& font, const QColor& color = QColor(), const Qt::AlignmentFlag halignment = Qt::AlignLeft, const Qt::AlignmentFlag valignment = Qt::AlignTop, const int flags = Qt::TextWordWrap );

};

#endif
