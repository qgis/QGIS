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
  return u"orderbyexpression"_s;
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
  return u"vectorgeneral"_s;
}

void QgsOrderByExpressionAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterExpression( u"EXPRESSION"_s, QObject::tr( "Expression" ), QVariant(), u"INPUT"_s ) );
  addParameter( new QgsProcessingParameterBoolean( u"ASCENDING"_s, QObject::tr( "Sort ascending" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( u"NULLS_FIRST"_s, QObject::tr( "Sort nulls first" ), false ) );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Ordered" ) ) );
}

QString QgsOrderByExpressionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm sorts a vector layer according to an expression. Be careful, it might not work as expected with some providers, "
                      "the order might not be kept every time.\n\n"
                      "For help with QGIS expression functions, see the inbuilt help for specific functions "
                      "which is available in the expression builder." );
}

QString QgsOrderByExpressionAlgorithm::shortDescription() const
{
  return QObject::tr( "Sorts a vector layer according to an expression." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsOrderByExpressionAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QgsOrderByExpressionAlgorithm *QgsOrderByExpressionAlgorithm::createInstance() const
{
  return new QgsOrderByExpressionAlgorithm();
}

QVariantMap QgsOrderByExpressionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const QString expressionString = parameterAsExpression( parameters, u"EXPRESSION"_s, context );

  const bool ascending = parameterAsBoolean( parameters, u"ASCENDING"_s, context );
  const bool nullsFirst = parameterAsBoolean( parameters, u"NULLS_FIRST"_s, context );

  QString sinkId;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, sinkId, source->fields(), source->wkbType(), source->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  const long count = source->featureCount();
  const double step = count > 0 ? 100.0 / count : 1;
  int current = 0;

  QgsFeatureRequest request;
  request.addOrderBy( expressionString, ascending, nullsFirst );

  QgsFeature inFeature;
  QgsFeatureIterator features = source->getFeatures( request, Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
  while ( features.nextFeature( inFeature ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }
    if ( !sink->addFeature( inFeature ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
    feedback->setProgress( current * step );
    current++;
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, sinkId );
  return outputs;
}

///@endcond
