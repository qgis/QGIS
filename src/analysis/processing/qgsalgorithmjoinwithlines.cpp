/***************************************************************************
                         qgsalgorithmjoinwithlines.cpp
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

#include "qgsalgorithmjoinwithlines.h"
#include "qgslinestring.h"

///@cond PRIVATE

QString QgsJoinWithLinesAlgorithm::name() const
{
  return QStringLiteral( "hublines" );
}

QString QgsJoinWithLinesAlgorithm::displayName() const
{
  return QObject::tr( "Join by lines (hub lines)" );
}

QStringList QgsJoinWithLinesAlgorithm::tags() const
{
  return QObject::tr( "join,connect,lines,points,hub,spoke" ).split( ',' );
}

QString QgsJoinWithLinesAlgorithm::group() const
{
  return QObject::tr( "Vector analysis" );
}

QString QgsJoinWithLinesAlgorithm::groupId() const
{
  return QStringLiteral( "vectoranalysis" );
}

QgsProcessingAlgorithm::Flags QgsJoinWithLinesAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

void QgsJoinWithLinesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "HUBS" ),
                QObject::tr( "Hub layer" ) ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "HUB_FIELD" ),
                QObject::tr( "Hub ID field" ), QVariant(), QStringLiteral( "HUBS" ) ) );

  addParameter( new QgsProcessingParameterField( QStringLiteral( "HUB_FIELDS" ),
                QObject::tr( "Hub layer fields to copy (leave empty to copy all fields)" ),
                QVariant(), QStringLiteral( "HUBS" ), QgsProcessingParameterField::Any,
                true, true ) );

  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "SPOKES" ),
                QObject::tr( "Spoke layer" ) ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "SPOKE_FIELD" ),
                QObject::tr( "Spoke ID field" ), QVariant(), QStringLiteral( "SPOKES" ) ) );

  addParameter( new QgsProcessingParameterField( QStringLiteral( "SPOKE_FIELDS" ),
                QObject::tr( "Spoke layer fields to copy (leave empty to copy all fields)" ),
                QVariant(), QStringLiteral( "SPOKES" ), QgsProcessingParameterField::Any,
                true, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Hub lines" ), QgsProcessing::TypeVectorLine ) );
}

QString QgsJoinWithLinesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates hub and spoke diagrams by connecting lines from points on the Spoke layer to matching points in the Hub layer.\n\n"
                      "Determination of which hub goes with each point is based on a match between the Hub ID field on the hub points and the Spoke ID field on the spoke points.\n\n"
                      "If input layers are not point layers, a point on the surface of the geometries will be taken as the connecting location." );
}

QgsJoinWithLinesAlgorithm *QgsJoinWithLinesAlgorithm::createInstance() const
{
  return new QgsJoinWithLinesAlgorithm();
}

QVariantMap QgsJoinWithLinesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( parameters.value( QStringLiteral( "SPOKES" ) ) == parameters.value( QStringLiteral( "HUBS" ) ) )
    throw QgsProcessingException( QObject::tr( "Same layer given for both hubs and spokes" ) );

  std::unique_ptr< QgsFeatureSource > hubSource( parameterAsSource( parameters, QStringLiteral( "HUBS" ), context ) );
  std::unique_ptr< QgsFeatureSource > spokeSource( parameterAsSource( parameters, QStringLiteral( "SPOKES" ), context ) );
  if ( !hubSource || !spokeSource )
    return QVariantMap();

  QString fieldHubName = parameterAsString( parameters, QStringLiteral( "HUB_FIELD" ), context );
  int fieldHubIndex = hubSource->fields().lookupField( fieldHubName );
  const QStringList hubFieldsToCopy = parameterAsFields( parameters, QStringLiteral( "HUB_FIELDS" ), context );

  QString fieldSpokeName = parameterAsString( parameters, QStringLiteral( "SPOKE_FIELD" ), context );
  int fieldSpokeIndex = spokeSource->fields().lookupField( fieldSpokeName );
  const QStringList spokeFieldsToCopy = parameterAsFields( parameters, QStringLiteral( "SPOKE_FIELDS" ), context );

  if ( fieldHubIndex < 0 || fieldSpokeIndex < 0 )
    throw QgsProcessingException( QObject::tr( "Invalid ID field" ) );

  QgsFields hubOutFields;
  QgsAttributeList hubFieldIndices;
  if ( hubFieldsToCopy.empty() )
  {
    hubOutFields = hubSource->fields();
    for ( int i = 0; i < hubOutFields.count(); ++i )
    {
      hubFieldIndices << i;
    }
  }
  else
  {
    for ( const QString &field : hubFieldsToCopy )
    {
      int index = hubSource->fields().lookupField( field );
      if ( index >= 0 )
      {
        hubFieldIndices << index;
        hubOutFields.append( hubSource->fields().at( index ) );
      }
    }
  }

  QgsAttributeList hubFields2Fetch = hubFieldIndices;
  hubFields2Fetch << fieldHubIndex;

  QgsFields spokeOutFields;
  QgsAttributeList spokeFieldIndices;
  if ( spokeFieldsToCopy.empty() )
  {
    spokeOutFields = spokeSource->fields();
    for ( int i = 0; i < spokeOutFields.count(); ++i )
    {
      spokeFieldIndices << i;
    }
  }
  else
  {
    for ( const QString &field : spokeFieldsToCopy )
    {
      int index = spokeSource->fields().lookupField( field );
      if ( index >= 0 )
      {
        spokeFieldIndices << index;
        spokeOutFields.append( spokeSource->fields().at( index ) );
      }
    }
  }

  QgsAttributeList spokeFields2Fetch = spokeFieldIndices;
  spokeFields2Fetch << fieldSpokeIndex;


  QgsFields fields = QgsProcessingUtils::combineFields( hubOutFields, spokeOutFields );

  QgsWkbTypes::Type outType = QgsWkbTypes::LineString;
  bool hasZ = false;
  if ( QgsWkbTypes::hasZ( hubSource->wkbType() ) || QgsWkbTypes::hasZ( spokeSource->wkbType() ) )
  {
    outType = QgsWkbTypes::addZ( outType );
    hasZ = true;
  }
  bool hasM = false;
  if ( QgsWkbTypes::hasM( hubSource->wkbType() ) || QgsWkbTypes::hasM( spokeSource->wkbType() ) )
  {
    outType = QgsWkbTypes::addM( outType );
    hasM = true;
  }

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields,
                                          outType, hubSource->sourceCrs() ) );
  if ( !sink )
    return QVariantMap();

  auto getPointFromFeature = [hasZ, hasM]( const QgsFeature & feature )->QgsPoint
  {
    QgsPoint p;
    if ( feature.geometry().type() == QgsWkbTypes::PointGeometry && !feature.geometry().isMultipart() )
      p = *static_cast< const QgsPoint *>( feature.geometry().constGet() );
    else
      p = *static_cast< const QgsPoint *>( feature.geometry().pointOnSurface().constGet() );
    if ( hasZ && !p.is3D() )
      p.addZValue( 0 );
    if ( hasM && !p.isMeasure() )
      p.addMValue( 0 );
    return p;
  };

  QgsFeatureIterator hubFeatures = hubSource->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( hubFields2Fetch ) );
  double step = hubSource->featureCount() > 0 ? 100.0 / hubSource->featureCount() : 1;
  int i = 0;
  QgsFeature hubFeature;
  while ( hubFeatures.nextFeature( hubFeature ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( i * step );

    if ( !hubFeature.hasGeometry() )
      continue;

    QgsPoint hubPoint = getPointFromFeature( hubFeature );

    // only keep selected attributes
    QgsAttributes hubAttributes;
    for ( int j = 0; j < hubFeature.attributes().count(); ++j )
    {
      if ( !hubFieldIndices.contains( j ) )
        continue;
      hubAttributes << hubFeature.attribute( j );
    }

    QgsFeatureRequest spokeRequest = QgsFeatureRequest().setDestinationCrs( hubSource->sourceCrs(), context.transformContext() );
    spokeRequest.setSubsetOfAttributes( spokeFields2Fetch );
    spokeRequest.setFilterExpression( QgsExpression::createFieldEqualityExpression( fieldSpokeName, hubFeature.attribute( fieldHubIndex ) ) );

    QgsFeatureIterator spokeFeatures = spokeSource->getFeatures( spokeRequest );
    QgsFeature spokeFeature;
    while ( spokeFeatures.nextFeature( spokeFeature ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }
      if ( !spokeFeature.hasGeometry() )
        continue;

      QgsPoint spokePoint = getPointFromFeature( spokeFeature );
      QgsGeometry line( new QgsLineString( QVector< QgsPoint >() << hubPoint << spokePoint ) );

      QgsFeature outFeature;
      QgsAttributes outAttributes = hubAttributes;

      // only keep selected attributes
      QgsAttributes spokeAttributes;
      for ( int j = 0; j < spokeFeature.attributes().count(); ++j )
      {
        if ( !spokeFieldIndices.contains( j ) )
          continue;
        spokeAttributes << spokeFeature.attribute( j );
      }

      outAttributes.append( spokeAttributes );
      outFeature.setAttributes( outAttributes );
      outFeature.setGeometry( line );
      sink->addFeature( outFeature, QgsFeatureSink::FastInsert );
    }
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond
