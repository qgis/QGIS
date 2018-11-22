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
#include "qgis.h"
#include "qgspointxy.h"

/**
 * \ingroup core
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
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsCoordinateFormatter
{
  public:

    /**
     * Available formats for displaying coordinates.
     */
    enum Format
    {
      FormatPair, //!< Formats coordinates as an "x,y" pair
      FormatDegreesMinutesSeconds, //!< Degrees, minutes and seconds, eg 30 degrees 45'30"
      FormatDegreesMinutes, //!< Degrees and decimal minutes, eg 30degrees 45.55'
      FormatDecimalDegrees, //!< Decimal degrees, eg 30.7555 degrees
    };

    /**
     * Flags for controlling formatting of coordinates.
     */
    enum FormatFlag
    {
      FlagDegreesUseStringSuffix = 1 << 1, //!< Include a direction suffix (eg 'N', 'E', 'S' or 'W'), otherwise a "-" prefix is used for west and south coordinates
      FlagDegreesPadMinutesSeconds = 1 << 2, //!< Pad minute and second values with leading zeros, eg '05' instead of '5'
    };
    Q_DECLARE_FLAGS( FormatFlags, FormatFlag )

    /**
     * Formats an \a x coordinate value according to the specified parameters.
     *
     * The \a format argument indicates the desired display format for the coordinate.
     *
     * The \a precision argument gives the number of decimal places to include for coordinates.
     *
     * Optional \a flags can be specified to control the output format.
     *
     * \see formatY()
     */
    static QString formatX( double x, Format format, int precision = 12, FormatFlags flags = FlagDegreesUseStringSuffix );

    /**
     * Formats a \a y coordinate value according to the specified parameters.
     *
     * The \a format argument indicates the desired display format for the coordinate.
     *
     * The \a precision argument gives the number of decimal places to include for coordinates.
     *
     * Optional \a flags can be specified to control the output format.
     *
     * \see formatX()
     */
    static QString formatY( double y, Format format, int precision = 12, FormatFlags flags = FlagDegreesUseStringSuffix );

    /**
     * Formats a \a point according to the specified parameters.
     *
     * The \a format argument indicates the desired display format for the coordinate.
     *
     * The \a precision argument gives the number of decimal places to include for coordinates.
     *
     * Optional \a flags can be specified to control the output format.
     */
    static QString format( const QgsPointXY &point, Format format, int precision = 12,  FormatFlags flags = FlagDegreesUseStringSuffix );

    /**
     * Formats coordinates as an "\a x,\a y" pair, with optional decimal \a precision (number
     * of decimal places to include).
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

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsCoordinateFormatter::FormatFlags )

#endif // QGSCOORDINATEFORMATTER_H
