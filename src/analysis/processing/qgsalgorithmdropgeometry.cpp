/***************************************************************************
                         qgsalgorithmdropgeometry.cpp
                         ---------------------
    begin                : April 2017
    copyright            : (C) 2020 by Wang Peng
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmdropgeometry.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsDropGeometryAlgorithm::name() const
{
  return QStringLiteral( "dropgeometries" );
}

QString QgsDropGeometryAlgorithm::displayName() const
{
  return QObject::tr( "Drop geometries" );
}

QStringList QgsDropGeometryAlgorithm::tags() const
{
  return QObject::tr( "remove,drop,delete,geometry,objects" ).split( ',' );
}

QString QgsDropGeometryAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsDropGeometryAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeneral" );
}

QString QgsDropGeometryAlgorithm::outputName() const
{
  return QObject::tr( "Dropped geometries" );
}

QString QgsDropGeometryAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm removes any geometries from an input layer and returns a layer containing only the feature attributes." );
}

QgsDropGeometryAlgorithm *QgsDropGeometryAlgorithm::createInstance() const
{
  return new QgsDropGeometryAlgorithm();
}

QgsCoordinateReferenceSystem QgsDropGeometryAlgorithm::outputCrs( const QgsCoordinateReferenceSystem & ) const
{
  return QgsCoordinateReferenceSystem();
}

bool QgsDropGeometryAlgorithm::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  return qobject_cast< const QgsVectorLayer * >( l );
}

QgsWkbTypes::Type QgsDropGeometryAlgorithm::outputWkbType( QgsWkbTypes::Type ) const
{
  return QgsWkbTypes::NoGeometry;
}

QgsProcessingFeatureSource::Flag QgsDropGeometryAlgorithm::sourceFlags() const
{
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

QgsFeatureRequest QgsDropGeometryAlgorithm::request() const
{
  return QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry );
}

QgsFeatureList QgsDropGeometryAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  f.clearGeometry();
  return QgsFeatureList() << f;
}

///@endcond
