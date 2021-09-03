/***************************************************************************
                         qgsalgorithmprojectpointcartesian.cpp
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

#include "qgsalgorithmprojectpointcartesian.h"
#include "qgsmultipoint.h"

///@cond PRIVATE

QString QgsProjectPointCartesianAlgorithm::name() const
{
  return QStringLiteral( "projectpointcartesian" );
}

QString QgsProjectPointCartesianAlgorithm::displayName() const
{
  return QObject::tr( "Project points (Cartesian)" );
}

QStringList QgsProjectPointCartesianAlgorithm::tags() const
{
  return QObject::tr( "bearing,azimuth,distance,angle" ).split( ',' );
}

QString QgsProjectPointCartesianAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsProjectPointCartesianAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsProjectPointCartesianAlgorithm::outputName() const
{
  return QObject::tr( "Projected" );
}

QString QgsProjectPointCartesianAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm projects point geometries by a specified distance and bearing (azimuth), creating a new point layer with the projected points.\n\n"
                      "The distance is specified in layer units, and the bearing in degrees clockwise from North." );
}

QList<int> QgsProjectPointCartesianAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorPoint;
}

QgsProcessing::SourceType QgsProjectPointCartesianAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorPoint;
}

QgsProjectPointCartesianAlgorithm *QgsProjectPointCartesianAlgorithm::createInstance() const
{
  return new QgsProjectPointCartesianAlgorithm();
}

void QgsProjectPointCartesianAlgorithm::initParameters( const QVariantMap & )
{
  std::unique_ptr< QgsProcessingParameterNumber > bearing = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "BEARING" ), QObject::tr( "Bearing (degrees from North)" ), QgsProcessingParameterNumber::Double, 0, false );
  bearing->setIsDynamic( true );
  bearing->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Bearing" ), QObject::tr( "Bearing (degrees from North)" ), QgsPropertyDefinition::Double ) );
  bearing->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( bearing.release() );

  std::unique_ptr< QgsProcessingParameterDistance > distance = std::make_unique< QgsProcessingParameterDistance >( QStringLiteral( "DISTANCE" ), QObject::tr( "Distance" ), 1, QStringLiteral( "INPUT" ), false );
  distance->setIsDynamic( true );
  distance->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Distance" ), QObject::tr( "Projection distance" ), QgsPropertyDefinition::Double ) );
  distance->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( distance.release() );
}

QgsProcessingFeatureSource::Flag QgsProjectPointCartesianAlgorithm::sourceFlags() const
{
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

bool QgsProjectPointCartesianAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mBearing = parameterAsDouble( parameters, QStringLiteral( "BEARING" ), context );
  mDynamicBearing = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "BEARING" ) );
  if ( mDynamicBearing )
    mBearingProperty = parameters.value( QStringLiteral( "BEARING" ) ).value< QgsProperty >();

  mDistance = parameterAsDouble( parameters, QStringLiteral( "DISTANCE" ), context );
  mDynamicDistance = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "DISTANCE" ) );
  if ( mDynamicDistance )
    mDistanceProperty = parameters.value( QStringLiteral( "DISTANCE" ) ).value< QgsProperty >();

  return true;
}

QgsFeatureList QgsProjectPointCartesianAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() && QgsWkbTypes::geometryType( f.geometry().wkbType() ) == QgsWkbTypes::PointGeometry )
  {
    double distance = mDistance;
    if ( mDynamicDistance )
      distance = mDistanceProperty.valueAsDouble( context.expressionContext(), distance );
    double bearing = mBearing;
    if ( mDynamicBearing )
      bearing = mBearingProperty.valueAsDouble( context.expressionContext(), bearing );

    const QgsGeometry g = f.geometry();
    if ( QgsWkbTypes::isMultiType( g.wkbType() ) )
    {
      const QgsMultiPoint *mp = static_cast< const QgsMultiPoint * >( g.constGet() );
      std::unique_ptr< QgsMultiPoint > result = std::make_unique< QgsMultiPoint >();
      result->reserve( mp->numGeometries() );
      for ( int i = 0; i < mp->numGeometries(); ++i )
      {
        const QgsPoint *p = mp->pointN( i );
        result->addGeometry( p->project( distance, bearing ).clone() );
      }
      f.setGeometry( QgsGeometry( std::move( result ) ) );
    }
    else
    {
      const QgsPoint *p = static_cast< const QgsPoint * >( g.constGet() );
      const QgsPoint result = p->project( distance, bearing );
      f.setGeometry( QgsGeometry( result.clone() ) );
    }
  }

  return QgsFeatureList() << f;
}

///@endcond

