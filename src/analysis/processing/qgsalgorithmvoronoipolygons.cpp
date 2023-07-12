/***************************************************************************
                         qgsalgorithmvoronoipolygons.cpp
                         ---------------------
    begin                : July 2023
    copyright            : (C) 2023 by Alexander Bruy
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

#include "qgsalgorithmvoronoipolygons.h"
#include "qgsspatialindex.h"
#include "qgsmultipoint.h"
#include "qgsgeometryengine.h"

///@cond PRIVATE

QString QgsVoronoiPolygonsAlgorithm::name() const
{
  return QStringLiteral( "voronoipolygons" );
}

QString QgsVoronoiPolygonsAlgorithm::displayName() const
{
  return QObject::tr( "Voronoi polygons" );
}

QStringList QgsVoronoiPolygonsAlgorithm::tags() const
{
  return QObject::tr( "voronoi,polygons,tessellation,diagram" ).split( ',' );
}

QString QgsVoronoiPolygonsAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsVoronoiPolygonsAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsVoronoiPolygonsAlgorithm::shortHelpString() const
{
  return QObject::tr( "Generates a polygon layer containing the Voronoi diagram corresponding to input points." );
}

QgsVoronoiPolygonsAlgorithm *QgsVoronoiPolygonsAlgorithm::createInstance() const
{
  return new QgsVoronoiPolygonsAlgorithm();
}

void QgsVoronoiPolygonsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), QList< int >() << QgsProcessing::TypeVectorPoint ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "BUFFER" ), QObject::tr( "Buffer region (% of extent)" ), QgsProcessingParameterNumber::Double, 0, false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "TOLERANCE" ), QObject::tr( "Tolerance" ), QgsProcessingParameterNumber::Double, 0, true, 0 ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "COPY_ATTRIBUTES" ), QObject::tr( "Copy attributes from input features" ), true ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Voronoi polygons" ), QgsProcessing::TypeVectorPolygon ) );
}

QVariantMap QgsVoronoiPolygonsAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  if ( source->featureCount() < 3 )
    throw QgsProcessingException( QObject::tr( "Input layer should contain at least 3 points." ) );

  const double buffer = parameterAsDouble( parameters, QStringLiteral( "BUFFER" ), context );
  const double tolerance = parameterAsDouble( parameters, QStringLiteral( "TOLERANCE" ), context );
  const bool copyAttributes = parameterAsBool( parameters, QStringLiteral( "COPY_ATTRIBUTES" ), context );

  QgsFields fields;
  if ( copyAttributes )
  {
    fields = source->fields();
  }
  else
  {
    fields.append( QgsField( QStringLiteral( "id" ), QVariant::LongLong ) );
  }

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, Qgis::WkbType::Polygon, source->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QgsFeatureRequest request;
  if ( !copyAttributes )
  {
    request.setSubsetOfAttributes( QList< int >() );
  }
  QgsFeatureIterator it = source->getFeatures( request, QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks );

  QgsGeometry allPoints;
  QHash< QgsFeatureId, QgsAttributes > attributeCache;

  long i = 0;
  double step = source->featureCount() > 0 ? 50.0 / source->featureCount() : 1;

  const QgsSpatialIndex index( it, [&]( const QgsFeature & f )->bool
  {
    i++;
    if ( feedback->isCanceled() )
      return false;

    feedback->setProgress( i * step );

    if ( !f.hasGeometry() )
      return true;

    const QgsAbstractGeometry *geom = f.geometry().constGet();
    if ( QgsWkbTypes::isMultiType( geom->wkbType() ) )
    {
      const QgsMultiPoint mp( *qgsgeometry_cast< const QgsMultiPoint * >( geom ) );
      for ( auto pit = mp.const_parts_begin(); pit != mp.const_parts_end(); ++pit )
      {
        allPoints.addPart( qgsgeometry_cast< QgsPoint * >( *pit )->clone(), Qgis::GeometryType::Point );
      }
    }
    else
    {
      allPoints.addPart( qgsgeometry_cast< QgsPoint * >( geom )->clone(), Qgis::GeometryType::Point );
    }

    attributeCache.insert( f.id(), f.attributes() );

    return true;
  }, QgsSpatialIndex::FlagStoreFeatureGeometries );

  QgsRectangle extent = source->sourceExtent();
  double delta = extent.width() * buffer / 100.0;
  extent.setXMinimum( extent.xMinimum() - delta );
  extent.setXMaximum( extent.xMaximum() + delta );
  delta = extent.height() * buffer / 100.0;
  extent.setYMinimum( extent.yMinimum() - delta );
  extent.setYMaximum( extent.yMaximum() + delta );
  QgsGeometry clippingGeom = QgsGeometry::fromRect( extent );

  QgsGeometry voronoiDiagram = allPoints.voronoiDiagram( clippingGeom, tolerance );

  if ( !voronoiDiagram.isEmpty() )
  {
    std::unique_ptr< QgsGeometryEngine > engine;
    std::unique_ptr< QgsGeometryEngine > extentEngine( QgsGeometry::createGeometryEngine( clippingGeom.constGet() ) );
    QVector< QgsGeometry > collection = voronoiDiagram.asGeometryCollection();
    for ( int i = 0; i < collection.length(); i++ )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }
      QgsFeature f;
      f.setFields( fields );
      f.setGeometry( QgsGeometry( extentEngine->intersection( collection[i].constGet() ) ) );
      if ( copyAttributes )
      {
        const QList< QgsFeatureId > intersected = index.intersects( collection[i].boundingBox() );
        engine.reset( QgsGeometry::createGeometryEngine( collection[i].constGet() ) );
        engine->prepareGeometry();
        for ( const QgsFeatureId id : intersected )
        {
          if ( engine->intersects( index.geometry( id ).constGet() ) )
          {
            f.setAttributes( attributeCache.value( id ) );
            break;
          }
        }
      }
      else
      {
        f.setAttributes( QgsAttributes() << i );
      }
      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
      feedback->setProgress( 50 + i * step );
    }
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond
