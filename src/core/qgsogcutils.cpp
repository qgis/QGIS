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

#include <memory>
#include <ogr_api.h>

#include "qgscoordinatereferencesystem.h"
#include "qgsexpression.h"
#include "qgsexpression_p.h"
#include "qgsexpressioncontextutils.h"
#include "qgsexpressionfunction.h"
#include "qgsexpressionnodeimpl.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmultipolygon.h"
#include "qgsogrutils.h"
#include "qgspolygon.h"
#include "qgsrectangle.h"
#include "qgsstringutils.h"
#include "qgsvectorlayer.h"
#include "qgswkbptr.h"

#include <QColor>
#include <QObject>
#include <QRegularExpression>
#include <QStringList>
#include <QTextStream>

#ifndef Q_OS_WIN
#include <netinet/in.h>
#else
#include <winsock.h>
#endif


#define GML_NAMESPACE u"http://www.opengis.net/gml"_s
#define GML32_NAMESPACE u"http://www.opengis.net/gml/3.2"_s
#define OGC_NAMESPACE u"http://www.opengis.net/ogc"_s
#define FES_NAMESPACE u"http://www.opengis.net/fes/2.0"_s
#define SE_NAMESPACE u"http://www.opengis.net/se"_s

QgsOgcUtilsExprToFilter::QgsOgcUtilsExprToFilter( QDomDocument &doc,
    QgsOgcUtils::GMLVersion gmlVersion,
    QgsOgcUtils::FilterVersion filterVersion,
    const QString &namespacePrefix,
    const QString &namespaceURI,
    const QString &geometryName,
    const QString &srsName,
    bool honourAxisOrientation,
    bool invertAxisOrientation,
    const QMap<QString, QString> &fieldNameToXPathMap,
    const QMap<QString, QString> &namespacePrefixToUriMap )
  : mDoc( doc )
  , mGMLVersion( gmlVersion )
  , mFilterVersion( filterVersion )
  , mNamespacePrefix( namespacePrefix )
  , mNamespaceURI( namespaceURI )
  , mGeometryName( geometryName )
  , mSrsName( srsName )
  , mInvertAxisOrientation( invertAxisOrientation )
  , mFieldNameToXPathMap( fieldNameToXPathMap )
  , mNamespacePrefixToUriMap( namespacePrefixToUriMap )
  , mFilterPrefix( ( filterVersion == QgsOgcUtils::FILTER_FES_2_0 ) ? "fes" : "ogc" )
  , mPropertyName( ( filterVersion == QgsOgcUtils::FILTER_FES_2_0 ) ? "ValueReference" : "PropertyName" )
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

  if ( !( geomType == "Point"_L1 || geomType == "LineString"_L1 || geomType == "Polygon"_L1 ||
          geomType == "MultiPoint"_L1 || geomType == "MultiLineString"_L1 || geomType == "MultiPolygon"_L1 ||
          geomType == "Box"_L1 || geomType == "Envelope"_L1 || geomType == "MultiCurve"_L1 ) )
  {
    const QDomNode geometryChild = geometryNode.firstChild();
    if ( geometryChild.isNull() )
    {
      return geometry;
    }
    geometryTypeElement = geometryChild.toElement();
    geomType = geometryTypeElement.tagName();
  }

  if ( !( geomType == "Point"_L1 || geomType == "LineString"_L1 || geomType == "Polygon"_L1 ||
          geomType == "MultiPoint"_L1 || geomType == "MultiLineString"_L1 || geomType == "MultiPolygon"_L1 ||
          geomType == "Box"_L1 || geomType == "Envelope"_L1  || geomType == "MultiCurve"_L1 ) )
    return QgsGeometry();

  if ( geomType == "Point"_L1 )
  {
    geometry = geometryFromGMLPoint( geometryTypeElement );
  }
  else if ( geomType == "LineString"_L1 )
  {
    geometry = geometryFromGMLLineString( geometryTypeElement );
  }
  else if ( geomType == "Polygon"_L1 )
  {
    geometry = geometryFromGMLPolygon( geometryTypeElement );
  }
  else if ( geomType == "MultiPoint"_L1 )
  {
    geometry = geometryFromGMLMultiPoint( geometryTypeElement );
  }
  else if ( geomType == "MultiLineString"_L1 )
  {
    geometry = geometryFromGMLMultiLineString( geometryTypeElement );
  }
  else if ( geomType == "MultiCurve"_L1 )
  {
    geometry = geometryFromGMLMultiCurve( geometryTypeElement );
  }
  else if ( geomType == "MultiPolygon"_L1 )
  {
    geometry = geometryFromGMLMultiPolygon( geometryTypeElement );
  }
  else if ( geomType == "Box"_L1 )
  {
    geometry = QgsGeometry::fromRect( rectangleFromGMLBox( geometryTypeElement ) );
  }
  else if ( geomType == "Envelope"_L1 )
  {
    geometry = QgsGeometry::fromRect( rectangleFromGMLEnvelope( geometryTypeElement ) );
  }
  else //unknown type
  {
    QgsDebugMsgLevel( u"Unknown geometry type %1"_s.arg( geomType ), 2 );
    return geometry;
  }

  // Handle srsName
  // Check if the XY coordinates of geometry need to be swapped by checking the srs from the GML
  QgsCoordinateReferenceSystem geomSrs;
  if ( geometryTypeElement.hasAttribute( u"srsName"_s ) )
  {
    QString srsName { geometryTypeElement.attribute( u"srsName"_s ) };

    // The logic here follows WFS GeoServer conventions from https://docs.geoserver.org/latest/en/user/services/wfs/axis_order.html
    const bool ignoreAxisOrientation { srsName.startsWith( "http://www.opengis.net/gml/srs/"_L1 ) || srsName.startsWith( "EPSG:"_L1 ) };

    // GDAL does not recognise http://www.opengis.net/gml/srs/epsg.xml#4326 but it does
    // http://www.opengis.net/def/crs/EPSG/0/4326 so, let's try that
    if ( srsName.startsWith( "http://www.opengis.net/gml/srs/"_L1 ) )
    {
      const auto parts { srsName.split( QRegularExpression( QStringLiteral( R"raw(/|#|\.)raw" ) ) ) };
      if ( parts.length() == 10 )
      {
        srsName = u"http://www.opengis.net/def/crs/%1/0/%2"_s.arg( parts[ 7 ].toUpper(), parts[ 9 ] );
      }
    }
    geomSrs.createFromUserInput( srsName );
    if ( geomSrs.isValid() && geomSrs.hasAxisInverted() && !ignoreAxisOrientation )
    {
      geometry.get()->swapXy();
    }
  }

  // Apply a coordinate transformation if context has information about the layer and the transformation context
  if ( geomSrs.isValid() && context.layer && geomSrs != context.layer->crs() )
  {
    const QgsCoordinateTransform transformer { geomSrs, context.layer->crs(), context.transformContext };
    try
    {
      const Qgis::GeometryOperationResult result = geometry.transform( transformer );
      if ( result != Qgis::GeometryOperationResult::Success )
      {
        QgsDebugMsgLevel( u"Error transforming geometry: %1"_s.arg( qgsEnumValueToKey( result ) ), 2 );
      }
    }
    catch ( QgsCsException & )
    {
      QgsDebugMsgLevel( u"CS error transforming geometry"_s, 2 );
    }
  }

  return geometry;
}

QgsGeometry QgsOgcUtils::geometryFromGML( const QString &xmlString, const Context &context )
{
  // wrap the string into a root tag to have "gml" namespace (and also as a default namespace)
  const QString xml = u"<tmp xmlns=\"%1\" xmlns:gml=\"%1\">%2</tmp>"_s.arg( GML_NAMESPACE, xmlString );
  QDomDocument doc;
  if ( !doc.setContent( xml, true ) )
    return QgsGeometry();

  return geometryFromGML( doc.documentElement().firstChildElement(), context );
}


QgsGeometry QgsOgcUtils::geometryFromGMLPoint( const QDomElement &geometryElement )
{
  QgsPolyline pointCoordinate;

  const QDomNodeList coordList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, u"coordinates"_s );
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
    const QDomNodeList posList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, u"pos"_s );
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

  const bool hasZ { !std::isnan( pointCoordinate.first().z() ) };
  QgsPolyline::const_iterator point_it = pointCoordinate.constBegin();
  const char e = static_cast<char>( htonl( 1 ) != 1 );
  const double x = point_it->x();
  const double y = point_it->y();
  const int size = 1 + static_cast<int>( sizeof( int ) ) + ( hasZ ? 3 : 2 ) * static_cast<int>( sizeof( double ) );

  const Qgis::WkbType type { hasZ ? Qgis::WkbType::PointZ : Qgis::WkbType::Point };
  unsigned char *wkb = new unsigned char[size];

  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  memcpy( &( wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( wkb )[wkbPosition], &x, sizeof( double ) );
  wkbPosition += sizeof( double );
  memcpy( &( wkb )[wkbPosition], &y, sizeof( double ) );

  if ( hasZ )
  {
    wkbPosition += sizeof( double );
    double z = point_it->z();
    memcpy( &( wkb )[wkbPosition], &z, sizeof( double ) );
  }

  QgsGeometry g;
  g.fromWkb( wkb, size );
  return g;
}

QgsGeometry QgsOgcUtils::geometryFromGMLLineString( const QDomElement &geometryElement )
{
  QgsPolyline lineCoordinates;

  const QDomNodeList coordList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, u"coordinates"_s );
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
    const QDomNodeList posList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, u"posList"_s );
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

  const bool hasZ { !std::isnan( lineCoordinates.first().z() ) };

  char e = static_cast<char>( htonl( 1 ) != 1 );
  const int size = 1 + 2 * static_cast<int>( sizeof( int ) + lineCoordinates.size() ) * ( hasZ ? 3 : 2 ) * static_cast<int>( sizeof( double ) );

  const Qgis::WkbType type{ hasZ ? Qgis::WkbType::LineStringZ : Qgis::WkbType::LineString };
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

  QgsPolyline::const_iterator iter;
  for ( iter = lineCoordinates.constBegin(); iter != lineCoordinates.constEnd(); ++iter )
  {
    x = iter->x();
    y = iter->y();
    memcpy( &( wkb )[wkbPosition], &x, sizeof( double ) );
    wkbPosition += sizeof( double );
    memcpy( &( wkb )[wkbPosition], &y, sizeof( double ) );
    wkbPosition += sizeof( double );

    if ( hasZ )
    {
      double z = iter->z();
      memcpy( &( wkb )[wkbPosition], &z, sizeof( double ) );
      wkbPosition += sizeof( double );
    }

  }

  QgsGeometry g;
  g.fromWkb( wkb, size );
  return g;
}

QgsGeometry QgsOgcUtils::geometryFromGMLPolygon( const QDomElement &geometryElement )
{
  //read all the coordinates (as QgsPoint) into memory. Each linear ring has an entry in the vector
  QgsMultiPolyline ringCoordinates;

  //read coordinates for outer boundary
  QgsPolyline exteriorPointList;
  const QDomNodeList outerBoundaryList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, u"outerBoundaryIs"_s );
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
    const QDomNodeList innerBoundaryList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, u"innerBoundaryIs"_s );
    for ( int i = 0; i < innerBoundaryList.size(); ++i )
    {
      QgsPolyline interiorPointList;
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
    const QDomNodeList exteriorList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, u"exterior"_s );
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
    const QDomNodeList interiorList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, u"interior"_s );
    for ( int i = 0; i < interiorList.size(); ++i )
    {
      QgsPolyline interiorPointList;
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
  for ( QgsMultiPolyline::const_iterator it = ringCoordinates.constBegin(); it != ringCoordinates.constEnd(); ++it )
  {
    npoints += it->size();
  }

  const bool hasZ { !std::isnan( ringCoordinates.first().first().z() ) };

  const int size = 1 + 2 * static_cast<int>( sizeof( int ) ) + nrings * static_cast<int>( sizeof( int ) ) + ( hasZ ? 3 : 2 ) * npoints * static_cast<int>( sizeof( double ) );

  const Qgis::WkbType type { hasZ ? Qgis::WkbType::PolygonZ : Qgis::WkbType::Polygon };
  unsigned char *wkb = new unsigned char[size];

  //char e = QgsApplication::endian();
  char e = static_cast<char>( htonl( 1 ) != 1 );
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  int nPointsInRing = 0;
  double x, y, z;

  //fill the contents into *wkb
  memcpy( &( wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( wkb )[wkbPosition], &nrings, sizeof( int ) );
  wkbPosition += sizeof( int );
  for ( QgsMultiPolyline::const_iterator it = ringCoordinates.constBegin(); it != ringCoordinates.constEnd(); ++it )
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

      if ( hasZ )
      {
        z = iter->z();
        memcpy( &( wkb )[wkbPosition], &z, sizeof( double ) );
        wkbPosition += sizeof( double );
      }
    }
  }

  QgsGeometry g;
  g.fromWkb( wkb, size );
  return g;
}

QgsGeometry QgsOgcUtils::geometryFromGMLMultiPoint( const QDomElement &geometryElement )
{
  QgsPolyline pointList;
  QgsPolyline currentPoint;
  const QDomNodeList pointMemberList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, u"pointMember"_s );
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
    pointNodeList = pointMemberList.at( i ).toElement().elementsByTagNameNS( GML_NAMESPACE, u"Point"_s );
    if ( pointNodeList.size() < 1 )
    {
      continue;
    }
    //<coordinates> element
    coordinatesList = pointNodeList.at( 0 ).toElement().elementsByTagNameNS( GML_NAMESPACE, u"coordinates"_s );
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
      posList = pointNodeList.at( 0 ).toElement().elementsByTagNameNS( GML_NAMESPACE, u"pos"_s );
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

  const bool hasZ { !std::isnan( pointList.first().z() ) };

  //calculate the required wkb size
  const int size = 1 + 2 * static_cast<int>( sizeof( int ) ) + static_cast<int>( pointList.size() ) * ( ( hasZ ? 3 : 2 ) * static_cast<int>( sizeof( double ) ) + 1 + static_cast<int>( sizeof( int ) ) );

  const Qgis::WkbType type { hasZ ? Qgis::WkbType::MultiPointZ :  Qgis::WkbType::MultiPoint };
  unsigned char *wkb = new unsigned char[size];

  //fill the wkb content
  char e = static_cast<char>( htonl( 1 ) != 1 );
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  double x, y, z;
  memcpy( &( wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( wkb )[wkbPosition], &nPoints, sizeof( int ) );
  wkbPosition += sizeof( int );
  const Qgis::WkbType pointType { hasZ ? Qgis::WkbType::PointZ : Qgis::WkbType::Point };
  for ( QgsPolyline::const_iterator it = pointList.constBegin(); it != pointList.constEnd(); ++it )
  {
    memcpy( &( wkb )[wkbPosition], &e, 1 );
    wkbPosition += 1;
    memcpy( &( wkb )[wkbPosition], &pointType, sizeof( int ) );
    wkbPosition += sizeof( int );
    x = it->x();
    memcpy( &( wkb )[wkbPosition], &x, sizeof( double ) );
    wkbPosition += sizeof( double );
    y = it->y();
    memcpy( &( wkb )[wkbPosition], &y, sizeof( double ) );
    wkbPosition += sizeof( double );

    if ( hasZ )
    {
      z = it->z();
      memcpy( &( wkb )[wkbPosition], &z, sizeof( double ) );
      wkbPosition += sizeof( double );
    }
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


  QList< QgsPolyline > lineCoordinates; //first list: lines, second list: points of one line
  QDomElement currentLineStringElement;
  QDomNodeList currentCoordList;
  QDomNodeList currentPosList;

  const QDomNodeList lineStringMemberList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, u"lineStringMember"_s );
  if ( !lineStringMemberList.isEmpty() ) //geoserver
  {
    for ( int i = 0; i < lineStringMemberList.size(); ++i )
    {
      const QDomNodeList lineStringNodeList = lineStringMemberList.at( i ).toElement().elementsByTagNameNS( GML_NAMESPACE, u"LineString"_s );
      if ( lineStringNodeList.size() < 1 )
      {
        return QgsGeometry();
      }
      currentLineStringElement = lineStringNodeList.at( 0 ).toElement();
      currentCoordList = currentLineStringElement.elementsByTagNameNS( GML_NAMESPACE, u"coordinates"_s );
      if ( !currentCoordList.isEmpty() )
      {
        QgsPolyline currentPointList;
        if ( readGMLCoordinates( currentPointList, currentCoordList.at( 0 ).toElement() ) != 0 )
        {
          return QgsGeometry();
        }
        lineCoordinates.push_back( currentPointList );
      }
      else
      {
        currentPosList = currentLineStringElement.elementsByTagNameNS( GML_NAMESPACE, u"posList"_s );
        if ( currentPosList.size() < 1 )
        {
          return QgsGeometry();
        }
        QgsPolyline currentPointList;
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
    const QDomNodeList lineStringList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, u"LineString"_s );
    if ( !lineStringList.isEmpty() ) //mapserver
    {
      for ( int i = 0; i < lineStringList.size(); ++i )
      {
        currentLineStringElement = lineStringList.at( i ).toElement();
        currentCoordList = currentLineStringElement.elementsByTagNameNS( GML_NAMESPACE, u"coordinates"_s );
        if ( !currentCoordList.isEmpty() )
        {
          QgsPolyline currentPointList;
          if ( readGMLCoordinates( currentPointList, currentCoordList.at( 0 ).toElement() ) != 0 )
          {
            return QgsGeometry();
          }
          lineCoordinates.push_back( currentPointList );
          return QgsGeometry();
        }
        else
        {
          currentPosList = currentLineStringElement.elementsByTagNameNS( GML_NAMESPACE, u"posList"_s );
          if ( currentPosList.size() < 1 )
          {
            return QgsGeometry();
          }
          QgsPolyline currentPointList;
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

  const bool hasZ { !std::isnan( lineCoordinates.first().first().z() ) };
  const int coordSize { hasZ ? 3 : 2 };

  //calculate the required wkb size
  int size = static_cast<int>( lineCoordinates.size() + 1 ) * ( 1 + 2 * sizeof( int ) );
  for ( QList< QgsPolyline >::const_iterator it = lineCoordinates.constBegin(); it != lineCoordinates.constEnd(); ++it )
  {
    size += it->size() * coordSize * sizeof( double );
  }

  const Qgis::WkbType type { hasZ ? Qgis::WkbType::MultiLineStringZ : Qgis::WkbType::MultiLineString };
  unsigned char *wkb = new unsigned char[size];

  //fill the wkb content
  char e = static_cast<char>( htonl( 1 ) != 1 );
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  int nPoints; //number of points in a line
  double x, y, z;
  memcpy( &( wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( wkb )[wkbPosition], &nLines, sizeof( int ) );
  wkbPosition += sizeof( int );
  const Qgis::WkbType lineType { hasZ ? Qgis::WkbType::LineStringZ : Qgis::WkbType::LineString };
  for ( QList< QgsPolyline >::const_iterator it = lineCoordinates.constBegin(); it != lineCoordinates.constEnd(); ++it )
  {
    memcpy( &( wkb )[wkbPosition], &e, 1 );
    wkbPosition += 1;
    memcpy( &( wkb )[wkbPosition], &lineType, sizeof( int ) );
    wkbPosition += sizeof( int );
    nPoints = it->size();
    memcpy( &( wkb )[wkbPosition], &nPoints, sizeof( int ) );
    wkbPosition += sizeof( int );
    for ( QgsPolyline::const_iterator iter = it->begin(); iter != it->end(); ++iter )
    {
      x = iter->x();
      y = iter->y();
      // QgsDebugMsgLevel( u"x, y is %1,%2"_s.arg( x, 'f' ).arg( y, 'f' ), 2 );
      memcpy( &( wkb )[wkbPosition], &x, sizeof( double ) );
      wkbPosition += sizeof( double );
      memcpy( &( wkb )[wkbPosition], &y, sizeof( double ) );
      wkbPosition += sizeof( double );

      if ( hasZ )
      {
        z = iter->z();
        memcpy( &( wkb )[wkbPosition], &z, sizeof( double ) );
        wkbPosition += sizeof( double );
      }
    }
  }

  QgsGeometry g;
  g.fromWkb( wkb, size );
  return g;
}

QgsGeometry QgsOgcUtils::geometryFromGMLMultiPolygon( const QDomElement &geometryElement )
{
  //first list: different polygons, second list: different rings, third list: different points
  QVector<QgsMultiPolyline> multiPolygonPoints;
  QDomElement currentPolygonMemberElement;
  QDomNodeList polygonList;
  QDomElement currentPolygonElement;
  // rings in GML2
  QDomNodeList outerBoundaryList;
  QDomElement currentOuterBoundaryElement;
  QDomElement currentInnerBoundaryElement;
  // rings in GML3
  QDomNodeList exteriorList;
  QDomElement currentExteriorElement;
  QDomElement currentInteriorElement;
  // linear ring
  QDomNodeList linearRingNodeList;
  QDomElement currentLinearRingElement;
  // Coordinates or position list
  QDomNodeList currentCoordinateList;
  QDomNodeList currentPosList;

  const QDomNodeList polygonMemberList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, u"polygonMember"_s );
  QgsMultiPolyline currentPolygonList;
  for ( int i = 0; i < polygonMemberList.size(); ++i )
  {
    currentPolygonList.resize( 0 ); // preserve capacity - don't use clear
    currentPolygonMemberElement = polygonMemberList.at( i ).toElement();
    polygonList = currentPolygonMemberElement.elementsByTagNameNS( GML_NAMESPACE, u"Polygon"_s );
    if ( polygonList.size() < 1 )
    {
      continue;
    }
    currentPolygonElement = polygonList.at( 0 ).toElement();

    //find exterior ring
    outerBoundaryList = currentPolygonElement.elementsByTagNameNS( GML_NAMESPACE, u"outerBoundaryIs"_s );
    if ( !outerBoundaryList.isEmpty() )
    {
      currentOuterBoundaryElement = outerBoundaryList.at( 0 ).toElement();
      QgsPolyline ringCoordinates;

      linearRingNodeList = currentOuterBoundaryElement.elementsByTagNameNS( GML_NAMESPACE, u"LinearRing"_s );
      if ( linearRingNodeList.size() < 1 )
      {
        continue;
      }
      currentLinearRingElement = linearRingNodeList.at( 0 ).toElement();
      currentCoordinateList = currentLinearRingElement.elementsByTagNameNS( GML_NAMESPACE, u"coordinates"_s );
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
      const QDomNodeList innerBoundaryList = currentPolygonElement.elementsByTagNameNS( GML_NAMESPACE, u"innerBoundaryIs"_s );
      for ( int j = 0; j < innerBoundaryList.size(); ++j )
      {
        QgsPolyline ringCoordinates;
        currentInnerBoundaryElement = innerBoundaryList.at( j ).toElement();
        linearRingNodeList = currentInnerBoundaryElement.elementsByTagNameNS( GML_NAMESPACE, u"LinearRing"_s );
        if ( linearRingNodeList.size() < 1 )
        {
          continue;
        }
        currentLinearRingElement = linearRingNodeList.at( 0 ).toElement();
        currentCoordinateList = currentLinearRingElement.elementsByTagNameNS( GML_NAMESPACE, u"coordinates"_s );
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
      exteriorList = currentPolygonElement.elementsByTagNameNS( GML_NAMESPACE, u"exterior"_s );
      if ( exteriorList.size() < 1 )
      {
        continue;
      }

      currentExteriorElement = exteriorList.at( 0 ).toElement();
      QgsPolyline ringPositions;

      linearRingNodeList = currentExteriorElement.elementsByTagNameNS( GML_NAMESPACE, u"LinearRing"_s );
      if ( linearRingNodeList.size() < 1 )
      {
        continue;
      }
      currentLinearRingElement = linearRingNodeList.at( 0 ).toElement();
      currentPosList = currentLinearRingElement.elementsByTagNameNS( GML_NAMESPACE, u"posList"_s );
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
      const QDomNodeList interiorList = currentPolygonElement.elementsByTagNameNS( GML_NAMESPACE, u"interior"_s );
      for ( int j = 0; j < interiorList.size(); ++j )
      {
        QgsPolyline ringPositions;
        currentInteriorElement = interiorList.at( j ).toElement();
        linearRingNodeList = currentInteriorElement.elementsByTagNameNS( GML_NAMESPACE, u"LinearRing"_s );
        if ( linearRingNodeList.size() < 1 )
        {
          continue;
        }
        currentLinearRingElement = linearRingNodeList.at( 0 ).toElement();
        currentPosList = currentLinearRingElement.elementsByTagNameNS( GML_NAMESPACE, u"posList"_s );
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

  const bool hasZ { !std::isnan( multiPolygonPoints.first().first().first().z() ) };

  int size = 1 + 2 * sizeof( int );
  //calculate the wkb size

  for ( auto it = multiPolygonPoints.constBegin(); it != multiPolygonPoints.constEnd(); ++it )
  {
    size += 1 + 2 * sizeof( int );
    for ( auto iter = it->begin(); iter != it->end(); ++iter )
    {
      size += static_cast<int>( sizeof( int ) ) + ( hasZ ? 3 : 2 ) * static_cast<int>( iter->size() * sizeof( double ) );
    }
  }

  Qgis::WkbType type = hasZ ? Qgis::WkbType::MultiPolygonZ : Qgis::WkbType::MultiPolygon;
  unsigned char *wkb = new unsigned char[size];

  char e = static_cast<char>( htonl( 1 ) != 1 );
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

  type = hasZ ? Qgis::WkbType::PolygonZ : Qgis::WkbType::Polygon;

  for ( auto it = multiPolygonPoints.constBegin(); it != multiPolygonPoints.constEnd(); ++it )
  {
    memcpy( &( wkb )[wkbPosition], &e, 1 );
    wkbPosition += 1;
    memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
    wkbPosition += sizeof( int );
    nRings = it->size();
    memcpy( &( wkb )[wkbPosition], &nRings, sizeof( int ) );
    wkbPosition += sizeof( int );
    for ( auto iter = it->begin(); iter != it->end(); ++iter )
    {
      nPointsInRing = iter->size();
      memcpy( &( wkb )[wkbPosition], &nPointsInRing, sizeof( int ) );
      wkbPosition += sizeof( int );
      for ( auto iterator = iter->begin(); iterator != iter->end(); ++iterator )
      {
        x = iterator->x();
        y = iterator->y();
        memcpy( &( wkb )[wkbPosition], &x, sizeof( double ) );
        wkbPosition += sizeof( double );
        memcpy( &( wkb )[wkbPosition], &y, sizeof( double ) );
        wkbPosition += sizeof( double );
        if ( hasZ )
        {
          double z = iterator->z();
          memcpy( &( wkb )[wkbPosition], &z, sizeof( double ) );
          wkbPosition += sizeof( double );
        }
      }
    }
  }

  QgsGeometry g;
  g.fromWkb( wkb, size );
  return g;
}

QDomElement QgsOgcUtils::filterElement( QDomDocument &doc, GMLVersion gmlVersion, FilterVersion filterVersion, bool GMLUsed )
{
  QDomElement filterElem =
    ( filterVersion == FILTER_FES_2_0 ) ?
    doc.createElementNS( FES_NAMESPACE, u"fes:Filter"_s ) :
    doc.createElementNS( OGC_NAMESPACE, u"ogc:Filter"_s );

  if ( GMLUsed )
  {
    QDomAttr attr = doc.createAttribute( u"xmlns:gml"_s );
    if ( gmlVersion == GML_3_2_1 )
      attr.setValue( GML32_NAMESPACE );
    else
      attr.setValue( GML_NAMESPACE );
    filterElem.setAttributeNode( attr );
  }
  return filterElem;
}


bool QgsOgcUtils::readGMLCoordinates( QgsPolyline &coords, const QDomElement &elem )
{
  QString coordSeparator = u","_s;
  QString tupleSeparator = u" "_s;
  //"decimal" has to be "."

  coords.clear();

  if ( elem.hasAttribute( u"cs"_s ) )
  {
    coordSeparator = elem.attribute( u"cs"_s );
  }
  if ( elem.hasAttribute( u"ts"_s ) )
  {
    tupleSeparator = elem.attribute( u"ts"_s );
  }

  const QStringList tupels = elem.text().split( tupleSeparator, Qt::SkipEmptyParts );
  QStringList tuple_coords;
  double x, y, z;
  bool conversionSuccess;

  QStringList::const_iterator it;
  for ( it = tupels.constBegin(); it != tupels.constEnd(); ++it )
  {
    tuple_coords = ( *it ).split( coordSeparator, Qt::SkipEmptyParts );
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
    if ( tuple_coords.size() > 2 )
    {
      z = tuple_coords.at( 2 ).toDouble( &conversionSuccess );
      if ( !conversionSuccess )
      {
        return true;
      }
    }
    else
    {
      z = std::numeric_limits<double>::quiet_NaN();
    }
    coords.append( QgsPoint( x, y, z ) );
  }
  return false;
}

QgsRectangle QgsOgcUtils::rectangleFromGMLBox( const QDomNode &boxNode )
{
  QgsRectangle rect;

  const QDomElement boxElem = boxNode.toElement();
  if ( boxElem.tagName() != "Box"_L1 )
    return rect;

  const QDomElement bElem = boxElem.firstChild().toElement();
  QString coordSeparator = u","_s;
  QString tupleSeparator = u" "_s;
  if ( bElem.hasAttribute( u"cs"_s ) )
  {
    coordSeparator = bElem.attribute( u"cs"_s );
  }
  if ( bElem.hasAttribute( u"ts"_s ) )
  {
    tupleSeparator = bElem.attribute( u"ts"_s );
  }

  const QString bString = bElem.text();
  bool ok1, ok2, ok3, ok4;
  const double xmin = bString.section( tupleSeparator, 0, 0 ).section( coordSeparator, 0, 0 ).toDouble( &ok1 );
  const double ymin = bString.section( tupleSeparator, 0, 0 ).section( coordSeparator, 1, 1 ).toDouble( &ok2 );
  const double xmax = bString.section( tupleSeparator, 1, 1 ).section( coordSeparator, 0, 0 ).toDouble( &ok3 );
  const double ymax = bString.section( tupleSeparator, 1, 1 ).section( coordSeparator, 1, 1 ).toDouble( &ok4 );

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

  const QStringList pos = elem.text().split( ' ', Qt::SkipEmptyParts );
  double x, y, z;
  bool conversionSuccess;
  const int posSize = pos.size();

  int srsDimension = 2;
  if ( elem.hasAttribute( u"srsDimension"_s ) )
  {
    srsDimension = elem.attribute( u"srsDimension"_s ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      srsDimension = 2;
    }
  }
  else if ( elem.hasAttribute( u"dimension"_s ) )
  {
    srsDimension = elem.attribute( u"dimension"_s ).toInt( &conversionSuccess );
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
    if ( srsDimension > 2 )
    {
      z = pos.at( i * srsDimension + 2 ).toDouble( &conversionSuccess );
      if ( !conversionSuccess )
      {
        return true;
      }
    }
    else
    {
      z = std::numeric_limits<double>::quiet_NaN();
    }
    coords.append( QgsPoint( x, y, z ) );
  }
  return false;
}


QgsRectangle QgsOgcUtils::rectangleFromGMLEnvelope( const QDomNode &envelopeNode )
{
  QgsRectangle rect;

  const QDomElement envelopeElem = envelopeNode.toElement();
  if ( envelopeElem.tagName() != "Envelope"_L1 )
    return rect;

  const QDomNodeList lowerCornerList = envelopeElem.elementsByTagNameNS( GML_NAMESPACE, u"lowerCorner"_s );
  if ( lowerCornerList.size() < 1 )
    return rect;

  const QDomNodeList upperCornerList = envelopeElem.elementsByTagNameNS( GML_NAMESPACE, u"upperCorner"_s );
  if ( upperCornerList.size() < 1 )
    return rect;

  bool conversionSuccess;
  int srsDimension = 2;

  QDomElement elem = lowerCornerList.at( 0 ).toElement();
  if ( elem.hasAttribute( u"srsDimension"_s ) )
  {
    srsDimension = elem.attribute( u"srsDimension"_s ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      srsDimension = 2;
    }
  }
  else if ( elem.hasAttribute( u"dimension"_s ) )
  {
    srsDimension = elem.attribute( u"dimension"_s ).toInt( &conversionSuccess );
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
  if ( elem.hasAttribute( u"srsDimension"_s ) )
  {
    srsDimension = elem.attribute( u"srsDimension"_s ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      srsDimension = 2;
    }
  }
  else if ( elem.hasAttribute( u"dimension"_s ) )
  {
    srsDimension = elem.attribute( u"dimension"_s ).toInt( &conversionSuccess );
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

QDomElement QgsOgcUtils::rectangleToGMLBox( const QgsRectangle *box, QDomDocument &doc, int precision )
{
  return rectangleToGMLBox( box, doc, QString(), false, precision );
}

QDomElement QgsOgcUtils::rectangleToGMLBox( const QgsRectangle *box, QDomDocument &doc,
    const QString &srsName,
    bool invertAxisOrientation,
    int precision )
{
  if ( !box )
  {
    return QDomElement();
  }

  QDomElement boxElem = doc.createElement( u"gml:Box"_s );
  if ( !srsName.isEmpty() )
  {
    boxElem.setAttribute( u"srsName"_s, srsName );
  }
  QDomElement coordElem = doc.createElement( u"gml:coordinates"_s );
  coordElem.setAttribute( u"cs"_s, u","_s );
  coordElem.setAttribute( u"ts"_s, u" "_s );

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

QDomElement QgsOgcUtils::rectangleToGMLEnvelope( const QgsRectangle *env, QDomDocument &doc, int precision )
{
  return rectangleToGMLEnvelope( env, doc, QString(), false, precision );
}

QDomElement QgsOgcUtils::rectangleToGMLEnvelope( const QgsRectangle *env, QDomDocument &doc,
    const QString &srsName,
    bool invertAxisOrientation,
    int precision )
{
  if ( !env )
  {
    return QDomElement();
  }

  QDomElement envElem = doc.createElement( u"gml:Envelope"_s );
  if ( !srsName.isEmpty() )
  {
    envElem.setAttribute( u"srsName"_s, srsName );
  }
  QString posList;

  QDomElement lowerCornerElem = doc.createElement( u"gml:lowerCorner"_s );
  posList = qgsDoubleToString( invertAxisOrientation ? env->yMinimum() : env->xMinimum(), precision );
  posList += ' ';
  posList += qgsDoubleToString( invertAxisOrientation ? env->xMinimum() : env->yMinimum(), precision );
  const QDomText lowerCornerText = doc.createTextNode( posList );
  lowerCornerElem.appendChild( lowerCornerText );
  envElem.appendChild( lowerCornerElem );

  QDomElement upperCornerElem = doc.createElement( u"gml:upperCorner"_s );
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
  return geometryToGML( geometry, doc, ( format == "GML2"_L1 ) ? GML_2_1_2 : GML_3_2_1, QString(), false, QString(), precision );
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
  QString cs = u","_s;
  // tuple separator
  const QString ts = u" "_s;
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
      case Qgis::WkbType::Point25D:
      case Qgis::WkbType::Point:
      case Qgis::WkbType::PointZ:
      case Qgis::WkbType::MultiPoint25D:
      case Qgis::WkbType::MultiPointZ:
      case Qgis::WkbType::MultiPoint:
        baseCoordElem = doc.createElement( u"gml:pos"_s );
        break;
      default:
        baseCoordElem = doc.createElement( u"gml:posList"_s );
        break;
    }
    cs = ' ';
  }
  else
  {
    baseCoordElem = doc.createElement( u"gml:coordinates"_s );
    baseCoordElem.setAttribute( u"cs"_s, cs );
    baseCoordElem.setAttribute( u"ts"_s, ts );
  }

  try
  {
    switch ( geometry.wkbType() )
    {
      case Qgis::WkbType::Point25D:
      case Qgis::WkbType::PointZ:
        hasZValue = true;
        //intentional fall-through
        [[fallthrough]];
      case Qgis::WkbType::Point:
      {
        QDomElement pointElem = doc.createElement( u"gml:Point"_s );
        if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
          pointElem.setAttribute( u"gml:id"_s, gmlIdBase );
        if ( !srsName.isEmpty() )
          pointElem.setAttribute( u"srsName"_s, srsName );
        QDomElement coordElem = baseCoordElem.cloneNode().toElement();

        double x, y;

        if ( invertAxisOrientation )
          wkbPtr >> y >> x;
        else
          wkbPtr >> x >> y;

        QString coordString = qgsDoubleToString( x, precision ) + cs + qgsDoubleToString( y, precision );

        // Add Z
        if ( hasZValue )
        {
          double z = 0;
          wkbPtr >> z;
          coordString += cs + qgsDoubleToString( z, precision );
        }
        const QDomText coordText = doc.createTextNode( coordString );

        coordElem.appendChild( coordText );
        if ( gmlVersion != GML_2_1_2 )
          coordElem.setAttribute( u"srsDimension"_s, hasZValue ? u"3"_s : u"2"_s );
        pointElem.appendChild( coordElem );
        return pointElem;
      }
      case Qgis::WkbType::MultiPoint25D:
      case Qgis::WkbType::MultiPointZ:
        hasZValue = true;
        //intentional fall-through
        [[fallthrough]];
      case Qgis::WkbType::MultiPoint:
      {
        QDomElement multiPointElem = doc.createElement( u"gml:MultiPoint"_s );
        if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
          multiPointElem.setAttribute( u"gml:id"_s, gmlIdBase );
        if ( !srsName.isEmpty() )
          multiPointElem.setAttribute( u"srsName"_s, srsName );

        int nPoints;
        wkbPtr >> nPoints;

        for ( int idx = 0; idx < nPoints; ++idx )
        {
          QDomElement pointMemberElem = doc.createElement( u"gml:pointMember"_s );
          QDomElement pointElem = doc.createElement( u"gml:Point"_s );
          if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
            pointElem.setAttribute( u"gml:id"_s, gmlIdBase + u".%1"_s.arg( idx + 1 ) );
          QDomElement coordElem = baseCoordElem.cloneNode().toElement();

          wkbPtr.readHeader();

          double x = 0;
          double y = 0;
          if ( invertAxisOrientation )
            wkbPtr >> y >> x;
          else
            wkbPtr >> x >> y;

          QString coordString = qgsDoubleToString( x, precision ) + cs + qgsDoubleToString( y, precision );
          // Add Z
          if ( hasZValue )
          {
            double z = 0;
            wkbPtr >> z;
            coordString += cs + qgsDoubleToString( z, precision );
          }

          const QDomText coordText = doc.createTextNode( coordString );

          coordElem.appendChild( coordText );
          if ( gmlVersion != GML_2_1_2 )
            coordElem.setAttribute( u"srsDimension"_s, hasZValue ? u"3"_s : u"2"_s );
          pointElem.appendChild( coordElem );


          pointMemberElem.appendChild( pointElem );
          multiPointElem.appendChild( pointMemberElem );
        }
        return multiPointElem;
      }
      case Qgis::WkbType::LineString25D:
      case Qgis::WkbType::LineStringZ:
        hasZValue = true;
        //intentional fall-through
        [[fallthrough]];
      case Qgis::WkbType::LineString:
      {
        QDomElement lineStringElem = doc.createElement( u"gml:LineString"_s );
        if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
          lineStringElem.setAttribute( u"gml:id"_s, gmlIdBase );
        if ( !srsName.isEmpty() )
          lineStringElem.setAttribute( u"srsName"_s, srsName );
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
            double z = 0;
            wkbPtr >> z;
            coordString += cs + qgsDoubleToString( z, precision );
          }

        }
        const QDomText coordText = doc.createTextNode( coordString );
        coordElem.appendChild( coordText );
        if ( gmlVersion != GML_2_1_2 )
          coordElem.setAttribute( u"srsDimension"_s, hasZValue ? u"3"_s : u"2"_s );
        lineStringElem.appendChild( coordElem );
        return lineStringElem;
      }
      case Qgis::WkbType::MultiLineString25D:
      case Qgis::WkbType::MultiLineStringZ:
        hasZValue = true;
        //intentional fall-through
        [[fallthrough]];
      case Qgis::WkbType::MultiLineString:
      {
        QDomElement multiLineStringElem = doc.createElement( u"gml:MultiLineString"_s );
        if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
          multiLineStringElem.setAttribute( u"gml:id"_s, gmlIdBase );
        if ( !srsName.isEmpty() )
          multiLineStringElem.setAttribute( u"srsName"_s, srsName );

        int nLines;
        wkbPtr >> nLines;

        for ( int jdx = 0; jdx < nLines; jdx++ )
        {
          QDomElement lineStringMemberElem = doc.createElement( u"gml:lineStringMember"_s );
          QDomElement lineStringElem = doc.createElement( u"gml:LineString"_s );
          if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
            lineStringElem.setAttribute( u"gml:id"_s, gmlIdBase + u".%1"_s.arg( jdx + 1 ) );

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
              double z = 0;
              wkbPtr >> z;
              coordString += cs + qgsDoubleToString( z, precision );
            }

          }
          const QDomText coordText = doc.createTextNode( coordString );
          coordElem.appendChild( coordText );
          if ( gmlVersion != GML_2_1_2 )
            coordElem.setAttribute( u"srsDimension"_s, hasZValue ? u"3"_s : u"2"_s );
          lineStringElem.appendChild( coordElem );
          lineStringMemberElem.appendChild( lineStringElem );
          multiLineStringElem.appendChild( lineStringMemberElem );
        }
        return multiLineStringElem;
      }
      case Qgis::WkbType::Polygon25D:
      case Qgis::WkbType::PolygonZ:
        hasZValue = true;
        //intentional fall-through
        [[fallthrough]];
      case Qgis::WkbType::Polygon:
      {
        QDomElement polygonElem = doc.createElement( u"gml:Polygon"_s );
        if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
          polygonElem.setAttribute( u"gml:id"_s, gmlIdBase );
        if ( !srsName.isEmpty() )
          polygonElem.setAttribute( u"srsName"_s, srsName );

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
          QDomElement ringElem = doc.createElement( u"gml:LinearRing"_s );
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
              double z = 0;
              wkbPtr >> z;
              coordString += cs + qgsDoubleToString( z, precision );
            }
          }
          const QDomText coordText = doc.createTextNode( coordString );
          coordElem.appendChild( coordText );
          if ( gmlVersion != GML_2_1_2 )
            coordElem.setAttribute( u"srsDimension"_s, hasZValue ? u"3"_s : u"2"_s );
          ringElem.appendChild( coordElem );
          boundaryElem.appendChild( ringElem );
          polygonElem.appendChild( boundaryElem );
        }

        return polygonElem;
      }
      case Qgis::WkbType::MultiPolygon25D:
      case Qgis::WkbType::MultiPolygonZ:
        hasZValue = true;
        //intentional fall-through
        [[fallthrough]];
      case Qgis::WkbType::MultiPolygon:
      {
        QDomElement multiPolygonElem = doc.createElement( u"gml:MultiPolygon"_s );
        if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
          multiPolygonElem.setAttribute( u"gml:id"_s, gmlIdBase );
        if ( !srsName.isEmpty() )
          multiPolygonElem.setAttribute( u"srsName"_s, srsName );

        int numPolygons;
        wkbPtr >> numPolygons;

        for ( int kdx = 0; kdx < numPolygons; kdx++ )
        {
          QDomElement polygonMemberElem = doc.createElement( u"gml:polygonMember"_s );
          QDomElement polygonElem = doc.createElement( u"gml:Polygon"_s );
          if ( gmlVersion == GML_3_2_1 && !gmlIdBase.isEmpty() )
            polygonElem.setAttribute( u"gml:id"_s, gmlIdBase + u".%1"_s.arg( kdx + 1 ) );

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
            QDomElement ringElem = doc.createElement( u"gml:LinearRing"_s );

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
                double z = 0;
                wkbPtr >> z;
                coordString += cs + qgsDoubleToString( z, precision );
              }

            }
            const QDomText coordText = doc.createTextNode( coordString );
            coordElem.appendChild( coordText );
            if ( gmlVersion != GML_2_1_2 )
              coordElem.setAttribute( u"srsDimension"_s, hasZValue ? u"3"_s : u"2"_s );
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
  return geometryToGML( geometry, doc, u"GML2"_s, precision );
}

QDomElement QgsOgcUtils::createGMLCoordinates( const QgsPolylineXY &points, QDomDocument &doc )
{
  QDomElement coordElem = doc.createElement( u"gml:coordinates"_s );
  coordElem.setAttribute( u"cs"_s, u","_s );
  coordElem.setAttribute( u"ts"_s, u" "_s );

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
  QDomElement posElem = doc.createElement( u"gml:pos"_s );
  if ( points.size() > 1 )
    posElem = doc.createElement( u"gml:posList"_s );
  posElem.setAttribute( u"srsDimension"_s, u"2"_s );

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
  QDomElement cssElem = fillElement.firstChildElement( u"CssParameter"_s );
  while ( !cssElem.isNull() )
  {
    cssName = cssElem.attribute( u"name"_s, u"not_found"_s );
    if ( cssName != "not_found"_L1 )
    {
      elemText = cssElem.text();
      if ( cssName == "fill"_L1 )
      {
        color.setNamedColor( elemText );
      }
      else if ( cssName == "fill-opacity"_L1 )
      {
        bool ok;
        const double opacity = elemText.toDouble( &ok );
        if ( ok )
        {
          color.setAlphaF( opacity );
        }
      }
    }

    cssElem = cssElem.nextSiblingElement( u"CssParameter"_s );
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
      expr->d->mRootNode.reset( node );
    }
    else
    {
      expr->d->mRootNode = std::make_unique<QgsExpressionNodeBinaryOperator>( QgsExpressionNodeBinaryOperator::boConcat, expr->d->mRootNode.release(), node );
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
  {  "Or"_L1, QgsExpressionNodeBinaryOperator::boOr },
  {  "And"_L1, QgsExpressionNodeBinaryOperator::boAnd },
  // comparison
  {  "PropertyIsEqualTo"_L1, QgsExpressionNodeBinaryOperator::boEQ },
  {  "PropertyIsNotEqualTo"_L1, QgsExpressionNodeBinaryOperator::boNE },
  {  "PropertyIsLessThanOrEqualTo"_L1, QgsExpressionNodeBinaryOperator::boLE },
  {  "PropertyIsGreaterThanOrEqualTo"_L1, QgsExpressionNodeBinaryOperator::boGE },
  {  "PropertyIsLessThan"_L1, QgsExpressionNodeBinaryOperator::boLT },
  {  "PropertyIsGreaterThan"_L1, QgsExpressionNodeBinaryOperator::boGT },
  {  "PropertyIsLike"_L1, QgsExpressionNodeBinaryOperator::boLike },
  // arithmetic
  {  "Add"_L1, QgsExpressionNodeBinaryOperator::boPlus },
  {  "Sub"_L1, QgsExpressionNodeBinaryOperator::boMinus },
  {  "Mul"_L1, QgsExpressionNodeBinaryOperator::boMul },
  {  "Div"_L1, QgsExpressionNodeBinaryOperator::boDiv },
} ) )

static int binaryOperatorFromTagName( const QString &tagName )
{

  return BINARY_OPERATORS_TAG_NAMES_MAP()->value( tagName, -1 );
}

static QString binaryOperatorToTagName( QgsExpressionNodeBinaryOperator::BinaryOperator op )
{
  if ( op == QgsExpressionNodeBinaryOperator::boILike )
  {
    return u"PropertyIsLike"_s;
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
    spatialOps << u"BBOX"_s << u"Intersects"_s << u"Contains"_s << u"Crosses"_s << u"Equals"_s
               << u"Disjoint"_s << u"Overlaps"_s << u"Touches"_s << u"Within"_s;
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
  return expressionToOgcFilter( exp, doc, GML_2_1_2, FILTER_OGC_1_0, QString(), QString(),
                                u"geometry"_s, QString(), false, false, errorMessage );
}

QDomElement QgsOgcUtils::expressionToOgcExpression( const QgsExpression &exp, QDomDocument &doc, QString *errorMessage, bool requiresFilterElement )
{
  return expressionToOgcExpression( exp, doc, GML_2_1_2, FILTER_OGC_1_0,
                                    u"geometry"_s, QString(), false, false, errorMessage, requiresFilterElement );
}

QDomElement QgsOgcUtils::elseFilterExpression( QDomDocument &doc )
{
  return doc.createElementNS( SE_NAMESPACE, u"se:ElseFilter"_s );
}


QDomElement QgsOgcUtils::expressionToOgcFilter( const QgsExpression &expression,
    QDomDocument &doc,
    GMLVersion gmlVersion,
    FilterVersion filterVersion,
    const QString &namespacePrefix,
    const QString &namespaceURI,
    const QString &geometryName,
    const QString &srsName,
    bool honourAxisOrientation,
    bool invertAxisOrientation,
    QString *errorMessage,
    const QMap<QString, QString> &fieldNameToXPathMap,
    const QMap<QString, QString> &namespacePrefixToUriMap )
{
  if ( !expression.rootNode() )
    return QDomElement();

  QgsExpression exp = expression;

  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope();
  QgsOgcUtilsExprToFilter utils( doc, gmlVersion, filterVersion, namespacePrefix, namespaceURI, geometryName, srsName, honourAxisOrientation, invertAxisOrientation, fieldNameToXPathMap, namespacePrefixToUriMap );
  const QDomElement exprRootElem = utils.expressionNodeToOgcFilter( exp.rootNode(), &exp, &context );
  if ( errorMessage )
    *errorMessage = utils.errorMessage();
  if ( exprRootElem.isNull() )
    return QDomElement();

  QDomElement filterElem = filterElement( doc, gmlVersion, filterVersion, utils.GMLNamespaceUsed() );

  if ( !namespacePrefix.isEmpty() && !namespaceURI.isEmpty() )
  {
    QDomAttr attr = doc.createAttribute( u"xmlns:"_s + namespacePrefix );
    attr.setValue( namespaceURI );
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
    QString *errorMessage,
    bool requiresFilterElement,
    const QMap<QString, QString> &fieldNameToXPathMap,
    const QMap<QString, QString> &namespacePrefixToUriMap )
{
  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope();

  QgsExpression exp = expression;

  const QgsExpressionNode *node = exp.rootNode();
  if ( !node )
    return QDomElement();

  QgsOgcUtilsExprToFilter utils( doc, gmlVersion, filterVersion, QString(), QString(), geometryName, srsName, honourAxisOrientation, invertAxisOrientation, fieldNameToXPathMap, namespacePrefixToUriMap );
  const QDomElement exprRootElem = utils.expressionNodeToOgcFilter( node, &exp, &context );

  if ( errorMessage )
  {
    *errorMessage = utils.errorMessage();
  }

  if ( !exprRootElem.isNull() )
  {
    if ( requiresFilterElement )
    {
      QDomElement filterElem = filterElement( doc, gmlVersion, filterVersion, utils.GMLNamespaceUsed() );

      filterElem.appendChild( exprRootElem );
      return filterElem;
    }
    return exprRootElem;
  }

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
    QString *errorMessage,
    const QMap<QString, QString> &fieldNameToXPathMap,
    const QMap<QString, QString> &namespacePrefixToUriMap )
{
  if ( !statement.rootNode() )
    return QDomElement();

  QgsOgcUtilsSQLStatementToFilter utils( doc, gmlVersion, filterVersion,
                                         layerProperties, honourAxisOrientation, invertAxisOrientation,
                                         mapUnprefixedTypenameToPrefixedTypename, fieldNameToXPathMap, namespacePrefixToUriMap );
  const QDomElement exprRootElem = utils.toOgcFilter( statement.rootNode() );
  if ( errorMessage )
    *errorMessage = utils.errorMessage();
  if ( exprRootElem.isNull() )
    return QDomElement();

  QDomElement filterElem = filterElement( doc, gmlVersion, filterVersion, utils.GMLNamespaceUsed() );

  QSet<QString> setNamespaceURI;
  for ( const LayerProperties &props : layerProperties )
  {
    if ( !props.mNamespacePrefix.isEmpty() && !props.mNamespaceURI.isEmpty() &&
         !setNamespaceURI.contains( props.mNamespaceURI ) )
    {
      setNamespaceURI.insert( props.mNamespaceURI );
      QDomAttr attr = doc.createAttribute( u"xmlns:"_s + props.mNamespacePrefix );
      attr.setValue( props.mNamespaceURI );
      filterElem.setAttributeNode( attr );
    }
  }
  filterElem.appendChild( exprRootElem );
  return filterElem;
}

//

/* static */ Qgis::WkbType QgsOgcUtils::geomTypeFromPropertyType( const QString &gmlGeomType )
{
  if ( gmlGeomType == "Point"_L1 )
    return Qgis::WkbType::Point;
  if ( gmlGeomType == "LineString"_L1 || gmlGeomType == "Curve"_L1 )
    return Qgis::WkbType::LineString;
  if ( gmlGeomType == "Polygon"_L1 || gmlGeomType == "Surface"_L1 )
    return Qgis::WkbType::Polygon;
  if ( gmlGeomType == "MultiPoint"_L1 )
    return Qgis::WkbType::MultiPoint;
  if ( gmlGeomType == "MultiLineString"_L1 || gmlGeomType == "MultiCurve"_L1 )
    return Qgis::WkbType::MultiLineString;
  if ( gmlGeomType == "MultiPolygon"_L1 || gmlGeomType == "MultiSurface"_L1 )
    return Qgis::WkbType::MultiPolygon;
  return Qgis::WkbType::Unknown;
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
      if ( QgsVariantUtils::isNull( rightLit->value() ) )
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
      boElem.setAttribute( u"matchCase"_s, u"false"_s );

    // setup wildCards to <ogc:PropertyIsLike>
    boElem.setAttribute( u"wildCard"_s, u"%"_s );
    boElem.setAttribute( u"singleChar"_s, u"_"_s );
    if ( mFilterVersion == QgsOgcUtils::FILTER_OGC_1_0 )
      boElem.setAttribute( u"escape"_s, u"\\"_s );
    else
      boElem.setAttribute( u"escapeChar"_s, u"\\"_s );
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
  switch ( node->value().userType() )
  {
    case QMetaType::Type::Int:
      value = QString::number( node->value().toInt() );
      break;
    case QMetaType::Type::Double:
      value = qgsDoubleToString( node->value().toDouble() );
      break;
    case QMetaType::Type::QString:
      value = node->value().toString();
      break;
    case QMetaType::Type::QDate:
      value = node->value().toDate().toString( Qt::ISODate );
      break;
    case QMetaType::Type::QDateTime:
      value = node->value().toDateTime().toString( Qt::ISODate );
      break;

    default:
      mErrorMessage = QObject::tr( "Literal type not supported: %1" ).arg( static_cast<QMetaType::Type>( node->value().userType() ) );
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
  if ( !mFieldNameToXPathMap.isEmpty() )
  {
    const auto iterFieldName = mFieldNameToXPathMap.constFind( node->name() );
    if ( iterFieldName != mFieldNameToXPathMap.constEnd() )
    {
      const QString xpath( *iterFieldName );

      if ( !mNamespacePrefixToUriMap.isEmpty() )
      {
        const QStringList parts = xpath.split( '/' );
        QSet<QString> setNamespacePrefix;
        for ( const QString &part : std::as_const( parts ) )
        {
          const QStringList subparts = part.split( ':' );
          if ( subparts.size() == 2 && !setNamespacePrefix.contains( subparts[0] ) )
          {
            const auto iterNamespacePrefix = mNamespacePrefixToUriMap.constFind( subparts[0] );
            if ( iterNamespacePrefix != mNamespacePrefixToUriMap.constEnd() )
            {
              setNamespacePrefix.insert( subparts[0] );
              QDomAttr attr = mDoc.createAttribute( u"xmlns:"_s +  subparts[0] );
              attr.setValue( *iterNamespacePrefix );
              propElem.setAttributeNode( attr );
            }
          }
        }
      }

      propElem.appendChild( mDoc.createTextNode( xpath ) );

      return propElem;
    }
  }
  QString columnRef( node->name() );
  if ( !mNamespacePrefix.isEmpty() && !mNamespaceURI.isEmpty() )
    columnRef =  mNamespacePrefix + u":"_s + columnRef;
  propElem.appendChild( mDoc.createTextNode( columnRef ) );
  return propElem;
}



QDomElement QgsOgcUtilsExprToFilter::expressionInOperatorToOgcFilter( const QgsExpressionNodeInOperator *node, QgsExpression *expression, const QgsExpressionContext *context )
{
  if ( node->list()->list().size() == 1 )
  {
    const QDomElement leftNode = expressionNodeToOgcFilter( node->node(), expression, context );
    const QDomElement firstListNode = expressionNodeToOgcFilter( node->list()->list().first(), expression, context );
    QDomElement eqElem = mDoc.createElement( mFilterPrefix + ":PropertyIsEqualTo" );
    eqElem.appendChild( leftNode );
    eqElem.appendChild( firstListNode );
    if ( node->isNotIn() )
    {
      QDomElement notElem = mDoc.createElement( mFilterPrefix + ":Not" );
      notElem.appendChild( eqElem );
      return notElem;
    }
    return eqElem;
  }

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
  { "disjoint"_L1,   "Disjoint"_L1 },
  { "intersects"_L1, "Intersects"_L1},
  { "touches"_L1,    "Touches"_L1 },
  { "crosses"_L1,    "Crosses"_L1 },
  { "contains"_L1,   "Contains"_L1 },
  { "overlaps"_L1,   "Overlaps"_L1 },
  { "within"_L1,     "Within"_L1 }
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
  return fd->name() == "$geometry"_L1 || ( fd->name() == "var"_L1 && fn->referencedVariables().contains( "geometry"_L1 ) );
}

static QgsGeometry geometryFromConstExpr( const QgsExpressionNode *node )
{
  // Right now we support only geomFromWKT(' ..... ')
  // Ideally we should support any constant sub-expression (not dependent on feature's geometry or attributes)

  if ( node->nodeType() == QgsExpressionNode::ntFunction )
  {
    const QgsExpressionNodeFunction *fnNode = static_cast<const QgsExpressionNodeFunction *>( node );
    QgsExpressionFunction *fnDef = QgsExpression::Functions()[fnNode->fnIndex()];
    if ( fnDef->name() == "geom_from_wkt"_L1 )
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

  if ( fd->name() == "intersects_bbox"_L1 )
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

      QDomElement funcElem = mDoc.createElement( mFilterPrefix + ":BBOX" );

      if ( !mGeometryName.isEmpty() )
      {
        // Geometry column is optional for a BBOX filter.
        QDomElement geomProperty = mDoc.createElement( mFilterPrefix + ":" + mPropertyName );
        QString columnRef( mGeometryName );
        if ( !mNamespacePrefix.isEmpty() && !mNamespaceURI.isEmpty() )
          columnRef =  mNamespacePrefix + u":"_s + columnRef;
        geomProperty.appendChild( mDoc.createTextNode( columnRef ) );

        funcElem.appendChild( geomProperty );
      }
      funcElem.appendChild( elemBox );
      return funcElem;
    }
    else
    {
      mErrorMessage = QObject::tr( "<BBOX> is currently supported only in form: bbox(@geometry, geomFromWKT('…'))" );
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
    if ( otherFnDef->name() == "geom_from_wkt"_L1 )
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
                      u"qgis_id_geom_%1"_s.arg( mGeomId ) );
      if ( otherGeomElem.isNull() )
      {
        mErrorMessage = QObject::tr( "geom_from_wkt: unable to generate GML from wkt geometry" );
        return QDomElement();
      }
      mGeomId ++;
    }
    else if ( otherFnDef->name() == "geom_from_gml"_L1 )
    {
      QgsExpressionNode *firstFnArg = otherFn->args()->list()[0];
      if ( firstFnArg->nodeType() != QgsExpressionNode::ntLiteral )
      {
        mErrorMessage = QObject::tr( "geom_from_gml: argument must be string literal" );
        return QDomElement();
      }

      QDomDocument geomDoc;
      const QString gml = static_cast<const QgsExpressionNodeLiteral *>( firstFnArg )->value().toString();
      // wrap the string into a root tag to have "gml" namespace
      const QString xml = u"<tmp xmlns:gml=\"%1\">%2</tmp>"_s.arg( GML_NAMESPACE, gml );
      if ( !geomDoc.setContent( xml, true ) )
      {
        mErrorMessage = QObject::tr( "geom_from_gml: unable to parse XML" );
        return QDomElement();
      }

      const QDomNode geomNode = mDoc.importNode( geomDoc.documentElement().firstChildElement(), true );
      otherGeomElem = geomNode.toElement();
    }
    else if ( otherNode->hasCachedStaticValue() && otherNode->cachedStaticValue().userType() == qMetaTypeId< QgsGeometry>() )
    {
      QgsGeometry geom = otherNode->cachedStaticValue().value<QgsGeometry>();
      otherGeomElem = QgsOgcUtils::geometryToGML( geom, mDoc, mGMLVersion, mSrsName, mInvertAxisOrientation,
                      u"qgis_id_geom_%1"_s.arg( mGeomId ) );
      if ( otherGeomElem.isNull() )
      {
        mErrorMessage = QObject::tr( "geom from static value: unable to generate GML from static variable" );
        return QDomElement();
      }
      mGeomId ++;
    }
    else
    {
      mErrorMessage = QObject::tr( "spatial operator: unknown geometry constructor function" );
      return QDomElement();
    }

    mGMLUsed = true;

    QDomElement funcElem = mDoc.createElement( mFilterPrefix + ":" + tagNameForSpatialOperator( fd->name() ) );
    QDomElement geomProperty = mDoc.createElement( mFilterPrefix + ":" + mPropertyName );
    QString columnRef( mGeometryName );
    if ( !mNamespacePrefix.isEmpty() && !mNamespaceURI.isEmpty() )
      columnRef =  mNamespacePrefix + u":"_s + columnRef;
    geomProperty.appendChild( mDoc.createTextNode( columnRef ) );
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
  funcElem.setAttribute( u"name"_s, fd->name() );
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
    const QMap< QString, QString> &mapUnprefixedTypenameToPrefixedTypename,
    const QMap<QString, QString> &fieldNameToXPathMap,
    const QMap<QString, QString> &namespacePrefixToUriMap )
  : mDoc( doc )
  , mGMLVersion( gmlVersion )
  , mFilterVersion( filterVersion )
  , mLayerProperties( layerProperties )
  , mHonourAxisOrientation( honourAxisOrientation )
  , mInvertAxisOrientation( invertAxisOrientation )
  , mFilterPrefix( ( filterVersion == QgsOgcUtils::FILTER_FES_2_0 ) ? "fes" : "ogc" )
  , mPropertyName( ( filterVersion == QgsOgcUtils::FILTER_FES_2_0 ) ? "ValueReference" : "PropertyName" )
  , mMapUnprefixedTypenameToPrefixedTypename( mapUnprefixedTypenameToPrefixedTypename )
  , mFieldNameToXPathMap( fieldNameToXPathMap )
  , mNamespacePrefixToUriMap( namespacePrefixToUriMap )
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
      if ( QgsVariantUtils::isNull( rightLit->value() ) )
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
    opText = u"Or"_s;
  else if ( op == QgsSQLStatement::boAnd )
    opText = u"And"_s;
  else if ( op == QgsSQLStatement::boEQ )
    opText = u"PropertyIsEqualTo"_s;
  else if ( op == QgsSQLStatement::boNE )
    opText = u"PropertyIsNotEqualTo"_s;
  else if ( op == QgsSQLStatement::boLE )
    opText = u"PropertyIsLessThanOrEqualTo"_s;
  else if ( op == QgsSQLStatement::boGE )
    opText = u"PropertyIsGreaterThanOrEqualTo"_s;
  else if ( op == QgsSQLStatement::boLT )
    opText = u"PropertyIsLessThan"_s;
  else if ( op == QgsSQLStatement::boGT )
    opText = u"PropertyIsGreaterThan"_s;
  else if ( op == QgsSQLStatement::boLike )
    opText = u"PropertyIsLike"_s;
  else if ( op == QgsSQLStatement::boILike )
    opText = u"PropertyIsLike"_s;

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
      boElem.setAttribute( u"matchCase"_s, u"false"_s );

    // setup wildCards to <ogc:PropertyIsLike>
    boElem.setAttribute( u"wildCard"_s, u"%"_s );
    boElem.setAttribute( u"singleChar"_s, u"_"_s );
    if ( mFilterVersion == QgsOgcUtils::FILTER_OGC_1_0 )
      boElem.setAttribute( u"escape"_s, u"\\"_s );
    else
      boElem.setAttribute( u"escapeChar"_s, u"\\"_s );
  }

  boElem.appendChild( leftElem );
  boElem.appendChild( rightElem );
  return boElem;
}


QDomElement QgsOgcUtilsSQLStatementToFilter::toOgcFilter( const QgsSQLStatement::NodeLiteral *node )
{
  QString value;
  switch ( node->value().userType() )
  {
    case QMetaType::Type::Int:
      value = QString::number( node->value().toInt() );
      break;
    case QMetaType::Type::LongLong:
      value = QString::number( node->value().toLongLong() );
      break;
    case QMetaType::Type::Double:
      value = qgsDoubleToString( node->value().toDouble() );
      break;
    case QMetaType::Type::QString:
      value = node->value().toString();
      break;

    default:
      mErrorMessage = QObject::tr( "Literal type not supported: %1" ).arg( static_cast<QMetaType::Type>( node->value().userType() ) );
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
    if ( !mFieldNameToXPathMap.isEmpty() )
    {
      const auto iterFieldName = mFieldNameToXPathMap.constFind( node->name() );
      if ( iterFieldName != mFieldNameToXPathMap.constEnd() )
      {
        const QString xpath( *iterFieldName );

        if ( !mNamespacePrefixToUriMap.isEmpty() )
        {
          const QStringList parts = xpath.split( '/' );
          QSet<QString> setNamespacePrefix;
          for ( const QString &part : std::as_const( parts ) )
          {
            const QStringList subparts = part.split( ':' );
            if ( subparts.size() == 2 && !setNamespacePrefix.contains( subparts[0] ) )
            {
              const auto iterNamespacePrefix = mNamespacePrefixToUriMap.constFind( subparts[0] );
              if ( iterNamespacePrefix != mNamespacePrefixToUriMap.constEnd() )
              {
                setNamespacePrefix.insert( subparts[0] );
                QDomAttr attr = mDoc.createAttribute( u"xmlns:"_s +  subparts[0] );
                attr.setValue( *iterNamespacePrefix );
                propElem.setAttributeNode( attr );
              }
            }
          }
        }

        propElem.appendChild( mDoc.createTextNode( xpath ) );

        return propElem;
      }
    }
    if ( mLayerProperties.size() == 1 && !mLayerProperties[0].mNamespacePrefix.isEmpty() && !mLayerProperties[0].mNamespaceURI.isEmpty() )
      propElem.appendChild( mDoc.createTextNode(
                              mLayerProperties[0].mNamespacePrefix + u":"_s + node->name() ) );
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
  {
    const QDomElement leftNode = toOgcFilter( node->node() );
    const QDomElement firstListNode = toOgcFilter( node->list()->list().first() );
    QDomElement eqElem = mDoc.createElement( mFilterPrefix + ":PropertyIsEqualTo" );
    eqElem.appendChild( leftNode );
    eqElem.appendChild( firstListNode );
    if ( node->isNotIn() )
    {
      QDomElement notElem = mDoc.createElement( mFilterPrefix + ":Not" );
      notElem.appendChild( eqElem );
      return notElem;
    }
    return eqElem;
  }

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
  if ( name.size() > 3 && QStringView {name} .mid( 0, 3 ).toString().compare( "ST_"_L1, Qt::CaseInsensitive ) == 0 )
    nameCompare = name.mid( 3 );
  QStringList spatialOps;
  spatialOps << u"BBOX"_s << u"Intersects"_s << u"Contains"_s << u"Crosses"_s << u"Equals"_s
             << u"Disjoint"_s << u"Overlaps"_s << u"Touches"_s << u"Within"_s;
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
  if ( name.size() > 3 && QStringView {name} .mid( 0, 3 ).compare( "ST_"_L1, Qt::CaseInsensitive ) == 0 )
    nameCompare = name.mid( 3 );
  if ( nameCompare.compare( "DWithin"_L1, Qt::CaseInsensitive ) == 0 )
    return u"DWithin"_s;
  if ( nameCompare.compare( "Beyond"_L1, Qt::CaseInsensitive ) == 0 )
    return u"Beyond"_s;
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
    if ( lit->value().userType() == QMetaType::Type::Int )
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
      if ( srsName.startsWith( "EPSG:"_L1, Qt::CaseInsensitive ) )
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
  if ( node->name().compare( "ST_GeometryFromText"_L1, Qt::CaseInsensitive ) == 0 )
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
                                 u"qgis_id_geom_%1"_s.arg( mGeomId ) );
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
  if ( node->name().compare( "ST_MakeEnvelope"_L1, Qt::CaseInsensitive ) == 0 )
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
      if ( lit->value().userType() == QMetaType::Type::Int )
        val = lit->value().toInt();
      else if ( lit->value().userType() == QMetaType::Type::LongLong )
        val = lit->value().toLongLong();
      else if ( lit->value().userType() == QMetaType::Type::Double )
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
  if ( node->name().compare( "ST_GeomFromGML"_L1, Qt::CaseInsensitive ) == 0 )
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
    // wrap the string into a root tag to have "gml" namespace
    const QString xml = u"<tmp xmlns:gml=\"%1\">%2</tmp>"_s.arg( GML_NAMESPACE, gml );
    if ( !geomDoc.setContent( xml, true ) )
    {
      mErrorMessage = QObject::tr( "ST_GeomFromGML: unable to parse XML" );
      return QDomElement();
    }

    const QDomNode geomNode = mDoc.importNode( geomDoc.documentElement().firstChildElement(), true );
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
           ( static_cast<const QgsSQLStatement::NodeFunction *>( args[i] )->name().compare( "ST_GeometryFromText"_L1, Qt::CaseInsensitive ) == 0 ||
             static_cast<const QgsSQLStatement::NodeFunction *>( args[i] )->name().compare( "ST_MakeEnvelope"_L1, Qt::CaseInsensitive ) == 0 ) )
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
           ( static_cast<const QgsSQLStatement::NodeFunction *>( args[i] )->name().compare( "ST_GeometryFromText"_L1, Qt::CaseInsensitive ) == 0 ||
             static_cast<const QgsSQLStatement::NodeFunction *>( args[i] )->name().compare( "ST_MakeEnvelope"_L1, Qt::CaseInsensitive ) == 0 ) )
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
    if ( QgsVariantUtils::isNull( lit->value() ) )
    {
      mErrorMessage = QObject::tr( "Function %1 3rd argument should be a numeric value or a string made of a numeric value followed by a string" ).arg( node->name() );
      return QDomElement();
    }
    QString distance;
    QString unit( u"m"_s );
    switch ( lit->value().userType() )
    {
      case QMetaType::Type::Int:
        distance = QString::number( lit->value().toInt() );
        break;
      case QMetaType::Type::LongLong:
        distance = QString::number( lit->value().toLongLong() );
        break;
      case QMetaType::Type::Double:
        distance = qgsDoubleToString( lit->value().toDouble() );
        break;
      case QMetaType::Type::QString:
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
        mErrorMessage = QObject::tr( "Literal type not supported: %1" ).arg( static_cast<QMetaType::Type>( lit->value().userType() ) );
        return QDomElement();
    }

    QDomElement distanceElem = mDoc.createElement( mFilterPrefix + ":Distance" );
    if ( mFilterVersion == QgsOgcUtils::FILTER_FES_2_0 )
      distanceElem.setAttribute( u"uom"_s, unit );
    else
      distanceElem.setAttribute( u"unit"_s, unit );
    distanceElem.appendChild( mDoc.createTextNode( distance ) );
    funcElem.appendChild( distanceElem );
    return funcElem;
  }

  // Other function
  QDomElement funcElem = mDoc.createElement( mFilterPrefix + ":Function" );
  funcElem.setAttribute( u"name"_s, node->name() );
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
  mPropertyName = u"PropertyName"_s;
  mPrefix = u"ogc"_s;

  if ( version == QgsOgcUtils::FILTER_FES_2_0 )
  {
    mPropertyName = u"ValueReference"_s;
    mPrefix = u"fes"_s;
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
  if ( element.tagName() == "Not"_L1 )
  {
    return nodeNotFromOgcFilter( element );
  }
  else if ( element.tagName() == "PropertyIsNull"_L1 )
  {
    return nodePropertyIsNullFromOgcFilter( element );
  }
  else if ( element.tagName() == "Literal"_L1 )
  {
    return nodeLiteralFromOgcFilter( element );
  }
  else if ( element.tagName() == "Function"_L1 )
  {
    return nodeFunctionFromOgcFilter( element );
  }
  else if ( element.tagName() == mPropertyName )
  {
    return nodeColumnRefFromOgcFilter( element );
  }
  else if ( element.tagName() == "PropertyIsBetween"_L1 )
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

  if ( op == QgsExpressionNodeBinaryOperator::boLike && element.hasAttribute( u"matchCase"_s ) && element.attribute( u"matchCase"_s ) == "false"_L1 )
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
      if ( element.hasAttribute( u"wildCard"_s ) )
      {
        wildCard = element.attribute( u"wildCard"_s );
      }
      QString singleChar;
      if ( element.hasAttribute( u"singleChar"_s ) )
      {
        singleChar = element.attribute( u"singleChar"_s );
      }
      QString escape = u"\\"_s;
      if ( element.hasAttribute( u"escape"_s ) )
      {
        escape = element.attribute( u"escape"_s );
      }
      if ( element.hasAttribute( u"escapeChar"_s ) )
      {
        escape = element.attribute( u"escapeChar"_s );
      }
      // replace
      QString oprValue = static_cast<const QgsExpressionNodeLiteral *>( opRight.get() )->value().toString();
      if ( !wildCard.isEmpty() && wildCard != "%"_L1 )
      {
        oprValue.replace( '%', "\\%"_L1 );
        if ( oprValue.startsWith( wildCard ) )
        {
          oprValue.replace( 0, 1, u"%"_s );
        }
        const QRegularExpression rx( "[^" + QgsStringUtils::qRegExpEscape( escape ) + "](" + QgsStringUtils::qRegExpEscape( wildCard ) + ")" );
        QRegularExpressionMatch match = rx.match( oprValue );
        int pos;
        while ( match.hasMatch() )
        {
          pos = match.capturedStart();
          oprValue.replace( pos + 1, 1, u"%"_s );
          pos += 1;
          match = rx.match( oprValue, pos );
        }
        oprValue.replace( escape + wildCard, wildCard );
      }
      if ( !singleChar.isEmpty() && singleChar != "_"_L1 )
      {
        oprValue.replace( '_', "\\_"_L1 );
        if ( oprValue.startsWith( singleChar ) )
        {
          oprValue.replace( 0, 1, u"_"_s );
        }
        const QRegularExpression rx( "[^" + QgsStringUtils::qRegExpEscape( escape ) + "](" + QgsStringUtils::qRegExpEscape( singleChar ) + ")" );
        QRegularExpressionMatch match = rx.match( oprValue );
        int pos;
        while ( match.hasMatch() )
        {
          pos = match.capturedStart();
          oprValue.replace( pos + 1, 1, u"_"_s );
          pos += 1;
          match = rx.match( oprValue, pos );
        }
        oprValue.replace( escape + singleChar, singleChar );
      }
      if ( !escape.isEmpty() && escape != "\\"_L1 )
      {
        oprValue.replace( escape + escape, escape );
      }
      opRight = std::make_unique<QgsExpressionNodeLiteral>( oprValue );
    }

    expr = std::make_unique<QgsExpressionNodeBinaryOperator>( static_cast< QgsExpressionNodeBinaryOperator::BinaryOperator >( op ), expr.release(), opRight.release() );
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

  auto gml2Args = std::make_unique<QgsExpressionNode::NodeList>();
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

  auto opArgs = std::make_unique<QgsExpressionNode::NodeList>();
  opArgs->append( new QgsExpressionNodeFunction( QgsExpression::functionIndex( u"$geometry"_s ), new QgsExpressionNode::NodeList() ) );
  opArgs->append( new QgsExpressionNodeFunction( QgsExpression::functionIndex( u"geomFromGML"_s ), gml2Args.release() ) );

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
  if ( element.isNull() || element.tagName() != "Literal"_L1 )
  {
    mErrorMessage = QObject::tr( "%1:Literal expected, got %2" ).arg( mPrefix, element.tagName() );
    return nullptr;
  }

  std::unique_ptr<QgsExpressionNode> root;
  if ( !element.hasChildNodes() )
  {
    root = std::make_unique<QgsExpressionNodeLiteral>( QVariant( "" ) );
    return root.release();
  }

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

      operand = std::make_unique<QgsExpressionNodeLiteral>( value );
    }

    // use the concat operator to merge the ogc:Literal children
    if ( !root )
    {
      root = std::move( operand );
    }
    else
    {
      root = std::make_unique<QgsExpressionNodeBinaryOperator>( QgsExpressionNodeBinaryOperator::boConcat, root.release(), operand.release() );
    }

    childNode = childNode.nextSibling();
  }

  if ( root )
    return root.release();

  return nullptr;
}

QgsExpressionNodeUnaryOperator *QgsOgcUtilsExpressionFromFilter::nodeNotFromOgcFilter( const QDomElement &element )
{
  if ( element.tagName() != "Not"_L1 )
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
  if ( element.tagName() != "PropertyIsNull"_L1 )
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
  if ( element.isNull() || element.tagName() != "Function"_L1 )
  {
    mErrorMessage = QObject::tr( "%1:Function expected, got %2" ).arg( mPrefix, element.tagName() );
    return nullptr;
  }

  for ( int i = 0; i < QgsExpression::Functions().size(); i++ )
  {
    const QgsExpressionFunction *funcDef = QgsExpression::Functions()[i];

    if ( element.attribute( u"name"_s ) != funcDef->name() )
      continue;

    auto args = std::make_unique<QgsExpressionNode::NodeList>();

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
    if ( operandElem.tagName() == "LowerBoundary"_L1 )
    {
      const QDomElement lowerBoundElem = operandElem.firstChildElement();
      lowerBound.reset( nodeFromOgcFilter( lowerBoundElem ) );
    }
    else if ( operandElem.tagName() == "UpperBoundary"_L1 )
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

QgsOgcCrsUtils::CRSFlavor QgsOgcCrsUtils::parseCrsName( const QString &crsName, QString &authority, QString &code )
{
  const thread_local QRegularExpression re_url( QRegularExpression::anchoredPattern( u"http://www\\.opengis\\.net/gml/srs/epsg\\.xml#(.+)"_s ), QRegularExpression::CaseInsensitiveOption );
  if ( const QRegularExpressionMatch match = re_url.match( crsName ); match.hasMatch() )
  {
    authority = u"EPSG"_s;
    code = match.captured( 1 );
    return CRSFlavor::HTTP_EPSG_DOT_XML;
  }

  const thread_local QRegularExpression re_ogc_urn( QRegularExpression::anchoredPattern( u"urn:ogc:def:crs:([^:]+).+(?<=:)([^:]+)"_s ), QRegularExpression::CaseInsensitiveOption );
  if ( const QRegularExpressionMatch match = re_ogc_urn.match( crsName ); match.hasMatch() )
  {
    authority = match.captured( 1 );
    code = match.captured( 2 );
    return CRSFlavor::OGC_URN;
  }

  const thread_local QRegularExpression re_x_ogc_urn( QRegularExpression::anchoredPattern( u"urn:x-ogc:def:crs:([^:]+).+(?<=:)([^:]+)"_s ), QRegularExpression::CaseInsensitiveOption );
  if ( const QRegularExpressionMatch match = re_x_ogc_urn.match( crsName ); match.hasMatch() )
  {
    authority = match.captured( 1 );
    code = match.captured( 2 );
    return CRSFlavor::X_OGC_URN;
  }

  const thread_local QRegularExpression re_http_uri( QRegularExpression::anchoredPattern( u"http://www\\.opengis\\.net/def/crs/([^/]+).+/([^/]+)"_s ), QRegularExpression::CaseInsensitiveOption );
  if ( const QRegularExpressionMatch match = re_http_uri.match( crsName ); match.hasMatch() )
  {
    authority = match.captured( 1 );
    code = match.captured( 2 );
    return CRSFlavor::OGC_HTTP_URI;
  }

  const thread_local QRegularExpression re_auth_code( QRegularExpression::anchoredPattern( u"([^:]+):(.+)"_s ), QRegularExpression::CaseInsensitiveOption );
  if ( const QRegularExpressionMatch match = re_auth_code.match( crsName ); match.hasMatch() )
  {
    authority = match.captured( 1 );
    code = match.captured( 2 );
    return CRSFlavor::AUTH_CODE;
  }

  return CRSFlavor::UNKNOWN;
}

QgsGeometry QgsOgcUtils::geometryFromGMLUsingGdal( const QDomElement &geometryElement )
{
  QString gml;
  QTextStream gmlStream( &gml );
  geometryElement.save( gmlStream, 0 );
  gdal::ogr_geometry_unique_ptr ogrGeom { OGR_G_CreateFromGML( gml.toUtf8().constData() ) };
  return QgsOgrUtils::ogrGeometryToQgsGeometry( ogrGeom.get() );
}

QgsGeometry QgsOgcUtils::geometryFromGMLMultiCurve( const QDomElement &geometryElement )
{
  return geometryFromGMLUsingGdal( geometryElement );
}
