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

  std::unique_ptr< QgsGeometryEngine > geomEngine;
  QgsGeometry geom;


  for ( int i = 0; i < mNativeMesh.faceCount(); i++ )
  {
    if ( feedback )
    {
      if ( feedback->isCanceled() )
        return QVariantMap();
    }

    const QgsMeshFace &face = mNativeMesh.face( i );
    QVector<QgsPoint> vertices( face.size() );
    for ( int j = 0; j < face.size(); ++j )
      vertices[j] = mNativeMesh.vertex( face.at( j ) );

    QgsPolygon *polygon = new QgsPolygon();
    polygon->setExteriorRing( new QgsLineString( vertices ) );

    if ( i == 0 )
    {
      geom = QgsGeometry( polygon );
    }
    else
    {
      std::unique_ptr< QgsGeometryEngine > geomEngine = QgsGeometryCheckerUtils::createGeomEngine( geom.get(), 0 );
      geom = QgsGeometry( geomEngine->combine( polygon ) );
    }

    feedback->setProgress( 100 * i / mNativeMesh.faceCount() );
  }

  try
  {
    geom.transform( mTransform );
  }
  catch ( QgsCsException & )
  {
    if ( feedback )
      feedback->reportError( QObject::tr( "Could not transform point to destination CRS" ) );
  }

  QgsFeature feat;
  feat.setGeometry( geom );

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
