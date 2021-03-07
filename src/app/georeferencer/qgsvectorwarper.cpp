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
#include "qgsvectorfilewriter.h"
#include "qgsfeedback.h"
#include "qgsmessagelog.h"

#include <QFile>
#include <QProgressDialog>

#include "qgsvectorwarper.h"

QgsVectorWarper::QgsVectorWarper( QgsGcpTransformerInterface::TransformMethod &method, const QgsGCPList &points,
                                  const QgsCoordinateReferenceSystem &destCrs )
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

bool QgsVectorWarper::executeTransformInplace( QgsVectorLayer *layer, QgsFeedback *feedback )
{
  if ( !layer )
    return false;
  if ( layer->sourceCrs() != mDestCRS )
    layer->setCrs( mDestCRS );

  QgsFeatureIterator it = layer->getFeatures( QgsFeatureRequest() );
  QgsFeature f;
  QgsGeometry transformed;
  bool ok;
  int featureCount;
  if ( feedback )
  {
    featureCount = layer->featureCount();
    feedback->setProgress( 0 );
  }
  int i = 0;
  while ( it.nextFeature( f ) )
  {
    if ( feedback )
    {
      if ( feedback->isCanceled() )
        break;

      feedback->setProgress( 100 * i / featureCount );
      i++;
    }
    transformed = mTransformer->transform( f.geometry(), ok );
    if ( ok )
      f.setGeometry( transformed );
    else
      QgsMessageLog::logMessage( QObject::tr( "Error performing transformation on a feature, geometry unchanged" ) );
  }
  return true;
}

bool QgsVectorWarper::executeTransform( const QgsVectorLayer *layer, const QString outputName, QgsFeedback *feedback )
{
  if ( !layer )
    return false;
  QgsVectorFileWriter::SaveVectorOptions saveOptions;
  std::unique_ptr< QgsVectorFileWriter > exporter( QgsVectorFileWriter::create( outputName, layer->fields(), layer->wkbType(), mDestCRS, QgsCoordinateTransformContext(), saveOptions ) );
  if ( exporter->hasError() )
    return false;

  QgsFeature outputFeature;
  QgsFeatureIterator it = layer->getFeatures( QgsFeatureRequest() );
  QgsFeature f;
  QgsGeometry transformed;
  bool ok;
  int featureCount;
  bool allGood = true;
  if ( feedback )
  {
    featureCount = layer->featureCount();
    feedback->setProgress( 0 );
  }
  int i = 0;
  while ( it.nextFeature( f ) )
  {
    if ( feedback )
    {
      if ( feedback->isCanceled() )
        break;

      feedback->setProgress( 100 * i / featureCount );
      i++;
    }
    outputFeature = QgsFeature( f );
    transformed = mTransformer->transform( f.geometry(), ok );
    if ( ok )
    {
      outputFeature.setGeometry( transformed );
      exporter->addFeature( outputFeature, QgsFeatureSink::FastInsert );
      if ( exporter->hasError() )
      {
        allGood = false;
        QgsMessageLog::logMessage( QObject::tr( "Error performing transformation: {}" ).arg( exporter->errorMessage() ) );
      }
    }
  }
  return allGood;
}
