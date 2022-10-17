/***************************************************************************
                         qgsalgorithmfixgeometries.cpp
                         -----------------------------
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

#include "qgsalgorithmfixgeometries.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsFixGeometriesAlgorithm::name() const
{
  return QStringLiteral( "fixgeometries" );
}

QString QgsFixGeometriesAlgorithm::displayName() const
{
  return QObject::tr( "Fix geometries" );
}

QStringList QgsFixGeometriesAlgorithm::tags() const
{
  return QObject::tr( "repair,invalid,geometry,make,valid,error" ).split( ',' );
}

QString QgsFixGeometriesAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsFixGeometriesAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QgsProcessingFeatureSource::Flag QgsFixGeometriesAlgorithm::sourceFlags() const
{
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

QString QgsFixGeometriesAlgorithm::outputName() const
{
  return QObject::tr( "Fixed geometries" );
}

QgsWkbTypes::Type QgsFixGeometriesAlgorithm::outputWkbType( QgsWkbTypes::Type type ) const
{
  return QgsWkbTypes::promoteNonPointTypesToMulti( type );
}

QString QgsFixGeometriesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm attempts to create a valid representation of a given invalid geometry without "
                      "losing any of the input vertices. Already-valid geometries are returned without further intervention. "
                      "Always outputs multi-geometry layer.\n\n"
                      "NOTE: M values will be dropped from the output." );
}

QgsFixGeometriesAlgorithm *QgsFixGeometriesAlgorithm::createInstance() const
{
  return new QgsFixGeometriesAlgorithm();
}

bool QgsFixGeometriesAlgorithm::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  const QgsVectorLayer *layer = qobject_cast< const QgsVectorLayer * >( l );
  if ( !layer )
    return false;

  if ( !layer->isSpatial() || ! QgsProcessingFeatureBasedAlgorithm::supportInPlaceEdit( layer ) )
    return false;
  // The algorithm would drop M, so disable it if the layer has M
  return ! QgsWkbTypes::hasM( layer->wkbType() );
}

void QgsFixGeometriesAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterEnum> methodParameter = std::make_unique< QgsProcessingParameterEnum >(
        QStringLiteral( "METHOD" ),
        QObject::tr( "Repair method" ),
        QStringList{ QObject::tr( "Linework" ), QObject::tr( "Structure" ) },
        0,
        false );
#if GEOS_VERSION_MAJOR==3 && GEOS_VERSION_MINOR<10
  methodParameter->setDefaultValue( 0 );
#else
  methodParameter->setDefaultValue( 1 );
#endif
  addParameter( methodParameter.release() );
}

bool QgsFixGeometriesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mMethod = static_cast< Qgis::MakeValidMethod>( parameterAsInt( parameters, QStringLiteral( "METHOD" ), context ) );
#if GEOS_VERSION_MAJOR==3 && GEOS_VERSION_MINOR<10
  if ( mMethod == Qgis::MakeValidMethod::Structure )
  {
    throw QgsProcessingException( "The structured method to make geometries valid requires a QGIS build based on GEOS 3.10 or later" );
  }
#endif
  return true;
}

QgsFeatureList QgsFixGeometriesAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  if ( !feature.hasGeometry() )
    return QgsFeatureList() << feature;

  QgsFeature outputFeature = feature;

  QgsGeometry outputGeometry = outputFeature.geometry().makeValid( mMethod );
  if ( outputGeometry.isNull() )
  {
    feedback->pushInfo( QObject::tr( "makeValid failed for feature %1 " ).arg( feature.id() ) );
    outputFeature.clearGeometry();
    return QgsFeatureList() << outputFeature;
  }

  if ( outputGeometry.wkbType() == QgsWkbTypes::Unknown ||
       QgsWkbTypes::flatType( outputGeometry.wkbType() ) == QgsWkbTypes::GeometryCollection )
  {
    // keep only the parts of the geometry collection with correct type
    const QVector< QgsGeometry > tmpGeometries = outputGeometry.asGeometryCollection();
    QVector< QgsGeometry > matchingParts;
    for ( const QgsGeometry &g : tmpGeometries )
    {
      if ( g.type() == feature.geometry().type() )
        matchingParts << g;
    }
    if ( !matchingParts.empty() )
      outputGeometry = QgsGeometry::collectGeometry( matchingParts );
    else
      outputGeometry = QgsGeometry();
  }

  if ( outputGeometry.type() != QgsWkbTypes::GeometryType::PointGeometry )
  {
    // some data providers are picky about the geometries we pass to them: we can't add single-part geometries
    // when we promised multi-part geometries, so ensure we have the right type
    outputGeometry.convertToMultiType();
  }

  if ( QgsWkbTypes::geometryType( outputGeometry.wkbType() ) != QgsWkbTypes::geometryType( feature.geometry().wkbType() ) )
  {
    // don't keep geometries which have different types - e.g. lines converted to points
    feedback->pushInfo( QObject::tr( "Fixing geometry for feature %1 resulted in %2, geometry has been dropped." ).arg( feature.id() ).arg( QgsWkbTypes::displayString( outputGeometry.wkbType() ) ) );
    outputFeature.clearGeometry();
  }
  else
  {
    outputFeature.setGeometry( outputGeometry );
  }
  return QgsFeatureList() << outputFeature;
}

///@endcond
