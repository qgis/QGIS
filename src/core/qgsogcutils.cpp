/***************************************************************************
    qgsogcutils.cpp
    ---------------------
    begin                : March 2013
    copyright            : (C) 2013 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsogcutils.h"

#include "qgsexpression.h"
#include "qgsexpressionprivate.h"
#include "qgsgeometry.h"
#include "qgswkbptr.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscrscache.h"

#include <QColor>
#include <QStringList>
#include <QTextStream>
#include <QObject>

#ifndef Q_OS_WIN
#include <netinet/in.h>
#else
#include <winsock.h>
#endif


static const QString GML_NAMESPACE = "http://www.opengis.net/gml";
static const QString GML32_NAMESPACE = "http://www.opengis.net/gml/3.2";
static const QString OGC_NAMESPACE = "http://www.opengis.net/ogc";
static const QString FES_NAMESPACE = "http://www.opengis.net/fes/2.0";

QgsOgcUtilsExprToFilter::QgsOgcUtilsExprToFilter( QDomDocument& doc,
    QgsOgcUtils::GMLVersion gmlVersion,
    QgsOgcUtils::FilterVersion filterVersion,
    const QString& geometryName,
    const QString& srsName,
    bool honourAxisOrientation,
    bool invertAxisOrientation )
    : mDoc( doc )
    , mGMLUsed( false )
    , mGMLVersion( gmlVersion )
    , mFilterVersion( filterVersion )
    , mGeometryName( geometryName )
    , mSrsName( srsName )
    , mInvertAxisOrientation( invertAxisOrientation )
    , mFilterPrefix(( filterVersion == QgsOgcUtils::FILTER_FES_2_0 ) ? "fes" : "ogc" )
    , mPropertyName(( filterVersion == QgsOgcUtils::FILTER_FES_2_0 ) ? "ValueReference" : "PropertyName" )
    , mGeomId( 1 )
{
  QgsCoordinateReferenceSystem crs;
  if ( !mSrsName.isEmpty() )
    crs = QgsCRSCache::instance()->crsByOgcWmsCrs( mSrsName );
  if ( crs.isValid() )
  {
    if ( honourAxisOrientation && crs.axisInverted() )
    {
      mInvertAxisOrientation = !mInvertAxisOrientation;
    }
  }
}

QgsGeometry* QgsOgcUtils::geometryFromGML( const QDomNode& geometryNode )
{
  QDomElement geometryTypeElement = geometryNode.toElement();
  QString geomType = geometryTypeElement.tagName();

  if ( !( geomType == "Point" || geomType == "LineString" || geomType == "Polygon" ||
          geomType == "MultiPoint" || geomType == "MultiLineString" || geomType == "MultiPolygon" ||
          geomType == "Box" || geomType == "Envelope" ) )
  {
    QDomNode geometryChild = geometryNode.firstChild();
    if ( geometryChild.isNull() )
    {
      return nullptr;
    }
    geometryTypeElement = geometryChild.toElement();
    geomType = geometryTypeElement.tagName();
  }

  if ( !( geomType == "Point" || geomType == "LineString" || geomType == "Polygon" ||
          geomType == "MultiPoint" || geomType == "MultiLineString" || geomType == "MultiPolygon" ||
          geomType == "Box" || geomType == "Envelope" ) )
    return nullptr;

  if ( geomType == "Point" )
  {
    return geometryFromGMLPoint( geometryTypeElement );
  }
  else if ( geomType == "LineString" )
  {
    return geometryFromGMLLineString( geometryTypeElement );
  }
  else if ( geomType == "Polygon" )
  {
    return geometryFromGMLPolygon( geometryTypeElement );
  }
  else if ( geomType == "MultiPoint" )
  {
    return geometryFromGMLMultiPoint( geometryTypeElement );
  }
  else if ( geomType == "MultiLineString" )
  {
    return geometryFromGMLMultiLineString( geometryTypeElement );
  }
  else if ( geomType == "MultiPolygon" )
  {
    return geometryFromGMLMultiPolygon( geometryTypeElement );
  }
  else if ( geomType == "Box" )
  {
    return QgsGeometry::fromRect( rectangleFromGMLBox( geometryTypeElement ) );
  }
  else if ( geomType == "Envelope" )
  {
    return QgsGeometry::fromRect( rectangleFromGMLEnvelope( geometryTypeElement ) );
  }
  else //unknown type
  {
    return nullptr;
  }
}

QgsGeometry* QgsOgcUtils::geometryFromGML( const QString& xmlString )
{
  // wrap the string into a root tag to have "gml" namespace (and also as a default namespace)
  QString xml = QString( "<tmp xmlns=\"%1\" xmlns:gml=\"%1\">%2</tmp>" ).arg( GML_NAMESPACE, xmlString );
  QDomDocument doc;
  if ( !doc.setContent( xml, true ) )
    return nullptr;

  return geometryFromGML( doc.documentElement().firstChildElement() );
}


QgsGeometry* QgsOgcUtils::geometryFromGMLPoint( const QDomElement& geometryElement )
{
  QgsPolyline pointCoordinate;

  QDomNodeList coordList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "coordinates" );
  if ( !coordList.isEmpty() )
  {
    QDomElement coordElement = coordList.at( 0 ).toElement();
    if ( readGMLCoordinates( pointCoordinate, coordElement ) != 0 )
    {
      return nullptr;
    }
  }
  else
  {
    QDomNodeList posList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "pos" );
    if ( posList.size() < 1 )
    {
      return nullptr;
    }
    QDomElement posElement = posList.at( 0 ).toElement();
    if ( readGMLPositions( pointCoordinate, posElement ) != 0 )
    {
      return nullptr;
    }
  }

  if ( pointCoordinate.size() < 1 )
  {
    return nullptr;
  }

  QgsPolyline::const_iterator point_it = pointCoordinate.begin();
  char e = htonl( 1 ) != 1;
  double x = point_it->x();
  double y = point_it->y();
  int size = 1 + sizeof( int ) + 2 * sizeof( double );

  QGis::WkbType type = QGis::WKBPoint;
  unsigned char* wkb = new unsigned char[size];

  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  memcpy( &( wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( wkb )[wkbPosition], &x, sizeof( double ) );
  wkbPosition += sizeof( double );
  memcpy( &( wkb )[wkbPosition], &y, sizeof( double ) );

  QgsGeometry* g = new QgsGeometry();
  g->fromWkb( wkb, size );
  return g;
}

QgsGeometry* QgsOgcUtils::geometryFromGMLLineString( const QDomElement& geometryElement )
{
  QgsPolyline lineCoordinates;

  QDomNodeList coordList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "coordinates" );
  if ( !coordList.isEmpty() )
  {
    QDomElement coordElement = coordList.at( 0 ).toElement();
    if ( readGMLCoordinates( lineCoordinates, coordElement ) != 0 )
    {
      return nullptr;
    }
  }
  else
  {
    QDomNodeList posList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "posList" );
    if ( posList.size() < 1 )
    {
      return nullptr;
    }
    QDomElement posElement = posList.at( 0 ).toElement();
    if ( readGMLPositions( lineCoordinates, posElement ) != 0 )
    {
      return nullptr;
    }
  }

  char e = htonl( 1 ) != 1;
  int size = 1 + 2 * sizeof( int ) + lineCoordinates.size() * 2 * sizeof( double );

  QGis::WkbType type = QGis::WKBLineString;
  unsigned char* wkb = new unsigned char[size];

  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  double x, y;
  int nPoints = lineCoordinates.size();

  //fill the contents into *wkb
  memcpy( &( wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( wkb )[wkbPosition], &nPoints, sizeof( int ) );
  wkbPosition += sizeof( int );

  QgsPolyline::const_iterator iter;
  for ( iter = lineCoordinates.begin(); iter != lineCoordinates.end(); ++iter )
  {
    x = iter->x();
    y = iter->y();
    memcpy( &( wkb )[wkbPosition], &x, sizeof( double ) );
    wkbPosition += sizeof( double );
    memcpy( &( wkb )[wkbPosition], &y, sizeof( double ) );
    wkbPosition += sizeof( double );
  }

  QgsGeometry* g = new QgsGeometry();
  g->fromWkb( wkb, size );
  return g;
}

QgsGeometry* QgsOgcUtils::geometryFromGMLPolygon( const QDomElement& geometryElement )
{
  //read all the coordinates (as QgsPoint) into memory. Each linear ring has an entry in the vector
  QgsMultiPolyline ringCoordinates;

  //read coordinates for outer boundary
  QgsPolyline exteriorPointList;
  QDomNodeList outerBoundaryList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "outerBoundaryIs" );
  if ( !outerBoundaryList.isEmpty() ) //outer ring is necessary
  {
    QDomElement coordinatesElement = outerBoundaryList.at( 0 ).firstChild().firstChild().toElement();
    if ( coordinatesElement.isNull() )
    {
      return nullptr;
    }
    if ( readGMLCoordinates( exteriorPointList, coordinatesElement ) != 0 )
    {
      return nullptr;
    }
    ringCoordinates.push_back( exteriorPointList );

    //read coordinates for inner boundary
    QDomNodeList innerBoundaryList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "innerBoundaryIs" );
    for ( int i = 0; i < innerBoundaryList.size(); ++i )
    {
      QgsPolyline interiorPointList;
      coordinatesElement = innerBoundaryList.at( i ).firstChild().firstChild().toElement();
      if ( coordinatesElement.isNull() )
      {
        return nullptr;
      }
      if ( readGMLCoordinates( interiorPointList, coordinatesElement ) != 0 )
      {
        return nullptr;
      }
      ringCoordinates.push_back( interiorPointList );
    }
  }
  else
  {
    //read coordinates for exterior
    QDomNodeList exteriorList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "exterior" );
    if ( exteriorList.size() < 1 ) //outer ring is necessary
    {
      return nullptr;
    }
    QDomElement posElement = exteriorList.at( 0 ).firstChild().firstChild().toElement();
    if ( posElement.isNull() )
    {
      return nullptr;
    }
    if ( readGMLPositions( exteriorPointList, posElement ) != 0 )
    {
      return nullptr;
    }
    ringCoordinates.push_back( exteriorPointList );

    //read coordinates for inner boundary
    QDomNodeList interiorList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "interior" );
    for ( int i = 0; i < interiorList.size(); ++i )
    {
      QgsPolyline interiorPointList;
      QDomElement posElement = interiorList.at( i ).firstChild().firstChild().toElement();
      if ( posElement.isNull() )
      {
        return nullptr;
      }
      if ( readGMLPositions( interiorPointList, posElement ) != 0 )
      {
        return nullptr;
      }
      ringCoordinates.push_back( interiorPointList );
    }
  }

  //calculate number of bytes to allocate
  int nrings = ringCoordinates.size();
  if ( nrings < 1 )
    return nullptr;

  int npoints = 0;//total number of points
  for ( QgsMultiPolyline::const_iterator it = ringCoordinates.begin(); it != ringCoordinates.end(); ++it )
  {
    npoints += it->size();
  }
  int size = 1 + 2 * sizeof( int ) + nrings * sizeof( int ) + 2 * npoints * sizeof( double );

  QGis::WkbType type = QGis::WKBPolygon;
  unsigned char* wkb = new unsigned char[size];

  //char e = QgsApplication::endian();
  char e = htonl( 1 ) != 1;
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  int nPointsInRing = 0;
  double x, y;

  //fill the contents into *wkb
  memcpy( &( wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( wkb )[wkbPosition], &nrings, sizeof( int ) );
  wkbPosition += sizeof( int );
  for ( QgsMultiPolyline::const_iterator it = ringCoordinates.begin(); it != ringCoordinates.end(); ++it )
  {
    nPointsInRing = it->size();
    memcpy( &( wkb )[wkbPosition], &nPointsInRing, sizeof( int ) );
    wkbPosition += sizeof( int );
    //iterate through the string list converting the strings to x-/y- doubles
    QgsPolyline::const_iterator iter;
    for ( iter = it->begin(); iter != it->end(); ++iter )
    {
      x = iter->x();
      y = iter->y();
      //qWarning("currentCoordinate: " + QString::number(x) + " // " + QString::number(y));
      memcpy( &( wkb )[wkbPosition], &x, sizeof( double ) );
      wkbPosition += sizeof( double );
      memcpy( &( wkb )[wkbPosition], &y, sizeof( double ) );
      wkbPosition += sizeof( double );
    }
  }

  QgsGeometry* g = new QgsGeometry();
  g->fromWkb( wkb, size );
  return g;
}

QgsGeometry* QgsOgcUtils::geometryFromGMLMultiPoint( const QDomElement& geometryElement )
{
  QgsPolyline pointList;
  QgsPolyline currentPoint;
  QDomNodeList pointMemberList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "pointMember" );
  if ( pointMemberList.size() < 1 )
  {
    return nullptr;
  }
  QDomNodeList pointNodeList;
  // coordinates or pos element
  QDomNodeList coordinatesList;
  QDomNodeList posList;
  for ( int i = 0; i < pointMemberList.size(); ++i )
  {
    //<Point> element
    pointNodeList = pointMemberList.at( i ).toElement().elementsByTagNameNS( GML_NAMESPACE, "Point" );
    if ( pointNodeList.size() < 1 )
    {
      continue;
    }
    //<coordinates> element
    coordinatesList = pointNodeList.at( 0 ).toElement().elementsByTagNameNS( GML_NAMESPACE, "coordinates" );
    if ( !coordinatesList.isEmpty() )
    {
      currentPoint.clear();
      if ( readGMLCoordinates( currentPoint, coordinatesList.at( 0 ).toElement() ) != 0 )
      {
        continue;
      }
      if ( currentPoint.size() < 1 )
      {
        continue;
      }
      pointList.push_back(( *currentPoint.begin() ) );
      continue;
    }
    else
    {
      //<pos> element
      posList = pointNodeList.at( 0 ).toElement().elementsByTagNameNS( GML_NAMESPACE, "pos" );
      if ( posList.size() < 1 )
      {
        continue;
      }
      currentPoint.clear();
      if ( readGMLPositions( currentPoint, posList.at( 0 ).toElement() ) != 0 )
      {
        continue;
      }
      if ( currentPoint.size() < 1 )
      {
        continue;
      }
      pointList.push_back(( *currentPoint.begin() ) );
    }
  }

  int nPoints = pointList.size(); //number of points
  if ( nPoints < 1 )
    return nullptr;

  //calculate the required wkb size
  int size = 1 + 2 * sizeof( int ) + pointList.size() * ( 2 * sizeof( double ) + 1 + sizeof( int ) );

  QGis::WkbType type = QGis::WKBMultiPoint;
  unsigned char* wkb = new unsigned char[size];

  //fill the wkb content
  char e = htonl( 1 ) != 1;
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  double x, y;
  memcpy( &( wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( wkb )[wkbPosition], &nPoints, sizeof( int ) );
  wkbPosition += sizeof( int );
  type = QGis::WKBPoint;
  for ( QgsPolyline::const_iterator it = pointList.begin(); it != pointList.end(); ++it )
  {
    memcpy( &( wkb )[wkbPosition], &e, 1 );
    wkbPosition += 1;
    memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
    wkbPosition += sizeof( int );
    x = it->x();
    memcpy( &( wkb )[wkbPosition], &x, sizeof( double ) );
    wkbPosition += sizeof( double );
    y = it->y();
    memcpy( &( wkb )[wkbPosition], &y, sizeof( double ) );
    wkbPosition += sizeof( double );
  }

  QgsGeometry* g = new QgsGeometry();
  g->fromWkb( wkb, size );
  return g;
}

QgsGeometry* QgsOgcUtils::geometryFromGMLMultiLineString( const QDomElement& geometryElement )
{
  //geoserver has
  //<gml:MultiLineString>
  //<gml:lineStringMember>
  //<gml:LineString>

  //mapserver has directly
  //<gml:MultiLineString
  //<gml:LineString

  QList< QgsPolyline > lineCoordinates; //first list: lines, second list: points of one line
  QDomElement currentLineStringElement;
  QDomNodeList currentCoordList;
  QDomNodeList currentPosList;

  QDomNodeList lineStringMemberList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "lineStringMember" );
  if ( !lineStringMemberList.isEmpty() ) //geoserver
  {
    for ( int i = 0; i < lineStringMemberList.size(); ++i )
    {
      QDomNodeList lineStringNodeList = lineStringMemberList.at( i ).toElement().elementsByTagNameNS( GML_NAMESPACE, "LineString" );
      if ( lineStringNodeList.size() < 1 )
      {
        return nullptr;
      }
      currentLineStringElement = lineStringNodeList.at( 0 ).toElement();
      currentCoordList = currentLineStringElement.elementsByTagNameNS( GML_NAMESPACE, "coordinates" );
      if ( !currentCoordList.isEmpty() )
      {
        QgsPolyline currentPointList;
        if ( readGMLCoordinates( currentPointList, currentCoordList.at( 0 ).toElement() ) != 0 )
        {
          return nullptr;
        }
        lineCoordinates.push_back( currentPointList );
      }
      else
      {
        currentPosList = currentLineStringElement.elementsByTagNameNS( GML_NAMESPACE, "posList" );
        if ( currentPosList.size() < 1 )
        {
          return nullptr;
        }
        QgsPolyline currentPointList;
        if ( readGMLPositions( currentPointList, currentPosList.at( 0 ).toElement() ) != 0 )
        {
          return nullptr;
        }
        lineCoordinates.push_back( currentPointList );
      }
    }
  }
  else
  {
    QDomNodeList lineStringList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "LineString" );
    if ( !lineStringList.isEmpty() ) //mapserver
    {
      for ( int i = 0; i < lineStringList.size(); ++i )
      {
        currentLineStringElement = lineStringList.at( i ).toElement();
        currentCoordList = currentLineStringElement.elementsByTagNameNS( GML_NAMESPACE, "coordinates" );
        if ( !currentCoordList.isEmpty() )
        {
          QgsPolyline currentPointList;
          if ( readGMLCoordinates( currentPointList, currentCoordList.at( 0 ).toElement() ) != 0 )
          {
            return nullptr;
          }
          lineCoordinates.push_back( currentPointList );
          return nullptr;
        }
        else
        {
          currentPosList = currentLineStringElement.elementsByTagNameNS( GML_NAMESPACE, "posList" );
          if ( currentPosList.size() < 1 )
          {
            return nullptr;
          }
          QgsPolyline currentPointList;
          if ( readGMLPositions( currentPointList, currentPosList.at( 0 ).toElement() ) != 0 )
          {
            return nullptr;
          }
          lineCoordinates.push_back( currentPointList );
        }
      }
    }
    else
    {
      return nullptr;
    }
  }

  int nLines = lineCoordinates.size();
  if ( nLines < 1 )
    return nullptr;

  //calculate the required wkb size
  int size = ( lineCoordinates.size() + 1 ) * ( 1 + 2 * sizeof( int ) );
  for ( QList< QgsPolyline >::const_iterator it = lineCoordinates.begin(); it != lineCoordinates.end(); ++it )
  {
    size += it->size() * 2 * sizeof( double );
  }

  QGis::WkbType type = QGis::WKBMultiLineString;
  unsigned char* wkb = new unsigned char[size];

  //fill the wkb content
  char e = htonl( 1 ) != 1;
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  int nPoints; //number of points in a line
  double x, y;
  memcpy( &( wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( wkb )[wkbPosition], &nLines, sizeof( int ) );
  wkbPosition += sizeof( int );
  type = QGis::WKBLineString;
  for ( QList< QgsPolyline >::const_iterator it = lineCoordinates.begin(); it != lineCoordinates.end(); ++it )
  {
    memcpy( &( wkb )[wkbPosition], &e, 1 );
    wkbPosition += 1;
    memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
    wkbPosition += sizeof( int );
    nPoints = it->size();
    memcpy( &( wkb )[wkbPosition], &nPoints, sizeof( int ) );
    wkbPosition += sizeof( int );
    for ( QgsPolyline::const_iterator iter = it->begin(); iter != it->end(); ++iter )
    {
      x = iter->x();
      y = iter->y();
      // QgsDebugMsg( QString( "x, y is %1,%2" ).arg( x, 'f' ).arg( y, 'f' ) );
      memcpy( &( wkb )[wkbPosition], &x, sizeof( double ) );
      wkbPosition += sizeof( double );
      memcpy( &( wkb )[wkbPosition], &y, sizeof( double ) );
      wkbPosition += sizeof( double );
    }
  }

  QgsGeometry* g = new QgsGeometry();
  g->fromWkb( wkb, size );
  return g;
}

QgsGeometry* QgsOgcUtils::geometryFromGMLMultiPolygon( const QDomElement& geometryElement )
{
  //first list: different polygons, second list: different rings, third list: different points
  QgsMultiPolygon multiPolygonPoints;
  QDomElement currentPolygonMemberElement;
  QDomNodeList polygonList;
  QDomElement currentPolygonElement;
  // rings in GML2
  QDomNodeList outerBoundaryList;
  QDomElement currentOuterBoundaryElement;
  QDomNodeList innerBoundaryList;
  QDomElement currentInnerBoundaryElement;
  // rings in GML3
  QDomNodeList exteriorList;
  QDomElement currentExteriorElement;
  QDomElement currentInteriorElement;
  QDomNodeList interiorList;
  // lienar ring
  QDomNodeList linearRingNodeList;
  QDomElement currentLinearRingElement;
  // Coordinates or position list
  QDomNodeList currentCoordinateList;
  QDomNodeList currentPosList;

  QDomNodeList polygonMemberList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "polygonMember" );
  for ( int i = 0; i < polygonMemberList.size(); ++i )
  {
    QgsPolygon currentPolygonList;
    currentPolygonMemberElement = polygonMemberList.at( i ).toElement();
    polygonList = currentPolygonMemberElement.elementsByTagNameNS( GML_NAMESPACE, "Polygon" );
    if ( polygonList.size() < 1 )
    {
      continue;
    }
    currentPolygonElement = polygonList.at( 0 ).toElement();

    //find exterior ring
    outerBoundaryList = currentPolygonElement.elementsByTagNameNS( GML_NAMESPACE, "outerBoundaryIs" );
    if ( !outerBoundaryList.isEmpty() )
    {
      currentOuterBoundaryElement = outerBoundaryList.at( 0 ).toElement();
      QgsPolyline ringCoordinates;

      linearRingNodeList = currentOuterBoundaryElement.elementsByTagNameNS( GML_NAMESPACE, "LinearRing" );
      if ( linearRingNodeList.size() < 1 )
      {
        continue;
      }
      currentLinearRingElement = linearRingNodeList.at( 0 ).toElement();
      currentCoordinateList = currentLinearRingElement.elementsByTagNameNS( GML_NAMESPACE, "coordinates" );
      if ( currentCoordinateList.size() < 1 )
      {
        continue;
      }
      if ( readGMLCoordinates( ringCoordinates, currentCoordinateList.at( 0 ).toElement() ) != 0 )
      {
        continue;
      }
      currentPolygonList.push_back( ringCoordinates );

      //find interior rings
      QDomNodeList innerBoundaryList = currentPolygonElement.elementsByTagNameNS( GML_NAMESPACE, "innerBoundaryIs" );
      for ( int j = 0; j < innerBoundaryList.size(); ++j )
      {
        QgsPolyline ringCoordinates;
        currentInnerBoundaryElement = innerBoundaryList.at( j ).toElement();
        linearRingNodeList = currentInnerBoundaryElement.elementsByTagNameNS( GML_NAMESPACE, "LinearRing" );
        if ( linearRingNodeList.size() < 1 )
        {
          continue;
        }
        currentLinearRingElement = linearRingNodeList.at( 0 ).toElement();
        currentCoordinateList = currentLinearRingElement.elementsByTagNameNS( GML_NAMESPACE, "coordinates" );
        if ( currentCoordinateList.size() < 1 )
        {
          continue;
        }
        if ( readGMLCoordinates( ringCoordinates, currentCoordinateList.at( 0 ).toElement() ) != 0 )
        {
          continue;
        }
        currentPolygonList.push_back( ringCoordinates );
      }
    }
    else
    {
      //find exterior ring
      exteriorList = currentPolygonElement.elementsByTagNameNS( GML_NAMESPACE, "exterior" );
      if ( exteriorList.size() < 1 )
      {
        continue;
      }

      currentExteriorElement = exteriorList.at( 0 ).toElement();
      QgsPolyline ringPositions;

      linearRingNodeList = currentExteriorElement.elementsByTagNameNS( GML_NAMESPACE, "LinearRing" );
      if ( linearRingNodeList.size() < 1 )
      {
        continue;
      }
      currentLinearRingElement = linearRingNodeList.at( 0 ).toElement();
      currentPosList = currentLinearRingElement.elementsByTagNameNS( GML_NAMESPACE, "posList" );
      if ( currentPosList.size() < 1 )
      {
        continue;
      }
      if ( readGMLPositions( ringPositions, currentPosList.at( 0 ).toElement() ) != 0 )
      {
        continue;
      }
      currentPolygonList.push_back( ringPositions );

      //find interior rings
      QDomNodeList interiorList = currentPolygonElement.elementsByTagNameNS( GML_NAMESPACE, "interior" );
      for ( int j = 0; j < interiorList.size(); ++j )
      {
        QgsPolyline ringPositions;
        currentInteriorElement = interiorList.at( j ).toElement();
        linearRingNodeList = currentInteriorElement.elementsByTagNameNS( GML_NAMESPACE, "LinearRing" );
        if ( linearRingNodeList.size() < 1 )
        {
          continue;
        }
        currentLinearRingElement = linearRingNodeList.at( 0 ).toElement();
        currentPosList = currentLinearRingElement.elementsByTagNameNS( GML_NAMESPACE, "posList" );
        if ( currentPosList.size() < 1 )
        {
          continue;
        }
        if ( readGMLPositions( ringPositions, currentPosList.at( 0 ).toElement() ) != 0 )
        {
          continue;
        }
        currentPolygonList.push_back( ringPositions );
      }
    }
    multiPolygonPoints.push_back( currentPolygonList );
  }

  int nPolygons = multiPolygonPoints.size();
  if ( nPolygons < 1 )
    return nullptr;

  int size = 1 + 2 * sizeof( int );
  //calculate the wkb size
  for ( QgsMultiPolygon::const_iterator it = multiPolygonPoints.begin(); it != multiPolygonPoints.end(); ++it )
  {
    size += 1 + 2 * sizeof( int );
    for ( QgsPolygon::const_iterator iter = it->begin(); iter != it->end(); ++iter )
    {
      size += sizeof( int ) + 2 * iter->size() * sizeof( double );
    }
  }

  QGis::WkbType type = QGis::WKBMultiPolygon;
  unsigned char* wkb = new unsigned char[size];

  char e = htonl( 1 ) != 1;
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  double x, y;
  int nRings;
  int nPointsInRing;

  //fill the contents into *wkb
  memcpy( &( wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( wkb )[wkbPosition], &nPolygons, sizeof( int ) );
  wkbPosition += sizeof( int );

  type = QGis::WKBPolygon;

  for ( QgsMultiPolygon::const_iterator it = multiPolygonPoints.begin(); it != multiPolygonPoints.end(); ++it )
  {
    memcpy( &( wkb )[wkbPosition], &e, 1 );
    wkbPosition += 1;
    memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
    wkbPosition += sizeof( int );
    nRings = it->size();
    memcpy( &( wkb )[wkbPosition], &nRings, sizeof( int ) );
    wkbPosition += sizeof( int );
    for ( QgsPolygon::const_iterator iter = it->begin(); iter != it->end(); ++iter )
    {
      nPointsInRing = iter->size();
      memcpy( &( wkb )[wkbPosition], &nPointsInRing, sizeof( int ) );
      wkbPosition += sizeof( int );
      for ( QgsPolyline::const_iterator iterator = iter->begin(); iterator != iter->end(); ++iterator )
      {
        x = iterator->x();
        y = iterator->y();
        memcpy( &( wkb )[wkbPosition], &x, sizeof( double ) );
        wkbPosition += sizeof( double );
        memcpy( &( wkb )[wkbPosition], &y, sizeof( double ) );
        wkbPosition += sizeof( double );
      }
    }
  }

  QgsGeometry* g = new QgsGeometry();
  g->fromWkb( wkb, size );
  return g;
}

bool QgsOgcUtils::readGMLCoordinates( QgsPolyline &coords, const QDomElement &elem )
{
  QString coordSeparator = ",";
  QString tupelSeparator = " ";
  //"decimal" has to be "."

  coords.clear();

  if ( elem.hasAttribute( "cs" ) )
  {
    coordSeparator = elem.attribute( "cs" );
  }
  if ( elem.hasAttribute( "ts" ) )
  {
    tupelSeparator = elem.attribute( "ts" );
  }

  QStringList tupels = elem.text().split( tupelSeparator, QString::SkipEmptyParts );
  QStringList tupel_coords;
  double x, y;
  bool conversionSuccess;

  QStringList::const_iterator it;
  for ( it = tupels.constBegin(); it != tupels.constEnd(); ++it )
  {
    tupel_coords = ( *it ).split( coordSeparator, QString::SkipEmptyParts );
    if ( tupel_coords.size() < 2 )
    {
      continue;
    }
    x = tupel_coords.at( 0 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      return 1;
    }
    y = tupel_coords.at( 1 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      return 1;
    }
    coords.push_back( QgsPoint( x, y ) );
  }
  return 0;
}

QgsRectangle QgsOgcUtils::rectangleFromGMLBox( const QDomNode& boxNode )
{
  QgsRectangle rect;

  QDomElement boxElem = boxNode.toElement();
  if ( boxElem.tagName() != "Box" )
    return rect;

  QDomElement bElem = boxElem.firstChild().toElement();
  QString coordSeparator = ",";
  QString tupelSeparator = " ";
  if ( bElem.hasAttribute( "cs" ) )
  {
    coordSeparator = bElem.attribute( "cs" );
  }
  if ( bElem.hasAttribute( "ts" ) )
  {
    tupelSeparator = bElem.attribute( "ts" );
  }

  QString bString = bElem.text();
  bool ok1, ok2, ok3, ok4;
  double xmin = bString.section( tupelSeparator, 0, 0 ).section( coordSeparator, 0, 0 ).toDouble( &ok1 );
  double ymin = bString.section( tupelSeparator, 0, 0 ).section( coordSeparator, 1, 1 ).toDouble( &ok2 );
  double xmax = bString.section( tupelSeparator, 1, 1 ).section( coordSeparator, 0, 0 ).toDouble( &ok3 );
  double ymax = bString.section( tupelSeparator, 1, 1 ).section( coordSeparator, 1, 1 ).toDouble( &ok4 );

  if ( ok1 && ok2 && ok3 && ok4 )
  {
    rect = QgsRectangle( xmin, ymin, xmax, ymax );
    rect.normalize();
  }

  return rect;
}

bool QgsOgcUtils::readGMLPositions( QgsPolyline &coords, const QDomElement &elem )
{
  coords.clear();

  QStringList pos = elem.text().split( ' ', QString::SkipEmptyParts );
  double x, y;
  bool conversionSuccess;
  int posSize = pos.size();

  int srsDimension = 2;
  if ( elem.hasAttribute( "srsDimension" ) )
  {
    srsDimension = elem.attribute( "srsDimension" ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      srsDimension = 2;
    }
  }
  else if ( elem.hasAttribute( "dimension" ) )
  {
    srsDimension = elem.attribute( "dimension" ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      srsDimension = 2;
    }
  }

  for ( int i = 0; i < posSize / srsDimension; i++ )
  {
    x = pos.at( i * srsDimension ).toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      return 1;
    }
    y = pos.at( i * srsDimension + 1 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      return 1;
    }
    coords.push_back( QgsPoint( x, y ) );
  }
  return 0;
}


QgsRectangle QgsOgcUtils::rectangleFromGMLEnvelope( const QDomNode& envelopeNode )
{
  QgsRectangle rect;

  QDomElement envelopeElem = envelopeNode.toElement();
  if ( envelopeElem.tagName() != "Envelope" )
    return rect;

  QDomNodeList lowerCornerList = envelopeElem.elementsByTagNameNS( GML_NAMESPACE, "lowerCorner" );
  if ( lowerCornerList.size() < 1 )
    return rect;

  QDomNodeList upperCornerList = envelopeElem.elementsByTagNameNS( GML_NAMESPACE, "upperCorner" );
  if ( upperCornerList.size() < 1 )
    return rect;

  bool conversionSuccess;
  int srsDimension = 2;

  QDomElement elem = lowerCornerList.at( 0 ).toElement();
  if ( elem.hasAttribute( "srsDimension" ) )
  {
    srsDimension = elem.attribute( "srsDimension" ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      srsDimension = 2;
    }
  }
  else if ( elem.hasAttribute( "dimension" ) )
  {
    srsDimension = elem.attribute( "dimension" ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      srsDimension = 2;
    }
  }
  QString bString = elem.text();

  double xmin = bString.section( ' ', 0, 0 ).toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    return rect;
  double ymin = bString.section( ' ', 1, 1 ).toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    return rect;

  elem = upperCornerList.at( 0 ).toElement();
  if ( elem.hasAttribute( "srsDimension" ) )
  {
    srsDimension = elem.attribute( "srsDimension" ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      srsDimension = 2;
    }
  }
  else if ( elem.hasAttribute( "dimension" ) )
  {
    srsDimension = elem.attribute( "dimension" ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      srsDimension = 2;
    }
  }

  Q_UNUSED( srsDimension );

  bString = elem.text();
  double xmax = bString.section( ' ', 0, 0 ).toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    return rect;
  double ymax = bString.section( ' ', 1, 1 ).toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    return rect;

  rect = QgsRectangle( xmin, ymin, xmax, ymax );
  rect.normalize();

  return rect;
}

QDomElement QgsOgcUtils::rectangleToGMLBox( QgsRectangle* box, QDomDocument& doc, int precision )
{
  return rectangleToGMLBox( box, doc, QString(), false, precision );
}

QDomElement QgsOgcUtils::rectangleToGMLBox( QgsRectangle* box, QDomDocument& doc,
    const QString& srsName,
    bool invertAxisOrientation,
    int precision )
{
  if ( !box )
  {
    return QDomElement();
  }

  QDomElement boxElem = doc.createElement( "gml:Box" );
  if ( !srsName.isEmpty() )
  {
    boxElem.setAttribute( "srsName", srsName );
  }
  QDomElement coordElem = doc.createElement( "gml:coordinates" );
  coordElem.setAttribute( "cs", "," );
  coordElem.setAttribute( "ts", " " );

  QString coordString;
  coordString += qgsDoubleToString( invertAxisOrientation ? box->yMinimum() : box->xMinimum(), precision );
  coordString += ',';
  coordString += qgsDoubleToString( invertAxisOrientation ? box->xMinimum() : box->yMinimum(), precision );
  coordString += ' ';
  coordString += qgsDoubleToString( invertAxisOrientation ? box->yMaximum() : box->xMaximum(), precision );
  coordString += ',';
  coordString += qgsDoubleToString( invertAxisOrientation ? box->xMaximum() : box->yMaximum(), precision );

  QDomText coordText = doc.createTextNode( coordString );
  coordElem.appendChild( coordText );
  boxElem.appendChild( coordElem );

  return boxElem;
}

QDomElement QgsOgcUtils::rectangleToGMLEnvelope( QgsRectangle* env, QDomDocument& doc, int precision )
{
  return rectangleToGMLEnvelope( env, doc, QString(), false, precision );
}

QDomElement QgsOgcUtils::rectangleToGMLEnvelope( QgsRectangle* env, QDomDocument& doc,
    const QString& srsName,
    bool invertAxisOrientation,
    int precision )
{
  if ( !env )
  {
    return QDomElement();
  }

  QDomElement envElem = doc.createElement( "gml:Envelope" );
  if ( !srsName.isEmpty() )
  {
    envElem.setAttribute( "srsName", srsName );
  }
  QString posList;

  QDomElement lowerCornerElem = doc.createElement( "gml:lowerCorner" );
  posList = qgsDoubleToString( invertAxisOrientation ? env->yMinimum() : env->xMinimum(), precision );
  posList += ' ';
  posList += qgsDoubleToString( invertAxisOrientation ? env->xMinimum() : env->yMinimum(), precision );
  QDomText lowerCornerText = doc.createTextNode( posList );
  lowerCornerElem.appendChild( lowerCornerText );
  envElem.appendChild( lowerCornerElem );

  QDomElement upperCornerElem = doc.createElement( "gml:upperCorner" );
  posList = qgsDoubleToString( invertAxisOrientation ? env->yMaximum() : env->xMaximum(), precision );
  posList += ' ';
  posList += qgsDoubleToString( invertAxisOrientation ? env->xMaximum() : env->yMaximum(), precision );
  QDomText upperCornerText = doc.createTextNode( posList );
  upperCornerElem.appendChild( upperCornerText );
  envElem.appendChild( upperCornerElem );

  return envElem;
}

QDomElement QgsOgcUtils::geometryToGML( const QgsGeometry* geometry, QDomDocument& doc, const QString& format, int precision )
{
  return geometryToGML( geometry, doc, ( format == "GML2" ) ? GML_2_1_2 : GML_3_2_1, QString(), false, QString(), precision );
}

QDomElement QgsOgcUtils::geometryToGML( const QgsGeometry* geometry, QDomDocument& doc,
                                        GMLVersion gmlVersion,
                                        const QString& srsName,
                                        bool invertAxisOrientation,
                                        const QString& gmlIdBase,
                                        int precision )
{
  if ( !geometry || !geometry->asWkb() )
    return QDomElement();

  // coordinate separator
  QString cs = ",";
  // tupel separator
  QString ts = " ";
  // coord element tagname
  QDomElement baseCoordElem;

  bool hasZValue = false;

  QgsConstWkbPtr wkbPtr( geometry->asWkb(), geometry->wkbSize() );
  try
  {
    wkbPtr.readHeader();
  }
  catch ( const QgsWkbException &e )
  {
    Q_UNUSED( e );
    // WKB exception while reading header
    return QDomElement();
  }

  if ( gmlVersion != GML_2_1_2 )
  {
    switch ( geometry->wkbType() )
    {
      case QGis::WKBPoint25D:
      case QGis::WKBPoint:
      case QGis::WKBMultiPoint25D:
      case QGis::WKBMultiPoint:
        baseCoordElem = doc.createElement( "gml:pos" );
        break;
      default:
        baseCoordElem = doc.createElement( "gml:posList" );
        break;
    }
    baseCoordElem.setAttribute( "srsDimension", "2" );
    cs = ' ';
  }
  else
  {
    baseCoordElem = doc.createElement( "gml:coordinates" );
    baseCoordElem.setAttribute( "cs", cs );
    baseCoordElem.setAttribute( "ts", ts );
  }

  try
  {
    switch ( geometry->wkbType() )
    {
      case QGis::WKBPoint25D:
      case QGis::WKBPoint:
      {
        QDomElement pointElem = doc.createElement( "gml:Point" );
        if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
          pointElem.setAttribute( "gml:id", gmlIdBase );
        if ( !srsName.isEmpty() )
          pointElem.setAttribute( "srsName", srsName );
        QDomElement coordElem = baseCoordElem.cloneNode().toElement();

        double x, y;

        if ( invertAxisOrientation )
          wkbPtr >> y >> x;
        else
          wkbPtr >> x >> y;
        QDomText coordText = doc.createTextNode( qgsDoubleToString( x, precision ) + cs + qgsDoubleToString( y, precision ) );

        coordElem.appendChild( coordText );
        pointElem.appendChild( coordElem );
        return pointElem;
      }
      case QGis::WKBMultiPoint25D:
        hasZValue = true;
        //intentional fall-through
        FALLTHROUGH;
      case QGis::WKBMultiPoint:
      {
        QDomElement multiPointElem = doc.createElement( "gml:MultiPoint" );
        if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
          multiPointElem.setAttribute( "gml:id", gmlIdBase );
        if ( !srsName.isEmpty() )
          multiPointElem.setAttribute( "srsName", srsName );

        int nPoints;
        wkbPtr >> nPoints;

        for ( int idx = 0; idx < nPoints; ++idx )
        {
          QDomElement pointMemberElem = doc.createElement( "gml:pointMember" );
          QDomElement pointElem = doc.createElement( "gml:Point" );
          if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
            pointElem.setAttribute( "gml:id", gmlIdBase + QString( ".%1" ).arg( idx + 1 ) );
          QDomElement coordElem = baseCoordElem.cloneNode().toElement();

          wkbPtr.readHeader();

          double x, y;
          if ( invertAxisOrientation )
            wkbPtr >> y >> x;
          else
            wkbPtr >> x >> y;
          QDomText coordText = doc.createTextNode( qgsDoubleToString( x, precision ) + cs + qgsDoubleToString( y, precision ) );

          coordElem.appendChild( coordText );
          pointElem.appendChild( coordElem );

          if ( hasZValue )
          {
            wkbPtr += sizeof( double );
          }
          pointMemberElem.appendChild( pointElem );
          multiPointElem.appendChild( pointMemberElem );
        }
        return multiPointElem;
      }
      case QGis::WKBLineString25D:
        hasZValue = true;
        //intentional fall-through
        FALLTHROUGH;
      case QGis::WKBLineString:
      {
        QDomElement lineStringElem = doc.createElement( "gml:LineString" );
        if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
          lineStringElem.setAttribute( "gml:id", gmlIdBase );
        if ( !srsName.isEmpty() )
          lineStringElem.setAttribute( "srsName", srsName );
        // get number of points in the line

        int nPoints;
        wkbPtr >> nPoints;

        QDomElement coordElem = baseCoordElem.cloneNode().toElement();
        QString coordString;
        for ( int idx = 0; idx < nPoints; ++idx )
        {
          if ( idx != 0 )
          {
            coordString += ts;
          }

          double x, y;
          if ( invertAxisOrientation )
            wkbPtr >> y >> x;
          else
            wkbPtr >> x >> y;
          coordString += qgsDoubleToString( x, precision ) + cs + qgsDoubleToString( y, precision );

          if ( hasZValue )
          {
            wkbPtr += sizeof( double );
          }
        }
        QDomText coordText = doc.createTextNode( coordString );
        coordElem.appendChild( coordText );
        lineStringElem.appendChild( coordElem );
        return lineStringElem;
      }
      case QGis::WKBMultiLineString25D:
        hasZValue = true;
        //intentional fall-through
        FALLTHROUGH;
      case QGis::WKBMultiLineString:
      {
        QDomElement multiLineStringElem = doc.createElement( "gml:MultiLineString" );
        if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
          multiLineStringElem.setAttribute( "gml:id", gmlIdBase );
        if ( !srsName.isEmpty() )
          multiLineStringElem.setAttribute( "srsName", srsName );

        int nLines;
        wkbPtr >> nLines;

        for ( int jdx = 0; jdx < nLines; jdx++ )
        {
          QDomElement lineStringMemberElem = doc.createElement( "gml:lineStringMember" );
          QDomElement lineStringElem = doc.createElement( "gml:LineString" );
          if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
            lineStringElem.setAttribute( "gml:id", gmlIdBase + QString( ".%1" ).arg( jdx + 1 ) );

          wkbPtr.readHeader();

          int nPoints;
          wkbPtr >> nPoints;

          QDomElement coordElem = baseCoordElem.cloneNode().toElement();
          QString coordString;
          for ( int idx = 0; idx < nPoints; idx++ )
          {
            if ( idx != 0 )
            {
              coordString += ts;
            }

            double x, y;
            if ( invertAxisOrientation )
              wkbPtr >> y >> x;
            else
              wkbPtr >> x >> y;

            coordString += qgsDoubleToString( x, precision ) + cs + qgsDoubleToString( y, precision );

            if ( hasZValue )
            {
              wkbPtr += sizeof( double );
            }
          }
          QDomText coordText = doc.createTextNode( coordString );
          coordElem.appendChild( coordText );
          lineStringElem.appendChild( coordElem );
          lineStringMemberElem.appendChild( lineStringElem );
          multiLineStringElem.appendChild( lineStringMemberElem );
        }
        return multiLineStringElem;
      }
      case QGis::WKBPolygon25D:
        hasZValue = true;
        //intentional fall-through
        FALLTHROUGH;
      case QGis::WKBPolygon:
      {
        QDomElement polygonElem = doc.createElement( "gml:Polygon" );
        if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
          polygonElem.setAttribute( "gml:id", gmlIdBase );
        if ( !srsName.isEmpty() )
          polygonElem.setAttribute( "srsName", srsName );

        // get number of rings in the polygon
        int numRings;
        wkbPtr >> numRings;

        if ( numRings == 0 ) // sanity check for zero rings in polygon
          return QDomElement();

        int *ringNumPoints = new int[numRings]; // number of points in each ring

        for ( int idx = 0; idx < numRings; idx++ )
        {
          QString boundaryName = ( gmlVersion == GML_2_1_2 ) ? "gml:outerBoundaryIs" : "gml:exterior";
          if ( idx != 0 )
          {
            boundaryName = ( gmlVersion == GML_2_1_2 ) ? "gml:innerBoundaryIs" : "gml:interior";
          }
          QDomElement boundaryElem = doc.createElement( boundaryName );
          QDomElement ringElem = doc.createElement( "gml:LinearRing" );
          // get number of points in the ring
          int nPoints;
          wkbPtr >> nPoints;
          ringNumPoints[idx] = nPoints;

          QDomElement coordElem = baseCoordElem.cloneNode().toElement();
          QString coordString;
          for ( int jdx = 0; jdx < nPoints; jdx++ )
          {
            if ( jdx != 0 )
            {
              coordString += ts;
            }

            double x, y;
            if ( invertAxisOrientation )
              wkbPtr >> y >> x;
            else
              wkbPtr >> x >> y;

            coordString += qgsDoubleToString( x, precision ) + cs + qgsDoubleToString( y, precision );
            if ( hasZValue )
            {
              wkbPtr += sizeof( double );
            }
          }
          QDomText coordText = doc.createTextNode( coordString );
          coordElem.appendChild( coordText );
          ringElem.appendChild( coordElem );
          boundaryElem.appendChild( ringElem );
          polygonElem.appendChild( boundaryElem );
        }
        delete [] ringNumPoints;
        return polygonElem;
      }
      case QGis::WKBMultiPolygon25D:
        hasZValue = true;
        //intentional fall-through
        FALLTHROUGH;
      case QGis::WKBMultiPolygon:
      {
        QDomElement multiPolygonElem = doc.createElement( "gml:MultiPolygon" );
        if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
          multiPolygonElem.setAttribute( "gml:id", gmlIdBase );
        if ( !srsName.isEmpty() )
          multiPolygonElem.setAttribute( "srsName", srsName );

        int numPolygons;
        wkbPtr >> numPolygons;

        for ( int kdx = 0; kdx < numPolygons; kdx++ )
        {
          QDomElement polygonMemberElem = doc.createElement( "gml:polygonMember" );
          QDomElement polygonElem = doc.createElement( "gml:Polygon" );
          if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
            polygonElem.setAttribute( "gml:id", gmlIdBase + QString( ".%1" ).arg( kdx + 1 ) );

          wkbPtr.readHeader();

          int numRings;
          wkbPtr >> numRings;

          for ( int idx = 0; idx < numRings; idx++ )
          {
            QString boundaryName = ( gmlVersion == GML_2_1_2 ) ? "gml:outerBoundaryIs" : "gml:exterior";
            if ( idx != 0 )
            {
              boundaryName = ( gmlVersion == GML_2_1_2 ) ? "gml:innerBoundaryIs" : "gml:interior";
            }
            QDomElement boundaryElem = doc.createElement( boundaryName );
            QDomElement ringElem = doc.createElement( "gml:LinearRing" );

            int nPoints;
            wkbPtr >> nPoints;

            QDomElement coordElem = baseCoordElem.cloneNode().toElement();
            QString coordString;
            for ( int jdx = 0; jdx < nPoints; jdx++ )
            {
              if ( jdx != 0 )
              {
                coordString += ts;
              }

              double x, y;
              if ( invertAxisOrientation )
                wkbPtr >> y >> x;
              else
                wkbPtr >> x >> y;

              coordString += qgsDoubleToString( x, precision ) + cs + qgsDoubleToString( y, precision );

              if ( hasZValue )
              {
                wkbPtr += sizeof( double );
              }
            }
            QDomText coordText = doc.createTextNode( coordString );
            coordElem.appendChild( coordText );
            ringElem.appendChild( coordElem );
            boundaryElem.appendChild( ringElem );
            polygonElem.appendChild( boundaryElem );
            polygonMemberElem.appendChild( polygonElem );
            multiPolygonElem.appendChild( polygonMemberElem );
          }
        }
        return multiPolygonElem;
      }
      default:
        return QDomElement();
    }
  }
  catch ( const QgsWkbException &e )
  {
    Q_UNUSED( e );
    return QDomElement();
  }
}

QDomElement QgsOgcUtils::geometryToGML( const QgsGeometry *geometry, QDomDocument &doc, int precision )
{
  return geometryToGML( geometry, doc, "GML2", precision );
}

QDomElement QgsOgcUtils::createGMLCoordinates( const QgsPolyline &points, QDomDocument &doc )
{
  QDomElement coordElem = doc.createElement( "gml:coordinates" );
  coordElem.setAttribute( "cs", "," );
  coordElem.setAttribute( "ts", " " );

  QString coordString;
  QVector<QgsPoint>::const_iterator pointIt = points.constBegin();
  for ( ; pointIt != points.constEnd(); ++pointIt )
  {
    if ( pointIt != points.constBegin() )
    {
      coordString += ' ';
    }
    coordString += qgsDoubleToString( pointIt->x() );
    coordString += ',';
    coordString += qgsDoubleToString( pointIt->y() );
  }

  QDomText coordText = doc.createTextNode( coordString );
  coordElem.appendChild( coordText );
  return coordElem;
}

QDomElement QgsOgcUtils::createGMLPositions( const QgsPolyline &points, QDomDocument& doc )
{
  QDomElement posElem = doc.createElement( "gml:pos" );
  if ( points.size() > 1 )
    posElem = doc.createElement( "gml:posList" );
  posElem.setAttribute( "srsDimension", "2" );

  QString coordString;
  QVector<QgsPoint>::const_iterator pointIt = points.constBegin();
  for ( ; pointIt != points.constEnd(); ++pointIt )
  {
    if ( pointIt != points.constBegin() )
    {
      coordString += ' ';
    }
    coordString += qgsDoubleToString( pointIt->x() );
    coordString += ' ';
    coordString += qgsDoubleToString( pointIt->y() );
  }

  QDomText coordText = doc.createTextNode( coordString );
  posElem.appendChild( coordText );
  return posElem;
}



// -----------------------------------------

QColor QgsOgcUtils::colorFromOgcFill( const QDomElement& fillElement )
{
  if ( fillElement.isNull() || !fillElement.hasChildNodes() )
  {
    return QColor();
  }

  QString cssName;
  QString elemText;
  QColor color;
  QDomElement cssElem = fillElement.firstChildElement( "CssParameter" );
  while ( !cssElem.isNull() )
  {
    cssName = cssElem.attribute( "name", "not_found" );
    if ( cssName != "not_found" )
    {
      elemText = cssElem.text();
      if ( cssName == "fill" )
      {
        color.setNamedColor( elemText );
      }
      else if ( cssName == "fill-opacity" )
      {
        bool ok;
        double opacity = elemText.toDouble( &ok );
        if ( ok )
        {
          color.setAlphaF( opacity );
        }
      }
    }

    cssElem = cssElem.nextSiblingElement( "CssParameter" );
  }

  return color;
}


QgsExpression* QgsOgcUtils::expressionFromOgcFilter( const QDomElement& element )
{
  if ( element.isNull() || !element.hasChildNodes() )
    return nullptr;

  QgsExpression *expr = new QgsExpression();

  QDomElement childElem = element.firstChildElement();
  while ( !childElem.isNull() )
  {
    QString errorMsg;
    QgsExpression::Node *node = nodeFromOgcFilter( childElem, errorMsg );
    if ( !node )
    {
      // invalid expression, parser error
      expr->d->mParserErrorString = errorMsg;
      return expr;
    }

    // use the concat binary operator to append to the root node
    if ( !expr->d->mRootNode )
    {
      expr->d->mRootNode = node;
    }
    else
    {
      expr->d->mRootNode = new QgsExpression::NodeBinaryOperator( QgsExpression::boConcat, expr->d->mRootNode, node );
    }

    childElem = childElem.nextSiblingElement();
  }

  // update expression string
  expr->d->mExp = expr->dump();

  return expr;
}


static const QMap<QString, int>& binaryOperatorsTagNamesMap()
{
  static QMap<QString, int> binOps;
  if ( binOps.isEmpty() )
  {
    // logical
    binOps.insert( "Or", QgsExpression::boOr );
    binOps.insert( "And", QgsExpression::boAnd );
    // comparison
    binOps.insert( "PropertyIsEqualTo", QgsExpression::boEQ );
    binOps.insert( "PropertyIsNotEqualTo", QgsExpression::boNE );
    binOps.insert( "PropertyIsLessThanOrEqualTo", QgsExpression::boLE );
    binOps.insert( "PropertyIsGreaterThanOrEqualTo", QgsExpression::boGE );
    binOps.insert( "PropertyIsLessThan", QgsExpression::boLT );
    binOps.insert( "PropertyIsGreaterThan", QgsExpression::boGT );
    binOps.insert( "PropertyIsLike", QgsExpression::boLike );
    // arithmetics
    binOps.insert( "Add", QgsExpression::boPlus );
    binOps.insert( "Sub", QgsExpression::boMinus );
    binOps.insert( "Mul", QgsExpression::boMul );
    binOps.insert( "Div", QgsExpression::boDiv );
  }
  return binOps;
}

static int binaryOperatorFromTagName( const QString& tagName )
{

  return binaryOperatorsTagNamesMap().value( tagName, -1 );
}

static QString binaryOperatorToTagName( QgsExpression::BinaryOperator op )
{
  return binaryOperatorsTagNamesMap().key( op, QString() );
}

static bool isBinaryOperator( const QString& tagName )
{
  return binaryOperatorFromTagName( tagName ) >= 0;
}


static bool isSpatialOperator( const QString& tagName )
{
  static QStringList spatialOps;
  if ( spatialOps.isEmpty() )
  {
    spatialOps << "BBOX" << "Intersects" << "Contains" << "Crosses" << "Equals"
    << "Disjoint" << "Overlaps" << "Touches" << "Within";
  }

  return spatialOps.contains( tagName );
}



QgsExpression::Node* QgsOgcUtils::nodeFromOgcFilter( QDomElement &element, QString &errorMessage )
{
  if ( element.isNull() )
    return nullptr;

  // check for binary operators
  if ( isBinaryOperator( element.tagName() ) )
  {
    return nodeBinaryOperatorFromOgcFilter( element, errorMessage );
  }

  // check for spatial operators
  if ( isSpatialOperator( element.tagName() ) )
  {
    return nodeSpatialOperatorFromOgcFilter( element, errorMessage );
  }

  // check for other OGC operators, convert them to expressions

  if ( element.tagName() == "Not" )
  {
    return nodeNotFromOgcFilter( element, errorMessage );
  }
  else if ( element.tagName() == "PropertyIsNull" )
  {
    return nodePropertyIsNullFromOgcFilter( element, errorMessage );
  }
  else if ( element.tagName() == "Literal" )
  {
    return nodeLiteralFromOgcFilter( element, errorMessage );
  }
  else if ( element.tagName() == "Function" )
  {
    return nodeFunctionFromOgcFilter( element, errorMessage );
  }
  else if ( element.tagName() == "PropertyName" )
  {
    return nodeColumnRefFromOgcFilter( element, errorMessage );
  }
  else if ( element.tagName() == "PropertyIsBetween" )
  {
    return nodeIsBetweenFromOgcFilter( element, errorMessage );
  }

  errorMessage += QObject::tr( "unable to convert '%1' element to a valid expression: it is not supported yet or it has invalid arguments" ).arg( element.tagName() );
  return nullptr;
}



QgsExpression::NodeBinaryOperator* QgsOgcUtils::nodeBinaryOperatorFromOgcFilter( QDomElement &element, QString &errorMessage )
{
  if ( element.isNull() )
    return nullptr;

  int op = binaryOperatorFromTagName( element.tagName() );
  if ( op < 0 )
  {
    if ( errorMessage.isEmpty() )
      errorMessage = QObject::tr( "'%1' binary operator not supported." ).arg( element.tagName() );
    return nullptr;
  }

  QDomElement operandElem = element.firstChildElement();
  QgsExpression::Node *expr = nodeFromOgcFilter( operandElem, errorMessage ), *leftOp = expr;
  if ( !expr )
  {
    if ( errorMessage.isEmpty() )
      errorMessage = QObject::tr( "invalid left operand for '%1' binary operator" ).arg( element.tagName() );
    return nullptr;
  }

  for ( operandElem = operandElem.nextSiblingElement(); !operandElem.isNull(); operandElem = operandElem.nextSiblingElement() )
  {
    QgsExpression::Node* opRight = nodeFromOgcFilter( operandElem, errorMessage );
    if ( !opRight )
    {
      if ( errorMessage.isEmpty() )
        errorMessage = QObject::tr( "invalid right operand for '%1' binary operator" ).arg( element.tagName() );
      delete expr;
      return nullptr;
    }

    expr = new QgsExpression::NodeBinaryOperator( static_cast< QgsExpression::BinaryOperator >( op ), expr, opRight );
  }

  if ( expr == leftOp )
  {
    if ( errorMessage.isEmpty() )
      errorMessage = QObject::tr( "only one operand for '%1' binary operator" ).arg( element.tagName() );
    delete expr;
    return nullptr;
  }

  QgsExpression::NodeBinaryOperator *ret = dynamic_cast< QgsExpression::NodeBinaryOperator * >( expr );
  if ( !ret )
    delete expr;

  return ret;
}


QgsExpression::NodeFunction* QgsOgcUtils::nodeSpatialOperatorFromOgcFilter( QDomElement& element, QString &errorMessage )
{
  // we are exploiting the fact that our function names are the same as the XML tag names
  int opIdx = QgsExpression::functionIndex( element.tagName().toLower() );

  QgsExpression::NodeList *gml2Args = new QgsExpression::NodeList();
  QDomElement childElem = element.firstChildElement();
  QString gml2Str;
  while ( !childElem.isNull() && gml2Str.isEmpty() )
  {
    if ( childElem.tagName() != "PropertyName" )
    {
      QTextStream gml2Stream( &gml2Str );
      childElem.save( gml2Stream, 0 );
    }
    childElem = childElem.nextSiblingElement();
  }
  if ( !gml2Str.isEmpty() )
  {
    gml2Args->append( new QgsExpression::NodeLiteral( QVariant( gml2Str.remove( '\n' ) ) ) );
  }
  else
  {
    errorMessage = QObject::tr( "No OGC Geometry found" );
    delete gml2Args;
    return nullptr;
  }

  QgsExpression::NodeList *opArgs = new QgsExpression::NodeList();
  opArgs->append( new QgsExpression::NodeFunction( QgsExpression::functionIndex( "$geometry" ), new QgsExpression::NodeList() ) );
  opArgs->append( new QgsExpression::NodeFunction( QgsExpression::functionIndex( "geomFromGML" ), gml2Args ) );

  return new QgsExpression::NodeFunction( opIdx, opArgs );
}


QgsExpression::NodeUnaryOperator* QgsOgcUtils::nodeNotFromOgcFilter( QDomElement &element, QString &errorMessage )
{
  if ( element.tagName() != "Not" )
    return nullptr;

  QDomElement operandElem = element.firstChildElement();
  QgsExpression::Node* operand = nodeFromOgcFilter( operandElem, errorMessage );
  if ( !operand )
  {
    if ( errorMessage.isEmpty() )
      errorMessage = QObject::tr( "invalid operand for '%1' unary operator" ).arg( element.tagName() );
    return nullptr;
  }

  return new QgsExpression::NodeUnaryOperator( QgsExpression::uoNot, operand );
}


QgsExpression::NodeFunction* QgsOgcUtils::nodeFunctionFromOgcFilter( QDomElement &element, QString &errorMessage )
{
  if ( element.isNull() || element.tagName() != "Function" )
  {
    errorMessage = QObject::tr( "ogc:Function expected, got %1" ).arg( element.tagName() );
    return nullptr;
  }

  for ( int i = 0; i < QgsExpression::Functions().size(); i++ )
  {
    QgsExpression::Function* funcDef = QgsExpression::Functions()[i];

    if ( element.attribute( "name" ) != funcDef->name() )
      continue;

    QgsExpression::NodeList *args = new QgsExpression::NodeList();

    QDomElement operandElem = element.firstChildElement();
    while ( !operandElem.isNull() )
    {
      QgsExpression::Node* op = nodeFromOgcFilter( operandElem, errorMessage );
      if ( !op )
      {
        delete args;
        return nullptr;
      }
      args->append( op );

      operandElem = operandElem.nextSiblingElement();
    }

    return new QgsExpression::NodeFunction( i, args );
  }

  return nullptr;
}



QgsExpression::Node* QgsOgcUtils::nodeLiteralFromOgcFilter( QDomElement &element, QString &errorMessage )
{
  if ( element.isNull() || element.tagName() != "Literal" )
  {
    errorMessage = QObject::tr( "ogc:Literal expected, got %1" ).arg( element.tagName() );
    return nullptr;
  }

  QgsExpression::Node *root = nullptr;

  // the literal content can have more children (e.g. CDATA section, text, ...)
  QDomNode childNode = element.firstChild();
  while ( !childNode.isNull() )
  {
    QgsExpression::Node* operand = nullptr;

    if ( childNode.nodeType() == QDomNode::ElementNode )
    {
      // found a element node (e.g. PropertyName), convert it
      QDomElement operandElem = childNode.toElement();
      operand = nodeFromOgcFilter( operandElem, errorMessage );
      if ( !operand )
      {
        if ( root )
          delete root;

        errorMessage = QObject::tr( "'%1' is an invalid or not supported content for ogc:Literal" ).arg( operandElem.tagName() );
        return nullptr;
      }
    }
    else
    {
      // probably a text/CDATA node
      QVariant value = childNode.nodeValue();

      // try to convert the node content to number if possible,
      // otherwise let's use it as string
      bool ok;
      double d = value.toDouble( &ok );
      if ( ok )
        value = d;

      operand = new QgsExpression::NodeLiteral( value );
      if ( !operand )
        continue;
    }

    // use the concat operator to merge the ogc:Literal children
    if ( !root )
    {
      root = operand;
    }
    else
    {
      root = new QgsExpression::NodeBinaryOperator( QgsExpression::boConcat, root, operand );
    }

    childNode = childNode.nextSibling();
  }

  if ( root )
    return root;

  return nullptr;
}


QgsExpression::NodeColumnRef* QgsOgcUtils::nodeColumnRefFromOgcFilter( QDomElement &element, QString &errorMessage )
{
  if ( element.isNull() || element.tagName() != "PropertyName" )
  {
    errorMessage = QObject::tr( "ogc:PropertyName expected, got %1" ).arg( element.tagName() );
    return nullptr;
  }

  return new QgsExpression::NodeColumnRef( element.firstChild().nodeValue() );
}


QgsExpression::Node* QgsOgcUtils::nodeIsBetweenFromOgcFilter( QDomElement& element, QString& errorMessage )
{
  // <ogc:PropertyIsBetween> encode a Range check
  QgsExpression::Node *operand = nullptr, *lowerBound = nullptr;
  QgsExpression::Node *operand2 = nullptr, *upperBound = nullptr;

  QDomElement operandElem = element.firstChildElement();
  while ( !operandElem.isNull() )
  {
    if ( operandElem.tagName() == "LowerBoundary" )
    {
      QDomElement lowerBoundElem = operandElem.firstChildElement();
      lowerBound = nodeFromOgcFilter( lowerBoundElem, errorMessage );
    }
    else if ( operandElem.tagName() ==  "UpperBoundary" )
    {
      QDomElement upperBoundElem = operandElem.firstChildElement();
      upperBound = nodeFromOgcFilter( upperBoundElem, errorMessage );
    }
    else
    {
      // <ogc:expression>
      // both operand and operand2 contain the same expression,
      // they are respectively compared to lower bound and upper bound
      operand = nodeFromOgcFilter( operandElem, errorMessage );
      operand2 = nodeFromOgcFilter( operandElem, errorMessage );
    }

    if ( operand && lowerBound && operand2 && upperBound )
      break;

    operandElem = operandElem.nextSiblingElement();
  }

  if ( !operand || !lowerBound || !operand2 || !upperBound )
  {
    if ( operand )
      delete operand;

    if ( lowerBound )
      delete lowerBound;

    if ( upperBound )
      delete upperBound;

    errorMessage = QObject::tr( "missing some required sub-elements in ogc:PropertyIsBetween" );
    return nullptr;
  }

  QgsExpression::Node *geOperator = new QgsExpression::NodeBinaryOperator( QgsExpression::boGE, operand, lowerBound );
  QgsExpression::Node *leOperator = new QgsExpression::NodeBinaryOperator( QgsExpression::boLE, operand2, upperBound );
  return new QgsExpression::NodeBinaryOperator( QgsExpression::boAnd, geOperator, leOperator );
}


QgsExpression::NodeBinaryOperator* QgsOgcUtils::nodePropertyIsNullFromOgcFilter( QDomElement& element, QString& errorMessage )
{
  // convert ogc:PropertyIsNull to IS operator with NULL right operand
  if ( element.tagName() != "PropertyIsNull" )
  {
    return nullptr;
  }

  QDomElement operandElem = element.firstChildElement();
  QgsExpression::Node* opLeft = nodeFromOgcFilter( operandElem, errorMessage );
  if ( !opLeft )
    return nullptr;

  QgsExpression::Node* opRight = new QgsExpression::NodeLiteral( QVariant() );
  return new QgsExpression::NodeBinaryOperator( QgsExpression::boIs, opLeft, opRight );
}


/////////////////


QDomElement QgsOgcUtils::expressionToOgcFilter( const QgsExpression& exp, QDomDocument& doc, QString* errorMessage )
{
  return expressionToOgcFilter( exp, doc, GML_2_1_2, FILTER_OGC_1_0,
                                "geometry", QString(), false, false, errorMessage );
}

QDomElement QgsOgcUtils::expressionToOgcFilter( const QgsExpression& exp,
    QDomDocument& doc,
    GMLVersion gmlVersion,
    FilterVersion filterVersion,
    const QString& geometryName,
    const QString& srsName,
    bool honourAxisOrientation,
    bool invertAxisOrientation,
    QString* errorMessage )
{
  if ( !exp.rootNode() )
    return QDomElement();

  QgsOgcUtilsExprToFilter utils( doc, gmlVersion, filterVersion, geometryName, srsName, honourAxisOrientation, invertAxisOrientation );
  QDomElement exprRootElem = utils.expressionNodeToOgcFilter( exp.rootNode() );
  if ( errorMessage )
    *errorMessage = utils.errorMessage();
  if ( exprRootElem.isNull() )
    return QDomElement();

  QDomElement filterElem =
    ( filterVersion == FILTER_FES_2_0 ) ?
    doc.createElementNS( FES_NAMESPACE, "fes:Filter" ) :
    doc.createElementNS( OGC_NAMESPACE, "ogc:Filter" );
  if ( utils.GMLNamespaceUsed() )
  {
    QDomAttr attr = doc.createAttribute( "xmlns:gml" );
    if ( gmlVersion == GML_3_2_1 )
      attr.setValue( GML32_NAMESPACE );
    else
      attr.setValue( GML_NAMESPACE );
    filterElem.setAttributeNode( attr );
  }
  filterElem.appendChild( exprRootElem );
  return filterElem;
}

QDomElement QgsOgcUtils::SQLStatementToOgcFilter( const QgsSQLStatement& statement,
    QDomDocument& doc,
    GMLVersion gmlVersion,
    FilterVersion filterVersion,
    const QList<LayerProperties>& layerProperties,
    bool honourAxisOrientation,
    bool invertAxisOrientation,
    const QMap< QString, QString>& mapUnprefixedTypenameToPrefixedTypename,
    QString* errorMessage )
{
  if ( !statement.rootNode() )
    return QDomElement();

  QgsOgcUtilsSQLStatementToFilter utils( doc, gmlVersion, filterVersion,
                                         layerProperties, honourAxisOrientation, invertAxisOrientation,
                                         mapUnprefixedTypenameToPrefixedTypename );
  QDomElement exprRootElem = utils.toOgcFilter( statement.rootNode() );
  if ( errorMessage )
    *errorMessage = utils.errorMessage();
  if ( exprRootElem.isNull() )
    return QDomElement();

  QDomElement filterElem =
    ( filterVersion == FILTER_FES_2_0 ) ?
    doc.createElementNS( FES_NAMESPACE, "fes:Filter" ) :
    doc.createElementNS( OGC_NAMESPACE, "ogc:Filter" );
  if ( utils.GMLNamespaceUsed() )
  {
    QDomAttr attr = doc.createAttribute( "xmlns:gml" );
    if ( gmlVersion == GML_3_2_1 )
      attr.setValue( GML32_NAMESPACE );
    else
      attr.setValue( GML_NAMESPACE );
    filterElem.setAttributeNode( attr );
  }
  filterElem.appendChild( exprRootElem );
  return filterElem;
}

//


QDomElement QgsOgcUtilsExprToFilter::expressionNodeToOgcFilter( const QgsExpression::Node* node )
{
  switch ( node->nodeType() )
  {
    case QgsExpression::ntUnaryOperator:
      return expressionUnaryOperatorToOgcFilter( static_cast<const QgsExpression::NodeUnaryOperator*>( node ) );
    case QgsExpression::ntBinaryOperator:
      return expressionBinaryOperatorToOgcFilter( static_cast<const QgsExpression::NodeBinaryOperator*>( node ) );
    case QgsExpression::ntInOperator:
      return expressionInOperatorToOgcFilter( static_cast<const QgsExpression::NodeInOperator*>( node ) );
    case QgsExpression::ntFunction:
      return expressionFunctionToOgcFilter( static_cast<const QgsExpression::NodeFunction*>( node ) );
    case QgsExpression::ntLiteral:
      return expressionLiteralToOgcFilter( static_cast<const QgsExpression::NodeLiteral*>( node ) );
    case QgsExpression::ntColumnRef:
      return expressionColumnRefToOgcFilter( static_cast<const QgsExpression::NodeColumnRef*>( node ) );

    default:
      mErrorMessage = QObject::tr( "Node type not supported: %1" ).arg( node->nodeType() );
      return QDomElement();
  }
}


QDomElement QgsOgcUtilsExprToFilter::expressionUnaryOperatorToOgcFilter( const QgsExpression::NodeUnaryOperator* node )
{

  QDomElement operandElem = expressionNodeToOgcFilter( node->operand() );
  if ( !mErrorMessage.isEmpty() )
    return QDomElement();

  QDomElement uoElem;
  switch ( node->op() )
  {
    case QgsExpression::uoMinus:
      uoElem = mDoc.createElement( mFilterPrefix + ":Literal" );
      if ( node->operand()->nodeType() == QgsExpression::ntLiteral )
      {
        // operand expression already created a Literal node:
        // take the literal value, prepend - and remove old literal node
        uoElem.appendChild( mDoc.createTextNode( "-" + operandElem.text() ) );
        mDoc.removeChild( operandElem );
      }
      else
      {
        mErrorMessage = QObject::tr( "This use of unary operator not implemented yet" );
        return QDomElement();
      }
      break;
    case QgsExpression::uoNot:
      uoElem = mDoc.createElement( mFilterPrefix + ":Not" );
      uoElem.appendChild( operandElem );
      break;

    default:
      mErrorMessage = QObject::tr( "Unary operator %1 not implemented yet" ).arg( QgsExpression::UnaryOperatorText[node->op()] );
      return QDomElement();
  }

  return uoElem;
}


QDomElement QgsOgcUtilsExprToFilter::expressionBinaryOperatorToOgcFilter( const QgsExpression::NodeBinaryOperator* node )
{
  QDomElement leftElem = expressionNodeToOgcFilter( node->opLeft() );
  if ( !mErrorMessage.isEmpty() )
    return QDomElement();

  QgsExpression::BinaryOperator op = node->op();

  // before right operator is parsed: to allow NULL handling
  if ( op == QgsExpression::boIs || op == QgsExpression::boIsNot )
  {
    if ( node->opRight()->nodeType() == QgsExpression::ntLiteral )
    {
      const QgsExpression::NodeLiteral* rightLit = static_cast<const QgsExpression::NodeLiteral*>( node->opRight() );
      if ( rightLit->value().isNull() )
      {

        QDomElement elem = mDoc.createElement( mFilterPrefix + ":PropertyIsNull" );
        elem.appendChild( leftElem );

        if ( op == QgsExpression::boIsNot )
        {
          QDomElement notElem = mDoc.createElement( mFilterPrefix + ":Not" );
          notElem.appendChild( elem );
          return notElem;
        }

        return elem;
      }

      // continue with equal / not equal operator once the null case is handled
      op = ( op == QgsExpression::boIs ? QgsExpression::boEQ : QgsExpression::boNE );
    }

  }

  QDomElement rightElem = expressionNodeToOgcFilter( node->opRight() );
  if ( !mErrorMessage.isEmpty() )
    return QDomElement();


  QString opText = binaryOperatorToTagName( op );
  if ( opText.isEmpty() )
  {
    // not implemented binary operators
    // TODO: regex, % (mod), ^ (pow) are not supported yet
    mErrorMessage = QObject::tr( "Binary operator %1 not implemented yet" ).arg( QgsExpression::BinaryOperatorText[op] );
    return QDomElement();
  }

  QDomElement boElem = mDoc.createElement( mFilterPrefix + ":" + opText );

  if ( op == QgsExpression::boLike || op == QgsExpression::boILike )
  {
    if ( op == QgsExpression::boILike )
      boElem.setAttribute( "matchCase", "false" );

    // setup wildcards to <ogc:PropertyIsLike>
    boElem.setAttribute( "wildCard", "%" );
    boElem.setAttribute( "singleChar", "?" );
    if ( mFilterVersion == QgsOgcUtils::FILTER_OGC_1_0 )
      boElem.setAttribute( "escape", "!" );
    else
      boElem.setAttribute( "escapeChar", "!" );
  }

  boElem.appendChild( leftElem );
  boElem.appendChild( rightElem );
  return boElem;
}


QDomElement QgsOgcUtilsExprToFilter::expressionLiteralToOgcFilter( const QgsExpression::NodeLiteral* node )
{
  QString value;
  switch ( node->value().type() )
  {
    case QVariant::Int:
      value = QString::number( node->value().toInt() );
      break;
    case QVariant::Double:
      value = qgsDoubleToString( node->value().toDouble() );
      break;
    case QVariant::String:
      value = node->value().toString();
      break;

    default:
      mErrorMessage = QObject::tr( "Literal type not supported: %1" ).arg( node->value().type() );
      return QDomElement();
  }

  QDomElement litElem = mDoc.createElement( mFilterPrefix + ":Literal" );
  litElem.appendChild( mDoc.createTextNode( value ) );
  return litElem;
}


QDomElement QgsOgcUtilsExprToFilter::expressionColumnRefToOgcFilter( const QgsExpression::NodeColumnRef* node )
{
  QDomElement propElem = mDoc.createElement( mFilterPrefix + ":" + mPropertyName );
  propElem.appendChild( mDoc.createTextNode( node->name() ) );
  return propElem;
}



QDomElement QgsOgcUtilsExprToFilter::expressionInOperatorToOgcFilter( const QgsExpression::NodeInOperator* node )
{
  if ( node->list()->list().size() == 1 )
    return expressionNodeToOgcFilter( node->list()->list()[0] );

  QDomElement orElem = mDoc.createElement( mFilterPrefix + ":Or" );
  QDomElement leftNode = expressionNodeToOgcFilter( node->node() );

  Q_FOREACH ( QgsExpression::Node* n, node->list()->list() )
  {
    QDomElement listNode = expressionNodeToOgcFilter( n );
    if ( !mErrorMessage.isEmpty() )
      return QDomElement();

    QDomElement eqElem = mDoc.createElement( mFilterPrefix + ":PropertyIsEqualTo" );
    eqElem.appendChild( leftNode.cloneNode() );
    eqElem.appendChild( listNode );

    orElem.appendChild( eqElem );
  }

  if ( node->isNotIn() )
  {
    QDomElement notElem = mDoc.createElement( mFilterPrefix + ":Not" );
    notElem.appendChild( orElem );
    return notElem;
  }

  return orElem;
}

static QMap<QString, QString> binarySpatialOpsMap()
{
  static QMap<QString, QString> binSpatialOps;
  if ( binSpatialOps.isEmpty() )
  {
    binSpatialOps.insert( "disjoint", "Disjoint" );
    binSpatialOps.insert( "intersects", "Intersects" );
    binSpatialOps.insert( "touches", "Touches" );
    binSpatialOps.insert( "crosses", "Crosses" );
    binSpatialOps.insert( "contains", "Contains" );
    binSpatialOps.insert( "overlaps", "Overlaps" );
    binSpatialOps.insert( "within", "Within" );
  }
  return binSpatialOps;
}

static bool isBinarySpatialOperator( const QString& fnName )
{
  return binarySpatialOpsMap().contains( fnName );
}

static QString tagNameForSpatialOperator( const QString& fnName )
{
  return binarySpatialOpsMap().value( fnName );
}

static bool isGeometryColumn( const QgsExpression::Node* node )
{
  if ( node->nodeType() != QgsExpression::ntFunction )
    return false;

  const QgsExpression::NodeFunction* fn = static_cast<const QgsExpression::NodeFunction*>( node );
  QgsExpression::Function* fd = QgsExpression::Functions()[fn->fnIndex()];
  return fd->name() == "$geometry";
}

static QgsGeometry* geometryFromConstExpr( const QgsExpression::Node* node )
{
  // Right now we support only geomFromWKT(' ..... ')
  // Ideally we should support any constant sub-expression (not dependent on feature's geometry or attributes)

  if ( node->nodeType() == QgsExpression::ntFunction )
  {
    const QgsExpression::NodeFunction* fnNode = static_cast<const QgsExpression::NodeFunction*>( node );
    QgsExpression::Function* fnDef = QgsExpression::Functions()[fnNode->fnIndex()];
    if ( fnDef->name() == "geom_from_wkt" )
    {
      const QList<QgsExpression::Node*>& args = fnNode->args()->list();
      if ( args[0]->nodeType() == QgsExpression::ntLiteral )
      {
        QString wkt = static_cast<const QgsExpression::NodeLiteral*>( args[0] )->value().toString();
        return QgsGeometry::fromWkt( wkt );
      }
    }
  }
  return nullptr;
}


QDomElement QgsOgcUtilsExprToFilter::expressionFunctionToOgcFilter( const QgsExpression::NodeFunction* node )
{
  QgsExpression::Function* fd = QgsExpression::Functions()[node->fnIndex()];

  if ( fd->name() == "intersects_bbox" )
  {
    QList<QgsExpression::Node*> argNodes = node->args()->list();
    Q_ASSERT( argNodes.count() == 2 ); // binary spatial ops must have two args

    QgsGeometry* geom = geometryFromConstExpr( argNodes[1] );
    if ( geom && isGeometryColumn( argNodes[0] ) )
    {
      QgsRectangle rect = geom->boundingBox();
      delete geom;

      mGMLUsed = true;

      QDomElement elemBox = ( mGMLVersion == QgsOgcUtils::GML_2_1_2 ) ?
                            QgsOgcUtils::rectangleToGMLBox( &rect, mDoc, mSrsName, mInvertAxisOrientation ) :
                            QgsOgcUtils::rectangleToGMLEnvelope( &rect, mDoc, mSrsName, mInvertAxisOrientation );

      QDomElement geomProperty = mDoc.createElement( mFilterPrefix + ":" + mPropertyName );
      geomProperty.appendChild( mDoc.createTextNode( mGeometryName ) );

      QDomElement funcElem = mDoc.createElement( mFilterPrefix + ":BBOX" );
      funcElem.appendChild( geomProperty );
      funcElem.appendChild( elemBox );
      return funcElem;
    }
    else
    {
      delete geom;

      mErrorMessage = QObject::tr( "<BBOX> is currently supported only in form: bbox($geometry, geomFromWKT('...'))" );
      return QDomElement();
    }
  }

  if ( isBinarySpatialOperator( fd->name() ) )
  {
    QList<QgsExpression::Node*> argNodes = node->args()->list();
    Q_ASSERT( argNodes.count() == 2 ); // binary spatial ops must have two args

    QgsExpression::Node* otherNode = nullptr;
    if ( isGeometryColumn( argNodes[0] ) )
      otherNode = argNodes[1];
    else if ( isGeometryColumn( argNodes[1] ) )
      otherNode = argNodes[0];
    else
    {
      mErrorMessage = QObject::tr( "Unable to translate spatial operator: at least one must refer to geometry." );
      return QDomElement();
    }

    QDomElement otherGeomElem;

    // the other node must be a geometry constructor
    if ( otherNode->nodeType() != QgsExpression::ntFunction )
    {
      mErrorMessage = QObject::tr( "spatial operator: the other operator must be a geometry constructor function" );
      return QDomElement();
    }

    const QgsExpression::NodeFunction* otherFn = static_cast<const QgsExpression::NodeFunction*>( otherNode );
    QgsExpression::Function* otherFnDef = QgsExpression::Functions()[otherFn->fnIndex()];
    if ( otherFnDef->name() == "geom_from_wkt" )
    {
      QgsExpression::Node* firstFnArg = otherFn->args()->list()[0];
      if ( firstFnArg->nodeType() != QgsExpression::ntLiteral )
      {
        mErrorMessage = QObject::tr( "geom_from_wkt: argument must be string literal" );
        return QDomElement();
      }
      QString wkt = static_cast<const QgsExpression::NodeLiteral*>( firstFnArg )->value().toString();
      QgsGeometry* geom = QgsGeometry::fromWkt( wkt );
      otherGeomElem = QgsOgcUtils::geometryToGML( geom, mDoc, mGMLVersion, mSrsName, mInvertAxisOrientation,
                      QString( "qgis_id_geom_%1" ).arg( mGeomId ) );
      mGeomId ++;
      delete geom;
    }
    else if ( otherFnDef->name() == "geom_from_gml" )
    {
      QgsExpression::Node* firstFnArg = otherFn->args()->list()[0];
      if ( firstFnArg->nodeType() != QgsExpression::ntLiteral )
      {
        mErrorMessage = QObject::tr( "geom_from_gml: argument must be string literal" );
        return QDomElement();
      }

      QDomDocument geomDoc;
      QString gml = static_cast<const QgsExpression::NodeLiteral*>( firstFnArg )->value().toString();
      if ( !geomDoc.setContent( gml, true ) )
      {
        mErrorMessage = QObject::tr( "geom_from_gml: unable to parse XML" );
        return QDomElement();
      }

      QDomNode geomNode = mDoc.importNode( geomDoc.documentElement(), true );
      otherGeomElem = geomNode.toElement();
    }
    else
    {
      mErrorMessage = QObject::tr( "spatial operator: unknown geometry constructor function" );
      return QDomElement();
    }

    mGMLUsed = true;

    QDomElement funcElem = mDoc.createElement( mFilterPrefix + ":" + tagNameForSpatialOperator( fd->name() ) );
    QDomElement geomProperty = mDoc.createElement( mFilterPrefix + ":" + mPropertyName );
    geomProperty.appendChild( mDoc.createTextNode( mGeometryName ) );
    funcElem.appendChild( geomProperty );
    funcElem.appendChild( otherGeomElem );
    return funcElem;
  }

  if ( fd->params() == 0 )
  {
    mErrorMessage = QObject::tr( "Special columns/constants are not supported." );
    return QDomElement();
  }

  // this is somehow wrong - we are just hoping that the other side supports the same functions as we do...
  QDomElement funcElem = mDoc.createElement( mFilterPrefix + ":Function" );
  funcElem.setAttribute( "name", fd->name() );
  Q_FOREACH ( QgsExpression::Node* n, node->args()->list() )
  {
    QDomElement childElem = expressionNodeToOgcFilter( n );
    if ( !mErrorMessage.isEmpty() )
      return QDomElement();

    funcElem.appendChild( childElem );
  }

  return funcElem;
}

//

QgsOgcUtilsSQLStatementToFilter::QgsOgcUtilsSQLStatementToFilter( QDomDocument& doc,
    QgsOgcUtils::GMLVersion gmlVersion,
    QgsOgcUtils::FilterVersion filterVersion,
    const QList<QgsOgcUtils::LayerProperties>& layerProperties,
    bool honourAxisOrientation,
    bool invertAxisOrientation,
    const QMap< QString, QString>& mapUnprefixedTypenameToPrefixedTypename )
    : mDoc( doc )
    , mGMLUsed( false )
    , mGMLVersion( gmlVersion )
    , mFilterVersion( filterVersion )
    , mLayerProperties( layerProperties )
    , mHonourAxisOrientation( honourAxisOrientation )
    , mInvertAxisOrientation( invertAxisOrientation )
    , mFilterPrefix(( filterVersion == QgsOgcUtils::FILTER_FES_2_0 ) ? "fes" : "ogc" )
    , mPropertyName(( filterVersion == QgsOgcUtils::FILTER_FES_2_0 ) ? "ValueReference" : "PropertyName" )
    , mGeomId( 1 )
    , mMapUnprefixedTypenameToPrefixedTypename( mapUnprefixedTypenameToPrefixedTypename )
{
}

QDomElement QgsOgcUtilsSQLStatementToFilter::toOgcFilter( const QgsSQLStatement::Node* node )
{
  switch ( node->nodeType() )
  {
    case QgsSQLStatement::ntUnaryOperator:
      return toOgcFilter( static_cast<const QgsSQLStatement::NodeUnaryOperator*>( node ) );
    case QgsSQLStatement::ntBinaryOperator:
      return toOgcFilter( static_cast<const QgsSQLStatement::NodeBinaryOperator*>( node ) );
    case QgsSQLStatement::ntInOperator:
      return toOgcFilter( static_cast<const QgsSQLStatement::NodeInOperator*>( node ) );
    case QgsSQLStatement::ntBetweenOperator:
      return toOgcFilter( static_cast<const QgsSQLStatement::NodeBetweenOperator*>( node ) );
    case QgsSQLStatement::ntFunction:
      return toOgcFilter( static_cast<const QgsSQLStatement::NodeFunction*>( node ) );
    case QgsSQLStatement::ntLiteral:
      return toOgcFilter( static_cast<const QgsSQLStatement::NodeLiteral*>( node ) );
    case QgsSQLStatement::ntColumnRef:
      return toOgcFilter( static_cast<const QgsSQLStatement::NodeColumnRef*>( node ) );
    case QgsSQLStatement::ntSelect:
      return toOgcFilter( static_cast<const QgsSQLStatement::NodeSelect*>( node ) );

    default:
      mErrorMessage = QObject::tr( "Node type not supported: %1" ).arg( node->nodeType() );
      return QDomElement();
  }
}


QDomElement QgsOgcUtilsSQLStatementToFilter::toOgcFilter( const QgsSQLStatement::NodeUnaryOperator* node )
{

  QDomElement operandElem = toOgcFilter( node->operand() );
  if ( !mErrorMessage.isEmpty() )
    return QDomElement();

  QDomElement uoElem;
  switch ( node->op() )
  {
    case QgsSQLStatement::uoMinus:
      uoElem = mDoc.createElement( mFilterPrefix + ":Literal" );
      if ( node->operand()->nodeType() == QgsSQLStatement::ntLiteral )
      {
        // operand expression already created a Literal node:
        // take the literal value, prepend - and remove old literal node
        uoElem.appendChild( mDoc.createTextNode( "-" + operandElem.text() ) );
        mDoc.removeChild( operandElem );
      }
      else
      {
        mErrorMessage = QObject::tr( "This use of unary operator not implemented yet" );
        return QDomElement();
      }
      break;
    case QgsSQLStatement::uoNot:
      uoElem = mDoc.createElement( mFilterPrefix + ":Not" );
      uoElem.appendChild( operandElem );
      break;

    default:
      mErrorMessage = QObject::tr( "Unary operator %1 not implemented yet" ).arg( QgsSQLStatement::UnaryOperatorText[node->op()] );
      return QDomElement();
  }

  return uoElem;
}


QDomElement QgsOgcUtilsSQLStatementToFilter::toOgcFilter( const QgsSQLStatement::NodeBinaryOperator* node )
{
  QDomElement leftElem = toOgcFilter( node->opLeft() );
  if ( !mErrorMessage.isEmpty() )
    return QDomElement();

  QgsSQLStatement::BinaryOperator op = node->op();

  // before right operator is parsed: to allow NULL handling
  if ( op == QgsSQLStatement::boIs || op == QgsSQLStatement::boIsNot )
  {
    if ( node->opRight()->nodeType() == QgsSQLStatement::ntLiteral )
    {
      const QgsSQLStatement::NodeLiteral* rightLit = static_cast<const QgsSQLStatement::NodeLiteral*>( node->opRight() );
      if ( rightLit->value().isNull() )
      {

        QDomElement elem = mDoc.createElement( mFilterPrefix + ":PropertyIsNull" );
        elem.appendChild( leftElem );

        if ( op == QgsSQLStatement::boIsNot )
        {
          QDomElement notElem = mDoc.createElement( mFilterPrefix + ":Not" );
          notElem.appendChild( elem );
          return notElem;
        }

        return elem;
      }

      // continue with equal / not equal operator once the null case is handled
      op = ( op == QgsSQLStatement::boIs ? QgsSQLStatement::boEQ : QgsSQLStatement::boNE );
    }

  }

  QDomElement rightElem = toOgcFilter( node->opRight() );
  if ( !mErrorMessage.isEmpty() )
    return QDomElement();


  QString opText;
  if ( op == QgsSQLStatement::boOr )
    opText = "Or";
  else if ( op == QgsSQLStatement::boAnd )
    opText = "And";
  else if ( op == QgsSQLStatement::boEQ )
    opText = "PropertyIsEqualTo";
  else if ( op == QgsSQLStatement::boNE )
    opText = "PropertyIsNotEqualTo";
  else if ( op == QgsSQLStatement::boLE )
    opText = "PropertyIsLessThanOrEqualTo";
  else if ( op == QgsSQLStatement::boGE )
    opText = "PropertyIsGreaterThanOrEqualTo";
  else if ( op == QgsSQLStatement::boLT )
    opText = "PropertyIsLessThan";
  else if ( op == QgsSQLStatement::boGT )
    opText = "PropertyIsGreaterThan";
  else if ( op == QgsSQLStatement::boLike )
    opText = "PropertyIsLike";

  if ( opText.isEmpty() )
  {
    // not implemented binary operators
    mErrorMessage = QObject::tr( "Binary operator %1 not implemented yet" ).arg( QgsSQLStatement::BinaryOperatorText[op] );
    return QDomElement();
  }

  QDomElement boElem = mDoc.createElement( mFilterPrefix + ":" + opText );

  if ( op == QgsSQLStatement::boLike || op == QgsSQLStatement::boILike )
  {
    if ( op == QgsSQLStatement::boILike )
      boElem.setAttribute( "matchCase", "false" );

    // setup wildcards to <ogc:PropertyIsLike>
    boElem.setAttribute( "wildCard", "%" );
    boElem.setAttribute( "singleChar", "?" );
    if ( mFilterVersion == QgsOgcUtils::FILTER_OGC_1_0 )
      boElem.setAttribute( "escape", "!" );
    else
      boElem.setAttribute( "escapeChar", "!" );
  }

  boElem.appendChild( leftElem );
  boElem.appendChild( rightElem );
  return boElem;
}


QDomElement QgsOgcUtilsSQLStatementToFilter::toOgcFilter( const QgsSQLStatement::NodeLiteral* node )
{
  QString value;
  switch ( node->value().type() )
  {
    case QVariant::Int:
      value = QString::number( node->value().toInt() );
      break;
    case QVariant::LongLong:
      value = QString::number( node->value().toLongLong() );
      break;
    case QVariant::Double:
      value = qgsDoubleToString( node->value().toDouble() );
      break;
    case QVariant::String:
      value = node->value().toString();
      break;

    default:
      mErrorMessage = QObject::tr( "Literal type not supported: %1" ).arg( node->value().type() );
      return QDomElement();
  }

  QDomElement litElem = mDoc.createElement( mFilterPrefix + ":Literal" );
  litElem.appendChild( mDoc.createTextNode( value ) );
  return litElem;
}


QDomElement QgsOgcUtilsSQLStatementToFilter::toOgcFilter( const QgsSQLStatement::NodeColumnRef* node )
{
  QDomElement propElem = mDoc.createElement( mFilterPrefix + ":" + mPropertyName );
  if ( node->tableName().isEmpty() || mLayerProperties.size() == 1 )
    propElem.appendChild( mDoc.createTextNode( node->name() ) );
  else
  {
    QString tableName( mMapTableAliasToNames[node->tableName()] );
    if ( mMapUnprefixedTypenameToPrefixedTypename.contains( tableName ) )
      tableName = mMapUnprefixedTypenameToPrefixedTypename[tableName];
    propElem.appendChild( mDoc.createTextNode( tableName + "/" + node->name() ) );
  }
  return propElem;
}

QDomElement QgsOgcUtilsSQLStatementToFilter::toOgcFilter( const QgsSQLStatement::NodeInOperator* node )
{
  if ( node->list()->list().size() == 1 )
    return toOgcFilter( node->list()->list()[0] );

  QDomElement orElem = mDoc.createElement( mFilterPrefix + ":Or" );
  QDomElement leftNode = toOgcFilter( node->node() );

  Q_FOREACH ( QgsSQLStatement::Node* n, node->list()->list() )
  {
    QDomElement listNode = toOgcFilter( n );
    if ( !mErrorMessage.isEmpty() )
      return QDomElement();

    QDomElement eqElem = mDoc.createElement( mFilterPrefix + ":PropertyIsEqualTo" );
    eqElem.appendChild( leftNode.cloneNode() );
    eqElem.appendChild( listNode );

    orElem.appendChild( eqElem );
  }

  if ( node->isNotIn() )
  {
    QDomElement notElem = mDoc.createElement( mFilterPrefix + ":Not" );
    notElem.appendChild( orElem );
    return notElem;
  }

  return orElem;
}

QDomElement QgsOgcUtilsSQLStatementToFilter::toOgcFilter( const QgsSQLStatement::NodeBetweenOperator* node )
{
  QDomElement elem = mDoc.createElement( mFilterPrefix + ":PropertyIsBetween" );
  elem.appendChild( toOgcFilter( node->node() ) );
  QDomElement lowerBoundary = mDoc.createElement( mFilterPrefix + ":LowerBoundary" );
  lowerBoundary.appendChild( toOgcFilter( node->minVal() ) );
  elem.appendChild( lowerBoundary );
  QDomElement upperBoundary = mDoc.createElement( mFilterPrefix + ":UpperBoundary" );
  upperBoundary.appendChild( toOgcFilter( node->maxVal() ) );
  elem.appendChild( upperBoundary );

  if ( node->isNotBetween() )
  {
    QDomElement notElem = mDoc.createElement( mFilterPrefix + ":Not" );
    notElem.appendChild( elem );
    return notElem;
  }

  return elem;
}

static QString mapBinarySpatialToOgc( const QString& name )
{
  QString nameCompare( name );
  if ( name.size() > 3 && name.mid( 0, 3 ).compare( "ST_", Qt::CaseInsensitive ) == 0 )
    nameCompare = name.mid( 3 );
  QStringList spatialOps;
  spatialOps << "BBOX" << "Intersects" << "Contains" << "Crosses" << "Equals"
  << "Disjoint" << "Overlaps" << "Touches" << "Within";
  Q_FOREACH ( QString op, spatialOps )
  {
    if ( nameCompare.compare( op, Qt::CaseInsensitive ) == 0 )
      return op;
  }
  return QString();
}

static QString mapTernarySpatialToOgc( const QString& name )
{
  QString nameCompare( name );
  if ( name.size() > 3 && name.mid( 0, 3 ).compare( "ST_", Qt::CaseInsensitive ) == 0 )
    nameCompare = name.mid( 3 );
  if ( nameCompare.compare( "DWithin", Qt::CaseInsensitive ) == 0 )
    return "DWithin";
  if ( nameCompare.compare( "Beyond", Qt::CaseInsensitive ) == 0 )
    return "Beyond";
  return QString();
}

QString QgsOgcUtilsSQLStatementToFilter::getGeometryColumnSRSName( const QgsSQLStatement::Node* node )
{
  if ( node->nodeType() != QgsSQLStatement::ntColumnRef )
    return QString();

  const QgsSQLStatement::NodeColumnRef* col = static_cast<const QgsSQLStatement::NodeColumnRef*>( node );
  if ( !col->tableName().isEmpty() )
  {
    Q_FOREACH ( QgsOgcUtils::LayerProperties prop, mLayerProperties )
    {
      if ( prop.mName.compare( mMapTableAliasToNames[col->tableName()], Qt::CaseInsensitive ) == 0 &&
           prop.mGeometryAttribute.compare( col->name(), Qt::CaseInsensitive ) == 0 )
      {
        return prop.mSRSName;
      }
    }
  }
  if ( mLayerProperties.size() != 0 &&
       mLayerProperties.at( 0 ).mGeometryAttribute.compare( col->name(), Qt::CaseInsensitive ) == 0 )
  {
    return  mLayerProperties.at( 0 ).mSRSName;
  }
  return QString();
}

bool QgsOgcUtilsSQLStatementToFilter::processSRSName( const QgsSQLStatement::NodeFunction* mainNode,
    QList<QgsSQLStatement::Node*> args,
    bool lastArgIsSRSName,
    QString& srsName,
    bool& axisInversion )
{
  srsName = mCurrentSRSName;
  axisInversion = mInvertAxisOrientation;

  if ( lastArgIsSRSName )
  {
    QgsSQLStatement::Node* lastArg = args[ args.size() - 1 ];
    if ( lastArg->nodeType() != QgsSQLStatement::ntLiteral )
    {
      mErrorMessage = QObject::tr( "%1: Last argument must be string or integer literal" ).arg( mainNode->name() );
      return false;
    }
    const QgsSQLStatement::NodeLiteral* lit = static_cast<const QgsSQLStatement::NodeLiteral*>( lastArg );
    if ( lit->value().type() == QVariant::Int )
    {
      if ( mFilterVersion == QgsOgcUtils::FILTER_OGC_1_0 )
      {
        srsName = "EPSG:" + QString::number( lit->value().toInt() );
      }
      else
      {
        srsName = "urn:ogc:def:crs:EPSG::" + QString::number( lit->value().toInt() );
      }
    }
    else
    {
      srsName = lit->value().toString();
      if ( srsName.startsWith( "EPSG:", Qt::CaseInsensitive ) )
        return true;
    }
  }

  QgsCoordinateReferenceSystem crs;
  if ( !srsName.isEmpty() )
    crs = QgsCRSCache::instance()->crsByOgcWmsCrs( srsName );
  if ( crs.isValid() )
  {
    if ( mHonourAxisOrientation && crs.axisInverted() )
    {
      axisInversion = !axisInversion;
    }
  }

  return true;
}

QDomElement QgsOgcUtilsSQLStatementToFilter::toOgcFilter( const QgsSQLStatement::NodeFunction* node )
{
  // ST_GeometryFromText
  if ( node->name().compare( "ST_GeometryFromText", Qt::CaseInsensitive ) == 0 )
  {
    QList<QgsSQLStatement::Node*> args = node->args()->list();
    if ( args.size() != 1 && args.size() != 2 )
    {
      mErrorMessage = QObject::tr( "Function %1 should have 1 or 2 arguments" ).arg( node->name() );
      return QDomElement();
    }

    QgsSQLStatement::Node* firstFnArg = args[0];
    if ( firstFnArg->nodeType() != QgsSQLStatement::ntLiteral )
    {
      mErrorMessage = QObject::tr( "%1: First argument must be string literal" ).arg( node->name() );
      return QDomElement();
    }

    QString srsName;
    bool axisInversion;
    if ( ! processSRSName( node, args, args.size() == 2, srsName, axisInversion ) )
    {
      return QDomElement();
    }

    QString wkt = static_cast<const QgsSQLStatement::NodeLiteral*>( firstFnArg )->value().toString();
    QgsGeometry* geom = QgsGeometry::fromWkt( wkt );
    QDomElement geomElem = QgsOgcUtils::geometryToGML( geom, mDoc, mGMLVersion, srsName, axisInversion,
                           QString( "qgis_id_geom_%1" ).arg( mGeomId ) );
    mGeomId ++;
    delete geom;
    if ( geomElem.isNull() )
    {
      mErrorMessage = QObject::tr( "%1: invalid WKT" ).arg( node->name() );
      return QDomElement();
    }
    mGMLUsed = true;
    return geomElem;
  }

  // ST_MakeEnvelope
  if ( node->name().compare( "ST_MakeEnvelope", Qt::CaseInsensitive ) == 0 )
  {
    QList<QgsSQLStatement::Node*> args = node->args()->list();
    if ( args.size() != 4 && args.size() != 5 )
    {
      mErrorMessage = QObject::tr( "Function %1 should have 4 or 5 arguments" ).arg( node->name() );
      return QDomElement();
    }

    QgsRectangle rect;

    for ( int i = 0; i < 4;i++ )
    {
      QgsSQLStatement::Node* arg = args[i];
      if ( arg->nodeType() != QgsSQLStatement::ntLiteral )
      {
        mErrorMessage = QObject::tr( "%1: Argument %2 must be numeric literal" ).arg( node->name() ).arg( i + 1 );
        return QDomElement();
      }
      const QgsSQLStatement::NodeLiteral* lit = static_cast<const QgsSQLStatement::NodeLiteral*>( arg );
      double val = 0.0;
      if ( lit->value().type() == QVariant::Int )
        val = lit->value().toInt();
      else if ( lit->value().type() == QVariant::LongLong )
        val = lit->value().toLongLong();
      else if ( lit->value().type() == QVariant::Double )
        val = lit->value().toDouble();
      else
      {
        mErrorMessage = QObject::tr( "%1 Argument %2 must be numeric literal" ).arg( node->name() ).arg( i + 1 );
        return QDomElement();
      }
      if ( i == 0 )
        rect.setXMinimum( val );
      else if ( i == 1 )
        rect.setYMinimum( val );
      else if ( i == 2 )
        rect.setXMaximum( val );
      else
        rect.setYMaximum( val );
    }

    QString srsName;
    bool axisInversion;
    if ( ! processSRSName( node, args, args.size() == 5, srsName, axisInversion ) )
    {
      return QDomElement();
    }

    mGMLUsed = true;

    return ( mGMLVersion == QgsOgcUtils::GML_2_1_2 ) ?
           QgsOgcUtils::rectangleToGMLBox( &rect, mDoc, srsName, axisInversion, 15 ) :
           QgsOgcUtils::rectangleToGMLEnvelope( &rect, mDoc, srsName, axisInversion, 15 );
  }

  // ST_GeomFromGML
  if ( node->name().compare( "ST_GeomFromGML", Qt::CaseInsensitive ) == 0 )
  {
    QList<QgsSQLStatement::Node*> args = node->args()->list();
    if ( args.size() != 1 )
    {
      mErrorMessage = QObject::tr( "Function %1 should have 1 argument" ).arg( node->name() );
      return QDomElement();
    }

    QgsSQLStatement::Node* firstFnArg = args[0];
    if ( firstFnArg->nodeType() != QgsSQLStatement::ntLiteral )
    {
      mErrorMessage = QObject::tr( "%1: Argument must be string literal" ).arg( node->name() );
      return QDomElement();
    }

    QDomDocument geomDoc;
    QString gml = static_cast<const QgsSQLStatement::NodeLiteral*>( firstFnArg )->value().toString();
    if ( !geomDoc.setContent( gml, true ) )
    {
      mErrorMessage = QObject::tr( "ST_GeomFromGML: unable to parse XML" );
      return QDomElement();
    }

    QDomNode geomNode = mDoc.importNode( geomDoc.documentElement(), true );
    mGMLUsed = true;
    return geomNode.toElement();
  }

  // Binary geometry operators
  QString ogcName( mapBinarySpatialToOgc( node->name() ) );
  if ( !ogcName.isEmpty() )
  {
    QList<QgsSQLStatement::Node*> args = node->args()->list();
    if ( args.size() != 2 )
    {
      mErrorMessage = QObject::tr( "Function %1 should have 2 arguments" ).arg( node->name() );
      return QDomElement();
    }

    for ( int i = 0; i < 2; i ++ )
    {
      if ( args[i]->nodeType() == QgsSQLStatement::ntFunction &&
           ( static_cast<const QgsSQLStatement::NodeFunction*>( args[i] )->name().compare( "ST_GeometryFromText", Qt::CaseInsensitive ) == 0 ||
             static_cast<const QgsSQLStatement::NodeFunction*>( args[i] )->name().compare( "ST_MakeEnvelope", Qt::CaseInsensitive ) == 0 ) )
      {
        mCurrentSRSName = getGeometryColumnSRSName( args[1-i] );
        break;
      }
    }

    //if( ogcName == "Intersects" && mFilterVersion == QgsOgcUtils::FILTER_OGC_1_0 )
    //  ogcName = "Intersect";
    QDomElement funcElem = mDoc.createElement( mFilterPrefix + ":" + ogcName );
    Q_FOREACH ( QgsSQLStatement::Node* n, args )
    {
      QDomElement childElem = toOgcFilter( n );
      if ( !mErrorMessage.isEmpty() )
      {
        mCurrentSRSName.clear();
        return QDomElement();
      }

      funcElem.appendChild( childElem );
    }

    mCurrentSRSName.clear();
    return funcElem;
  }

  ogcName = mapTernarySpatialToOgc( node->name() );
  if ( !ogcName.isEmpty() )
  {
    QList<QgsSQLStatement::Node*> args = node->args()->list();
    if ( args.size() != 3 )
    {
      mErrorMessage = QObject::tr( "Function %1 should have 3 arguments" ).arg( node->name() );
      return QDomElement();
    }

    for ( int i = 0; i < 2; i ++ )
    {
      if ( args[i]->nodeType() == QgsSQLStatement::ntFunction &&
           ( static_cast<const QgsSQLStatement::NodeFunction*>( args[i] )->name().compare( "ST_GeometryFromText", Qt::CaseInsensitive ) == 0 ||
             static_cast<const QgsSQLStatement::NodeFunction*>( args[i] )->name().compare( "ST_MakeEnvelope", Qt::CaseInsensitive ) == 0 ) )
      {
        mCurrentSRSName = getGeometryColumnSRSName( args[1-i] );
        break;
      }
    }

    QDomElement funcElem = mDoc.createElement( mFilterPrefix + ":" + node->name().mid( 3 ) );
    for ( int i = 0; i < 2; i++ )
    {
      QDomElement childElem = toOgcFilter( args[i] );
      if ( !mErrorMessage.isEmpty() )
      {
        mCurrentSRSName.clear();
        return QDomElement();
      }

      funcElem.appendChild( childElem );
    }
    mCurrentSRSName.clear();

    QgsSQLStatement::Node* distanceNode = args[2];
    if ( distanceNode->nodeType() != QgsSQLStatement::ntLiteral )
    {
      mErrorMessage = QObject::tr( "Function %1 3rd argument should be a numeric value or a string made of a numeric value followed by a string" ).arg( node->name() );
      return QDomElement();
    }
    const QgsSQLStatement::NodeLiteral* lit = static_cast<const QgsSQLStatement::NodeLiteral*>( distanceNode );
    if ( lit->value().isNull() )
    {
      mErrorMessage = QObject::tr( "Function %1 3rd argument should be a numeric value or a string made of a numeric value followed by a string" ).arg( node->name() );
      return QDomElement();
    }
    QString distance;
    QString unit( "m" );
    switch ( lit->value().type() )
    {
      case QVariant::Int:
        distance = QString::number( lit->value().toInt() );
        break;
      case QVariant::LongLong:
        distance = QString::number( lit->value().toLongLong() );
        break;
      case QVariant::Double:
        distance = qgsDoubleToString( lit->value().toDouble() );
        break;
      case QVariant::String:
      {
        distance = lit->value().toString();
        for ( int i = 0; i < distance.size(); i++ )
        {
          if ( !(( distance[i] >= '0' && distance[i] <= '9' ) || distance[i] == '-' || distance[i] == '.' || distance[i] == 'e' || distance[i] == 'E' ) )
          {
            unit = distance.mid( i ).trimmed();
            distance = distance.mid( 0, i );
            break;
          }
        }
        break;
      }

      default:
        mErrorMessage = QObject::tr( "Literal type not supported: %1" ).arg( lit->value().type() );
        return QDomElement();
    }

    QDomElement distanceElem = mDoc.createElement( mFilterPrefix + ":Distance" );
    if ( mFilterVersion == QgsOgcUtils::FILTER_FES_2_0 )
      distanceElem.setAttribute( "uom", unit );
    else
      distanceElem.setAttribute( "unit", unit );
    distanceElem.appendChild( mDoc.createTextNode( distance ) );
    funcElem.appendChild( distanceElem );
    return funcElem;
  }

  // Other function
  QDomElement funcElem = mDoc.createElement( mFilterPrefix + ":Function" );
  funcElem.setAttribute( "name", node->name() );
  Q_FOREACH ( QgsSQLStatement::Node* n, node->args()->list() )
  {
    QDomElement childElem = toOgcFilter( n );
    if ( !mErrorMessage.isEmpty() )
      return QDomElement();

    funcElem.appendChild( childElem );
  }
  return funcElem;
}

QDomElement QgsOgcUtilsSQLStatementToFilter::toOgcFilter( const QgsSQLStatement::NodeJoin* node,
    const QString& leftTable )
{
  QgsSQLStatement::Node* onExpr = node->onExpr();
  if ( onExpr )
  {
    return toOgcFilter( onExpr );
  }

  QList<QDomElement> listElem;
  Q_FOREACH ( QString columnName, node->usingColumns() )
  {
    QDomElement eqElem = mDoc.createElement( mFilterPrefix + ":PropertyIsEqualTo" );
    QDomElement propElem1 = mDoc.createElement( mFilterPrefix + ":" + mPropertyName );
    propElem1.appendChild( mDoc.createTextNode( leftTable + "/" + columnName ) );
    eqElem.appendChild( propElem1 );
    QDomElement propElem2 = mDoc.createElement( mFilterPrefix + ":" + mPropertyName );
    propElem2.appendChild( mDoc.createTextNode( node->tableDef()->name() + "/" + columnName ) );
    eqElem.appendChild( propElem2 );
    listElem.append( eqElem );
  }

  if ( listElem.size() == 1 )
  {
    return listElem[0];
  }
  else if ( listElem.size() > 1 )
  {
    QDomElement andElem = mDoc.createElement( mFilterPrefix + ":And" );
    Q_FOREACH ( QDomElement elem, listElem )
    {
      andElem.appendChild( elem );
    }
    return andElem;
  }

  return QDomElement();
}

void QgsOgcUtilsSQLStatementToFilter::visit( const QgsSQLStatement::NodeTableDef* node )
{
  if ( node->alias().isEmpty() )
  {
    mMapTableAliasToNames[ node->name()] = node->name();
  }
  else
  {
    mMapTableAliasToNames[ node->alias()] = node->name();
  }
}

QDomElement QgsOgcUtilsSQLStatementToFilter::toOgcFilter( const QgsSQLStatement::NodeSelect* node )
{
  QList<QDomElement> listElem;

  if ( mFilterVersion != QgsOgcUtils::FILTER_FES_2_0 &&
       ( node->tables().size() != 1 || node->joins().size() != 0 ) )
  {
    mErrorMessage = QObject::tr( "Joins are only supported with WFS 2.0" );
    return QDomElement();
  }

  // Register all table name aliases
  Q_FOREACH ( QgsSQLStatement::NodeTableDef* table, node->tables() )
  {
    visit( table );
  }
  Q_FOREACH ( QgsSQLStatement::NodeJoin* join, node->joins() )
  {
    visit( join->tableDef() );
  }

  // Process JOIN conditions
  QString leftTable = node->tables().last()->name();
  Q_FOREACH ( QgsSQLStatement::NodeJoin* join, node->joins() )
  {
    QDomElement joinElem = toOgcFilter( join, leftTable );
    if ( !mErrorMessage.isEmpty() )
      return QDomElement();
    listElem.append( joinElem );
    leftTable = join->tableDef()->name();
  }

  // Process WHERE conditions
  if ( node->where() )
  {
    QDomElement whereElem = toOgcFilter( node->where() );
    if ( !mErrorMessage.isEmpty() )
      return QDomElement();
    listElem.append( whereElem );
  }

  // Concatenate all conditions
  if ( listElem.size() == 1 )
  {
    return listElem[0];
  }
  else if ( listElem.size() > 1 )
  {
    QDomElement andElem = mDoc.createElement( mFilterPrefix + ":And" );
    Q_FOREACH ( QDomElement elem, listElem )
    {
      andElem.appendChild( elem );
    }
    return andElem;
  }

  return QDomElement();
}
