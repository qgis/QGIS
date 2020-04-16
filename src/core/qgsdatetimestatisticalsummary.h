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
#include <QDate>
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
 * \since QGIS 2.16
 */

class CORE_EXPORT QgsDateTimeStatisticalSummary
{
  public:

    //! Enumeration of flags that specify statistics to be calculated
    enum Statistic
    {
      Count = 1 << 0,               //!< Count
      CountDistinct = 1 << 1,       //!< Number of distinct datetime values
      CountMissing = 1 << 2,        //!< Number of missing (null) values
      Mean = 1 << 14,               //!< Mean of values (since QGIS 3.16)
      Median = 1 << 15,             //!< Median of values (since QGIS 3.16)
      StDev = 1 << 16,              //!< Standard deviation of values (since QGIS 3.16)
      StDevSample = 1 << 17,        //!< Sample standard deviation of values (since QGIS 3.16)
      Min = 1 << 3,                 //!< Minimum (earliest) datetime value
      Max = 1 << 4,                 //!< Maximum (latest) datetime value
      Range = 1 << 5,               //!< Interval between earliest and latest datetime value
      Minority = 1 << 6,            //!< Minority of values (since QGIS 3.16)
      Majority = 1 << 7,            //!< Majority of values (since QGIS 3.16)
      FirstQuartile = 1 << 18,      //!< First quartile (since QGIS 3.16)
      ThirdQuartile = 1 << 19,      //!< Third quartile (since QGIS 3.16)
      InterQuartileRange = 1 << 20, //!< Inter quartile range (IQR) (since QGIS 3.16)
      First = 1 << 11,              //!< First value (since QGIS 3.16)
      Last = 1 << 12,               //!< Last value (since QGIS 3.16)
      Mode = 1 << 13,               //!< Mode value (since QGIS 3.16)

      All = Count | CountDistinct | CountMissing | Mean | Median | StDev | StDevSample | Min | Max | Range | FirstQuartile | ThirdQuartile | InterQuartileRange | Minority | Majority | First | Last | Mode, //!< All statistics
    };
    Q_DECLARE_FLAGS( Statistics, Statistic )

    /**
     * Constructor for QgsDateTimeStatisticalSummary
     * \param stats flags for statistics to calculate
     */
    QgsDateTimeStatisticalSummary( QgsDateTimeStatisticalSummary::Statistics stats = All );

    /**
     * Returns flags which specify which statistics will be calculated. Some statistics
     * are always calculated (e.g., count).
     * \see setStatistics
     */
    Statistics statistics() const { return mStatistics; }

    /**
     * Sets flags which specify which statistics will be calculated. Some statistics
     * are always calculated (e.g., count).
     * \param stats flags for statistics to calculate
     * \see statistics
     */
    void setStatistics( Statistics stats ) { mStatistics = stats; }

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
    QVariant statistic( QgsDateTimeStatisticalSummary::Statistic stat ) const;

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
    QSet<QDateTime> distinctValues() const { return QSet<QDateTime>::fromList( mValues.keys() ); }

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
    QgsInterval range() const { return mMax - mMin; }

    /**
     * Returns calculated mean of values. A null QDateTime may be returned if the mean cannot
     * be calculated.
     * \since QGIS 3.16
     */
    QDateTime mean() const { return mMean; }

    /**
     * Returns calculated median of values. This is only calculated if Statistic::Median has
     * been specified in the constructor or via setStatistics. 
     * \since QGIS 3.16
     */
    QDateTime median() const { return ( mIsTimes ) ? QDateTime( QDate::fromJulianDay( 0 ), mMedian.time() ) : mMedian; }

    /**
     * Returns population standard deviation. This is only calculated if Statistic::StDev has
     * been specified in the constructor or via setStatistics. A null QDateTime may be returned if the standard deviation cannot
     * be calculated.
     * \see sampleStDev
     * \since QGIS 3.16
     */
    QgsInterval stDev() const { return mStDev; }

    /**
     * Returns sample standard deviation. This is only calculated if Statistic::StDev has
     * been specified in the constructor or via setStatistics. A null QDateTime may be returned if the standard deviation cannot
     * be calculated.
     * \see stDev
     * \since QGIS 3.16
     */
    QgsInterval sampleStDev() const { return mSampleStDev; }

    /**
     * Returns the first quartile of the values. The quartile is calculated using the
     * "Tukey's hinges" method. A null QDateTime may be returned if the first quartile cannot
     * be calculated.
     * \see thirdQuartile
     * \see interQuartileRange
     * \since QGIS 3.16
     */
    QDateTime firstQuartile() const { return mFirstQuartile; }

    /**
     * Returns the third quartile of the values. The quartile is calculated using the
     * "Tukey's hinges" method. A null QDateTime may be returned if the third quartile cannot
     * be calculated.
     * \see firstQuartile
     * \see interQuartileRange
     * \since QGIS 3.16
     */
    QDateTime thirdQuartile() const { return mThirdQuartile; }

    /**
     * Returns the inter quartile range of the values. The quartiles are calculated using the
     * "Tukey's hinges" method. A null QDateTime may be returned if the IQR cannot
     * be calculated.
     * \see firstQuartile
     * \see thirdQuartile
     * \since QGIS 3.16
     */
    QgsInterval interQuartileRange() const { return mThirdQuartile - mFirstQuartile; }

    /**
     * Returns the least common string. The minority is the value with least occurrences in the list
     * This is only calculated if Statistic::Minority has been specified in the constructor
     * or via setStatistics. If multiple values match, return the first value sorted alphabetically
     * ascending (this may change in future versions).
     * \see majority
     * \since QGIS 3.16
     */
    QDateTime minority() const { return mMinority; }

    /**
     * Returns the most common string. The majority is the value with most occurrences in the list
     * This is only calculated if Statistic::Majority has been specified in the constructor
     * or via setStatistics. If multiple values match, return the first value sorted alphabetically
     * ascending (this may change in future versions).
     * \see minority
     * \since QGIS 3.16
     */
    QDateTime majority() const { return mMajority; }

    /**
     * Returns the first value obtained.
     *
     * \see last()
     * \since QGIS 3.16
     */
    QDateTime first() const { return mFirst; }

    /**
     * Returns the last value obtained.
     *
     * \see first()
     * \since QGIS 3.16
     */
    QDateTime last() const { return mLast; }

    /**
     * Returns the mode of the values. The values are sorted alphabetically in ascending order (this may change in future versions).
     *
     * \see majority()
     * \since QGIS 3.16
     */
    QList<QDateTime> mode() const { return mMode; }

    /**
     * Returns the friendly display name for a statistic
     * \param statistic statistic to return name for
     */
    static QString displayName( QgsDateTimeStatisticalSummary::Statistic statistic );

  private:

    Statistics mStatistics;

    int mCount;
    QMap<QDateTime, int> mValues;
    QList<QDateTime> mAllValues;
    int mCountMissing;
    long long mSumMSec;
    QDateTime mMean;
    QgsInterval mStDev;
    QgsInterval mSampleStDev;
    QDateTime mMedian;
    QDateTime mMin;
    QDateTime mMax;
    QDateTime mFirstQuartile;
    QDateTime mThirdQuartile;
    QDateTime mMinority;
    QDateTime mMajority;
    QDateTime mFirst;
    QDateTime mLast;
    QList<QDateTime> mMode;
    bool mIsTimes;

    void testDateTime( const QDateTime &dateTime );
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsDateTimeStatisticalSummary::Statistics )

#endif // QGSDATETIMESTATISTICALSUMMARY_H
