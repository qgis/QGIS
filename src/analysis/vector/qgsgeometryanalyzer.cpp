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

  QgsVectorFileWriter vWriter( shapefileName, dp->encoding(), layer->pendingFields(), outputType, &crs );
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

  QGis::WkbType outputType = QGis::WKBPoint;
  const QgsCoordinateReferenceSystem crs = layer->crs();

  QgsVectorFileWriter vWriter( shapefileName, dp->encoding(), layer->pendingFields(), outputType, &crs );
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
  QgsGeometry* featureGeometry = f.geometry();
  QgsGeometry* tmpGeometry = 0;

  if ( !featureGeometry )
  {
    return;
  }

  tmpGeometry = featureGeometry->centroid();

  QgsFeature newFeature;
  newFeature.setGeometry( tmpGeometry );
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

  QGis::WkbType outputType = QGis::WKBPolygon;
  const QgsCoordinateReferenceSystem crs = layer->crs();

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
  QgsFields fields;
  fields.append( QgsField( QString( "UID" ), QVariant::String ) );
  fields.append( QgsField( QString( "AREA" ), QVariant::Double ) );
  fields.append( QgsField( QString( "PERIM" ), QVariant::Double ) );

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
      QgsAttributes attributes( 3 );
      attributes[0] = QVariant( currentKey );
      attributes[1] = values[ 0 ];
      attributes[2] = values[ 1 ];
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

  QgsVectorFileWriter vWriter( shapefileName, dp->encoding(), layer->pendingFields(), outputType, &crs );
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
          if ( !layer->getFeatures( QgsFeatureRequest().setFilterFid( jt.value() ) ).nextFeature( currentFeature ) )
          {
            continue;
          }
          if ( first )
          {
            outputFeature.setAttributes( currentFeature.attributes() );
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
        if ( !layer->getFeatures( QgsFeatureRequest().setFilterFid( jt.value() ) ).nextFeature( currentFeature ) )
        {
          continue;
        }
        {
          outputFeature.setAttributes( currentFeature.attributes() );
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

  QgsVectorFileWriter vWriter( shapefileName, dp->encoding(), layer->pendingFields(), outputType, &crs );
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
      if ( !layer->getFeatures( QgsFeatureRequest().setFilterFid( *it ) ).nextFeature( currentFeature ) )
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
    currentBufferDistance = f.attribute( bufferDistanceField ).toDouble();
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
    newFeature.setAttributes( f.attributes() );

    //add it to vector file writer
    if ( vfw )
    {
      vfw->addFeature( newFeature );
    }
  }
}

bool QgsGeometryAnalyzer::eventLayer( QgsVectorLayer* lineLayer, QgsVectorLayer* eventLayer, int lineField, int eventField, QList<int>& unlocatedFeatureIds, const QString& outputLayer,
                                      const QString& outputFormat, int locationField1, int locationField2, int offsetField, double offsetScale,
                                      bool forceSingleGeometry, QgsVectorDataProvider* memoryProvider, QProgressDialog* p )
{
  if ( !lineLayer || !eventLayer || !lineLayer->isValid() || !eventLayer->isValid() )
  {
    return false;
  }

  //create line field / id map for line layer
  QMultiHash< QString, QgsFeatureId > lineLayerIdMap; //1:n possible (e.g. several linear reference geometries for one feature in the event layer)
  QgsFeatureIterator fit = lineLayer->getFeatures( QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ).setSubsetOfAttributes( QgsAttributeList() << lineField ) );
  QgsFeature fet;
  while ( fit.nextFeature( fet ) )
  {
    lineLayerIdMap.insert( fet.attribute( lineField ).toString(), fet.id() );
  }

  //create output datasource or attributes in memory provider
  QgsVectorFileWriter* fileWriter = 0;
  QgsFeatureList memoryProviderFeatures;
  if ( !memoryProvider )
  {
    QGis::WkbType memoryProviderType = QGis::WKBMultiLineString;
    if ( locationField2 == -1 )
    {
      memoryProviderType = forceSingleGeometry ? QGis::WKBPoint : QGis::WKBMultiPoint;
    }
    else
    {
      memoryProviderType = forceSingleGeometry ? QGis::WKBLineString : QGis::WKBMultiLineString;
    }
    fileWriter = new QgsVectorFileWriter( outputLayer,
                                          eventLayer->dataProvider()->encoding(),
                                          eventLayer->pendingFields(),
                                          memoryProviderType,
                                          &( lineLayer->crs() ),
                                          outputFormat );
  }
  else
  {
    memoryProvider->addAttributes( eventLayer->pendingFields().toList() );
  }

  //iterate over eventLayer and write new features to output file or layer
  fit = eventLayer->getFeatures( QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ) );
  QgsGeometry* lrsGeom = 0;
  QgsFeature lineFeature;
  double measure1, measure2 = 0.0;

  int nEventFeatures = eventLayer->pendingFeatureCount();
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
    }

    QList<QgsFeatureId> featureIdList = lineLayerIdMap.values( fet.attribute( eventField ).toString() );
    QList<QgsFeatureId>::const_iterator featureIdIt = featureIdList.constBegin();
    for ( ; featureIdIt != featureIdList.constEnd(); ++featureIdIt )
    {
      if ( !lineLayer->getFeatures( QgsFeatureRequest().setFilterFid( *featureIdIt ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( lineFeature ) )
      {
        continue;
      }

      if ( locationField2 == -1 )
      {
        lrsGeom = locateAlongMeasure( measure1, lineFeature.geometry() );
      }
      else
      {
        lrsGeom = locateBetweenMeasures( measure1, measure2, lineFeature.geometry() );
      }

      if ( lrsGeom )
      {
        ++nOutputFeatures;
        addEventLayerFeature( fet, lrsGeom, lineFeature.geometry(), fileWriter, memoryProviderFeatures, offsetField, offsetScale, forceSingleGeometry );
      }
    }
    if ( nOutputFeatures < 1 )
    {
      unlocatedFeatureIds.push_back( fet.id() );
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

void QgsGeometryAnalyzer::addEventLayerFeature( QgsFeature& feature, QgsGeometry* geom, QgsGeometry* lineGeom, QgsVectorFileWriter* fileWriter, QgsFeatureList& memoryFeatures,
    int offsetField, double offsetScale, bool forceSingleType )
{
  if ( !geom )
  {
    return;
  }

  QList<QgsGeometry*> geomList;
  if ( forceSingleType )
  {
    geomList = geom->asGeometryCollection();
  }
  else
  {
    geomList.push_back( geom );
  }

  QList<QgsGeometry*>::iterator geomIt = geomList.begin();
  for ( ; geomIt != geomList.end(); ++geomIt )
  {
    //consider offset
    if ( offsetField >= 0 )
    {
      double offsetVal = feature.attribute( offsetField ).toDouble();
      offsetVal *= offsetScale;
      createOffsetGeometry( *geomIt, lineGeom, offsetVal );
    }

    feature.setGeometry( *geomIt );
    if ( fileWriter )
    {
      fileWriter->addFeature( feature );
    }
    else
    {
      memoryFeatures << feature;
    }
  }

  if ( forceSingleType )
  {
    delete geom;
  }
}

void QgsGeometryAnalyzer::createOffsetGeometry( QgsGeometry* geom, QgsGeometry* lineGeom, double offset )
{
  if ( !geom || !lineGeom )
  {
    return;
  }

  QList<QgsGeometry*> inputGeomList;

  if ( geom->isMultipart() )
  {
    inputGeomList = geom->asGeometryCollection();
  }
  else
  {
    inputGeomList.push_back( geom );
  }

  QList<GEOSGeometry*> outputGeomList;
  QList<QgsGeometry*>::const_iterator inputGeomIt = inputGeomList.constBegin();
  for ( ; inputGeomIt != inputGeomList.constEnd(); ++inputGeomIt )
  {
    if ( geom->type() == QGis::Line )
    {
      //geos 3.3 needed for line offsets
#if defined(GEOS_VERSION_MAJOR) && defined(GEOS_VERSION_MINOR) && \
      ((GEOS_VERSION_MAJOR>3) || ((GEOS_VERSION_MAJOR==3) && (GEOS_VERSION_MINOR>=3)))
      outputGeomList.push_back( GEOSOffsetCurve(( *inputGeomIt )->asGeos(), -offset, 8 /*quadSegments*/, 0 /*joinStyle*/, 5.0 /*mitreLimit*/ ) );
#else
      outputGeomList.push_back( GEOSGeom_clone(( *inputGeomIt )->asGeos() ) );
#endif
    }
    else if ( geom->type() == QGis::Point )
    {
      QgsPoint p = ( *inputGeomIt )->asPoint();
      p = createPointOffset( p.x(), p.y(), offset, lineGeom );
      GEOSCoordSequence* ptSeq = GEOSCoordSeq_create( 1, 2 );
      GEOSCoordSeq_setX( ptSeq, 0, p.x() );
      GEOSCoordSeq_setY( ptSeq, 0, p.y() );
      GEOSGeometry* geosPt = GEOSGeom_createPoint( ptSeq );
      outputGeomList.push_back( geosPt );
    }
  }

  if ( !geom->isMultipart() )
  {
    GEOSGeometry* outputGeom = outputGeomList.at( 0 );
    if ( outputGeom )
    {
      geom->fromGeos( outputGeom );
    }
  }
  else
  {
    GEOSGeometry** geomArray = new GEOSGeometry*[outputGeomList.size()];
    for ( int i = 0; i < outputGeomList.size(); ++i )
    {
      geomArray[i] = outputGeomList.at( i );
    }
    GEOSGeometry* collection = 0;
    if ( geom->type() == QGis::Point )
    {
      collection = GEOSGeom_createCollection( GEOS_MULTIPOINT, geomArray, outputGeomList.size() );
    }
    else if ( geom->type() == QGis::Line )
    {
      collection = GEOSGeom_createCollection( GEOS_MULTILINESTRING, geomArray, outputGeomList.size() );
    }
    geom->fromGeos( collection );
    delete[] geomArray;
  }
}

QgsPoint QgsGeometryAnalyzer::createPointOffset( double x, double y, double dist, QgsGeometry* lineGeom ) const
{
  QgsPoint p( x, y );
  QgsPoint minDistPoint;
  int afterVertexNr;
  lineGeom->closestSegmentWithContext( p, minDistPoint, afterVertexNr );

  int beforeVertexNr = afterVertexNr - 1;
  QgsPoint beforeVertex = lineGeom->vertexAt( beforeVertexNr );
  QgsPoint afterVertex = lineGeom->vertexAt( afterVertexNr );

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

QgsGeometry* QgsGeometryAnalyzer::locateBetweenMeasures( double fromMeasure, double toMeasure, QgsGeometry* lineGeom )
{
  if ( !lineGeom )
  {
    return 0;
  }

  QgsMultiPolyline resultGeom;

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

  if ( resultGeom.size() < 1 )
  {
    return 0;
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

  if ( resultGeom.size() < 1 )
  {
    return 0;
  }
  return QgsGeometry::fromMultiPoint( resultGeom );
}

unsigned char* QgsGeometryAnalyzer::locateBetweenWkbString( unsigned char* ptr, QgsMultiPolyline& result, double fromMeasure, double toMeasure )
{
  int* nPoints = ( int* ) ptr;
  ptr += sizeof( int );
  double prevx = 0.0, prevy = 0.0, prevz = 0.0;
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

        if ( pt1 != pt2 ) //avoid duplicated entry if measure value equals m-value of vertex
        {
          currentLine.append( pt2 );
        }

        if ( secondPointClipped || i == *nPoints - 1 ) //close current segment
        {
          if ( currentLine.size() > 1 )
          {
            result.append( currentLine );
          }
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
  double prevx = 0.0, prevy = 0.0, prevz = 0.0;
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
      pt1.setX( x2 ); pt1.setY( y2 );
      pt2.setX( x1 ); pt2.setY( y1 );
    }
    else
    {
      pt1.setX( x1 ); pt1.setY( y1 );
      pt2.setX( x2 ); pt2.setY( y2 );
    }
    secondPointClipped = false;
    return true;
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
  if ( doubleNear( m1, measure, tolerance ) )
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
  if ( doubleNear( m2, measure, tolerance ) )
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
