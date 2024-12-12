/***************************************************************************
                         qgsalgorithmmeshsurfacetopolygon.cpp
                         ---------------------------
    begin                : September 2024
    copyright            : (C) 2024 by Jan Caha
    email                : jan.caha at outlook dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmmeshsurfacetopolygon.h"
#include "qgsprocessingparametermeshdataset.h"
#include "qgsmeshlayer.h"
#include "qgspolygon.h"
#include "qgslinestring.h"
#include "qgsmultilinestring.h"
#include "qgsmultipolygon.h"
#include "qgsgeometryengine.h"

#include <QTextStream>

///@cond PRIVATE


QString QgsMeshSurfaceToPolygonAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm exports a polygon file containing mesh layer boundary. It may contain holes and it may be a multi-part polygon." );
}

QString QgsMeshSurfaceToPolygonAlgorithm::name() const
{
  return QStringLiteral( "surfacetopolygon" );
}

QString QgsMeshSurfaceToPolygonAlgorithm::displayName() const
{
  return QObject::tr( "Surface to polygon" );
}

QString QgsMeshSurfaceToPolygonAlgorithm::group() const
{
  return QObject::tr( "Mesh" );
}

QString QgsMeshSurfaceToPolygonAlgorithm::groupId() const
{
  return QStringLiteral( "mesh" );
}

QgsProcessingAlgorithm *QgsMeshSurfaceToPolygonAlgorithm::createInstance() const
{
  return new QgsMeshSurfaceToPolygonAlgorithm();
}

void QgsMeshSurfaceToPolygonAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration );

  addParameter( new QgsProcessingParameterMeshLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input mesh layer" ) ) );

  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS_OUTPUT" ), QObject::tr( "Output coordinate system" ), QVariant(), true ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Output vector layer" ), Qgis::ProcessingSourceType::VectorPolygon ) );
}

bool QgsMeshSurfaceToPolygonAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMeshLayer *meshLayer = parameterAsMeshLayer( parameters, QStringLiteral( "INPUT" ), context );

  if ( !meshLayer || !meshLayer->isValid() )
    return false;

  if ( meshLayer->isEditable() )
    throw QgsProcessingException( QObject::tr( "Input mesh layer in edit mode is not supported" ) );

  QgsCoordinateReferenceSystem outputCrs = parameterAsCrs( parameters, QStringLiteral( "CRS_OUTPUT" ), context );
  if ( !outputCrs.isValid() )
    outputCrs = meshLayer->crs();
  mTransform = QgsCoordinateTransform( meshLayer->crs(), outputCrs, context.transformContext() );
  if ( !meshLayer->nativeMesh() )
    meshLayer->updateTriangularMesh( mTransform ); //necessary to load the native mesh

  mNativeMesh = *meshLayer->nativeMesh();

  return true;
}


QVariantMap QgsMeshSurfaceToPolygonAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( feedback )
  {
    if ( feedback->isCanceled() )
      return QVariantMap();
    feedback->setProgress( 0 );
    feedback->pushInfo( QObject::tr( "Creating output vector layer" ) );
  }

  QgsCoordinateReferenceSystem outputCrs = parameterAsCrs( parameters, QStringLiteral( "CRS_OUTPUT" ), context );
  QString identifier;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, identifier, QgsFields(), Qgis::WkbType::MultiPolygon, outputCrs ) );
  if ( !sink )
    return QVariantMap();

  if ( feedback )
  {
    if ( feedback->isCanceled() )
      return QVariantMap();
    feedback->setProgress( 0 );
  }

  QgsGeometry lines;
  QgsMeshFace face;
  QMap<std::pair<int, int>, int> edges; // edge as key and count of edge usage as value
  std::pair<int, int> edge;

  if ( feedback )
    feedback->setProgressText( QObject::tr( "Parsing mesh faces to extract edges." ) );

  for ( int i = 0; i < mNativeMesh.faceCount(); i++ )
  {
    if ( feedback )
    {
      if ( feedback->isCanceled() )
        return QVariantMap();
    }

    face = mNativeMesh.face( i );

    for ( int j = 0; j < face.size(); j++ )
    {
      int indexEnd;
      if ( j == face.size() - 1 )
        indexEnd = 0;
      else
        indexEnd = j + 1;
      int edgeFirstVertex = face.at( j );
      int edgeSecondVertex = face.at( indexEnd );

      // make vertex sorted to avoid have 1,2 and 2,1 as different keys
      if ( edgeSecondVertex < edgeFirstVertex )
        edge = std::make_pair( edgeSecondVertex, edgeFirstVertex );
      else
        edge = std::make_pair( edgeFirstVertex, edgeSecondVertex );

      // if edge exist in map increase its count otherwise set count to 1
      auto it = edges.find( edge );
      if ( it != edges.end() )
      {
        int count = edges.take( edge ) + 1;
        edges.insert( edge, count );
      }
      else
      {
        edges.insert( edge, 1 );
      }
    }

    if ( feedback )
      feedback->setProgress( 100.0 * static_cast<double>( i ) / mNativeMesh.faceCount() );
  }

  if ( feedback )
  {
    feedback->setProgress( 0 );
    feedback->setProgressText( QObject::tr( "Parsing mesh edges." ) );
  }

  std::unique_ptr<QgsMultiLineString> multiLineString( new QgsMultiLineString() );

  int i = 0;
  for ( auto it = edges.begin(); it != edges.end(); it++ )
  {
    if ( feedback )
    {
      if ( feedback->isCanceled() )
        return QVariantMap();
    }

    // only consider edges with count 1 which are on the edge of mesh surface
    if ( it.value() == 1 )
    {
      std::unique_ptr<QgsLineString> line( new QgsLineString( mNativeMesh.vertex( it.key().first ), mNativeMesh.vertex( it.key().second ) ) );
      multiLineString->addGeometry( line.release() );
    }

    if ( feedback )
      feedback->setProgress( 100.0 * static_cast<double>( i ) / edges.size() );

    i++;
  }

  if ( feedback )
  {
    feedback->setProgressText( QObject::tr( "Creating final geometry." ) );
    if ( feedback->isCanceled() )
      return QVariantMap();
  }

  // merge lines
  QgsGeometry mergedLines = QgsGeometry( multiLineString.release() );
  mergedLines = mergedLines.mergeLines();
  QgsAbstractGeometry *multiLinesAbstract = mergedLines.get();

  // set of polygons to construct result
  QVector<QgsAbstractGeometry *> polygons;

  // for every part create polygon and add to resulting multipolygon
  for ( auto pit = multiLinesAbstract->const_parts_begin(); pit != multiLinesAbstract->const_parts_end(); ++pit )
  {
    if ( feedback )
    {
      if ( feedback->isCanceled() )
        return QVariantMap();
    }

    // individula polygon - can be either polygon or hole in polygon
    QgsPolygon *polygon = new QgsPolygon();
    polygon->setExteriorRing( qgsgeometry_cast<QgsLineString *>( *pit )->clone() );

    // add first polygon, no need to check anything
    if ( polygons.empty() )
    {
      polygons.push_back( polygon );
      continue;
    }

    // engine for spatial relations
    std::unique_ptr<QgsGeometryEngine> engine( QgsGeometry::createGeometryEngine( polygon ) );

    // need to check if polygon is not either contained (hole) or covering (main polygon) with another
    // this solves meshes with holes
    bool isHole = false;

    for ( int i = 0; i < polygons.count(); i++ )
    {
      QgsPolygon *p = qgsgeometry_cast<QgsPolygon *>( polygons.at( i ) );

      // polygon covers another, turn contained polygon into interior ring
      if ( engine->contains( p ) )
      {
        polygons.removeAt( i );
        polygon->addInteriorRing( p->exteriorRing()->clone() );
        break;
      }
      // polygon is within another, make it interior rind and do not add it
      else if ( engine->within( p ) )
      {
        p->addInteriorRing( polygon->exteriorRing()->clone() );
        isHole = true;
        break;
      }
    }

    // if is not a hole polygon add it to the vector of polygons
    if ( !isHole )
      polygons.append( polygon );
    else
      // polygon was used as a hole, it is not needed anymore, delete it to avoid memory leak
      delete polygon;
  }

  // create resulting multipolygon
  std::unique_ptr<QgsMultiPolygon> multiPolygon = std::make_unique<QgsMultiPolygon>();
  multiPolygon->addGeometries( polygons );

  if ( feedback )
  {
    if ( feedback->isCanceled() )
      return QVariantMap();
  }

  // create final geom and transform it
  QgsGeometry resultGeom = QgsGeometry( multiPolygon.release() );

  try
  {
    resultGeom.transform( mTransform );
  }
  catch ( QgsCsException & )
  {
    if ( feedback )
      feedback->reportError( QObject::tr( "Could not transform point to destination CRS" ) );
  }

  QgsFeature feat;
  feat.setGeometry( resultGeom );

  if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
    throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );

  if ( feedback )
  {
    feedback->pushInfo( QObject::tr( "Output vector layer created" ) );
    if ( feedback->isCanceled() )
      return QVariantMap();
  }

  QVariantMap ret;
  ret[QStringLiteral( "OUTPUT" )] = identifier;

  return ret;
}

///@endcond PRIVATE
