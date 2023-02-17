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
#include "qgsmultilinestring.h"
#include "qgsdistancearea.h"

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
  return QObject::tr( "join,connect,lines,points,hub,spoke,geodesic,great,circle" ).split( ',' );
}

QString QgsJoinWithLinesAlgorithm::group() const
{
  return QObject::tr( "Vector analysis" );
}

QString QgsJoinWithLinesAlgorithm::groupId() const
{
  return QStringLiteral( "vectoranalysis" );
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

  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "GEODESIC" ), QObject::tr( "Create geodesic lines" ), false ) );

  auto distanceParam = std::make_unique< QgsProcessingParameterDistance >( QStringLiteral( "GEODESIC_DISTANCE" ), QObject::tr( "Distance between vertices (geodesic lines only)" ), 1000 );
  distanceParam->setFlags( distanceParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  distanceParam->setDefaultUnit( QgsUnitTypes::DistanceKilometers );
  distanceParam->setIsDynamic( true );
  distanceParam->setDynamicPropertyDefinition( QgsPropertyDefinition( QStringLiteral( "Geodesic Distance" ), QObject::tr( "Distance between vertices" ), QgsPropertyDefinition::DoublePositive ) );
  distanceParam->setDynamicLayerParameterName( QStringLiteral( "HUBS" ) );
  addParameter( distanceParam.release() );

  auto breakParam = std::make_unique< QgsProcessingParameterBoolean >( QStringLiteral( "ANTIMERIDIAN_SPLIT" ), QObject::tr( "Split lines at antimeridian (±180 degrees longitude)" ), false );
  breakParam->setFlags( breakParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( breakParam.release() );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Hub lines" ), QgsProcessing::TypeVectorLine ) );
}

QString QgsJoinWithLinesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates hub and spoke diagrams by connecting lines from points on the Spoke layer to matching points in the Hub layer.\n\n"
                      "Determination of which hub goes with each point is based on a match between the Hub ID field on the hub points and the Spoke ID field on the spoke points.\n\n"
                      "If input layers are not point layers, a point on the surface of the geometries will be taken as the connecting location.\n\n"
                      "Optionally, geodesic lines can be created, which represent the shortest path on the surface of an ellipsoid. When "
                      "geodesic mode is used, it is possible to split the created lines at the antimeridian (±180 degrees longitude), which can improve "
                      "rendering of the lines. Additionally, the distance between vertices can be specified. A smaller distance results in a denser, more "
                      "accurate line." );
}

QString QgsJoinWithLinesAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates lines joining two point layers, based on a common attribute value." );
}

QgsJoinWithLinesAlgorithm *QgsJoinWithLinesAlgorithm::createInstance() const
{
  return new QgsJoinWithLinesAlgorithm();
}

QVariantMap QgsJoinWithLinesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( parameters.value( QStringLiteral( "SPOKES" ) ) == parameters.value( QStringLiteral( "HUBS" ) ) )
    throw QgsProcessingException( QObject::tr( "Same layer given for both hubs and spokes" ) );

  std::unique_ptr< QgsProcessingFeatureSource > hubSource( parameterAsSource( parameters, QStringLiteral( "HUBS" ), context ) );
  if ( !hubSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "HUBS" ) ) );

  std::unique_ptr< QgsProcessingFeatureSource > spokeSource( parameterAsSource( parameters, QStringLiteral( "SPOKES" ), context ) );
  if ( !spokeSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "SPOKES" ) ) );

  const QString fieldHubName = parameterAsString( parameters, QStringLiteral( "HUB_FIELD" ), context );
  const int fieldHubIndex = hubSource->fields().lookupField( fieldHubName );
  const QStringList hubFieldsToCopy = parameterAsFields( parameters, QStringLiteral( "HUB_FIELDS" ), context );

  const QString fieldSpokeName = parameterAsString( parameters, QStringLiteral( "SPOKE_FIELD" ), context );
  const int fieldSpokeIndex = spokeSource->fields().lookupField( fieldSpokeName );
  const QStringList spokeFieldsToCopy = parameterAsFields( parameters, QStringLiteral( "SPOKE_FIELDS" ), context );

  if ( fieldHubIndex < 0 || fieldSpokeIndex < 0 )
    throw QgsProcessingException( QObject::tr( "Invalid ID field" ) );

  const bool geodesic = parameterAsBoolean( parameters, QStringLiteral( "GEODESIC" ), context );
  const double geodesicDistance = parameterAsDouble( parameters, QStringLiteral( "GEODESIC_DISTANCE" ), context ) * 1000;
  const bool dynamicGeodesicDistance = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "GEODESIC_DISTANCE" ) );
  QgsExpressionContext expressionContext = createExpressionContext( parameters, context, hubSource.get() );
  QgsProperty geodesicDistanceProperty;
  if ( dynamicGeodesicDistance )
  {
    geodesicDistanceProperty = parameters.value( QStringLiteral( "GEODESIC_DISTANCE" ) ).value< QgsProperty >();
  }

  const bool splitAntimeridian = parameterAsBoolean( parameters, QStringLiteral( "ANTIMERIDIAN_SPLIT" ), context );
  QgsDistanceArea da;
  da.setSourceCrs( hubSource->sourceCrs(), context.transformContext() );
  da.setEllipsoid( context.ellipsoid() );

  QgsFields hubOutFields;
  QgsAttributeList hubFieldIndices;
  if ( hubFieldsToCopy.empty() )
  {
    hubOutFields = hubSource->fields();
    hubFieldIndices.reserve( hubOutFields.count() );
    for ( int i = 0; i < hubOutFields.count(); ++i )
    {
      hubFieldIndices << i;
    }
  }
  else
  {
    hubFieldIndices.reserve( hubOutFields.count() );
    for ( const QString &field : hubFieldsToCopy )
    {
      const int index = hubSource->fields().lookupField( field );
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
    spokeFieldIndices.reserve( spokeOutFields.count() );
    for ( int i = 0; i < spokeOutFields.count(); ++i )
    {
      spokeFieldIndices << i;
    }
  }
  else
  {
    for ( const QString &field : spokeFieldsToCopy )
    {
      const int index = spokeSource->fields().lookupField( field );
      if ( index >= 0 )
      {
        spokeFieldIndices << index;
        spokeOutFields.append( spokeSource->fields().at( index ) );
      }
    }
  }

  QgsAttributeList spokeFields2Fetch = spokeFieldIndices;
  spokeFields2Fetch << fieldSpokeIndex;


  const QgsFields fields = QgsProcessingUtils::combineFields( hubOutFields, spokeOutFields );

  QgsWkbTypes::Type outType = geodesic ? QgsWkbTypes::MultiLineString : QgsWkbTypes::LineString;
  bool hasZ = false;
  if ( !geodesic && ( QgsWkbTypes::hasZ( hubSource->wkbType() ) || QgsWkbTypes::hasZ( spokeSource->wkbType() ) ) )
  {
    outType = QgsWkbTypes::addZ( outType );
    hasZ = true;
  }
  bool hasM = false;
  if ( !geodesic && ( QgsWkbTypes::hasM( hubSource->wkbType() ) || QgsWkbTypes::hasM( spokeSource->wkbType() ) ) )
  {
    outType = QgsWkbTypes::addM( outType );
    hasM = true;
  }

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields,
                                          outType, hubSource->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

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

  QgsFeatureIterator hubFeatures = hubSource->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( hubFields2Fetch ), QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks );
  const double step = hubSource->featureCount() > 0 ? 100.0 / hubSource->featureCount() : 1;
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

    const QgsPoint hubPoint = getPointFromFeature( hubFeature );

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

    QgsFeatureIterator spokeFeatures = spokeSource->getFeatures( spokeRequest, QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks );
    QgsFeature spokeFeature;
    while ( spokeFeatures.nextFeature( spokeFeature ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }
      if ( !spokeFeature.hasGeometry() )
        continue;

      const QgsPoint spokePoint = getPointFromFeature( spokeFeature );
      QgsGeometry line;
      if ( !geodesic )
      {
        line = QgsGeometry( new QgsLineString( QVector< QgsPoint >() << hubPoint << spokePoint ) );
        if ( splitAntimeridian )
          line = da.splitGeometryAtAntimeridian( line );
      }
      else
      {
        double distance = geodesicDistance;
        if ( dynamicGeodesicDistance )
        {
          expressionContext.setFeature( hubFeature );
          distance = geodesicDistanceProperty.valueAsDouble( expressionContext, distance );
        }

        std::unique_ptr< QgsMultiLineString > ml = std::make_unique< QgsMultiLineString >();
        std::unique_ptr< QgsLineString > l = std::make_unique< QgsLineString >( QVector< QgsPoint >() << hubPoint );
        const QVector< QVector< QgsPointXY > > points = da.geodesicLine( QgsPointXY( hubPoint ), QgsPointXY( spokePoint ), distance, splitAntimeridian );
        QVector< QgsPointXY > points1 = points.at( 0 );
        points1.pop_front();
        if ( points.count() == 1 )
          points1.pop_back();

        const QgsLineString geodesicPoints( points1 );
        l->append( &geodesicPoints );
        if ( points.count() == 1 )
          l->addVertex( spokePoint );

        ml->addGeometry( l.release() );
        if ( points.count() > 1 )
        {
          QVector< QgsPointXY > points2 = points.at( 1 );
          points2.pop_back();
          l = std::make_unique< QgsLineString >( points2 );
          if ( hasZ )
            l->addZValue( std::numeric_limits<double>::quiet_NaN() );
          if ( hasM )
            l->addMValue( std::numeric_limits<double>::quiet_NaN() );

          l->addVertex( spokePoint );
          ml->addGeometry( l.release() );
        }
        line = QgsGeometry( std::move( ml ) );
      }

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
      if ( !sink->addFeature( outFeature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
    }
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond
