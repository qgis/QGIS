/***************************************************************************
    qgsoverlayanalyzer.cpp - QGIS Tools for vector geometry analysis
                             -------------------
    begin                : 8 Nov 2009
    copyright            : (C) Carson J. Q. Farmer
    email                : carson.farmer@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsoverlayanalyzer.h"

#include "qgsapplication.h"
#include "qgsfield.h"
#include "qgsfeature.h"
#include "qgslogger.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectordataprovider.h"
#include "qgsdistancearea.h"
#include <QProgressDialog>

bool QgsOverlayAnalyzer::intersection( QgsVectorLayer* layerA, QgsVectorLayer* layerB,
                                       const QString& shapefileName, bool onlySelectedFeatures,
                                       QProgressDialog* p )
{
  if ( !layerA || !layerB )
  {
    return false;
  }

  QgsVectorDataProvider *dpA = layerA->dataProvider();
  QgsVectorDataProvider *dpB = layerB->dataProvider();
  if ( !dpA || !dpB )
  {
    return false;
  }

  QGis::WkbType outputType = dpA->geometryType();
  QgsCoordinateReferenceSystem crs = layerA->crs();
  QgsFields fieldsA = layerA->fields();
  QgsFields fieldsB = layerB->fields();
  combineFieldLists( fieldsA, fieldsB );

  QgsVectorFileWriter vWriter( shapefileName, dpA->encoding(), fieldsA, outputType, &crs );
  QgsFeature currentFeature;
  QgsSpatialIndex index;

  //take only selection
  if ( onlySelectedFeatures )
  {
    const QgsFeatureIds selectionB = layerB->selectedFeaturesIds();
    QgsFeatureIds::const_iterator it = selectionB.constBegin();
    for ( ; it != selectionB.constEnd(); ++it )
    {
      if ( !layerB->getFeatures( QgsFeatureRequest().setFilterFid( *it ) ).nextFeature( currentFeature ) )
      {
        continue;
      }
      index.insertFeature( currentFeature );
    }
    //use QgsVectorLayer::featureAtId
    const QgsFeatureIds selectionA = layerA->selectedFeaturesIds();
    if ( p )
    {
      p->setMaximum( selectionA.size() );
    }
    QgsFeature currentFeature;
    int processedFeatures = 0;
    it = selectionA.constBegin();
    for ( ; it != selectionA.constEnd(); ++it )
    {
      if ( p )
      {
        p->setValue( processedFeatures );
      }

      if ( p && p->wasCanceled() )
      {
        break;
      }
      if ( !layerA->getFeatures( QgsFeatureRequest().setFilterFid( *it ) ).nextFeature( currentFeature ) )
      {
        continue;
      }
      intersectFeature( currentFeature, &vWriter, layerB, &index );
      ++processedFeatures;
    }

    if ( p )
    {
      p->setValue( selectionA.size() );
    }
  }
  //take all features
  else
  {
    QgsFeatureIterator fit = layerB->getFeatures();
    while ( fit.nextFeature( currentFeature ) )
    {
      index.insertFeature( currentFeature );
    }

    int featureCount = layerA->featureCount();
    if ( p )
    {
      p->setMaximum( featureCount );
    }
    int processedFeatures = 0;

    fit = layerA->getFeatures();

    QgsFeature currentFeature;
    while ( fit.nextFeature( currentFeature ) )
    {
      if ( p )
      {
        p->setValue( processedFeatures );
      }
      if ( p && p->wasCanceled() )
      {
        break;
      }
      intersectFeature( currentFeature, &vWriter, layerB, &index );
      ++processedFeatures;
    }
    if ( p )
    {
      p->setValue( featureCount );
    }
  }
  return true;
}

void QgsOverlayAnalyzer::intersectFeature( QgsFeature& f, QgsVectorFileWriter* vfw,
    QgsVectorLayer* vl, QgsSpatialIndex* index )
{
  if ( !f.constGeometry() )
  {
    return;
  }

  const QgsGeometry* featureGeometry = f.constGeometry();
  QgsGeometry* intersectGeometry = nullptr;
  QgsFeature overlayFeature;

  QList<QgsFeatureId> intersects;
  intersects = index->intersects( featureGeometry->boundingBox() );
  QList<QgsFeatureId>::const_iterator it = intersects.constBegin();
  QgsFeature outFeature;
  for ( ; it != intersects.constEnd(); ++it )
  {
    if ( !vl->getFeatures( QgsFeatureRequest().setFilterFid( *it ) ).nextFeature( overlayFeature ) )
    {
      continue;
    }

    if ( featureGeometry->intersects( overlayFeature.constGeometry() ) )
    {
      intersectGeometry = featureGeometry->intersection( overlayFeature.constGeometry() );

      outFeature.setGeometry( intersectGeometry );
      QgsAttributes attributesA = f.attributes();
      QgsAttributes attributesB = overlayFeature.attributes();
      combineAttributeMaps( attributesA, attributesB );
      outFeature.setAttributes( attributesA );

      //add it to vector file writer
      if ( vfw )
      {
        vfw->addFeature( outFeature );
      }
    }
  }
}

void QgsOverlayAnalyzer::combineFieldLists( QgsFields& fieldListA, const QgsFields& fieldListB )
{
  QList<QString> names;
  Q_FOREACH ( const QgsField& field, fieldListA )
    names.append( field.name() );

  for ( int idx = 0; idx < fieldListB.count(); ++idx )
  {
    QgsField field = fieldListB[idx];
    int count = 0;
    while ( names.contains( field.name() ) )
    {
      QString name = QString( "%1_%2" ).arg( field.name() ).arg( count );
      field = QgsField( name, field.type() );
      ++count;
    }
    fieldListA.append( field );
    names.append( field.name() );
  }
}

void QgsOverlayAnalyzer::combineAttributeMaps( QgsAttributes& attributesA, const QgsAttributes& attributesB )
{
  attributesA += attributesB;
}

