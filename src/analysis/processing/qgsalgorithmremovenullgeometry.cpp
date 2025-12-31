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
  return u"removenullgeometries"_s;
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
  return u"vectorgeometry"_s;
}

void QgsRemoveNullGeometryAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterBoolean( u"REMOVE_EMPTY"_s, QObject::tr( "Also remove empty geometries" ), false ) );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Non null geometries" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true ) );
  QgsProcessingParameterFeatureSink *nullOutput = new QgsProcessingParameterFeatureSink( u"NULL_OUTPUT"_s, QObject::tr( "Null geometries" ), Qgis::ProcessingSourceType::Vector, QVariant(), true );
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

QString QgsRemoveNullGeometryAlgorithm::shortDescription() const
{
  return QObject::tr( "Removes any features which do not have a geometry from a vector layer." );
}

QgsRemoveNullGeometryAlgorithm *QgsRemoveNullGeometryAlgorithm::createInstance() const
{
  return new QgsRemoveNullGeometryAlgorithm();
}

QVariantMap QgsRemoveNullGeometryAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const bool removeEmpty = parameterAsBoolean( parameters, u"REMOVE_EMPTY"_s, context );

  QString nonNullSinkId;
  std::unique_ptr<QgsFeatureSink> nonNullSink( parameterAsSink( parameters, u"OUTPUT"_s, context, nonNullSinkId, source->fields(), source->wkbType(), source->sourceCrs() ) );

  QString nullSinkId;
  std::unique_ptr<QgsFeatureSink> nullSink( parameterAsSink( parameters, u"NULL_OUTPUT"_s, context, nullSinkId, source->fields() ) );

  const long count = source->featureCount();

  const double step = count > 0 ? 100.0 / count : 1;
  int current = 0;

  QgsFeature f;
  QgsFeatureIterator it = source->getFeatures( QgsFeatureRequest(), Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    if ( ( ( !removeEmpty && f.hasGeometry() ) || ( removeEmpty && !f.geometry().isEmpty() ) ) && nonNullSink )
    {
      if ( !nonNullSink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( nonNullSink.get(), parameters, u"OUTPUT"_s ) );
    }
    else if ( ( ( !removeEmpty && !f.hasGeometry() ) || ( removeEmpty && f.geometry().isEmpty() ) ) && nullSink )
    {
      if ( !nullSink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( nullSink.get(), parameters, u"NULL_OUTPUT"_s ) );
    }

    feedback->setProgress( current * step );
    current++;
  }

  QVariantMap outputs;
  if ( nonNullSink )
  {
    nonNullSink->finalize();
    outputs.insert( u"OUTPUT"_s, nonNullSinkId );
  }
  if ( nullSink )
  {
    nullSink->finalize();
    outputs.insert( u"NULL_OUTPUT"_s, nullSinkId );
  }
  return outputs;
}


///@endcond
