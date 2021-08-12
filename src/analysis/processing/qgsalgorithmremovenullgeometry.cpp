/***************************************************************************
                         qgsalgorithmremovenullgeometry.cpp
                         ---------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsalgorithmremovenullgeometry.h"

///@cond PRIVATE

QString QgsRemoveNullGeometryAlgorithm::name() const
{
  return QStringLiteral( "removenullgeometries" );
}

QString QgsRemoveNullGeometryAlgorithm::displayName() const
{
  return QObject::tr( "Remove null geometries" );
}

QStringList QgsRemoveNullGeometryAlgorithm::tags() const
{
  return QObject::tr( "remove,drop,delete,empty,geometry" ).split( ',' );
}

QString QgsRemoveNullGeometryAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsRemoveNullGeometryAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

void QgsRemoveNullGeometryAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "REMOVE_EMPTY" ), QObject::tr( "Also remove empty geometries" ), false ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Non null geometries" ),
                QgsProcessing::TypeVectorAnyGeometry, QVariant(), true ) );
  QgsProcessingParameterFeatureSink *nullOutput = new QgsProcessingParameterFeatureSink( QStringLiteral( "NULL_OUTPUT" ),  QObject::tr( "Null geometries" ),
      QgsProcessing::TypeVector, QVariant(), true );
  nullOutput->setCreateByDefault( false );
  addParameter( nullOutput );
}

QString QgsRemoveNullGeometryAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm removes any features which do not have a geometry from a vector layer. "
                      "All other features will be copied unchanged.\n\n"
                      "Optionally, the features with null geometries can be saved to a separate output.\n\n"
                      "If 'Also remove empty geometries' is checked, the algorithm removes features whose geometries "
                      "have no coordinates, i.e., geometries that are empty. In that case, also the null "
                      "output will reflect this option, containing both null and empty geometries." );
}

QgsRemoveNullGeometryAlgorithm *QgsRemoveNullGeometryAlgorithm::createInstance() const
{
  return new QgsRemoveNullGeometryAlgorithm();
}

QVariantMap QgsRemoveNullGeometryAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const bool removeEmpty = parameterAsBoolean( parameters, QStringLiteral( "REMOVE_EMPTY" ), context );

  QString nonNullSinkId;
  std::unique_ptr< QgsFeatureSink > nonNullSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, nonNullSinkId, source->fields(),
      source->wkbType(), source->sourceCrs() ) );

  QString nullSinkId;
  std::unique_ptr< QgsFeatureSink > nullSink( parameterAsSink( parameters, QStringLiteral( "NULL_OUTPUT" ), context, nullSinkId, source->fields() ) );

  const long count = source->featureCount();

  const double step = count > 0 ? 100.0 / count : 1;
  int current = 0;

  QgsFeature f;
  QgsFeatureIterator it = source->getFeatures( QgsFeatureRequest(), QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks );
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    if ( ( ( !removeEmpty && f.hasGeometry() ) || ( removeEmpty && !f.geometry().isEmpty() ) ) && nonNullSink )
    {
      if ( !nonNullSink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( nonNullSink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
    }
    else if ( ( ( !removeEmpty && !f.hasGeometry() ) || ( removeEmpty && f.geometry().isEmpty() ) ) && nullSink )
    {
      if ( !nullSink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( nullSink.get(), parameters, QStringLiteral( "NULL_OUTPUT" ) ) );
    }

    feedback->setProgress( current * step );
    current++;
  }

  QVariantMap outputs;
  if ( nonNullSink )
    outputs.insert( QStringLiteral( "OUTPUT" ), nonNullSinkId );
  if ( nullSink )
    outputs.insert( QStringLiteral( "NULL_OUTPUT" ), nullSinkId );
  return outputs;
}


///@endcond


