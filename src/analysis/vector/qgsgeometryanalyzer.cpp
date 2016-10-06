/***************************************************************************
    qgsgeometryanalyzer.cpp - QGIS Tools for vector geometry analysis
                             -------------------
    begin                : 19 March 2009
    copyright            : (C) Carson Farmer
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

#include "qgsgeometryanalyzer.h"

#include "qgsapplication.h"
#include "qgsfields.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgslogger.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectordataprovider.h"
#include "qgsdistancearea.h"
#include "qgis.h"
#include "qgsvectorlayer.h"

#include <QProgressDialog>

bool QgsGeometryAnalyzer::simplify( QgsVectorLayer* layer,
                                    const QString& shapefileName,
                                    double tolerance,
                                    bool onlySelectedFeatures,
                                    QProgressDialog *p )
{
  if ( !layer )
  {
    return false;
  }

  QgsVectorDataProvider* dp = layer->dataProvider();
  if ( !dp )
  {
    return false;
  }

  QgsWkbTypes::Type outputType = dp->wkbType();
  QgsCoordinateReferenceSystem crs = layer->crs();

  QgsVectorFileWriter vWriter( shapefileName, dp->encoding(), layer->fields(), outputType, crs );
  QgsFeature currentFeature;

  //take only selection
  if ( onlySelectedFeatures )
  {
    //use QgsVectorLayer::featureAtId
    const QgsFeatureIds selection = layer->selectedFeaturesIds();
    if ( p )
    {
      p->setMaximum( selection.size() );
    }

    int processedFeatures = 0;
    QgsFeatureIds::const_iterator it = selection.constBegin();
    for ( ; it != selection.constEnd(); ++it )
    {
      if ( p )
      {
        p->setValue( processedFeatures );
      }

      if ( p && p->wasCanceled() )
      {
        break;
      }
      if ( !layer->getFeatures( QgsFeatureRequest().setFilterFid( *it ) ).nextFeature( currentFeature ) )
      {
        continue;
      }
      simplifyFeature( currentFeature, &vWriter, tolerance );
      ++processedFeatures;
    }

    if ( p )
    {
      p->setValue( selection.size() );
    }
  }
  //take all features
  else
  {
    QgsFeatureIterator fit = layer->getFeatures();

    int featureCount = layer->featureCount();
    if ( p )
    {
      p->setMaximum( featureCount );
    }
    int processedFeatures = 0;

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
      simplifyFeature( currentFeature, &vWriter, tolerance );
      ++processedFeatures;
    }
    if ( p )
    {
      p->setValue( featureCount );
    }
  }

  return true;
}

void QgsGeometryAnalyzer::simplifyFeature( QgsFeature& f, QgsVectorFileWriter* vfw, double tolerance )
{
  if ( !f.hasGeometry() )
  {
    return;
  }

  QgsGeometry featureGeometry = f.geometry();

  // simplify feature
  QgsGeometry tmpGeometry = featureGeometry.simplify( tolerance );

  QgsFeature newFeature;
  newFeature.setGeometry( tmpGeometry );
  newFeature.setAttributes( f.attributes() );

  //add it to vector file writer
  if ( vfw )
  {
    vfw->addFeature( newFeature );
  }
}

bool QgsGeometryAnalyzer::centroids( QgsVectorLayer* layer, const QString& shapefileName,
                                     bool onlySelectedFeatures, QProgressDialog* p )
{
  if ( !layer )
  {
    QgsDebugMsg( "No layer passed to centroids" );
    return false;
  }

  QgsVectorDataProvider* dp = layer->dataProvider();
  if ( !dp )
  {
    QgsDebugMsg( "No data provider for layer passed to centroids" );
    return false;
  }

  QgsWkbTypes::Type outputType = QgsWkbTypes::Point;
  QgsCoordinateReferenceSystem crs = layer->crs();

  QgsVectorFileWriter vWriter( shapefileName, dp->encoding(), layer->fields(), outputType, crs );
  QgsFeature currentFeature;

  //take only selection
  if ( onlySelectedFeatures )
  {
    //use QgsVectorLayer::featureAtId
    const QgsFeatureIds selection = layer->selectedFeaturesIds();
    if ( p )
    {
      p->setMaximum( selection.size() );
    }

    int processedFeatures = 0;
    QgsFeatureIds::const_iterator it = selection.constBegin();
    for ( ; it != selection.constEnd(); ++it )
    {
      if ( p )
      {
        p->setValue( processedFeatures );
      }

      if ( p && p->wasCanceled() )
      {
        break;
      }
      if ( !layer->getFeatures( QgsFeatureRequest().setFilterFid( *it ) ).nextFeature( currentFeature ) )
      {
        continue;
      }
      centroidFeature( currentFeature, &vWriter );
      ++processedFeatures;
    }

    if ( p )
    {
      p->setValue( selection.size() );
    }
  }
  //take all features
  else
  {
    QgsFeatureIterator fit = layer->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() ) );

    int featureCount = layer->featureCount();
    if ( p )
    {
      p->setMaximum( featureCount );
    }
    int processedFeatures = 0;

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
      centroidFeature( currentFeature, &vWriter );
      ++processedFeatures;
    }
    if ( p )
    {
      p->setValue( featureCount );
    }
  }

  return true;
}


void QgsGeometryAnalyzer::centroidFeature( QgsFeature& f, QgsVectorFileWriter* vfw )
{
  if ( !f.hasGeometry() )
  {
    return;
  }

  QgsGeometry featureGeometry = f.geometry();
  QgsFeature newFeature;
  newFeature.setGeometry( featureGeometry.centroid() );
  newFeature.setAttributes( f.attributes() );

  //add it to vector file writer
  if ( vfw )
  {
    vfw->addFeature( newFeature );
  }
}

bool QgsGeometryAnalyzer::extent( QgsVectorLayer* layer,
                                  const QString& shapefileName,
                                  bool onlySelectedFeatures,
                                  QProgressDialog * )
{
  if ( !layer )
  {
    return false;
  }

  QgsVectorDataProvider* dp = layer->dataProvider();
  if ( !dp )
  {
    return false;
  }

  QgsWkbTypes::Type outputType = QgsWkbTypes::Polygon;
  QgsCoordinateReferenceSystem crs = layer->crs();

  QgsFields fields;
  fields.append( QgsField( QString( "MINX" ), QVariant::Double ) );
  fields.append( QgsField( QString( "MINY" ), QVariant::Double ) );
  fields.append( QgsField( QString( "MAXX" ), QVariant::Double ) );
  fields.append( QgsField( QString( "MAXY" ), QVariant::Double ) );
  fields.append( QgsField( QString( "CNTX" ), QVariant::Double ) );
  fields.append( QgsField( QString( "CNTY" ), QVariant::Double ) );
  fields.append( QgsField( QString( "AREA" ), QVariant::Double ) );
  fields.append( QgsField( QString( "PERIM" ), QVariant::Double ) );
  fields.append( QgsField( QString( "HEIGHT" ), QVariant::Double ) );
  fields.append( QgsField( QString( "WIDTH" ), QVariant::Double ) );

  QgsVectorFileWriter vWriter( shapefileName, dp->encoding(), fields, outputType, crs );

  QgsRectangle rect;
  if ( onlySelectedFeatures )  // take only selection
  {
    rect = layer->boundingBoxOfSelected();
  }
  else
  {
    rect = layer->extent();
  }

  double minx = rect.xMinimum();
  double miny = rect.yMinimum();
  double maxx = rect.xMaximum();
  double maxy = rect.yMaximum();
  double height = rect.height();
  double width = rect.width();
  double cntx = minx + ( width / 2.0 );
  double cnty = miny + ( height / 2.0 );
  double area = width * height;
  double perim = ( 2 * width ) + ( 2 * height );

  QgsFeature feat;
  QgsAttributes attrs( 10 );
  attrs[0] = QVariant( minx );
  attrs[1] = QVariant( miny );
  attrs[2] = QVariant( maxx );
  attrs[3] = QVariant( maxy );
  attrs[4] = QVariant( cntx );
  attrs[5] = QVariant( cnty );
  attrs[6] = QVariant( area );
  attrs[7] = QVariant( perim );
  attrs[8] = QVariant( height );
  attrs[9] = QVariant( width );
  feat.setAttributes( attrs );
  feat.setGeometry( QgsGeometry::fromRect( rect ) );
  vWriter.addFeature( feat );
  return true;
}

QList<double> QgsGeometryAnalyzer::simpleMeasure( QgsGeometry& mpGeometry )
{
  QList<double> list;
  double perim;
  if ( mpGeometry.wkbType() == QgsWkbTypes::Point )
  {
    QgsPoint pt = mpGeometry.asPoint();
    list.append( pt.x() );
    list.append( pt.y() );
  }
  else
  {
    QgsDistanceArea measure;
    list.append( measure.measureArea( mpGeometry ) );
    if ( mpGeometry.type() == QgsWkbTypes::PolygonGeometry )
    {
      perim = perimeterMeasure( mpGeometry, measure );
      list.append( perim );
    }
  }
  return list;
}

double QgsGeometryAnalyzer::perimeterMeasure( const QgsGeometry& geometry, QgsDistanceArea& measure )
{
  return measure.measurePerimeter( geometry );
}

bool QgsGeometryAnalyzer::convexHull( QgsVectorLayer* layer, const QString& shapefileName,
                                      bool onlySelectedFeatures, int uniqueIdField, QProgressDialog* p )
{
  if ( !layer )
  {
    return false;
  }
  QgsVectorDataProvider* dp = layer->dataProvider();
  if ( !dp )
  {
    return false;
  }
  bool useField = false;
  if ( uniqueIdField == -1 )
  {
    uniqueIdField = 0;
  }
  else
  {
    useField = true;
  }
  QgsFields fields;
  fields.append( QgsField( QString( "UID" ), QVariant::String ) );
  fields.append( QgsField( QString( "AREA" ), QVariant::Double ) );
  fields.append( QgsField( QString( "PERIM" ), QVariant::Double ) );

  QgsWkbTypes::Type outputType = QgsWkbTypes::Polygon;
  QgsCoordinateReferenceSystem crs = layer->crs();

  QgsVectorFileWriter vWriter( shapefileName, dp->encoding(), fields, outputType, crs );
  QgsFeature currentFeature;
  QgsGeometry dissolveGeometry; //dissolve geometry
  QMultiMap<QString, QgsFeatureId> map;

  if ( onlySelectedFeatures )
  {
    //use QgsVectorLayer::featureAtId
    const QgsFeatureIds selection = layer->selectedFeaturesIds();
    QgsFeatureIds::const_iterator it = selection.constBegin();
    for ( ; it != selection.constEnd(); ++it )
    {
#if 0
      if ( p )
      {
        p->setValue( processedFeatures );
      }
      if ( p && p->wasCanceled() )
      {
        // break; // it may be better to do something else here?
        return false;
      }
#endif
      if ( !layer->getFeatures( QgsFeatureRequest().setFilterFid( *it ) ).nextFeature( currentFeature ) )
      {
        continue;
      }
      map.insert( currentFeature.attribute( uniqueIdField ).toString(), currentFeature.id() );
    }
  }
  else
  {
    QgsFeatureIterator fit = layer->getFeatures();
    while ( fit.nextFeature( currentFeature ) )
    {
#if 0
      if ( p )
      {
        p->setValue( processedFeatures );
      }
      if ( p && p->wasCanceled() )
      {
        // break; // it may be better to do something else here?
        return false;
      }
#endif
      map.insert( currentFeature.attribute( uniqueIdField ).toString(), currentFeature.id() );
    }
  }

  QMultiMap<QString, QgsFeatureId>::const_iterator jt = map.constBegin();
  while ( jt != map.constEnd() )
  {
    QString currentKey = jt.key();
    int processedFeatures = 0;
    //take only selection
    if ( onlySelectedFeatures )
    {
      //use QgsVectorLayer::featureAtId
      const QgsFeatureIds selection = layer->selectedFeaturesIds();
      if ( p )
      {
        p->setMaximum( selection.size() );
      }
      processedFeatures = 0;
      while ( jt != map.constEnd() && ( jt.key() == currentKey || !useField ) )
      {
        if ( p && p->wasCanceled() )
        {
          break;
        }
        if ( selection.contains( jt.value() ) )
        {
          if ( p )
          {
            p->setValue( processedFeatures );
          }
          if ( !layer->getFeatures( QgsFeatureRequest().setFilterFid( jt.value() ) ).nextFeature( currentFeature ) )
          {
            continue;
          }
          convexFeature( currentFeature, processedFeatures, dissolveGeometry );
          ++processedFeatures;
        }
        ++jt;
      }
      QList<double> values;
      if ( dissolveGeometry.isEmpty() )
      {
        QgsDebugMsg( "no dissolved geometry - should not happen" );
        return false;
      }
      dissolveGeometry = dissolveGeometry.convexHull();
      values = simpleMeasure( dissolveGeometry );
      QgsAttributes attributes( 3 );
      attributes[0] = QVariant( currentKey );
      attributes[1] = values.at( 0 );
      attributes[2] = values.at( 1 );
      QgsFeature dissolveFeature;
      dissolveFeature.setAttributes( attributes );
      dissolveFeature.setGeometry( dissolveGeometry );
      vWriter.addFeature( dissolveFeature );
    }
    //take all features
    else
    {
      int featureCount = layer->featureCount();
      if ( p )
      {
        p->setMaximum( featureCount );
      }
      processedFeatures = 0;
      while ( jt != map.constEnd() && ( jt.key() == currentKey || !useField ) )
      {
        if ( p )
        {
          p->setValue( processedFeatures );
        }

        if ( p && p->wasCanceled() )
        {
          break;
        }
        if ( !layer->getFeatures( QgsFeatureRequest().setFilterFid( jt.value() ) ).nextFeature( currentFeature ) )
        {
          continue;
        }
        convexFeature( currentFeature, processedFeatures, dissolveGeometry );
        ++processedFeatures;
        ++jt;
      }
      QList<double> values;
      if ( dissolveGeometry.isEmpty() )
      {
        QgsDebugMsg( "no dissolved geometry - should not happen" );
        return false;
      }
      dissolveGeometry = dissolveGeometry.convexHull();
      // values = simpleMeasure( tmpGeometry );
      values = simpleMeasure( dissolveGeometry );
      QgsAttributes attributes;
      attributes[0] = QVariant( currentKey );
      attributes[1] = QVariant( values[ 0 ] );
      attributes[2] = QVariant( values[ 1 ] );
      QgsFeature dissolveFeature;
      dissolveFeature.setAttributes( attributes );
      dissolveFeature.setGeometry( dissolveGeometry );
      vWriter.addFeature( dissolveFeature );
    }
  }
  return true;
}


void QgsGeometryAnalyzer::convexFeature( QgsFeature& f, int nProcessedFeatures, QgsGeometry& dissolveGeometry )
{
  if ( !f.hasGeometry() )
  {
    return;
  }

  QgsGeometry featureGeometry = f.geometry();
  QgsGeometry convexGeometry = featureGeometry.convexHull();

  if ( nProcessedFeatures == 0 )
  {
    dissolveGeometry = convexGeometry;
  }
  else
  {
    dissolveGeometry = dissolveGeometry.combine( convexGeometry );
  }
}

bool QgsGeometryAnalyzer::dissolve( QgsVectorLayer* layer, const QString& shapefileName,
                                    bool onlySelectedFeatures, int uniqueIdField, QProgressDialog* p )
{
  if ( !layer )
  {
    return false;
  }
  QgsVectorDataProvider* dp = layer->dataProvider();
  if ( !dp )
  {
    return false;
  }
  bool useField = false;
  if ( uniqueIdField == -1 )
  {
    uniqueIdField = 0;
  }
  else
  {
    useField = true;
  }

  QgsWkbTypes::Type outputType = dp->wkbType();
  QgsCoordinateReferenceSystem crs = layer->crs();

  QgsVectorFileWriter vWriter( shapefileName, dp->encoding(), layer->fields(), outputType, crs );
  QgsFeature currentFeature;
  QMultiMap<QString, QgsFeatureId> map;

  if ( onlySelectedFeatures )
  {
    //use QgsVectorLayer::featureAtId
    const QgsFeatureIds selection = layer->selectedFeaturesIds();
    QgsFeatureIds::const_iterator it = selection.constBegin();
    for ( ; it != selection.constEnd(); ++it )
    {
      if ( !layer->getFeatures( QgsFeatureRequest().setFilterFid( *it ) ).nextFeature( currentFeature ) )
      {
        continue;
      }
      map.insert( currentFeature.attribute( uniqueIdField ).toString(), currentFeature.id() );
    }
  }
  else
  {
    QgsFeatureIterator fit = layer->getFeatures();
    while ( fit.nextFeature( currentFeature ) )
    {
      map.insert( currentFeature.attribute( uniqueIdField ).toString(), currentFeature.id() );
    }
  }

  QgsGeometry dissolveGeometry; //dissolve geometry
  QMultiMap<QString, QgsFeatureId>::const_iterator jt = map.constBegin();
  QgsFeature outputFeature;
  while ( jt != map.constEnd() )
  {
    QString currentKey = jt.key();
    int processedFeatures = 0;
    bool first = true;
    //take only selection
    if ( onlySelectedFeatures )
    {
      //use QgsVectorLayer::featureAtId
      const QgsFeatureIds selection = layer->selectedFeaturesIds();
      if ( p )
      {
        p->setMaximum( selection.size() );
      }
      while ( jt != map.constEnd() && ( jt.key() == currentKey || !useField ) )
      {
        if ( p && p->wasCanceled() )
        {
          break;
        }
        if ( selection.contains( jt.value() ) )
        {
          if ( p )
          {
            p->setValue( processedFeatures );
          }
          if ( !layer->getFeatures( QgsFeatureRequest().setFilterFid( jt.value() ) ).nextFeature( currentFeature ) )
          {
            continue;
          }
          if ( first )
          {
            outputFeature.setAttributes( currentFeature.attributes() );
            first = false;
          }
          dissolveGeometry = dissolveFeature( currentFeature, dissolveGeometry );
          ++processedFeatures;
        }
        ++jt;
      }
    }
    //take all features
    else
    {
      int featureCount = layer->featureCount();
      if ( p )
      {
        p->setMaximum( featureCount );
      }
      while ( jt != map.constEnd() && ( jt.key() == currentKey || !useField ) )
      {
        if ( p )
        {
          p->setValue( processedFeatures );
        }

        if ( p && p->wasCanceled() )
        {
          break;
        }
        if ( !layer->getFeatures( QgsFeatureRequest().setFilterFid( jt.value() ) ).nextFeature( currentFeature ) )
        {
          continue;
        }
        {
          outputFeature.setAttributes( currentFeature.attributes() );
          first = false;
        }
        dissolveGeometry = dissolveFeature( currentFeature, dissolveGeometry );
        ++processedFeatures;
        ++jt;
      }
    }
    outputFeature.setGeometry( dissolveGeometry );
    vWriter.addFeature( outputFeature );
  }
  return true;
}

QgsGeometry QgsGeometryAnalyzer::dissolveFeature( const QgsFeature& f, const QgsGeometry& dissolveInto )
{
  if ( !f.hasGeometry() )
  {
    return dissolveInto;
  }

  QgsGeometry featureGeometry = f.geometry();

  if ( dissolveInto.isEmpty() )
  {
    return featureGeometry;
  }
  else
  {
    return dissolveInto.combine( featureGeometry );
  }
}

bool QgsGeometryAnalyzer::buffer( QgsVectorLayer* layer, const QString& shapefileName, double bufferDistance,
                                  bool onlySelectedFeatures, bool dissolve, int bufferDistanceField, QProgressDialog* p )
{
  if ( !layer )
  {
    return false;
  }

  QgsVectorDataProvider* dp = layer->dataProvider();
  if ( !dp )
  {
    return false;
  }

  QgsWkbTypes::Type outputType = QgsWkbTypes::Polygon;
  if ( dissolve )
  {
    outputType = QgsWkbTypes::MultiPolygon;
  }
  QgsCoordinateReferenceSystem crs = layer->crs();

  QgsVectorFileWriter vWriter( shapefileName, dp->encoding(), layer->fields(), outputType, crs );
  QgsFeature currentFeature;
  QgsGeometry dissolveGeometry; //dissolve geometry (if dissolve enabled)

  //take only selection
  if ( onlySelectedFeatures )
  {
    //use QgsVectorLayer::featureAtId
    const QgsFeatureIds selection = layer->selectedFeaturesIds();
    if ( p )
    {
      p->setMaximum( selection.size() );
    }

    int processedFeatures = 0;
    QgsFeatureIds::const_iterator it = selection.constBegin();
    for ( ; it != selection.constEnd(); ++it )
    {
      if ( p )
      {
        p->setValue( processedFeatures );
      }

      if ( p && p->wasCanceled() )
      {
        break;
      }
      if ( !layer->getFeatures( QgsFeatureRequest().setFilterFid( *it ) ).nextFeature( currentFeature ) )
      {
        continue;
      }
      bufferFeature( currentFeature, processedFeatures, &vWriter, dissolve, dissolveGeometry, bufferDistance, bufferDistanceField );
      ++processedFeatures;
    }

    if ( p )
    {
      p->setValue( selection.size() );
    }
  }
  //take all features
  else
  {
    QgsFeatureIterator fit = layer->getFeatures();

    int featureCount = layer->featureCount();
    if ( p )
    {
      p->setMaximum( featureCount );
    }
    int processedFeatures = 0;

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
      bufferFeature( currentFeature, processedFeatures, &vWriter, dissolve, dissolveGeometry, bufferDistance, bufferDistanceField );
      ++processedFeatures;
    }
    if ( p )
    {
      p->setValue( featureCount );
    }
  }

  if ( dissolve )
  {
    QgsFeature dissolveFeature;
    if ( dissolveGeometry.isEmpty() )
    {
      QgsDebugMsg( "no dissolved geometry - should not happen" );
      return false;
    }
    dissolveFeature.setGeometry( dissolveGeometry );
    vWriter.addFeature( dissolveFeature );
  }
  return true;
}

void QgsGeometryAnalyzer::bufferFeature( QgsFeature& f, int nProcessedFeatures, QgsVectorFileWriter* vfw, bool dissolve,
    QgsGeometry& dissolveGeometry, double bufferDistance, int bufferDistanceField )
{
  if ( !f.hasGeometry() )
  {
    return;
  }

  double currentBufferDistance;
  QgsGeometry featureGeometry = f.geometry();
  QgsGeometry bufferGeometry;

  //create buffer
  if ( bufferDistanceField == -1 )
  {
    currentBufferDistance = bufferDistance;
  }
  else
  {
    currentBufferDistance = f.attribute( bufferDistanceField ).toDouble();
  }
  bufferGeometry = featureGeometry.buffer( currentBufferDistance, 5 );

  if ( dissolve )
  {
    if ( nProcessedFeatures == 0 )
    {
      dissolveGeometry = bufferGeometry;
    }
    else
    {
      dissolveGeometry = dissolveGeometry.combine( bufferGeometry );
    }
  }
  else //dissolve
  {
    QgsFeature newFeature;
    newFeature.setGeometry( bufferGeometry );
    newFeature.setAttributes( f.attributes() );

    //add it to vector file writer
    if ( vfw )
    {
      vfw->addFeature( newFeature );
    }
  }
}

bool QgsGeometryAnalyzer::eventLayer( QgsVectorLayer* lineLayer, QgsVectorLayer* eventLayer, int lineField, int eventField, QgsFeatureIds &unlocatedFeatureIds, const QString& outputLayer,
                                      const QString& outputFormat, int locationField1, int locationField2, int offsetField, double offsetScale,
                                      bool forceSingleGeometry, QgsVectorDataProvider* memoryProvider, QProgressDialog* p )
{
  if ( !lineLayer || !eventLayer || !lineLayer->isValid() || !eventLayer->isValid() )
  {
    return false;
  }

  //create line field / id map for line layer
  QMultiHash< QString, QgsFeature > lineLayerIdMap; //1:n possible (e.g. several linear reference geometries for one feature in the event layer)
  QgsFeatureIterator fit = lineLayer->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() << lineField ) );
  QgsFeature fet;
  while ( fit.nextFeature( fet ) )
  {
    lineLayerIdMap.insert( fet.attribute( lineField ).toString(), fet );
  }

  //create output datasource or attributes in memory provider
  QgsVectorFileWriter* fileWriter = nullptr;
  QgsFeatureList memoryProviderFeatures;
  if ( !memoryProvider )
  {
    QgsWkbTypes::Type memoryProviderType = QgsWkbTypes::MultiLineString;
    if ( locationField2 == -1 )
    {
      memoryProviderType = forceSingleGeometry ? QgsWkbTypes::Point : QgsWkbTypes::MultiPoint;
    }
    else
    {
      memoryProviderType = forceSingleGeometry ? QgsWkbTypes::LineString : QgsWkbTypes::MultiLineString;
    }
    fileWriter = new QgsVectorFileWriter( outputLayer,
                                          eventLayer->dataProvider()->encoding(),
                                          eventLayer->fields(),
                                          memoryProviderType,
                                          lineLayer->crs(),
                                          outputFormat );
  }
  else
  {
    memoryProvider->addAttributes( eventLayer->fields().toList() );
  }

  //iterate over eventLayer and write new features to output file or layer
  fit = eventLayer->getFeatures( QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ) );
  QgsGeometry lrsGeom;
  double measure1, measure2 = 0.0;

  int nEventFeatures = eventLayer->featureCount();
  int featureCounter = 0;
  int nOutputFeatures = 0; //number of output features for the current event feature
  if ( p )
  {
    p->setWindowModality( Qt::WindowModal );
    p->setMinimum( 0 );
    p->setMaximum( nEventFeatures );
    p->show();
  }

  while ( fit.nextFeature( fet ) )
  {
    nOutputFeatures = 0;

    //update progress dialog
    if ( p )
    {
      if ( p->wasCanceled() )
      {
        break;
      }
      p->setValue( featureCounter );
      ++featureCounter;
    }

    measure1 = fet.attribute( locationField1 ).toDouble();
    if ( locationField2 != -1 )
    {
      measure2 = fet.attribute( locationField2 ).toDouble();
      if ( qgsDoubleNear(( measure2 - measure1 ), 0.0 ) )
      {
        continue;
      }
    }

    QList<QgsFeature> featureIdList = lineLayerIdMap.values( fet.attribute( eventField ).toString() );
    QList<QgsFeature>::iterator featureIdIt = featureIdList.begin();
    for ( ; featureIdIt != featureIdList.end(); ++featureIdIt )
    {
      if ( locationField2 == -1 )
      {
        lrsGeom = locateAlongMeasure( measure1, featureIdIt->geometry() );
      }
      else
      {
        lrsGeom = locateBetweenMeasures( measure1, measure2, featureIdIt->geometry() );
      }

      if ( !lrsGeom.isEmpty() )
      {
        ++nOutputFeatures;
        addEventLayerFeature( fet, lrsGeom, featureIdIt->geometry(), fileWriter, memoryProviderFeatures, offsetField, offsetScale, forceSingleGeometry );
      }
    }
    if ( nOutputFeatures < 1 )
    {
      unlocatedFeatureIds.insert( fet.id() );
    }
  }

  if ( p )
  {
    p->setValue( nEventFeatures );
  }

  if ( memoryProvider )
  {
    memoryProvider->addFeatures( memoryProviderFeatures );
  }
  delete fileWriter;
  return true;
}

void QgsGeometryAnalyzer::addEventLayerFeature( QgsFeature& feature, const QgsGeometry& geom, const QgsGeometry& lineGeom, QgsVectorFileWriter* fileWriter, QgsFeatureList& memoryFeatures,
    int offsetField, double offsetScale, bool forceSingleType )
{
  if ( geom.isEmpty() )
  {
    return;
  }

  QList<QgsGeometry> geomList;
  if ( forceSingleType )
  {
    geomList = geom.asGeometryCollection();
  }
  else
  {
    geomList.push_back( geom );
  }

  QList<QgsGeometry>::iterator geomIt = geomList.begin();
  for ( ; geomIt != geomList.end(); ++geomIt )
  {
    //consider offset
    QgsGeometry newGeom = *geomIt;
    if ( offsetField >= 0 )
    {
      double offsetVal = feature.attribute( offsetField ).toDouble();
      offsetVal *= offsetScale;
      newGeom = createOffsetGeometry( *geomIt, lineGeom, offsetVal );
      if ( newGeom.isEmpty() )
      {
        continue;
      }
    }

    feature.setGeometry( newGeom );
    if ( fileWriter )
    {
      fileWriter->addFeature( feature );
    }
    else
    {
      memoryFeatures << feature;
    }
  }
}

QgsGeometry QgsGeometryAnalyzer::createOffsetGeometry( const QgsGeometry& geom, const QgsGeometry& lineGeom, double offset )
{
  if ( !geom || lineGeom.isEmpty() )
  {
    return QgsGeometry();
  }

  QList<QgsGeometry> inputGeomList;

  if ( geom.isMultipart() )
  {
    inputGeomList = geom.asGeometryCollection();
  }
  else
  {
    inputGeomList.push_back( geom );
  }

  QList<GEOSGeometry*> outputGeomList;
  QList<QgsGeometry>::const_iterator inputGeomIt = inputGeomList.constBegin();
  GEOSContextHandle_t geosctxt = QgsGeometry::getGEOSHandler();
  for ( ; inputGeomIt != inputGeomList.constEnd(); ++inputGeomIt )
  {
    if ( geom.type() == QgsWkbTypes::LineGeometry )
    {
      GEOSGeometry* offsetGeom = GEOSOffsetCurve_r( geosctxt, ( *inputGeomIt ).asGeos(), -offset, 8 /*quadSegments*/, 0 /*joinStyle*/, 5.0 /*mitreLimit*/ );
      if ( !offsetGeom || !GEOSisValid_r( geosctxt, offsetGeom ) )
      {
        return QgsGeometry();
      }
      if ( !GEOSisValid_r( geosctxt, offsetGeom ) || GEOSGeomTypeId_r( geosctxt, offsetGeom ) != GEOS_LINESTRING || GEOSGeomGetNumPoints_r( geosctxt, offsetGeom ) < 1 )
      {
        GEOSGeom_destroy_r( geosctxt, offsetGeom );
        return QgsGeometry();
      }
      outputGeomList.push_back( offsetGeom );
    }
    else if ( geom.type() == QgsWkbTypes::PointGeometry )
    {
      QgsPoint p = ( *inputGeomIt ).asPoint();
      p = createPointOffset( p.x(), p.y(), offset, lineGeom );
      GEOSCoordSequence* ptSeq = GEOSCoordSeq_create_r( geosctxt, 1, 2 );
      GEOSCoordSeq_setX_r( geosctxt, ptSeq, 0, p.x() );
      GEOSCoordSeq_setY_r( geosctxt, ptSeq, 0, p.y() );
      GEOSGeometry* geosPt = GEOSGeom_createPoint_r( geosctxt, ptSeq );
      outputGeomList.push_back( geosPt );
    }
  }

  QgsGeometry outGeometry;
  if ( !geom.isMultipart() )
  {
    GEOSGeometry* outputGeom = outputGeomList.at( 0 );
    if ( outputGeom )
    {
      outGeometry.fromGeos( outputGeom );
    }
  }
  else
  {
    GEOSGeometry** geomArray = new GEOSGeometry*[outputGeomList.size()];
    for ( int i = 0; i < outputGeomList.size(); ++i )
    {
      geomArray[i] = outputGeomList.at( i );
    }
    GEOSGeometry* collection = nullptr;
    if ( geom.type() == QgsWkbTypes::PointGeometry )
    {
      collection = GEOSGeom_createCollection_r( geosctxt, GEOS_MULTIPOINT, geomArray, outputGeomList.size() );
    }
    else if ( geom.type() == QgsWkbTypes::LineGeometry )
    {
      collection = GEOSGeom_createCollection_r( geosctxt, GEOS_MULTILINESTRING, geomArray, outputGeomList.size() );
    }
    outGeometry.fromGeos( collection );
    delete[] geomArray;
  }
  return outGeometry;
}

QgsPoint QgsGeometryAnalyzer::createPointOffset( double x, double y, double dist, const QgsGeometry& lineGeom ) const
{
  QgsPoint p( x, y );
  QgsPoint minDistPoint;
  int afterVertexNr;
  lineGeom.closestSegmentWithContext( p, minDistPoint, afterVertexNr );

  int beforeVertexNr = afterVertexNr - 1;
  QgsPoint beforeVertex = lineGeom.vertexAt( beforeVertexNr );
  QgsPoint afterVertex = lineGeom.vertexAt( afterVertexNr );

  //get normal vector
  double dx = afterVertex.x() - beforeVertex.x();
  double dy = afterVertex.y() - beforeVertex.y();
  double normalX = -dy;
  double normalY = dx;
  double normalLength = sqrt( normalX * normalX + normalY * normalY );
  normalX *= ( dist / normalLength );
  normalY *= ( dist / normalLength );

  double debugLength = sqrt( normalX * normalX + normalY * normalY ); //control
  Q_UNUSED( debugLength );
  return QgsPoint( x - normalX, y - normalY ); //negative values -> left side, positive values -> right side
}

QgsGeometry QgsGeometryAnalyzer::locateBetweenMeasures( double fromMeasure, double toMeasure, const QgsGeometry& lineGeom )
{
  if ( lineGeom.isEmpty() )
  {
    return QgsGeometry();
  }

  QgsMultiPolyline resultGeom;

  //need to go with WKB and z coordinate until QgsGeometry supports M values
  QgsConstWkbPtr wkbPtr( lineGeom.asWkb(), lineGeom.wkbSize() );
  wkbPtr.readHeader();

  QgsWkbTypes::Type wkbType = lineGeom.wkbType();
  if ( wkbType != QgsWkbTypes::LineString25D && wkbType != QgsWkbTypes::MultiLineString25D )
  {
    return QgsGeometry();
  }

  if ( wkbType == QgsWkbTypes::LineString25D )
  {
    locateBetweenWkbString( wkbPtr, resultGeom, fromMeasure, toMeasure );
  }
  else if ( wkbType == QgsWkbTypes::MultiLineString25D )
  {
    int nLines;
    wkbPtr >> nLines;
    for ( int i = 0; i < nLines; ++i )
    {
      wkbPtr.readHeader();
      wkbPtr = locateBetweenWkbString( wkbPtr, resultGeom, fromMeasure, toMeasure );
    }
  }

  if ( resultGeom.size() < 1 )
  {
    return QgsGeometry();
  }
  return QgsGeometry::fromMultiPolyline( resultGeom );
}

QgsGeometry QgsGeometryAnalyzer::locateAlongMeasure( double measure, const QgsGeometry& lineGeom )
{
  if ( lineGeom.isEmpty() )
  {
    return QgsGeometry();
  }

  QgsMultiPoint resultGeom;

  //need to go with WKB and z coordinate until QgsGeometry supports M values
  QgsConstWkbPtr wkbPtr( lineGeom.asWkb(), lineGeom.wkbSize() );
  QgsWkbTypes::Type wkbType = lineGeom.wkbType();

  if ( wkbType != QgsWkbTypes::LineString25D && wkbType != QgsWkbTypes::MultiLineString25D )
  {
    return QgsGeometry();
  }

  if ( wkbType == QgsWkbTypes::LineString25D )
  {
    locateAlongWkbString( wkbPtr, resultGeom, measure );
  }
  else if ( wkbType == QgsWkbTypes::MultiLineString25D )
  {
    int nLines;
    wkbPtr >> nLines;
    for ( int i = 0; i < nLines; ++i )
    {
      wkbPtr.readHeader();
      wkbPtr = locateAlongWkbString( wkbPtr, resultGeom, measure );
    }
  }

  if ( resultGeom.size() < 1 )
  {
    return QgsGeometry();
  }

  return QgsGeometry::fromMultiPoint( resultGeom );
}

QgsConstWkbPtr QgsGeometryAnalyzer::locateBetweenWkbString( QgsConstWkbPtr wkbPtr, QgsMultiPolyline& result, double fromMeasure, double toMeasure )
{
  int nPoints;
  wkbPtr >> nPoints;

  QgsPolyline currentLine;
  double prevx = 0.0, prevy = 0.0, prevz = 0.0;
  for ( int i = 0; i < nPoints; ++i )
  {
    double x, y, z;
    wkbPtr >> x >> y >> z;

    if ( i > 0 )
    {
      QgsPoint pt1, pt2;
      bool secondPointClipped; //true if second point is != segment endpoint
      bool measureInSegment = clipSegmentByRange( prevx, prevy, prevz, x, y, z, fromMeasure, toMeasure, pt1, pt2, secondPointClipped );
      if ( measureInSegment )
      {
        if ( currentLine.size() < 1 ) //no points collected yet, so the first point needs to be added to the line
        {
          currentLine.append( pt1 );
        }

        if ( pt1 != pt2 ) //avoid duplicated entry if measure value equals m-value of vertex
        {
          currentLine.append( pt2 );
        }

        if ( secondPointClipped || i == nPoints - 1 ) //close current segment
        {
          if ( currentLine.size() > 1 )
          {
            result.append( currentLine );
          }
          currentLine.clear();
        }
      }
    }
    prevx = x;
    prevy = y;
    prevz = z;
  }

  return wkbPtr;
}

QgsConstWkbPtr QgsGeometryAnalyzer::locateAlongWkbString( QgsConstWkbPtr wkbPtr, QgsMultiPoint& result, double measure )
{
  int nPoints;
  wkbPtr >> nPoints;

  double x, y, z;
  double prevx = 0.0, prevy = 0.0, prevz = 0.0;
  for ( int i = 0; i < nPoints; ++i )
  {
    wkbPtr >> x >> y >> z;

    if ( i > 0 )
    {
      QgsPoint pt1, pt2;
      bool pt1Ok, pt2Ok;
      locateAlongSegment( prevx, prevy, prevz, x, y, z, measure, pt1Ok, pt1, pt2Ok, pt2 );
      if ( pt1Ok )
      {
        result.append( pt1 );
      }
      if ( pt2Ok && i == nPoints - 1 )
      {
        result.append( pt2 );
      }
    }
    prevx = x;
    prevy = y;
    prevz = z;
  }

  return wkbPtr;
}

bool QgsGeometryAnalyzer::clipSegmentByRange( double x1, double y1, double m1, double x2, double y2, double m2, double range1, double range2, QgsPoint& pt1,
    QgsPoint& pt2, bool& secondPointClipped )
{
  bool reversed = m1 > m2;
  double tmp;

  //reverse m1, m2 if necessary (and consequently also x1,x2 / y1, y2)
  if ( reversed )
  {
    tmp = m1;
    m1 = m2;
    m2 = tmp;

    tmp = x1;
    x1 = x2;
    x2 = tmp;

    tmp = y1;
    y1 = y2;
    y2 = tmp;
  }

  //reverse range1, range2 if necessary
  if ( range1 > range2 )
  {
    tmp = range1;
    range1 = range2;
    range2 = tmp;
  }

  //segment completely outside of range
  if ( m2 < range1 || m1 > range2 )
  {
    return false;
  }

  //segment completely inside of range
  if ( m2 <= range2 && m1 >= range1 )
  {
    if ( reversed )
    {
      pt1.setX( x2 );
      pt1.setY( y2 );
      pt2.setX( x1 );
      pt2.setY( y1 );
    }
    else
    {
      pt1.setX( x1 );
      pt1.setY( y1 );
      pt2.setX( x2 );
      pt2.setY( y2 );
    }
    secondPointClipped = false;
    return true;
  }

  //m1 inside and m2 not
  if ( m1 >= range1 && m1 <= range2 )
  {
    pt1.setX( x1 );
    pt1.setY( y1 );
    double dist = ( range2 - m1 ) / ( m2 - m1 );
    pt2.setX( x1 + ( x2 - x1 ) * dist );
    pt2.setY( y1 + ( y2 - y1 ) * dist );
    secondPointClipped = !reversed;
  }

  //m2 inside and m1 not
  if ( m2 >= range1 && m2 <= range2 )
  {
    pt2.setX( x2 );
    pt2.setY( y2 );
    double dist = ( m2 - range1 ) / ( m2 - m1 );
    pt1.setX( x2 - ( x2 - x1 ) * dist );
    pt1.setY( y2 - ( y2 - y1 ) * dist );
    secondPointClipped = reversed;
  }

  //range1 and range 2 both inside the segment
  if ( range1 >= m1 && range2 <= m2 )
  {
    double dist1 = ( range1 - m1 ) / ( m2 - m1 );
    double dist2 = ( range2 - m1 ) / ( m2 - m1 );
    pt1.setX( x1 + ( x2 - x1 ) * dist1 );
    pt1.setY( y1 + ( y2 - y1 ) * dist1 );
    pt2.setX( x1 + ( x2 - x1 ) * dist2 );
    pt2.setY( y1 + ( y2 - y1 ) * dist2 );
    secondPointClipped = true;
  }

  if ( reversed ) //switch p1 and p2
  {
    QgsPoint tmpPt = pt1;
    pt1 = pt2;
    pt2 = tmpPt;
  }

  return true;
}

void QgsGeometryAnalyzer::locateAlongSegment( double x1, double y1, double m1, double x2, double y2, double m2, double measure, bool& pt1Ok, QgsPoint& pt1, bool& pt2Ok, QgsPoint& pt2 )
{
  bool reversed = false;
  pt1Ok = false;
  pt2Ok = false;
  double tolerance = 0.000001; //work with a small tolerance to catch e.g. locations at endpoints

  if ( m1 > m2 )
  {
    double tmp = m1;
    m1 = m2;
    m2 = tmp;
    reversed = true;
  }

  //segment does not match
  if (( m1 - measure ) > tolerance || ( measure - m2 ) > tolerance )
  {
    pt1Ok = false;
    pt2Ok = false;
    return;
  }

  //match with vertex1
  if ( qgsDoubleNear( m1, measure, tolerance ) )
  {
    if ( reversed )
    {
      pt2Ok = true;
      pt2.setX( x2 );
      pt2.setY( y2 );
    }
    else
    {
      pt1Ok = true;
      pt1.setX( x1 );
      pt1.setY( y1 );
    }
  }

  //match with vertex2
  if ( qgsDoubleNear( m2, measure, tolerance ) )
  {
    if ( reversed )
    {
      pt1Ok = true;
      pt1.setX( x1 );
      pt1.setY( y1 );
    }
    else
    {
      pt2Ok = true;
      pt2.setX( x2 );
      pt2.setY( y2 );
    }
  }


  if ( pt1Ok || pt2Ok )
  {
    return;
  }

  //match between the vertices
  if ( qgsDoubleNear( m1, m2 ) )
  {
    pt1.setX( x1 );
    pt1.setY( y1 );
    pt1Ok = true;
    return;
  }
  double dist = ( measure - m1 ) / ( m2 - m1 );
  if ( reversed )
  {
    dist = 1 - dist;
  }

  pt1.setX( x1 + dist * ( x2 - x1 ) );
  pt1.setY( y1 + dist * ( y2 - y1 ) );
  pt1Ok = true;
}
