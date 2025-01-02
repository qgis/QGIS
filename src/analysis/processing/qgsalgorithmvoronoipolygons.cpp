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
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "BUFFER" ), QObject::tr( "Buffer region (% of extent)" ), Qgis::ProcessingNumberParameterType::Double, 0, false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "TOLERANCE" ), QObject::tr( "Tolerance" ), Qgis::ProcessingNumberParameterType::Double, 0, true, 0 ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "COPY_ATTRIBUTES" ), QObject::tr( "Copy attributes from input features" ), true ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Voronoi polygons" ), Qgis::ProcessingSourceType::VectorPolygon ) );
}

bool QgsVoronoiPolygonsAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  mSource.reset( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !mSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  if ( mSource->featureCount() < 3 )
    throw QgsProcessingException( QObject::tr( "Input layer should contain at least 3 points." ) );

  mBuffer = parameterAsDouble( parameters, QStringLiteral( "BUFFER" ), context );
  mTolerance = parameterAsDouble( parameters, QStringLiteral( "TOLERANCE" ), context );
  mCopyAttributes = parameterAsBool( parameters, QStringLiteral( "COPY_ATTRIBUTES" ), context );

  return true;
}

QVariantMap QgsVoronoiPolygonsAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QString dest;
  if ( mCopyAttributes )
  {
    dest = voronoiWithAttributes( parameters, context, feedback );
  }
  else
  {
    dest = voronoiWithoutAttributes( parameters, context, feedback );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

QString QgsVoronoiPolygonsAlgorithm::voronoiWithAttributes( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QgsFields fields = mSource->fields();

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, Qgis::WkbType::Polygon, mSource->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QgsFeatureRequest request;
  QgsFeatureIterator it = mSource->getFeatures( request, Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );

  QgsGeometry allPoints;
  QHash<QgsFeatureId, QgsAttributes> attributeCache;

  long long i = 0;
  const double step = mSource->featureCount() > 0 ? 50.0 / mSource->featureCount() : 1;

  const QgsSpatialIndex index( it, [&]( const QgsFeature &f ) -> bool {
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
        allPoints.addPartV2( qgsgeometry_cast< QgsPoint * >( *pit )->clone(), Qgis::WkbType::Point );
      }
    }
    else
    {
      allPoints.addPartV2( qgsgeometry_cast< QgsPoint * >( geom )->clone(), Qgis::WkbType::Point );
    }

    attributeCache.insert( f.id(), f.attributes() );

    return true; }, QgsSpatialIndex::FlagStoreFeatureGeometries );

  QgsRectangle extent = mSource->sourceExtent();
  double delta = extent.width() * mBuffer / 100.0;
  extent.setXMinimum( extent.xMinimum() - delta );
  extent.setXMaximum( extent.xMaximum() + delta );
  delta = extent.height() * mBuffer / 100.0;
  extent.setYMinimum( extent.yMinimum() - delta );
  extent.setYMaximum( extent.yMaximum() + delta );
  const QgsGeometry clippingGeom = QgsGeometry::fromRect( extent );

  const QgsGeometry voronoiDiagram = allPoints.voronoiDiagram( clippingGeom, mTolerance );

  if ( !voronoiDiagram.isEmpty() )
  {
    std::unique_ptr<QgsGeometryEngine> engine;
    std::unique_ptr<QgsGeometryEngine> extentEngine( QgsGeometry::createGeometryEngine( clippingGeom.constGet() ) );
    const QVector<QgsGeometry> collection = voronoiDiagram.asGeometryCollection();
    for ( int i = 0; i < collection.length(); i++ )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }
      QgsFeature f;
      f.setFields( fields );
      f.setGeometry( QgsGeometry( extentEngine->intersection( collection[i].constGet() ) ) );
      const QList<QgsFeatureId> intersected = index.intersects( collection[i].boundingBox() );
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
      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
      feedback->setProgress( 50 + i * step );
    }
  }

  sink->finalize();

  return dest;
}

QString QgsVoronoiPolygonsAlgorithm::voronoiWithoutAttributes( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "id" ), QMetaType::Type::LongLong ) );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, Qgis::WkbType::Polygon, mSource->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  std::unique_ptr<QgsMultiPoint> points = std::make_unique<QgsMultiPoint>();

  long long i = 0;
  const double step = mSource->featureCount() > 0 ? 50.0 / mSource->featureCount() : 1;
  QgsFeatureIterator it = mSource->getFeatures( QgsFeatureRequest().setNoAttributes(), Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    i++;

    if ( feedback->isCanceled() )
      break;

    feedback->setProgress( i * step );

    if ( !f.hasGeometry() )
      continue;

    const QgsAbstractGeometry *geom = f.geometry().constGet();
    if ( QgsWkbTypes::isMultiType( geom->wkbType() ) )
    {
      const QgsMultiPoint mp( *qgsgeometry_cast<const QgsMultiPoint *>( geom ) );
      for ( auto pit = mp.const_parts_begin(); pit != mp.const_parts_end(); ++pit )
      {
        points->addGeometry( qgsgeometry_cast<QgsPoint *>( *pit )->clone() );
      }
    }
    else
    {
      points->addGeometry( qgsgeometry_cast<QgsPoint *>( geom )->clone() );
    }
  }

  QgsRectangle extent = mSource->sourceExtent();
  double delta = extent.width() * mBuffer / 100.0;
  extent.setXMinimum( extent.xMinimum() - delta );
  extent.setXMaximum( extent.xMaximum() + delta );
  delta = extent.height() * mBuffer / 100.0;
  extent.setYMinimum( extent.yMinimum() - delta );
  extent.setYMaximum( extent.yMaximum() + delta );
  const QgsGeometry clippingGeom = QgsGeometry::fromRect( extent );

  QgsGeometry allPoints = QgsGeometry( std::move( points ) );
  const QgsGeometry voronoiDiagram = allPoints.voronoiDiagram( clippingGeom, mTolerance );

  if ( !voronoiDiagram.isEmpty() )
  {
    std::unique_ptr<QgsGeometryEngine> engine;
    std::unique_ptr<QgsGeometryEngine> extentEngine( QgsGeometry::createGeometryEngine( clippingGeom.constGet() ) );
    const QVector<QgsGeometry> collection = voronoiDiagram.asGeometryCollection();
    for ( int i = 0; i < collection.length(); i++ )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }
      QgsFeature f;
      f.setFields( fields );
      f.setGeometry( QgsGeometry( extentEngine->intersection( collection[i].constGet() ) ) );
      f.setAttributes( QgsAttributes() << i );
      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
      feedback->setProgress( i * step );
    }
  }

  sink->finalize();

  return dest;
}

///@endcond
