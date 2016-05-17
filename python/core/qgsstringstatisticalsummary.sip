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

    /** Calculates summary statistics for a list of strings.
     * @param values list of strings
     * @see calculateFromVariants()
     */
    void calculate( const QStringList& values );

    /** Calculates summary statistics for a list of variants. Any non-string variants will be
     * ignored.
     * @param values list of variants
     * @see calculate()
     */
    void calculateFromVariants( const QVariantList& values );

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

