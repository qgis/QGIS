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

#ifndef SIP_RUN

/**
 * \ingroup core
 * \class QgsTimeDuration
 * \brief Contains utility methods for working with temporal layers and projects.
 *
 * Designed for storage of ISO8601 duration values.
 *
 * \note Not available in Python bindings
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsTimeDuration
{
  public:

    //! Years
    int years = 0;
    //! Months
    int months = 0;
    //! Weeks
    int weeks = 0;
    //! Days
    int days = 0;
    //! Hours
    int hours = 0;
    //! Minutes
    int minutes = 0;
    //! Seconds
    double seconds = 0;

    /**
     * Returns TRUE if the duration is null, i.e. an empty duration.
     */
    bool isNull() const
    {
      return !years && !months && !days &&
             !hours && !minutes && !seconds;
    }

    // TODO c++20 - replace with = default
    bool operator==( const QgsTimeDuration &other ) const
    {
      return years == other.years && months == other.months && weeks == other.weeks &&
             days == other.days && hours == other.hours &&
             minutes == other.minutes && seconds == other.seconds;
    }

    bool operator!=( const QgsTimeDuration &other ) const
    {
      return !( *this == other );
    }

    /**
     * Converts the duration to an interval value.
     */
    QgsInterval toInterval() const;

    /**
     * Converts the duration to an ISO8601 duration string.
     */
    QString toString() const;

    /**
     * Returns the total duration in seconds.
     *
     * \warning If the duration contains year or month intervals then the returned
     * value is approximate only, due to the variable length of these intervals.
     */
    long long toSeconds() const;

    /**
     * Adds this duration to a starting \a dateTime value.
     */
    QDateTime addToDateTime( const QDateTime &dateTime );

    /**
     * Creates a QgsTimeDuration from a \a string value.
     */
    static QgsTimeDuration fromString( const QString &string, bool &ok );

};
#endif


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
     * Calculates all temporal ranges which are in use for a \a project.
     *
     * This method considers the temporal range available from layers contained within the project and
     * returns a list of ranges which cover only the temporal ranges which are actually in use by layers
     * in the project.
     *
     * The returned list may be non-contiguous and have gaps in the ranges. The ranges are sorted in ascending order.
     *
     * \since QGIS 3.20
     */
    static QList< QgsDateTimeRange > usedTemporalRangesForProject( QgsProject *project );

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

      /**
       * Target animation frame rate in frames per second.
       *
       * \since QGIS 3.26
       */
      double frameRate = 30;

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

    /**
     * Calculates the frame time for an animation.
     *
     * If the interval original duration is fractional or interval original unit is
     * unknown (QgsUnitTypes::TemporalUnit::TemporalUnknownUnit), then QgsInterval is used
     * to determine the duration of the frame. This uses average durations for months and years.
     *
     * Otherwise, we use QDateTime to advance by the exact duration of the current
     * month or year.
     * So a time step of 1.5 months will result in a duration of 45
     * days, but a time step of 1 month will result in a duration that depends upon
     * the number of days in the current month.
     *
     * \param start time of the animation
     * \param frame number
     * \param interval duration of the animation
     *
     * \returns The calculated datetime for the frame.
     *
     * \since QGIS 3.18
     */
    static QDateTime calculateFrameTime( const QDateTime &start, const long long frame, const QgsInterval &interval );

    /**
     * Calculates a complete list of datetimes between \a start and \a end, using the specified ISO8601 \a duration string (eg "PT12H").
     * \param start start date time
     * \param end end date time
     * \param duration ISO8601 duration string
     * \param ok will be set to TRUE if \a duration was successfully parsed and date times could be calculated
     * \param maxValuesExceeded will be set to TRUE if the maximum number of values to return was exceeded
     * \param maxValues maximum number of values to return, or -1 to return all values
     * \returns calculated list of date times
     * \since QGIS 3.20
     */
    static QList< QDateTime > calculateDateTimesUsingDuration( const QDateTime &start, const QDateTime &end, const QString &duration, bool &ok SIP_OUT, bool &maxValuesExceeded SIP_OUT, int maxValues = -1 );

#ifndef SIP_RUN

    /**
     * Calculates a complete list of datetimes between \a start and \a end, using the specified ISO8601 \a duration string (eg "PT12H").
     * \param start start date time
     * \param end end date time
     * \param duration ISO8601 duration
     * \param maxValuesExceeded will be set to TRUE if the maximum number of values to return was exceeded
     * \param maxValues maximum number of values to return, or -1 to return all values
     * \returns calculated list of date times
     * \note Not available in Python bindings
     * \since QGIS 3.20
     */
    static QList< QDateTime > calculateDateTimesUsingDuration( const QDateTime &start, const QDateTime &end, const QgsTimeDuration &duration, bool &maxValuesExceeded SIP_OUT, int maxValues = -1 );
#endif

    /**
     * Calculates a complete list of datetimes from a ISO8601 \a string containing a duration (eg "2021-03-23T00:00:00Z/2021-03-24T12:00:00Z/PT12H").
     * \param string ISO8601 compatible string
     * \param ok will be set to TRUE if \a string was successfully parsed and date times could be calculated
     * \param maxValuesExceeded will be set to TRUE if the maximum number of values to return was exceeded
     * \param maxValues maximum number of values to return, or -1 to return all values
     * \returns calculated list of date times
     * \since QGIS 3.20
     */
    static QList< QDateTime > calculateDateTimesFromISO8601( const QString &string, bool &ok SIP_OUT, bool &maxValuesExceeded SIP_OUT, int maxValues = -1 );

};


#endif // QGSTEMPORALUTILS_H
