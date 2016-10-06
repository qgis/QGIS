/***************************************************************************
  qgsstatisticalsummary.h
  --------------------------------------
  Date                 : May 2015
  Copyright            : (C) 2015 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTATISTICALSUMMARY_H
#define QGSSTATISTICALSUMMARY_H

#include <QMap>
#include <QVariant>

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests in testqgsstatisticalsummary.cpp.
 * See details in QEP #17
 ****************************************************************************/

/** \ingroup core
 * \class QgsStatisticalSummary
 * \brief Calculator for summary statistics for a list of doubles.
 *
 * Statistics are calculated by calling @link calculate @endlink and passing a list of doubles. The
 * individual statistics can then be retrieved using the associated methods. Note that not all statistics
 * are calculated by default. Statistics which require slower computations are only calculated by
 * specifying the statistic in the constructor or via @link setStatistics @endlink.
 *
 * \note Added in version 2.9
 */

class CORE_EXPORT QgsStatisticalSummary
{
  public:

    //! Enumeration of flags that specify statistics to be calculated
    enum Statistic
    {
      Count = 1,  //!< Count
      CountMissing = 32770, //!< Number of missing (null) values
      Sum = 2,  //!< Sum of values
      Mean = 4,  //!< Mean of values
      Median = 8, //!< Median of values
      StDev = 16, //!< Standard deviation of values
      StDevSample = 32, //!< Sample standard deviation of values
      Min = 64,  //!< Min of values
      Max = 128,  //!< Max of values
      Range = 256, //!< Range of values (max - min)
      Minority = 512, //!< Minority of values
      Majority = 1024, //!< Majority of values
      Variety = 2048, //!< Variety (count of distinct) values
      FirstQuartile = 4096, //!< First quartile
      ThirdQuartile = 8192, //!< Third quartile
      InterQuartileRange = 16384, //!< Inter quartile range (IQR)
      All = Count | CountMissing | Sum | Mean | Median | StDev | Max | Min | Range | Minority | Majority | Variety | FirstQuartile | ThirdQuartile | InterQuartileRange
    };
    Q_DECLARE_FLAGS( Statistics, Statistic )

    /** Constructor for QgsStatisticalSummary
     * @param stats flags for statistics to calculate
     */
    QgsStatisticalSummary( const QgsStatisticalSummary::Statistics& stats = All );

    virtual ~QgsStatisticalSummary();

    /** Returns flags which specify which statistics will be calculated. Some statistics
     * are always calculated (eg sum, min and max).
     * @see setStatistics
     */
    Statistics statistics() const { return mStatistics; }

    /** Sets flags which specify which statistics will be calculated. Some statistics
     * are always calculated (eg sum, min and max).
     * @param stats flags for statistics to calculate
     * @see statistics
     */
    void setStatistics( const Statistics& stats ) { mStatistics = stats; }

    /** Resets the calculated values
     */
    void reset();

    /** Calculates summary statistics for a list of values
     * @param values list of doubles
     */
    void calculate( const QList<double>& values );

    /** Adds a single value to the statistics calculation. Calling this method
     * allows values to be added to the calculation one at a time. For large
     * quantities of values this may be more efficient then first adding all the
     * values to a list and calling calculate().
     * @param value value to add
     * @note call reset() before adding the first value using this method
     * to clear the results from any previous calculations
     * @note finalize() must be called after adding the final value and before
     * retrieving calculated statistics.
     * @see calculate()
     * @see addVariant()
     * @see finalize()
     * @note added in QGIS 2.16
     */
    void addValue( double value );

    /** Adds a single value to the statistics calculation. Calling this method
     * allows values to be added to the calculation one at a time. For large
     * quantities of values this may be more efficient then first adding all the
     * values to a list and calling calculate().
     * @param value variant containing to add. Non-numeric values are treated as null.
     * @note call reset() before adding the first value using this method
     * to clear the results from any previous calculations
     * @note finalize() must be called after adding the final value and before
     * retrieving calculated statistics.
     * @see addValue()
     * @see calculate()
     * @see finalize()
     * @note added in QGIS 2.16
     */
    void addVariant( const QVariant& value );

    /** Must be called after adding all values with addValues() and before retrieving
     * any calculated statistics.
     * @see addValue()
     * @see addVariant()
     * @note added in QGIS 2.16
     */
    void finalize();

    /** Returns the value of a specified statistic
     * @param stat statistic to return
     * @returns calculated value of statistic
     */
    double statistic( Statistic stat ) const;

    /** Returns calculated count of values
     */
    int count() const { return mCount; }

    /** Returns the number of missing (null) values
     * @note added in QGIS 2.16
     */
    int countMissing() const { return mMissing; }

    /** Returns calculated sum of values
     */
    double sum() const { return mSum; }

    /** Returns calculated mean of values
     */
    double mean() const { return mMean; }

    /** Returns calculated median of values. This is only calculated if Statistic::Median has
     * been specified in the constructor or via setStatistics.
     */
    double median() const { return mMedian; }

    /** Returns calculated minimum from values.
     */
    double min() const { return mMin; }

    /** Returns calculated maximum from values.
     */
    double max() const { return mMax; }

    /** Returns calculated range (difference between maximum and minimum values).
     */
    double range() const { return mMax - mMin; }

    /** Returns population standard deviation. This is only calculated if Statistic::StDev has
     * been specified in the constructor or via setStatistics.
     * @see sampleStDev
     */
    double stDev() const { return mStdev; }

    /** Returns sample standard deviation. This is only calculated if Statistic::StDev has
     * been specified in the constructor or via setStatistics.
     * @see stDev
     */
    double sampleStDev() const { return mSampleStdev; }

    /** Returns variety of values. The variety is the count of unique values from the list.
     * This is only calculated if Statistic::Variety has been specified in the constructor
     * or via setStatistics.
     */
    int variety() const { return mValueCount.count(); }

    /** Returns minority of values. The minority is the value with least occurances in the list
     * This is only calculated if Statistic::Minority has been specified in the constructor
     * or via setStatistics.
     * @see majority
     */
    double minority() const { return mMinority; }

    /** Returns majority of values. The majority is the value with most occurances in the list
     * This is only calculated if Statistic::Majority has been specified in the constructor
     * or via setStatistics.
     * @see minority
     */
    double majority() const { return mMajority; }

    /** Returns the first quartile of the values. The quartile is calculated using the
     * "Tukey's hinges" method.
     * @see thirdQuartile
     * @see interQuartileRange
     */
    double firstQuartile() const { return mFirstQuartile; }

    /** Returns the third quartile of the values. The quartile is calculated using the
     * "Tukey's hinges" method.
     * @see firstQuartile
     * @see interQuartileRange
     */
    double thirdQuartile() const { return mThirdQuartile; }

    /** Returns the inter quartile range of the values. The quartiles are calculated using the
     * "Tukey's hinges" method.
     * @see firstQuartile
     * @see thirdQuartile
     */
    double interQuartileRange() const { return mThirdQuartile - mFirstQuartile; }

    /** Returns the friendly display name for a statistic
     * @param statistic statistic to return name for
     */
    static QString displayName( Statistic statistic );

  private:

    Statistics mStatistics;

    int mCount;
    int mMissing;
    double mSum;
    double mMean;
    double mMedian;
    double mMin;
    double mMax;
    double mStdev;
    double mSampleStdev;
    double mMinority;
    double mMajority;
    double mFirstQuartile;
    double mThirdQuartile;
    QMap< double, int > mValueCount;
    QList< double > mValues;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsStatisticalSummary::Statistics )

#endif // QGSSTATISTICALSUMMARY_H
