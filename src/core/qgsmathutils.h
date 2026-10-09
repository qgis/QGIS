/***************************************************************************
    qgsmathutils.h
    ----------------------
    begin                : July 2025
    copyright            : (C) 2025 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrange.h"

#include <QObject>

#ifndef QGSMATHUTILS_H
#define QGSMATHUTILS_H

/**
 * \ingroup core
 * \brief Contains utility functions for mathematical operations.
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsMathUtils
{
    Q_GADGET

  public:
    /**
     * Converts a double \a value to a rational fraction.
     *
     * \param value value to convert
     * \param numerator calculated numerator
     * \param denominator calculated denominator
     * \param tolerance desired precision. The returned fraction will be at within this tolerance of the original value.
     * \param maxIterations maximum number of iterations. Higher values result in better approximations, but at the cost of additional computation.
     */
    Q_INVOKABLE static void doubleToRational( double value, qlonglong &numerator SIP_OUT, qlonglong &denominator SIP_OUT, double tolerance = 1.0e-9, int maxIterations = 100 );

    /**
     * Returns a round interval, a power of ten, which splits the given \a span into at least
     * \a divisions parts. E.g. a span of 2222 returns 100 for 10 divisions and 10 for 100 divisions.
     *
     * Returns 0 if \a span is not finite or is not greater than 0, or if \a divisions is lower than 1.
     *
     * \since QGIS 4.4
     */
    static double roundingInterval( double span, int divisions = 10 );

    /**
     * Expands a \a range outward to round values, e.g. a range of 1234.5 - 3456.7 is expanded
     * to 1200 - 3500.
     *
     * The bounds are rounded to a tenth of the order of magnitude of the range's size, so that
     * the returned range stays close to the original one.
     *
     * Infinite and empty ranges are returned unchanged.
     *
     * \since QGIS 4.4
     */
    static QgsDoubleRange roundedRange( const QgsDoubleRange &range );
};

#endif // QGSMATHUTILS_H
