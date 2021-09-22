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
#include "qgsexpressionnodeimpl.h"
#include "qgsexpressionfunction.h"
#include "qgsexpression_p.h"
#include "qgsgeometry.h"
#include "qgswkbptr.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsrectangle.h"
#include "qgsvectorlayer.h"
#include "qgsexpressioncontextutils.h"
#include "qgslogger.h"
#include "qgsstringutils.h"

#include <QColor>
#include <QStringList>
#include <QTextStream>
#include <QObject>
#include <QRegularExpression>

#ifndef Q_OS_WIN
#include <netinet/in.h>
#else
#include <winsock.h>
#endif


#define GML_NAMESPACE QStringLiteral( "http://www.opengis.net/gml" )
#define GML32_NAMESPACE QStringLiteral( "http://www.opengis.net/gml/3.2" )
#define OGC_NAMESPACE QStringLiteral( "http://www.opengis.net/ogc" )
#define FES_NAMESPACE QStringLiteral( "http://www.opengis.net/fes/2.0" )

QgsOgcUtilsExprToFilter::QgsOgcUtilsExprToFilter( QDomDocument &doc,
    QgsOgcUtils::GMLVersion gmlVersion,
    QgsOgcUtils::FilterVersion filterVersion,
    const QString &geometryName,
    const QString &srsName,
    bool honourAxisOrientation,
    bool invertAxisOrientation )
  : mDoc( doc )
  , mGMLUsed( false )
  , mGMLVersion( gmlVersion )
  , mFilterVersion( filterVersion )
  , mGeometryName( geometryName )
  , mSrsName( srsName )
  , mInvertAxisOrientation( invertAxisOrientation )
  , mFilterPrefix( ( filterVersion == QgsOgcUtils::FILTER_FES_2_0 ) ? "fes" : "ogc" )
  , mPropertyName( ( filterVersion == QgsOgcUtils::FILTER_FES_2_0 ) ? "ValueReference" : "PropertyName" )
  , mGeomId( 1 )
{
  QgsCoordinateReferenceSystem crs;
  if ( !mSrsName.isEmpty() )
    crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( mSrsName );
  if ( crs.isValid() )
  {
    if ( honourAxisOrientation && crs.hasAxisInverted() )
    {
      mInvertAxisOrientation = !mInvertAxisOrientation;
    }
  }
}

QgsGeometry QgsOgcUtils::geometryFromGML( const QDomNode &geometryNode, const Context &context )
{
  QDomElement geometryTypeElement = geometryNode.toElement();
  QString geomType = geometryTypeElement.tagName();
  QgsGeometry geometry;

  if ( !( geomType == QLatin1String( "Point" ) || geomType == QLatin1String( "LineString" ) || geomType == QLatin1String( "Polygon" ) ||
          geomType == QLatin1String( "MultiPoint" ) || geomType == QLatin1String( "MultiLineString" ) || geomType == QLatin1String( "MultiPolygon" ) ||
          geomType == QLatin1String( "Box" ) || geomType == QLatin1String( "Envelope" ) ) )
  {
    const QDomNode geometryChild = geometryNode.firstChild();
    if ( geometryChild.isNull() )
    {
      return geometry;
    }
    geometryTypeElement = geometryChild.toElement();
    geomType = geometryTypeElement.tagName();
  }

  if ( !( geomType == QLatin1String( "Point" ) || geomType == QLatin1String( "LineString" ) || geomType == QLatin1String( "Polygon" ) ||
          geomType == QLatin1String( "MultiPoint" ) || geomType == QLatin1String( "MultiLineString" ) || geomType == QLatin1String( "MultiPolygon" ) ||
          geomType == QLatin1String( "Box" ) || geomType == QLatin1String( "Envelope" ) ) )
    return QgsGeometry();

  if ( geomType == QLatin1String( "Point" ) )
  {
    geometry = geometryFromGMLPoint( geometryTypeElement );
  }
  else if ( geomType == QLatin1String( "LineString" ) )
  {
    geometry = geometryFromGMLLineString( geometryTypeElement );
  }
  else if ( geomType == QLatin1String( "Polygon" ) )
  {
    geometry = geometryFromGMLPolygon( geometryTypeElement );
  }
  else if ( geomType == QLatin1String( "MultiPoint" ) )
  {
    geometry = geometryFromGMLMultiPoint( geometryTypeElement );
  }
  else if ( geomType == QLatin1String( "MultiLineString" ) )
  {
    geometry = geometryFromGMLMultiLineString( geometryTypeElement );
  }
  else if ( geomType == QLatin1String( "MultiPolygon" ) )
  {
    geometry = geometryFromGMLMultiPolygon( geometryTypeElement );
  }
  else if ( geomType == QLatin1String( "Box" ) )
  {
    geometry = QgsGeometry::fromRect( rectangleFromGMLBox( geometryTypeElement ) );
  }
  else if ( geomType == QLatin1String( "Envelope" ) )
  {
    geometry = QgsGeometry::fromRect( rectangleFromGMLEnvelope( geometryTypeElement ) );
  }
  else //unknown type
  {
    return geometry;
  }

  // Handle srsName if context has information about the layer and the transformation context
  if ( context.layer )
  {
    QgsCoordinateReferenceSystem geomSrs;

    if ( geometryTypeElement.hasAttribute( QStringLiteral( "srsName" ) ) )
    {
      QString srsName { geometryTypeElement.attribute( QStringLiteral( "srsName" ) ) };

      // The logic here follows WFS GeoServer conventions from https://docs.geoserver.org/latest/en/user/services/wfs/axis_order.html
      const bool ignoreAxisOrientation { srsName.startsWith( QLatin1String( "http://www.opengis.net/gml/srs/" ) ) || srsName.startsWith( QLatin1String( "EPSG:" ) ) };

      // GDAL does not recognise http://www.opengis.net/gml/srs/epsg.xml#4326 but it does
      // http://www.opengis.net/def/crs/EPSG/0/4326 so, let's try that
      if ( srsName.startsWith( QLatin1String( "http://www.opengis.net/gml/srs/" ) ) )
      {
        const auto parts { srsName.split( QRegularExpression( QStringLiteral( R"raw(/|#|\.)raw" ) ) ) };
        if ( parts.length() == 10 )
        {
          srsName = QStringLiteral( "http://www.opengis.net/def/crs/%1/0/%2" ).arg( parts[ 7 ].toUpper(), parts[ 9 ] );
        }
      }
      geomSrs.createFromUserInput( srsName );
      if ( geomSrs.isValid() && geomSrs != context.layer->crs() )
      {
        if ( geomSrs.hasAxisInverted() && ! ignoreAxisOrientation )
        {
          geometry.get()->swapXy();
        }
        const QgsCoordinateTransform transformer { geomSrs, context.layer->crs(), context.transformContext };
        try
        {
          const Qgis::GeometryOperationResult result = geometry.transform( transformer );
          if ( result != Qgis::GeometryOperationResult::Success )
          {
            QgsDebugMsgLevel( QStringLiteral( "Error transforming geometry: %1" ).arg( qgsEnumValueToKey( result ) ), 2 );
          }
        }
        catch ( QgsCsException & )
        {
          QgsDebugMsgLevel( QStringLiteral( "CS error transforming geometry" ), 2 );
        }
      }
    }
  }

  return geometry;
}

QgsGeometry QgsOgcUtils::geometryFromGML( const QString &xmlString, const Context &context )
{
  // wrap the string into a root tag to have "gml" namespace (and also as a default namespace)
  const QString xml = QStringLiteral( "<tmp xmlns=\"%1\" xmlns:gml=\"%1\">%2</tmp>" ).arg( GML_NAMESPACE, xmlString );
  QDomDocument doc;
  if ( !doc.setContent( xml, true ) )
    return QgsGeometry();

  return geometryFromGML( doc.documentElement().firstChildElement(), context );
}


QgsGeometry QgsOgcUtils::geometryFromGMLPoint( const QDomElement &geometryElement )
{
  QgsPolylineXY pointCoordinate;

  const QDomNodeList coordList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "coordinates" ) );
  if ( !coordList.isEmpty() )
  {
    const QDomElement coordElement = coordList.at( 0 ).toElement();
    if ( readGMLCoordinates( pointCoordinate, coordElement ) != 0 )
    {
      return QgsGeometry();
    }
  }
  else
  {
    const QDomNodeList posList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "pos" ) );
    if ( posList.size() < 1 )
    {
      return QgsGeometry();
    }
    const QDomElement posElement = posList.at( 0 ).toElement();
    if ( readGMLPositions( pointCoordinate, posElement ) != 0 )
    {
      return QgsGeometry();
    }
  }

  if ( pointCoordinate.empty() )
  {
    return QgsGeometry();
  }

  QgsPolylineXY::const_iterator point_it = pointCoordinate.constBegin();
  char e = htonl( 1 ) != 1;
  double x = point_it->x();
  double y = point_it->y();
  const int size = 1 + sizeof( int ) + 2 * sizeof( double );

  QgsWkbTypes::Type type = QgsWkbTypes::Point;
  unsigned char *wkb = new unsigned char[size];

  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  memcpy( &( wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( wkb )[wkbPosition], &x, sizeof( double ) );
  wkbPosition += sizeof( double );
  memcpy( &( wkb )[wkbPosition], &y, sizeof( double ) );

  QgsGeometry g;
  g.fromWkb( wkb, size );
  return g;
}

QgsGeometry QgsOgcUtils::geometryFromGMLLineString( const QDomElement &geometryElement )
{
  QgsPolylineXY lineCoordinates;

  const QDomNodeList coordList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "coordinates" ) );
  if ( !coordList.isEmpty() )
  {
    const QDomElement coordElement = coordList.at( 0 ).toElement();
    if ( readGMLCoordinates( lineCoordinates, coordElement ) != 0 )
    {
      return QgsGeometry();
    }
  }
  else
  {
    const QDomNodeList posList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "posList" ) );
    if ( posList.size() < 1 )
    {
      return QgsGeometry();
    }
    const QDomElement posElement = posList.at( 0 ).toElement();
    if ( readGMLPositions( lineCoordinates, posElement ) != 0 )
    {
      return QgsGeometry();
    }
  }

  char e = htonl( 1 ) != 1;
  const int size = 1 + 2 * sizeof( int ) + lineCoordinates.size() * 2 * sizeof( double );

  QgsWkbTypes::Type type = QgsWkbTypes::LineString;
  unsigned char *wkb = new unsigned char[size];

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

  QgsPolylineXY::const_iterator iter;
  for ( iter = lineCoordinates.constBegin(); iter != lineCoordinates.constEnd(); ++iter )
  {
    x = iter->x();
    y = iter->y();
    memcpy( &( wkb )[wkbPosition], &x, sizeof( double ) );
    wkbPosition += sizeof( double );
    memcpy( &( wkb )[wkbPosition], &y, sizeof( double ) );
    wkbPosition += sizeof( double );
  }

  QgsGeometry g;
  g.fromWkb( wkb, size );
  return g;
}

QgsGeometry QgsOgcUtils::geometryFromGMLPolygon( const QDomElement &geometryElement )
{
  //read all the coordinates (as QgsPoint) into memory. Each linear ring has an entry in the vector
  QgsMultiPolylineXY ringCoordinates;

  //read coordinates for outer boundary
  QgsPolylineXY exteriorPointList;
  const QDomNodeList outerBoundaryList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "outerBoundaryIs" ) );
  if ( !outerBoundaryList.isEmpty() ) //outer ring is necessary
  {
    QDomElement coordinatesElement = outerBoundaryList.at( 0 ).firstChild().firstChild().toElement();
    if ( coordinatesElement.isNull() )
    {
      return QgsGeometry();
    }
    if ( readGMLCoordinates( exteriorPointList, coordinatesElement ) != 0 )
    {
      return QgsGeometry();
    }
    ringCoordinates.push_back( exteriorPointList );

    //read coordinates for inner boundary
    const QDomNodeList innerBoundaryList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "innerBoundaryIs" ) );
    for ( int i = 0; i < innerBoundaryList.size(); ++i )
    {
      QgsPolylineXY interiorPointList;
      coordinatesElement = innerBoundaryList.at( i ).firstChild().firstChild().toElement();
      if ( coordinatesElement.isNull() )
      {
        return QgsGeometry();
      }
      if ( readGMLCoordinates( interiorPointList, coordinatesElement ) != 0 )
      {
        return QgsGeometry();
      }
      ringCoordinates.push_back( interiorPointList );
    }
  }
  else
  {
    //read coordinates for exterior
    const QDomNodeList exteriorList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "exterior" ) );
    if ( exteriorList.size() < 1 ) //outer ring is necessary
    {
      return QgsGeometry();
    }
    const QDomElement posElement = exteriorList.at( 0 ).firstChild().firstChild().toElement();
    if ( posElement.isNull() )
    {
      return QgsGeometry();
    }
    if ( readGMLPositions( exteriorPointList, posElement ) != 0 )
    {
      return QgsGeometry();
    }
    ringCoordinates.push_back( exteriorPointList );

    //read coordinates for inner boundary
    const QDomNodeList interiorList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "interior" ) );
    for ( int i = 0; i < interiorList.size(); ++i )
    {
      QgsPolylineXY interiorPointList;
      const QDomElement posElement = interiorList.at( i ).firstChild().firstChild().toElement();
      if ( posElement.isNull() )
      {
        return QgsGeometry();
      }
      // Note: readGMLPositions returns true on errors and false on success
      if ( readGMLPositions( interiorPointList, posElement ) )
      {
        return QgsGeometry();
      }
      ringCoordinates.push_back( interiorPointList );
    }
  }

  //calculate number of bytes to allocate
  int nrings = ringCoordinates.size();
  if ( nrings < 1 )
    return QgsGeometry();

  int npoints = 0;//total number of points
  for ( QgsMultiPolylineXY::const_iterator it = ringCoordinates.constBegin(); it != ringCoordinates.constEnd(); ++it )
  {
    npoints += it->size();
  }
  const int size = 1 + 2 * sizeof( int ) + nrings * sizeof( int ) + 2 * npoints * sizeof( double );

  QgsWkbTypes::Type type = QgsWkbTypes::Polygon;
  unsigned char *wkb = new unsigned char[size];

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
  for ( QgsMultiPolylineXY::const_iterator it = ringCoordinates.constBegin(); it != ringCoordinates.constEnd(); ++it )
  {
    nPointsInRing = it->size();
    memcpy( &( wkb )[wkbPosition], &nPointsInRing, sizeof( int ) );
    wkbPosition += sizeof( int );
    //iterate through the string list converting the strings to x-/y- doubles
    QgsPolylineXY::const_iterator iter;
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

  QgsGeometry g;
  g.fromWkb( wkb, size );
  return g;
}

QgsGeometry QgsOgcUtils::geometryFromGMLMultiPoint( const QDomElement &geometryElement )
{
  QgsPolylineXY pointList;
  QgsPolylineXY currentPoint;
  const QDomNodeList pointMemberList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "pointMember" ) );
  if ( pointMemberList.size() < 1 )
  {
    return QgsGeometry();
  }
  QDomNodeList pointNodeList;
  // coordinates or pos element
  QDomNodeList coordinatesList;
  QDomNodeList posList;
  for ( int i = 0; i < pointMemberList.size(); ++i )
  {
    //<Point> element
    pointNodeList = pointMemberList.at( i ).toElement().elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "Point" ) );
    if ( pointNodeList.size() < 1 )
    {
      continue;
    }
    //<coordinates> element
    coordinatesList = pointNodeList.at( 0 ).toElement().elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "coordinates" ) );
    if ( !coordinatesList.isEmpty() )
    {
      currentPoint.clear();
      if ( readGMLCoordinates( currentPoint, coordinatesList.at( 0 ).toElement() ) != 0 )
      {
        continue;
      }
      if ( currentPoint.empty() )
      {
        continue;
      }
      pointList.push_back( ( *currentPoint.begin() ) );
      continue;
    }
    else
    {
      //<pos> element
      posList = pointNodeList.at( 0 ).toElement().elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "pos" ) );
      if ( posList.size() < 1 )
      {
        continue;
      }
      currentPoint.clear();
      if ( readGMLPositions( currentPoint, posList.at( 0 ).toElement() ) != 0 )
      {
        continue;
      }
      if ( currentPoint.empty() )
      {
        continue;
      }
      pointList.push_back( ( *currentPoint.begin() ) );
    }
  }

  int nPoints = pointList.size(); //number of points
  if ( nPoints < 1 )
    return QgsGeometry();

  //calculate the required wkb size
  const int size = 1 + 2 * sizeof( int ) + pointList.size() * ( 2 * sizeof( double ) + 1 + sizeof( int ) );

  QgsWkbTypes::Type type = QgsWkbTypes::MultiPoint;
  unsigned char *wkb = new unsigned char[size];

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
  type = QgsWkbTypes::Point;
  for ( QgsPolylineXY::const_iterator it = pointList.constBegin(); it != pointList.constEnd(); ++it )
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

  QgsGeometry g;
  g.fromWkb( wkb, size );
  return g;
}

QgsGeometry QgsOgcUtils::geometryFromGMLMultiLineString( const QDomElement &geometryElement )
{
  //geoserver has
  //<gml:MultiLineString>
  //<gml:lineStringMember>
  //<gml:LineString>

  //mapserver has directly
  //<gml:MultiLineString
  //<gml:LineString

  QList< QgsPolylineXY > lineCoordinates; //first list: lines, second list: points of one line
  QDomElement currentLineStringElement;
  QDomNodeList currentCoordList;
  QDomNodeList currentPosList;

  const QDomNodeList lineStringMemberList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "lineStringMember" ) );
  if ( !lineStringMemberList.isEmpty() ) //geoserver
  {
    for ( int i = 0; i < lineStringMemberList.size(); ++i )
    {
      const QDomNodeList lineStringNodeList = lineStringMemberList.at( i ).toElement().elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "LineString" ) );
      if ( lineStringNodeList.size() < 1 )
      {
        return QgsGeometry();
      }
      currentLineStringElement = lineStringNodeList.at( 0 ).toElement();
      currentCoordList = currentLineStringElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "coordinates" ) );
      if ( !currentCoordList.isEmpty() )
      {
        QgsPolylineXY currentPointList;
        if ( readGMLCoordinates( currentPointList, currentCoordList.at( 0 ).toElement() ) != 0 )
        {
          return QgsGeometry();
        }
        lineCoordinates.push_back( currentPointList );
      }
      else
      {
        currentPosList = currentLineStringElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "posList" ) );
        if ( currentPosList.size() < 1 )
        {
          return QgsGeometry();
        }
        QgsPolylineXY currentPointList;
        if ( readGMLPositions( currentPointList, currentPosList.at( 0 ).toElement() ) != 0 )
        {
          return QgsGeometry();
        }
        lineCoordinates.push_back( currentPointList );
      }
    }
  }
  else
  {
    const QDomNodeList lineStringList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "LineString" ) );
    if ( !lineStringList.isEmpty() ) //mapserver
    {
      for ( int i = 0; i < lineStringList.size(); ++i )
      {
        currentLineStringElement = lineStringList.at( i ).toElement();
        currentCoordList = currentLineStringElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "coordinates" ) );
        if ( !currentCoordList.isEmpty() )
        {
          QgsPolylineXY currentPointList;
          if ( readGMLCoordinates( currentPointList, currentCoordList.at( 0 ).toElement() ) != 0 )
          {
            return QgsGeometry();
          }
          lineCoordinates.push_back( currentPointList );
          return QgsGeometry();
        }
        else
        {
          currentPosList = currentLineStringElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "posList" ) );
          if ( currentPosList.size() < 1 )
          {
            return QgsGeometry();
          }
          QgsPolylineXY currentPointList;
          if ( readGMLPositions( currentPointList, currentPosList.at( 0 ).toElement() ) != 0 )
          {
            return QgsGeometry();
          }
          lineCoordinates.push_back( currentPointList );
        }
      }
    }
    else
    {
      return QgsGeometry();
    }
  }

  int nLines = lineCoordinates.size();
  if ( nLines < 1 )
    return QgsGeometry();

  //calculate the required wkb size
  int size = ( lineCoordinates.size() + 1 ) * ( 1 + 2 * sizeof( int ) );
  for ( QList< QgsPolylineXY >::const_iterator it = lineCoordinates.constBegin(); it != lineCoordinates.constEnd(); ++it )
  {
    size += it->size() * 2 * sizeof( double );
  }

  QgsWkbTypes::Type type = QgsWkbTypes::MultiLineString;
  unsigned char *wkb = new unsigned char[size];

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
  type = QgsWkbTypes::LineString;
  for ( QList< QgsPolylineXY >::const_iterator it = lineCoordinates.constBegin(); it != lineCoordinates.constEnd(); ++it )
  {
    memcpy( &( wkb )[wkbPosition], &e, 1 );
    wkbPosition += 1;
    memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
    wkbPosition += sizeof( int );
    nPoints = it->size();
    memcpy( &( wkb )[wkbPosition], &nPoints, sizeof( int ) );
    wkbPosition += sizeof( int );
    for ( QgsPolylineXY::const_iterator iter = it->begin(); iter != it->end(); ++iter )
    {
      x = iter->x();
      y = iter->y();
      // QgsDebugMsg( QStringLiteral( "x, y is %1,%2" ).arg( x, 'f' ).arg( y, 'f' ) );
      memcpy( &( wkb )[wkbPosition], &x, sizeof( double ) );
      wkbPosition += sizeof( double );
      memcpy( &( wkb )[wkbPosition], &y, sizeof( double ) );
      wkbPosition += sizeof( double );
    }
  }

  QgsGeometry g;
  g.fromWkb( wkb, size );
  return g;
}

QgsGeometry QgsOgcUtils::geometryFromGMLMultiPolygon( const QDomElement &geometryElement )
{
  //first list: different polygons, second list: different rings, third list: different points
  QgsMultiPolygonXY multiPolygonPoints;
  QDomElement currentPolygonMemberElement;
  QDomNodeList polygonList;
  QDomElement currentPolygonElement;
  // rings in GML2
  QDomNodeList outerBoundaryList;
  QDomElement currentOuterBoundaryElement;
  const QDomNodeList innerBoundaryList;
  QDomElement currentInnerBoundaryElement;
  // rings in GML3
  QDomNodeList exteriorList;
  QDomElement currentExteriorElement;
  QDomElement currentInteriorElement;
  const QDomNodeList interiorList;
  // lienar ring
  QDomNodeList linearRingNodeList;
  QDomElement currentLinearRingElement;
  // Coordinates or position list
  QDomNodeList currentCoordinateList;
  QDomNodeList currentPosList;

  const QDomNodeList polygonMemberList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "polygonMember" ) );
  QgsPolygonXY currentPolygonList;
  for ( int i = 0; i < polygonMemberList.size(); ++i )
  {
    currentPolygonList.resize( 0 ); // preserve capacity - don't use clear
    currentPolygonMemberElement = polygonMemberList.at( i ).toElement();
    polygonList = currentPolygonMemberElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "Polygon" ) );
    if ( polygonList.size() < 1 )
    {
      continue;
    }
    currentPolygonElement = polygonList.at( 0 ).toElement();

    //find exterior ring
    outerBoundaryList = currentPolygonElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "outerBoundaryIs" ) );
    if ( !outerBoundaryList.isEmpty() )
    {
      currentOuterBoundaryElement = outerBoundaryList.at( 0 ).toElement();
      QgsPolylineXY ringCoordinates;

      linearRingNodeList = currentOuterBoundaryElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "LinearRing" ) );
      if ( linearRingNodeList.size() < 1 )
      {
        continue;
      }
      currentLinearRingElement = linearRingNodeList.at( 0 ).toElement();
      currentCoordinateList = currentLinearRingElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "coordinates" ) );
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
      const QDomNodeList innerBoundaryList = currentPolygonElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "innerBoundaryIs" ) );
      for ( int j = 0; j < innerBoundaryList.size(); ++j )
      {
        QgsPolylineXY ringCoordinates;
        currentInnerBoundaryElement = innerBoundaryList.at( j ).toElement();
        linearRingNodeList = currentInnerBoundaryElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "LinearRing" ) );
        if ( linearRingNodeList.size() < 1 )
        {
          continue;
        }
        currentLinearRingElement = linearRingNodeList.at( 0 ).toElement();
        currentCoordinateList = currentLinearRingElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "coordinates" ) );
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
      exteriorList = currentPolygonElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "exterior" ) );
      if ( exteriorList.size() < 1 )
      {
        continue;
      }

      currentExteriorElement = exteriorList.at( 0 ).toElement();
      QgsPolylineXY ringPositions;

      linearRingNodeList = currentExteriorElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "LinearRing" ) );
      if ( linearRingNodeList.size() < 1 )
      {
        continue;
      }
      currentLinearRingElement = linearRingNodeList.at( 0 ).toElement();
      currentPosList = currentLinearRingElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "posList" ) );
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
      const QDomNodeList interiorList = currentPolygonElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "interior" ) );
      for ( int j = 0; j < interiorList.size(); ++j )
      {
        QgsPolylineXY ringPositions;
        currentInteriorElement = interiorList.at( j ).toElement();
        linearRingNodeList = currentInteriorElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "LinearRing" ) );
        if ( linearRingNodeList.size() < 1 )
        {
          continue;
        }
        currentLinearRingElement = linearRingNodeList.at( 0 ).toElement();
        currentPosList = currentLinearRingElement.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "posList" ) );
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
    return QgsGeometry();

  int size = 1 + 2 * sizeof( int );
  //calculate the wkb size
  for ( QgsMultiPolygonXY::const_iterator it = multiPolygonPoints.constBegin(); it != multiPolygonPoints.constEnd(); ++it )
  {
    size += 1 + 2 * sizeof( int );
    for ( QgsPolygonXY::const_iterator iter = it->begin(); iter != it->end(); ++iter )
    {
      size += sizeof( int ) + 2 * iter->size() * sizeof( double );
    }
  }

  QgsWkbTypes::Type type = QgsWkbTypes::MultiPolygon;
  unsigned char *wkb = new unsigned char[size];

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

  type = QgsWkbTypes::Polygon;

  for ( QgsMultiPolygonXY::const_iterator it = multiPolygonPoints.constBegin(); it != multiPolygonPoints.constEnd(); ++it )
  {
    memcpy( &( wkb )[wkbPosition], &e, 1 );
    wkbPosition += 1;
    memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
    wkbPosition += sizeof( int );
    nRings = it->size();
    memcpy( &( wkb )[wkbPosition], &nRings, sizeof( int ) );
    wkbPosition += sizeof( int );
    for ( QgsPolygonXY::const_iterator iter = it->begin(); iter != it->end(); ++iter )
    {
      nPointsInRing = iter->size();
      memcpy( &( wkb )[wkbPosition], &nPointsInRing, sizeof( int ) );
      wkbPosition += sizeof( int );
      for ( QgsPolylineXY::const_iterator iterator = iter->begin(); iterator != iter->end(); ++iterator )
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

  QgsGeometry g;
  g.fromWkb( wkb, size );
  return g;
}

bool QgsOgcUtils::readGMLCoordinates( QgsPolylineXY &coords, const QDomElement &elem )
{
  QString coordSeparator = QStringLiteral( "," );
  QString tupelSeparator = QStringLiteral( " " );
  //"decimal" has to be "."

  coords.clear();

  if ( elem.hasAttribute( QStringLiteral( "cs" ) ) )
  {
    coordSeparator = elem.attribute( QStringLiteral( "cs" ) );
  }
  if ( elem.hasAttribute( QStringLiteral( "ts" ) ) )
  {
    tupelSeparator = elem.attribute( QStringLiteral( "ts" ) );
  }

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  QStringList tupels = elem.text().split( tupelSeparator, QString::SkipEmptyParts );
#else
  const QStringList tupels = elem.text().split( tupelSeparator, Qt::SkipEmptyParts );
#endif
  QStringList tuple_coords;
  double x, y;
  bool conversionSuccess;

  QStringList::const_iterator it;
  for ( it = tupels.constBegin(); it != tupels.constEnd(); ++it )
  {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    tuple_coords = ( *it ).split( coordSeparator, QString::SkipEmptyParts );
#else
    tuple_coords = ( *it ).split( coordSeparator, Qt::SkipEmptyParts );
#endif
    if ( tuple_coords.size() < 2 )
    {
      continue;
    }
    x = tuple_coords.at( 0 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      return true;
    }
    y = tuple_coords.at( 1 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      return true;
    }
    coords.push_back( QgsPointXY( x, y ) );
  }
  return false;
}

QgsRectangle QgsOgcUtils::rectangleFromGMLBox( const QDomNode &boxNode )
{
  QgsRectangle rect;

  const QDomElement boxElem = boxNode.toElement();
  if ( boxElem.tagName() != QLatin1String( "Box" ) )
    return rect;

  const QDomElement bElem = boxElem.firstChild().toElement();
  QString coordSeparator = QStringLiteral( "," );
  QString tupelSeparator = QStringLiteral( " " );
  if ( bElem.hasAttribute( QStringLiteral( "cs" ) ) )
  {
    coordSeparator = bElem.attribute( QStringLiteral( "cs" ) );
  }
  if ( bElem.hasAttribute( QStringLiteral( "ts" ) ) )
  {
    tupelSeparator = bElem.attribute( QStringLiteral( "ts" ) );
  }

  const QString bString = bElem.text();
  bool ok1, ok2, ok3, ok4;
  const double xmin = bString.section( tupelSeparator, 0, 0 ).section( coordSeparator, 0, 0 ).toDouble( &ok1 );
  const double ymin = bString.section( tupelSeparator, 0, 0 ).section( coordSeparator, 1, 1 ).toDouble( &ok2 );
  const double xmax = bString.section( tupelSeparator, 1, 1 ).section( coordSeparator, 0, 0 ).toDouble( &ok3 );
  const double ymax = bString.section( tupelSeparator, 1, 1 ).section( coordSeparator, 1, 1 ).toDouble( &ok4 );

  if ( ok1 && ok2 && ok3 && ok4 )
  {
    rect = QgsRectangle( xmin, ymin, xmax, ymax );
    rect.normalize();
  }

  return rect;
}

bool QgsOgcUtils::readGMLPositions( QgsPolylineXY &coords, const QDomElement &elem )
{
  coords.clear();

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  QStringList pos = elem.text().split( ' ', QString::SkipEmptyParts );
#else
  const QStringList pos = elem.text().split( ' ', Qt::SkipEmptyParts );
#endif
  double x, y;
  bool conversionSuccess;
  const int posSize = pos.size();

  int srsDimension = 2;
  if ( elem.hasAttribute( QStringLiteral( "srsDimension" ) ) )
  {
    srsDimension = elem.attribute( QStringLiteral( "srsDimension" ) ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      srsDimension = 2;
    }
  }
  else if ( elem.hasAttribute( QStringLiteral( "dimension" ) ) )
  {
    srsDimension = elem.attribute( QStringLiteral( "dimension" ) ).toInt( &conversionSuccess );
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
      return true;
    }
    y = pos.at( i * srsDimension + 1 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      return true;
    }
    coords.push_back( QgsPointXY( x, y ) );
  }
  return false;
}


QgsRectangle QgsOgcUtils::rectangleFromGMLEnvelope( const QDomNode &envelopeNode )
{
  QgsRectangle rect;

  const QDomElement envelopeElem = envelopeNode.toElement();
  if ( envelopeElem.tagName() != QLatin1String( "Envelope" ) )
    return rect;

  const QDomNodeList lowerCornerList = envelopeElem.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "lowerCorner" ) );
  if ( lowerCornerList.size() < 1 )
    return rect;

  const QDomNodeList upperCornerList = envelopeElem.elementsByTagNameNS( GML_NAMESPACE, QStringLiteral( "upperCorner" ) );
  if ( upperCornerList.size() < 1 )
    return rect;

  bool conversionSuccess;
  int srsDimension = 2;

  QDomElement elem = lowerCornerList.at( 0 ).toElement();
  if ( elem.hasAttribute( QStringLiteral( "srsDimension" ) ) )
  {
    srsDimension = elem.attribute( QStringLiteral( "srsDimension" ) ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      srsDimension = 2;
    }
  }
  else if ( elem.hasAttribute( QStringLiteral( "dimension" ) ) )
  {
    srsDimension = elem.attribute( QStringLiteral( "dimension" ) ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      srsDimension = 2;
    }
  }
  QString bString = elem.text();

  const double xmin = bString.section( ' ', 0, 0 ).toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    return rect;
  const double ymin = bString.section( ' ', 1, 1 ).toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    return rect;

  elem = upperCornerList.at( 0 ).toElement();
  if ( elem.hasAttribute( QStringLiteral( "srsDimension" ) ) )
  {
    srsDimension = elem.attribute( QStringLiteral( "srsDimension" ) ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      srsDimension = 2;
    }
  }
  else if ( elem.hasAttribute( QStringLiteral( "dimension" ) ) )
  {
    srsDimension = elem.attribute( QStringLiteral( "dimension" ) ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      srsDimension = 2;
    }
  }

  Q_UNUSED( srsDimension )

  bString = elem.text();
  const double xmax = bString.section( ' ', 0, 0 ).toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    return rect;
  const double ymax = bString.section( ' ', 1, 1 ).toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    return rect;

  rect = QgsRectangle( xmin, ymin, xmax, ymax );
  rect.normalize();

  return rect;
}

QDomElement QgsOgcUtils::rectangleToGMLBox( QgsRectangle *box, QDomDocument &doc, int precision )
{
  return rectangleToGMLBox( box, doc, QString(), false, precision );
}

QDomElement QgsOgcUtils::rectangleToGMLBox( QgsRectangle *box, QDomDocument &doc,
    const QString &srsName,
    bool invertAxisOrientation,
    int precision )
{
  if ( !box )
  {
    return QDomElement();
  }

  QDomElement boxElem = doc.createElement( QStringLiteral( "gml:Box" ) );
  if ( !srsName.isEmpty() )
  {
    boxElem.setAttribute( QStringLiteral( "srsName" ), srsName );
  }
  QDomElement coordElem = doc.createElement( QStringLiteral( "gml:coordinates" ) );
  coordElem.setAttribute( QStringLiteral( "cs" ), QStringLiteral( "," ) );
  coordElem.setAttribute( QStringLiteral( "ts" ), QStringLiteral( " " ) );

  QString coordString;
  coordString += qgsDoubleToString( invertAxisOrientation ? box->yMinimum() : box->xMinimum(), precision );
  coordString += ',';
  coordString += qgsDoubleToString( invertAxisOrientation ? box->xMinimum() : box->yMinimum(), precision );
  coordString += ' ';
  coordString += qgsDoubleToString( invertAxisOrientation ? box->yMaximum() : box->xMaximum(), precision );
  coordString += ',';
  coordString += qgsDoubleToString( invertAxisOrientation ? box->xMaximum() : box->yMaximum(), precision );

  const QDomText coordText = doc.createTextNode( coordString );
  coordElem.appendChild( coordText );
  boxElem.appendChild( coordElem );

  return boxElem;
}

QDomElement QgsOgcUtils::rectangleToGMLEnvelope( QgsRectangle *env, QDomDocument &doc, int precision )
{
  return rectangleToGMLEnvelope( env, doc, QString(), false, precision );
}

QDomElement QgsOgcUtils::rectangleToGMLEnvelope( QgsRectangle *env, QDomDocument &doc,
    const QString &srsName,
    bool invertAxisOrientation,
    int precision )
{
  if ( !env )
  {
    return QDomElement();
  }

  QDomElement envElem = doc.createElement( QStringLiteral( "gml:Envelope" ) );
  if ( !srsName.isEmpty() )
  {
    envElem.setAttribute( QStringLiteral( "srsName" ), srsName );
  }
  QString posList;

  QDomElement lowerCornerElem = doc.createElement( QStringLiteral( "gml:lowerCorner" ) );
  posList = qgsDoubleToString( invertAxisOrientation ? env->yMinimum() : env->xMinimum(), precision );
  posList += ' ';
  posList += qgsDoubleToString( invertAxisOrientation ? env->xMinimum() : env->yMinimum(), precision );
  const QDomText lowerCornerText = doc.createTextNode( posList );
  lowerCornerElem.appendChild( lowerCornerText );
  envElem.appendChild( lowerCornerElem );

  QDomElement upperCornerElem = doc.createElement( QStringLiteral( "gml:upperCorner" ) );
  posList = qgsDoubleToString( invertAxisOrientation ? env->yMaximum() : env->xMaximum(), precision );
  posList += ' ';
  posList += qgsDoubleToString( invertAxisOrientation ? env->xMaximum() : env->yMaximum(), precision );
  const QDomText upperCornerText = doc.createTextNode( posList );
  upperCornerElem.appendChild( upperCornerText );
  envElem.appendChild( upperCornerElem );

  return envElem;
}

QDomElement QgsOgcUtils::geometryToGML( const QgsGeometry &geometry, QDomDocument &doc, const QString &format, int precision )
{
  return geometryToGML( geometry, doc, ( format == QLatin1String( "GML2" ) ) ? GML_2_1_2 : GML_3_2_1, QString(), false, QString(), precision );
}

QDomElement QgsOgcUtils::geometryToGML( const QgsGeometry &geometry,
                                        QDomDocument &doc,
                                        GMLVersion gmlVersion,
                                        const QString &srsName,
                                        bool invertAxisOrientation,
                                        const QString &gmlIdBase,
                                        int precision )
{
  if ( geometry.isNull() )
    return QDomElement();

  // coordinate separator
  QString cs = QStringLiteral( "," );
  // tuple separator
  const QString ts = QStringLiteral( " " );
  // coord element tagname
  QDomElement baseCoordElem;

  bool hasZValue = false;

  const QByteArray wkb( geometry.asWkb() );
  QgsConstWkbPtr wkbPtr( wkb );
  try
  {
    wkbPtr.readHeader();
  }
  catch ( const QgsWkbException &e )
  {
    Q_UNUSED( e )
    // WKB exception while reading header
    return QDomElement();
  }

  if ( gmlVersion != GML_2_1_2 )
  {
    switch ( geometry.wkbType() )
    {
      case QgsWkbTypes::Point25D:
      case QgsWkbTypes::Point:
      case QgsWkbTypes::MultiPoint25D:
      case QgsWkbTypes::MultiPoint:
        baseCoordElem = doc.createElement( QStringLiteral( "gml:pos" ) );
        break;
      default:
        baseCoordElem = doc.createElement( QStringLiteral( "gml:posList" ) );
        break;
    }
    baseCoordElem.setAttribute( QStringLiteral( "srsDimension" ), QStringLiteral( "2" ) );
    cs = ' ';
  }
  else
  {
    baseCoordElem = doc.createElement( QStringLiteral( "gml:coordinates" ) );
    baseCoordElem.setAttribute( QStringLiteral( "cs" ), cs );
    baseCoordElem.setAttribute( QStringLiteral( "ts" ), ts );
  }

  try
  {
    switch ( geometry.wkbType() )
    {
      case QgsWkbTypes::Point25D:
      case QgsWkbTypes::Point:
      {
        QDomElement pointElem = doc.createElement( QStringLiteral( "gml:Point" ) );
        if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
          pointElem.setAttribute( QStringLiteral( "gml:id" ), gmlIdBase );
        if ( !srsName.isEmpty() )
          pointElem.setAttribute( QStringLiteral( "srsName" ), srsName );
        QDomElement coordElem = baseCoordElem.cloneNode().toElement();

        double x, y;

        if ( invertAxisOrientation )
          wkbPtr >> y >> x;
        else
          wkbPtr >> x >> y;
        const QDomText coordText = doc.createTextNode( qgsDoubleToString( x, precision ) + cs + qgsDoubleToString( y, precision ) );

        coordElem.appendChild( coordText );
        pointElem.appendChild( coordElem );
        return pointElem;
      }
      case QgsWkbTypes::MultiPoint25D:
        hasZValue = true;
        //intentional fall-through
        FALLTHROUGH
      case QgsWkbTypes::MultiPoint:
      {
        QDomElement multiPointElem = doc.createElement( QStringLiteral( "gml:MultiPoint" ) );
        if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
          multiPointElem.setAttribute( QStringLiteral( "gml:id" ), gmlIdBase );
        if ( !srsName.isEmpty() )
          multiPointElem.setAttribute( QStringLiteral( "srsName" ), srsName );

        int nPoints;
        wkbPtr >> nPoints;

        for ( int idx = 0; idx < nPoints; ++idx )
        {
          QDomElement pointMemberElem = doc.createElement( QStringLiteral( "gml:pointMember" ) );
          QDomElement pointElem = doc.createElement( QStringLiteral( "gml:Point" ) );
          if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
            pointElem.setAttribute( QStringLiteral( "gml:id" ), gmlIdBase + QStringLiteral( ".%1" ).arg( idx + 1 ) );
          QDomElement coordElem = baseCoordElem.cloneNode().toElement();

          wkbPtr.readHeader();

          double x = 0;
          double y = 0;
          if ( invertAxisOrientation )
            wkbPtr >> y >> x;
          else
            wkbPtr >> x >> y;
          const QDomText coordText = doc.createTextNode( qgsDoubleToString( x, precision ) + cs + qgsDoubleToString( y, precision ) );

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
      case QgsWkbTypes::LineString25D:
        hasZValue = true;
        //intentional fall-through
        FALLTHROUGH
      case QgsWkbTypes::LineString:
      {
        QDomElement lineStringElem = doc.createElement( QStringLiteral( "gml:LineString" ) );
        if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
          lineStringElem.setAttribute( QStringLiteral( "gml:id" ), gmlIdBase );
        if ( !srsName.isEmpty() )
          lineStringElem.setAttribute( QStringLiteral( "srsName" ), srsName );
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

          double x = 0;
          double y = 0;
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
        const QDomText coordText = doc.createTextNode( coordString );
        coordElem.appendChild( coordText );
        lineStringElem.appendChild( coordElem );
        return lineStringElem;
      }
      case QgsWkbTypes::MultiLineString25D:
        hasZValue = true;
        //intentional fall-through
        FALLTHROUGH
      case QgsWkbTypes::MultiLineString:
      {
        QDomElement multiLineStringElem = doc.createElement( QStringLiteral( "gml:MultiLineString" ) );
        if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
          multiLineStringElem.setAttribute( QStringLiteral( "gml:id" ), gmlIdBase );
        if ( !srsName.isEmpty() )
          multiLineStringElem.setAttribute( QStringLiteral( "srsName" ), srsName );

        int nLines;
        wkbPtr >> nLines;

        for ( int jdx = 0; jdx < nLines; jdx++ )
        {
          QDomElement lineStringMemberElem = doc.createElement( QStringLiteral( "gml:lineStringMember" ) );
          QDomElement lineStringElem = doc.createElement( QStringLiteral( "gml:LineString" ) );
          if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
            lineStringElem.setAttribute( QStringLiteral( "gml:id" ), gmlIdBase + QStringLiteral( ".%1" ).arg( jdx + 1 ) );

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

            double x = 0;
            double y = 0;
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
          const QDomText coordText = doc.createTextNode( coordString );
          coordElem.appendChild( coordText );
          lineStringElem.appendChild( coordElem );
          lineStringMemberElem.appendChild( lineStringElem );
          multiLineStringElem.appendChild( lineStringMemberElem );
        }
        return multiLineStringElem;
      }
      case QgsWkbTypes::Polygon25D:
        hasZValue = true;
        //intentional fall-through
        FALLTHROUGH
      case QgsWkbTypes::Polygon:
      {
        QDomElement polygonElem = doc.createElement( QStringLiteral( "gml:Polygon" ) );
        if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
          polygonElem.setAttribute( QStringLiteral( "gml:id" ), gmlIdBase );
        if ( !srsName.isEmpty() )
          polygonElem.setAttribute( QStringLiteral( "srsName" ), srsName );

        // get number of rings in the polygon
        int numRings;
        wkbPtr >> numRings;

        if ( numRings == 0 ) // sanity check for zero rings in polygon
          return QDomElement();

        for ( int idx = 0; idx < numRings; idx++ )
        {
          QString boundaryName = ( gmlVersion == GML_2_1_2 ) ? "gml:outerBoundaryIs" : "gml:exterior";
          if ( idx != 0 )
          {
            boundaryName = ( gmlVersion == GML_2_1_2 ) ? "gml:innerBoundaryIs" : "gml:interior";
          }
          QDomElement boundaryElem = doc.createElement( boundaryName );
          QDomElement ringElem = doc.createElement( QStringLiteral( "gml:LinearRing" ) );
          // get number of points in the ring
          int nPoints = 0;
          wkbPtr >> nPoints;

          QDomElement coordElem = baseCoordElem.cloneNode().toElement();
          QString coordString;
          for ( int jdx = 0; jdx < nPoints; jdx++ )
          {
            if ( jdx != 0 )
            {
              coordString += ts;
            }

            double x = 0;
            double y = 0;
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
          const QDomText coordText = doc.createTextNode( coordString );
          coordElem.appendChild( coordText );
          ringElem.appendChild( coordElem );
          boundaryElem.appendChild( ringElem );
          polygonElem.appendChild( boundaryElem );
        }
        return polygonElem;
      }
      case QgsWkbTypes::MultiPolygon25D:
        hasZValue = true;
        //intentional fall-through
        FALLTHROUGH
      case QgsWkbTypes::MultiPolygon:
      {
        QDomElement multiPolygonElem = doc.createElement( QStringLiteral( "gml:MultiPolygon" ) );
        if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
          multiPolygonElem.setAttribute( QStringLiteral( "gml:id" ), gmlIdBase );
        if ( !srsName.isEmpty() )
          multiPolygonElem.setAttribute( QStringLiteral( "srsName" ), srsName );

        int numPolygons;
        wkbPtr >> numPolygons;

        for ( int kdx = 0; kdx < numPolygons; kdx++ )
        {
          QDomElement polygonMemberElem = doc.createElement( QStringLiteral( "gml:polygonMember" ) );
          QDomElement polygonElem = doc.createElement( QStringLiteral( "gml:Polygon" ) );
          if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
            polygonElem.setAttribute( QStringLiteral( "gml:id" ), gmlIdBase + QStringLiteral( ".%1" ).arg( kdx + 1 ) );

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
            QDomElement ringElem = doc.createElement( QStringLiteral( "gml:LinearRing" ) );

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

              double x = 0;
              double y = 0;
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
            const QDomText coordText = doc.createTextNode( coordString );
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
    Q_UNUSED( e )
    return QDomElement();
  }
}

QDomElement QgsOgcUtils::geometryToGML( const QgsGeometry &geometry, QDomDocument &doc, int precision )
{
  return geometryToGML( geometry, doc, QStringLiteral( "GML2" ), precision );
}

QDomElement QgsOgcUtils::createGMLCoordinates( const QgsPolylineXY &points, QDomDocument &doc )
{
  QDomElement coordElem = doc.createElement( QStringLiteral( "gml:coordinates" ) );
  coordElem.setAttribute( QStringLiteral( "cs" ), QStringLiteral( "," ) );
  coordElem.setAttribute( QStringLiteral( "ts" ), QStringLiteral( " " ) );

  QString coordString;
  QVector<QgsPointXY>::const_iterator pointIt = points.constBegin();
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

  const QDomText coordText = doc.createTextNode( coordString );
  coordElem.appendChild( coordText );
  return coordElem;
}

QDomElement QgsOgcUtils::createGMLPositions( const QgsPolylineXY &points, QDomDocument &doc )
{
  QDomElement posElem = doc.createElement( QStringLiteral( "gml:pos" ) );
  if ( points.size() > 1 )
    posElem = doc.createElement( QStringLiteral( "gml:posList" ) );
  posElem.setAttribute( QStringLiteral( "srsDimension" ), QStringLiteral( "2" ) );

  QString coordString;
  QVector<QgsPointXY>::const_iterator pointIt = points.constBegin();
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

  const QDomText coordText = doc.createTextNode( coordString );
  posElem.appendChild( coordText );
  return posElem;
}



// -----------------------------------------

QColor QgsOgcUtils::colorFromOgcFill( const QDomElement &fillElement )
{
  if ( fillElement.isNull() || !fillElement.hasChildNodes() )
  {
    return QColor();
  }

  QString cssName;
  QString elemText;
  QColor color;
  QDomElement cssElem = fillElement.firstChildElement( QStringLiteral( "CssParameter" ) );
  while ( !cssElem.isNull() )
  {
    cssName = cssElem.attribute( QStringLiteral( "name" ), QStringLiteral( "not_found" ) );
    if ( cssName != QLatin1String( "not_found" ) )
    {
      elemText = cssElem.text();
      if ( cssName == QLatin1String( "fill" ) )
      {
        color.setNamedColor( elemText );
      }
      else if ( cssName == QLatin1String( "fill-opacity" ) )
      {
        bool ok;
        const double opacity = elemText.toDouble( &ok );
        if ( ok )
        {
          color.setAlphaF( opacity );
        }
      }
    }

    cssElem = cssElem.nextSiblingElement( QStringLiteral( "CssParameter" ) );
  }

  return color;
}


QgsExpression *QgsOgcUtils::expressionFromOgcFilter( const QDomElement &element, QgsVectorLayer *layer )
{
  return expressionFromOgcFilter( element, QgsOgcUtils::FILTER_OGC_1_0, layer );
}

QgsExpression *QgsOgcUtils::expressionFromOgcFilter( const QDomElement &element, const FilterVersion version, QgsVectorLayer *layer )
{
  if ( element.isNull() || !element.hasChildNodes() )
    return nullptr;

  QgsExpression *expr = new QgsExpression();

  // check if it is a single string value not having DOM elements
  // that express OGC operators
  if ( element.firstChild().nodeType() == QDomNode::TextNode )
  {
    expr->setExpression( element.firstChild().nodeValue() );
    return expr;
  }

  QgsOgcUtilsExpressionFromFilter utils( version, layer );

  // then check OGC DOM elements that contain OGC tags specifying
  // OGC operators.
  QDomElement childElem = element.firstChildElement();
  while ( !childElem.isNull() )
  {
    QgsExpressionNode *node = utils.nodeFromOgcFilter( childElem );

    if ( !node )
    {
      // invalid expression, parser error
      expr->d->mParserErrorString = utils.errorMessage();
      return expr;
    }

    // use the concat binary operator to append to the root node
    if ( !expr->d->mRootNode )
    {
      expr->d->mRootNode = node;
    }
    else
    {
      expr->d->mRootNode = new QgsExpressionNodeBinaryOperator( QgsExpressionNodeBinaryOperator::boConcat, expr->d->mRootNode, node );
    }

    childElem = childElem.nextSiblingElement();
  }

  // update expression string
  expr->d->mExp = expr->dump();

  return expr;
}

typedef QMap<QString, int> IntMap;
Q_GLOBAL_STATIC_WITH_ARGS( IntMap, BINARY_OPERATORS_TAG_NAMES_MAP, (
{
  // logical
  {  QLatin1String( "Or" ), QgsExpressionNodeBinaryOperator::boOr },
  {  QLatin1String( "And" ), QgsExpressionNodeBinaryOperator::boAnd },
  // comparison
  {  QLatin1String( "PropertyIsEqualTo" ), QgsExpressionNodeBinaryOperator::boEQ },
  {  QLatin1String( "PropertyIsNotEqualTo" ), QgsExpressionNodeBinaryOperator::boNE },
  {  QLatin1String( "PropertyIsLessThanOrEqualTo" ), QgsExpressionNodeBinaryOperator::boLE },
  {  QLatin1String( "PropertyIsGreaterThanOrEqualTo" ), QgsExpressionNodeBinaryOperator::boGE },
  {  QLatin1String( "PropertyIsLessThan" ), QgsExpressionNodeBinaryOperator::boLT },
  {  QLatin1String( "PropertyIsGreaterThan" ), QgsExpressionNodeBinaryOperator::boGT },
  {  QLatin1String( "PropertyIsLike" ), QgsExpressionNodeBinaryOperator::boLike },
  // arithmetic
  {  QLatin1String( "Add" ), QgsExpressionNodeBinaryOperator::boPlus },
  {  QLatin1String( "Sub" ), QgsExpressionNodeBinaryOperator::boMinus },
  {  QLatin1String( "Mul" ), QgsExpressionNodeBinaryOperator::boMul },
  {  QLatin1String( "Div" ), QgsExpressionNodeBinaryOperator::boDiv },
} ) )

static int binaryOperatorFromTagName( const QString &tagName )
{

  return BINARY_OPERATORS_TAG_NAMES_MAP()->value( tagName, -1 );
}

static QString binaryOperatorToTagName( QgsExpressionNodeBinaryOperator::BinaryOperator op )
{
  if ( op == QgsExpressionNodeBinaryOperator::boILike )
  {
    return QStringLiteral( "PropertyIsLike" );
  }
  return BINARY_OPERATORS_TAG_NAMES_MAP()->key( op, QString() );
}

static bool isBinaryOperator( const QString &tagName )
{
  return binaryOperatorFromTagName( tagName ) >= 0;
}


static bool isSpatialOperator( const QString &tagName )
{
  static QStringList spatialOps;
  if ( spatialOps.isEmpty() )
  {
    spatialOps << QStringLiteral( "BBOX" ) << QStringLiteral( "Intersects" ) << QStringLiteral( "Contains" ) << QStringLiteral( "Crosses" ) << QStringLiteral( "Equals" )
               << QStringLiteral( "Disjoint" ) << QStringLiteral( "Overlaps" ) << QStringLiteral( "Touches" ) << QStringLiteral( "Within" );
  }

  return spatialOps.contains( tagName );
}

QgsExpressionNode *QgsOgcUtils::nodeFromOgcFilter( QDomElement &element, QString &errorMessage, QgsVectorLayer *layer )
{
  QgsOgcUtilsExpressionFromFilter utils( QgsOgcUtils::FILTER_OGC_1_0, layer );
  QgsExpressionNode *node = utils.nodeFromOgcFilter( element );
  errorMessage = utils.errorMessage();
  return node;
}

QgsExpressionNodeBinaryOperator *QgsOgcUtils::nodeBinaryOperatorFromOgcFilter( QDomElement &element, QString &errorMessage, QgsVectorLayer *layer )
{
  QgsOgcUtilsExpressionFromFilter utils( QgsOgcUtils::FILTER_OGC_1_0, layer );
  QgsExpressionNodeBinaryOperator *node = utils.nodeBinaryOperatorFromOgcFilter( element );
  errorMessage = utils.errorMessage();
  return node;
}

QgsExpressionNodeFunction *QgsOgcUtils::nodeSpatialOperatorFromOgcFilter( QDomElement &element, QString &errorMessage )
{
  QgsOgcUtilsExpressionFromFilter utils( QgsOgcUtils::FILTER_OGC_1_0 );
  QgsExpressionNodeFunction *node = utils.nodeSpatialOperatorFromOgcFilter( element );
  errorMessage = utils.errorMessage();
  return node;
}

QgsExpressionNodeUnaryOperator *QgsOgcUtils::nodeNotFromOgcFilter( QDomElement &element, QString &errorMessage )
{
  QgsOgcUtilsExpressionFromFilter utils( QgsOgcUtils::FILTER_OGC_1_0 );
  QgsExpressionNodeUnaryOperator *node = utils.nodeNotFromOgcFilter( element );
  errorMessage = utils.errorMessage();
  return node;
}

QgsExpressionNodeFunction *QgsOgcUtils::nodeFunctionFromOgcFilter( QDomElement &element, QString &errorMessage )
{
  QgsOgcUtilsExpressionFromFilter utils( QgsOgcUtils::FILTER_OGC_1_0 );
  QgsExpressionNodeFunction *node = utils.nodeFunctionFromOgcFilter( element );
  errorMessage = utils.errorMessage();
  return node;
}

QgsExpressionNode *QgsOgcUtils::nodeLiteralFromOgcFilter( QDomElement &element, QString &errorMessage, QgsVectorLayer *layer )
{
  QgsOgcUtilsExpressionFromFilter utils( QgsOgcUtils::FILTER_OGC_1_0, layer );
  QgsExpressionNode *node = utils.nodeLiteralFromOgcFilter( element );
  errorMessage = utils.errorMessage();
  return node;
}

QgsExpressionNodeColumnRef *QgsOgcUtils::nodeColumnRefFromOgcFilter( QDomElement &element, QString &errorMessage )
{
  QgsOgcUtilsExpressionFromFilter utils( QgsOgcUtils::FILTER_OGC_1_0 );
  QgsExpressionNodeColumnRef *node = utils.nodeColumnRefFromOgcFilter( element );
  errorMessage = utils.errorMessage();
  return node;
}

QgsExpressionNode *QgsOgcUtils::nodeIsBetweenFromOgcFilter( QDomElement &element, QString &errorMessage )
{
  QgsOgcUtilsExpressionFromFilter utils( QgsOgcUtils::FILTER_OGC_1_0 );
  QgsExpressionNode *node = utils.nodeIsBetweenFromOgcFilter( element );
  errorMessage = utils.errorMessage();
  return node;
}

QgsExpressionNodeBinaryOperator *QgsOgcUtils::nodePropertyIsNullFromOgcFilter( QDomElement &element, QString &errorMessage )
{
  QgsOgcUtilsExpressionFromFilter utils( QgsOgcUtils::FILTER_OGC_1_0 );
  QgsExpressionNodeBinaryOperator *node = utils.nodePropertyIsNullFromOgcFilter( element );
  errorMessage = utils.errorMessage();
  return node;
}


/////////////////


QDomElement QgsOgcUtils::expressionToOgcFilter( const QgsExpression &exp, QDomDocument &doc, QString *errorMessage )
{
  return expressionToOgcFilter( exp, doc, GML_2_1_2, FILTER_OGC_1_0,
                                QStringLiteral( "geometry" ), QString(), false, false, errorMessage );
}

QDomElement QgsOgcUtils::expressionToOgcExpression( const QgsExpression &exp, QDomDocument &doc, QString *errorMessage )
{
  return expressionToOgcExpression( exp, doc, GML_2_1_2, FILTER_OGC_1_0,
                                    QStringLiteral( "geometry" ), QString(), false, false, errorMessage );
}

QDomElement QgsOgcUtils::expressionToOgcFilter( const QgsExpression &expression,
    QDomDocument &doc,
    GMLVersion gmlVersion,
    FilterVersion filterVersion,
    const QString &geometryName,
    const QString &srsName,
    bool honourAxisOrientation,
    bool invertAxisOrientation,
    QString *errorMessage )
{
  if ( !expression.rootNode() )
    return QDomElement();

  QgsExpression exp = expression;

  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope();
  QgsOgcUtilsExprToFilter utils( doc, gmlVersion, filterVersion, geometryName, srsName, honourAxisOrientation, invertAxisOrientation );
  const QDomElement exprRootElem = utils.expressionNodeToOgcFilter( exp.rootNode(), &exp, &context );
  if ( errorMessage )
    *errorMessage = utils.errorMessage();
  if ( exprRootElem.isNull() )
    return QDomElement();

  QDomElement filterElem =
    ( filterVersion == FILTER_FES_2_0 ) ?
    doc.createElementNS( FES_NAMESPACE, QStringLiteral( "fes:Filter" ) ) :
    doc.createElementNS( OGC_NAMESPACE, QStringLiteral( "ogc:Filter" ) );
  if ( utils.GMLNamespaceUsed() )
  {
    QDomAttr attr = doc.createAttribute( QStringLiteral( "xmlns:gml" ) );
    if ( gmlVersion == GML_3_2_1 )
      attr.setValue( GML32_NAMESPACE );
    else
      attr.setValue( GML_NAMESPACE );
    filterElem.setAttributeNode( attr );
  }
  filterElem.appendChild( exprRootElem );
  return filterElem;
}

QDomElement QgsOgcUtils::expressionToOgcExpression( const QgsExpression &expression,
    QDomDocument &doc,
    GMLVersion gmlVersion,
    FilterVersion filterVersion,
    const QString &geometryName,
    const QString &srsName,
    bool honourAxisOrientation,
    bool invertAxisOrientation,
    QString *errorMessage )
{
  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope();

  QgsExpression exp = expression;

  const QgsExpressionNode *node = exp.rootNode();
  if ( !node )
    return QDomElement();

  switch ( node->nodeType() )
  {
    case QgsExpressionNode::ntFunction:
    case QgsExpressionNode::ntLiteral:
    case QgsExpressionNode::ntColumnRef:
    {
      QgsOgcUtilsExprToFilter utils( doc, gmlVersion, filterVersion, geometryName, srsName, honourAxisOrientation, invertAxisOrientation );
      const QDomElement exprRootElem = utils.expressionNodeToOgcFilter( node, &exp, &context );

      if ( errorMessage )
        *errorMessage = utils.errorMessage();

      if ( !exprRootElem.isNull() )
      {
        return exprRootElem;
      }
      break;
    }
    default:
    {
      if ( errorMessage )
        *errorMessage = QObject::tr( "Node type not supported in expression translation: %1" ).arg( node->nodeType() );
    }
  }
  // got an error
  return QDomElement();
}

QDomElement QgsOgcUtils::SQLStatementToOgcFilter( const QgsSQLStatement &statement,
    QDomDocument &doc,
    GMLVersion gmlVersion,
    FilterVersion filterVersion,
    const QList<LayerProperties> &layerProperties,
    bool honourAxisOrientation,
    bool invertAxisOrientation,
    const QMap< QString, QString> &mapUnprefixedTypenameToPrefixedTypename,
    QString *errorMessage )
{
  if ( !statement.rootNode() )
    return QDomElement();

  QgsOgcUtilsSQLStatementToFilter utils( doc, gmlVersion, filterVersion,
                                         layerProperties, honourAxisOrientation, invertAxisOrientation,
                                         mapUnprefixedTypenameToPrefixedTypename );
  const QDomElement exprRootElem = utils.toOgcFilter( statement.rootNode() );
  if ( errorMessage )
    *errorMessage = utils.errorMessage();
  if ( exprRootElem.isNull() )
    return QDomElement();

  QDomElement filterElem =
    ( filterVersion == FILTER_FES_2_0 ) ?
    doc.createElementNS( FES_NAMESPACE, QStringLiteral( "fes:Filter" ) ) :
    doc.createElementNS( OGC_NAMESPACE, QStringLiteral( "ogc:Filter" ) );
  if ( utils.GMLNamespaceUsed() )
  {
    QDomAttr attr = doc.createAttribute( QStringLiteral( "xmlns:gml" ) );
    if ( gmlVersion == GML_3_2_1 )
      attr.setValue( GML32_NAMESPACE );
    else
      attr.setValue( GML_NAMESPACE );
    filterElem.setAttributeNode( attr );
  }

  QSet<QString> setNamespaceURI;
  for ( const LayerProperties &props : layerProperties )
  {
    if ( !props.mNamespacePrefix.isEmpty() && !props.mNamespaceURI.isEmpty() &&
         !setNamespaceURI.contains( props.mNamespaceURI ) )
    {
      setNamespaceURI.insert( props.mNamespaceURI );
      QDomAttr attr = doc.createAttribute( QStringLiteral( "xmlns:" ) + props.mNamespacePrefix );
      attr.setValue( props.mNamespaceURI );
      filterElem.setAttributeNode( attr );
    }
  }

  filterElem.appendChild( exprRootElem );
  return filterElem;
}

//


QDomElement QgsOgcUtilsExprToFilter::expressionNodeToOgcFilter( const QgsExpressionNode *node, QgsExpression *expression, const QgsExpressionContext *context )
{
  switch ( node->nodeType() )
  {
    case QgsExpressionNode::ntUnaryOperator:
      return expressionUnaryOperatorToOgcFilter( static_cast<const QgsExpressionNodeUnaryOperator *>( node ), expression, context );
    case QgsExpressionNode::ntBinaryOperator:
      return expressionBinaryOperatorToOgcFilter( static_cast<const QgsExpressionNodeBinaryOperator *>( node ), expression, context );
    case QgsExpressionNode::ntInOperator:
      return expressionInOperatorToOgcFilter( static_cast<const QgsExpressionNodeInOperator *>( node ), expression, context );
    case QgsExpressionNode::ntFunction:
      return expressionFunctionToOgcFilter( static_cast<const QgsExpressionNodeFunction *>( node ), expression, context );
    case QgsExpressionNode::ntLiteral:
      return expressionLiteralToOgcFilter( static_cast<const QgsExpressionNodeLiteral *>( node ), expression, context );
    case QgsExpressionNode::ntColumnRef:
      return expressionColumnRefToOgcFilter( static_cast<const QgsExpressionNodeColumnRef *>( node ), expression, context );

    default:
      mErrorMessage = QObject::tr( "Node type not supported: %1" ).arg( node->nodeType() );
      return QDomElement();
  }
}

QDomElement QgsOgcUtilsExprToFilter::expressionUnaryOperatorToOgcFilter( const QgsExpressionNodeUnaryOperator *node, QgsExpression *expression, const QgsExpressionContext *context )
{
  const QDomElement operandElem = expressionNodeToOgcFilter( node->operand(), expression, context );
  if ( !mErrorMessage.isEmpty() )
    return QDomElement();

  QDomElement uoElem;
  switch ( node->op() )
  {
    case QgsExpressionNodeUnaryOperator::uoMinus:
      uoElem = mDoc.createElement( mFilterPrefix + ":Literal" );
      if ( node->operand()->nodeType() == QgsExpressionNode::ntLiteral )
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
    case QgsExpressionNodeUnaryOperator::uoNot:
      uoElem = mDoc.createElement( mFilterPrefix + ":Not" );
      uoElem.appendChild( operandElem );
      break;

    default:
      mErrorMessage = QObject::tr( "Unary operator '%1' not implemented yet" ).arg( node->text() );
      return QDomElement();
  }

  return uoElem;
}


QDomElement QgsOgcUtilsExprToFilter::expressionBinaryOperatorToOgcFilter( const QgsExpressionNodeBinaryOperator *node, QgsExpression *expression, const QgsExpressionContext *context )
{
  const QDomElement leftElem = expressionNodeToOgcFilter( node->opLeft(), expression, context );
  if ( !mErrorMessage.isEmpty() )
    return QDomElement();

  QgsExpressionNodeBinaryOperator::BinaryOperator op = node->op();

  // before right operator is parsed: to allow NULL handling
  if ( op == QgsExpressionNodeBinaryOperator::boIs || op == QgsExpressionNodeBinaryOperator::boIsNot )
  {
    if ( node->opRight()->nodeType() == QgsExpressionNode::ntLiteral )
    {
      const QgsExpressionNodeLiteral *rightLit = static_cast<const QgsExpressionNodeLiteral *>( node->opRight() );
      if ( rightLit->value().isNull() )
      {

        QDomElement elem = mDoc.createElement( mFilterPrefix + ":PropertyIsNull" );
        elem.appendChild( leftElem );

        if ( op == QgsExpressionNodeBinaryOperator::boIsNot )
        {
          QDomElement notElem = mDoc.createElement( mFilterPrefix + ":Not" );
          notElem.appendChild( elem );
          return notElem;
        }

        return elem;
      }

      // continue with equal / not equal operator once the null case is handled
      op = ( op == QgsExpressionNodeBinaryOperator::boIs ? QgsExpressionNodeBinaryOperator::boEQ : QgsExpressionNodeBinaryOperator::boNE );
    }

  }

  const QDomElement rightElem = expressionNodeToOgcFilter( node->opRight(), expression, context );
  if ( !mErrorMessage.isEmpty() )
    return QDomElement();


  const QString opText = binaryOperatorToTagName( op );
  if ( opText.isEmpty() )
  {
    // not implemented binary operators
    // TODO: regex, % (mod), ^ (pow) are not supported yet
    mErrorMessage = QObject::tr( "Binary operator %1 not implemented yet" ).arg( node->text() );
    return QDomElement();
  }

  QDomElement boElem = mDoc.createElement( mFilterPrefix + ":" + opText );

  if ( op == QgsExpressionNodeBinaryOperator::boLike || op == QgsExpressionNodeBinaryOperator::boILike )
  {
    if ( op == QgsExpressionNodeBinaryOperator::boILike )
      boElem.setAttribute( QStringLiteral( "matchCase" ), QStringLiteral( "false" ) );

    // setup wildCards to <ogc:PropertyIsLike>
    boElem.setAttribute( QStringLiteral( "wildCard" ), QStringLiteral( "%" ) );
    boElem.setAttribute( QStringLiteral( "singleChar" ), QStringLiteral( "_" ) );
    if ( mFilterVersion == QgsOgcUtils::FILTER_OGC_1_0 )
      boElem.setAttribute( QStringLiteral( "escape" ), QStringLiteral( "\\" ) );
    else
      boElem.setAttribute( QStringLiteral( "escapeChar" ), QStringLiteral( "\\" ) );
  }

  boElem.appendChild( leftElem );
  boElem.appendChild( rightElem );
  return boElem;
}


QDomElement QgsOgcUtilsExprToFilter::expressionLiteralToOgcFilter( const QgsExpressionNodeLiteral *node, QgsExpression *expression, const QgsExpressionContext *context )
{
  Q_UNUSED( expression )
  Q_UNUSED( context )
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
    case QVariant::Date:
      value = node->value().toDate().toString( Qt::ISODate );
      break;
    case QVariant::DateTime:
      value = node->value().toDateTime().toString( Qt::ISODate );
      break;

    default:
      mErrorMessage = QObject::tr( "Literal type not supported: %1" ).arg( node->value().type() );
      return QDomElement();
  }

  QDomElement litElem = mDoc.createElement( mFilterPrefix + ":Literal" );
  litElem.appendChild( mDoc.createTextNode( value ) );
  return litElem;
}


QDomElement QgsOgcUtilsExprToFilter::expressionColumnRefToOgcFilter( const QgsExpressionNodeColumnRef *node, QgsExpression *expression, const QgsExpressionContext *context )
{
  Q_UNUSED( expression )
  Q_UNUSED( context )
  QDomElement propElem = mDoc.createElement( mFilterPrefix + ":" + mPropertyName );
  propElem.appendChild( mDoc.createTextNode( node->name() ) );
  return propElem;
}



QDomElement QgsOgcUtilsExprToFilter::expressionInOperatorToOgcFilter( const QgsExpressionNodeInOperator *node, QgsExpression *expression, const QgsExpressionContext *context )
{
  if ( node->list()->list().size() == 1 )
    return expressionNodeToOgcFilter( node->list()->list()[0], expression, context );

  QDomElement orElem = mDoc.createElement( mFilterPrefix + ":Or" );
  const QDomElement leftNode = expressionNodeToOgcFilter( node->node(), expression, context );

  const auto constList = node->list()->list();
  for ( QgsExpressionNode *n : constList )
  {
    const QDomElement listNode = expressionNodeToOgcFilter( n, expression, context );
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

Q_GLOBAL_STATIC_WITH_ARGS( QgsStringMap, BINARY_SPATIAL_OPS_MAP, (
{
  { QLatin1String( "disjoint" ),   QLatin1String( "Disjoint" ) },
  { QLatin1String( "intersects" ), QLatin1String( "Intersects" )},
  { QLatin1String( "touches" ),    QLatin1String( "Touches" ) },
  { QLatin1String( "crosses" ),    QLatin1String( "Crosses" ) },
  { QLatin1String( "contains" ),   QLatin1String( "Contains" ) },
  { QLatin1String( "overlaps" ),   QLatin1String( "Overlaps" ) },
  { QLatin1String( "within" ),     QLatin1String( "Within" ) }
} ) )

static bool isBinarySpatialOperator( const QString &fnName )
{
  return BINARY_SPATIAL_OPS_MAP()->contains( fnName );
}

static QString tagNameForSpatialOperator( const QString &fnName )
{
  return BINARY_SPATIAL_OPS_MAP()->value( fnName );
}

static bool isGeometryColumn( const QgsExpressionNode *node )
{
  if ( node->nodeType() != QgsExpressionNode::ntFunction )
    return false;

  const QgsExpressionNodeFunction *fn = static_cast<const QgsExpressionNodeFunction *>( node );
  QgsExpressionFunction *fd = QgsExpression::Functions()[fn->fnIndex()];
  return fd->name() == QLatin1String( "$geometry" );
}

static QgsGeometry geometryFromConstExpr( const QgsExpressionNode *node )
{
  // Right now we support only geomFromWKT(' ..... ')
  // Ideally we should support any constant sub-expression (not dependent on feature's geometry or attributes)

  if ( node->nodeType() == QgsExpressionNode::ntFunction )
  {
    const QgsExpressionNodeFunction *fnNode = static_cast<const QgsExpressionNodeFunction *>( node );
    QgsExpressionFunction *fnDef = QgsExpression::Functions()[fnNode->fnIndex()];
    if ( fnDef->name() == QLatin1String( "geom_from_wkt" ) )
    {
      const QList<QgsExpressionNode *> &args = fnNode->args()->list();
      if ( args[0]->nodeType() == QgsExpressionNode::ntLiteral )
      {
        const QString wkt = static_cast<const QgsExpressionNodeLiteral *>( args[0] )->value().toString();
        return QgsGeometry::fromWkt( wkt );
      }
    }
  }
  return QgsGeometry();
}


QDomElement QgsOgcUtilsExprToFilter::expressionFunctionToOgcFilter( const QgsExpressionNodeFunction *node, QgsExpression *expression, const QgsExpressionContext *context )
{
  QgsExpressionFunction *fd = QgsExpression::Functions()[node->fnIndex()];

  if ( fd->name() == QLatin1String( "intersects_bbox" ) )
  {
    QList<QgsExpressionNode *> argNodes = node->args()->list();
    Q_ASSERT( argNodes.count() == 2 ); // binary spatial ops must have two args

    const QgsGeometry geom = geometryFromConstExpr( argNodes[1] );
    if ( !geom.isNull() && isGeometryColumn( argNodes[0] ) )
    {
      QgsRectangle rect = geom.boundingBox();

      mGMLUsed = true;

      const QDomElement elemBox = ( mGMLVersion == QgsOgcUtils::GML_2_1_2 ) ?
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
      mErrorMessage = QObject::tr( "<BBOX> is currently supported only in form: bbox($geometry, geomFromWKT('…'))" );
      return QDomElement();
    }
  }

  if ( isBinarySpatialOperator( fd->name() ) )
  {
    QList<QgsExpressionNode *> argNodes = node->args()->list();
    Q_ASSERT( argNodes.count() == 2 ); // binary spatial ops must have two args

    QgsExpressionNode *otherNode = nullptr;
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
    if ( otherNode->nodeType() != QgsExpressionNode::ntFunction )
    {
      mErrorMessage = QObject::tr( "spatial operator: the other operator must be a geometry constructor function" );
      return QDomElement();
    }

    const QgsExpressionNodeFunction *otherFn = static_cast<const QgsExpressionNodeFunction *>( otherNode );
    QgsExpressionFunction *otherFnDef = QgsExpression::Functions()[otherFn->fnIndex()];
    if ( otherFnDef->name() == QLatin1String( "geom_from_wkt" ) )
    {
      QgsExpressionNode *firstFnArg = otherFn->args()->list()[0];
      if ( firstFnArg->nodeType() != QgsExpressionNode::ntLiteral )
      {
        mErrorMessage = QObject::tr( "geom_from_wkt: argument must be string literal" );
        return QDomElement();
      }
      const QString wkt = static_cast<const QgsExpressionNodeLiteral *>( firstFnArg )->value().toString();
      const QgsGeometry geom = QgsGeometry::fromWkt( wkt );
      otherGeomElem = QgsOgcUtils::geometryToGML( geom, mDoc, mGMLVersion, mSrsName, mInvertAxisOrientation,
                      QStringLiteral( "qgis_id_geom_%1" ).arg( mGeomId ) );
      mGeomId ++;
    }
    else if ( otherFnDef->name() == QLatin1String( "geom_from_gml" ) )
    {
      QgsExpressionNode *firstFnArg = otherFn->args()->list()[0];
      if ( firstFnArg->nodeType() != QgsExpressionNode::ntLiteral )
      {
        mErrorMessage = QObject::tr( "geom_from_gml: argument must be string literal" );
        return QDomElement();
      }

      QDomDocument geomDoc;
      const QString gml = static_cast<const QgsExpressionNodeLiteral *>( firstFnArg )->value().toString();
      if ( !geomDoc.setContent( gml, true ) )
      {
        mErrorMessage = QObject::tr( "geom_from_gml: unable to parse XML" );
        return QDomElement();
      }

      const QDomNode geomNode = mDoc.importNode( geomDoc.documentElement(), true );
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

  if ( fd->isStatic( node, expression, context ) )
  {
    const QVariant result = fd->run( node->args(), context, expression, node );
    const QgsExpressionNodeLiteral literal( result );
    return expressionLiteralToOgcFilter( &literal, expression, context );
  }

  if ( fd->params() == 0 )
  {
    mErrorMessage = QObject::tr( "Special columns/constants are not supported." );
    return QDomElement();
  }

  // this is somehow wrong - we are just hoping that the other side supports the same functions as we do...
  QDomElement funcElem = mDoc.createElement( mFilterPrefix + ":Function" );
  funcElem.setAttribute( QStringLiteral( "name" ), fd->name() );
  const auto constList = node->args()->list();
  for ( QgsExpressionNode *n : constList )
  {
    const QDomElement childElem = expressionNodeToOgcFilter( n, expression, context );
    if ( !mErrorMessage.isEmpty() )
      return QDomElement();

    funcElem.appendChild( childElem );
  }

  return funcElem;
}

//

QgsOgcUtilsSQLStatementToFilter::QgsOgcUtilsSQLStatementToFilter( QDomDocument &doc,
    QgsOgcUtils::GMLVersion gmlVersion,
    QgsOgcUtils::FilterVersion filterVersion,
    const QList<QgsOgcUtils::LayerProperties> &layerProperties,
    bool honourAxisOrientation,
    bool invertAxisOrientation,
    const QMap< QString, QString> &mapUnprefixedTypenameToPrefixedTypename )
  : mDoc( doc )
  , mGMLUsed( false )
  , mGMLVersion( gmlVersion )
  , mFilterVersion( filterVersion )
  , mLayerProperties( layerProperties )
  , mHonourAxisOrientation( honourAxisOrientation )
  , mInvertAxisOrientation( invertAxisOrientation )
  , mFilterPrefix( ( filterVersion == QgsOgcUtils::FILTER_FES_2_0 ) ? "fes" : "ogc" )
  , mPropertyName( ( filterVersion == QgsOgcUtils::FILTER_FES_2_0 ) ? "ValueReference" : "PropertyName" )
  , mGeomId( 1 )
  , mMapUnprefixedTypenameToPrefixedTypename( mapUnprefixedTypenameToPrefixedTypename )
{
}

QDomElement QgsOgcUtilsSQLStatementToFilter::toOgcFilter( const QgsSQLStatement::Node *node )
{
  switch ( node->nodeType() )
  {
    case QgsSQLStatement::ntUnaryOperator:
      return toOgcFilter( static_cast<const QgsSQLStatement::NodeUnaryOperator *>( node ) );
    case QgsSQLStatement::ntBinaryOperator:
      return toOgcFilter( static_cast<const QgsSQLStatement::NodeBinaryOperator *>( node ) );
    case QgsSQLStatement::ntInOperator:
      return toOgcFilter( static_cast<const QgsSQLStatement::NodeInOperator *>( node ) );
    case QgsSQLStatement::ntBetweenOperator:
      return toOgcFilter( static_cast<const QgsSQLStatement::NodeBetweenOperator *>( node ) );
    case QgsSQLStatement::ntFunction:
      return toOgcFilter( static_cast<const QgsSQLStatement::NodeFunction *>( node ) );
    case QgsSQLStatement::ntLiteral:
      return toOgcFilter( static_cast<const QgsSQLStatement::NodeLiteral *>( node ) );
    case QgsSQLStatement::ntColumnRef:
      return toOgcFilter( static_cast<const QgsSQLStatement::NodeColumnRef *>( node ) );
    case QgsSQLStatement::ntSelect:
      return toOgcFilter( static_cast<const QgsSQLStatement::NodeSelect *>( node ) );

    default:
      mErrorMessage = QObject::tr( "Node type not supported: %1" ).arg( node->nodeType() );
      return QDomElement();
  }
}


QDomElement QgsOgcUtilsSQLStatementToFilter::toOgcFilter( const QgsSQLStatement::NodeUnaryOperator *node )
{

  const QDomElement operandElem = toOgcFilter( node->operand() );
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
      mErrorMessage = QObject::tr( "Unary operator %1 not implemented yet" ).arg( QgsSQLStatement::UNARY_OPERATOR_TEXT[node->op()] );
      return QDomElement();
  }

  return uoElem;
}


QDomElement QgsOgcUtilsSQLStatementToFilter::toOgcFilter( const QgsSQLStatement::NodeBinaryOperator *node )
{
  const QDomElement leftElem = toOgcFilter( node->opLeft() );
  if ( !mErrorMessage.isEmpty() )
    return QDomElement();

  QgsSQLStatement::BinaryOperator op = node->op();

  // before right operator is parsed: to allow NULL handling
  if ( op == QgsSQLStatement::boIs || op == QgsSQLStatement::boIsNot )
  {
    if ( node->opRight()->nodeType() == QgsSQLStatement::ntLiteral )
    {
      const QgsSQLStatement::NodeLiteral *rightLit = static_cast<const QgsSQLStatement::NodeLiteral *>( node->opRight() );
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

  const QDomElement rightElem = toOgcFilter( node->opRight() );
  if ( !mErrorMessage.isEmpty() )
    return QDomElement();


  QString opText;
  if ( op == QgsSQLStatement::boOr )
    opText = QStringLiteral( "Or" );
  else if ( op == QgsSQLStatement::boAnd )
    opText = QStringLiteral( "And" );
  else if ( op == QgsSQLStatement::boEQ )
    opText = QStringLiteral( "PropertyIsEqualTo" );
  else if ( op == QgsSQLStatement::boNE )
    opText = QStringLiteral( "PropertyIsNotEqualTo" );
  else if ( op == QgsSQLStatement::boLE )
    opText = QStringLiteral( "PropertyIsLessThanOrEqualTo" );
  else if ( op == QgsSQLStatement::boGE )
    opText = QStringLiteral( "PropertyIsGreaterThanOrEqualTo" );
  else if ( op == QgsSQLStatement::boLT )
    opText = QStringLiteral( "PropertyIsLessThan" );
  else if ( op == QgsSQLStatement::boGT )
    opText = QStringLiteral( "PropertyIsGreaterThan" );
  else if ( op == QgsSQLStatement::boLike )
    opText = QStringLiteral( "PropertyIsLike" );
  else if ( op == QgsSQLStatement::boILike )
    opText = QStringLiteral( "PropertyIsLike" );

  if ( opText.isEmpty() )
  {
    // not implemented binary operators
    mErrorMessage = QObject::tr( "Binary operator %1 not implemented yet" ).arg( QgsSQLStatement::BINARY_OPERATOR_TEXT[op] );
    return QDomElement();
  }

  QDomElement boElem = mDoc.createElement( mFilterPrefix + ":" + opText );

  if ( op == QgsSQLStatement::boLike || op == QgsSQLStatement::boILike )
  {
    if ( op == QgsSQLStatement::boILike )
      boElem.setAttribute( QStringLiteral( "matchCase" ), QStringLiteral( "false" ) );

    // setup wildCards to <ogc:PropertyIsLike>
    boElem.setAttribute( QStringLiteral( "wildCard" ), QStringLiteral( "%" ) );
    boElem.setAttribute( QStringLiteral( "singleChar" ), QStringLiteral( "_" ) );
    if ( mFilterVersion == QgsOgcUtils::FILTER_OGC_1_0 )
      boElem.setAttribute( QStringLiteral( "escape" ), QStringLiteral( "\\" ) );
    else
      boElem.setAttribute( QStringLiteral( "escapeChar" ), QStringLiteral( "\\" ) );
  }

  boElem.appendChild( leftElem );
  boElem.appendChild( rightElem );
  return boElem;
}


QDomElement QgsOgcUtilsSQLStatementToFilter::toOgcFilter( const QgsSQLStatement::NodeLiteral *node )
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


QDomElement QgsOgcUtilsSQLStatementToFilter::toOgcFilter( const QgsSQLStatement::NodeColumnRef *node )
{
  QDomElement propElem = mDoc.createElement( mFilterPrefix + ":" + mPropertyName );
  if ( node->tableName().isEmpty() || mLayerProperties.size() == 1 )
  {
    if ( mLayerProperties.size() == 1 && !mLayerProperties[0].mNamespacePrefix.isEmpty() )
      propElem.appendChild( mDoc.createTextNode(
                              mLayerProperties[0].mNamespacePrefix + QStringLiteral( ":" ) + node->name() ) );
    else
      propElem.appendChild( mDoc.createTextNode( node->name() ) );
  }
  else
  {
    QString tableName( mMapTableAliasToNames[node->tableName()] );
    if ( mMapUnprefixedTypenameToPrefixedTypename.contains( tableName ) )
      tableName = mMapUnprefixedTypenameToPrefixedTypename[tableName];
    propElem.appendChild( mDoc.createTextNode( tableName + "/" + node->name() ) );
  }
  return propElem;
}

QDomElement QgsOgcUtilsSQLStatementToFilter::toOgcFilter( const QgsSQLStatement::NodeInOperator *node )
{
  if ( node->list()->list().size() == 1 )
    return toOgcFilter( node->list()->list()[0] );

  QDomElement orElem = mDoc.createElement( mFilterPrefix + ":Or" );
  const QDomElement leftNode = toOgcFilter( node->node() );

  const auto constList = node->list()->list();
  for ( QgsSQLStatement::Node *n : constList )
  {
    const QDomElement listNode = toOgcFilter( n );
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

QDomElement QgsOgcUtilsSQLStatementToFilter::toOgcFilter( const QgsSQLStatement::NodeBetweenOperator *node )
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

static QString mapBinarySpatialToOgc( const QString &name )
{
  QString nameCompare( name );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 2)
  if ( name.size() > 3 && name.midRef( 0, 3 ).compare( QLatin1String( "ST_" ), Qt::CaseInsensitive ) == 0 )
    nameCompare = name.mid( 3 );
#else
  if ( name.size() > 3 && QStringView {name}.mid( 0, 3 ).toString().compare( QLatin1String( "ST_" ), Qt::CaseInsensitive ) == 0 )
    nameCompare = name.mid( 3 );
#endif
  QStringList spatialOps;
  spatialOps << QStringLiteral( "BBOX" ) << QStringLiteral( "Intersects" ) << QStringLiteral( "Contains" ) << QStringLiteral( "Crosses" ) << QStringLiteral( "Equals" )
             << QStringLiteral( "Disjoint" ) << QStringLiteral( "Overlaps" ) << QStringLiteral( "Touches" ) << QStringLiteral( "Within" );
  const auto constSpatialOps = spatialOps;
  for ( QString op : constSpatialOps )
  {
    if ( nameCompare.compare( op, Qt::CaseInsensitive ) == 0 )
      return op;
  }
  return QString();
}

static QString mapTernarySpatialToOgc( const QString &name )
{
  QString nameCompare( name );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 2)
  if ( name.size() > 3 && name.midRef( 0, 3 ).compare( QLatin1String( "ST_" ), Qt::CaseInsensitive ) == 0 )
    nameCompare = name.mid( 3 );
#else
  if ( name.size() > 3 && QStringView {name}.mid( 0, 3 ).compare( QLatin1String( "ST_" ), Qt::CaseInsensitive ) == 0 )
    nameCompare = name.mid( 3 );
#endif
  if ( nameCompare.compare( QLatin1String( "DWithin" ), Qt::CaseInsensitive ) == 0 )
    return QStringLiteral( "DWithin" );
  if ( nameCompare.compare( QLatin1String( "Beyond" ), Qt::CaseInsensitive ) == 0 )
    return QStringLiteral( "Beyond" );
  return QString();
}

QString QgsOgcUtilsSQLStatementToFilter::getGeometryColumnSRSName( const QgsSQLStatement::Node *node )
{
  if ( node->nodeType() != QgsSQLStatement::ntColumnRef )
    return QString();

  const QgsSQLStatement::NodeColumnRef *col = static_cast<const QgsSQLStatement::NodeColumnRef *>( node );
  if ( !col->tableName().isEmpty() )
  {
    const auto constMLayerProperties = mLayerProperties;
    for ( const QgsOgcUtils::LayerProperties &prop : constMLayerProperties )
    {
      if ( prop.mName.compare( mMapTableAliasToNames[col->tableName()], Qt::CaseInsensitive ) == 0 &&
           prop.mGeometryAttribute.compare( col->name(), Qt::CaseInsensitive ) == 0 )
      {
        return prop.mSRSName;
      }
    }
  }
  if ( !mLayerProperties.empty() &&
       mLayerProperties.at( 0 ).mGeometryAttribute.compare( col->name(), Qt::CaseInsensitive ) == 0 )
  {
    return  mLayerProperties.at( 0 ).mSRSName;
  }
  return QString();
}

bool QgsOgcUtilsSQLStatementToFilter::processSRSName( const QgsSQLStatement::NodeFunction *mainNode,
    QList<QgsSQLStatement::Node *> args,
    bool lastArgIsSRSName,
    QString &srsName,
    bool &axisInversion )
{
  srsName = mCurrentSRSName;
  axisInversion = mInvertAxisOrientation;

  if ( lastArgIsSRSName )
  {
    QgsSQLStatement::Node *lastArg = args[ args.size() - 1 ];
    if ( lastArg->nodeType() != QgsSQLStatement::ntLiteral )
    {
      mErrorMessage = QObject::tr( "%1: Last argument must be string or integer literal" ).arg( mainNode->name() );
      return false;
    }
    const QgsSQLStatement::NodeLiteral *lit = static_cast<const QgsSQLStatement::NodeLiteral *>( lastArg );
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
      if ( srsName.startsWith( QLatin1String( "EPSG:" ), Qt::CaseInsensitive ) )
        return true;
    }
  }

  QgsCoordinateReferenceSystem crs;
  if ( !srsName.isEmpty() )
    crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( srsName );
  if ( crs.isValid() )
  {
    if ( mHonourAxisOrientation && crs.hasAxisInverted() )
    {
      axisInversion = !axisInversion;
    }
  }

  return true;
}

QDomElement QgsOgcUtilsSQLStatementToFilter::toOgcFilter( const QgsSQLStatement::NodeFunction *node )
{
  // ST_GeometryFromText
  if ( node->name().compare( QLatin1String( "ST_GeometryFromText" ), Qt::CaseInsensitive ) == 0 )
  {
    QList<QgsSQLStatement::Node *> args = node->args()->list();
    if ( args.size() != 1 && args.size() != 2 )
    {
      mErrorMessage = QObject::tr( "Function %1 should have 1 or 2 arguments" ).arg( node->name() );
      return QDomElement();
    }

    QgsSQLStatement::Node *firstFnArg = args[0];
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

    const QString wkt = static_cast<const QgsSQLStatement::NodeLiteral *>( firstFnArg )->value().toString();
    const QgsGeometry geom = QgsGeometry::fromWkt( wkt );
    const QDomElement geomElem = QgsOgcUtils::geometryToGML( geom, mDoc, mGMLVersion, srsName, axisInversion,
                                 QStringLiteral( "qgis_id_geom_%1" ).arg( mGeomId ) );
    mGeomId ++;
    if ( geomElem.isNull() )
    {
      mErrorMessage = QObject::tr( "%1: invalid WKT" ).arg( node->name() );
      return QDomElement();
    }
    mGMLUsed = true;
    return geomElem;
  }

  // ST_MakeEnvelope
  if ( node->name().compare( QLatin1String( "ST_MakeEnvelope" ), Qt::CaseInsensitive ) == 0 )
  {
    QList<QgsSQLStatement::Node *> args = node->args()->list();
    if ( args.size() != 4 && args.size() != 5 )
    {
      mErrorMessage = QObject::tr( "Function %1 should have 4 or 5 arguments" ).arg( node->name() );
      return QDomElement();
    }

    QgsRectangle rect;

    for ( int i = 0; i < 4; i++ )
    {
      QgsSQLStatement::Node *arg = args[i];
      if ( arg->nodeType() != QgsSQLStatement::ntLiteral )
      {
        mErrorMessage = QObject::tr( "%1: Argument %2 must be numeric literal" ).arg( node->name() ).arg( i + 1 );
        return QDomElement();
      }
      const QgsSQLStatement::NodeLiteral *lit = static_cast<const QgsSQLStatement::NodeLiteral *>( arg );
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
  if ( node->name().compare( QLatin1String( "ST_GeomFromGML" ), Qt::CaseInsensitive ) == 0 )
  {
    QList<QgsSQLStatement::Node *> args = node->args()->list();
    if ( args.size() != 1 )
    {
      mErrorMessage = QObject::tr( "Function %1 should have 1 argument" ).arg( node->name() );
      return QDomElement();
    }

    QgsSQLStatement::Node *firstFnArg = args[0];
    if ( firstFnArg->nodeType() != QgsSQLStatement::ntLiteral )
    {
      mErrorMessage = QObject::tr( "%1: Argument must be string literal" ).arg( node->name() );
      return QDomElement();
    }

    QDomDocument geomDoc;
    const QString gml = static_cast<const QgsSQLStatement::NodeLiteral *>( firstFnArg )->value().toString();
    if ( !geomDoc.setContent( gml, true ) )
    {
      mErrorMessage = QObject::tr( "ST_GeomFromGML: unable to parse XML" );
      return QDomElement();
    }

    const QDomNode geomNode = mDoc.importNode( geomDoc.documentElement(), true );
    mGMLUsed = true;
    return geomNode.toElement();
  }

  // Binary geometry operators
  QString ogcName( mapBinarySpatialToOgc( node->name() ) );
  if ( !ogcName.isEmpty() )
  {
    QList<QgsSQLStatement::Node *> args = node->args()->list();
    if ( args.size() != 2 )
    {
      mErrorMessage = QObject::tr( "Function %1 should have 2 arguments" ).arg( node->name() );
      return QDomElement();
    }

    for ( int i = 0; i < 2; i ++ )
    {
      if ( args[i]->nodeType() == QgsSQLStatement::ntFunction &&
           ( static_cast<const QgsSQLStatement::NodeFunction *>( args[i] )->name().compare( QLatin1String( "ST_GeometryFromText" ), Qt::CaseInsensitive ) == 0 ||
             static_cast<const QgsSQLStatement::NodeFunction *>( args[i] )->name().compare( QLatin1String( "ST_MakeEnvelope" ), Qt::CaseInsensitive ) == 0 ) )
      {
        mCurrentSRSName = getGeometryColumnSRSName( args[1 - i] );
        break;
      }
    }

    //if( ogcName == "Intersects" && mFilterVersion == QgsOgcUtils::FILTER_OGC_1_0 )
    //  ogcName = "Intersect";
    QDomElement funcElem = mDoc.createElement( mFilterPrefix + ":" + ogcName );
    const auto constArgs = args;
    for ( QgsSQLStatement::Node *n : constArgs )
    {
      const QDomElement childElem = toOgcFilter( n );
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
    QList<QgsSQLStatement::Node *> args = node->args()->list();
    if ( args.size() != 3 )
    {
      mErrorMessage = QObject::tr( "Function %1 should have 3 arguments" ).arg( node->name() );
      return QDomElement();
    }

    for ( int i = 0; i < 2; i ++ )
    {
      if ( args[i]->nodeType() == QgsSQLStatement::ntFunction &&
           ( static_cast<const QgsSQLStatement::NodeFunction *>( args[i] )->name().compare( QLatin1String( "ST_GeometryFromText" ), Qt::CaseInsensitive ) == 0 ||
             static_cast<const QgsSQLStatement::NodeFunction *>( args[i] )->name().compare( QLatin1String( "ST_MakeEnvelope" ), Qt::CaseInsensitive ) == 0 ) )
      {
        mCurrentSRSName = getGeometryColumnSRSName( args[1 - i] );
        break;
      }
    }

    QDomElement funcElem = mDoc.createElement( mFilterPrefix + ":" + node->name().mid( 3 ) );
    for ( int i = 0; i < 2; i++ )
    {
      const QDomElement childElem = toOgcFilter( args[i] );
      if ( !mErrorMessage.isEmpty() )
      {
        mCurrentSRSName.clear();
        return QDomElement();
      }

      funcElem.appendChild( childElem );
    }
    mCurrentSRSName.clear();

    QgsSQLStatement::Node *distanceNode = args[2];
    if ( distanceNode->nodeType() != QgsSQLStatement::ntLiteral )
    {
      mErrorMessage = QObject::tr( "Function %1 3rd argument should be a numeric value or a string made of a numeric value followed by a string" ).arg( node->name() );
      return QDomElement();
    }
    const QgsSQLStatement::NodeLiteral *lit = static_cast<const QgsSQLStatement::NodeLiteral *>( distanceNode );
    if ( lit->value().isNull() )
    {
      mErrorMessage = QObject::tr( "Function %1 3rd argument should be a numeric value or a string made of a numeric value followed by a string" ).arg( node->name() );
      return QDomElement();
    }
    QString distance;
    QString unit( QStringLiteral( "m" ) );
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
          if ( !( ( distance[i] >= '0' && distance[i] <= '9' ) || distance[i] == '-' || distance[i] == '.' || distance[i] == 'e' || distance[i] == 'E' ) )
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
      distanceElem.setAttribute( QStringLiteral( "uom" ), unit );
    else
      distanceElem.setAttribute( QStringLiteral( "unit" ), unit );
    distanceElem.appendChild( mDoc.createTextNode( distance ) );
    funcElem.appendChild( distanceElem );
    return funcElem;
  }

  // Other function
  QDomElement funcElem = mDoc.createElement( mFilterPrefix + ":Function" );
  funcElem.setAttribute( QStringLiteral( "name" ), node->name() );
  const auto constList = node->args()->list();
  for ( QgsSQLStatement::Node *n : constList )
  {
    const QDomElement childElem = toOgcFilter( n );
    if ( !mErrorMessage.isEmpty() )
      return QDomElement();

    funcElem.appendChild( childElem );
  }
  return funcElem;
}

QDomElement QgsOgcUtilsSQLStatementToFilter::toOgcFilter( const QgsSQLStatement::NodeJoin *node,
    const QString &leftTable )
{
  QgsSQLStatement::Node *onExpr = node->onExpr();
  if ( onExpr )
  {
    return toOgcFilter( onExpr );
  }

  QList<QDomElement> listElem;
  const auto constUsingColumns = node->usingColumns();
  for ( const QString &columnName : constUsingColumns )
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
    const auto constListElem = listElem;
    for ( const QDomElement &elem : constListElem )
    {
      andElem.appendChild( elem );
    }
    return andElem;
  }

  return QDomElement();
}

void QgsOgcUtilsSQLStatementToFilter::visit( const QgsSQLStatement::NodeTableDef *node )
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

QDomElement QgsOgcUtilsSQLStatementToFilter::toOgcFilter( const QgsSQLStatement::NodeSelect *node )
{
  QList<QDomElement> listElem;

  if ( mFilterVersion != QgsOgcUtils::FILTER_FES_2_0 &&
       ( node->tables().size() != 1 || !node->joins().empty() ) )
  {
    mErrorMessage = QObject::tr( "Joins are only supported with WFS 2.0" );
    return QDomElement();
  }

  // Register all table name aliases
  const auto constTables = node->tables();
  for ( QgsSQLStatement::NodeTableDef *table : constTables )
  {
    visit( table );
  }
  const auto constJoins = node->joins();
  for ( QgsSQLStatement::NodeJoin *join : constJoins )
  {
    visit( join->tableDef() );
  }

  // Process JOIN conditions
  const QList< QgsSQLStatement::NodeTableDef *> nodeTables = node->tables();
  QString leftTable = nodeTables.at( nodeTables.length() - 1 )->name();
  for ( QgsSQLStatement::NodeJoin *join : constJoins )
  {
    const QDomElement joinElem = toOgcFilter( join, leftTable );
    if ( !mErrorMessage.isEmpty() )
      return QDomElement();
    listElem.append( joinElem );
    leftTable = join->tableDef()->name();
  }

  // Process WHERE conditions
  if ( node->where() )
  {
    const QDomElement whereElem = toOgcFilter( node->where() );
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
    const auto constListElem = listElem;
    for ( const QDomElement &elem : constListElem )
    {
      andElem.appendChild( elem );
    }
    return andElem;
  }

  return QDomElement();
}

QgsOgcUtilsExpressionFromFilter::QgsOgcUtilsExpressionFromFilter( const QgsOgcUtils::FilterVersion version, const QgsVectorLayer *layer )
  : mLayer( layer )
{
  mPropertyName = QStringLiteral( "PropertyName" );
  mPrefix = QStringLiteral( "ogc" );

  if ( version == QgsOgcUtils::FILTER_FES_2_0 )
  {
    mPropertyName = QStringLiteral( "ValueReference" );
    mPrefix = QStringLiteral( "fes" );
  }
}

QgsExpressionNode *QgsOgcUtilsExpressionFromFilter::nodeFromOgcFilter( const QDomElement &element )
{
  if ( element.isNull() )
    return nullptr;

  // check for binary operators
  if ( isBinaryOperator( element.tagName() ) )
  {
    return nodeBinaryOperatorFromOgcFilter( element );
  }

  // check for spatial operators
  if ( isSpatialOperator( element.tagName() ) )
  {
    return nodeSpatialOperatorFromOgcFilter( element );
  }

  // check for other OGC operators, convert them to expressions
  if ( element.tagName() == QLatin1String( "Not" ) )
  {
    return nodeNotFromOgcFilter( element );
  }
  else if ( element.tagName() == QLatin1String( "PropertyIsNull" ) )
  {
    return nodePropertyIsNullFromOgcFilter( element );
  }
  else if ( element.tagName() == QLatin1String( "Literal" ) )
  {
    return nodeLiteralFromOgcFilter( element );
  }
  else if ( element.tagName() == QLatin1String( "Function" ) )
  {
    return nodeFunctionFromOgcFilter( element );
  }
  else if ( element.tagName() == mPropertyName )
  {
    return nodeColumnRefFromOgcFilter( element );
  }
  else if ( element.tagName() == QLatin1String( "PropertyIsBetween" ) )
  {
    return nodeIsBetweenFromOgcFilter( element );
  }

  mErrorMessage += QObject::tr( "unable to convert '%1' element to a valid expression: it is not supported yet or it has invalid arguments" ).arg( element.tagName() );
  return nullptr;
}

QgsExpressionNodeBinaryOperator *QgsOgcUtilsExpressionFromFilter::nodeBinaryOperatorFromOgcFilter( const QDomElement &element )
{
  if ( element.isNull() )
    return nullptr;

  int op = binaryOperatorFromTagName( element.tagName() );
  if ( op < 0 )
  {
    mErrorMessage = QObject::tr( "'%1' binary operator not supported." ).arg( element.tagName() );
    return nullptr;
  }

  if ( op == QgsExpressionNodeBinaryOperator::boLike && element.hasAttribute( QStringLiteral( "matchCase" ) ) && element.attribute( QStringLiteral( "matchCase" ) ) == QLatin1String( "false" ) )
  {
    op = QgsExpressionNodeBinaryOperator::boILike;
  }

  QDomElement operandElem = element.firstChildElement();
  std::unique_ptr<QgsExpressionNode> expr( nodeFromOgcFilter( operandElem ) );

  if ( !expr )
  {
    mErrorMessage = QObject::tr( "invalid left operand for '%1' binary operator" ).arg( element.tagName() );
    return nullptr;
  }

  const std::unique_ptr<QgsExpressionNode> leftOp( expr->clone() );
  for ( operandElem = operandElem.nextSiblingElement(); !operandElem.isNull(); operandElem = operandElem.nextSiblingElement() )
  {
    std::unique_ptr<QgsExpressionNode> opRight( nodeFromOgcFilter( operandElem ) );
    if ( !opRight )
    {
      mErrorMessage = QObject::tr( "invalid right operand for '%1' binary operator" ).arg( element.tagName() );
      return nullptr;
    }

    if ( op == QgsExpressionNodeBinaryOperator::boLike || op == QgsExpressionNodeBinaryOperator::boILike )
    {
      QString wildCard;
      if ( element.hasAttribute( QStringLiteral( "wildCard" ) ) )
      {
        wildCard = element.attribute( QStringLiteral( "wildCard" ) );
      }
      QString singleChar;
      if ( element.hasAttribute( QStringLiteral( "singleChar" ) ) )
      {
        singleChar = element.attribute( QStringLiteral( "singleChar" ) );
      }
      QString escape = QStringLiteral( "\\" );
      if ( element.hasAttribute( QStringLiteral( "escape" ) ) )
      {
        escape = element.attribute( QStringLiteral( "escape" ) );
      }
      if ( element.hasAttribute( QStringLiteral( "escapeChar" ) ) )
      {
        escape = element.attribute( QStringLiteral( "escapeChar" ) );
      }
      // replace
      QString oprValue = static_cast<const QgsExpressionNodeLiteral *>( opRight.get() )->value().toString();
      if ( !wildCard.isEmpty() && wildCard != QLatin1String( "%" ) )
      {
        oprValue.replace( '%', QLatin1String( "\\%" ) );
        if ( oprValue.startsWith( wildCard ) )
        {
          oprValue.replace( 0, 1, QStringLiteral( "%" ) );
        }
        const QRegularExpression rx( "[^" + QgsStringUtils::qRegExpEscape( escape ) + "](" + QgsStringUtils::qRegExpEscape( wildCard ) + ")" );
        QRegularExpressionMatch match = rx.match( oprValue );
        int pos;
        while ( match.hasMatch() )
        {
          pos = match.capturedStart();
          oprValue.replace( pos + 1, 1, QStringLiteral( "%" ) );
          pos += 1;
          match = rx.match( oprValue, pos );
        }
        oprValue.replace( escape + wildCard, wildCard );
      }
      if ( !singleChar.isEmpty() && singleChar != QLatin1String( "_" ) )
      {
        oprValue.replace( '_', QLatin1String( "\\_" ) );
        if ( oprValue.startsWith( singleChar ) )
        {
          oprValue.replace( 0, 1, QStringLiteral( "_" ) );
        }
        const QRegularExpression rx( "[^" + QgsStringUtils::qRegExpEscape( escape ) + "](" + QgsStringUtils::qRegExpEscape( singleChar ) + ")" );
        QRegularExpressionMatch match = rx.match( oprValue );
        int pos;
        while ( match.hasMatch() )
        {
          pos = match.capturedStart();
          oprValue.replace( pos + 1, 1, QStringLiteral( "_" ) );
          pos += 1;
          match = rx.match( oprValue, pos );
        }
        oprValue.replace( escape + singleChar, singleChar );
      }
      if ( !escape.isEmpty() && escape != QLatin1String( "\\" ) )
      {
        oprValue.replace( escape + escape, escape );
      }
      opRight.reset( new QgsExpressionNodeLiteral( oprValue ) );
    }

    expr.reset( new QgsExpressionNodeBinaryOperator( static_cast< QgsExpressionNodeBinaryOperator::BinaryOperator >( op ), expr.release(), opRight.release() ) );
  }

  if ( expr == leftOp )
  {
    mErrorMessage = QObject::tr( "only one operand for '%1' binary operator" ).arg( element.tagName() );
    return nullptr;
  }

  return dynamic_cast< QgsExpressionNodeBinaryOperator * >( expr.release() );
}


QgsExpressionNodeFunction *QgsOgcUtilsExpressionFromFilter::nodeSpatialOperatorFromOgcFilter( const QDomElement &element )
{
  // we are exploiting the fact that our function names are the same as the XML tag names
  const int opIdx = QgsExpression::functionIndex( element.tagName().toLower() );

  std::unique_ptr<QgsExpressionNode::NodeList> gml2Args( new QgsExpressionNode::NodeList() );
  QDomElement childElem = element.firstChildElement();
  QString gml2Str;
  while ( !childElem.isNull() && gml2Str.isEmpty() )
  {
    if ( childElem.tagName() != mPropertyName )
    {
      QTextStream gml2Stream( &gml2Str );
      childElem.save( gml2Stream, 0 );
    }
    childElem = childElem.nextSiblingElement();
  }
  if ( !gml2Str.isEmpty() )
  {
    gml2Args->append( new QgsExpressionNodeLiteral( QVariant( gml2Str.remove( '\n' ) ) ) );
  }
  else
  {
    mErrorMessage = QObject::tr( "No OGC Geometry found" );
    return nullptr;
  }

  std::unique_ptr<QgsExpressionNode::NodeList> opArgs( new QgsExpressionNode::NodeList() );
  opArgs->append( new QgsExpressionNodeFunction( QgsExpression::functionIndex( QStringLiteral( "$geometry" ) ), new QgsExpressionNode::NodeList() ) );
  opArgs->append( new QgsExpressionNodeFunction( QgsExpression::functionIndex( QStringLiteral( "geomFromGML" ) ), gml2Args.release() ) );

  return new QgsExpressionNodeFunction( opIdx, opArgs.release() );
}

QgsExpressionNodeColumnRef *QgsOgcUtilsExpressionFromFilter::nodeColumnRefFromOgcFilter( const QDomElement &element )
{
  if ( element.isNull() || element.tagName() != mPropertyName )
  {
    mErrorMessage = QObject::tr( "%1:PropertyName expected, got %2" ).arg( mPrefix, element.tagName() );
    return nullptr;
  }

  return new QgsExpressionNodeColumnRef( element.firstChild().nodeValue() );
}

QgsExpressionNode *QgsOgcUtilsExpressionFromFilter::nodeLiteralFromOgcFilter( const QDomElement &element )
{
  if ( element.isNull() || element.tagName() != QLatin1String( "Literal" ) )
  {
    mErrorMessage = QObject::tr( "%1:Literal expected, got %2" ).arg( mPrefix, element.tagName() );
    return nullptr;
  }

  std::unique_ptr<QgsExpressionNode> root;

  // the literal content can have more children (e.g. CDATA section, text, ...)
  QDomNode childNode = element.firstChild();
  while ( !childNode.isNull() )
  {
    std::unique_ptr<QgsExpressionNode> operand;

    if ( childNode.nodeType() == QDomNode::ElementNode )
    {
      // found a element node (e.g. PropertyName), convert it
      const QDomElement operandElem = childNode.toElement();
      operand.reset( nodeFromOgcFilter( operandElem ) );
      if ( !operand )
      {
        mErrorMessage = QObject::tr( "'%1' is an invalid or not supported content for %2:Literal" ).arg( operandElem.tagName(), mPrefix );
        return nullptr;
      }
    }
    else
    {
      // probably a text/CDATA node
      QVariant value = childNode.nodeValue();

      bool converted = false;

      // try to convert the node content to corresponding field type if possible
      if ( mLayer )
      {
        QDomElement propertyNameElement = element.previousSiblingElement( mPropertyName );
        if ( propertyNameElement.isNull() || propertyNameElement.tagName() != mPropertyName )
        {
          propertyNameElement = element.nextSiblingElement( mPropertyName );
        }
        if ( !propertyNameElement.isNull() || propertyNameElement.tagName() == mPropertyName )
        {
          const int fieldIndex = mLayer->fields().indexOf( propertyNameElement.firstChild().nodeValue() );
          if ( fieldIndex != -1 )
          {
            const QgsField field = mLayer->fields().field( propertyNameElement.firstChild().nodeValue() );
            field.convertCompatible( value );
            converted = true;
          }
        }
      }
      if ( !converted )
      {
        // try to convert the node content to number if possible,
        // otherwise let's use it as string
        bool ok;
        const double d = value.toDouble( &ok );
        if ( ok )
          value = d;
      }

      operand.reset( new QgsExpressionNodeLiteral( value ) );
      if ( !operand )
        continue;
    }

    // use the concat operator to merge the ogc:Literal children
    if ( !root )
    {
      root = std::move( operand );
    }
    else
    {
      root.reset( new QgsExpressionNodeBinaryOperator( QgsExpressionNodeBinaryOperator::boConcat, root.release(), operand.release() ) );
    }

    childNode = childNode.nextSibling();
  }

  if ( root )
    return root.release();

  return nullptr;
}

QgsExpressionNodeUnaryOperator *QgsOgcUtilsExpressionFromFilter::nodeNotFromOgcFilter( const QDomElement &element )
{
  if ( element.tagName() != QLatin1String( "Not" ) )
    return nullptr;

  const QDomElement operandElem = element.firstChildElement();
  std::unique_ptr<QgsExpressionNode> operand( nodeFromOgcFilter( operandElem ) );
  if ( !operand )
  {
    mErrorMessage = QObject::tr( "invalid operand for '%1' unary operator" ).arg( element.tagName() );
    return nullptr;
  }

  return new QgsExpressionNodeUnaryOperator( QgsExpressionNodeUnaryOperator::uoNot, operand.release() );
}

QgsExpressionNodeBinaryOperator *QgsOgcUtilsExpressionFromFilter::nodePropertyIsNullFromOgcFilter( const QDomElement &element )
{
  // convert ogc:PropertyIsNull to IS operator with NULL right operand
  if ( element.tagName() != QLatin1String( "PropertyIsNull" ) )
  {
    return nullptr;
  }

  const QDomElement operandElem = element.firstChildElement();
  std::unique_ptr<QgsExpressionNode> opLeft( nodeFromOgcFilter( operandElem ) );
  if ( !opLeft )
    return nullptr;

  std::unique_ptr<QgsExpressionNode> opRight( new QgsExpressionNodeLiteral( QVariant() ) );
  return new QgsExpressionNodeBinaryOperator( QgsExpressionNodeBinaryOperator::boIs, opLeft.release(), opRight.release() );
}

QgsExpressionNodeFunction *QgsOgcUtilsExpressionFromFilter::nodeFunctionFromOgcFilter( const QDomElement &element )
{
  if ( element.isNull() || element.tagName() != QLatin1String( "Function" ) )
  {
    mErrorMessage = QObject::tr( "%1:Function expected, got %2" ).arg( mPrefix, element.tagName() );
    return nullptr;
  }

  for ( int i = 0; i < QgsExpression::Functions().size(); i++ )
  {
    const QgsExpressionFunction *funcDef = QgsExpression::Functions()[i];

    if ( element.attribute( QStringLiteral( "name" ) ) != funcDef->name() )
      continue;

    std::unique_ptr<QgsExpressionNode::NodeList> args( new QgsExpressionNode::NodeList() );

    QDomElement operandElem = element.firstChildElement();
    while ( !operandElem.isNull() )
    {
      std::unique_ptr<QgsExpressionNode> op( nodeFromOgcFilter( operandElem ) );
      if ( !op )
      {
        return nullptr;
      }
      args->append( op.release() );

      operandElem = operandElem.nextSiblingElement();
    }

    return new QgsExpressionNodeFunction( i, args.release() );
  }

  return nullptr;
}

QgsExpressionNode *QgsOgcUtilsExpressionFromFilter::nodeIsBetweenFromOgcFilter( const QDomElement &element )
{
  // <ogc:PropertyIsBetween> encode a Range check
  std::unique_ptr<QgsExpressionNode> operand;
  std::unique_ptr<QgsExpressionNode> lowerBound;
  std::unique_ptr<QgsExpressionNode> upperBound;

  QDomElement operandElem = element.firstChildElement();
  while ( !operandElem.isNull() )
  {
    if ( operandElem.tagName() == QLatin1String( "LowerBoundary" ) )
    {
      const QDomElement lowerBoundElem = operandElem.firstChildElement();
      lowerBound.reset( nodeFromOgcFilter( lowerBoundElem ) );
    }
    else if ( operandElem.tagName() == QLatin1String( "UpperBoundary" ) )
    {
      const QDomElement upperBoundElem = operandElem.firstChildElement();
      upperBound.reset( nodeFromOgcFilter( upperBoundElem ) );
    }
    else
    {
      // <ogc:expression>
      operand.reset( nodeFromOgcFilter( operandElem ) );
    }

    if ( operand && lowerBound && upperBound )
      break;

    operandElem = operandElem.nextSiblingElement();
  }

  if ( !operand || !lowerBound || !upperBound )
  {
    mErrorMessage = QObject::tr( "missing some required sub-elements in %1:PropertyIsBetween" ).arg( mPrefix );
    return nullptr;
  }

  std::unique_ptr<QgsExpressionNode> leOperator( new QgsExpressionNodeBinaryOperator( QgsExpressionNodeBinaryOperator::boLE, operand->clone(), upperBound.release() ) );
  std::unique_ptr<QgsExpressionNode> geOperator( new QgsExpressionNodeBinaryOperator( QgsExpressionNodeBinaryOperator::boGE, operand.release(), lowerBound.release() ) );
  return new QgsExpressionNodeBinaryOperator( QgsExpressionNodeBinaryOperator::boAnd, geOperator.release(), leOperator.release() );
}

QString QgsOgcUtilsExpressionFromFilter::errorMessage() const
{
  return mErrorMessage;
}
