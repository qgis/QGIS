/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscoordinateformatter.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsCoordinateFormatter
{
%Docstring
 Contains methods for converting coordinates for display in various formats.

QgsCoordinateFormatter contains static methods for converting numeric coordinates into different
formats, for instance as degrees, minutes, seconds values. Note that QgsCoordinateFormatter has
no consideration for the validity of converting coordinates to the various display formats, and it
is up to the caller to ensure that sensible formats are used for particular coordinates. For instance,
ensuring that only geographic coordinates and not projected coordinates are formatted to degree
based formats.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgscoordinateformatter.h"
%End
  public:

    enum Format
    {
      FormatPair,
      FormatDegreesMinutesSeconds,
      FormatDegreesMinutes,
      FormatDecimalDegrees,
    };

    enum FormatFlag
    {
      FlagDegreesUseStringSuffix,
      FlagDegreesPadMinutesSeconds,
    };
    typedef QFlags<QgsCoordinateFormatter::FormatFlag> FormatFlags;


    static QString formatX( double x, Format format, int precision = 12, FormatFlags flags = FlagDegreesUseStringSuffix );
%Docstring
Formats an ``x`` coordinate value according to the specified parameters.

The ``format`` argument indicates the desired display format for the coordinate.

The ``precision`` argument gives the number of decimal places to include for coordinates.

Optional ``flags`` can be specified to control the output format.

.. seealso:: :py:func:`formatY`
%End

    static QString formatY( double y, Format format, int precision = 12, FormatFlags flags = FlagDegreesUseStringSuffix );
%Docstring
Formats a ``y`` coordinate value according to the specified parameters.

The ``format`` argument indicates the desired display format for the coordinate.

The ``precision`` argument gives the number of decimal places to include for coordinates.

Optional ``flags`` can be specified to control the output format.

.. seealso:: :py:func:`formatX`
%End

    static QString format( QgsPointXY point, Format format, int precision = 12,  FormatFlags flags = FlagDegreesUseStringSuffix );
%Docstring
Formats a ``point`` according to the specified parameters.

The ``format`` argument indicates the desired display format for the coordinate.

The ``precision`` argument gives the number of decimal places to include for coordinates.

Optional ``flags`` can be specified to control the output format.
%End

    static QString asPair( double x, double y, int precision = 12 );
%Docstring
Formats coordinates as an "``x``,``y``" pair, with optional decimal ``precision`` (number
of decimal places to include).
%End

};

QFlags<QgsCoordinateFormatter::FormatFlag> operator|(QgsCoordinateFormatter::FormatFlag f1, QFlags<QgsCoordinateFormatter::FormatFlag> f2);


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscoordinateformatter.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
