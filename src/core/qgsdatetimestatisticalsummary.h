/***************************************************************************
  qgsdatetimestatisticalsummary.h
  -------------------------------
  Date                 : May 2016
  Copyright            : (C) 2016 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATETIMESTATISTICALSUMMARY_H
#define QGSDATETIMESTATISTICALSUMMARY_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsinterval.h"
#include <QSet>
#include <QDateTime>
#include <QVariantList>

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsdatetimestatisticalsummary.py.
 * See details in QEP #17
 ****************************************************************************/

/**
 * \ingroup core
 * \class QgsDateTimeStatisticalSummary
 * \brief Calculator for summary statistics and aggregates for a list of datetimes.
 *
 * Statistics are calculated by calling calculate() and passing a list of datetimes. The
 * individual statistics can then be retrieved using the associated methods. Note that not all statistics
 * are calculated by default. Statistics which require slower computations are only calculated by
 * specifying the statistic in the constructor or via setStatistics().
 *
 */

class CORE_EXPORT QgsDateTimeStatisticalSummary
{
  public:
    /**
     * Constructor for QgsDateTimeStatisticalSummary
     * \param stats flags for statistics to calculate
     */
    QgsDateTimeStatisticalSummary( Qgis::DateTimeStatistics stats = Qgis::DateTimeStatistic::All );

    /**
     * Returns flags which specify which statistics will be calculated. Some statistics
     * are always calculated (e.g., count).
     * \see setStatistics
     */
    Qgis::DateTimeStatistics statistics() const { return mStatistics; }

    /**
     * Sets flags which specify which statistics will be calculated. Some statistics
     * are always calculated (e.g., count).
     * \param stats flags for statistics to calculate
     * \see statistics
     */
    void setStatistics( Qgis::DateTimeStatistics stats ) { mStatistics = stats; }

    /**
     * Resets the calculated values
     */
    void reset();

    /**
     * Calculates summary statistics for a list of variants. Any non-datetime variants will be
     * ignored.
     * \param values list of variants
     * \see addValue()
     */
    void calculate( const QVariantList &values );

    /**
     * Adds a single datetime to the statistics calculation. Calling this method
     * allows datetimes to be added to the calculation one at a time. For large
     * quantities of dates this may be more efficient then first adding all the
     * variants to a list and calling calculate().
     * \param value datetime to add. Any non-datetime variants will be ignored.
     * \note call reset() before adding the first datetime using this method
     * to clear the results from any previous calculations
     * \note finalize() must be called after adding the final value and before
     * retrieving calculated statistics.
     * \see calculate()
     * \see finalize()
     */
    void addValue( const QVariant &value );

    /**
     * Must be called after adding all datetimes with addValue() and before retrieving
     * any calculated datetime statistics.
     * \see addValue()
     */
    void finalize();

    /**
     * Returns the value of a specified statistic
     * \param stat statistic to return
     * \returns calculated value of statistic
     */
    QVariant statistic( Qgis::DateTimeStatistic stat ) const;

    /**
     * Returns the calculated count of values.
     */
    int count() const { return mCount; }

    /**
     * Returns the number of distinct datetime values.
     */
    int countDistinct() const { return mValues.count(); }

    /**
     * Returns the set of distinct datetime values.
     */
    QSet<QDateTime> distinctValues() const { return mValues; }

    /**
     * Returns the number of missing (null) datetime values.
     */
    int countMissing() const { return mCountMissing; }

    /**
     * Returns the minimum (earliest) non-null datetime value.
     */
    QDateTime min() const { return mMin; }

    /**
     * Returns the maximum (latest) non-null datetime value.
     */
    QDateTime max() const { return mMax; }

    /**
     * Returns the range (interval between earliest and latest non-null datetime values).
     */
    QgsInterval range() const;

    /**
     * Returns the friendly display name for a statistic
     * \param statistic statistic to return name for
     */
    static QString displayName( Qgis::DateTimeStatistic statistic );

  private:
    Qgis::DateTimeStatistics mStatistics;

    int mCount;
    QSet<QDateTime> mValues;
    int mCountMissing;
    QDateTime mMin;
    QDateTime mMax;
    bool mIsTimes;

    void testDateTime( const QDateTime &dateTime, bool isNull );
};

#endif // QGSDATETIMESTATISTICALSUMMARY_H
