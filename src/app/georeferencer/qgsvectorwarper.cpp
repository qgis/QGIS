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
#include "qgsvectorlayer.h"
#include "qgsgeometry.h"
#include "qgsvectorlayerexporter.h"
#include "qgsprocessingfeaturesource.h"

#include <QFile>
#include <QProgressDialog>

#include "qgsvectorwarper.h"




QgsVectorWarper::QgsVectorWarper( QgsGcpTransformerInterface::TransformMethod method, const QgsGCPList points,
                                  const QgsCoordinateReferenceSystem destCrs )
  : mDestCRS( destCrs )
{
  QVector<QgsPointXY> srcPts;
  QVector<QgsPointXY> destPts;
  for ( QgsGeorefDataPoint *pt : qgis::as_const( mPoints ) )
  {
    srcPts << pt->pixelCoords();
    destPts << pt->transCoords();
  }
  mTransformer = QgsGcpGeometryTransformer( method, srcPts, destPts );
}

bool QgsVectorWarper::executeTransformInplace( QgsVectorLayer *layer )
{
  if ( !layer )
    return false;
  if ( layer->sourceCrs() != mDestCRS )
    layer.setCoordinateSystem( mDestCRS );

  QgsFeatureIterator it = layer->getFeatures( QgsFeatureRequest(), QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks );
  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    f.setGeometry( mTransofrmer.transform( f.geometry() );
  }
  return true;
}

bool QgsVectorWarper::executeTransform( const QgsVectorLayer *layer, const QString outputName )
{
  if ( !layer )
    return false;
  exporter = QgsVectorLayerExporter( ouputName, "OGR", layer->fields(), layer->geometryType(), mDestCRS, true );
  if ( exporter.errorCode() != QgsVectorLayerExporter::NoError )
    return false;

  QgsFeature outputFeature;
  QgsFeatureIterator it = layer->getFeatures( QgsFeatureRequest(), QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks );
  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    outputFeature = QgsFeature( f );
    outputFeature.setGeometry( mTransofrmer.transform( f.geometry() ) );
    exporter.addFeature( outputFeature, QgsFeatureSink.FastInsert );
  }
  exporter.flushBuffer();
  return true;
}
