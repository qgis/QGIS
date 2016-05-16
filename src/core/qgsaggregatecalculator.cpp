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
#include "qgsvectorlayer.h"



QgsAggregateCalculator::QgsAggregateCalculator( QgsVectorLayer* layer )
    : mLayer( layer )
{

}

QgsVectorLayer*QgsAggregateCalculator::layer() const
{
  return mLayer;
}

QVariant QgsAggregateCalculator::calculate( QgsAggregateCalculator::Aggregate aggregate,
    const QString& fieldOrExpression,
    QgsExpressionContext* context, bool* ok ) const
{
  if ( ok )
    *ok = false;

  if ( !mLayer )
    return QVariant();

  QScopedPointer<QgsExpression> expression;
  QScopedPointer<QgsExpressionContext> defaultContext;
  if ( !context )
  {
    defaultContext.reset( createContext() );
    context = defaultContext.data();
  }

  int attrNum = mLayer->fieldNameIndex( fieldOrExpression );

  if ( attrNum == -1 )
  {
    Q_ASSERT( context );
    // try to use expression
    expression.reset( new QgsExpression( fieldOrExpression ) );

    if ( expression->hasParserError() || !expression->prepare( context ) )
    {
      return QVariant();
    }
  }

  QStringList lst;
  if ( expression.isNull() )
    lst.append( fieldOrExpression );
  else
    lst = expression->referencedColumns();

  QgsFeatureRequest request = QgsFeatureRequest()
                              .setFlags(( expression.data() && expression->needsGeometry() ) ?
                                        QgsFeatureRequest::NoFlags :
                                        QgsFeatureRequest::NoGeometry )
                              .setSubsetOfAttributes( lst, mLayer->fields() );
  if ( !mFilterExpression.isEmpty() )
    request.setFilterExpression( mFilterExpression );

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
      return QVariant();

    context->setFeature( f );
    QVariant v = expression->evaluate( context );
    resultType = v.type();
  }
  else
  {
    resultType = mLayer->fields().at( attrNum ).type();
  }


  QgsFeatureIterator fit = mLayer->getFeatures( request );
  switch ( resultType )
  {
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Double:
    {
      QgsStatisticalSummary::Statistic stat = numericStatFromAggregate( aggregate, ok );
      if ( !ok )
        return QVariant();


      return calculateNumericAggregate( fit, attrNum, expression.data(), context, stat );
    }

    case QVariant::Date:
    case QVariant::DateTime:
    {
      QgsDateTimeStatisticalSummary::Statistic stat = dateTimeStatFromAggregate( aggregate, ok );
      if ( !ok )
        return QVariant();

      return calculateDateTimeAggregate( fit, attrNum, expression.data(), context, stat );
    }

    default:
    {
      // treat as string
      QgsStringStatisticalSummary::Statistic stat = stringStatFromAggregate( aggregate, ok );
      if ( !ok )
        return QVariant();

      return calculateStringAggregate( fit, attrNum, expression.data(), context, stat );
    }
  }

  return QVariant();
}

QgsStatisticalSummary::Statistic QgsAggregateCalculator::numericStatFromAggregate( QgsAggregateCalculator::Aggregate aggregate, bool* ok )
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

QgsStringStatisticalSummary::Statistic QgsAggregateCalculator::stringStatFromAggregate( QgsAggregateCalculator::Aggregate aggregate, bool* ok )
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

QgsDateTimeStatisticalSummary::Statistic QgsAggregateCalculator::dateTimeStatFromAggregate( QgsAggregateCalculator::Aggregate aggregate, bool* ok )
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

QVariant QgsAggregateCalculator::calculateNumericAggregate( QgsFeatureIterator& fit, int attr, QgsExpression* expression,
    QgsExpressionContext* context, QgsStatisticalSummary::Statistic stat )
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
  return s.statistic( stat );
}

QVariant QgsAggregateCalculator::calculateStringAggregate( QgsFeatureIterator& fit, int attr, QgsExpression* expression,
    QgsExpressionContext* context, QgsStringStatisticalSummary::Statistic stat )
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

QVariant QgsAggregateCalculator::calculateDateTimeAggregate( QgsFeatureIterator& fit, int attr, QgsExpression* expression,
    QgsExpressionContext* context, QgsDateTimeStatisticalSummary::Statistic stat )
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

QgsExpressionContext* QgsAggregateCalculator::createContext() const
{
  QgsExpressionContext* context = new QgsExpressionContext();
  context->appendScope( QgsExpressionContextUtils::globalScope() );
  context->appendScope( QgsExpressionContextUtils::projectScope() );
  context->appendScope( QgsExpressionContextUtils::layerScope( mLayer ) );
  return context;
}
