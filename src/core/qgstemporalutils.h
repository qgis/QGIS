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
class QgsMapDecoration;

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

    //! Contains settings relating to exporting animations
    struct AnimationExportSettings
    {
      //! Dictates the overall temporal range of the animation.
      QgsDateTimeRange animationRange;

      //! Duration of individual export frames
      QgsInterval frameDuration;

      //! Destination directory for created image files.
      QString outputDirectory;

      /**
       * The filename template for exporting the frames.
       *
       * This must be in format prefix####.format, where number of
       * \a # characters represents how many 0's should be left-padded to the frame number
       * e.g. my###.jpg will create frames my001.jpg, my002.jpg, etc
       */
      QString fileNameTemplate;

      //! List of decorations to draw onto exported frames.
      QList<QgsMapDecoration *> decorations;

    };

    /**
     * Exports animation frames by rendering the map to multiple destination images.
     *
     * The \a mapSettings argument dictates the overall map settings such as extent
     * and size, while animation and export specific settings are specified via the \a settings
     * argument.
     *
     * An optional \a feedback argument can be used to provide progress reports and cancellation
     * support.
     *
     * \param mapSettings settings controlling the map render
     * \param settings animation and export settings
     * \param error will be set to a descriptive error message if the export fails
     * \param feedback optional feedback object for progress reports and cancellation checks
     *
     * \returns TRUE if the export was successful.
     */
    static bool exportAnimation( const QgsMapSettings &mapSettings, const AnimationExportSettings &settings, QString &error SIP_OUT, QgsFeedback *feedback = nullptr );
};


#endif // QGSTEMPORALUTILS_H
