/***************************************************************************
  qgstemporalutils.h
  ------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTEMPORALUTILS_H
#define QGSTEMPORALUTILS_H

#include "qgis_core.h"
#include "qgsrange.h"

class QgsProject;

/**
 * \ingroup core
 * \class QgsTemporalUtils
 * \brief Contains utility methods for working with temporal layers and projects.
 *
 * \since QGIS 3.14
 */

class CORE_EXPORT QgsTemporalUtils
{
  public:

    /**
     * Calculates the temporal range for a \a project.
     *
     * This method considers the temporal range available from layers contained within the project and
     * returns the maximal combined temporal extent of these layers.
     */
    static QgsDateTimeRange calculateTemporalRangeForProject( QgsProject *project );

};


#endif // QGSTEMPORALUTILS_H
