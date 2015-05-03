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
      All = Count | Sum | Mean | Median | StDev | Max | Min | Range | Minority | Majority | Variety | FirstQuartile | ThirdQuartile | InterQuartileRange
    };
    Q_DECLARE_FLAGS( Statistics, Statistic )

    /** Constructor for QgsStatisticalSummary
     * @param stats flags for statistics to calculate
     */
    QgsStatisticalSummary( Statistics stats = Statistics( 0 ) );

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
    void setStatistics( Statistics stats ) { mStatistics = stats; }

    /** Resets the calculated values
     */
    void reset();

    /** Calculates summary statistics for a list of values
     * @param values list of doubles
     */
    void calculate( const QList<double>& values );

    /** Returns calculated count of values
     */
    int count() const { return mCount; }

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

  private:

    Statistics mStatistics;

    int mCount;
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
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsStatisticalSummary::Statistics )

#endif // QGSSTATISTICALSUMMARY_H
