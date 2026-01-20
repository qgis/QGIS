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
  return u"filterbygeometry"_s;
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
  return QObject::tr( "Vector selection" );
}

QString QgsFilterByGeometryAlgorithm::groupId() const
{
  return u"vectorselection"_s;
}

void QgsFilterByGeometryAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );

  addParameter( new QgsProcessingParameterFeatureSink( u"POINTS"_s, QObject::tr( "Point features" ), Qgis::ProcessingSourceType::VectorPoint, QVariant(), true, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( u"LINES"_s, QObject::tr( "Line features" ), Qgis::ProcessingSourceType::VectorLine, QVariant(), true, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( u"POLYGONS"_s, QObject::tr( "Polygon features" ), Qgis::ProcessingSourceType::VectorPolygon, QVariant(), true, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( u"NO_GEOMETRY"_s, QObject::tr( "Features with no geometry" ), Qgis::ProcessingSourceType::Vector, QVariant(), true, true ) );

  addOutput( new QgsProcessingOutputNumber( u"POINT_COUNT"_s, QObject::tr( "Total count of point features" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"LINE_COUNT"_s, QObject::tr( "Total count of line features" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"POLYGON_COUNT"_s, QObject::tr( "Total count of polygon features" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"NO_GEOMETRY_COUNT"_s, QObject::tr( "Total count of features without geometry" ) ) );
}

QString QgsFilterByGeometryAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm filters features by their geometry type. Incoming features will be directed to different "
                      "outputs based on whether they have a point, line or polygon geometry." );
}

QString QgsFilterByGeometryAlgorithm::shortDescription() const
{
  return QObject::tr( "Filters features by geometry type." );
}

QgsFilterByGeometryAlgorithm *QgsFilterByGeometryAlgorithm::createInstance() const
{
  return new QgsFilterByGeometryAlgorithm();
}

QVariantMap QgsFilterByGeometryAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const bool hasM = QgsWkbTypes::hasM( source->wkbType() );
  const bool hasZ = QgsWkbTypes::hasZ( source->wkbType() );

  Qgis::WkbType pointType = Qgis::WkbType::Point;
  Qgis::WkbType lineType = Qgis::WkbType::LineString;
  Qgis::WkbType polygonType = Qgis::WkbType::Polygon;
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
  std::unique_ptr<QgsFeatureSink> pointSink( parameterAsSink( parameters, u"POINTS"_s, context, pointSinkId, source->fields(), pointType, source->sourceCrs() ) );
  if ( parameters.value( u"POINTS"_s, QVariant() ).isValid() && !pointSink )
    throw QgsProcessingException( invalidSinkError( parameters, u"POINTS"_s ) );

  QString lineSinkId;
  std::unique_ptr<QgsFeatureSink> lineSink( parameterAsSink( parameters, u"LINES"_s, context, lineSinkId, source->fields(), lineType, source->sourceCrs() ) );
  if ( parameters.value( u"LINES"_s, QVariant() ).isValid() && !lineSink )
    throw QgsProcessingException( invalidSinkError( parameters, u"LINES"_s ) );

  QString polygonSinkId;
  std::unique_ptr<QgsFeatureSink> polygonSink( parameterAsSink( parameters, u"POLYGONS"_s, context, polygonSinkId, source->fields(), polygonType, source->sourceCrs() ) );
  if ( parameters.value( u"POLYGONS"_s, QVariant() ).isValid() && !polygonSink )
    throw QgsProcessingException( invalidSinkError( parameters, u"POLYGONS"_s ) );

  QString noGeomSinkId;
  std::unique_ptr<QgsFeatureSink> noGeomSink( parameterAsSink( parameters, u"NO_GEOMETRY"_s, context, noGeomSinkId, source->fields(), Qgis::WkbType::NoGeometry ) );
  if ( parameters.value( u"NO_GEOMETRY"_s, QVariant() ).isValid() && !noGeomSink )
    throw QgsProcessingException( invalidSinkError( parameters, u"NO_GEOMETRY"_s ) );

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
        case Qgis::GeometryType::Point:
          if ( pointSink )
          {
            if ( !pointSink->addFeature( f, QgsFeatureSink::FastInsert ) )
              throw QgsProcessingException( writeFeatureError( pointSink.get(), parameters, u"POINTS"_s ) );
          }
          pointCount++;
          break;
        case Qgis::GeometryType::Line:
          if ( lineSink )
          {
            if ( !lineSink->addFeature( f, QgsFeatureSink::FastInsert ) )
              throw QgsProcessingException( writeFeatureError( lineSink.get(), parameters, u"LINES"_s ) );
          }
          lineCount++;
          break;
        case Qgis::GeometryType::Polygon:
          if ( polygonSink )
          {
            if ( !polygonSink->addFeature( f, QgsFeatureSink::FastInsert ) )
              throw QgsProcessingException( writeFeatureError( polygonSink.get(), parameters, u"POLYGONS"_s ) );
          }
          polygonCount++;
          break;
        case Qgis::GeometryType::Null:
        case Qgis::GeometryType::Unknown:
          break;
      }
    }
    else
    {
      if ( noGeomSink )
      {
        if ( !noGeomSink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( noGeomSink.get(), parameters, u"NO_GEOMETRY"_s ) );
      }
      nullCount++;
    }

    feedback->setProgress( current * step );
    current++;
  }

  QVariantMap outputs;

  if ( pointSink )
  {
    pointSink->finalize();
    outputs.insert( u"POINTS"_s, pointSinkId );
  }
  if ( lineSink )
  {
    lineSink->finalize();
    outputs.insert( u"LINES"_s, lineSinkId );
  }
  if ( polygonSink )
  {
    polygonSink->finalize();
    outputs.insert( u"POLYGONS"_s, polygonSinkId );
  }
  if ( noGeomSink )
  {
    noGeomSink->finalize();
    outputs.insert( u"NO_GEOMETRY"_s, noGeomSinkId );
  }

  outputs.insert( u"POINT_COUNT"_s, pointCount );
  outputs.insert( u"LINE_COUNT"_s, lineCount );
  outputs.insert( u"POLYGON_COUNT"_s, polygonCount );
  outputs.insert( u"NO_GEOMETRY_COUNT"_s, nullCount );

  return outputs;
}


//
// QgsFilterByLayerTypeAlgorithm
//

QString QgsFilterByLayerTypeAlgorithm::name() const
{
  return u"filterlayersbytype"_s;
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
  return u"modelertools"_s;
}

Qgis::ProcessingAlgorithmFlags QgsFilterByLayerTypeAlgorithm::flags() const
{
  Qgis::ProcessingAlgorithmFlags f = QgsProcessingAlgorithm::flags();
  f |= Qgis::ProcessingAlgorithmFlag::HideFromToolbox | Qgis::ProcessingAlgorithmFlag::PruneModelBranchesBasedOnAlgorithmResults;
  return f;
}

void QgsFilterByLayerTypeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );

  addParameter( new QgsProcessingParameterVectorDestination( u"VECTOR"_s, QObject::tr( "Vector features" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true, false ) );

  addParameter( new QgsProcessingParameterRasterDestination( u"RASTER"_s, QObject::tr( "Raster layer" ), QVariant(), true, false ) );
}

QString QgsFilterByLayerTypeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm filters layer by their type. Incoming layers will be directed to different "
                      "outputs based on whether they are a vector or raster layer." );
}

QString QgsFilterByLayerTypeAlgorithm::shortDescription() const
{
  return QObject::tr( "Filters layers by type." );
}

QgsFilterByLayerTypeAlgorithm *QgsFilterByLayerTypeAlgorithm::createInstance() const
{
  return new QgsFilterByLayerTypeAlgorithm();
}

QVariantMap QgsFilterByLayerTypeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  const QgsMapLayer *layer = parameterAsLayer( parameters, u"INPUT"_s, context );
  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Could not load input layer" ) );

  QVariantMap outputs;

  switch ( layer->type() )
  {
    case Qgis::LayerType::Vector:
      outputs.insert( u"VECTOR"_s, parameters.value( u"INPUT"_s ) );
      break;

    case Qgis::LayerType::Raster:
      outputs.insert( u"RASTER"_s, parameters.value( u"INPUT"_s ) );
      break;

    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::Mesh:
    case Qgis::LayerType::VectorTile:
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::PointCloud:
    case Qgis::LayerType::Group:
    case Qgis::LayerType::TiledScene:
      break;
  }

  return outputs;
}

///@endcond
