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

.. note::

   Added in version 2.14
%End

%TypeHeaderCode
#include "qgscoordinateformatter.h"
%End
  public:

    enum Format
    {
      Pair,
      DegreesMinutesSeconds,
      DegreesMinutes,
      DecimalDegrees,
    };

    enum FormatFlag
    {
      DegreesUseStringSuffix,
      DegreesPadMinutesSeconds,
    };
    typedef QFlags<QgsCoordinateFormatter::FormatFlag> FormatFlags;


    static QString formatX( double x, Format format, int precision = 12, FormatFlags flags = DegreesUseStringSuffix );
%Docstring
 Formats an X coordinate value according to the specified parameters.
 @param x x-coordinate
 @param format string format to use for coordinate
 @param precision number of decimal places to include
 @param flags flags controlling format options
 @returns formatted X coordinate string
 @see formatY()
 :rtype: str
%End

    static QString formatY( double y, Format format, int precision = 12, FormatFlags flags = DegreesUseStringSuffix );
%Docstring
 Formats an Y coordinate value according to the specified parameters.
 @param y y-coordinate
 @param format string format to use for coordinate
 @param precision number of decimal places to include
 @param flags flags controlling format options
 @returns formatted Y coordinate string
 @see formatX()
 :rtype: str
%End

    static QString asPair( double x, double y, int precision = 12 );
%Docstring
 Formats coordinates as an "x,y" pair, with optional decimal precision.
 @param x x-coordinate
 @param y y-coordinate
 @param precision number of decimal places to include
 @returns formatted coordinate string
 :rtype: str
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgscoordinateformatter.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
