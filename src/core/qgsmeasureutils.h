/***************************************************************************
                             qgsmeasureutils.h
                             --------------------
    begin                : February 2026
    copyright            : (C) 2026 by Nyall Dawson
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

#ifndef QGSMEASUREUTILS_H
#define QGSMEASUREUTILS_H

#include "qgis.h"
#include "qgis_core.h"

#include <QObject>
#include <QString>

class QgsProject;

/**
 * \ingroup core
 * \class QgsMeasureUtils
 * \brief Utilities for handling and formatting measurements.
 * \since QGIS 4.0
 */
class CORE_EXPORT QgsMeasureUtils
{
    Q_GADGET

  public:

    /**
     * Formats an \a area measurement (with the specified \a unit) for use with a \a project, respecting the project's
     * area unit settings.
     */
    Q_INVOKABLE static QString formatAreaForProject( QgsProject *project, double area, Qgis::AreaUnit unit );

    /**
     * Formats a \a distance measurement (with the specified \a unit) for use with a \a project, respecting the project's
     * distance unit settings.
     */
    Q_INVOKABLE static QString formatDistanceForProject( QgsProject *project, double distance, Qgis::DistanceUnit unit );

};


#endif //QGSMEASUREUTILS_H
