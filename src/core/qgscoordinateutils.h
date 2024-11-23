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
class QgsRectangle;

//not stable api - I plan on reworking this when QgsCoordinateFormatter lands in 2.16
///@cond NOT_STABLE_API

/**
 * \ingroup core
 * \class QgsCoordinateUtils
 * \brief Utilities for handling and formatting coordinates
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
     * Calculates coordinate precision for a \a crs and \a project.
     *
     * This method checks whether the \a project has explicit precision settings, and if not
     * calculates a precision based on CRS units.
     *
     * \param crs Coordinate system
     * \param project QGIS project. Takes QgsProject.instance() if NULL
     *
     * \returns number of decimal places behind the dot
     * \since QGIS 3.18
     */
    Q_INVOKABLE static int calculateCoordinatePrecisionForCrs( const QgsCoordinateReferenceSystem &crs, QgsProject *project = nullptr );

    /**
     * Calculates a reasonable coordinate precision for a \a crs.
     *
     * This method considers the CRS units only.
     *
     * \since QGIS 3.30
     */
    Q_INVOKABLE static int calculateCoordinatePrecision( const QgsCoordinateReferenceSystem &crs );

    /**
     * Formats a \a point coordinate for use with the specified \a project, respecting the project's
     * coordinate display settings.
     *
     * \note This method returns the x and y coordinates as a concatenated string which respects the project's coordinate axis display order.
     * See formatCoordinatePartsForProject() for a method which returns separate x and y coordinate strings.
     *
     * \since QGIS 3.2
     */
    Q_INVOKABLE static QString formatCoordinateForProject( QgsProject *project, const QgsPointXY &point, const QgsCoordinateReferenceSystem &destCrs, int precision );

    /**
     * Formats a \a point coordinate for use with the specified \a project, respecting the project's
     * coordinate display settings.
     *
     * \note This method returns the x and y coordinates as separate strings, see formatCoordinateForProject() for a method which returns a single concatenated string
     * respecting the project's coordinate axis display order.
     *
     * \since QGIS 3.28
     */
    Q_INVOKABLE static void formatCoordinatePartsForProject( QgsProject *project, const QgsPointXY &point, const QgsCoordinateReferenceSystem &destCrs, int precision, QString &x, QString &y );

    /**
     * Formats an \a extent for use with the specified \a project, respecting the project's
     * coordinate display settings.
     * \since QGIS 3.18
     */
    Q_INVOKABLE static QString formatExtentForProject( QgsProject *project, const QgsRectangle &extent, const QgsCoordinateReferenceSystem &destCrs, int precision );

    /**
     * Converts a degree minute second coordinate string to its decimal equivalent.
     * \param string degree minute second string to convert
     * \returns Double decimal value
     * \since QGIS 3.16
     */
    Q_INVOKABLE static double dmsToDecimal( const QString &string, bool *ok = nullptr, bool *isEasting = nullptr );

    /**
     * Converts a decimal degree with suffix string to its raw decimal equivalent.
     * \param string decimal degree to convert (must include a [N,S,E,W] suffix)
     * \returns Double decimal value
     * \since QGIS 3.26
     */
    Q_INVOKABLE static double degreeToDecimal( const QString &string, bool *ok = nullptr, bool *isEasting = nullptr );

};


///@endcond

#endif //QGSCOORDINATEUTILS_H
