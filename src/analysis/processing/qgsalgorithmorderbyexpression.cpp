/***************************************************************************
                         qgsalgorithmorderbyexpression.h
                         ---------------------
    begin                : November 2017
    copyright            : (C) 2017 by Etienne Trimaille
    email                : etienne dot trimaille at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsalgorithmorderbyexpression.h"
#include "qgsfeaturerequest.h"


///@cond PRIVATE

QString QgsOrderByExpressionAlgorithm::name() const
{
  return QStringLiteral( "orderbyexpression" );
}

QString QgsOrderByExpressionAlgorithm::displayName() const
{
  return QObject::tr( "Order by expression" );
}

QStringList QgsOrderByExpressionAlgorithm::tags() const
{
  return QObject::tr( "orderby,sort,expression,field" ).split( ',' );
}

QString QgsOrderByExpressionAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsOrderByExpressionAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeneral" );
}

QgsProcessingAlgorithm::Flags QgsOrderByExpressionAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

void QgsOrderByExpressionAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterExpression( QStringLiteral( "EXPRESSION" ), QObject::tr( "Expression" ), QVariant(), QStringLiteral( "INPUT" ) ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "ASCENDING" ), QObject::tr( "Ascending" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "NULLS_FIRST" ), QObject::tr( "Nulls first" ), false ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Output layer" ) ) );
}

QString QgsOrderByExpressionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm sorts a vector layer according to an expression. Be careful, it might not work as expected with some providers, the order might not be kept every time." );
}

QgsOrderByExpressionAlgorithm *QgsOrderByExpressionAlgorithm::createInstance() const
{
  return new QgsOrderByExpressionAlgorithm();
}

QVariantMap QgsOrderByExpressionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  QString expressionString = parameterAsExpression( parameters, QStringLiteral( "EXPRESSION" ), context );

  bool ascending = parameterAsBool( parameters, QStringLiteral( "ASCENDING" ), context );
  bool nullsFirst = parameterAsBool( parameters, QStringLiteral( "NULLS_FIRST" ), context );

  QString sinkId;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, sinkId, source->fields(), source->wkbType(), source->sourceCrs() ) );
  if ( !sink )
    return QVariantMap();

  long count = source->featureCount();
  double step = count > 0 ? 100.0 / count : 1;
  int current = 0;

  QgsFeatureRequest request;
  request.addOrderBy( expressionString, ascending, nullsFirst );

  QgsFeature inFeature;
  QgsFeatureIterator features = source->getFeatures( request );
  while ( features.nextFeature( inFeature ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }
    sink->addFeature( inFeature );
    feedback->setProgress( current * step );
    current++;
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), sinkId );
  return outputs;
}

///@endcond
