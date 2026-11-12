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

#include <cstdint>
#include <optional>

#include "qgis_core.h"
#include "qgsinterval.h"
#include "qgsrange.h"

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
    bool isNull() const { return !years && !months && !days && !hours && !minutes && !seconds; }

    // TODO c++20 - replace with = default
    bool operator==( const QgsTimeDuration &other ) const
    {
      return years == other.years && months == other.months && weeks == other.weeks && days == other.days && hours == other.hours && minutes == other.minutes && seconds == other.seconds;
    }

    bool operator!=( const QgsTimeDuration &other ) const { return !( *this == other ); }

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
    QDateTime addToDateTime( const QDateTime &dateTime ) const;

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
        /**
         * Animation mode.
         *
         * \since QGIS 4.4
         */
        Qgis::TemporalNavigationMode animationMode = Qgis::TemporalNavigationMode::Animated;

        //! Dictates the overall temporal range of the animation.
        QgsDateTimeRange animationRange;

        //! Duration of individual export frames
        QgsInterval frameDuration;

        /**
       * The animation temporal range cumulative settings.
       *
       * \since QGIS 4.0
       */
        bool temporalRangeCumulative = false;

        /**
         * Total frame count, for Qgis::TemporalNavigationMode::Movie mode only.
         *
         * \since QGIS 4.4
         */
        long long totalMovieFrames = 0;

        //! Destination directory for created image files.
        QString outputDirectory;

        /**
       * The filename template for exporting the frames.
       *
       * This must be in format ``prefix####.format``, where number of
       * \a ``#`` characters represents how many 0's should be left-padded to the frame number
       * e.g. ``my###.jpg`` will create frames ``my001.jpg``, ``my002.jpg``, etc
       */
        QString fileNameTemplate;

        //! List of decorations to draw onto exported frames.
        QList<QgsMapDecoration *> decorations;

        /**
       * Contains the list of all available temporal ranges which have data available.
       *
       * The list can be a list of non-contiguous ranges (i.e. containing gaps)
       * which together describe the complete range of times which contain data.
       *
       * This list is required whenever the QgsUnitTypes::TemporalIrregularStep interval is used
       * for an animation.
       *
       * \since QGIS 3.30
       */
        QList<QgsDateTimeRange> availableTemporalRanges;

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

#ifndef SIP_RUN

/**
 * \ingroup core
 * \class QgsNetCdfGribTimeParser
 * \brief Parses date times stored in netCDF and GRIB datasets (as exposed by GDAL metadata).
 *
 * The implementation is a port of MDAL's time handling (MDAL::parseCFTimeUnit(), MDAL::parseCFReferenceTime(),
 * MDAL::DateTime, MDAL::RelativeTimestamp and MDAL::DriverGdalGrib), so the results are identical
 * to the times reported for the same files opened as mesh layers.
 *
 * If anything changes in this class it should likely be reflected in the corresponding MDAL function.
 *
 * \note Not available in Python bindings
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsNetCdfGribTimeParser
{
  public:
    /**
     * Converts a time \a value expressed following the CF (Climate and Forecast) conventions used
     * by netCDF files to a date time, e.g. a value of 43465 with \a units "days since 1900-01-01 00:00:00"
     * to 2019-01-01T00:00:00.
     *
     * Same as MDAL::DriverCF::parseTime() / MDAL::DriverGdalNetCDF::parseGlobals().
     *
     * \param value the time value to convert
     * \param units the CF "units" attribute of the time variable (e.g., "days since 1900-01-01 00:00:00")
     * \param calendar the CF "calendar" attribute of the time variable
     *
     * \returns the corresponding date time in UTC, expressed in the proleptic Gregorian calendar and rounded down to whole seconds
     */
    static QDateTime parseCfTime( double value, const QString &units, const QString &calendar = QString() );

    /**
     * Converts a GRIB time stored in GDAL metadata (items "GRIB_REF_TIME" and "GRIB_VALID_TIME",
     * e.g. "1546300800 sec UTC", i.e. seconds since 1970-01-01T00:00:00 UTC) to a date time.
     *
     * Same as MDAL::DriverGdalGrib::parseBandInfo(): MDAL::DateTime( parseMetadataTime( value ), DateTime::Unix ).
     *
     * \param metadataValue the GRIB metadata value to convert (e.g., "1546300800 sec UTC")
     *
     * \returns the corresponding date time in UTC, expressed in the proleptic Gregorian calendar and rounded down to whole seconds. Invalid if \a metadataValue is empty or if the resulting year is not positive.
     */
    static QDateTime parseGribTime( const QString &metadataValue );

  private:
    static constexpr double MILLISECONDS_IN_SECOND = 1000;
    static constexpr double MILLISECONDS_IN_MINUTE = 1000 * 60;
    static constexpr double MILLISECONDS_IN_HOUR = 1000 * 60 * 60;
    static constexpr double MILLISECONDS_IN_DAY = 1000 * 60 * 60 * 24;
    static constexpr double MILLISECONDS_IN_WEEK = 1000 * 60 * 60 * 24 * 7;
    static constexpr double MILLISECONDS_IN_EXACT_YEAR = 3.15569259747e10;                //CF Compliant
    static constexpr double MILLISECONDS_IN_MONTH_CF = MILLISECONDS_IN_EXACT_YEAR / 12.0; //CF Compliant

    enum class CFTimeUnit
    {
      Milliseconds,
      Seconds,
      Minutes,
      Hours,
      Days,
      Weeks,
      MonthsCF,
      ExactYears
    };

    enum class CFCalendar
    {
      Gregorian,
      ProlepticGregorian,
      Julian
    };

    struct CFDateTimeValues
    {
        int year;
        int month;
        int day;
        int hours;
        int minutes;
        double seconds;
    };

    // MDAL::split( str, char ) - empty tokens are skipped
    static QStringList cfSplit( const QString &str, QChar delimiter );
    // MDAL::toInt( std::string )
    static int cfToInt( const QString &str );
    // MDAL::toDouble( std::string )
    static double cfToDouble( const QString &str );
    // MDAL::parseDurationTimeUnit()
    static CFTimeUnit cfParseDurationTimeUnit( const QString &timeUnit );
    // MDAL::parseCFTimeUnit()
    static CFTimeUnit cfParseTimeUnit( const QString &timeInformation );
    // MDAL::DateTime::setWithGregorianCalendarDate()
    static int64_t cfJulianTimeFromGregorianCalendarDate( CFDateTimeValues values );
    // MDAL::DateTime::setWithJulianCalendarDate()
    static int64_t cfJulianTimeFromJulianCalendarDate( CFDateTimeValues values );
    // MDAL::DateTime::setWithGregorianJulianCalendarDate()
    static int64_t cfJulianTimeFromGregorianJulianCalendarDate( const CFDateTimeValues &values );
    // MDAL::parseCFReferenceTime(), returns the reference time as Julian day in ms (MDAL::DateTime::mJulianTime)
    static std::optional<int64_t> cfParseReferenceTime( const QString &timeInformation, const QString &calendarString );
    // MDAL::RelativeTimestamp( double duration, Unit unit ), returns the duration in ms
    static int64_t cfRelativeTimestamp( double duration, CFTimeUnit unit );
    // MDAL::DateTime::dateTimeGregorianProleptic()
    static CFDateTimeValues cfDateTimeGregorianProleptic( int64_t julianTime );
    // MDAL::DateTime::toStandardCalendarISO8601(), returns an invalid QDateTime when MDAL returns an empty string
    static QDateTime cfJulianTimeToDateTime( int64_t julianTime );
    // MDAL::DriverGdal::parseMetadataTime()
    static std::optional<double> gribParseMetadataTime( const QString &timeString );
};

#endif

#endif // QGSTEMPORALUTILS_H
