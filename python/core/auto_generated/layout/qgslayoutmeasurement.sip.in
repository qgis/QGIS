/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutmeasurement.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLayoutMeasurement
{
%Docstring
 This class provides a method of storing measurements for use in QGIS layouts
using a variety of different measurement units.

.. seealso:: :py:class:`QgsLayoutMeasurementConverter`

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutmeasurement.h"
%End
  public:

    explicit QgsLayoutMeasurement( const double length, const QgsUnitTypes::LayoutUnit units = QgsUnitTypes::LayoutMillimeters );
%Docstring
Constructor for QgsLayoutMeasurement.

:param length: measurement length
:param units: measurement units
%End

    double length() const;
%Docstring
Returns the length of the measurement.

.. seealso:: :py:func:`setLength`
%End

    void setLength( const double length );
%Docstring
Sets the ``length`` of the measurement.

.. seealso:: :py:func:`length`
%End

    QgsUnitTypes::LayoutUnit units() const;
%Docstring
Returns the units for the measurement.

.. seealso:: :py:func:`setUnits`
%End

    void setUnits( const QgsUnitTypes::LayoutUnit units );
%Docstring
Sets the ``units`` for the measurement. Does not alter the stored length,
ie. no length conversion is done.

.. seealso:: :py:func:`units`
%End

    QString encodeMeasurement() const;
%Docstring
Encodes the layout measurement to a string

.. seealso:: :py:func:`decodeMeasurement`
%End

    static QgsLayoutMeasurement decodeMeasurement( const QString &string );
%Docstring
Decodes a measurement from a ``string``.

.. seealso:: :py:func:`encodeMeasurement`
%End

    bool operator==( const QgsLayoutMeasurement &other ) const;
    bool operator!=( const QgsLayoutMeasurement &other ) const;

    QgsLayoutMeasurement operator+( const double v ) const;

    QgsLayoutMeasurement operator+=( const double v );

    QgsLayoutMeasurement operator-( const double v ) const;

    QgsLayoutMeasurement operator-=( const double v );

    QgsLayoutMeasurement operator*( const double v ) const;

    QgsLayoutMeasurement operator*=( const double v );

    QgsLayoutMeasurement operator/( const double v ) const;

    QgsLayoutMeasurement operator/=( const double v );

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutmeasurement.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
