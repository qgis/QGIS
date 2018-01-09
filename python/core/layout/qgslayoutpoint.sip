/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutpoint.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayoutPoint
{
%Docstring
 This class provides a method of storing points, consisting of an x and y coordinate,
for use in QGIS layouts. Measurement units are stored alongside the position.

.. seealso:: :py:class:`QgsLayoutMeasurementConverter`

.. versionadded:: 3.0

.. note::

   This class does not inherit from QPointF since QPointF includes methods which should not apply
to positions with with units. For instance, the + and - operators would mislead users of this class
to believe that addition of two QgsLayoutPoints with different unit types would automatically convert
units. Instead, all unit conversion must be handled by a QgsLayoutMeasurementConverter so that
conversion between paper and screen units can be correctly performed.
%End

%TypeHeaderCode
#include "qgslayoutpoint.h"
%End
  public:

    QgsLayoutPoint( const double x, const double y, const QgsUnitTypes::LayoutUnit units = QgsUnitTypes::LayoutMillimeters );
%Docstring
Constructor for QgsLayoutPoint.
%End

    explicit QgsLayoutPoint( const QPointF point, const QgsUnitTypes::LayoutUnit units = QgsUnitTypes::LayoutMillimeters );
%Docstring
Constructor for QgsLayoutPoint.
%End

    explicit QgsLayoutPoint( const QgsUnitTypes::LayoutUnit units = QgsUnitTypes::LayoutMillimeters );
%Docstring
Constructor for an empty point, where both x and y are set to 0.

:param units: units for measurement
%End

    void setPoint( const double x, const double y );
%Docstring
Sets new x and y coordinates for the point.

.. seealso:: :py:func:`setX`

.. seealso:: :py:func:`setY`

.. seealso:: :py:func:`setUnits`
%End

    double x() const;
%Docstring
Returns x coordinate of point.

.. seealso:: :py:func:`setX`

.. seealso:: y
%End

    void setX( const double x );
%Docstring
Sets the x coordinate of point.

.. seealso:: x

.. seealso:: :py:func:`setY`
%End

    double y() const;
%Docstring
Returns y coordinate of point.

.. seealso:: :py:func:`setY`

.. seealso:: x
%End

    void setY( const double y );
%Docstring
Sets y coordinate of point.

.. seealso:: y

.. seealso:: :py:func:`setX`
%End

    QgsUnitTypes::LayoutUnit units() const;
%Docstring
Returns the units for the point.

.. seealso:: :py:func:`setUnits`
%End

    void setUnits( const QgsUnitTypes::LayoutUnit units );
%Docstring
Sets the ``units`` for the point. Does not alter the stored coordinates,
ie. no conversion is done.

.. seealso:: :py:func:`units`
%End

    bool isNull() const;
%Docstring
Tests whether the position is null, ie both its x and y coordinates
are zero.

:return: true if point is null
%End

    QPointF toQPointF() const;
%Docstring
Converts the layout point to a QPointF. The unit information is discarded
during this operation.

:return: QPointF with same x and y coordinates as layout point
%End

    QString encodePoint() const;
%Docstring
Encodes the layout point to a string

.. seealso:: :py:func:`decodePoint`
%End

    static QgsLayoutPoint decodePoint( const QString &string );
%Docstring
Decodes a point from a ``string``.

.. seealso:: :py:func:`encodePoint`
%End

    bool operator==( const QgsLayoutPoint &other ) const;
    bool operator!=( const QgsLayoutPoint &other ) const;

    QgsLayoutPoint operator*( const double v ) const;

    QgsLayoutPoint operator*=( const double v );

    QgsLayoutPoint operator/( const double v ) const;

    QgsLayoutPoint operator/=( const double v );

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutpoint.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
