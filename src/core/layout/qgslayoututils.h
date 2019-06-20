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
#include "qgslayoutitempage.h"
#include <QFont>
#include <QColor>

class QgsRenderContext;
class QgsLayout;
class QgsLayoutItemMap;
class QPainter;
class QRectF;
class QStyleOptionGraphicsItem;

/**
 * \ingroup core
 * Utilities for layouts.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutUtils
{
  public:

    /**
     * Rotates a point / vector around the origin.
     * \param angle rotation angle in degrees, counterclockwise
     * \param x in/out: x coordinate before / after the rotation
     * \param y in/out: y coordinate before / after the rotation
     */
    static void rotate( double angle, double &x, double &y );

    /**
     * Ensures that an \a angle (in degrees) is in the range 0 <= angle < 360.
     * If \a allowNegative is TRUE then angles between (-360, 360) are allowed. If FALSE,
     * angles are converted to positive angles in the range [0, 360).
     */
    static double normalizedAngle( double angle, bool allowNegative = false );

    /**
     * Snaps an \a angle (in degrees) to its closest 45 degree angle.
     * \returns angle snapped to 0, 45/90/135/180/225/270 or 315 degrees
     */
    static double snappedAngle( double angle );

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
    static double relativePosition( double position, double beforeMin, double beforeMax, double afterMin, double afterMax );

    /**
     * Returns a \a font where size is set in points and the size has been upscaled with FONT_WORKAROUND_SCALE
     * to workaround QT font rendering bugs.
     * Returns a font with size set in pixels.
     */
    static QFont scaledFontPixelSize( const QFont &font );

    /**
     * Calculates a \a font ascent in millimeters, including workarounds for QT font rendering issues.
     * \see fontDescentMM()
     * \see fontHeightMM()
     * \see fontHeightCharacterMM()
     * \see textWidthMM()
     */
    static double fontAscentMM( const QFont &font );

    /**
     * Calculate a \a font descent in millimeters, including workarounds for QT font rendering issues.
     * \see fontAscentMM()
     * \see fontHeightMM()
     * \see fontHeightCharacterMM()
     * \see textWidthMM()
     */
    static double fontDescentMM( const QFont &font );

    /**
     * Calculate a \a font height in millimeters, including workarounds for QT font rendering issues.
     * The font height is the font ascent + descent + 1 (for the baseline).
     * \see fontAscentMM()
     * \see fontDescentMM()
     * \see fontHeightCharacterMM()
     * \see textWidthMM()
     */
    static double fontHeightMM( const QFont &font );

    /**
     * Calculate a \a font height in millimeters of a single \a character, including workarounds for QT font
     * rendering issues.
     * \see fontAscentMM()
     * \see fontDescentMM()
     * \see fontHeightMM()
     * \see textWidthMM()
     */
    static double fontHeightCharacterMM( const QFont &font, QChar character );

    /**
     * Calculate a \a font width in millimeters for a \a text string, including workarounds for QT font
     * rendering issues.
     * \see fontAscentMM()
     * \see fontDescentMM()
     * \see fontHeightMM()
     * \see fontHeightCharacterMM()
     * \see textHeightMM()
     */
    static double textWidthMM( const QFont &font, const QString &text );

    /**
     * Calculate a \a font height in millimeters for a \a text string, including workarounds for QT font
     * rendering issues. Note that this method uses a non-standard measure of text height,
     * where only the font ascent is considered for the first line of text.
     *
     * The \a multiLineHeight parameter specifies the line spacing factor.
     *
     * \see textWidthMM()
     */
    static double textHeightMM( const QFont &font, const QString &text, double multiLineHeight = 1.0 );

    /**
     * Draws \a text on a \a painter at a specific \a position, taking care of layout specific issues (calculation to pixel,
     * scaling of font and painter to work around Qt font bugs).
     *
     * If \a color is specified, text will be rendered in that color. If not specified, the current painter pen
     * color will be used instead.
     */
    static void drawText( QPainter *painter, QPointF position, const QString &text, const QFont &font, const QColor &color = QColor() );

    /**
     * Draws \a text on a \a painter within a \a rectangle, taking care of layout specific issues (calculation to pixel,
     * scaling of font and painter to work around Qt font bugs).
     *
     * If \a color is specified, text will be rendered in that color. If not specified, the current painter pen
     * color will be used instead.
     *
     * The text alignment within \a rectangle can be set via the \a halignment and \a valignment
     * arguments.
     *
     * The \a flags parameter allows for passing Qt::TextFlags to control appearance of rendered text.
     */
    static void drawText( QPainter *painter, const QRectF &rectangle, const QString &text, const QFont &font, const QColor &color = QColor(), Qt::AlignmentFlag halignment = Qt::AlignLeft, Qt::AlignmentFlag valignment = Qt::AlignTop, int flags = Qt::TextWordWrap );

    /**
     * Calculates the largest scaled version of \a originalRect which fits within \a boundsRect, when it is rotated by
     * the a specified \a rotation amount.
     * \param originalRect QRectF to be rotated and scaled
     * \param boundsRect QRectF specifying the bounds which the rotated and scaled rectangle must fit within
     * \param rotation the rotation in degrees to be applied to the rectangle
     * \returns largest scaled version of the rectangle possible
     */
    static QRectF largestRotatedRectWithinBounds( const QRectF &originalRect, const QRectF &boundsRect, double rotation );

    /**
     * Decodes a \a string representing a paper orientation and returns the
     * decoded orientation.
     * If the string was correctly decoded, \a ok will be set to TRUE.
     */
    static QgsLayoutItemPage::Orientation decodePaperOrientation( const QString &string, bool &ok );

    /**
     * Extracts the scale factor from an item \a style.
     */
    static double scaleFactorFromItemStyle( const QStyleOptionGraphicsItem *style );

    /**
     * Resolves a \a string into a map layer from a given \a project. Attempts different
     * forms of layer matching such as matching by layer id or layer name.
     *
     * Layer names are matched using a case-insensitive check, ONLY if an exact case
     * match was not found.
     *
     * \since QGIS 3.2
     */
    static QgsMapLayer *mapLayerFromString( const QString &string, QgsProject *project );

  private:

    //! Scale factor for upscaling fontsize and downscaling painter
    static constexpr double FONT_WORKAROUND_SCALE = 10;

    /**
     * Returns the size in mm corresponding to a font \a pointSize.
     * \see mmToPoints()
     */
    static double pointsToMM( double pointSize );

    /**
     * Returns the size in points corresponding to a font \a mmSize in mm.
     * \see pointsToMM()
     */
    static double mmToPoints( double mmSize );

    friend class TestQgsLayoutUtils;
};

#endif //QGSLAYOUTUTILS_H
