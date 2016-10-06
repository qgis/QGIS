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

class QgsStatisticalSummary
{
%TypeHeaderCode
#include <qgsstatisticalsummary.h>
%End

  public:

    //! Enumeration of flags that specify statistics to be calculated
    enum Statistic
    {
      Count,  //!< Count
      CountMissing, //!< Number of missing (null) values
      Sum,  //!< Sum of values
      Mean,  //!< Mean of values
      Median, //!< Median of values
      StDev, //!< Standard deviation of values
      StDevSample, //!< Sample standard deviation of values
      Min,  //!< Min of values
      Max,  //!< Max of values
      Range, //!< Range of values (max - min)
      Minority, //!< Minority of values
      Majority, //!< Majority of values
      Variety, //!< Variety (count of distinct) values
      FirstQuartile, //!< First quartile
      ThirdQuartile, //!< Third quartile
      InterQuartileRange, //!< Inter quartile range (IQR)
      All
    };
    typedef QFlags<QgsStatisticalSummary::Statistic> Statistics;

    /** Constructor for QgsStatisticalSummary
     * @param stats flags for statistics to calculate
     */
    QgsStatisticalSummary( const QgsStatisticalSummary::Statistics& stats = QgsStatisticalSummary::All );

    virtual ~QgsStatisticalSummary();

    /** Returns flags which specify which statistics will be calculated. Some statistics
     * are always calculated (eg sum, min and max).
     * @see setStatistics
     */
    QgsStatisticalSummary::Statistics statistics() const;

    /** Sets flags which specify which statistics will be calculated. Some statistics
     * are always calculated (eg sum, min and max).
     * @param stats flags for statistics to calculate
     * @see statistics
     */
    void setStatistics( const QgsStatisticalSummary::Statistics& stats );

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
    int count() const;

    /** Returns the number of missing (null) values
     * @note added in QGIS 2.16
     */
    int countMissing() const;

    /** Returns calculated sum of values
     */
    double sum() const;

    /** Returns calculated mean of values
     */
    double mean() const;

    /** Returns calculated median of values. This is only calculated if Statistic::Median has
     * been specified in the constructor or via setStatistics.
     */
    double median() const;

    /** Returns calculated minimum from values.
     */
    double min() const;

    /** Returns calculated maximum from values.
     */
    double max() const;

    /** Returns calculated range (difference between maximum and minimum values).
     */
    double range() const;

    /** Returns population standard deviation. This is only calculated if Statistic::StDev has
     * been specified in the constructor or via setStatistics.
     * @see sampleStDev
     */
    double stDev() const;

    /** Returns sample standard deviation. This is only calculated if Statistic::StDev has
     * been specified in the constructor or via setStatistics.
     * @see stDev
     */
    double sampleStDev() const;

    /** Returns variety of values. The variety is the count of unique values from the list.
     * This is only calculated if Statistic::Variety has been specified in the constructor
     * or via setStatistics.
     */
    int variety() const;

    /** Returns minority of values. The minority is the value with least occurances in the list
     * This is only calculated if Statistic::Minority has been specified in the constructor
     * or via setStatistics.
     * @see majority
     */
    double minority() const;

    /** Returns majority of values. The majority is the value with most occurances in the list
     * This is only calculated if Statistic::Majority has been specified in the constructor
     * or via setStatistics.
     * @see minority
     */
    double majority() const;

    /** Returns the first quartile of the values. The quartile is calculated using the
     * "Tukey's hinges" method.
     * @see thirdQuartile
     * @see interQuartileRange
     */
    double firstQuartile() const;

    /** Returns the third quartile of the values. The quartile is calculated using the
     * "Tukey's hinges" method.
     * @see firstQuartile
     * @see interQuartileRange
     */
    double thirdQuartile() const;

    /** Returns the inter quartile range of the values. The quartiles are calculated using the
     * "Tukey's hinges" method.
     * @see firstQuartile
     * @see thirdQuartile
     */
    double interQuartileRange() const;

    /** Returns the friendly display name for a statistic
     * @param statistic statistic to return name for
     */
    static QString displayName( Statistic statistic );

};

QFlags<QgsStatisticalSummary::Statistic> operator|(QgsStatisticalSummary::Statistic f1, QFlags<QgsStatisticalSummary::Statistic> f2);

