/***************************************************************************
                         qgsalgorithmfilterbygeometry.cpp
                         ---------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#include "qgsalgorithmfilterbygeometry.h"

///@cond PRIVATE

QString QgsFilterByGeometryAlgorithm::name() const
{
  return QStringLiteral( "filterbygeometry" );
}

QString QgsFilterByGeometryAlgorithm::displayName() const
{
  return QObject::tr( "Filter by geometry type" );
}

QStringList QgsFilterByGeometryAlgorithm::tags() const
{
  return QObject::tr( "extract,filter,geometry,linestring,point,polygon" ).split( ',' );
}

QString QgsFilterByGeometryAlgorithm::group() const
{
  return QObject::tr( "Modeler tools" );
}

QString QgsFilterByGeometryAlgorithm::groupId() const
{
  return QStringLiteral( "modelertools" );
}

QgsProcessingAlgorithm::Flags QgsFilterByGeometryAlgorithm::flags() const
{
  Flags f = QgsProcessingAlgorithm::flags();
  f |= QgsProcessingAlgorithm::FlagHideFromToolbox;
  return f;
}

void QgsFilterByGeometryAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ),
                QList< int >() << QgsProcessing::TypeVector ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "POINTS" ),  QObject::tr( "Point features" ),
                QgsProcessing::TypeVectorPoint, QVariant(), true, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "LINES" ),  QObject::tr( "Line features" ),
                QgsProcessing::TypeVectorLine, QVariant(), true, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "POLYGONS" ),  QObject::tr( "Polygon features" ),
                QgsProcessing::TypeVectorPolygon, QVariant(), true, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "NO_GEOMETRY" ),  QObject::tr( "Features with no geometry" ),
                QgsProcessing::TypeVector, QVariant(), true, true ) );

  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "POINT_COUNT" ), QObject::tr( "Total count of point features" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "LINE_COUNT" ), QObject::tr( "Total count of line features" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "POLYGON_COUNT" ), QObject::tr( "Total count of polygon features" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "NO_GEOMETRY_COUNT" ), QObject::tr( "Total count of features without geometry" ) ) );
}

QString QgsFilterByGeometryAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm filters features by their geometry type. Incoming features will be directed to different "
                      "outputs based on whether they have a point, line or polygon geometry." );
}

QString QgsFilterByGeometryAlgorithm::shortDescription() const
{
  return QObject::tr( "Filters features by geometry type" );
}

QgsFilterByGeometryAlgorithm *QgsFilterByGeometryAlgorithm::createInstance() const
{
  return new QgsFilterByGeometryAlgorithm();
}

QVariantMap QgsFilterByGeometryAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const bool hasM = QgsWkbTypes::hasM( source->wkbType() );
  const bool hasZ = QgsWkbTypes::hasZ( source->wkbType() );

  QgsWkbTypes::Type pointType = QgsWkbTypes::Point;
  QgsWkbTypes::Type lineType = QgsWkbTypes::LineString;
  QgsWkbTypes::Type polygonType = QgsWkbTypes::Polygon;
  if ( hasM )
  {
    pointType = QgsWkbTypes::addM( pointType );
    lineType = QgsWkbTypes::addM( lineType );
    polygonType = QgsWkbTypes::addM( polygonType );
  }
  if ( hasZ )
  {
    pointType = QgsWkbTypes::addZ( pointType );
    lineType = QgsWkbTypes::addZ( lineType );
    polygonType = QgsWkbTypes::addZ( polygonType );
  }

  QString pointSinkId;
  std::unique_ptr< QgsFeatureSink > pointSink( parameterAsSink( parameters, QStringLiteral( "POINTS" ), context, pointSinkId, source->fields(),
      pointType, source->sourceCrs() ) );
  if ( parameters.value( QStringLiteral( "POINTS" ), QVariant() ).isValid() && !pointSink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "POINTS" ) ) );

  QString lineSinkId;
  std::unique_ptr< QgsFeatureSink > lineSink( parameterAsSink( parameters, QStringLiteral( "LINES" ), context, lineSinkId, source->fields(),
      lineType, source->sourceCrs() ) );
  if ( parameters.value( QStringLiteral( "LINES" ), QVariant() ).isValid() && !lineSink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "LINES" ) ) );

  QString polygonSinkId;
  std::unique_ptr< QgsFeatureSink > polygonSink( parameterAsSink( parameters, QStringLiteral( "POLYGONS" ), context, polygonSinkId, source->fields(),
      polygonType, source->sourceCrs() ) );
  if ( parameters.value( QStringLiteral( "POLYGONS" ), QVariant() ).isValid() && !polygonSink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "POLYGONS" ) ) );

  QString noGeomSinkId;
  std::unique_ptr< QgsFeatureSink > noGeomSink( parameterAsSink( parameters, QStringLiteral( "NO_GEOMETRY" ), context, noGeomSinkId, source->fields(),
      QgsWkbTypes::NoGeometry ) );
  if ( parameters.value( QStringLiteral( "NO_GEOMETRY" ), QVariant() ).isValid() && !noGeomSink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "NO_GEOMETRY" ) ) );

  const long count = source->featureCount();
  long long pointCount = 0;
  long long lineCount = 0;
  long long polygonCount = 0;
  long long nullCount = 0;

  const double step = count > 0 ? 100.0 / count : 1;
  int current = 0;

  QgsFeatureIterator it = source->getFeatures();
  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    if ( f.hasGeometry() )
    {
      switch ( f.geometry().type() )
      {
        case QgsWkbTypes::PointGeometry:
          if ( pointSink )
          {
            if ( !pointSink->addFeature( f, QgsFeatureSink::FastInsert ) )
              throw QgsProcessingException( writeFeatureError( pointSink.get(), parameters, QStringLiteral( "POINTS" ) ) );
          }
          pointCount++;
          break;
        case QgsWkbTypes::LineGeometry:
          if ( lineSink )
          {
            if ( !lineSink->addFeature( f, QgsFeatureSink::FastInsert ) )
              throw QgsProcessingException( writeFeatureError( lineSink.get(), parameters, QStringLiteral( "LINES" ) ) );
          }
          lineCount++;
          break;
        case QgsWkbTypes::PolygonGeometry:
          if ( polygonSink )
          {
            if ( !polygonSink->addFeature( f, QgsFeatureSink::FastInsert ) )
              throw QgsProcessingException( writeFeatureError( polygonSink.get(), parameters, QStringLiteral( "POLYGONS" ) ) );
          }
          polygonCount++;
          break;
        case QgsWkbTypes::NullGeometry:
        case QgsWkbTypes::UnknownGeometry:
          break;
      }
    }
    else
    {
      if ( noGeomSink )
      {
        if ( !noGeomSink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( noGeomSink.get(), parameters, QStringLiteral( "NO_GEOMETRY" ) ) );
      }
      nullCount++;
    }

    feedback->setProgress( current * step );
    current++;
  }

  QVariantMap outputs;

  if ( pointSink )
    outputs.insert( QStringLiteral( "POINTS" ), pointSinkId );
  if ( lineSink )
    outputs.insert( QStringLiteral( "LINES" ), lineSinkId );
  if ( polygonSink )
    outputs.insert( QStringLiteral( "POLYGONS" ), polygonSinkId );
  if ( noGeomSink )
    outputs.insert( QStringLiteral( "NO_GEOMETRY" ), noGeomSinkId );

  outputs.insert( QStringLiteral( "POINT_COUNT" ), pointCount );
  outputs.insert( QStringLiteral( "LINE_COUNT" ), lineCount );
  outputs.insert( QStringLiteral( "POLYGON_COUNT" ), polygonCount );
  outputs.insert( QStringLiteral( "NO_GEOMETRY_COUNT" ), nullCount );

  return outputs;
}



//
// QgsFilterByLayerTypeAlgorithm
//

QString QgsFilterByLayerTypeAlgorithm::name() const
{
  return QStringLiteral( "filterlayersbytype" );
}

QString QgsFilterByLayerTypeAlgorithm::displayName() const
{
  return QObject::tr( "Filter layers by type" );
}

QStringList QgsFilterByLayerTypeAlgorithm::tags() const
{
  return QObject::tr( "filter,vector,raster,select" ).split( ',' );
}

QString QgsFilterByLayerTypeAlgorithm::group() const
{
  return QObject::tr( "Modeler tools" );
}

QString QgsFilterByLayerTypeAlgorithm::groupId() const
{
  return QStringLiteral( "modelertools" );
}

QgsProcessingAlgorithm::Flags QgsFilterByLayerTypeAlgorithm::flags() const
{
  Flags f = QgsProcessingAlgorithm::flags();
  f |= FlagHideFromToolbox | FlagPruneModelBranchesBasedOnAlgorithmResults;
  return f;
}

void QgsFilterByLayerTypeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );

  addParameter( new QgsProcessingParameterVectorDestination( QStringLiteral( "VECTOR" ),  QObject::tr( "Vector features" ),
                QgsProcessing::TypeVectorAnyGeometry, QVariant(), true, false ) );

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "RASTER" ),  QObject::tr( "Raster layer" ), QVariant(), true, false ) );
}

QString QgsFilterByLayerTypeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm filters layer by their type. Incoming layers will be directed to different "
                      "outputs based on whether they are a vector or raster layer." );
}

QString QgsFilterByLayerTypeAlgorithm::shortDescription() const
{
  return QObject::tr( "Filters layers by type" );
}

QgsFilterByLayerTypeAlgorithm *QgsFilterByLayerTypeAlgorithm::createInstance() const
{
  return new QgsFilterByLayerTypeAlgorithm();
}

QVariantMap QgsFilterByLayerTypeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  const QgsMapLayer *layer = parameterAsLayer( parameters, QStringLiteral( "INPUT" ), context );
  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Could not load input layer" ) );

  QVariantMap outputs;

  switch ( layer->type() )
  {
    case QgsMapLayerType::VectorLayer:
      outputs.insert( QStringLiteral( "VECTOR" ), parameters.value( QStringLiteral( "INPUT" ) ) );
      break;

    case QgsMapLayerType::RasterLayer:
      outputs.insert( QStringLiteral( "RASTER" ), parameters.value( QStringLiteral( "INPUT" ) ) );
      break;

    case QgsMapLayerType::PluginLayer:
    case QgsMapLayerType::MeshLayer:
    case QgsMapLayerType::VectorTileLayer:
    case QgsMapLayerType::AnnotationLayer:
    case QgsMapLayerType::PointCloudLayer:
    case QgsMapLayerType::GroupLayer:
      break;
  }

  return outputs;
}

///@endcond

