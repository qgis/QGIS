/***************************************************************************
                         qgsalgorithmcentroid.cpp
                         ------------------------
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

#include "qgsalgorithmcentroid.h"
#include "qgsgeometrycollection.h"

///@cond PRIVATE

QString QgsCentroidAlgorithm::name() const
{
  return QStringLiteral( "centroids" );
}

QString QgsCentroidAlgorithm::displayName() const
{
  return QObject::tr( "Centroids" );
}

QStringList QgsCentroidAlgorithm::tags() const
{
  return QObject::tr( "centroid,center,average,point,middle" ).split( ',' );
}

QString QgsCentroidAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsCentroidAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsCentroidAlgorithm::outputName() const
{
  return QObject::tr( "Centroids" );
}

QgsFeatureSink::SinkFlags QgsCentroidAlgorithm::sinkFlags() const
{
  if ( mAllParts )
    return QgsProcessingFeatureBasedAlgorithm::sinkFlags() | QgsFeatureSink::RegeneratePrimaryKey;
  else
    return QgsProcessingFeatureBasedAlgorithm::sinkFlags();
}

QString QgsCentroidAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new point layer, with points representing the centroid of the geometries in an input layer.\n\n"
                      "The attributes associated to each point in the output layer are the same ones associated to the original features." );
}

QgsCentroidAlgorithm *QgsCentroidAlgorithm::createInstance() const
{
  return new QgsCentroidAlgorithm();
}

void QgsCentroidAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterBoolean> allParts = std::make_unique< QgsProcessingParameterBoolean >(
        QStringLiteral( "ALL_PARTS" ),
        QObject::tr( "Create centroid for each part" ),
        false );
  allParts->setIsDynamic( true );
  allParts->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "All parts" ), QObject::tr( "Create centroid for each part" ), QgsPropertyDefinition::Boolean ) );
  allParts->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( allParts.release() );
}

bool QgsCentroidAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mAllParts = parameterAsBoolean( parameters, QStringLiteral( "ALL_PARTS" ), context );
  mDynamicAllParts = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "ALL_PARTS" ) );
  if ( mDynamicAllParts )
    mAllPartsProperty = parameters.value( QStringLiteral( "ALL_PARTS" ) ).value< QgsProperty >();

  return true;
}

QgsFeatureList QgsCentroidAlgorithm::processFeature( const QgsFeature &f, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsFeatureList list;
  QgsFeature feature = f;
  if ( feature.hasGeometry() && !feature.geometry().isEmpty() )
  {
    const QgsGeometry geom = feature.geometry();

    bool allParts = mAllParts;
    if ( mDynamicAllParts )
      allParts = mAllPartsProperty.valueAsBool( context.expressionContext(), allParts );

    if ( allParts && geom.isMultipart() )
    {
      const QgsGeometryCollection *geomCollection = static_cast<const QgsGeometryCollection *>( geom.constGet() );

      const int partCount = geomCollection->partCount();
      list.reserve( partCount );
      for ( int i = 0; i < partCount; ++i )
      {
        const QgsGeometry partGeometry( geomCollection->geometryN( i )->clone() );
        const QgsGeometry outputGeometry = partGeometry.centroid();
        if ( outputGeometry.isNull() )
        {
          feedback->reportError( QObject::tr( "Error calculating centroid for feature %1 part %2: %3" ).arg( feature.id() ).arg( i ).arg( outputGeometry.lastError() ) );
        }
        feature.setGeometry( outputGeometry );
        list << feature;
      }
    }
    else
    {
      const QgsGeometry outputGeometry = feature.geometry().centroid();
      if ( outputGeometry.isNull() )
      {
        feedback->reportError( QObject::tr( "Error calculating centroid for feature %1: %2" ).arg( feature.id() ).arg( outputGeometry.lastError() ) );
      }
      feature.setGeometry( outputGeometry );
      list << feature;
    }
  }
  else
  {
    list << feature;
  }
  return list;
}

///@endcond
