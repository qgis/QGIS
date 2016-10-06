/** \ingroup core
 * \class QgsStringStatisticalSummary
 * \brief Calculator for summary statistics and aggregates for a list of strings.
 *
 * Statistics are calculated by calling @link calculate @endlink and passing a list of strings. The
 * individual statistics can then be retrieved using the associated methods. Note that not all statistics
 * are calculated by default. Statistics which require slower computations are only calculated by
 * specifying the statistic in the constructor or via @link setStatistics @endlink.
 *
 * \note Added in version 2.16
 */


class QgsStringStatisticalSummary
{
%TypeHeaderCode
#include <qgsstringstatisticalsummary.h>
%End

  public:

    public:

    //! Enumeration of flags that specify statistics to be calculated
    enum Statistic
    {
      Count,  //!< Count
      CountDistinct,  //!< Number of distinct string values
      CountMissing,  //!< Number of missing (null) values
      Min, //!< Minimum string value
      Max, //!< Maximum string value
      MinimumLength, //!< Minimum length of string
      MaximumLength, //!< Maximum length of string
      All, //! All statistics
    };
    typedef QFlags<QgsStringStatisticalSummary::Statistic> Statistics;

    /** Constructor for QgsStringStatistics
     * @param stats flags for statistics to calculate
     */
    QgsStringStatisticalSummary( const QgsStringStatisticalSummary::Statistics& stats = All );

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

    /** Calculates summary statistics for an entire list of strings at once.
     * @param values list of strings
     * @see calculateFromVariants()
     * @see addString()
     */
    void calculate( const QStringList& values );

    /** Calculates summary statistics for an entire list of variants at once. Any
     * non-string variants will be ignored.
     * @param values list of variants
     * @see calculate()
     * @see addValue()
     */
    void calculateFromVariants( const QVariantList& values );

    /** Adds a single string to the statistics calculation. Calling this method
     * allows strings to be added to the calculation one at a time. For large
     * quantities of strings this may be more efficient then first adding all the
     * strings to a list and calling calculate().
     * @param string string to add
     * @note call reset() before adding the first string using this method
     * to clear the results from any previous calculations
     * @note finalize() must be called after adding the final string and before
     * retrieving calculated statistics.
     * @see calculate()
     * @see addValue()
     * @see finalize()
     */
    void addString( const QString& string );

    /** Adds a single variant to the statistics calculation. Calling this method
     * allows variants to be added to the calculation one at a time. For large
     * quantities of variants this may be more efficient then first adding all the
     * variants to a list and calling calculateFromVariants().
     * @param value variant to add
     * @note call reset() before adding the first string using this method
     * to clear the results from any previous calculations
     * @note finalize() must be called after adding the final value and before
     * retrieving calculated statistics.
     * @see calculateFromVariants()
     * @see finalize()
     */
    void addValue( const QVariant& value );

    /** Must be called after adding all strings with addString() and before retrieving
     * any calculated string statistics.
     * @see addString()
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

    /** Returns the number of distinct string values.
     * @see distinctValues()
     */
    int countDistinct() const;

    /** Returns the set of distinct string values.
     * @see countDistinct()
     */
    QSet< QString > distinctValues() const;

    /** Returns the number of missing (null) string values.
     */
    int countMissing() const;

    /** Returns the minimum (non-null) string value.
     */
    QString min() const;

    /** Returns the maximum (non-null) string value.
     */
    QString max() const;

    /** Returns the minimum length of strings.
     */
    int minLength() const;

    /** Returns the maximum length of strings.
     */
    int maxLength() const;

    /** Returns the friendly display name for a statistic
     * @param statistic statistic to return name for
     */
    static QString displayName( Statistic statistic );

};

QFlags<QgsStringStatisticalSummary::Statistic> operator|(QgsStringStatisticalSummary::Statistic f1, QFlags<QgsStringStatisticalSummary::Statistic> f2);

