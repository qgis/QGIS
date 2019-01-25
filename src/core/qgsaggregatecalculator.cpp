/***************************************************************************
                             qgsaggregatecalculator.cpp
                             --------------------------
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

#include "qgsaggregatecalculator.h"
#include "qgsfeature.h"
#include "qgsfeaturerequest.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgsvectorlayer.h"



QgsAggregateCalculator::QgsAggregateCalculator( const QgsVectorLayer *layer )
  : mLayer( layer )
{

}

const QgsVectorLayer *QgsAggregateCalculator::layer() const
{
  return mLayer;
}

void QgsAggregateCalculator::setParameters( const AggregateParameters &parameters )
{
  mFilterExpression = parameters.filter;
  mDelimiter = parameters.delimiter;
}

QVariant QgsAggregateCalculator::calculate( QgsAggregateCalculator::Aggregate aggregate,
    const QString &fieldOrExpression,
    QgsExpressionContext *context, bool *ok,
    const QgsFeatureIds ids ) const
{
  if ( ok )
    *ok = false;

  if ( !mLayer )
    return QVariant();

  QgsExpressionContext defaultContext = mLayer->createExpressionContext();
  context = context ? context : &defaultContext;

  std::unique_ptr<QgsExpression> expression;

  int attrNum = mLayer->fields().lookupField( fieldOrExpression );

  if ( attrNum == -1 )
  {
    Q_ASSERT( context );
    context->setFields( mLayer->fields() );
    // try to use expression
    expression.reset( new QgsExpression( fieldOrExpression ) );

    if ( expression->hasParserError() || !expression->prepare( context ) )
    {
      return QVariant();
    }
  }

  QSet<QString> lst;
  if ( !expression )
    lst.insert( fieldOrExpression );
  else
    lst = expression->referencedColumns();

  QgsFeatureRequest request = QgsFeatureRequest()
                              .setFlags( ( expression && expression->needsGeometry() ) ?
                                         QgsFeatureRequest::NoFlags :
                                         QgsFeatureRequest::NoGeometry )
                              .setSubsetOfAttributes( lst, mLayer->fields() );
  if ( !mFilterExpression.isEmpty() )
    request.setFilterExpression( mFilterExpression );
  if ( context )
    request.setExpressionContext( *context );
  if ( !ids.empty() )
    request.setFilterFids( ids );
  //determine result type
  QVariant::Type resultType = QVariant::Double;
  if ( attrNum == -1 )
  {
    // evaluate first feature, check result type
    QgsFeatureRequest testRequest( request );
    testRequest.setLimit( 1 );
    QgsFeature f;
    QgsFeatureIterator fit = mLayer->getFeatures( testRequest );
    if ( !fit.nextFeature( f ) )
    {
      //no matching features
      if ( ok )
        *ok = true;
      return defaultValue( aggregate );
    }

    if ( context )
      context->setFeature( f );
    QVariant v = expression->evaluate( context );
    resultType = v.type();
  }
  else
  {
    resultType = mLayer->fields().at( attrNum ).type();
  }

  QgsFeatureIterator fit = mLayer->getFeatures( request );
  return calculate( aggregate, fit, resultType, attrNum, expression.get(), mDelimiter, context, ok );
}

QgsAggregateCalculator::Aggregate QgsAggregateCalculator::stringToAggregate( const QString &string, bool *ok )
{
  QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == QLatin1String( "count" ) )
    return Count;
  else if ( normalized == QLatin1String( "count_distinct" ) )
    return CountDistinct;
  else if ( normalized == QLatin1String( "count_missing" ) )
    return CountMissing;
  else if ( normalized == QLatin1String( "min" ) )
    return Min;
  else if ( normalized == QLatin1String( "max" ) )
    return Max;
  else if ( normalized == QLatin1String( "sum" ) )
    return Sum;
  else if ( normalized == QLatin1String( "mean" ) )
    return Mean;
  else if ( normalized == QLatin1String( "median" ) )
    return Median;
  else if ( normalized == QLatin1String( "stdev" ) )
    return StDev;
  else if ( normalized == QLatin1String( "stdevsample" ) )
    return StDevSample;
  else if ( normalized == QLatin1String( "range" ) )
    return Range;
  else if ( normalized == QLatin1String( "minority" ) )
    return Minority;
  else if ( normalized == QLatin1String( "majority" ) )
    return Majority;
  else if ( normalized == QLatin1String( "q1" ) )
    return FirstQuartile;
  else if ( normalized == QLatin1String( "q3" ) )
    return ThirdQuartile;
  else if ( normalized == QLatin1String( "iqr" ) )
    return InterQuartileRange;
  else if ( normalized == QLatin1String( "min_length" ) )
    return StringMinimumLength;
  else if ( normalized == QLatin1String( "max_length" ) )
    return StringMaximumLength;
  else if ( normalized == QLatin1String( "concatenate" ) )
    return StringConcatenate;
  else if ( normalized == QLatin1String( "collect" ) )
    return GeometryCollect;
  else if ( normalized == QLatin1String( "array_agg" ) )
    return ArrayAggregate;

  if ( ok )
    *ok = false;

  return Count;
}

QList<QgsAggregateCalculator::AggregateInfo> QgsAggregateCalculator::aggregates()
{
  QList< AggregateInfo > aggregates;
  aggregates
      << AggregateInfo
  {
    QStringLiteral( "count" ),
    QCoreApplication::tr( "Count" ),
    QSet<QVariant::Type>()
        << QVariant::DateTime
        << QVariant::Date
        << QVariant::Int
        << QVariant::UInt
        << QVariant::LongLong
        << QVariant::ULongLong
        << QVariant::String
  }
      << AggregateInfo
  {
    QStringLiteral( "count_distinct" ),
    QCoreApplication::tr( "Count Distinct" ),
    QSet<QVariant::Type>()
        << QVariant::DateTime
        << QVariant::Date
        << QVariant::UInt
        << QVariant::Int
        << QVariant::LongLong
        << QVariant::ULongLong
        << QVariant::String
  }
      << AggregateInfo
  {
    QStringLiteral( "count_missing" ),
    QCoreApplication::tr( "Count Missing" ),
    QSet<QVariant::Type>()
        << QVariant::DateTime
        << QVariant::Date
        << QVariant::Int
        << QVariant::UInt
        << QVariant::LongLong
        << QVariant::String
  }
      << AggregateInfo
  {
    QStringLiteral( "min" ),
    QCoreApplication::tr( "Min" ),
    QSet<QVariant::Type>()
        << QVariant::DateTime
        << QVariant::Date
        << QVariant::Int
        << QVariant::UInt
        << QVariant::LongLong
        << QVariant::ULongLong
        << QVariant::Double
        << QVariant::String
  }
      << AggregateInfo
  {
    QStringLiteral( "max" ),
    QCoreApplication::tr( "Max" ),
    QSet<QVariant::Type>()
        << QVariant::DateTime
        << QVariant::Date
        << QVariant::Int
        << QVariant::UInt
        << QVariant::LongLong
        << QVariant::ULongLong
        << QVariant::Double
        << QVariant::String
  }
      << AggregateInfo
  {
    QStringLiteral( "sum" ),
    QCoreApplication::tr( "Sum" ),
    QSet<QVariant::Type>()
        << QVariant::Int
        << QVariant::UInt
        << QVariant::LongLong
        << QVariant::ULongLong
        << QVariant::Double
  }
      << AggregateInfo
  {
    QStringLiteral( "mean" ),
    QCoreApplication::tr( "Mean" ),
    QSet<QVariant::Type>()
        << QVariant::Int
        << QVariant::UInt
        << QVariant::LongLong
        << QVariant::ULongLong
        << QVariant::Double
  }
      << AggregateInfo
  {
    QStringLiteral( "median" ),
    QCoreApplication::tr( "Median" ),
    QSet<QVariant::Type>()
        << QVariant::Int
        << QVariant::UInt
        << QVariant::Double
  }
      << AggregateInfo
  {
    QStringLiteral( "stdev" ),
    QCoreApplication::tr( "Stdev" ),
    QSet<QVariant::Type>()
        << QVariant::Int
        << QVariant::UInt
        << QVariant::LongLong
        << QVariant::ULongLong
        << QVariant::Double
  }
      << AggregateInfo
  {
    QStringLiteral( "stdevsample" ),
    QCoreApplication::tr( "Stdev Sample" ),
    QSet<QVariant::Type>()
        << QVariant::Int
        << QVariant::UInt
        << QVariant::LongLong
        << QVariant::ULongLong
        << QVariant::Double
  }
      << AggregateInfo
  {
    QStringLiteral( "range" ),
    QCoreApplication::tr( "Range" ),
    QSet<QVariant::Type>()
        << QVariant::Date
        << QVariant::DateTime
        << QVariant::Int
        << QVariant::UInt
        << QVariant::LongLong
        << QVariant::ULongLong
        << QVariant::Double
  }
      << AggregateInfo
  {
    QStringLiteral( "minority" ),
    QCoreApplication::tr( "Minority" ),
    QSet<QVariant::Type>()
        << QVariant::Int
        << QVariant::UInt
        << QVariant::LongLong
        << QVariant::ULongLong
        << QVariant::Double
  }
      << AggregateInfo
  {
    QStringLiteral( "majority" ),
    QCoreApplication::tr( "Majority" ),
    QSet<QVariant::Type>()
        << QVariant::Int
        << QVariant::UInt
        << QVariant::LongLong
        << QVariant::ULongLong
        << QVariant::Double
  }
      << AggregateInfo
  {
    QStringLiteral( "q1" ),
    QCoreApplication::tr( "Q1" ),
    QSet<QVariant::Type>()
        << QVariant::Int
        << QVariant::UInt
        << QVariant::LongLong
        << QVariant::ULongLong
        << QVariant::Double
  }
      << AggregateInfo
  {
    QStringLiteral( "q3" ),
    QCoreApplication::tr( "Q3" ),
    QSet<QVariant::Type>()
        << QVariant::Int
        << QVariant::UInt
        << QVariant::LongLong
        << QVariant::ULongLong
        << QVariant::Double
  }
      << AggregateInfo
  {
    QStringLiteral( "iqr" ),
    QCoreApplication::tr( "InterQuartileRange" ),
    QSet<QVariant::Type>()
        << QVariant::Int
        << QVariant::UInt
        << QVariant::LongLong
        << QVariant::ULongLong
        << QVariant::Double
  }
      << AggregateInfo
  {
    QStringLiteral( "min_length" ),
    QCoreApplication::tr( "Min Length" ),
    QSet<QVariant::Type>()
        << QVariant::String
  }
      << AggregateInfo
  {
    QStringLiteral( "max_length" ),
    QCoreApplication::tr( "Max Length" ),
    QSet<QVariant::Type>()
        << QVariant::String
  }
      << AggregateInfo
  {
    QStringLiteral( "concatenate" ),
    QCoreApplication::tr( "Concatenate" ),
    QSet<QVariant::Type>()
        << QVariant::String
  }
      << AggregateInfo
  {
    QStringLiteral( "collect" ),
    QCoreApplication::tr( "Collect" ),
    QSet<QVariant::Type>()
  }
      << AggregateInfo
  {
    QStringLiteral( "array_agg" ),
    QCoreApplication::tr( "Array Aggregate" ),
    QSet<QVariant::Type>()
  };

  return aggregates;
}

QVariant QgsAggregateCalculator::calculate( QgsAggregateCalculator::Aggregate aggregate, QgsFeatureIterator &fit, QVariant::Type resultType,
    int attr, QgsExpression *expression, const QString &delimiter, QgsExpressionContext *context, bool *ok )
{
  if ( ok )
    *ok = false;

  if ( aggregate == QgsAggregateCalculator::ArrayAggregate )
  {
    if ( ok )
      *ok = true;
    return calculateArrayAggregate( fit, attr, expression, context );
  }

  switch ( resultType )
  {
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Double:
    {
      bool statOk = false;
      QgsStatisticalSummary::Statistic stat = numericStatFromAggregate( aggregate, &statOk );
      if ( !statOk )
        return QVariant();

      if ( ok )
        *ok = true;
      return calculateNumericAggregate( fit, attr, expression, context, stat );
    }

    case QVariant::Date:
    case QVariant::DateTime:
    {
      bool statOk = false;
      QgsDateTimeStatisticalSummary::Statistic stat = dateTimeStatFromAggregate( aggregate, &statOk );
      if ( !statOk )
        return QVariant();

      if ( ok )
        *ok = true;
      return calculateDateTimeAggregate( fit, attr, expression, context, stat );
    }

    case QVariant::UserType:
    {
      if ( aggregate == GeometryCollect )
      {
        if ( ok )
          *ok = true;
        return calculateGeometryAggregate( fit, expression, context );
      }
      else
      {
        return QVariant();
      }
    }

    default:
    {
      // treat as string
      if ( aggregate == StringConcatenate )
      {
        //special case
        if ( ok )
          *ok = true;
        return concatenateStrings( fit, attr, expression, context, delimiter );
      }

      bool statOk = false;
      QgsStringStatisticalSummary::Statistic stat = stringStatFromAggregate( aggregate, &statOk );
      if ( !statOk )
        return QVariant();

      if ( ok )
        *ok = true;
      return calculateStringAggregate( fit, attr, expression, context, stat );
    }
  }

#ifndef _MSC_VER
  return QVariant();
#endif
}

QgsStatisticalSummary::Statistic QgsAggregateCalculator::numericStatFromAggregate( QgsAggregateCalculator::Aggregate aggregate, bool *ok )
{
  if ( ok )
    *ok = true;

  switch ( aggregate )
  {
    case Count:
      return QgsStatisticalSummary::Count;
    case CountDistinct:
      return QgsStatisticalSummary::Variety;
    case CountMissing:
      return QgsStatisticalSummary::CountMissing;
    case Min:
      return QgsStatisticalSummary::Min;
    case Max:
      return QgsStatisticalSummary::Max;
    case Sum:
      return QgsStatisticalSummary::Sum;
    case Mean:
      return QgsStatisticalSummary::Mean;
    case Median:
      return QgsStatisticalSummary::Median;
    case StDev:
      return QgsStatisticalSummary::StDev;
    case StDevSample:
      return QgsStatisticalSummary::StDevSample;
    case Range:
      return QgsStatisticalSummary::Range;
    case Minority:
      return QgsStatisticalSummary::Minority;
    case Majority:
      return QgsStatisticalSummary::Majority;
    case FirstQuartile:
      return QgsStatisticalSummary::FirstQuartile;
    case ThirdQuartile:
      return QgsStatisticalSummary::ThirdQuartile;
    case InterQuartileRange:
      return QgsStatisticalSummary::InterQuartileRange;
    case StringMinimumLength:
    case StringMaximumLength:
    case StringConcatenate:
    case GeometryCollect:
    case ArrayAggregate:
    {
      if ( ok )
        *ok = false;
      return QgsStatisticalSummary::Count;
    }
  }

  if ( ok )
    *ok = false;
  return QgsStatisticalSummary::Count;
}

QgsStringStatisticalSummary::Statistic QgsAggregateCalculator::stringStatFromAggregate( QgsAggregateCalculator::Aggregate aggregate, bool *ok )
{
  if ( ok )
    *ok = true;

  switch ( aggregate )
  {
    case Count:
      return QgsStringStatisticalSummary::Count;
    case CountDistinct:
      return QgsStringStatisticalSummary::CountDistinct;
    case CountMissing:
      return QgsStringStatisticalSummary::CountMissing;
    case Min:
      return QgsStringStatisticalSummary::Min;
    case Max:
      return QgsStringStatisticalSummary::Max;
    case StringMinimumLength:
      return QgsStringStatisticalSummary::MinimumLength;
    case StringMaximumLength:
      return QgsStringStatisticalSummary::MaximumLength;

    case Sum:
    case Mean:
    case Median:
    case StDev:
    case StDevSample:
    case Range:
    case Minority:
    case Majority:
    case FirstQuartile:
    case ThirdQuartile:
    case InterQuartileRange:
    case StringConcatenate:
    case GeometryCollect:
    case ArrayAggregate:
    {
      if ( ok )
        *ok = false;
      return QgsStringStatisticalSummary::Count;
    }
  }

  if ( ok )
    *ok = false;
  return QgsStringStatisticalSummary::Count;
}

QgsDateTimeStatisticalSummary::Statistic QgsAggregateCalculator::dateTimeStatFromAggregate( QgsAggregateCalculator::Aggregate aggregate, bool *ok )
{
  if ( ok )
    *ok = true;

  switch ( aggregate )
  {
    case Count:
      return QgsDateTimeStatisticalSummary::Count;
    case CountDistinct:
      return QgsDateTimeStatisticalSummary::CountDistinct;
    case CountMissing:
      return QgsDateTimeStatisticalSummary::CountMissing;
    case Min:
      return QgsDateTimeStatisticalSummary::Min;
    case Max:
      return QgsDateTimeStatisticalSummary::Max;
    case Range:
      return QgsDateTimeStatisticalSummary::Range;

    case Sum:
    case Mean:
    case Median:
    case StDev:
    case StDevSample:
    case Minority:
    case Majority:
    case FirstQuartile:
    case ThirdQuartile:
    case InterQuartileRange:
    case StringMinimumLength:
    case StringMaximumLength:
    case StringConcatenate:
    case GeometryCollect:
    case ArrayAggregate:
    {
      if ( ok )
        *ok = false;
      return QgsDateTimeStatisticalSummary::Count;
    }
  }

  if ( ok )
    *ok = false;
  return QgsDateTimeStatisticalSummary::Count;
}

QVariant QgsAggregateCalculator::calculateNumericAggregate( QgsFeatureIterator &fit, int attr, QgsExpression *expression,
    QgsExpressionContext *context, QgsStatisticalSummary::Statistic stat )
{
  Q_ASSERT( expression || attr >= 0 );

  QgsStatisticalSummary s( stat );
  QgsFeature f;

  while ( fit.nextFeature( f ) )
  {
    if ( expression )
    {
      Q_ASSERT( context );
      context->setFeature( f );
      QVariant v = expression->evaluate( context );
      s.addVariant( v );
    }
    else
    {
      s.addVariant( f.attribute( attr ) );
    }
  }
  s.finalize();
  double val = s.statistic( stat );
  return std::isnan( val ) ? QVariant() : val;
}

QVariant QgsAggregateCalculator::calculateStringAggregate( QgsFeatureIterator &fit, int attr, QgsExpression *expression,
    QgsExpressionContext *context, QgsStringStatisticalSummary::Statistic stat )
{
  Q_ASSERT( expression || attr >= 0 );

  QgsStringStatisticalSummary s( stat );
  QgsFeature f;

  while ( fit.nextFeature( f ) )
  {
    if ( expression )
    {
      Q_ASSERT( context );
      context->setFeature( f );
      QVariant v = expression->evaluate( context );
      s.addValue( v );
    }
    else
    {
      s.addValue( f.attribute( attr ) );
    }
  }
  s.finalize();
  return s.statistic( stat );
}

QVariant QgsAggregateCalculator::calculateGeometryAggregate( QgsFeatureIterator &fit, QgsExpression *expression, QgsExpressionContext *context )
{
  Q_ASSERT( expression );

  QgsFeature f;
  QVector< QgsGeometry > geometries;
  while ( fit.nextFeature( f ) )
  {
    Q_ASSERT( context );
    context->setFeature( f );
    QVariant v = expression->evaluate( context );
    if ( v.canConvert<QgsGeometry>() )
    {
      geometries << v.value<QgsGeometry>();
    }
  }

  return QVariant::fromValue( QgsGeometry::collectGeometry( geometries ) );
}

QVariant QgsAggregateCalculator::concatenateStrings( QgsFeatureIterator &fit, int attr, QgsExpression *expression,
    QgsExpressionContext *context, const QString &delimiter )
{
  Q_ASSERT( expression || attr >= 0 );

  QgsFeature f;
  QString result;
  while ( fit.nextFeature( f ) )
  {
    if ( !result.isEmpty() )
      result += delimiter;

    if ( expression )
    {
      Q_ASSERT( context );
      context->setFeature( f );
      QVariant v = expression->evaluate( context );
      result += v.toString();
    }
    else
    {
      result += f.attribute( attr ).toString();
    }
  }
  return result;
}

QVariant QgsAggregateCalculator::defaultValue( QgsAggregateCalculator::Aggregate aggregate ) const
{
  // value to return when NO features are aggregated:
  switch ( aggregate )
  {
    // sensible values:
    case Count:
    case CountDistinct:
    case CountMissing:
      return 0;

    case StringConcatenate:
      return ""; // zero length string - not null!

    case ArrayAggregate:
      return QVariantList(); // empty list

    // undefined - nothing makes sense here
    case Sum:
    case Min:
    case Max:
    case Mean:
    case Median:
    case StDev:
    case StDevSample:
    case Range:
    case Minority:
    case Majority:
    case FirstQuartile:
    case ThirdQuartile:
    case InterQuartileRange:
    case StringMinimumLength:
    case StringMaximumLength:
    case GeometryCollect:
      return QVariant();
  }
  return QVariant();
}

QVariant QgsAggregateCalculator::calculateDateTimeAggregate( QgsFeatureIterator &fit, int attr, QgsExpression *expression,
    QgsExpressionContext *context, QgsDateTimeStatisticalSummary::Statistic stat )
{
  Q_ASSERT( expression || attr >= 0 );

  QgsDateTimeStatisticalSummary s( stat );
  QgsFeature f;

  while ( fit.nextFeature( f ) )
  {
    if ( expression )
    {
      Q_ASSERT( context );
      context->setFeature( f );
      QVariant v = expression->evaluate( context );
      s.addValue( v );
    }
    else
    {
      s.addValue( f.attribute( attr ) );
    }
  }
  s.finalize();
  return s.statistic( stat );
}

QVariant QgsAggregateCalculator::calculateArrayAggregate( QgsFeatureIterator &fit, int attr, QgsExpression *expression,
    QgsExpressionContext *context )
{
  Q_ASSERT( expression || attr >= 0 );

  QgsFeature f;

  QVariantList array;

  while ( fit.nextFeature( f ) )
  {
    if ( expression )
    {
      Q_ASSERT( context );
      context->setFeature( f );
      QVariant v = expression->evaluate( context );
      array.append( v );
    }
    else
    {
      array.append( f.attribute( attr ) );
    }
  }
  return array;
}
