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
#include "qgsstatisticalsummary.h"
#include "qgsdatetimestatisticalsummary.h"
#include "qgsstringstatisticalsummary.h"


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

QVariant QgsAggregateCalculator::calculate( Qgis::Aggregate aggregate,
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

  bool expressionNeedsGeometry {  expression &&expression->needsGeometry() };

  if ( !mOrderBy.empty() )
  {
    request.setOrderBy( mOrderBy );
    for ( const QgsFeatureRequest::OrderByClause &orderBy : std::as_const( mOrderBy ) )
    {
      if ( orderBy.expression().needsGeometry() )
      {
        expressionNeedsGeometry = true;
        break;
      }
    }
  }

  request.setFlags( expressionNeedsGeometry ?
                    Qgis::FeatureRequestFlag::NoFlags :
                    Qgis::FeatureRequestFlag::NoGeometry )
  .setSubsetOfAttributes( lst, mLayer->fields() );

  if ( mFidsSet )
    request.setFilterFids( mFidsFilter );

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
    if ( aggregate == Qgis::Aggregate::GeometryCollect )
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
          case Qgis::Aggregate::StringConcatenate:
          case Qgis::Aggregate::StringConcatenateUnique:
          case Qgis::Aggregate::StringMinimumLength:
          case Qgis::Aggregate::StringMaximumLength:
            v = QString();
            break;

          // numerical
          case Qgis::Aggregate::Sum:
          case Qgis::Aggregate::Mean:
          case Qgis::Aggregate::Median:
          case Qgis::Aggregate::StDev:
          case Qgis::Aggregate::StDevSample:
          case Qgis::Aggregate::Range:
          case Qgis::Aggregate::FirstQuartile:
          case Qgis::Aggregate::ThirdQuartile:
          case Qgis::Aggregate::InterQuartileRange:
          // mixed type, fallback to numerical
          case Qgis::Aggregate::Count:
          case Qgis::Aggregate::CountDistinct:
          case Qgis::Aggregate::CountMissing:
          case Qgis::Aggregate::Minority:
          case Qgis::Aggregate::Majority:
          case Qgis::Aggregate::Min:
          case Qgis::Aggregate::Max:
            v = 0.0;
            break;

          // geometry
          case Qgis::Aggregate::GeometryCollect:
            v = QgsGeometry();
            break;

          // list, fallback to string
          case Qgis::Aggregate::ArrayAggregate:
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

Qgis::Aggregate QgsAggregateCalculator::stringToAggregate( const QString &string, bool *ok )
{
  const QString normalized = string.trimmed().toLower();

  if ( ok )
    *ok = true;

  if ( normalized == QLatin1String( "count" ) )
    return Qgis::Aggregate::Count;
  else if ( normalized == QLatin1String( "count_distinct" ) )
    return Qgis::Aggregate::CountDistinct;
  else if ( normalized == QLatin1String( "count_missing" ) )
    return Qgis::Aggregate::CountMissing;
  else if ( normalized == QLatin1String( "min" ) || normalized == QLatin1String( "minimum" ) )
    return Qgis::Aggregate::Min;
  else if ( normalized == QLatin1String( "max" ) || normalized == QLatin1String( "maximum" ) )
    return Qgis::Aggregate::Max;
  else if ( normalized == QLatin1String( "sum" ) )
    return Qgis::Aggregate::Sum;
  else if ( normalized == QLatin1String( "mean" ) )
    return Qgis::Aggregate::Mean;
  else if ( normalized == QLatin1String( "median" ) )
    return Qgis::Aggregate::Median;
  else if ( normalized == QLatin1String( "stdev" ) )
    return Qgis::Aggregate::StDev;
  else if ( normalized == QLatin1String( "stdevsample" ) )
    return Qgis::Aggregate::StDevSample;
  else if ( normalized == QLatin1String( "range" ) )
    return Qgis::Aggregate::Range;
  else if ( normalized == QLatin1String( "minority" ) )
    return Qgis::Aggregate::Minority;
  else if ( normalized == QLatin1String( "majority" ) )
    return Qgis::Aggregate::Majority;
  else if ( normalized == QLatin1String( "q1" ) )
    return Qgis::Aggregate::FirstQuartile;
  else if ( normalized == QLatin1String( "q3" ) )
    return Qgis::Aggregate::ThirdQuartile;
  else if ( normalized == QLatin1String( "iqr" ) )
    return Qgis::Aggregate::InterQuartileRange;
  else if ( normalized == QLatin1String( "min_length" ) )
    return Qgis::Aggregate::StringMinimumLength;
  else if ( normalized == QLatin1String( "max_length" ) )
    return Qgis::Aggregate::StringMaximumLength;
  else if ( normalized == QLatin1String( "concatenate" ) )
    return Qgis::Aggregate::StringConcatenate;
  else if ( normalized == QLatin1String( "concatenate_unique" ) )
    return Qgis::Aggregate::StringConcatenateUnique;
  else if ( normalized == QLatin1String( "collect" ) )
    return Qgis::Aggregate::GeometryCollect;
  else if ( normalized == QLatin1String( "array_agg" ) )
    return Qgis::Aggregate::ArrayAggregate;

  if ( ok )
    *ok = false;

  return Qgis::Aggregate::Count;
}

QString QgsAggregateCalculator::displayName( Qgis::Aggregate aggregate )
{
  switch ( aggregate )
  {
    case Qgis::Aggregate::Count:
      return QObject::tr( "count" );
    case Qgis::Aggregate::CountDistinct:
      return QObject::tr( "count distinct" );
    case Qgis::Aggregate::CountMissing:
      return QObject::tr( "count missing" );
    case Qgis::Aggregate::Min:
      return QObject::tr( "minimum" );
    case Qgis::Aggregate::Max:
      return QObject::tr( "maximum" );
    case Qgis::Aggregate::Sum:
      return QObject::tr( "sum" );
    case Qgis::Aggregate::Mean:
      return QObject::tr( "mean" );
    case Qgis::Aggregate::Median:
      return QObject::tr( "median" );
    case Qgis::Aggregate::StDev:
      return QObject::tr( "standard deviation" );
    case Qgis::Aggregate::StDevSample:
      return QObject::tr( "standard deviation (sample)" );
    case Qgis::Aggregate::Range:
      return QObject::tr( "range" );
    case Qgis::Aggregate::Minority:
      return QObject::tr( "minority" );
    case Qgis::Aggregate::Majority:
      return QObject::tr( "majority" );
    case Qgis::Aggregate::FirstQuartile:
      return QObject::tr( "first quartile" );
    case Qgis::Aggregate::ThirdQuartile:
      return QObject::tr( "third quartile" );
    case Qgis::Aggregate::InterQuartileRange:
      return QObject::tr( "inter quartile range" );
    case Qgis::Aggregate::StringMinimumLength:
      return QObject::tr( "minimum length" );
    case Qgis::Aggregate::StringMaximumLength:
      return QObject::tr( "maximum length" );
    case Qgis::Aggregate::StringConcatenate:
      return QObject::tr( "concatenate" );
    case Qgis::Aggregate::GeometryCollect:
      return QObject::tr( "collection" );
    case Qgis::Aggregate::ArrayAggregate:
      return QObject::tr( "array aggregate" );
    case Qgis::Aggregate::StringConcatenateUnique:
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

QVariant QgsAggregateCalculator::calculate( Qgis::Aggregate aggregate, QgsFeatureIterator &fit, QVariant::Type resultType, int userType,
    int attr, QgsExpression *expression, const QString &delimiter, QgsExpressionContext *context, bool *ok, QString *error )
{
  if ( ok )
    *ok = false;

  if ( aggregate == Qgis::Aggregate::ArrayAggregate )
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
      const Qgis::Statistic stat = numericStatFromAggregate( aggregate, &statOk );
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
      const Qgis::DateTimeStatistic stat = dateTimeStatFromAggregate( aggregate, &statOk );
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
      if ( aggregate == Qgis::Aggregate::GeometryCollect )
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
      if ( aggregate == Qgis::Aggregate::StringConcatenate )
      {
        //special case
        if ( ok )
          *ok = true;
        return concatenateStrings( fit, attr, expression, context, delimiter );
      }
      else if ( aggregate == Qgis::Aggregate::StringConcatenateUnique )
      {
        //special case
        if ( ok )
          *ok = true;
        return concatenateStrings( fit, attr, expression, context, delimiter, true );
      }

      bool statOk = false;
      const Qgis::StringStatistic stat = stringStatFromAggregate( aggregate, &statOk );
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

Qgis::Statistic QgsAggregateCalculator::numericStatFromAggregate( Qgis::Aggregate aggregate, bool *ok )
{
  if ( ok )
    *ok = true;

  switch ( aggregate )
  {
    case Qgis::Aggregate::Count:
      return Qgis::Statistic::Count;
    case Qgis::Aggregate::CountDistinct:
      return Qgis::Statistic::Variety;
    case Qgis::Aggregate::CountMissing:
      return Qgis::Statistic::CountMissing;
    case Qgis::Aggregate::Min:
      return Qgis::Statistic::Min;
    case Qgis::Aggregate::Max:
      return Qgis::Statistic::Max;
    case Qgis::Aggregate::Sum:
      return Qgis::Statistic::Sum;
    case Qgis::Aggregate::Mean:
      return Qgis::Statistic::Mean;
    case Qgis::Aggregate::Median:
      return Qgis::Statistic::Median;
    case Qgis::Aggregate::StDev:
      return Qgis::Statistic::StDev;
    case Qgis::Aggregate::StDevSample:
      return Qgis::Statistic::StDevSample;
    case Qgis::Aggregate::Range:
      return Qgis::Statistic::Range;
    case Qgis::Aggregate::Minority:
      return Qgis::Statistic::Minority;
    case Qgis::Aggregate::Majority:
      return Qgis::Statistic::Majority;
    case Qgis::Aggregate::FirstQuartile:
      return Qgis::Statistic::FirstQuartile;
    case Qgis::Aggregate::ThirdQuartile:
      return Qgis::Statistic::ThirdQuartile;
    case Qgis::Aggregate::InterQuartileRange:
      return Qgis::Statistic::InterQuartileRange;
    case Qgis::Aggregate::StringMinimumLength:
    case Qgis::Aggregate::StringMaximumLength:
    case Qgis::Aggregate::StringConcatenate:
    case Qgis::Aggregate::StringConcatenateUnique:
    case Qgis::Aggregate::GeometryCollect:
    case Qgis::Aggregate::ArrayAggregate:
    {
      if ( ok )
        *ok = false;
      return Qgis::Statistic::Count;
    }
  }

  if ( ok )
    *ok = false;
  return Qgis::Statistic::Count;
}

Qgis::StringStatistic QgsAggregateCalculator::stringStatFromAggregate( Qgis::Aggregate aggregate, bool *ok )
{
  if ( ok )
    *ok = true;

  switch ( aggregate )
  {
    case Qgis::Aggregate::Count:
      return Qgis::StringStatistic::Count;
    case Qgis::Aggregate::CountDistinct:
      return Qgis::StringStatistic::CountDistinct;
    case Qgis::Aggregate::CountMissing:
      return Qgis::StringStatistic::CountMissing;
    case Qgis::Aggregate::Min:
      return Qgis::StringStatistic::Min;
    case Qgis::Aggregate::Max:
      return Qgis::StringStatistic::Max;
    case Qgis::Aggregate::StringMinimumLength:
      return Qgis::StringStatistic::MinimumLength;
    case Qgis::Aggregate::StringMaximumLength:
      return Qgis::StringStatistic::MaximumLength;
    case Qgis::Aggregate::Minority:
      return Qgis::StringStatistic::Minority;
    case Qgis::Aggregate::Majority:
      return Qgis::StringStatistic::Majority;

    case Qgis::Aggregate::Sum:
    case Qgis::Aggregate::Mean:
    case Qgis::Aggregate::Median:
    case Qgis::Aggregate::StDev:
    case Qgis::Aggregate::StDevSample:
    case Qgis::Aggregate::Range:
    case Qgis::Aggregate::FirstQuartile:
    case Qgis::Aggregate::ThirdQuartile:
    case Qgis::Aggregate::InterQuartileRange:
    case Qgis::Aggregate::StringConcatenate:
    case Qgis::Aggregate::StringConcatenateUnique:
    case Qgis::Aggregate::GeometryCollect:
    case Qgis::Aggregate::ArrayAggregate:
    {
      if ( ok )
        *ok = false;
      return Qgis::StringStatistic::Count;
    }
  }

  if ( ok )
    *ok = false;
  return Qgis::StringStatistic::Count;
}

Qgis::DateTimeStatistic QgsAggregateCalculator::dateTimeStatFromAggregate( Qgis::Aggregate aggregate, bool *ok )
{
  if ( ok )
    *ok = true;

  switch ( aggregate )
  {
    case Qgis::Aggregate::Count:
      return Qgis::DateTimeStatistic::Count;
    case Qgis::Aggregate::CountDistinct:
      return Qgis::DateTimeStatistic::CountDistinct;
    case Qgis::Aggregate::CountMissing:
      return Qgis::DateTimeStatistic::CountMissing;
    case Qgis::Aggregate::Min:
      return Qgis::DateTimeStatistic::Min;
    case Qgis::Aggregate::Max:
      return Qgis::DateTimeStatistic::Max;
    case Qgis::Aggregate::Range:
      return Qgis::DateTimeStatistic::Range;

    case Qgis::Aggregate::Sum:
    case Qgis::Aggregate::Mean:
    case Qgis::Aggregate::Median:
    case Qgis::Aggregate::StDev:
    case Qgis::Aggregate::StDevSample:
    case Qgis::Aggregate::Minority:
    case Qgis::Aggregate::Majority:
    case Qgis::Aggregate::FirstQuartile:
    case Qgis::Aggregate::ThirdQuartile:
    case Qgis::Aggregate::InterQuartileRange:
    case Qgis::Aggregate::StringMinimumLength:
    case Qgis::Aggregate::StringMaximumLength:
    case Qgis::Aggregate::StringConcatenate:
    case Qgis::Aggregate::StringConcatenateUnique:
    case Qgis::Aggregate::GeometryCollect:
    case Qgis::Aggregate::ArrayAggregate:
    {
      if ( ok )
        *ok = false;
      return Qgis::DateTimeStatistic::Count;
    }
  }

  if ( ok )
    *ok = false;
  return Qgis::DateTimeStatistic::Count;
}

QVariant QgsAggregateCalculator::calculateNumericAggregate( QgsFeatureIterator &fit, int attr, QgsExpression *expression,
    QgsExpressionContext *context, Qgis::Statistic stat )
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
    QgsExpressionContext *context, Qgis::StringStatistic stat )
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
    if ( v.userType() == QMetaType::type( "QgsGeometry" ) )
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

QVariant QgsAggregateCalculator::defaultValue( Qgis::Aggregate aggregate ) const
{
  // value to return when NO features are aggregated:
  switch ( aggregate )
  {
    // sensible values:
    case Qgis::Aggregate::Count:
    case Qgis::Aggregate::CountDistinct:
    case Qgis::Aggregate::CountMissing:
      return 0;

    case Qgis::Aggregate::StringConcatenate:
    case Qgis::Aggregate::StringConcatenateUnique:
      return ""; // zero length string - not null!

    case Qgis::Aggregate::ArrayAggregate:
      return QVariantList(); // empty list

    // undefined - nothing makes sense here
    case Qgis::Aggregate::Sum:
    case Qgis::Aggregate::Min:
    case Qgis::Aggregate::Max:
    case Qgis::Aggregate::Mean:
    case Qgis::Aggregate::Median:
    case Qgis::Aggregate::StDev:
    case Qgis::Aggregate::StDevSample:
    case Qgis::Aggregate::Range:
    case Qgis::Aggregate::Minority:
    case Qgis::Aggregate::Majority:
    case Qgis::Aggregate::FirstQuartile:
    case Qgis::Aggregate::ThirdQuartile:
    case Qgis::Aggregate::InterQuartileRange:
    case Qgis::Aggregate::StringMinimumLength:
    case Qgis::Aggregate::StringMaximumLength:
    case Qgis::Aggregate::GeometryCollect:
      return QVariant();
  }
  return QVariant();
}

QVariant QgsAggregateCalculator::calculateDateTimeAggregate( QgsFeatureIterator &fit, int attr, QgsExpression *expression,
    QgsExpressionContext *context, Qgis::DateTimeStatistic stat )
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

