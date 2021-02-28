/***************************************************************************
     qgsvectorwarper.cpp
     --------------------------------------
    Date                 :
    Copyright            :
    Email                :
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <cmath>
#include <cstdio>

#include "qgsgcpgeometrytransformer.h"
#include "qgsgcplist.h"
#include "qgsgeorefdatapoint.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransformcontext.h"
#include "qgsvectorlayer.h"
#include "qgsgeometry.h"
#include "qgsprocessingutils.h"
#include "qgsprocessingcontext.h"

#include <QFile>
#include <QProgressDialog>

#include "qgsvectorwarper.h"

QgsVectorWarper::QgsVectorWarper( QgsGcpTransformerInterface::TransformMethod& method, const QgsGCPList& points,
                                  const QgsCoordinateReferenceSystem& destCrs )
  : mDestCRS( destCrs )
{
  QVector<QgsPointXY> srcPts;
  QVector<QgsPointXY> destPts;
  for ( QgsGeorefDataPoint *pt : qgis::as_const( points ) )
  {
    srcPts << pt->pixelCoords();
    destPts << pt->transCoords();
  }
  mTransformer = new QgsGcpGeometryTransformer( method, srcPts, destPts );
}

bool QgsVectorWarper::executeTransformInplace( QgsVectorLayer *layer )
{
  if ( !layer )
    return false;
  if ( layer->sourceCrs() != mDestCRS )
    layer->setCrs( mDestCRS );

  QgsFeatureIterator it = layer->getFeatures( QgsFeatureRequest() );
  QgsFeature f;
  QgsGeometry transformed;
  bool ok;
  while ( it.nextFeature( f ) )
  {
    transformed = mTransformer->transform( f.geometry(), ok );
    if ( ok )
      f.setGeometry( transformed );
  }
  return true;
}

bool QgsVectorWarper::executeTransform( const QgsVectorLayer *layer, const QString outputName )
{
  if ( !layer )
    return false;
  QString name2( outputName );
  QgsProcessingContext *context = new QgsProcessingContext();
  std::unique_ptr< QgsFeatureSink > exporter( QgsProcessingUtils::createFeatureSink( name2, *context, layer->fields(), layer->wkbType(), mDestCRS ) );
  if ( !exporter->lastError().isEmpty() )
    return false;

  QgsFeature outputFeature;
  QgsFeatureIterator it = layer->getFeatures( QgsFeatureRequest() );
  QgsFeature f;
  QgsGeometry transformed;
  bool ok;
  bool allGood = true;
  while ( it.nextFeature( f ) )
  {
    outputFeature = QgsFeature( f );
    transformed = mTransformer->transform( f.geometry(), ok );
    if ( ok )
    {
      outputFeature.setGeometry( transformed );
      exporter->addFeature( outputFeature, QgsFeatureSink::FastInsert );
      if ( !exporter->lastError().isEmpty() )
          allGood = false;
    }
  }
  return allGood;
}
