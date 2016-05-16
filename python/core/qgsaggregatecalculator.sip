/** \ingroup core
 * \class QgsAggregateCalculator
 * \brief Utility class for calculating aggregates for a field (or expression) over the features
 * from a vector layer. It is recommended that QgsVectorLayer::aggregate() is used rather then
 * directly using this class, as the QgsVectorLayer method can handle delegating aggregate calculation
 * to a data provider for remote calculation.
 * \note added in QGIS 2.16
 */
class QgsAggregateCalculator
{

%TypeHeaderCode
#include <qgsaggregatecalculator.h>
%End

  public:

    //! Available aggregates to calculate. Not all aggregates are available for all field
    //! types.
    enum Aggregate
    {
      Count,  //!< Count
      CountDistinct,  //!< Number of distinct values
      CountMissing,  //!< Number of missing (null) values
      Min,  //!< Min of values
      Max,  //!< Max of values
      Sum,  //!< Sum of values
      Mean,  //!< Mean of values (numeric fields only)
      Median, //!< Median of values (numeric fields only)
      StDev, //!< Standard deviation of values (numeric fields only)
      StDevSample, //!< Sample standard deviation of values (numeric fields only)
      Range, //!< Range of values (max - min) (numeric and datetime fields only)
      Minority, //!< Minority of values (numeric fields only)
      Majority, //!< Majority of values (numeric fields only)
      FirstQuartile, //!< First quartile (numeric fields only)
      ThirdQuartile, //!< Third quartile (numeric fields only)
      InterQuartileRange, //!< Inter quartile range (IQR) (numeric fields only)
      StringMinimumLength, //!< Minimum length of string (string fields only)
      StringMaximumLength, //!< Maximum length of string (string fields only)
      StringConcatenate, //! Concatenate values with a joining string (string fields only). Specify the delimiter using setDelimiter().
    };

    //! A bundle of parameters controlling aggregate calculation
    struct AggregateParameters
    {
      /** Optional filter for calculating aggregate over a subset of features, or an
       * empty string to use all features.
       * @see QgsAggregateCalculator::setFilter()
       * @see QgsAggregateCalculator::filter()
       */
      QString filter;

      /** Delimiter to use for joining values with the StringConcatenate aggregate.
       * @see QgsAggregateCalculator::setDelimiter()
       * @see QgsAggregateCalculator::delimiter()
       */
      QString delimiter;
    };

    /** Constructor for QgsAggregateCalculator.
     * @param layer vector layer to calculate aggregate from
     */
    QgsAggregateCalculator( QgsVectorLayer* layer );

    /** Returns the associated vector layer.
     */
    QgsVectorLayer* layer() const;

    /** Sets all aggregate parameters from a parameter bundle.
     * @param parameters aggregate parameters
     */
    void setParameters( const AggregateParameters& parameters );

    /** Sets a filter to limit the features used during the aggregate calculation.
     * @param filterExpression expression for filtering features, or empty string to remove filter
     * @see filter()
     */
    void setFilter( const QString& filterExpression );

    /** Returns the filter which limits the features used during the aggregate calculation.
     * @see setFilter()
     */
    QString filter() const;

    /** Sets the delimiter to use for joining values with the StringConcatenate aggregate.
     * @param delimiter string delimiter
     * @see delimiter()
     */
    void setDelimiter( const QString& delimiter );

    /** Returns the delimiter used for joining values with the StringConcatenate aggregate.
     * @see setDelimiter()
     */
    QString delimiter() const;

    /** Calculates the value of an aggregate.
     * @param aggregate aggregate to calculate
     * @param fieldOrExpression source field or expression to use as basis for aggregated values.
     * If an expression is used, then the context parameter must be set.
     * @param context expression context for evaluating expressions
     * @param ok if specified, will be set to true if aggregate calculation was successful
     * @returns calculated aggregate value
     */
    QVariant calculate( Aggregate aggregate, const QString& fieldOrExpression,
                        QgsExpressionContext* context = nullptr, bool* ok = nullptr ) const;

    /** Converts a string to a aggregate type.
     * @param string string to convert
     * @param ok if specified, will be set to true if conversion was successful
     * @returns aggregate type
     */
    static Aggregate stringToAggregate( const QString& string, bool* ok = nullptr );

};
