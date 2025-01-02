/***************************************************************************
    qgscaleutils.h
    ----------------------
    begin                : July 2012
    copyright            : (C) 2012 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QString>
#include <QStringList>

#include "qgis_core.h"

#ifndef QGSSCALEUTILS_H
#define QGSSCALEUTILS_H

/**
 * \ingroup core
 * \brief Contains utility functions for working with map scales.
 */
class CORE_EXPORT QgsScaleUtils
{
  public:

    /**
     * Save scales to the given file
     * \param fileName the name of the output file
     * \param scales the list of scales to save
     * \param errorMessage it will contain the error message if something
     * went wrong
     * \returns TRUE on success and FALSE if failed
     */
    static bool saveScaleList( const QString &fileName, const QStringList &scales, QString &errorMessage );

    /**
     * Load scales from the given file
     * \param fileName the name of the file to process
     * \param scales it will contain loaded scales
     * \param errorMessage it will contain the error message if something
     * went wrong
     * \returns TRUE on success and FALSE if failed
     */
    static bool loadScaleList( const QString &fileName, QStringList &scales, QString &errorMessage );

    /**
     * Returns whether the \a scale is equal to or greater than the \a minScale,
     * taking non-round numbers into account.
     *
     * \param scale The current scale to be compared.
     * \param minScale The minimum map scale (i.e. most "zoomed out" scale) at
     * which features, labels or diagrams will be visible. The scale value
     * indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see lessThanMaximumScale()
     *
     * \since QGIS 3.40
     */
    static bool equalToOrGreaterThanMinimumScale( const double scale, const double minScale );

    /**
     * Returns whether the \a scale is less than the \a maxScale, taking non-round
     * numbers into account.
     *
     * \param scale The current scale to be compared.
     * \param maxScale The maximum map scale (i.e. most "zoomed in" scale) at which
     * features, labels or diagrams will be visible. The scale value indicates the
     * scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see equalToOrGreaterThanMinimumScale()
     *
     * \since QGIS 3.40
     */
    static bool lessThanMaximumScale( const double scale, const double maxScale );
};

#endif
