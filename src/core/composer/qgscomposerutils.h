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

#include "qgis_core.h"
#include "qgscomposition.h" //for page size and orientation enums
#include "qgsrendercontext.h"
#include <QPointF>
#include <QRectF>

class QPainter;

/**
 * \ingroup core
 * Utilities for compositions.
 */
class CORE_EXPORT QgsComposerUtils
{
  public:

    /**
     * Draws an arrow head on to a QPainter.
     * \param p destination painter
     * \param x x-coordinate of arrow center
     * \param y y-coordinate of arrow center
     * \param angle angle in degrees which arrow should point toward, measured
     * clockwise from pointing vertical upward
     * \param arrowHeadWidth size of arrow head
     */
    static void drawArrowHead( QPainter *p, const double x, const double y, const double angle, const double arrowHeadWidth );

    /**
     * Calculates the angle of the line from p1 to p2 (counter clockwise,
     * starting from a line from north to south)
     * \param p1 start point of line
     * \param p2 end point of line
     * \returns angle in degrees, clockwise from south
     */
    static double angle( QPointF p1, QPointF p2 );

    /**
     * Rotates a point / vector around the origin.
     * \param angle rotation angle in degrees, counterclockwise
     * \param x in/out: x coordinate before / after the rotation
     * \param y in/out: y cooreinate before / after the rotation
     */
    static void rotate( const double angle, double &x, double &y );

    /**
     * Ensures that an angle is in the range 0 <= angle < 360
     * \param angle angle in degrees
     * \returns equivalent angle within the range [0, 360)
     * \see snappedAngle
     */
    static double normalizedAngle( const double angle );

    /**
     * Snaps an angle to its closest 45 degree angle
     * \param angle angle in degrees
     * \returns angle snapped to 0, 45/90/135/180/225/270 or 315 degrees
     */
    static double snappedAngle( const double angle );

    /**
     * Calculates the largest scaled version of originalRect which fits within boundsRect, when it is rotated by
     * a specified amount.
     * \param originalRect QRectF to be rotated and scaled
     * \param boundsRect QRectF specifying the bounds which the rotated and scaled rectangle must fit within
     * \param rotation the rotation in degrees to be applied to the rectangle
     * \returns largest scaled version of the rectangle possible
     */
    static QRectF largestRotatedRectWithinBounds( const QRectF &originalRect, const QRectF &boundsRect, const double rotation );

    /**
     * Returns the size in mm corresponding to a font point size
     * \param pointSize font size in points
     * \see mmToPoints
     */
    static double pointsToMM( const double pointSize );

    /**
     * Returns the size in mm corresponding to a font point size
     * \param mmSize font size in mm
     * \see pointsToMM
     */
    static double mmToPoints( const double mmSize );

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
     * Decodes a string representing a paper orientation
     * \param orientationString string to decode
     * \param ok will be true if string could be decoded
     * \returns decoded paper orientation
     */
    static QgsComposition::PaperOrientation decodePaperOrientation( const QString &orientationString, bool &ok );

    /**
     * Decodes a string representing a preset page size
     * \param presetString string to decode
     * \param width double for decoded paper width
     * \param height double for decoded paper height
     * \returns true if string could be decoded successfully
     */
    static bool decodePresetPaperSize( const QString &presetString, double &width, double &height );

    /**
     * Reads all pre 3.0 data defined properties from an XML element.
     * \since QGIS 3.0
     * \see readDataDefinedProperty
     * \see writeDataDefinedPropertyMap
     */
    static void readOldDataDefinedPropertyMap( const QDomElement &itemElem,
        QgsPropertyCollection &dataDefinedProperties );

    /**
     * Reads a pre 3.0 data defined property from an XML DOM element.
     * \since QGIS 3.0
     * \see readDataDefinedPropertyMap
     */
    static QgsProperty readOldDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property, const QDomElement &ddElem );

    /**
     * Returns a font where size is set in pixels and the size has been upscaled with FONT_WORKAROUND_SCALE
     * to workaround QT font rendering bugs
     * \param font source font with size set in points
     * \returns font with size set in pixels
     * \since QGIS 2.5
     */
    static QFont scaledFontPixelSize( const QFont &font );

    /**
     * Calculate font ascent in millimeters, including workarounds for QT font rendering issues
     * \param font input font
     * \returns font ascent in millimeters
     * \since QGIS 2.5
     * \see fontDescentMM
     * \see fontHeightMM
     * \see fontHeightCharacterMM
     * \see textWidthMM
     */
    static double fontAscentMM( const QFont &font );

    /**
     * Calculate font descent in millimeters, including workarounds for QT font rendering issues
     * \param font input font
     * \returns font descent in millimeters
     * \since QGIS 2.5
     * \see fontAscentMM
     * \see fontHeightMM
     * \see fontHeightCharacterMM
     * \see textWidthMM
     */
    static double fontDescentMM( const QFont &font );

    /**
     * Calculate font height in millimeters, including workarounds for QT font rendering issues
     * The font height is the font ascent + descent + 1 (for the baseline).
     * \param font input font
     * \returns font height in millimeters
     * \since QGIS 2.5
     * \see fontAscentMM
     * \see fontDescentMM
     * \see fontHeightCharacterMM
     * \see textWidthMM
     */
    static double fontHeightMM( const QFont &font );

    /**
     * Calculate font height in millimeters of a single character, including workarounds for QT font
     * rendering issues
     * \param font input font
     * \param character character to calculate height for
     * \returns character height in millimeters
     * \since QGIS 2.5
     * \see fontAscentMM
     * \see fontDescentMM
     * \see fontHeightMM
     * \see textWidthMM
     */
    static double fontHeightCharacterMM( const QFont &font, QChar character );

    /**
     * Calculate font width in millimeters for a string, including workarounds for QT font
     * rendering issues
     * \param font input font
     * \param text string to calculate width of
     * \returns string width in millimeters
     * \since QGIS 2.5
     * \see fontAscentMM
     * \see fontDescentMM
     * \see fontHeightMM
     * \see fontHeightCharacterMM
     * \see textHeightMM
     */
    static double textWidthMM( const QFont &font, const QString &text );

    /**
     * Calculate font height in millimeters for a string, including workarounds for QT font
     * rendering issues. Note that this method uses a non-standard measure of text height,
     * where only the font ascent is considered for the first line of text.
     * \param font input font
     * \param text string to calculate height of
     * \param multiLineHeight line spacing factor
     * \returns string height in millimeters
     * \since QGIS 2.12
     * \see textWidthMM
     */
    static double textHeightMM( const QFont &font, const QString &text, double multiLineHeight = 1.0 );

    /**
     * Draws text on a painter at a specific position, taking care of composer specific issues (calculation to pixel,
     * scaling of font and painter to work around Qt font bugs)
     * \param painter destination QPainter
     * \param pos position to draw text
     * \param text string to draw
     * \param font font to use for drawing text
     * \param color color to draw text
     * \since QGIS 2.5
     */
    static void drawText( QPainter *painter, QPointF pos, const QString &text, const QFont &font, const QColor &color = QColor() );

    /**
     * Draws text on a painter within a rectangle, taking care of composer specific issues (calculation to pixel,
     * scaling of font and painter to work around Qt font bugs)
     * \param painter destination QPainter
     * \param rect rectangle to draw into
     * \param text string to draw
     * \param font font to use for drawing text
     * \param color color to draw text
     * \param halignment optional horizontal alignment
     * \param valignment optional vertical alignment
     * \param flags allows for passing Qt::TextFlags to control appearance of rendered text
     * \since QGIS 2.5
     */
    static void drawText( QPainter *painter, const QRectF &rect, const QString &text, const QFont &font, const QColor &color = QColor(), const Qt::AlignmentFlag halignment = Qt::AlignLeft, const Qt::AlignmentFlag valignment = Qt::AlignTop, const int flags = Qt::TextWordWrap );

    /**
     * Creates a render context suitable for the specified composer \a map and \a painter destination.
     * This method returns a new QgsRenderContext which matches the scale and settings of the
     * target map. If the \a dpi argument is not specified then the dpi will be taken from the destinatation
     * painter device.
     * \since QGIS 3.0
     * \see createRenderContextForComposition()
     */
    static QgsRenderContext createRenderContextForMap( QgsComposerMap *map, QPainter *painter, double dpi = -1 );

    /**
     * Creates a render context suitable for the specified \a composition and \a painter destination.
     * This method returns a new QgsRenderContext which matches the scale and settings from the composition's
     * QgsComposition::referenceMap().
     * \since QGIS 3.0
     * \see createRenderContextForMap()
     */
    static QgsRenderContext createRenderContextForComposition( QgsComposition *composition, QPainter *painter );

};

#endif
