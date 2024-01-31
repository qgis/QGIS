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
    if ( aggregate == Aggregate::GeometryCollect )
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
          case Aggregate::StringConcatenate:
          case Aggregate::StringConcatenateUnique:
          case Aggregate::StringMinimumLength:
          case Aggregate::StringMaximumLength:
            v = QString();
            break;

          // numerical
          case Aggregate::Sum:
          case Aggregate::Mean:
          case Aggregate::Median:
          case Aggregate::StDev:
          case Aggregate::StDevSample:
          case Aggregate::Range:
          case Aggregate::FirstQuartile:
          case Aggregate::ThirdQuartile:
          case Aggregate::InterQuartileRange:
          // mixed type, fallback to numerical
          case Aggregate::Count:
          case Aggregate::CountDistinct:
          case Aggregate::CountMissing:
          case Aggregate::Minority:
          case Aggregate::Majority:
          case Aggregate::Min:
          case Aggregate::Max:
            v = 0.0;
            break;

          // geometry
          case Aggregate::GeometryCollect:
            v = QgsGeometry();
            break;

          // list, fallback to string
          case Aggregate::ArrayAggregate:
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
    return Aggregate::Count;
  else if ( normalized == QLatin1String( "count_distinct" ) )
    return Aggregate::CountDistinct;
  else if ( normalized == QLatin1String( "count_missing" ) )
    return Aggregate::CountMissing;
  else if ( normalized == QLatin1String( "min" ) || normalized == QLatin1String( "minimum" ) )
    return Aggregate::Min;
  else if ( normalized == QLatin1String( "max" ) || normalized == QLatin1String( "maximum" ) )
    return Aggregate::Max;
  else if ( normalized == QLatin1String( "sum" ) )
    return Aggregate::Sum;
  else if ( normalized == QLatin1String( "mean" ) )
    return Aggregate::Mean;
  else if ( normalized == QLatin1String( "median" ) )
    return Aggregate::Median;
  else if ( normalized == QLatin1String( "stdev" ) )
    return Aggregate::StDev;
  else if ( normalized == QLatin1String( "stdevsample" ) )
    return Aggregate::StDevSample;
  else if ( normalized == QLatin1String( "range" ) )
    return Aggregate::Range;
  else if ( normalized == QLatin1String( "minority" ) )
    return Aggregate::Minority;
  else if ( normalized == QLatin1String( "majority" ) )
    return Aggregate::Majority;
  else if ( normalized == QLatin1String( "q1" ) )
    return Aggregate::FirstQuartile;
  else if ( normalized == QLatin1String( "q3" ) )
    return Aggregate::ThirdQuartile;
  else if ( normalized == QLatin1String( "iqr" ) )
    return Aggregate::InterQuartileRange;
  else if ( normalized == QLatin1String( "min_length" ) )
    return Aggregate::StringMinimumLength;
  else if ( normalized == QLatin1String( "max_length" ) )
    return Aggregate::StringMaximumLength;
  else if ( normalized == QLatin1String( "concatenate" ) )
    return Aggregate::StringConcatenate;
  else if ( normalized == QLatin1String( "concatenate_unique" ) )
    return Aggregate::StringConcatenateUnique;
  else if ( normalized == QLatin1String( "collect" ) )
    return Aggregate::GeometryCollect;
  else if ( normalized == QLatin1String( "array_agg" ) )
    return Aggregate::ArrayAggregate;

  if ( ok )
    *ok = false;

  return Aggregate::Count;
}

QString QgsAggregateCalculator::displayName( Aggregate aggregate )
{
  switch ( aggregate )
  {
    case QgsAggregateCalculator::Aggregate::Count:
      return QObject::tr( "count" );
    case QgsAggregateCalculator::Aggregate::CountDistinct:
      return QObject::tr( "count distinct" );
    case QgsAggregateCalculator::Aggregate::CountMissing:
      return QObject::tr( "count missing" );
    case QgsAggregateCalculator::Aggregate::Min:
      return QObject::tr( "minimum" );
    case QgsAggregateCalculator::Aggregate::Max:
      return QObject::tr( "maximum" );
    case QgsAggregateCalculator::Aggregate::Sum:
      return QObject::tr( "sum" );
    case QgsAggregateCalculator::Aggregate::Mean:
      return QObject::tr( "mean" );
    case QgsAggregateCalculator::Aggregate::Median:
      return QObject::tr( "median" );
    case QgsAggregateCalculator::Aggregate::StDev:
      return QObject::tr( "standard deviation" );
    case QgsAggregateCalculator::Aggregate::StDevSample:
      return QObject::tr( "standard deviation (sample)" );
    case QgsAggregateCalculator::Aggregate::Range:
      return QObject::tr( "range" );
    case QgsAggregateCalculator::Aggregate::Minority:
      return QObject::tr( "minority" );
    case QgsAggregateCalculator::Aggregate::Majority:
      return QObject::tr( "majority" );
    case QgsAggregateCalculator::Aggregate::FirstQuartile:
      return QObject::tr( "first quartile" );
    case QgsAggregateCalculator::Aggregate::ThirdQuartile:
      return QObject::tr( "third quartile" );
    case QgsAggregateCalculator::Aggregate::InterQuartileRange:
      return QObject::tr( "inter quartile range" );
    case QgsAggregateCalculator::Aggregate::StringMinimumLength:
      return QObject::tr( "minimum length" );
    case QgsAggregateCalculator::Aggregate::StringMaximumLength:
      return QObject::tr( "maximum length" );
    case QgsAggregateCalculator::Aggregate::StringConcatenate:
      return QObject::tr( "concatenate" );
    case QgsAggregateCalculator::Aggregate::GeometryCollect:
      return QObject::tr( "collection" );
    case QgsAggregateCalculator::Aggregate::ArrayAggregate:
      return QObject::tr( "array aggregate" );
    case QgsAggregateCalculator::Aggregate::StringConcatenateUnique:
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

  if ( aggregate == QgsAggregateCalculator::Aggregate::ArrayAggregate )
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
      if ( aggregate == Aggregate::GeometryCollect )
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
      if ( aggregate == Aggregate::StringConcatenate )
      {
        //special case
        if ( ok )
          *ok = true;
        return concatenateStrings( fit, attr, expression, context, delimiter );
      }
      else if ( aggregate == Aggregate::StringConcatenateUnique )
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
    case Aggregate::Count:
      return QgsStatisticalSummary::Statistic::Count;
    case Aggregate::CountDistinct:
      return QgsStatisticalSummary::Statistic::Variety;
    case Aggregate::CountMissing:
      return QgsStatisticalSummary::Statistic::CountMissing;
    case Aggregate::Min:
      return QgsStatisticalSummary::Statistic::Min;
    case Aggregate::Max:
      return QgsStatisticalSummary::Statistic::Max;
    case Aggregate::Sum:
      return QgsStatisticalSummary::Statistic::Sum;
    case Aggregate::Mean:
      return QgsStatisticalSummary::Statistic::Mean;
    case Aggregate::Median:
      return QgsStatisticalSummary::Statistic::Median;
    case Aggregate::StDev:
      return QgsStatisticalSummary::Statistic::StDev;
    case Aggregate::StDevSample:
      return QgsStatisticalSummary::Statistic::StDevSample;
    case Aggregate::Range:
      return QgsStatisticalSummary::Statistic::Range;
    case Aggregate::Minority:
      return QgsStatisticalSummary::Statistic::Minority;
    case Aggregate::Majority:
      return QgsStatisticalSummary::Statistic::Majority;
    case Aggregate::FirstQuartile:
      return QgsStatisticalSummary::Statistic::FirstQuartile;
    case Aggregate::ThirdQuartile:
      return QgsStatisticalSummary::Statistic::ThirdQuartile;
    case Aggregate::InterQuartileRange:
      return QgsStatisticalSummary::Statistic::InterQuartileRange;
    case Aggregate::StringMinimumLength:
    case Aggregate::StringMaximumLength:
    case Aggregate::StringConcatenate:
    case Aggregate::StringConcatenateUnique:
    case Aggregate::GeometryCollect:
    case Aggregate::ArrayAggregate:
    {
      if ( ok )
        *ok = false;
      return QgsStatisticalSummary::Statistic::Count;
    }
  }

  if ( ok )
    *ok = false;
  return QgsStatisticalSummary::Statistic::Count;
}

QgsStringStatisticalSummary::Statistic QgsAggregateCalculator::stringStatFromAggregate( QgsAggregateCalculator::Aggregate aggregate, bool *ok )
{
  if ( ok )
    *ok = true;

  switch ( aggregate )
  {
    case Aggregate::Count:
      return QgsStringStatisticalSummary::Statistic::Count;
    case Aggregate::CountDistinct:
      return QgsStringStatisticalSummary::Statistic::CountDistinct;
    case Aggregate::CountMissing:
      return QgsStringStatisticalSummary::Statistic::CountMissing;
    case Aggregate::Min:
      return QgsStringStatisticalSummary::Statistic::Min;
    case Aggregate::Max:
      return QgsStringStatisticalSummary::Statistic::Max;
    case Aggregate::StringMinimumLength:
      return QgsStringStatisticalSummary::Statistic::MinimumLength;
    case Aggregate::StringMaximumLength:
      return QgsStringStatisticalSummary::Statistic::MaximumLength;
    case Aggregate::Minority:
      return QgsStringStatisticalSummary::Statistic::Minority;
    case Aggregate::Majority:
      return QgsStringStatisticalSummary::Statistic::Majority;

    case Aggregate::Sum:
    case Aggregate::Mean:
    case Aggregate::Median:
    case Aggregate::StDev:
    case Aggregate::StDevSample:
    case Aggregate::Range:
    case Aggregate::FirstQuartile:
    case Aggregate::ThirdQuartile:
    case Aggregate::InterQuartileRange:
    case Aggregate::StringConcatenate:
    case Aggregate::StringConcatenateUnique:
    case Aggregate::GeometryCollect:
    case Aggregate::ArrayAggregate:
    {
      if ( ok )
        *ok = false;
      return QgsStringStatisticalSummary::Statistic::Count;
    }
  }

  if ( ok )
    *ok = false;
  return QgsStringStatisticalSummary::Statistic::Count;
}

QgsDateTimeStatisticalSummary::Statistic QgsAggregateCalculator::dateTimeStatFromAggregate( QgsAggregateCalculator::Aggregate aggregate, bool *ok )
{
  if ( ok )
    *ok = true;

  switch ( aggregate )
  {
    case Aggregate::Count:
      return QgsDateTimeStatisticalSummary::Statistic::Count;
    case Aggregate::CountDistinct:
      return QgsDateTimeStatisticalSummary::Statistic::CountDistinct;
    case Aggregate::CountMissing:
      return QgsDateTimeStatisticalSummary::Statistic::CountMissing;
    case Aggregate::Min:
      return QgsDateTimeStatisticalSummary::Statistic::Min;
    case Aggregate::Max:
      return QgsDateTimeStatisticalSummary::Statistic::Max;
    case Aggregate::Range:
      return QgsDateTimeStatisticalSummary::Statistic::Range;

    case Aggregate::Sum:
    case Aggregate::Mean:
    case Aggregate::Median:
    case Aggregate::StDev:
    case Aggregate::StDevSample:
    case Aggregate::Minority:
    case Aggregate::Majority:
    case Aggregate::FirstQuartile:
    case Aggregate::ThirdQuartile:
    case Aggregate::InterQuartileRange:
    case Aggregate::StringMinimumLength:
    case Aggregate::StringMaximumLength:
    case Aggregate::StringConcatenate:
    case Aggregate::StringConcatenateUnique:
    case Aggregate::GeometryCollect:
    case Aggregate::ArrayAggregate:
    {
      if ( ok )
        *ok = false;
      return QgsDateTimeStatisticalSummary::Statistic::Count;
    }
  }

  if ( ok )
    *ok = false;
  return QgsDateTimeStatisticalSummary::Statistic::Count;
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

QVariant QgsAggregateCalculator::defaultValue( QgsAggregateCalculator::Aggregate aggregate ) const
{
  // value to return when NO features are aggregated:
  switch ( aggregate )
  {
    // sensible values:
    case Aggregate::Count:
    case Aggregate::CountDistinct:
    case Aggregate::CountMissing:
      return 0;

    case Aggregate::StringConcatenate:
    case Aggregate::StringConcatenateUnique:
      return ""; // zero length string - not null!

    case Aggregate::ArrayAggregate:
      return QVariantList(); // empty list

    // undefined - nothing makes sense here
    case Aggregate::Sum:
    case Aggregate::Min:
    case Aggregate::Max:
    case Aggregate::Mean:
    case Aggregate::Median:
    case Aggregate::StDev:
    case Aggregate::StDevSample:
    case Aggregate::Range:
    case Aggregate::Minority:
    case Aggregate::Majority:
    case Aggregate::FirstQuartile:
    case Aggregate::ThirdQuartile:
    case Aggregate::InterQuartileRange:
    case Aggregate::StringMinimumLength:
    case Aggregate::StringMaximumLength:
    case Aggregate::GeometryCollect:
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

