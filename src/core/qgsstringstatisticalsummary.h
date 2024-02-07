/***************************************************************************
  qgsstringstatisticalsummary.h
  -----------------------------
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

#ifndef QGSSTRINGSTATISTICALSUMMARY_H
#define QGSSTRINGSTATISTICALSUMMARY_H

#include <QSet>
#include <QVariantList>

#include "qgis_core.h"
#include "qgis.h"

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in test_qgsstringstatisticalsummary.py.
 * See details in QEP #17
 ****************************************************************************/

/**
 * \ingroup core
 * \class QgsStringStatisticalSummary
 * \brief Calculator for summary statistics and aggregates for a list of strings.
 *
 * Statistics are calculated by calling calculate() and passing a list of strings. The
 * individual statistics can then be retrieved using the associated methods. Note that not all statistics
 * are calculated by default. Statistics which require slower computations are only calculated by
 * specifying the statistic in the constructor or via setStatistics().
 *
 */

class CORE_EXPORT QgsStringStatisticalSummary
{
  public:

    /**
     * Constructor for QgsStringStatistics
     * \param stats flags for statistics to calculate
     */
    QgsStringStatisticalSummary( Qgis::StringStatistics stats = Qgis::StringStatistic::All );

    /**
     * Returns flags which specify which statistics will be calculated. Some statistics
     * are always calculated (e.g., count).
     * \see setStatistics
     */
    Qgis::StringStatistics statistics() const { return mStatistics; }

    /**
     * Sets flags which specify which statistics will be calculated. Some statistics
     * are always calculated (e.g., count).
     * \param stats flags for statistics to calculate
     * \see statistics
     */
    void setStatistics( Qgis::StringStatistics stats ) { mStatistics = stats; }

    /**
     * Resets the calculated values
     */
    void reset();

    /**
     * Calculates summary statistics for an entire list of strings at once.
     * \param values list of strings
     * \see calculateFromVariants()
     * \see addString()
     */
    void calculate( const QStringList &values );

    /**
     * Calculates summary statistics for an entire list of variants at once. Any
     * non-string variants will be ignored.
     * \param values list of variants
     * \see calculate()
     * \see addValue()
     */
    void calculateFromVariants( const QVariantList &values );

    /**
     * Adds a single string to the statistics calculation. Calling this method
     * allows strings to be added to the calculation one at a time. For large
     * quantities of strings this may be more efficient then first adding all the
     * strings to a list and calling calculate().
     * \param string string to add
     * \note call reset() before adding the first string using this method
     * to clear the results from any previous calculations
     * \note finalize() must be called after adding the final string and before
     * retrieving calculated statistics.
     * \see calculate()
     * \see addValue()
     * \see finalize()
     */
    void addString( const QString &string );

    /**
     * Adds a single variant to the statistics calculation. Calling this method
     * allows variants to be added to the calculation one at a time. For large
     * quantities of variants this may be more efficient then first adding all the
     * variants to a list and calling calculateFromVariants().
     * \param value variant to add
     * \note call reset() before adding the first string using this method
     * to clear the results from any previous calculations
     * \note finalize() must be called after adding the final value and before
     * retrieving calculated statistics.
     * \see calculateFromVariants()
     * \see finalize()
     */
    void addValue( const QVariant &value );

    /**
     * Must be called after adding all strings with addString() and before retrieving
     * any calculated string statistics.
     * \see addString()
     */
    void finalize();

    /**
     * Returns the value of a specified statistic
     * \param stat statistic to return
     * \returns calculated value of statistic
     */
    QVariant statistic( Qgis::StringStatistic stat ) const;

    /**
     * Returns the calculated count of values.
     */
    int count() const { return mCount; }

    /**
     * Returns the number of distinct string values.
     * \see distinctValues()
     */
    int countDistinct() const { return mValues.count(); }

    /**
     * Returns the set of distinct string values.
     * \see countDistinct()
     */
    QSet< QString > distinctValues() const;

    /**
     * Returns the number of missing (null) string values.
     */
    int countMissing() const { return mCountMissing; }

    /**
     * Returns the minimum (non-null) string value.
     */
    QString min() const { return mMin; }

    /**
     * Returns the maximum (non-null) string value.
     */
    QString max() const { return mMax; }

    /**
     * Returns the minimum length of strings.
     */
    int minLength() const { return mMinLength; }

    /**
     * Returns the maximum length of strings.
     */
    int maxLength() const { return mMaxLength; }

    /**
      * Returns the mean length of strings.
      */
    double meanLength() const { return mMeanLength; }

    /**
     * Returns the least common string. The minority is the value with least occurrences in the list
     * This is only calculated if Statistic::Minority has been specified in the constructor
     * or via setStatistics. If multiple values match, return the first value relative to the
     * initial values order.
     * \see majority
     * \since QGIS 3.14
     */
    QString minority() const { return mMinority; }

    /**
     * Returns the most common string. The majority is the value with most occurrences in the list
     * This is only calculated if Statistic::Majority has been specified in the constructor
     * or via setStatistics. If multiple values match, return the first value relative to the
     * initial values order.
     * \see minority
     * \since QGIS 3.14
     */
    QString majority() const { return mMajority; }

    /**
     * Returns the friendly display name for a statistic
     * \param statistic statistic to return name for
     */
    static QString displayName( Qgis::StringStatistic statistic );

  private:

    Qgis::StringStatistics mStatistics;

    int mCount;
    QMap< QString, int > mValues;
    int mCountMissing;
    QString mMin;
    QString mMax;
    int mMinLength;
    int mMaxLength;
    long mSumLengths;
    double mMeanLength;
    QString mMinority;
    QString mMajority;

    void testString( const QString &string );
};

#endif // QGSSTRINGSTATISTICALSUMMARY_H
