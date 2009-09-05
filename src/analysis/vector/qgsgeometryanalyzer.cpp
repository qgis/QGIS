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
/* $Id: qgis.h 9774 2008-12-12 05:41:24Z timlinux $ */

#include "qgsgeometryanalyzer.h"

#include "qgsapplication.h"
#include "qgsfield.h"
#include "qgsfeature.h"
#include "qgslogger.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectordataprovider.h"
#include "qgsdistancearea.h"



bool QgsGeometryAnalyzer::singlepartsToMultipart( QgsVectorLayer* layer,
                             const QString& shapefileName,
                             const QString& fileEncoding,
                             const int fieldIndex )
{
/*
  QgsVectorDataProvider* provider = layer->dataProvider();
  QgsAttributeList allAttrs = provider->attributeIndexes();
  provider->select( allAttrs, QgsRectangle(), true );
  const QgsCoordinateReferenceSystem* outputCRS;
  outputCRS = &layer->srs();
  QgsVectorFileWriter* writer = new QgsVectorFileWriter( shapefileName, 
  fileEncoding, provider->fields(), provider->geometryType(), outputCRS );
  
  QgsGeometry inGeom;
  QgsGeometry outGeom;
  QList<QVariant> unique;
  provider->uniqueValues( index, unique )
  if ( unique->size() < layer->featureCount() )
  {
    QList<QgsGeometry> multiGeom;
    bool first;
    QgsAttributeMap atMap;

    for ( int it = unique.begin(); it != unique.end(); ++it )
    {
      provider->select( allAttrs, QgsRectangle(), true );
      first = true;
      while ( provider->nextFeature( inFeat ) )
      {
        if ( inFeat.attributeMap()[ index ].toString().trimmed() == it.toString().trimmed() )
        {
          if (first)
          {
            atMap = inFeat.attributeMap();
            first = false;
          }
          inGeom = inFeat.geometry();
          multiGeom << inGeom.asGeometryCollection()
        }
          outFeat.setAttributeMap( atMap );
          outGeom = convertGeometry( multifeature, vtype );
          outFeat.setGeometry( outGeom );
          writer.addFeature( outFeat );
      }
    }
    delete writer;
    return true;
*/
}

bool QgsGeometryAnalyzer::multipartToSingleparts( QgsVectorLayer* layer,
                             const QString& shapefileName,
                             const QString& fileEncoding )
{
  /*
  QgsVectorDataProvider* provider = layer->dataProvider();
  QgsAttributeList allAttrs = provider->attributeIndexes();
  provider->select( allAttrs, QgsRectangle(), true );
  const QgsCoordinateReferenceSystem* outputCRS;
  outputCRS = &layer->srs();
  QgsVectorFileWriter* writer = new QgsVectorFileWriter( shapefileName, 
  fileEncoding, provider->fields(), provider->geometryType(), outputCRS );
    inFeat = QgsFeature()
    outFeat = QgsFeature()
    inGeom = QgsGeometry()
    outGeom = QgsGeometry()
    nFeat = vprovider.featureCount()
    nElement = 0
    self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), 0 )
    self.emit( SIGNAL( "runRange(PyQt_PyObject)" ), ( 0, nFeat ) )
    while vprovider.nextFeature( inFeat )
      nElement += 1  
      self.emit( SIGNAL( "runStatus(PyQt_PyObject)" ), nElement )
      inGeom = inFeat.geometry()
      atMap = inFeat.attributeMap()
      featList = self.extractAsSingle( inGeom )
      outFeat.setAttributeMap( atMap )
      for i in featList:
        outFeat.setGeometry( i )
        writer.addFeature( outFeat )
    del writer
    return True

 */ 
}

bool QgsGeometryAnalyzer::extractNodes( QgsVectorLayer* layer,
                             const QString& shapefileName,
                             const QString& fileEncoding )
{
/*
  QgsVectorDataProvider* provider = layer->dataProvider();
  QgsAttributeList allAttrs = provider->attributeIndexes();
  provider->select( allAttrs, QgsRectangle(), true );
  const QgsCoordinateReferenceSystem* outputCRS;
  outputCRS = &layer->srs();
  QgsVectorFileWriter* writer = new QgsVectorFileWriter( shapefileName, 
  fileEncoding, provider->fields(), provider->geometryType(), outputCRS );
 
  QgsFeature inFeat;
  QgsFeature outFeat;

  QgsGeometry outGeom;
  QList<QgsPoint> pointList;
  QgsPoint geomPoint;
  QList<QgsPoint>::iterator it;
  while ( provider->nextFeature( inFeat ) )
  {
    pointList = extractPoints( inFeat.geometry() );
    outFeat.setAttributeMap( inFeat.attributeMap() );
    for (it = pointList.begin(); it != pointList.end(); ++it )
    {
      geomPoint = QgsGeometry::fromPoint( it );
      outFeat.setGeometry( geomPoint );
      writer.addFeature( outFeat );
    }
  }
  delete writer;
  return true;
*/
}

bool QgsGeometryAnalyzer::polygonsToLines( QgsVectorLayer* layer,
                             const QString& shapefileName,
                             const QString& fileEncoding )
{
/*
  QgsVectorDataProvider* provider = layer->dataProvider();
  QgsAttributeList allAttrs = provider->attributeIndexes();
  provider->select( allAttrs, QgsRectangle(), true );
  QgsVectorFileWriter* writer = new QgsVectorFileWriter( shapefileName,
      fileEncoding, provider->fields(), QGis::WKBPoint, provider->crs() );


  
  QgsFeature inFeat;
  QgsFeature outFeat;
  QgsGeometry inGeom;
  QgsGeometry outGeom;
  QList<QgsPolyline> lineList;

  while ( provider->nextFeature( inFeat ) )
  {
    lineList = QgsGeometryAnalyzer::extractLines( inFeat.geometry() );
    outFeat.setAttributeMap( inFeat.attributeMap() );
    for ( line = lineList.begin(); line != lineList.end(); line++ )
    {
      outFeat.setGeometry( outGeom.fromPolyline( line ) );
      writer.addFeature( outFeat );
    }
  }
  delete writer;
  return true;
  */
}

bool QgsGeometryAnalyzer::exportGeometryInformation( QgsVectorLayer* layer,
                             const QString& shapefileName,
                             const QString& fileEncoding )
{
  QgsVectorDataProvider* provider = layer->dataProvider();
  QgsAttributeList allAttrs = provider->attributeIndexes();
  provider->select( allAttrs, QgsRectangle(), true );
  QgsCoordinateReferenceSystem outputCRS = layer->srs();
  int index1;
  int index2;
  //( fields, index1, index2 ) = self.checkGeometryFields( self.vlayer )
  
  QgsVectorFileWriter writer = QgsVectorFileWriter( shapefileName, 
  fileEncoding, provider->fields(), provider->geometryType(), &outputCRS );
 
  QgsFeature inFeat;
  QgsFeature outFeat;
  QgsGeometry* inGeom;
  QList<double> attrs;
  
  while ( provider->nextFeature( inFeat ) )
  {
    inGeom = inFeat.geometry();
    outFeat.setAttributeMap( inFeat.attributeMap() );
    attrs = QgsGeometryAnalyzer::simpleMeasure( inGeom );
    outFeat.setGeometry( inGeom );
    outFeat.setAttributeMap( inFeat.attributeMap() );
    outFeat.addAttribute( index1, QVariant( attrs[0] ) );
    outFeat.addAttribute( index2, QVariant( attrs[1] ) );
    writer.addFeature( outFeat );
  }
  return true;

}
bool QgsGeometryAnalyzer::simplifyGeometry( QgsVectorLayer* layer,
                             const QString shapefileName,
                             const QString fileEncoding,
                             const double tolerance )
{
  QgsVectorDataProvider* provider = layer->dataProvider();
  QgsAttributeList allAttrs = provider->attributeIndexes();
  provider->select( allAttrs, QgsRectangle(), true );
  QgsCoordinateReferenceSystem outputCRS = layer->srs();
  QgsVectorFileWriter writer = QgsVectorFileWriter( shapefileName, 
  fileEncoding, provider->fields(), provider->geometryType(), &outputCRS );
 
  QgsFeature inFeat;
  QgsFeature outFeat;
  QgsGeometry* inGeom;

  while ( provider->nextFeature( inFeat ) )
  {
    inGeom = inFeat.geometry();
    outFeat.setAttributeMap( inFeat.attributeMap() );
    outFeat.setGeometry( inGeom->simplify( tolerance ) );
    writer.addFeature( outFeat );
  }

  return true;

}

bool QgsGeometryAnalyzer::polygonCentroids( QgsVectorLayer* layer,
                             const QString& shapefileName,
                             const QString& fileEncoding )
{
  QgsVectorDataProvider* provider = layer->dataProvider();
  QgsAttributeList allAttrs = provider->attributeIndexes();
  provider->select( allAttrs, QgsRectangle(), true );
  QgsCoordinateReferenceSystem outputCRS = layer->srs();
  QgsVectorFileWriter writer = QgsVectorFileWriter( shapefileName, 
  fileEncoding, provider->fields(), provider->geometryType(), &outputCRS );
 
  QgsFeature inFeat;
  QgsFeature outFeat;
  QgsGeometry* inGeom;
  
  while ( provider->nextFeature( inFeat ) )
  {
    inGeom = inFeat.geometry();
    outFeat.setAttributeMap( inFeat.attributeMap() );
    outFeat.setGeometry( inGeom->centroid() );
    writer.addFeature( outFeat );
  }
  return true;
}

bool QgsGeometryAnalyzer::layerExtent( QgsVectorLayer* layer,
                             const QString& shapefileName,
                             const QString& fileEncoding )
{
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
  fields.insert( 9 , QgsField( QString( "WIDTH" ), QVariant::Double  ) );
  
  QgsVectorDataProvider* provider = layer->dataProvider();
  QgsCoordinateReferenceSystem outputCRS = layer->srs();
  QgsVectorFileWriter writer = QgsVectorFileWriter( shapefileName, 
  fileEncoding, provider->fields(), provider->geometryType(), &outputCRS );

  QgsRectangle rect;
  rect = layer->extent();
  double minx = rect.xMinimum();
  double miny = rect.yMinimum();
  double maxx = rect.xMaximum();
  double maxy = rect.yMaximum();
  double height = rect.height();
  double width = rect.width();
  
  double cntx = minx + ( width / 2.0 );
  double cnty = miny + ( height / 2.0 );
  double area = width * height;
  double perim = ( 2 * width ) + (2 * height );

  QgsFeature feat;
  QgsAttributeMap atMap;
  atMap.insert( 0 , QVariant( minx ) );
  atMap.insert( 1 , QVariant( miny ) );
  atMap.insert( 2 , QVariant( maxx ) );
  atMap.insert( 3 , QVariant( maxy ) );
  atMap.insert( 4 , QVariant( cntx ) );
  atMap.insert( 5 , QVariant( cnty ) );
  atMap.insert( 6 , QVariant( area ) );
  atMap.insert( 7 , QVariant( perim ) );
  atMap.insert( 8 , QVariant( height ) );
  atMap.insert( 9 , QVariant( width ) );
  feat.setAttributeMap( atMap );
  feat.setGeometry( QgsGeometry::fromRect( rect ) );
  writer.addFeature( feat );
  return true;
}

QList<double> QgsGeometryAnalyzer::simpleMeasure( QgsGeometry* mpGeometry )
{
  QList<double> list;
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
      list.append( perimeterMeasure( mpGeometry, measure ) );
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

QgsFieldMap QgsGeometryAnalyzer::checkGeometryFields( QgsVectorLayer* layer, int& index1, int& index2 )
{
/*  QgsVectorDataProvider* provider = layer->dataProvider();
  QgsAttributeList allAttrs = provider->attributeIndexes();
//  provider->select( allAttrs, QgsRectangle(), true );
  QgsFieldMap fields = provider->fields()
  QGis::GeometryType geomType = layer->geometryType();
  
  for i in fieldList.keys()
    nameList.append( fieldList[ i ].name().toLower() )
  if geomType == QGis.Polygon:
    plp = "Poly"
    ( found, index1 ) = self.checkForField( nameList, "AREA" )           
    if not found:
      field = QgsField( "AREA", QVariant.Double, "double", 10, 6, "Polygon area" )
      index1 = len( fieldList.keys() )
      fieldList[ index1 ] = field        
    ( found, index2 ) = self.checkForField( nameList, "PERIMETER" )
      
    if not found:
      field = QgsField( "PERIMETER", QVariant.Double, "double", 10, 6, "Polygon perimeter" )
      index2 = len( fieldList.keys() )
      fieldList[ index2 ] = field         
  elif geomType == QGis.Line:
    plp = "Line"
    (found, index1) = self.checkForField(nameList, "LENGTH")
    if not found:
      field = QgsField("LENGTH", QVariant.Double, "double", 10, 6, "Line length")
      index1 = len(fieldList.keys())
      fieldList[index1] = field
    index2 = index1
  else:
    plp = "Point"
    (found, index1) = self.checkForField(nameList, "XCOORD")
    if not found:
      field = QgsField("XCOORD", QVariant.Double, "double", 10, 6, "Point x coordinate")
      index1 = len(fieldList.keys())
      fieldList[index1] = field
    (found, index2) = self.checkForField(nameList, "YCOORD")
    if not found:
      field = QgsField("YCOORD", QVariant.Double, "double", 10, 6, "Point y coordinate")
      index2 = len(fieldList.keys())
      fieldList[index2] = field
  return (fieldList, index1, index2)
  */
  
}

QgsGeometry* QgsGeometryAnalyzer::extractLines( QgsGeometry* geometry )
{
/*
  QGis::WkbType wkbType = geometry.wkbType();
  QList<QgsPolyline> lineList;
  QgsMultiPolygon polyList
  if ( geometry.type() == QGis::Polygon )
  {
    if ( geometry.isMultipart() )
    {
      polyList = geometry.asMultiPolygon();
      for ( polygon = polyList.begin(); polygon != polyList.end(); polygon++ )
      {
        for ( lines = polygon.begin(); lines != polygon.end(); lines++ )
        {
          lineList << lines;
        }
      }
      else
      {
        lineList = geometry.asPolygon();
      }
    }
  }
  return lineList
  */
}
QgsGeometry* QgsGeometryAnalyzer::extractAsSingle( QgsGeometry* geometry )
{
/*
    multi_geom = QgsGeometry()
    temp_geom = []
    if geom.type() == 0:
      if geom.isMultipart()
        multi_geom = geom.asMultiPoint()
        for i in multi_geom:
          temp_geom.append( QgsGeometry().fromPoint ( i ) )
      else:
        temp_geom.append( geom )
    elif geom.type() == 1:
      if geom.isMultipart()
        multi_geom = geom.asMultiPolyline()
        for i in multi_geom:
          temp_geom.append( QgsGeometry().fromPolyline( i ) )
      else:
        temp_geom.append( geom )
    elif geom.type() == 2:
      if geom.isMultipart()
        multi_geom = geom.asMultiPolygon()
        for i in multi_geom:
          temp_geom.append( QgsGeometry().fromPolygon( i ) )
      else:
        temp_geom.append( geom )
    return temp_geom
        
*/
  
}

QgsGeometry* QgsGeometryAnalyzer::extractAsMulti( QgsGeometry* geometry )
{
/*
  if ( geometry->mGeos == NULL )
  {
    geometry->exportWkbToGeos();
  }
  if ( !geometry->mGeos )
  {
    return 0;
  }
    return fromGeosGeom( GEOSIntersection( mGeos, geometry->mGeos ) );

  for ( int i = 0; i < geometry.size(); i++ )
    geomarr[i] = geometry->mGeos[i];

  GEOSGeometry *geom = 0;

  try
  {
    geom = GEOSGeom_createCollection( typeId, geomarr, geoms.size() );
  }
  catch ( GEOSException &e )
  {
    Q_UNUSED( e );
  }

  delete [] geomarr;

  return geom;
}  

    temp_geom = []
    if geom.type() == 0:
      if geom.isMultipart()
        return geom.asMultiPoint()
      else:
        return [ geom.asPoint() ]
    elif geom.type() == 1:
      if geom.isMultipart()
        return geom.asMultiPolyline()
      else:
        return [ geom.asPolyline() ]
    else:
      if geom.isMultipart()
        return geom.asMultiPolygon()
      else:
        return [ geom.asPolygon() ]

*/
  
}

QgsGeometry* QgsGeometryAnalyzer::convertGeometry( QgsGeometry* geometry )
{
  /*
    if vType == 0:
      return QgsGeometry().fromMultiPoint(geom_list)
    elif vType == 1:
      return QgsGeometry().fromMultiPolyline(geom_list)
    else:
      return QgsGeometry().fromMultiPolygon(geom_list)
  */
}

QList<QgsPoint> QgsGeometryAnalyzer::extractPoints( QgsGeometry* geometry )
{
/*  QGis::WkbType wkbType = geometry.wkbType();
  QList<QgsPoint> pointList;
  QList<QgsPolyline> lineList;
  switch ( wkbType )
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
    case QGis::WKBMultiLineString25D:
    case QGis::WKBMultiLineString:
    {
      geometry->convertToMultitype();
      pointList = geometry.asMultiPoint();
      break;
    }
    case QGis::WKBLineString25D:
    case QGis::WKBLineString:
    case QGis::WKBMultiLineString25D:
    case QGis::WKBMultiLineString:
    {
      geometry->convertToMultitype();
      lineList = geometry.asMultiPolyline();
      for ( line = lineList.begin(); line != lineList.end(); line++ )
      {
        pointList << line;
      }
      break;
    }
    case QGis::WKBPolygon25D:
    case QGis::WKBPolygon:
    case QGis::WKBMultiPolygon25D:
    case QGis::WKBMultiPolygon:
    {
      geometry->convertToMultitype();
      QgsPolygon polyList = geometry.asMultiPolygon();
      for ( lineList = polyList.begin(); lineList != polyList.end(); lineList++ )
      {
        for ( line = lineList.begin(); line != lineList.end(); line++ )
        {
          pointList << line;
        }
      }
      break;
    }
    default:
      break;
  }
  return pointList;
  */
}
