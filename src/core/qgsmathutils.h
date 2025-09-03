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

#include <QObject>

#include "qgis_core.h"
#include "qgis_sip.h"

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

};

#endif // QGSMATHUTILS_H
