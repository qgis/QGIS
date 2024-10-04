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
#include "qgsmeshcontours.h"
#include "qgsmeshdataset.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerutils.h"
#include "qgsmeshlayertemporalproperties.h"
#include "qgsmeshlayerinterpolator.h"
#include "qgspolygon.h"
#include "qgsrasterfilewriter.h"
#include "qgslinestring.h"
#include "qgsgeometrycheckerutils.h"
#include "qgsgeometryengine.h"
#include "qgsmultilinestring.h"
#include "qgsmultipolygon.h"

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

bool QgsMeshSurfaceToPolygonAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
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

  if ( feedback )
  {
    feedback->pushInfo( QObject::tr( "Preparing data" ) );
  }

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
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters,
                                        QStringLiteral( "OUTPUT" ),
                                        context,
                                        identifier,
                                        QgsFields(),
                                        Qgis::WkbType::MultiPolygon,
                                        outputCrs ) );
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
  QMap<std::pair<int, int>, int> edges; // edge as key and count of edge occurence as value
  std::pair<int, int> edge;

  feedback->setProgressText( "Parsing mesh faces to extract edges." );

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

    feedback->setProgress( 100 * i / mNativeMesh.faceCount() );
  }

  feedback->setProgress( 0 );
  feedback->setProgressText( "Parsing mesh edges." );

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

    feedback->setProgress( 100 * i / edges.size() );
    i++;
  }

  feedback->setProgressText( "Creating final geometry." );

  if ( feedback )
  {
    if ( feedback->isCanceled() )
      return QVariantMap();
  }

  // merge lines
  QgsGeometry mergedLines = QgsGeometry( multiLineString.release() );
  mergedLines = mergedLines.mergeLines();
  QgsAbstractGeometry *multiLinesAbstract = mergedLines.get();

  // create resulting multipolygon
  std::unique_ptr<QgsMultiPolygon> multiPolygon = std::make_unique<QgsMultiPolygon>();

  // for every part create polygon and add to resulting multipolygon
  for ( int i = 0; i < mergedLines.get()->partCount(); i++ )
  {
    for ( auto pit = multiLinesAbstract->const_parts_begin(); pit != multiLinesAbstract->const_parts_end(); ++pit )
    {
      std::unique_ptr<QgsPolygon> polygon = std::make_unique<QgsPolygon>();
      polygon->setExteriorRing( qgsgeometry_cast< QgsLineString * >( *pit )->clone() );
      multiPolygon->addGeometry( polygon.release() );
    }

  }

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
