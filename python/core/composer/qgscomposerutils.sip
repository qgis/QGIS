/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerutils.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsComposerUtils
{
%Docstring
 Utilities for compositions.
%End

%TypeHeaderCode
#include "qgscomposerutils.h"
%End
  public:

    static void drawArrowHead( QPainter *p, const double x, const double y, const double angle, const double arrowHeadWidth );
%Docstring
 Draws an arrow head on to a QPainter.
 \param p destination painter
 \param x x-coordinate of arrow center
 \param y y-coordinate of arrow center
 \param angle angle in degrees which arrow should point toward, measured
 clockwise from pointing vertical upward
 \param arrowHeadWidth size of arrow head
%End

    static double angle( QPointF p1, QPointF p2 );
%Docstring
 Calculates the angle of the line from p1 to p2 (counter clockwise,
 starting from a line from north to south)
 \param p1 start point of line
 \param p2 end point of line
 :return: angle in degrees, clockwise from south
 :rtype: float
%End

    static void rotate( const double angle, double &x, double &y );
%Docstring
 Rotates a point / vector around the origin.
 \param angle rotation angle in degrees, counterclockwise
 \param x in/out: x coordinate before / after the rotation
 \param y in/out: y cooreinate before / after the rotation
%End

    static double normalizedAngle( const double angle );
%Docstring
 Ensures that an angle is in the range 0 <= angle < 360
 \param angle angle in degrees
 :return: equivalent angle within the range [0, 360)
.. seealso:: :py:func:`snappedAngle`
 :rtype: float
%End

    static double snappedAngle( const double angle );
%Docstring
 Snaps an angle to its closest 45 degree angle
 \param angle angle in degrees
 :return: angle snapped to 0, 45/90/135/180/225/270 or 315 degrees
 :rtype: float
%End

    static QRectF largestRotatedRectWithinBounds( const QRectF &originalRect, const QRectF &boundsRect, const double rotation );
%Docstring
 Calculates the largest scaled version of originalRect which fits within boundsRect, when it is rotated by
 a specified amount.
 \param originalRect QRectF to be rotated and scaled
 \param boundsRect QRectF specifying the bounds which the rotated and scaled rectangle must fit within
 \param rotation the rotation in degrees to be applied to the rectangle
 :return: largest scaled version of the rectangle possible
 :rtype: QRectF
%End

    static double pointsToMM( const double pointSize );
%Docstring
 Returns the size in mm corresponding to a font point size
 \param pointSize font size in points
.. seealso:: :py:func:`mmToPoints`
 :rtype: float
%End

    static double mmToPoints( const double mmSize );
%Docstring
 Returns the size in mm corresponding to a font point size
 \param mmSize font size in mm
.. seealso:: :py:func:`pointsToMM`
 :rtype: float
%End

    static void relativeResizeRect( QRectF &rectToResize, const QRectF &boundsBefore, const QRectF &boundsAfter );
%Docstring
 Resizes a QRectF relative to a resized bounding rectangle.
 \param rectToResize QRectF to resize, contained within boundsBefore. The
 rectangle is linearly scaled to retain its relative position and size within
 boundsAfter.
 \param boundsBefore QRectF of bounds before resize
 \param boundsAfter QRectF of bounds after resize
%End

    static double relativePosition( const double position, const double beforeMin, const double beforeMax, const double afterMin, const double afterMax );
%Docstring
 Returns a scaled position given a before and after range
 \param position initial position within before range to scale
 \param beforeMin minimum value in before range
 \param beforeMax maximum value in before range
 \param afterMin minimum value in after range
 \param afterMax maximum value in after range
 :return: position scaled to range specified by afterMin and afterMax
 :rtype: float
%End

    static QgsComposition::PaperOrientation decodePaperOrientation( const QString &orientationString, bool &ok );
%Docstring
 Decodes a string representing a paper orientation
 \param orientationString string to decode
 \param ok will be true if string could be decoded
 :return: decoded paper orientation
 :rtype: QgsComposition.PaperOrientation
%End

    static bool decodePresetPaperSize( const QString &presetString, double &width, double &height );
%Docstring
 Decodes a string representing a preset page size
 \param presetString string to decode
 \param width double for decoded paper width
 \param height double for decoded paper height
 :return: true if string could be decoded successfully
 :rtype: bool
%End

    static void readOldDataDefinedPropertyMap( const QDomElement &itemElem,
        QgsPropertyCollection &dataDefinedProperties );
%Docstring
 Reads all pre 3.0 data defined properties from an XML element.
.. versionadded:: 3.0
.. seealso:: :py:func:`readDataDefinedProperty`
.. seealso:: :py:func:`writeDataDefinedPropertyMap`
%End

    static QgsProperty readOldDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property, const QDomElement &ddElem );
%Docstring
 Reads a pre 3.0 data defined property from an XML DOM element.
.. versionadded:: 3.0
.. seealso:: :py:func:`readDataDefinedPropertyMap`
 :rtype: QgsProperty
%End

    static QFont scaledFontPixelSize( const QFont &font );
%Docstring
 Returns a font where size is set in pixels and the size has been upscaled with FONT_WORKAROUND_SCALE
 to workaround QT font rendering bugs
 \param font source font with size set in points
 :return: font with size set in pixels
.. versionadded:: 2.5
 :rtype: QFont
%End

    static double fontAscentMM( const QFont &font );
%Docstring
 Calculate font ascent in millimeters, including workarounds for QT font rendering issues
 \param font input font
 :return: font ascent in millimeters
.. versionadded:: 2.5
.. seealso:: :py:func:`fontDescentMM`
.. seealso:: :py:func:`fontHeightMM`
.. seealso:: :py:func:`fontHeightCharacterMM`
.. seealso:: :py:func:`textWidthMM`
 :rtype: float
%End

    static double fontDescentMM( const QFont &font );
%Docstring
 Calculate font descent in millimeters, including workarounds for QT font rendering issues
 \param font input font
 :return: font descent in millimeters
.. versionadded:: 2.5
.. seealso:: :py:func:`fontAscentMM`
.. seealso:: :py:func:`fontHeightMM`
.. seealso:: :py:func:`fontHeightCharacterMM`
.. seealso:: :py:func:`textWidthMM`
 :rtype: float
%End

    static double fontHeightMM( const QFont &font );
%Docstring
 Calculate font height in millimeters, including workarounds for QT font rendering issues
 The font height is the font ascent + descent + 1 (for the baseline).
 \param font input font
 :return: font height in millimeters
.. versionadded:: 2.5
.. seealso:: :py:func:`fontAscentMM`
.. seealso:: :py:func:`fontDescentMM`
.. seealso:: :py:func:`fontHeightCharacterMM`
.. seealso:: :py:func:`textWidthMM`
 :rtype: float
%End

    static double fontHeightCharacterMM( const QFont &font, QChar character );
%Docstring
 Calculate font height in millimeters of a single character, including workarounds for QT font
 rendering issues
 \param font input font
 \param character character to calculate height for
 :return: character height in millimeters
.. versionadded:: 2.5
.. seealso:: :py:func:`fontAscentMM`
.. seealso:: :py:func:`fontDescentMM`
.. seealso:: :py:func:`fontHeightMM`
.. seealso:: :py:func:`textWidthMM`
 :rtype: float
%End

    static double textWidthMM( const QFont &font, const QString &text );
%Docstring
 Calculate font width in millimeters for a string, including workarounds for QT font
 rendering issues
 \param font input font
 \param text string to calculate width of
 :return: string width in millimeters
.. versionadded:: 2.5
.. seealso:: :py:func:`fontAscentMM`
.. seealso:: :py:func:`fontDescentMM`
.. seealso:: :py:func:`fontHeightMM`
.. seealso:: :py:func:`fontHeightCharacterMM`
.. seealso:: :py:func:`textHeightMM`
 :rtype: float
%End

    static double textHeightMM( const QFont &font, const QString &text, double multiLineHeight = 1.0 );
%Docstring
 Calculate font height in millimeters for a string, including workarounds for QT font
 rendering issues. Note that this method uses a non-standard measure of text height,
 where only the font ascent is considered for the first line of text.
 \param font input font
 \param text string to calculate height of
 \param multiLineHeight line spacing factor
 :return: string height in millimeters
.. versionadded:: 2.12
.. seealso:: :py:func:`textWidthMM`
 :rtype: float
%End

    static void drawText( QPainter *painter, QPointF pos, const QString &text, const QFont &font, const QColor &color = QColor() );
%Docstring
 Draws text on a painter at a specific position, taking care of composer specific issues (calculation to pixel,
 scaling of font and painter to work around Qt font bugs)
 \param painter destination QPainter
 \param pos position to draw text
 \param text string to draw
 \param font font to use for drawing text
 \param color color to draw text
.. versionadded:: 2.5
%End

    static void drawText( QPainter *painter, const QRectF &rect, const QString &text, const QFont &font, const QColor &color = QColor(), const Qt::AlignmentFlag halignment = Qt::AlignLeft, const Qt::AlignmentFlag valignment = Qt::AlignTop, const int flags = Qt::TextWordWrap );
%Docstring
 Draws text on a painter within a rectangle, taking care of composer specific issues (calculation to pixel,
 scaling of font and painter to work around Qt font bugs)
 \param painter destination QPainter
 \param rect rectangle to draw into
 \param text string to draw
 \param font font to use for drawing text
 \param color color to draw text
 \param halignment optional horizontal alignment
 \param valignment optional vertical alignment
 \param flags allows for passing Qt.TextFlags to control appearance of rendered text
.. versionadded:: 2.5
%End

    static QgsRenderContext createRenderContextForMap( QgsComposerMap *map, QPainter *painter, double dpi = -1 );
%Docstring
 Creates a render context suitable for the specified composer ``map`` and ``painter`` destination.
 This method returns a new QgsRenderContext which matches the scale and settings of the
 target map. If the ``dpi`` argument is not specified then the dpi will be taken from the destinatation
 painter device.
.. versionadded:: 3.0
.. seealso:: :py:func:`createRenderContextForComposition()`
 :rtype: QgsRenderContext
%End

    static QgsRenderContext createRenderContextForComposition( QgsComposition *composition, QPainter *painter );
%Docstring
 Creates a render context suitable for the specified ``composition`` and ``painter`` destination.
 This method returns a new QgsRenderContext which matches the scale and settings from the composition's
 QgsComposition.referenceMap().
.. versionadded:: 3.0
.. seealso:: :py:func:`createRenderContextForMap()`
 :rtype: QgsRenderContext
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerutils.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
