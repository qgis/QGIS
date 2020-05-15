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
#include "qgsinterval.h"

class QgsProject;
class QgsMapSettings;
class QgsFeedback;

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

    /**
     * Exports animation frames by rendering the map to multiple destination images.
     *
     * The \a mapSettings argument dictates the overall map settings such as extent
     * and size.
     *
     * The \a animationRange argument specifies the overall temporal range of the animation.
     * Temporal duration of individual frames is given by \a frameDuration.
     *
     * An \a outputDirectory must be set, which controls where the created image files are
     * stored. \a fileNameTemplate gives the template for exporting the frames.
     * This must be in format prefix####.format, where number of
     * # represents how many 0 should be left-padded to the frame number
     * e.g. my###.jpg will create frames my001.jpg, my002.jpg, etc
     */
    static bool exportAnimation( const QgsMapSettings &mapSettings,
                                 const QgsDateTimeRange &animationRange,
                                 QgsInterval frameDuration,
                                 const QString &outputDirectory,
                                 const QString &fileNameTemplate,
                                 QString &error SIP_OUT,
                                 QgsFeedback *feedback );
};


#endif // QGSTEMPORALUTILS_H
