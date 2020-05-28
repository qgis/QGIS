/***************************************************************************
                         qgsalgorithmnetworkanalysisbase.cpp
                         ---------------------
    begin                : July 2018
    copyright            : (C) 2018 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmnetworkanalysisbase.h"

#include "qgsgraphanalyzer.h"
#include "qgsnetworkspeedstrategy.h"
#include "qgsnetworkdistancestrategy.h"

///@cond PRIVATE

//
// QgsNetworkAnalysisAlgorithmBase
//

QString QgsNetworkAnalysisAlgorithmBase::group() const
{
  return QObject::tr( "Network analysis" );
}

QString QgsNetworkAnalysisAlgorithmBase::groupId() const
{
  return QStringLiteral( "networkanalysis" );
}

void QgsNetworkAnalysisAlgorithmBase::addCommonParams()
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Vector layer representing network" ), QList< int >() << QgsProcessing::TypeVectorLine ) );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "STRATEGY" ), QObject::tr( "Path type to calculate" ), QStringList() << QObject::tr( "Shortest" ) << QObject::tr( "Fastest" ), false, 0 ) );

  std::unique_ptr< QgsProcessingParameterField > directionField = qgis::make_unique< QgsProcessingParameterField >( QStringLiteral( "DIRECTION_FIELD" ),
      QObject::tr( "Direction field" ), QVariant(), QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any, false, true );
  directionField->setFlags( directionField->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( directionField.release() );

  std::unique_ptr< QgsProcessingParameterString > forwardValue = qgis::make_unique< QgsProcessingParameterString >( QStringLiteral( "VALUE_FORWARD" ),
      QObject::tr( "Value for forward direction" ), QVariant(), false, true );
  forwardValue->setFlags( forwardValue->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( forwardValue.release() );

  std::unique_ptr< QgsProcessingParameterString > backwardValue = qgis::make_unique< QgsProcessingParameterString >( QStringLiteral( "VALUE_BACKWARD" ),
      QObject::tr( "Value for backward direction" ), QVariant(), false, true );
  backwardValue->setFlags( backwardValue->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( backwardValue.release() );

  std::unique_ptr< QgsProcessingParameterString > bothValue = qgis::make_unique< QgsProcessingParameterString >( QStringLiteral( "VALUE_BOTH" ),
      QObject::tr( "Value for both directions" ), QVariant(), false, true );
  bothValue->setFlags( bothValue->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( bothValue.release() );

  std::unique_ptr< QgsProcessingParameterEnum > directionValue = qgis::make_unique< QgsProcessingParameterEnum >( QStringLiteral( "DEFAULT_DIRECTION" ),
      QObject::tr( "Default direction" ), QStringList() << QObject::tr( "Forward direction" ) << QObject::tr( "Backward direction" ) << QObject::tr( "Both directions" ), false, 2 );
  directionValue->setFlags( directionValue->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( directionValue.release() );

  std::unique_ptr< QgsProcessingParameterField > speedField = qgis::make_unique< QgsProcessingParameterField >( QStringLiteral( "SPEED_FIELD" ),
      QObject::tr( "Speed field" ), QVariant(), QStringLiteral( "INPUT" ), QgsProcessingParameterField::Numeric, false, true );
  speedField->setFlags( speedField->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( speedField.release() );

  std::unique_ptr< QgsProcessingParameterNumber > speed = qgis::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "DEFAULT_SPEED" ), QObject::tr( "Default speed (km/h)" ), QgsProcessingParameterNumber::Double, 50, false, 0 );
  speed->setFlags( speed->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( speed.release() );

  std::unique_ptr< QgsProcessingParameterNumber > tolerance = qgis::make_unique < QgsProcessingParameterDistance >( QStringLiteral( "TOLERANCE" ), QObject::tr( "Topology tolerance" ), 0, QStringLiteral( "INPUT" ), false, 0 );
  tolerance->setFlags( tolerance->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( tolerance.release() );
}

void QgsNetworkAnalysisAlgorithmBase::loadCommonParams( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback )

  mNetwork.reset( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !mNetwork )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  int strategy = parameterAsInt( parameters, QStringLiteral( "STRATEGY" ), context );
  QString directionFieldName = parameterAsString( parameters, QStringLiteral( "DIRECTION_FIELD" ), context );
  QString forwardValue = parameterAsString( parameters, QStringLiteral( "VALUE_FORWARD" ), context );
  QString backwardValue = parameterAsString( parameters, QStringLiteral( "VALUE_BACKWARD" ), context );
  QString bothValue = parameterAsString( parameters, QStringLiteral( "VALUE_BOTH" ), context );
  QgsVectorLayerDirector::Direction defaultDirection = static_cast< QgsVectorLayerDirector::Direction>( parameterAsInt( parameters, QStringLiteral( "DEFAULT_DIRECTION" ), context ) );
  QString speedFieldName = parameterAsString( parameters, QStringLiteral( "SPEED_FIELD" ), context );
  double defaultSpeed = parameterAsDouble( parameters, QStringLiteral( "DEFAULT_SPEED" ), context );
  double tolerance = parameterAsDouble( parameters, QStringLiteral( "TOLERANCE" ), context );

  int directionField = -1;
  if ( !directionFieldName.isEmpty() )
  {
    directionField = mNetwork->fields().lookupField( directionFieldName );
  }

  int speedField = -1;
  if ( !speedFieldName.isEmpty() )
  {
    speedField = mNetwork->fields().lookupField( speedFieldName );
  }

  mDirector = new QgsVectorLayerDirector( mNetwork.get(), directionField, forwardValue, backwardValue, bothValue, defaultDirection );

  QgsUnitTypes::DistanceUnit distanceUnits = context.project()->crs().mapUnits();
  mMultiplier = QgsUnitTypes::fromUnitToUnitFactor( distanceUnits, QgsUnitTypes::DistanceMeters );

  if ( strategy )
  {
    mDirector->addStrategy( new QgsNetworkSpeedStrategy( speedField, defaultSpeed, mMultiplier * 1000.0 / 3600.0 ) );
    mMultiplier = 3600;
  }
  else
  {
    mDirector->addStrategy( new QgsNetworkDistanceStrategy() );
  }

  mBuilder = qgis::make_unique< QgsGraphBuilder >( mNetwork->sourceCrs(), true, tolerance );
}

void QgsNetworkAnalysisAlgorithmBase::loadPoints( QgsFeatureSource *source, QVector< QgsPointXY > &points, QHash< int, QgsAttributes > &attributes, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  feedback->pushInfo( QObject::tr( "Loading points…" ) );

  QgsFeature feat;
  int i = 0;
  int pointId = 1;
  double step = source->featureCount() > 0 ? 100.0 / source->featureCount() : 0;
  QgsFeatureIterator features = source->getFeatures( QgsFeatureRequest().setDestinationCrs( mNetwork->sourceCrs(), context.transformContext() ) );

  while ( features.nextFeature( feat ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( i * step );
    if ( !feat.hasGeometry() )
      continue;

    QgsGeometry geom = feat.geometry();
    QgsAbstractGeometry::vertex_iterator it = geom.vertices_begin();
    while ( it != geom.vertices_end() )
    {
      points.push_back( QgsPointXY( *it ) );
      attributes.insert( pointId, feat.attributes() );
      it++;
      pointId++;
    }
  }
}

///@endcond
