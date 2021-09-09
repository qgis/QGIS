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
#include "qgsexpressionutils.h"
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
  mOrderBy = parameters.orderBy;
}

void QgsAggregateCalculator::setFidsFilter( const QgsFeatureIds &fids )
{
  mFidsSet = true;
  mFidsFilter = fids;
}

QVariant QgsAggregateCalculator::calculate( QgsAggregateCalculator::Aggregate aggregate,
    const QString &fieldOrExpression, QgsExpressionContext *context, bool *ok, QgsFeedback *feedback ) const
{
  mLastError.clear();
  if ( ok )
    *ok = false;

  QgsFeatureRequest request = QgsFeatureRequest();

  if ( !mLayer )
    return QVariant();

  QgsExpressionContext defaultContext = mLayer->createExpressionContext();
  context = context ? context : &defaultContext;

  std::unique_ptr<QgsExpression> expression;

  const int attrNum = QgsExpression::expressionToLayerFieldIndex( fieldOrExpression, mLayer );
  if ( attrNum == -1 )
  {
    Q_ASSERT( context );
    context->setFields( mLayer->fields() );
    // try to use expression
    expression.reset( new QgsExpression( fieldOrExpression ) );

    if ( expression->hasParserError() || !expression->prepare( context ) )
    {
      mLastError = !expression->parserErrorString().isEmpty() ? expression->parserErrorString() : expression->evalErrorString();
      return QVariant();
    }
  }

  QSet<QString> lst;
  if ( !expression )
    lst.insert( mLayer->fields().at( attrNum ).name() );
  else
    lst = expression->referencedColumns();

  request.setFlags( ( expression && expression->needsGeometry() ) ?
                    QgsFeatureRequest::NoFlags :
                    QgsFeatureRequest::NoGeometry )
  .setSubsetOfAttributes( lst, mLayer->fields() );

  if ( mFidsSet )
    request.setFilterFids( mFidsFilter );

  if ( !mOrderBy.empty() )
    request.setOrderBy( mOrderBy );

  if ( !mFilterExpression.isEmpty() )
    request.setFilterExpression( mFilterExpression );
  if ( context )
    request.setExpressionContext( *context );

  request.setFeedback( feedback ? feedback : ( context ? context->feedback() : nullptr ) );

  //determine result type
  QVariant::Type resultType = QVariant::Double;
  int userType = 0;
  if ( attrNum == -1 )
  {
    if ( aggregate == GeometryCollect )
    {
      // in this case we know the result should be a geometry value, so no need to sniff it out...
      resultType = QVariant::UserType;
    }
    else
    {
      // check expression result type
      bool foundFeatures = false;
      std::tuple<QVariant::Type, int> returnType = QgsExpressionUtils::determineResultType( fieldOrExpression, mLayer, request, *context, &foundFeatures );
      if ( !foundFeatures )
      {
        if ( ok )
          *ok = true;
        return defaultValue( aggregate );
      }

      resultType = std::get<0>( returnType );
      userType = std::get<1>( returnType );
      if ( resultType == QVariant::Invalid )
      {
        QVariant v;
        switch ( aggregate )
        {
          // string
          case StringConcatenate:
          case StringConcatenateUnique:
          case StringMinimumLength:
          case StringMaximumLength:
            v = QString();
            break;

          // numerical
          case Sum:
          case Mean:
          case Median:
          case StDev:
          case StDevSample:
          case Range:
          case FirstQuartile:
          case ThirdQuartile:
          case InterQuartileRange:
          // mixed type, fallback to numerical
          case Count:
          case CountDistinct:
          case CountMissing:
          case Minority:
          case Majority:
          case Min:
          case Max:
            v = 0.0;
            break;

          // geometry
          case GeometryCollect:
            v = QgsGeometry();
            break;

          // list, fallback to string
          case ArrayAggregate:
            v = QString();
            break;
        }
        resultType = v.type();
        userType = v.userType();
      }
    }
  }
  else
    resultType = mLayer->fields().at( attrNum ).type();

  QgsFeatureIterator fit = mLayer->getFeatures( request );
  return calculate( aggregate, fit, resultType, userType, attrNum, expression.get(), mDelimiter, context, ok, &mLastError );
}

QgsAggregateCalculator::Aggregate QgsAggregateCalculator::stringToAggregate( const QString &string, bool *ok )
{
  const QString normalized = string.trimmed().toLower();

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
  else if ( normalized == QLatin1String( "concatenate_unique" ) )
    return StringConcatenateUnique;
  else if ( normalized == QLatin1String( "collect" ) )
    return GeometryCollect;
  else if ( normalized == QLatin1String( "array_agg" ) )
    return ArrayAggregate;

  if ( ok )
    *ok = false;

  return Count;
}

QString QgsAggregateCalculator::displayName( Aggregate aggregate )
{
  switch ( aggregate )
  {
    case QgsAggregateCalculator::Count:
      return QObject::tr( "count" );
    case QgsAggregateCalculator::CountDistinct:
      return QObject::tr( "count distinct" );
    case QgsAggregateCalculator::CountMissing:
      return QObject::tr( "count missing" );
    case QgsAggregateCalculator::Min:
      return QObject::tr( "minimum" );
    case QgsAggregateCalculator::Max:
      return QObject::tr( "maximum" );
    case QgsAggregateCalculator::Sum:
      return QObject::tr( "sum" );
    case QgsAggregateCalculator::Mean:
      return QObject::tr( "mean" );
    case QgsAggregateCalculator::Median:
      return QObject::tr( "median" );
    case QgsAggregateCalculator::StDev:
      return QObject::tr( "standard deviation" );
    case QgsAggregateCalculator::StDevSample:
      return QObject::tr( "standard deviation (sample)" );
    case QgsAggregateCalculator::Range:
      return QObject::tr( "range" );
    case QgsAggregateCalculator::Minority:
      return QObject::tr( "minority" );
    case QgsAggregateCalculator::Majority:
      return QObject::tr( "majority" );
    case QgsAggregateCalculator::FirstQuartile:
      return QObject::tr( "first quartile" );
    case QgsAggregateCalculator::ThirdQuartile:
      return QObject::tr( "third quartile" );
    case QgsAggregateCalculator::InterQuartileRange:
      return QObject::tr( "inter quartile range" );
    case QgsAggregateCalculator::StringMinimumLength:
      return QObject::tr( "minimum length" );
    case QgsAggregateCalculator::StringMaximumLength:
      return QObject::tr( "maximum length" );
    case QgsAggregateCalculator::StringConcatenate:
      return QObject::tr( "concatenate" );
    case QgsAggregateCalculator::GeometryCollect:
      return QObject::tr( "collection" );
    case QgsAggregateCalculator::ArrayAggregate:
      return QObject::tr( "array aggregate" );
    case QgsAggregateCalculator::StringConcatenateUnique:
      return QObject::tr( "concatenate (unique)" );
  }
  return QString();
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
        << QVariant::String
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
        << QVariant::String
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

QVariant QgsAggregateCalculator::calculate( QgsAggregateCalculator::Aggregate aggregate, QgsFeatureIterator &fit, QVariant::Type resultType, int userType,
    int attr, QgsExpression *expression, const QString &delimiter, QgsExpressionContext *context, bool *ok, QString *error )
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
      const QgsStatisticalSummary::Statistic stat = numericStatFromAggregate( aggregate, &statOk );
      if ( !statOk )
      {
        if ( error )
          *error = expression ? QObject::tr( "Cannot calculate %1 on numeric values" ).arg( displayName( aggregate ) )
                   : QObject::tr( "Cannot calculate %1 on numeric fields" ).arg( displayName( aggregate ) );
        return QVariant();
      }

      if ( ok )
        *ok = true;
      return calculateNumericAggregate( fit, attr, expression, context, stat );
    }

    case QVariant::Date:
    case QVariant::DateTime:
    {
      bool statOk = false;
      const QgsDateTimeStatisticalSummary::Statistic stat = dateTimeStatFromAggregate( aggregate, &statOk );
      if ( !statOk )
      {
        if ( error )
          *error = ( expression ? QObject::tr( "Cannot calculate %1 on %2 values" ).arg( displayName( aggregate ) ) :
                     QObject::tr( "Cannot calculate %1 on %2 fields" ).arg( displayName( aggregate ) ) ).arg( resultType == QVariant::Date ? QObject::tr( "date" ) : QObject::tr( "datetime" ) );
        return QVariant();
      }

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
      else if ( aggregate == StringConcatenateUnique )
      {
        //special case
        if ( ok )
          *ok = true;
        return concatenateStrings( fit, attr, expression, context, delimiter, true );
      }

      bool statOk = false;
      const QgsStringStatisticalSummary::Statistic stat = stringStatFromAggregate( aggregate, &statOk );
      if ( !statOk )
      {
        QString typeString;
        if ( resultType == QVariant::Invalid )
          typeString = QObject::tr( "null" );
        else if ( resultType == QVariant::UserType )
          typeString = QMetaType::typeName( userType );
        else
          typeString = resultType == QVariant::String ? QObject::tr( "string" ) : QVariant::typeToName( resultType );

        if ( error )
          *error = expression ? QObject::tr( "Cannot calculate %1 on %3 values" ).arg( displayName( aggregate ), typeString )
                   : QObject::tr( "Cannot calculate %1 on %3 fields" ).arg( displayName( aggregate ), typeString );
        return QVariant();
      }

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
    case StringConcatenateUnique:
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
    case Minority:
      return QgsStringStatisticalSummary::Minority;
    case Majority:
      return QgsStringStatisticalSummary::Majority;

    case Sum:
    case Mean:
    case Median:
    case StDev:
    case StDevSample:
    case Range:
    case FirstQuartile:
    case ThirdQuartile:
    case InterQuartileRange:
    case StringConcatenate:
    case StringConcatenateUnique:
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
    case StringConcatenateUnique:
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
      const QVariant v = expression->evaluate( context );
      s.addVariant( v );
    }
    else
    {
      s.addVariant( f.attribute( attr ) );
    }
  }
  s.finalize();
  const double val = s.statistic( stat );
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
      const QVariant v = expression->evaluate( context );
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
    const QVariant v = expression->evaluate( context );
    if ( v.canConvert<QgsGeometry>() )
    {
      geometries << v.value<QgsGeometry>();
    }
  }

  return QVariant::fromValue( QgsGeometry::collectGeometry( geometries ) );
}

QVariant QgsAggregateCalculator::concatenateStrings( QgsFeatureIterator &fit, int attr, QgsExpression *expression,
    QgsExpressionContext *context, const QString &delimiter, bool unique )
{
  Q_ASSERT( expression || attr >= 0 );

  QgsFeature f;
  QStringList results;
  while ( fit.nextFeature( f ) )
  {
    QString result;
    if ( expression )
    {
      Q_ASSERT( context );
      context->setFeature( f );
      const QVariant v = expression->evaluate( context );
      result = v.toString();
    }
    else
    {
      result = f.attribute( attr ).toString();
    }

    if ( !unique || !results.contains( result ) )
      results << result;
  }

  return results.join( delimiter );
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
    case StringConcatenateUnique:
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
      const QVariant v = expression->evaluate( context );
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
      const QVariant v = expression->evaluate( context );
      array.append( v );
    }
    else
    {
      array.append( f.attribute( attr ) );
    }
  }
  return array;
}

