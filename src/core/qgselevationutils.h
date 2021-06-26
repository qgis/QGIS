/***************************************************************************
  qgselevationutils.h
  ------------------
  Date                 : November 2020
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

#ifndef QGSELEVATIONUTILS_H
#define QGSELEVATIONUTILS_H

#include "qgis_core.h"
#include "qgsrange.h"

class QgsProject;

/**
 * \ingroup core
 * \class QgsElevationUtils
 * \brief Contains utility methods for working with elevation from layers and projects.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsElevationUtils
{
  public:

    /**
     * Calculates the elevation range for a \a project.
     *
     * This method considers the elevation (or z) range available from layers contained within the project and
     * returns the maximal combined elevation range of these layers.
     */
    static QgsDoubleRange calculateZRangeForProject( QgsProject *project );

};


#endif // QGSELEVATIONUTILS_H
