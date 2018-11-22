/***************************************************************************
                             qgscoordinateutils.h
                             --------------------
    begin                : February 2016
    copyright            : (C) 2016 by Nyall Dawson
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

#ifndef QGSCOORDINATEUTILS_H
#define QGSCOORDINATEUTILS_H

#define SIP_NO_FILE

#include <QString>
#include <QObject>

#include "qgis_core.h"

class QgsPointXY;
class QgsCoordinateReferenceSystem;
class QgsProject;

//not stable api - I plan on reworking this when QgsCoordinateFormatter lands in 2.16
///@cond NOT_STABLE_API

/**
 * \ingroup core
 * \class QgsCoordinateUtils
 * \brief Utilities for handling and formatting coordinates
 * \since QGIS 2.14
 */
class CORE_EXPORT QgsCoordinateUtils
{
    Q_GADGET

  public:

    /**
     * Returns the precision to use for displaying coordinates in \a mapCrs to the user.
     * It respects the user's \a project settings.
     * If the user has set the project to use "automatic" precision, this function tries
     * to calculate an optimal coordinate precision for a given \a mapUnitsPerPixel by
     * calculating the number of decimal places for the coordinates with the aim of always
     * having enough decimal places to show the difference in position between adjacent
     * pixels.
     *
     * \note  Since QGIS 3.6 a new \a project parameter is available. Using the method without this
     *        a \a project parameter is deprecated and will be removed with QGIS 4.
     *        For backward compatibility, QgsProject.instance() will be used if the \a project
     *        parameter is not specified.
     */
    Q_INVOKABLE static int calculateCoordinatePrecision( double mapUnitsPerPixel, const QgsCoordinateReferenceSystem &mapCrs, QgsProject *project = nullptr );

    /**
     * Formats a \a point coordinate for use with the specified \a project, respecting the project's
     * coordinate display settings.
     * \since QGIS 3.2
     */
    Q_INVOKABLE static QString formatCoordinateForProject( QgsProject *project, const QgsPointXY &point, const QgsCoordinateReferenceSystem &destCrs, int precision );

};


///@endcond

#endif //QGSCOORDINATEUTILS_H
