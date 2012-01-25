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

QgsGeometry* QgsGeometryAnalyzer::locateAlongMeasure( double measure, const QgsGeometry* lineGeom )
{
  //only for testing
  QgsGeometry* geom = new QgsGeometry();
  *geom = *lineGeom;
  geom->convertToMultiType();
  return geom;
}

QgsGeometry* QgsGeometryAnalyzer::locateBetweenMeasures( double fromMeasure, double toMeasure, const QgsGeometry* lineGeom )
{
  //only for testing
  QgsGeometry* geom = new QgsGeometry();
  *geom = *lineGeom;
  geom->convertToMultiType();
  return geom;
}
