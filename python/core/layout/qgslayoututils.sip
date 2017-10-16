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

    static double normalizedAngle( const double angle, const bool allowNegative = false );
%Docstring
 Ensures that an ``angle`` (in degrees) is in the range 0 <= angle < 360.
 If ``allowNegative`` is true then angles between (-360, 360) are allowed. If false,
 angles are converted to positive angles in the range [0, 360).
 :rtype: float
%End

    static QgsRenderContext createRenderContextForMap( QgsLayoutItemMap *map, QPainter *painter, double dpi = -1 );
%Docstring
 Creates a render context suitable for the specified layout ``map`` and ``painter`` destination.
 This method returns a new QgsRenderContext which matches the scale and settings of the
 target map. If the ``dpi`` argument is not specified then the dpi will be taken from the destination
 painter device.
.. seealso:: createRenderContextForLayout()
 :rtype: QgsRenderContext
%End

    static QgsRenderContext createRenderContextForLayout( QgsLayout *layout, QPainter *painter, double dpi = -1 );
%Docstring
 Creates a render context suitable for the specified ``layout`` and ``painter`` destination.
 This method returns a new QgsRenderContext which matches the scale and settings from the layout's
 QgsLayout.referenceMap().
 If the ``dpi`` argument is not specified then the dpi will be taken from the destination
 painter device.
.. seealso:: createRenderContextForMap()
 :rtype: QgsRenderContext
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


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoututils.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
