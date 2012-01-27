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
#include "qgsfield.h"
#include "qgsfeature.h"
#include "qgslogger.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectordataprovider.h"
#include "qgsdistancearea.h"
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

  QGis::WkbType outputType = dp->geometryType();
  const QgsCoordinateReferenceSystem crs = layer->crs();

  QgsVectorFileWriter vWriter( shapefileName, dp->encoding(), dp->fields(), outputType, &crs );
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
      if ( !layer->featureAtId( *it, currentFeature, true, true ) )
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
    layer->select( layer->pendingAllAttributesList(), QgsRectangle(), true, false );


    int featureCount = layer->featureCount();
    if ( p )
    {
      p->setMaximum( featureCount );
    }
    int processedFeatures = 0;

    while ( layer->nextFeature( currentFeature ) )
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
  QgsGeometry* featureGeometry = f.geometry();
  QgsGeometry* tmpGeometry = 0;

  if ( !featureGeometry )
  {
    return;
  }
  // simplify feature
  tmpGeometry = featureGeometry->simplify( tolerance );

  QgsFeature newFeature;
  newFeature.setGeometry( tmpGeometry );
  newFeature.setAttributeMap( f.attributeMap() );

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

  QGis::WkbType outputType = QGis::WKBPoint;
  const QgsCoordinateReferenceSystem crs = layer->crs();

  QgsVectorFileWriter vWriter( shapefileName, dp->encoding(), dp->fields(), outputType, &crs );
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
      if ( !layer->featureAtId( *it, currentFeature, true, true ) )
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
    layer->select( layer->pendingAllAttributesList(), QgsRectangle(), true, false );

    int featureCount = layer->featureCount();
    if ( p )
    {
      p->setMaximum( featureCount );
    }
    int processedFeatures = 0;

    while ( layer->nextFeature( currentFeature ) )
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
  QgsGeometry* featureGeometry = f.geometry();
  QgsGeometry* tmpGeometry = 0;

  if ( !featureGeometry )
  {
    return;
  }

  tmpGeometry = featureGeometry->centroid();

  QgsFeature newFeature;
  newFeature.setGeometry( tmpGeometry );
  newFeature.setAttributeMap( f.attributeMap() );

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

  QGis::WkbType outputType = QGis::WKBPolygon;
  const QgsCoordinateReferenceSystem crs = layer->crs();

  QgsFieldMap fields;
  fields.insert( 0 , QgsField( QString( "MINX" ), QVariant::Double ) );
  fields.insert( 1 , QgsField( QString( "MINY" ), QVariant::Double ) );
  fields.insert( 2 , QgsField( QString( "MAXX" ), QVariant::Double ) );
  fields.insert( 3 , QgsField( QString( "MAXY" ), QVariant::Double ) );
  fields.insert( 4 , QgsField( QString( "CNTX" ), QVariant::Double ) );
  fields.insert( 5 , QgsField( QString( "CNTY" ), QVariant::Double ) );
  fields.insert( 6 , QgsField( QString( "AREA" ), QVariant::Double ) );
  fields.insert( 7 , QgsField( QString( "PERIM" ), QVariant::Double ) );
  fields.insert( 8 , QgsField( QString( "HEIGHT" ), QVariant::Double ) );
  fields.insert( 9 , QgsField( QString( "WIDTH" ), QVariant::Double ) );

  QgsVectorFileWriter vWriter( shapefileName, dp->encoding(), fields, outputType, &crs );

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
  QgsAttributeMap map;
  map.insert( 0 , QVariant( minx ) );
  map.insert( 1 , QVariant( miny ) );
  map.insert( 2 , QVariant( maxx ) );
  map.insert( 3 , QVariant( maxy ) );
  map.insert( 4 , QVariant( cntx ) );
  map.insert( 5 , QVariant( cnty ) );
  map.insert( 6 , QVariant( area ) );
  map.insert( 7 , QVariant( perim ) );
  map.insert( 8 , QVariant( height ) );
  map.insert( 9 , QVariant( width ) );
  feat.setAttributeMap( map );
  feat.setGeometry( QgsGeometry::fromRect( rect ) );
  vWriter.addFeature( feat );
  return true;
}

QList<double> QgsGeometryAnalyzer::simpleMeasure( QgsGeometry* mpGeometry )
{
  QList<double> list;
  double perim;
  if ( mpGeometry->wkbType() == QGis::WKBPoint )
  {
    QgsPoint pt = mpGeometry->asPoint();
    list.append( pt.x() );
    list.append( pt.y() );
  }
  else
  {
    QgsDistanceArea measure;
    list.append( measure.measure( mpGeometry ) );
    if ( mpGeometry->type() == QGis::Polygon )
    {
      perim = perimeterMeasure( mpGeometry, measure );
      list.append( perim );
    }
  }
  return list;
}

double QgsGeometryAnalyzer::perimeterMeasure( QgsGeometry* geometry, QgsDistanceArea& measure )
{
  double value = 0.00;
  if ( geometry->isMultipart() )
  {
    QgsMultiPolygon poly = geometry->asMultiPolygon();
    QgsMultiPolygon::iterator it;
    QgsPolygon::iterator jt;
    for ( it = poly.begin(); it != poly.end(); ++it )
    {
      for ( jt = it->begin(); jt != it->end(); ++jt )
      {
        value = value + measure.measure( QgsGeometry::fromPolyline( *jt ) );
      }
    }
  }
  else
  {
    QgsPolygon::iterator jt;
    QgsPolygon poly = geometry->asPolygon();
    for ( jt = poly.begin(); jt != poly.end(); ++jt )
    {
      value = value + measure.measure( QgsGeometry::fromPolyline( *jt ) );
    }
  }
  return value;
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
  QgsFieldMap fields;
  fields.insert( 0 , QgsField( QString( "UID" ), QVariant::String ) );
  fields.insert( 1 , QgsField( QString( "AREA" ), QVariant::Double ) );
  fields.insert( 2 , QgsField( QString( "PERIM" ), QVariant::Double ) );

  QGis::WkbType outputType = QGis::WKBPolygon;
  const QgsCoordinateReferenceSystem crs = layer->crs();

  QgsVectorFileWriter vWriter( shapefileName, dp->encoding(), fields, outputType, &crs );
  QgsFeature currentFeature;
  QgsGeometry* dissolveGeometry = 0; //dissolve geometry
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
      if ( !layer->featureAtId( *it, currentFeature, true, true ) )
      {
        continue;
      }
      map.insert( currentFeature.attributeMap()[ uniqueIdField ].toString(), currentFeature.id() );
    }
  }
  else
  {
    layer->select( layer->pendingAllAttributesList(), QgsRectangle(), true, false );
    while ( layer->nextFeature( currentFeature ) )
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
      map.insert( currentFeature.attributeMap()[ uniqueIdField ].toString(), currentFeature.id() );
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
          if ( !layer->featureAtId( jt.value(), currentFeature, true, true ) )
          {
            continue;
          }
          convexFeature( currentFeature, processedFeatures, &dissolveGeometry );
          ++processedFeatures;
        }
        ++jt;
      }
      QList<double> values;
      if ( !dissolveGeometry )
      {
        QgsDebugMsg( "no dissolved geometry - should not happen" );
        return false;
      }
      dissolveGeometry = dissolveGeometry->convexHull();
      values = simpleMeasure( dissolveGeometry );
      QgsAttributeMap attributeMap;
      attributeMap.insert( 0 , QVariant( currentKey ) );
      attributeMap.insert( 1 , values[ 0 ] );
      attributeMap.insert( 2 , values[ 1 ] );
      QgsFeature dissolveFeature;
      dissolveFeature.setAttributeMap( attributeMap );
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
        if ( !layer->featureAtId( jt.value(), currentFeature, true, true ) )
        {
          continue;
        }
        convexFeature( currentFeature, processedFeatures, &dissolveGeometry );
        ++processedFeatures;
        ++jt;
      }
      QList<double> values;
      // QgsGeometry* tmpGeometry = 0;
      if ( !dissolveGeometry )
      {
        QgsDebugMsg( "no dissolved geometry - should not happen" );
        return false;
      }
      dissolveGeometry = dissolveGeometry->convexHull();
      // values = simpleMeasure( tmpGeometry );
      values = simpleMeasure( dissolveGeometry );
      QgsAttributeMap attributeMap;
      attributeMap.insert( 0 , QVariant( currentKey ) );
      attributeMap.insert( 1 , QVariant( values[ 0 ] ) );
      attributeMap.insert( 2 , QVariant( values[ 1 ] ) );
      QgsFeature dissolveFeature;
      dissolveFeature.setAttributeMap( attributeMap );
      dissolveFeature.setGeometry( dissolveGeometry );
      vWriter.addFeature( dissolveFeature );
    }
  }
  return true;
}


void QgsGeometryAnalyzer::convexFeature( QgsFeature& f, int nProcessedFeatures, QgsGeometry** dissolveGeometry )
{
  QgsGeometry* featureGeometry = f.geometry();
  QgsGeometry* tmpGeometry = 0;
  QgsGeometry* convexGeometry = 0;

  if ( !featureGeometry )
  {
    return;
  }

  convexGeometry = featureGeometry->convexHull();

  if ( nProcessedFeatures == 0 )
  {
    *dissolveGeometry = convexGeometry;
  }
  else
  {
    tmpGeometry = *dissolveGeometry;
    *dissolveGeometry = ( *dissolveGeometry )->combine( convexGeometry );
    delete tmpGeometry;
    delete convexGeometry;
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

  QGis::WkbType outputType = dp->geometryType();
  const QgsCoordinateReferenceSystem crs = layer->crs();

  QgsVectorFileWriter vWriter( shapefileName, dp->encoding(), dp->fields(), outputType, &crs );
  QgsFeature currentFeature;
  QMultiMap<QString, QgsFeatureId> map;

  if ( onlySelectedFeatures )
  {
    //use QgsVectorLayer::featureAtId
    const QgsFeatureIds selection = layer->selectedFeaturesIds();
    QgsFeatureIds::const_iterator it = selection.constBegin();
    for ( ; it != selection.constEnd(); ++it )
    {
      if ( !layer->featureAtId( *it, currentFeature, true, true ) )
      {
        continue;
      }
      map.insert( currentFeature.attributeMap()[ uniqueIdField ].toString(), currentFeature.id() );
    }
  }
  else
  {
    layer->select( layer->pendingAllAttributesList(), QgsRectangle(), true, false );
    while ( layer->nextFeature( currentFeature ) )
    {
      map.insert( currentFeature.attributeMap()[ uniqueIdField ].toString(), currentFeature.id() );
    }
  }

  QgsGeometry *dissolveGeometry = 0; //dissolve geometry
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
          if ( !layer->featureAtId( jt.value(), currentFeature, true, true ) )
          {
            continue;
          }
          if ( first )
          {
            outputFeature.setAttributeMap( currentFeature.attributeMap() );
            first = false;
          }
          dissolveFeature( currentFeature, processedFeatures, &dissolveGeometry );
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
        if ( !layer->featureAtId( jt.value(), currentFeature, true, true ) )
        {
          continue;
        }
        {
          outputFeature.setAttributeMap( currentFeature.attributeMap() );
          first = false;
        }
        dissolveFeature( currentFeature, processedFeatures, &dissolveGeometry );
        ++processedFeatures;
        ++jt;
      }
    }
    outputFeature.setGeometry( dissolveGeometry );
    vWriter.addFeature( outputFeature );
  }
  return true;
}

void QgsGeometryAnalyzer::dissolveFeature( QgsFeature& f, int nProcessedFeatures, QgsGeometry** dissolveGeometry )
{
  QgsGeometry* featureGeometry = f.geometry();

  if ( !featureGeometry )
  {
    return;
  }

  if ( nProcessedFeatures == 0 )
  {
    int geomSize = featureGeometry->wkbSize();
    *dissolveGeometry = new QgsGeometry();
    unsigned char* wkb = new unsigned char[geomSize];
    memcpy( wkb, featureGeometry->asWkb(), geomSize );
    ( *dissolveGeometry )->fromWkb( wkb, geomSize );
  }
  else
  {
    *dissolveGeometry = ( *dissolveGeometry )->combine( featureGeometry );
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

  QGis::WkbType outputType = QGis::WKBPolygon;
  if ( dissolve )
  {
    outputType = QGis::WKBMultiPolygon;
  }
  const QgsCoordinateReferenceSystem crs = layer->crs();

  QgsVectorFileWriter vWriter( shapefileName, dp->encoding(), dp->fields(), outputType, &crs );
  QgsFeature currentFeature;
  QgsGeometry *dissolveGeometry = 0; //dissolve geometry (if dissolve enabled)

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
      if ( !layer->featureAtId( *it, currentFeature, true, true ) )
      {
        continue;
      }
      bufferFeature( currentFeature, processedFeatures, &vWriter, dissolve, &dissolveGeometry, bufferDistance, bufferDistanceField );
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
    layer->select( layer->pendingAllAttributesList(), QgsRectangle(), true, false );


    int featureCount = layer->featureCount();
    if ( p )
    {
      p->setMaximum( featureCount );
    }
    int processedFeatures = 0;

    while ( layer->nextFeature( currentFeature ) )
    {
      if ( p )
      {
        p->setValue( processedFeatures );
      }
      if ( p && p->wasCanceled() )
      {
        break;
      }
      bufferFeature( currentFeature, processedFeatures, &vWriter, dissolve, &dissolveGeometry, bufferDistance, bufferDistanceField );
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
    if ( !dissolveGeometry )
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
    QgsGeometry** dissolveGeometry, double bufferDistance, int bufferDistanceField )
{
  double currentBufferDistance;
  QgsGeometry* featureGeometry = f.geometry();
  QgsGeometry* tmpGeometry = 0;
  QgsGeometry* bufferGeometry = 0;

  if ( !featureGeometry )
  {
    return;
  }

  //create buffer
  if ( bufferDistanceField == -1 )
  {
    currentBufferDistance = bufferDistance;
  }
  else
  {
    currentBufferDistance = f.attributeMap()[bufferDistanceField].toDouble();
  }
  bufferGeometry = featureGeometry->buffer( currentBufferDistance, 5 );

  if ( dissolve )
  {
    if ( nProcessedFeatures == 0 )
    {
      *dissolveGeometry = bufferGeometry;
    }
    else
    {
      tmpGeometry = *dissolveGeometry;
      *dissolveGeometry = ( *dissolveGeometry )->combine( bufferGeometry );
      delete tmpGeometry;
      delete bufferGeometry;
    }
  }
  else //dissolve
  {
    QgsFeature newFeature;
    newFeature.setGeometry( bufferGeometry );
    newFeature.setAttributeMap( f.attributeMap() );

    //add it to vector file writer
    if ( vfw )
    {
      vfw->addFeature( newFeature );
    }
  }
}

bool QgsGeometryAnalyzer::eventLayer( QgsVectorLayer* lineLayer, QgsVectorLayer* eventLayer, int lineField, int eventField, const QString& outputLayer,
                                      const QString& outputFormat, int locationField1, int locationField2, QgsVectorDataProvider* memoryProvider, QProgressDialog* p )
{
  if ( !lineLayer || !eventLayer || !lineLayer->isValid() || !eventLayer->isValid() )
  {
    return false;
  }

  //create line field / id map for line layer
  QHash< QString, QgsFeatureId > lineLayerIdMap;
  lineLayer->select( QgsAttributeList() << lineField,
                     QgsRectangle(), false, false );
  QgsFeature fet;
  while ( lineLayer->nextFeature( fet ) )
  {
    lineLayerIdMap.insert( fet.attributeMap()[lineField].toString(), fet.id() );
  }

  //create output datasource or attributes in memory provider
  QgsVectorFileWriter* fileWriter = 0;
  if ( !memoryProvider )
  {
    fileWriter = new QgsVectorFileWriter( outputLayer,
                                          eventLayer->dataProvider()->encoding(),
                                          eventLayer->pendingFields(),
                                          locationField2 == -1 ? QGis::WKBMultiPoint25D : QGis::WKBMultiLineString25D,
                                          &( lineLayer->crs() ),
                                          outputFormat );
  }
  else
  {
    memoryProvider->addAttributes( eventLayer->pendingFields().values() );
  }

  //iterate over eventLayer and write new features to output file or layer
  eventLayer->select( eventLayer->pendingAllAttributesList(), QgsRectangle(), true, false );
  QgsGeometry* lrsGeom = 0;
  QgsFeature lineFeature;
  QgsGeometry* lineGeom = 0;
  double measure1, measure2;

  while ( eventLayer->nextFeature( fet ) )
  {
    //get corresponding line feature
    QHash< QString, QgsFeatureId >::const_iterator layerIdIt = lineLayerIdMap.find( fet.attributeMap()[eventField].toString() );
    if ( layerIdIt == lineLayerIdMap.constEnd() )
    {
      continue;
    }
    if ( !lineLayer->featureAtId( *layerIdIt, lineFeature, true, false ) )
    {
      continue;
    }

    measure1 = fet.attributeMap()[locationField1].toDouble();
    if ( locationField2 == -1 )
    {
      lrsGeom = locateAlongMeasure( measure1, lineFeature.geometry() );
    }
    else
    {
      measure2 = fet.attributeMap()[locationField2].toDouble();
      lrsGeom = locateBetweenMeasures( measure1, measure2, lineFeature.geometry() );
    }

    if ( lrsGeom )
    {
      fet.setGeometry( lrsGeom );
      if ( memoryProvider )
      {
        memoryProvider->addFeatures( QgsFeatureList() << fet );
      }
      else
      {
        fileWriter->addFeature( fet );
      }
    }
  }

  return true;
}

QgsGeometry* QgsGeometryAnalyzer::locateBetweenMeasures( double fromMeasure, double toMeasure, QgsGeometry* lineGeom )
{
  if ( !lineGeom )
  {
    return 0;
  }

  QgsMultiPolyline resultGeom;

  //need to go with WKB and z coordinate until QgsGeometry supports M values
  unsigned char* lineWkb = lineGeom->asWkb();
  int wkbSize = lineGeom->wkbSize();

  unsigned char* ptr = lineWkb + 1;
  QGis::WkbType wkbType;
  memcpy( &wkbType, ptr, sizeof( wkbType ) );
  ptr += sizeof( wkbType );

  if ( wkbType != QGis::WKBLineString25D && wkbType != QGis::WKBMultiLineString25D )
  {
    return 0;
  }

  if ( wkbType == QGis::WKBLineString25D )
  {
    locateBetweenWkbString( ptr, resultGeom, fromMeasure, toMeasure );
  }
  else if ( wkbType == QGis::WKBMultiLineString25D )
  {
    int* nLines = ( int* )ptr;
    ptr += sizeof( int );
    for ( int i = 0; i < *nLines; ++i )
    {
      ptr += ( 1 + sizeof( wkbType ) );
      ptr = locateBetweenWkbString( ptr, resultGeom, fromMeasure, toMeasure );
    }
  }

  return QgsGeometry::fromMultiPolyline( resultGeom );
}

QgsGeometry* QgsGeometryAnalyzer::locateAlongMeasure( double measure, QgsGeometry* lineGeom )
{
  if ( !lineGeom )
  {
    return 0;
  }

  QgsMultiPoint resultGeom;

  //need to go with WKB and z coordinate until QgsGeometry supports M values
  unsigned char* lineWkb = lineGeom->asWkb();

  unsigned char* ptr = lineWkb + 1;
  QGis::WkbType wkbType;
  memcpy( &wkbType, ptr, sizeof( wkbType ) );
  ptr += sizeof( wkbType );

  if ( wkbType != QGis::WKBLineString25D && wkbType != QGis::WKBMultiLineString25D )
  {
    return 0;
  }

  if ( wkbType == QGis::WKBLineString25D )
  {
    locateAlongWkbString( ptr, resultGeom, measure );
  }
  else if ( wkbType == QGis::WKBMultiLineString25D )
  {
    int* nLines = ( int* )ptr;
    ptr += sizeof( int );
    for ( int i = 0; i < *nLines; ++i )
    {
      ptr += ( 1 + sizeof( wkbType ) );
      ptr = locateAlongWkbString( ptr, resultGeom, measure );
    }
  }

  return QgsGeometry::fromMultiPoint( resultGeom );
}

unsigned char* QgsGeometryAnalyzer::locateBetweenWkbString( unsigned char* ptr, QgsMultiPolyline& result, double fromMeasure, double toMeasure )
{
  int* nPoints = ( int* ) ptr;
  ptr += sizeof( int );
  double prevx, prevy, prevz;
  double *x, *y, *z;
  QgsPolyline currentLine;

  QgsPoint pt1, pt2;
  bool measureInSegment; //true if measure is contained in the segment
  bool secondPointClipped; //true if second point is != segment endpoint


  for ( int i = 0; i < *nPoints; ++i )
  {
    x = ( double* )ptr;
    ptr += sizeof( double );
    y = ( double* )ptr;
    ptr += sizeof( double );
    z = ( double* ) ptr;
    ptr += sizeof( double );

    if ( i > 0 )
    {
      measureInSegment = clipSegmentByRange( prevx, prevy, prevz, *x, *y, *z, fromMeasure, toMeasure, pt1, pt2, secondPointClipped );
      if ( measureInSegment )
      {
        if ( currentLine.size() < 1 ) //no points collected yet, so the first point needs to be added to the line
        {
          currentLine.append( pt1 );
        }
        currentLine.append( pt2 );
        if ( secondPointClipped || i == *nPoints - 1 ) //close current segment
        {
          result.append( currentLine );
          currentLine.clear();
        }
      }
    }
    prevx = *x; prevy = *y; prevz = *z;
  }
  return ptr;
}

unsigned char* QgsGeometryAnalyzer::locateAlongWkbString( unsigned char* ptr, QgsMultiPoint& result, double measure )
{
  int* nPoints = ( int* ) ptr;
  ptr += sizeof( int );
  double prevx, prevy, prevz;
  double *x, *y, *z;

  QgsPoint pt1, pt2;
  bool pt1Ok, pt2Ok;

  for ( int i = 0; i < *nPoints; ++i )
  {
    x = ( double* )ptr;
    ptr += sizeof( double );
    y = ( double* )ptr;
    ptr += sizeof( double );
    z = ( double* ) ptr;
    ptr += sizeof( double );

    if ( i > 0 )
    {
      locateAlongSegment( prevx, prevy, prevz, *x, *y, *z, measure, pt1Ok, pt1, pt2Ok, pt2 );
      if ( pt1Ok )
      {
        result.append( pt1 );
      }
      if ( pt2Ok && ( i == ( *nPoints - 1 ) ) )
      {
        result.append( pt2 );
      }
    }
    prevx = *x; prevy = *y; prevz = *z;
  }
  return ptr;
}

bool QgsGeometryAnalyzer::clipSegmentByRange( double x1, double y1, double m1, double x2, double y2, double m2, double range1, double range2, QgsPoint& pt1,
    QgsPoint& pt2, bool& secondPointClipped )
{
  bool reversed = m1 > m2;
  double tmp;

  //reverse m1, m2 if necessary
  if ( reversed )
  {
    tmp = m1;
    m1 = m2;
    m2 = tmp;
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
    pt1.setX( x1 ); pt1.setY( y1 );
    pt2.setX( x2 ); pt2.setY( y2 );
    secondPointClipped = false;
  }

  //m1 inside and m2 not
  if ( m1 >= range1 && m1 <= range2 )
  {
    pt1.setX( x1 ); pt1.setY( y1 );
    double dist = ( range2 - m1 ) / ( m2 - m1 );
    pt2.setX( x1 + ( x2 - x1 ) * dist );
    pt2.setY( y1 + ( y2 - y1 ) * dist );
    secondPointClipped = !reversed;
  }

  //m2 inside and m1 not
  if ( m2 >= range1 && m2 <= range2 )
  {
    pt2.setX( x2 ); pt2.setY( y2 );
    double dist = ( m2 - range1 ) / ( m2 - m1 );
    pt1.setX( x2 - ( x2 - x1 ) * dist );
    pt1.setY( y2 - ( y2 - y1 ) * dist );
    secondPointClipped = reversed;
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

  if ( m1 > m2 )
  {
    double tmp = m1;
    m1 = m2;
    m2 = tmp;
    reversed = true;
  }

  //segment does not match
  if ( measure < m1 || measure > m2 )
  {
    pt1Ok = false;
    pt2Ok = false;
    return;
  }

  //match with vertex1
  if ( doubleNear( m1, measure ) )
  {
    if ( reversed )
    {
      pt2Ok = true;
      pt2.setX( x2 ); pt2.setY( y2 );
    }
    else
    {
      pt1Ok = true;
      pt1.setX( x1 ); pt1.setY( y1 );
    }
  }

  //match with vertex2
  if ( doubleNear( m2, measure ) )
  {
    if ( reversed )
    {
      pt1Ok = true;
      pt1.setX( x1 ); pt1.setY( y1 );
    }
    else
    {
      pt2Ok = true;
      pt2.setX( x2 ); pt2.setY( y2 );
    }
  }


  if ( pt1Ok || pt2Ok )
  {
    return;
  }

  //match between the vertices
  if ( doubleNear( m1, m2 ) )
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
