/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoututils.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLayoutUtils
{
%Docstring
Utilities for layouts.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoututils.h"
%End
  public:

    static void rotate( double angle, double &x, double &y );
%Docstring
Rotates a point / vector around the origin.

:param angle: rotation angle in degrees, counterclockwise
:param x: in/out: x coordinate before / after the rotation
:param y: in/out: y coordinate before / after the rotation
%End

    static double normalizedAngle( const double angle, const bool allowNegative = false );
%Docstring
Ensures that an ``angle`` (in degrees) is in the range 0 <= angle < 360.
If ``allowNegative`` is true then angles between (-360, 360) are allowed. If false,
angles are converted to positive angles in the range [0, 360).
%End

    static double snappedAngle( double angle );
%Docstring
Snaps an ``angle`` (in degrees) to its closest 45 degree angle.

:return: angle snapped to 0, 45/90/135/180/225/270 or 315 degrees
%End

    static QgsRenderContext createRenderContextForMap( QgsLayoutItemMap *map, QPainter *painter, double dpi = -1 );
%Docstring
Creates a render context suitable for the specified layout ``map`` and ``painter`` destination.
This method returns a new QgsRenderContext which matches the scale and settings of the
target map. If the ``dpi`` argument is not specified then the dpi will be taken from the destination
painter device.

.. seealso:: :py:func:`createRenderContextForLayout`
%End

    static QgsRenderContext createRenderContextForLayout( QgsLayout *layout, QPainter *painter, double dpi = -1 );
%Docstring
Creates a render context suitable for the specified ``layout`` and ``painter`` destination.
This method returns a new QgsRenderContext which matches the scale and settings from the layout's
:py:func:`QgsLayout.referenceMap()`
If the ``dpi`` argument is not specified then the dpi will be taken from the destination
painter device.

.. seealso:: :py:func:`createRenderContextForMap`
%End

    static void relativeResizeRect( QRectF &rectToResize, const QRectF &boundsBefore, const QRectF &boundsAfter );
%Docstring
Resizes a QRectF relative to a resized bounding rectangle.

:param rectToResize: QRectF to resize, contained within boundsBefore. The
rectangle is linearly scaled to retain its relative position and size within
boundsAfter.
:param boundsBefore: QRectF of bounds before resize
:param boundsAfter: QRectF of bounds after resize
%End

    static double relativePosition( const double position, const double beforeMin, const double beforeMax, const double afterMin, const double afterMax );
%Docstring
Returns a scaled position given a before and after range

:param position: initial position within before range to scale
:param beforeMin: minimum value in before range
:param beforeMax: maximum value in before range
:param afterMin: minimum value in after range
:param afterMax: maximum value in after range

:return: position scaled to range specified by afterMin and afterMax
%End

    static QFont scaledFontPixelSize( const QFont &font );
%Docstring
Returns a ``font`` where size is set in points and the size has been upscaled with FONT_WORKAROUND_SCALE
to workaround QT font rendering bugs.
Returns a font with size set in pixels.
%End

    static double fontAscentMM( const QFont &font );
%Docstring
Calculates a ``font`` ascent in millimeters, including workarounds for QT font rendering issues.

.. seealso:: :py:func:`fontDescentMM`

.. seealso:: :py:func:`fontHeightMM`

.. seealso:: :py:func:`fontHeightCharacterMM`

.. seealso:: :py:func:`textWidthMM`
%End

    static double fontDescentMM( const QFont &font );
%Docstring
Calculate a ``font`` descent in millimeters, including workarounds for QT font rendering issues.

.. seealso:: :py:func:`fontAscentMM`

.. seealso:: :py:func:`fontHeightMM`

.. seealso:: :py:func:`fontHeightCharacterMM`

.. seealso:: :py:func:`textWidthMM`
%End

    static double fontHeightMM( const QFont &font );
%Docstring
Calculate a ``font`` height in millimeters, including workarounds for QT font rendering issues.
The font height is the font ascent + descent + 1 (for the baseline).

.. seealso:: :py:func:`fontAscentMM`

.. seealso:: :py:func:`fontDescentMM`

.. seealso:: :py:func:`fontHeightCharacterMM`

.. seealso:: :py:func:`textWidthMM`
%End

    static double fontHeightCharacterMM( const QFont &font, QChar character );
%Docstring
Calculate a ``font`` height in millimeters of a single ``character``, including workarounds for QT font
rendering issues.

.. seealso:: :py:func:`fontAscentMM`

.. seealso:: :py:func:`fontDescentMM`

.. seealso:: :py:func:`fontHeightMM`

.. seealso:: :py:func:`textWidthMM`
%End

    static double textWidthMM( const QFont &font, const QString &text );
%Docstring
Calculate a ``font`` width in millimeters for a ``text`` string, including workarounds for QT font
rendering issues.

.. seealso:: :py:func:`fontAscentMM`

.. seealso:: :py:func:`fontDescentMM`

.. seealso:: :py:func:`fontHeightMM`

.. seealso:: :py:func:`fontHeightCharacterMM`

.. seealso:: :py:func:`textHeightMM`
%End

    static double textHeightMM( const QFont &font, const QString &text, double multiLineHeight = 1.0 );
%Docstring
Calculate a ``font`` height in millimeters for a ``text`` string, including workarounds for QT font
rendering issues. Note that this method uses a non-standard measure of text height,
where only the font ascent is considered for the first line of text.

The ``multiLineHeight`` parameter specifies the line spacing factor.

.. seealso:: :py:func:`textWidthMM`
%End

    static void drawText( QPainter *painter, QPointF position, const QString &text, const QFont &font, const QColor &color = QColor() );
%Docstring
Draws ``text`` on a ``painter`` at a specific ``position``, taking care of layout specific issues (calculation to pixel,
scaling of font and painter to work around Qt font bugs).

If ``color`` is specified, text will be rendered in that color. If not specified, the current painter pen
color will be used instead.
%End

    static void drawText( QPainter *painter, const QRectF &rectangle, const QString &text, const QFont &font, const QColor &color = QColor(), const Qt::AlignmentFlag halignment = Qt::AlignLeft, const Qt::AlignmentFlag valignment = Qt::AlignTop, const int flags = Qt::TextWordWrap );
%Docstring
Draws ``text`` on a ``painter`` within a ``rectangle``, taking care of layout specific issues (calculation to pixel,
scaling of font and painter to work around Qt font bugs).

If ``color`` is specified, text will be rendered in that color. If not specified, the current painter pen
color will be used instead.

The text alignment within ``rectangle`` can be set via the ``halignment`` and ``valignment``
arguments.

The ``flags`` parameter allows for passing Qt.TextFlags to control appearance of rendered text.
%End

    static QRectF largestRotatedRectWithinBounds( const QRectF &originalRect, const QRectF &boundsRect, const double rotation );
%Docstring
Calculates the largest scaled version of ``originalRect`` which fits within ``boundsRect``, when it is rotated by
the a specified ``rotation`` amount.

:param originalRect: QRectF to be rotated and scaled
:param boundsRect: QRectF specifying the bounds which the rotated and scaled rectangle must fit within
:param rotation: the rotation in degrees to be applied to the rectangle

:return: largest scaled version of the rectangle possible
%End

    static QgsLayoutItemPage::Orientation decodePaperOrientation( const QString &string, bool &ok );
%Docstring
Decodes a ``string`` representing a paper orientation and returns the
decoded orientation.
If the string was correctly decoded, ``ok`` will be set to true.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoututils.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
