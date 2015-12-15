/***************************************************************************
                          qgscoordinateformatter.h
                          ------------------------
    begin                : Decemeber 2015
    copyright            : (C) 2015 by Nyall Dawson
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

#ifndef QGSCOORDINATEFORMATTER_H
#define QGSCOORDINATEFORMATTER_H

#include <QString>

/** \ingroup core
 * \class QgsCoordinateFormatter
 * \brief Contains methods for converting coordinates for display in various formats.
 *
 * QgsCoordinateFormatter contains static methods for converting numeric coordinates into different
 * formats, for instance as degrees, minutes, seconds values. Note that QgsCoordinateFormatter has
 * no consideration for the validity of converting coordinates to the various display formats, and it
 * is up to the caller to ensure that sensible formats are used for particular coordinates. For instance,
 * ensuring that only geographic coordinates and not projected coordinates are formatted to degree
 * based formats.
 *
 * \note Added in version 2.14
 */

class CORE_EXPORT QgsCoordinateFormatter
{
  public:

    /** Available formats for displaying coordinates.
     */
    enum Format
    {
      Pair, /*!< Formats coordinates as an "x,y" pair */
      DegreesMinutesSeconds, /*!< Degrees, minutes and seconds, eg 30 degrees 45'30" */
      DegreesMinutes, /*!< Degrees and decimal minutes, eg 30degrees 45.55' */
      DecimalDegrees, /*!< Decimal degrees, eg 30.7555 degrees */
    };

    /** Flags for controlling formatting of coordinates.
     */
    enum FormatFlag
    {
      DegreesUseStringSuffix = 0x01, /*!< include a direction suffix (eg 'N', 'E', 'S' or 'W'), otherwise a "-" prefix is used for west and south coordinates */
      DegreesPadMinutesSeconds = 0x02, /*!< pad minute and second values with leading zeros, eg '05' instead of '5' */
    };
    Q_DECLARE_FLAGS( FormatFlags, FormatFlag )

    /** Formats an X coordinate value according to the specified parameters.
     * @param x x-coordinate
     * @param format string format to use for coordinate
     * @param precision number of decimal places to include
     * @param flags flags controlling format options
     * @returns formatted X coordinate string
     * @see formatY()
     */
    static QString formatX( double x, Format format, int precision = 12, FormatFlags flags = DegreesUseStringSuffix );

    /** Formats an Y coordinate value according to the specified parameters.
     * @param y y-coordinate
     * @param format string format to use for coordinate
     * @param precision number of decimal places to include
     * @param flags flags controlling format options
     * @returns formatted Y coordinate string
     * @see formatX()
     */
    static QString formatY( double y, Format format, int precision = 12, FormatFlags flags = DegreesUseStringSuffix );

    /** Formats coordinates as an "x,y" pair, with optional decimal precision.
     * @param x x-coordinate
     * @param y y-coordinate
     * @param precision number of decimal places to include
     * @returns formatted coordinate string
     */
    static QString asPair( double x, double y, int precision = 12 );

  private:

    static QString formatAsPair( double val, int precision );

    static QString formatXAsDegreesMinutesSeconds( double val, int precision, FormatFlags flags );
    static QString formatYAsDegreesMinutesSeconds( double val, int precision, FormatFlags flags );

    static QString formatXAsDegreesMinutes( double val, int precision, FormatFlags flags );
    static QString formatYAsDegreesMinutes( double val, int precision, FormatFlags flags );

    static QString formatXAsDegrees( double val, int precision, FormatFlags flags );
    static QString formatYAsDegrees( double val, int precision, FormatFlags flags );
};

#endif // QGSCOORDINATEFORMATTER_H
