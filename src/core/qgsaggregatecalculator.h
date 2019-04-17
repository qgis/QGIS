/***************************************************************************
                             qgsaggregatecalculator.h
                             ------------------------
    begin                : May 2016
    copyright            : (C) 2016 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAGGREGATECALCULATOR_H
#define QGSAGGREGATECALCULATOR_H

#include "qgis_core.h"
#include "qgsstatisticalsummary.h"
#include "qgsdatetimestatisticalsummary.h"
#include "qgsstringstatisticalsummary.h"
#include <QVariant>


class QgsFeatureIterator;
class QgsExpression;
class QgsVectorLayer;
class QgsExpressionContext;

/**
 * \ingroup core
 * \class QgsAggregateCalculator
 * \brief Utility class for calculating aggregates for a field (or expression) over the features
 * from a vector layer. It is recommended that QgsVectorLayer::aggregate() is used rather then
 * directly using this class, as the QgsVectorLayer method can handle delegating aggregate calculation
 * to a data provider for remote calculation.
 * \since QGIS 2.16
 */
class CORE_EXPORT QgsAggregateCalculator
{
  public:

    /**
     * Structured information about the available aggregates.
     *
     * \since QGIS 3.0
     */
    struct AggregateInfo
    {
      QString function; //!< The expression function
      QString name; //!< A translated, human readable name
      QSet<QVariant::Type> supportedTypes; //!< This aggregate function can only be used with these datatypes
    };

    /**
     * Available aggregates to calculate. Not all aggregates are available for all field
     * types.
     */
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
      StringConcatenate, //!< Concatenate values with a joining string (string fields only). Specify the delimiter using setDelimiter().
      GeometryCollect, //!< Create a multipart geometry from aggregated geometries
      ArrayAggregate, //!< Create an array of values
      StringConcatenateUnique //!< Concatenate unique values with a joining string (string fields only). Specify the delimiter using setDelimiter().
    };

    //! A bundle of parameters controlling aggregate calculation
    struct AggregateParameters
    {

      /**
       * Optional filter for calculating aggregate over a subset of features, or an
       * empty string to use all features.
       * \see QgsAggregateCalculator::setFilter()
       * \see QgsAggregateCalculator::filter()
       */
      QString filter;

      /**
       * Delimiter to use for joining values with the StringConcatenate aggregate.
       * \see QgsAggregateCalculator::setDelimiter()
       * \see QgsAggregateCalculator::delimiter()
       */
      QString delimiter;
    };

    /**
     * Constructor for QgsAggregateCalculator.
     * \param layer vector layer to calculate aggregate from
     */
    QgsAggregateCalculator( const QgsVectorLayer *layer );

    /**
     * Returns the associated vector layer.
     */
    const QgsVectorLayer *layer() const;

    /**
     * Sets all aggregate parameters from a parameter bundle.
     * \param parameters aggregate parameters
     */
    void setParameters( const AggregateParameters &parameters );

    /**
     * Sets a filter to limit the features used during the aggregate calculation.
     * \param filterExpression expression for filtering features, or empty string to remove filter
     * \see filter()
     */
    void setFilter( const QString &filterExpression ) { mFilterExpression = filterExpression; }

    /**
     * Returns the filter which limits the features used during the aggregate calculation.
     * \see setFilter()
     */
    QString filter() const { return mFilterExpression; }

    /**
     * Sets the delimiter to use for joining values with the StringConcatenate aggregate.
     * \param delimiter string delimiter
     * \see delimiter()
     */
    void setDelimiter( const QString &delimiter ) { mDelimiter = delimiter; }

    /**
     * Returns the delimiter used for joining values with the StringConcatenate aggregate.
     * \see setDelimiter()
     */
    QString delimiter() const { return mDelimiter; }

    /**
     * Calculates the value of an aggregate.
     * \param aggregate aggregate to calculate
     * \param fieldOrExpression source field or expression to use as basis for aggregated values.
     * If an expression is used, then the context parameter must be set.
     * \param context expression context for evaluating expressions
     * \param ok if specified, will be set to TRUE if aggregate calculation was successful
     * \returns calculated aggregate value
     */
    QVariant calculate( Aggregate aggregate, const QString &fieldOrExpression,
                        QgsExpressionContext *context = nullptr, bool *ok = nullptr ) const;

    /**
     * Converts a string to a aggregate type.
     * \param string string to convert
     * \param ok if specified, will be set to TRUE if conversion was successful
     * \returns aggregate type
     */
    static Aggregate stringToAggregate( const QString &string, bool *ok = nullptr );

    /**
     * Structured information for available aggregates.
     *
     * \since QGIS 3.2
     */
    static QList< QgsAggregateCalculator::AggregateInfo > aggregates();

  private:

    //! Source layer
    const QgsVectorLayer *mLayer = nullptr;

    //! Filter expression, or empty for no filter
    QString mFilterExpression;

    //! Delimiter to use for concatenate aggregate
    QString mDelimiter;

    static QgsStatisticalSummary::Statistic numericStatFromAggregate( Aggregate aggregate, bool *ok = nullptr );
    static QgsStringStatisticalSummary::Statistic stringStatFromAggregate( Aggregate aggregate, bool *ok = nullptr );
    static QgsDateTimeStatisticalSummary::Statistic dateTimeStatFromAggregate( Aggregate aggregate, bool *ok = nullptr );

    static QVariant calculateNumericAggregate( QgsFeatureIterator &fit, int attr, QgsExpression *expression,
        QgsExpressionContext *context, QgsStatisticalSummary::Statistic stat );

    static QVariant calculateStringAggregate( QgsFeatureIterator &fit, int attr, QgsExpression *expression,
        QgsExpressionContext *context, QgsStringStatisticalSummary::Statistic stat );

    static QVariant calculateDateTimeAggregate( QgsFeatureIterator &fit, int attr, QgsExpression *expression,
        QgsExpressionContext *context, QgsDateTimeStatisticalSummary::Statistic stat );
    static QVariant calculateGeometryAggregate( QgsFeatureIterator &fit, QgsExpression *expression, QgsExpressionContext *context );

    static QVariant calculateArrayAggregate( QgsFeatureIterator &fit, int attr, QgsExpression *expression,
        QgsExpressionContext *context );

    static QVariant calculate( Aggregate aggregate, QgsFeatureIterator &fit, QVariant::Type resultType,
                               int attr, QgsExpression *expression,
                               const QString &delimiter,
                               QgsExpressionContext *context, bool *ok = nullptr );
    static QVariant concatenateStrings( QgsFeatureIterator &fit, int attr, QgsExpression *expression,
                                        QgsExpressionContext *context, const QString &delimiter, bool unique = false );

    QVariant defaultValue( Aggregate aggregate ) const;
};

#endif //QGSAGGREGATECALCULATOR_H

