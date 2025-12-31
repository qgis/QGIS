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
#include "qgsnetworkdistancestrategy.h"
#include "qgsnetworkspeedstrategy.h"
#include "qgsunittypes.h"

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
  return u"networkanalysis"_s;
}

Qgis::ProcessingAlgorithmFlags QgsNetworkAnalysisAlgorithmBase::flags() const
{
  // TODO -- remove the dependency on the project from these algorithms, it shouldn't be required
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::RequiresProject;
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsNetworkAnalysisAlgorithmBase::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RespectsEllipsoid;
}

void QgsNetworkAnalysisAlgorithmBase::addCommonParams()
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Vector layer representing network" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) ) );
  addParameter( new QgsProcessingParameterEnum( u"STRATEGY"_s, QObject::tr( "Path type to calculate" ), QStringList() << QObject::tr( "Shortest" ) << QObject::tr( "Fastest" ), false, 0 ) );

  auto directionField = std::make_unique<QgsProcessingParameterField>( u"DIRECTION_FIELD"_s, QObject::tr( "Direction field" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Any, false, true );
  directionField->setFlags( directionField->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( directionField.release() );

  auto forwardValue = std::make_unique<QgsProcessingParameterString>( u"VALUE_FORWARD"_s, QObject::tr( "Value for forward direction" ), QVariant(), false, true );
  forwardValue->setFlags( forwardValue->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( forwardValue.release() );

  auto backwardValue = std::make_unique<QgsProcessingParameterString>( u"VALUE_BACKWARD"_s, QObject::tr( "Value for backward direction" ), QVariant(), false, true );
  backwardValue->setFlags( backwardValue->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( backwardValue.release() );

  auto bothValue = std::make_unique<QgsProcessingParameterString>( u"VALUE_BOTH"_s, QObject::tr( "Value for both directions" ), QVariant(), false, true );
  bothValue->setFlags( bothValue->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( bothValue.release() );

  auto directionValue = std::make_unique<QgsProcessingParameterEnum>( u"DEFAULT_DIRECTION"_s, QObject::tr( "Default direction" ), QStringList() << QObject::tr( "Forward direction" ) << QObject::tr( "Backward direction" ) << QObject::tr( "Both directions" ), false, 2 );
  directionValue->setFlags( directionValue->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( directionValue.release() );

  auto speedField = std::make_unique<QgsProcessingParameterField>( u"SPEED_FIELD"_s, QObject::tr( "Speed field" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Numeric, false, true );
  speedField->setFlags( speedField->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( speedField.release() );

  auto speed = std::make_unique<QgsProcessingParameterNumber>( u"DEFAULT_SPEED"_s, QObject::tr( "Default speed (km/h)" ), Qgis::ProcessingNumberParameterType::Double, 50, false, 0 );
  speed->setFlags( speed->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( speed.release() );

  std::unique_ptr<QgsProcessingParameterNumber> tolerance = std::make_unique<QgsProcessingParameterDistance>( u"TOLERANCE"_s, QObject::tr( "Topology tolerance" ), 0, u"INPUT"_s, false, 0 );
  tolerance->setFlags( tolerance->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( tolerance.release() );
}

void QgsNetworkAnalysisAlgorithmBase::loadCommonParams( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback )

  mNetwork.reset( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !mNetwork )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const int strategy = parameterAsInt( parameters, u"STRATEGY"_s, context );
  const QString directionFieldName = parameterAsString( parameters, u"DIRECTION_FIELD"_s, context );
  const QString forwardValue = parameterAsString( parameters, u"VALUE_FORWARD"_s, context );
  const QString backwardValue = parameterAsString( parameters, u"VALUE_BACKWARD"_s, context );
  const QString bothValue = parameterAsString( parameters, u"VALUE_BOTH"_s, context );
  const QgsVectorLayerDirector::Direction defaultDirection = static_cast<QgsVectorLayerDirector::Direction>( parameterAsInt( parameters, u"DEFAULT_DIRECTION"_s, context ) );
  const QString speedFieldName = parameterAsString( parameters, u"SPEED_FIELD"_s, context );
  const double defaultSpeed = parameterAsDouble( parameters, u"DEFAULT_SPEED"_s, context );
  const double tolerance = parameterAsDouble( parameters, u"TOLERANCE"_s, context );

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

  const Qgis::DistanceUnit distanceUnits = context.project()->crs().mapUnits();
  mMultiplier = QgsUnitTypes::fromUnitToUnitFactor( distanceUnits, Qgis::DistanceUnit::Meters );

  if ( strategy )
  {
    mDirector->addStrategy( new QgsNetworkSpeedStrategy( speedField, defaultSpeed, mMultiplier * 1000.0 / 3600.0 ) );
    mMultiplier = 3600;
  }
  else
  {
    mDirector->addStrategy( new QgsNetworkDistanceStrategy() );
  }

  mBuilder = std::make_unique<QgsGraphBuilder>( mNetwork->sourceCrs(), true, tolerance, context.ellipsoid() );
}

void QgsNetworkAnalysisAlgorithmBase::loadPoints( QgsFeatureSource *source, QVector<QgsPointXY> *points, QHash<int, QgsAttributes> *attributes, QgsProcessingContext &context, QgsProcessingFeedback *feedback, QHash<int, QgsFeature> *featureHash )
{
  feedback->pushInfo( QObject::tr( "Loading points…" ) );

  QgsFeature feat;
  int i = 0;
  int pointId = 1;
  const double step = source->featureCount() > 0 ? 100.0 / source->featureCount() : 0;
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

    const QgsGeometry geom = feat.geometry();
    QgsAbstractGeometry::vertex_iterator it = geom.vertices_begin();
    while ( it != geom.vertices_end() )
    {
      if ( points )
        points->push_back( QgsPointXY( *it ) );
      if ( attributes )
        attributes->insert( pointId, feat.attributes() );
      if ( featureHash )
        featureHash->insert( pointId, feat );
      it++;
      pointId++;
    }
  }
}

///@endcond
