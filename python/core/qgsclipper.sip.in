/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsclipper.h                                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






%Feature ARM // Some parts are not available in sip bindings on ARM because of qreal double vs. float issues


class QgsClipper
{
%Docstring
A class to trim lines and polygons to within a rectangular region.
The functions in this class are likely to be called from within a
render loop and hence need to as CPU efficient as possible.
The main purpose of the functions in this class are to trim lines
and polygons to lie within a rectangular region. This is necessary
for drawing items to an X11 display which have a limit on the
magnitude of the screen coordinates (+/- 32768, i.e. 16 bit integer).
%End

%TypeHeaderCode
#include "qgsclipper.h"
%End
  public:



    static const double MAX_X;
    static const double MIN_X;
    static const double MAX_Y;
    static const double MIN_Y;


    enum Boundary
    {
      XMax,
      XMin,
      YMax,
      YMin
    };

%If (!ARM) // Not available on ARM sip bindings because of qreal issues

    static void trimFeature( QVector<double> &x,
                             QVector<double> &y,
                             bool shapeOpen );
%Docstring
Trims the given feature to a rectangular box. Returns the trimmed
feature in x and y. The shapeOpen parameter determines whether
the function treats the points as a closed shape (polygon), or as
an open shape (linestring).

.. note::

   not available in Python bindings on android
%End

%End

    static void trimPolygon( QPolygonF &pts, const QgsRectangle &clipRect );

    static QPolygonF clippedLine( const QgsCurve &curve, const QgsRectangle &clipExtent );
%Docstring
Takes a linestring and clips it to clipExtent

:param curve: the linestring
:param clipExtent: clipping bounds

:return: clipped line coordinates
%End

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsclipper.h                                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
