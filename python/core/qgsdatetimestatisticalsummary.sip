/** \ingroup core
 * \class QgsDateTimeStatisticalSummary
 * \brief Calculator for summary statistics and aggregates for a list of datetimes.
 *
 * Statistics are calculated by calling @link calculate @endlink and passing a list of datetimes. The
 * individual statistics can then be retrieved using the associated methods. Note that not all statistics
 * are calculated by default. Statistics which require slower computations are only calculated by
 * specifying the statistic in the constructor or via @link setStatistics @endlink.
 *
 * \note Added in version 2.16
 */

class QgsDateTimeStatisticalSummary
{
%TypeHeaderCode
#include <qgsdatetimestatisticalsummary.h>
%End

  public:

    //! Enumeration of flags that specify statistics to be calculated
    enum Statistic
    {
      Count,  //!< Count
      CountDistinct,  //!< Number of distinct datetime values
      CountMissing,  //!< Number of missing (null) values
      Min, //!< Minimum (earliest) datetime value
      Max, //!< Maximum (latest) datetime value
      Range, //!< Interval between earliest and latest datetime value
      All, //! All statistics
    };
    typedef QFlags<QgsDateTimeStatisticalSummary::Statistic> Statistics;

    /** Constructor for QgsDateTimeStatisticalSummary
     * @param stats flags for statistics to calculate
     */
    QgsDateTimeStatisticalSummary( const QgsDateTimeStatisticalSummary::Statistics& stats = All );

    /** Returns flags which specify which statistics will be calculated. Some statistics
     * are always calculated (eg count).
     * @see setStatistics
     */
    Statistics statistics() const;

    /** Sets flags which specify which statistics will be calculated. Some statistics
     * are always calculated (eg count).
     * @param stats flags for statistics to calculate
     * @see statistics
     */
    void setStatistics( const Statistics& stats );

    /** Resets the calculated values
     */
    void reset();

    /** Calculates summary statistics for a list of variants. Any non-datetime variants will be
     * ignored.
     * @param values list of variants
     * @see addValue()
     */
    void calculate( const QVariantList& values );

    /** Adds a single datetime to the statistics calculation. Calling this method
     * allows datetimes to be added to the calculation one at a time. For large
     * quantities of dates this may be more efficient then first adding all the
     * variants to a list and calling calculate().
     * @param value datetime to add. Any non-datetime variants will be ignored.
     * @note call reset() before adding the first datetime using this method
     * to clear the results from any previous calculations
     * @note finalize() must be called after adding the final value and before
     * retrieving calculated statistics.
     * @see calculate()
     * @see finalize()
     */
    void addValue( const QVariant& value );

    /** Must be called after adding all datetimes with addValue() and before retrieving
     * any calculated datetime statistics.
     * @see addValue()
     */
    void finalize();

    /** Returns the value of a specified statistic
     * @param stat statistic to return
     * @returns calculated value of statistic
     */
    QVariant statistic( Statistic stat ) const;

    /** Returns the calculated count of values.
     */
    int count() const;

    /** Returns the number of distinct datetime values.
     */
    int countDistinct() const;

    /** Returns the set of distinct datetime values.
     */
    QSet< QDateTime > distinctValues() const;

    /** Returns the number of missing (null) datetime values.
     */
    int countMissing() const;

    /** Returns the minimum (earliest) non-null datetime value.
     */
    QDateTime min() const;

    /** Returns the maximum (latest) non-null datetime value.
     */
    QDateTime max() const;

    /** Returns the range (interval between earliest and latest non-null datetime values).
     */
    QgsInterval range() const;

    /** Returns the friendly display name for a statistic
     * @param statistic statistic to return name for
     */
    static QString displayName( Statistic statistic );

};

QFlags<QgsDateTimeStatisticalSummary::Statistic> operator|(QgsDateTimeStatisticalSummary::Statistic f1, QFlags<QgsDateTimeStatisticalSummary::Statistic> f2);

