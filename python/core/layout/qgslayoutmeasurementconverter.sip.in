/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutmeasurementconverter.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLayoutMeasurementConverter
{
%Docstring
 This class provides a method of converting QgsLayoutMeasurements from
one unit to another. Conversion to or from pixel units utilizes a specified
dots per inch (DPI) property for the converter. Converters default to using
300 DPI.

.. seealso:: :py:class:`QgsLayoutMeasurement`

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutmeasurementconverter.h"
%End
  public:

    QgsLayoutMeasurementConverter();
%Docstring
Constructor for QgsLayoutMeasurementConverter.
%End

    void setDpi( const double dpi );
%Docstring
Sets the dots per inch (``dpi``) for the measurement converter. This is used
when converting measurements to and from pixels.

.. seealso:: :py:func:`dpi`
%End

    double dpi() const;
%Docstring
Returns the Dots per inch (DPI) of the measurement converter. This is used
when converting measurements to and from pixels.

.. seealso:: :py:func:`setDpi`
%End

    QgsLayoutMeasurement convert( const QgsLayoutMeasurement &measurement, const QgsUnitTypes::LayoutUnit targetUnits ) const;
%Docstring
Converts a measurement from one unit to another.

:param measurement: measurement to convert
:param targetUnits: units to convert measurement into

:return: measurement converted to target units
%End

    QgsLayoutSize convert( const QgsLayoutSize &size, const QgsUnitTypes::LayoutUnit targetUnits ) const;
%Docstring
Converts a layout size from one unit to another.

:param size: layout size to convert
:param targetUnits: units to convert size into

:return: size converted to target units
%End

    QgsLayoutPoint convert( const QgsLayoutPoint &point, const QgsUnitTypes::LayoutUnit targetUnits ) const;
%Docstring
Converts a layout point from one unit to another.

:param point: layout point to convert
:param targetUnits: units to convert point into

:return: point converted to target units
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutmeasurementconverter.h                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
