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

#include "qgsdistancearea.h"
#include "qgslinestring.h"
#include "qgsmultilinestring.h"

///@cond PRIVATE

QString QgsJoinWithLinesAlgorithm::name() const
{
  return u"hublines"_s;
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
  return u"vectoranalysis"_s;
}

void QgsJoinWithLinesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"HUBS"_s, QObject::tr( "Hub layer" ) ) );
  addParameter( new QgsProcessingParameterField( u"HUB_FIELD"_s, QObject::tr( "Hub ID field" ), QVariant(), u"HUBS"_s ) );

  addParameter( new QgsProcessingParameterField( u"HUB_FIELDS"_s, QObject::tr( "Hub layer fields to copy (leave empty to copy all fields)" ), QVariant(), u"HUBS"_s, Qgis::ProcessingFieldParameterDataType::Any, true, true ) );

  addParameter( new QgsProcessingParameterFeatureSource( u"SPOKES"_s, QObject::tr( "Spoke layer" ) ) );
  addParameter( new QgsProcessingParameterField( u"SPOKE_FIELD"_s, QObject::tr( "Spoke ID field" ), QVariant(), u"SPOKES"_s ) );

  addParameter( new QgsProcessingParameterField( u"SPOKE_FIELDS"_s, QObject::tr( "Spoke layer fields to copy (leave empty to copy all fields)" ), QVariant(), u"SPOKES"_s, Qgis::ProcessingFieldParameterDataType::Any, true, true ) );

  addParameter( new QgsProcessingParameterBoolean( u"GEODESIC"_s, QObject::tr( "Create geodesic lines" ), false ) );

  auto distanceParam = std::make_unique<QgsProcessingParameterDistance>( u"GEODESIC_DISTANCE"_s, QObject::tr( "Distance between vertices (geodesic lines only)" ), 1000 );
  distanceParam->setFlags( distanceParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  distanceParam->setDefaultUnit( Qgis::DistanceUnit::Kilometers );
  distanceParam->setIsDynamic( true );
  distanceParam->setDynamicPropertyDefinition( QgsPropertyDefinition( u"Geodesic Distance"_s, QObject::tr( "Distance between vertices" ), QgsPropertyDefinition::DoublePositive ) );
  distanceParam->setDynamicLayerParameterName( u"HUBS"_s );
  addParameter( distanceParam.release() );

  auto breakParam = std::make_unique<QgsProcessingParameterBoolean>( u"ANTIMERIDIAN_SPLIT"_s, QObject::tr( "Split lines at antimeridian (±180 degrees longitude)" ), false );
  breakParam->setFlags( breakParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( breakParam.release() );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Hub lines" ), Qgis::ProcessingSourceType::VectorLine ) );
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

Qgis::ProcessingAlgorithmDocumentationFlags QgsJoinWithLinesAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RegeneratesPrimaryKey;
}

QgsJoinWithLinesAlgorithm *QgsJoinWithLinesAlgorithm::createInstance() const
{
  return new QgsJoinWithLinesAlgorithm();
}

QVariantMap QgsJoinWithLinesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( parameters.value( u"SPOKES"_s ) == parameters.value( u"HUBS"_s ) )
    throw QgsProcessingException( QObject::tr( "Same layer given for both hubs and spokes" ) );

  std::unique_ptr<QgsProcessingFeatureSource> hubSource( parameterAsSource( parameters, u"HUBS"_s, context ) );
  if ( !hubSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"HUBS"_s ) );

  std::unique_ptr<QgsProcessingFeatureSource> spokeSource( parameterAsSource( parameters, u"SPOKES"_s, context ) );
  if ( !spokeSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"SPOKES"_s ) );

  const QString fieldHubName = parameterAsString( parameters, u"HUB_FIELD"_s, context );
  const int fieldHubIndex = hubSource->fields().lookupField( fieldHubName );
  const QStringList hubFieldsToCopy = parameterAsStrings( parameters, u"HUB_FIELDS"_s, context );

  const QString fieldSpokeName = parameterAsString( parameters, u"SPOKE_FIELD"_s, context );
  const int fieldSpokeIndex = spokeSource->fields().lookupField( fieldSpokeName );
  const QStringList spokeFieldsToCopy = parameterAsStrings( parameters, u"SPOKE_FIELDS"_s, context );

  if ( fieldHubIndex < 0 || fieldSpokeIndex < 0 )
    throw QgsProcessingException( QObject::tr( "Invalid ID field" ) );

  const bool geodesic = parameterAsBoolean( parameters, u"GEODESIC"_s, context );
  const double geodesicDistance = parameterAsDouble( parameters, u"GEODESIC_DISTANCE"_s, context ) * 1000;
  const bool dynamicGeodesicDistance = QgsProcessingParameters::isDynamic( parameters, u"GEODESIC_DISTANCE"_s );
  QgsExpressionContext expressionContext = createExpressionContext( parameters, context, hubSource.get() );
  QgsProperty geodesicDistanceProperty;
  if ( dynamicGeodesicDistance )
  {
    geodesicDistanceProperty = parameters.value( u"GEODESIC_DISTANCE"_s ).value<QgsProperty>();
  }

  const bool splitAntimeridian = parameterAsBoolean( parameters, u"ANTIMERIDIAN_SPLIT"_s, context );
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

  Qgis::WkbType outType = geodesic ? Qgis::WkbType::MultiLineString : Qgis::WkbType::LineString;
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
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, outType, hubSource->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  auto getPointFromFeature = [hasZ, hasM]( const QgsFeature &feature ) -> QgsPoint {
    QgsPoint p;
    if ( feature.geometry().type() == Qgis::GeometryType::Point && !feature.geometry().isMultipart() )
      p = *static_cast<const QgsPoint *>( feature.geometry().constGet() );
    else
      p = *static_cast<const QgsPoint *>( feature.geometry().pointOnSurface().constGet() );
    if ( hasZ && !p.is3D() )
      p.addZValue( 0 );
    if ( hasM && !p.isMeasure() )
      p.addMValue( 0 );
    return p;
  };

  QgsFeatureIterator hubFeatures = hubSource->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( hubFields2Fetch ), Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
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
    const int attributeCount = hubFeature.attributeCount();
    for ( int j = 0; j < attributeCount; ++j )
    {
      if ( !hubFieldIndices.contains( j ) )
        continue;
      hubAttributes << hubFeature.attribute( j );
    }

    QgsFeatureRequest spokeRequest = QgsFeatureRequest().setDestinationCrs( hubSource->sourceCrs(), context.transformContext() );
    spokeRequest.setSubsetOfAttributes( spokeFields2Fetch );
    spokeRequest.setFilterExpression( QgsExpression::createFieldEqualityExpression( fieldSpokeName, hubFeature.attribute( fieldHubIndex ) ) );

    QgsFeatureIterator spokeFeatures = spokeSource->getFeatures( spokeRequest, Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
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
        line = QgsGeometry( new QgsLineString( QVector<QgsPoint>() << hubPoint << spokePoint ) );
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

        auto ml = std::make_unique<QgsMultiLineString>();
        auto l = std::make_unique<QgsLineString>( QVector<QgsPoint>() << hubPoint );
        const QVector<QVector<QgsPointXY>> points = da.geodesicLine( QgsPointXY( hubPoint ), QgsPointXY( spokePoint ), distance, splitAntimeridian );
        QVector<QgsPointXY> points1 = points.at( 0 );
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
          QVector<QgsPointXY> points2 = points.at( 1 );
          points2.pop_back();
          l = std::make_unique<QgsLineString>( points2 );
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
      const int attributeCount = spokeFeature.attributeCount();
      for ( int j = 0; j < attributeCount; ++j )
      {
        if ( !spokeFieldIndices.contains( j ) )
          continue;
        spokeAttributes << spokeFeature.attribute( j );
      }

      outAttributes.append( spokeAttributes );
      outFeature.setAttributes( outAttributes );
      outFeature.setGeometry( line );
      if ( !sink->addFeature( outFeature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
    }
  }
  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

///@endcond
