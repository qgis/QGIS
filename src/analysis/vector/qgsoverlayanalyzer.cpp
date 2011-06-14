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
  if ( !layerA && !layerB )
  {
    return false;
  }

  QgsVectorDataProvider* dpA = layerA->dataProvider();
  QgsVectorDataProvider* dpB = layerB->dataProvider();
  if ( !dpA && !dpB )
  {
    return false;
  }

  QGis::WkbType outputType = dpA->geometryType();
  const QgsCoordinateReferenceSystem crs = layerA->crs();
  QgsFieldMap fieldsA = dpA->fields();
  QgsFieldMap fieldsB = dpB->fields();
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
      if ( !layerB->featureAtId( *it, currentFeature, true, true ) )
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
      if ( !layerA->featureAtId( *it, currentFeature, true, true ) )
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
    layerB->select( layerB->pendingAllAttributesList(), QgsRectangle(), true, false );
    while ( layerB->nextFeature( currentFeature ) )
    {
      index.insertFeature( currentFeature );
    }
    QgsFeature currentFeature;
    layerA->select( layerA->pendingAllAttributesList(), QgsRectangle(), true, false );

    int featureCount = layerA->featureCount();
    if ( p )
    {
      p->setMaximum( featureCount );
    }
    int processedFeatures = 0;

    while ( layerA->nextFeature( currentFeature ) )
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
  QgsGeometry* featureGeometry = f.geometry();
  QgsGeometry* intersectGeometry = 0;
  QgsFeature overlayFeature;

  if ( !featureGeometry )
  {
    return;
  }

  QList<int> intersects;
  intersects = index->intersects( featureGeometry->boundingBox() );
  QList<int>::const_iterator it = intersects.constBegin();
  QgsFeature outFeature;
  for ( ; it != intersects.constEnd(); ++it )
  {
    if ( !vl->featureAtId( *it, overlayFeature, true, true ) )
    {
      continue;
    }

    if ( featureGeometry->intersects( overlayFeature.geometry() ) )
    {
      intersectGeometry = featureGeometry->intersection( overlayFeature.geometry() );

      outFeature.setGeometry( intersectGeometry );
      QgsAttributeMap attributeMapA = f.attributeMap();
      QgsAttributeMap attributeMapB = overlayFeature.attributeMap();
      combineAttributeMaps( attributeMapA, attributeMapB );
      outFeature.setAttributeMap( attributeMapA );

      //add it to vector file writer
      if ( vfw )
      {
        vfw->addFeature( outFeature );
      }
    }
  }
}

void QgsOverlayAnalyzer::combineFieldLists( QgsFieldMap& fieldListA, QgsFieldMap fieldListB )
{
  QList<QString> names;
  QMap<int, QgsField>::const_iterator j = fieldListA.constBegin();
  while ( j != fieldListA.constEnd() )
  {
    names.append( j.value().name() );
    ++j;
  }
  QMap<int, QgsField>::const_iterator i = fieldListB.constBegin();
  int count = 0;
  int fcount = fieldListA.size();
  QgsField field;
  while ( i != fieldListB.constEnd() )
  {
    field = i.value();
    while ( names.contains( field.name() ) )
    {
      QString name = field.name();
      name.append( "_" ).append( QString( count ) );
      field = QgsField( name, field.type() );
      ++count;
    }
    fieldListA.insert( fcount, field );
    count = 0;
    ++fcount;
    ++i;
  }
}

void QgsOverlayAnalyzer::combineAttributeMaps( QgsAttributeMap& attributeMapA, QgsAttributeMap attributeMapB )
{
  QMap<int, QVariant>::const_iterator i = attributeMapB.constBegin();
  QVariant attribute;
  int fcount = attributeMapA.size();
  while ( i != attributeMapB.constEnd() )
  {
    attribute = i.value();
    attributeMapA.insert( fcount, attribute );
    ++i;
    ++fcount;
  }
}

