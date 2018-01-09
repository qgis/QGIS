/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsaggregatecalculator.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsAggregateCalculator
{
%Docstring
 Utility class for calculating aggregates for a field (or expression) over the features
from a vector layer. It is recommended that QgsVectorLayer.aggregate() is used rather then
directly using this class, as the QgsVectorLayer method can handle delegating aggregate calculation
to a data provider for remote calculation.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgsaggregatecalculator.h"
%End
  public:

    struct AggregateInfo
    {
      QString function; //!< The expression function
      QString name; //!< A translated, human readable name
      QSet<QVariant::Type> supportedTypes; //!< This aggregate function can only be used with these datatypes
    };

    enum Aggregate
    {
      Count,
      CountDistinct,
      CountMissing,
      Min,
      Max,
      Sum,
      Mean,
      Median,
      StDev,
      StDevSample,
      Range,
      Minority,
      Majority,
      FirstQuartile,
      ThirdQuartile,
      InterQuartileRange,
      StringMinimumLength,
      StringMaximumLength,
      StringConcatenate,
      GeometryCollect,
      ArrayAggregate
    };

    struct AggregateParameters
    {

      QString filter;

      QString delimiter;
    };

    QgsAggregateCalculator( const QgsVectorLayer *layer );
%Docstring
Constructor for QgsAggregateCalculator.

:param layer: vector layer to calculate aggregate from
%End

    const QgsVectorLayer *layer() const;
%Docstring
Returns the associated vector layer.
%End

    void setParameters( const AggregateParameters &parameters );
%Docstring
Sets all aggregate parameters from a parameter bundle.

:param parameters: aggregate parameters
%End

    void setFilter( const QString &filterExpression );
%Docstring
Sets a filter to limit the features used during the aggregate calculation.

:param filterExpression: expression for filtering features, or empty string to remove filter

.. seealso:: :py:func:`filter`
%End

    QString filter() const;
%Docstring
Returns the filter which limits the features used during the aggregate calculation.

.. seealso:: :py:func:`setFilter`
%End

    void setDelimiter( const QString &delimiter );
%Docstring
Sets the delimiter to use for joining values with the StringConcatenate aggregate.

:param delimiter: string delimiter

.. seealso:: :py:func:`delimiter`
%End

    QString delimiter() const;
%Docstring
Returns the delimiter used for joining values with the StringConcatenate aggregate.

.. seealso:: :py:func:`setDelimiter`
%End

    QVariant calculate( Aggregate aggregate, const QString &fieldOrExpression,
                        QgsExpressionContext *context = 0, bool *ok = 0 ) const;
%Docstring
Calculates the value of an aggregate.

:param aggregate: aggregate to calculate
:param fieldOrExpression: source field or expression to use as basis for aggregated values.
If an expression is used, then the context parameter must be set.
:param context: expression context for evaluating expressions
:param ok: if specified, will be set to true if aggregate calculation was successful

:return: calculated aggregate value
%End

    static Aggregate stringToAggregate( const QString &string, bool *ok = 0 );
%Docstring
Converts a string to a aggregate type.

:param string: string to convert
:param ok: if specified, will be set to true if conversion was successful

:return: aggregate type
%End

    static QList< QgsAggregateCalculator::AggregateInfo > aggregates();
%Docstring
Structured information for available aggregates.

.. versionadded:: 3.2
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsaggregatecalculator.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
