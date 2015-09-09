#include "qgsogcutils.h"

#include "qgsexpression.h"
#include "qgsgeometry.h"
#include "qgswkbptr.h"

#include <QColor>
#include <QStringList>
#include <QTextStream>

#ifndef Q_OS_WIN
#include <netinet/in.h>
#else
#include <winsock.h>
#endif


static const QString GML_NAMESPACE = "http://www.opengis.net/gml";
static const QString OGC_NAMESPACE = "http://www.opengis.net/ogc";

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
      return 0;
    }
    geometryTypeElement = geometryChild.toElement();
    geomType = geometryTypeElement.tagName();
  }

  if ( !( geomType == "Point" || geomType == "LineString" || geomType == "Polygon" ||
          geomType == "MultiPoint" || geomType == "MultiLineString" || geomType == "MultiPolygon" ||
          geomType == "Box" || geomType == "Envelope" ) )
    return 0;

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
    return 0;
  }
}

QgsGeometry* QgsOgcUtils::geometryFromGML( const QString& xmlString )
{
  // wrap the string into a root tag to have "gml" namespace (and also as a default namespace)
  QString xml = QString( "<tmp xmlns=\"%1\" xmlns:gml=\"%1\">%2</tmp>" ).arg( GML_NAMESPACE ).arg( xmlString );
  QDomDocument doc;
  if ( !doc.setContent( xml, true ) )
    return 0;

  return geometryFromGML( doc.documentElement().firstChildElement() );
}


QgsGeometry* QgsOgcUtils::geometryFromGMLPoint( const QDomElement& geometryElement )
{
  QgsPolyline pointCoordinate;

  QDomNodeList coordList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "coordinates" );
  if ( coordList.size() > 0 )
  {
    QDomElement coordElement = coordList.at( 0 ).toElement();
    if ( readGMLCoordinates( pointCoordinate, coordElement ) != 0 )
    {
      return 0;
    }
  }
  else
  {
    QDomNodeList posList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "pos" );
    if ( posList.size() < 1 )
    {
      return 0;
    }
    QDomElement posElement = posList.at( 0 ).toElement();
    if ( readGMLPositions( pointCoordinate, posElement ) != 0 )
    {
      return 0;
    }
  }

  if ( pointCoordinate.size() < 1 )
  {
    return 0;
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
  if ( coordList.size() > 0 )
  {
    QDomElement coordElement = coordList.at( 0 ).toElement();
    if ( readGMLCoordinates( lineCoordinates, coordElement ) != 0 )
    {
      return 0;
    }
  }
  else
  {
    QDomNodeList posList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "posList" );
    if ( posList.size() < 1 )
    {
      return 0;
    }
    QDomElement posElement = posList.at( 0 ).toElement();
    if ( readGMLPositions( lineCoordinates, posElement ) != 0 )
    {
      return 0;
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
  if ( outerBoundaryList.size() > 0 ) //outer ring is necessary
  {
    QDomElement coordinatesElement = outerBoundaryList.at( 0 ).firstChild().firstChild().toElement();
    if ( coordinatesElement.isNull() )
    {
      return 0;
    }
    if ( readGMLCoordinates( exteriorPointList, coordinatesElement ) != 0 )
    {
      return 0;
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
        return 0;
      }
      if ( readGMLCoordinates( interiorPointList, coordinatesElement ) != 0 )
      {
        return 0;
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
      return 0;
    }
    QDomElement posElement = exteriorList.at( 0 ).firstChild().firstChild().toElement();
    if ( posElement.isNull() )
    {
      return 0;
    }
    if ( readGMLPositions( exteriorPointList, posElement ) != 0 )
    {
      return 0;
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
        return 0;
      }
      if ( readGMLPositions( interiorPointList, posElement ) != 0 )
      {
        return 0;
      }
      ringCoordinates.push_back( interiorPointList );
    }
  }

  //calculate number of bytes to allocate
  int nrings = ringCoordinates.size();
  if ( nrings < 1 )
    return 0;

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
    return 0;
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
    if ( coordinatesList.size() > 0 )
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
    return 0;

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
  if ( lineStringMemberList.size() > 0 ) //geoserver
  {
    for ( int i = 0; i < lineStringMemberList.size(); ++i )
    {
      QDomNodeList lineStringNodeList = lineStringMemberList.at( i ).toElement().elementsByTagNameNS( GML_NAMESPACE, "LineString" );
      if ( lineStringNodeList.size() < 1 )
      {
        return 0;
      }
      currentLineStringElement = lineStringNodeList.at( 0 ).toElement();
      currentCoordList = currentLineStringElement.elementsByTagNameNS( GML_NAMESPACE, "coordinates" );
      if ( currentCoordList.size() > 0 )
      {
        QgsPolyline currentPointList;
        if ( readGMLCoordinates( currentPointList, currentCoordList.at( 0 ).toElement() ) != 0 )
        {
          return 0;
        }
        lineCoordinates.push_back( currentPointList );
      }
      else
      {
        currentPosList = currentLineStringElement.elementsByTagNameNS( GML_NAMESPACE, "posList" );
        if ( currentPosList.size() < 1 )
        {
          return 0;
        }
        QgsPolyline currentPointList;
        if ( readGMLPositions( currentPointList, currentPosList.at( 0 ).toElement() ) != 0 )
        {
          return 0;
        }
        lineCoordinates.push_back( currentPointList );
      }
    }
  }
  else
  {
    QDomNodeList lineStringList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "LineString" );
    if ( lineStringList.size() > 0 ) //mapserver
    {
      for ( int i = 0; i < lineStringList.size(); ++i )
      {
        currentLineStringElement = lineStringList.at( i ).toElement();
        currentCoordList = currentLineStringElement.elementsByTagNameNS( GML_NAMESPACE, "coordinates" );
        if ( currentCoordList.size() > 0 )
        {
          QgsPolyline currentPointList;
          if ( readGMLCoordinates( currentPointList, currentCoordList.at( 0 ).toElement() ) != 0 )
          {
            return 0;
          }
          lineCoordinates.push_back( currentPointList );
          return 0;
        }
        else
        {
          currentPosList = currentLineStringElement.elementsByTagNameNS( GML_NAMESPACE, "posList" );
          if ( currentPosList.size() < 1 )
          {
            return 0;
          }
          QgsPolyline currentPointList;
          if ( readGMLPositions( currentPointList, currentPosList.at( 0 ).toElement() ) != 0 )
          {
            return 0;
          }
          lineCoordinates.push_back( currentPointList );
        }
      }
    }
    else
    {
      return 0;
    }
  }

  int nLines = lineCoordinates.size();
  if ( nLines < 1 )
    return 0;

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
    if ( outerBoundaryList.size() > 0 )
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
    return 0;

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
  //tupel and coord separator are the same
  QString coordSeparator = " ";
  QString tupelSeparator = " ";
  //"decimal" has to be "."


  coords.clear();

  QStringList pos = elem.text().split( " ", QString::SkipEmptyParts );
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

  double xmin = bString.section( " ", 0, 0 ).toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    return rect;
  double ymin = bString.section( " ", 1, 1 ).toDouble( &conversionSuccess );
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
  double xmax = bString.section( " ", 0, 0 ).toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    return rect;
  double ymax = bString.section( " ", 1, 1 ).toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    return rect;

  rect = QgsRectangle( xmin, ymin, xmax, ymax );
  rect.normalize();

  return rect;
}

QDomElement QgsOgcUtils::rectangleToGMLBox( QgsRectangle* box, QDomDocument& doc, const int &precision )
{
  if ( !box )
  {
    return QDomElement();
  }

  QDomElement boxElem = doc.createElement( "gml:Box" );
  QDomElement coordElem = doc.createElement( "gml:coordinates" );
  coordElem.setAttribute( "cs", "," );
  coordElem.setAttribute( "ts", " " );

  QString coordString;
  coordString += qgsDoubleToString( box->xMinimum(), precision );
  coordString += ",";
  coordString += qgsDoubleToString( box->yMinimum(), precision );
  coordString += " ";
  coordString += qgsDoubleToString( box->xMaximum(), precision );
  coordString += ",";
  coordString += qgsDoubleToString( box->yMaximum(), precision );

  QDomText coordText = doc.createTextNode( coordString );
  coordElem.appendChild( coordText );
  boxElem.appendChild( coordElem );

  return boxElem;
}

QDomElement QgsOgcUtils::rectangleToGMLEnvelope( QgsRectangle* env, QDomDocument& doc, const int &precision )
{
  if ( !env )
  {
    return QDomElement();
  }

  QDomElement envElem = doc.createElement( "gml:Envelope" );
  QString posList;

  QDomElement lowerCornerElem = doc.createElement( "gml:lowerCorner" );
  posList = qgsDoubleToString( env->xMinimum(), precision );
  posList += " ";
  posList += qgsDoubleToString( env->yMinimum(), precision );
  QDomText lowerCornerText = doc.createTextNode( posList );
  lowerCornerElem.appendChild( lowerCornerText );
  envElem.appendChild( lowerCornerElem );

  QDomElement upperCornerElem = doc.createElement( "gml:upperCorner" );
  posList = qgsDoubleToString( env->xMaximum(), precision );
  posList += " ";
  posList += qgsDoubleToString( env->yMaximum(), precision );
  QDomText upperCornerText = doc.createTextNode( posList );
  upperCornerElem.appendChild( upperCornerText );
  envElem.appendChild( upperCornerElem );

  return envElem;
}

QDomElement QgsOgcUtils::geometryToGML( const QgsGeometry* geometry, QDomDocument& doc, QString format, const int &precision )
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

  QgsConstWkbPtr wkbPtr( geometry->asWkb() + 1 + sizeof( int ) );

  if ( format == "GML3" )
  {
    switch ( geometry->wkbType() )
    {
      case QGis::WKBPoint25D:
      case QGis::WKBPoint:
      case QGis::WKBMultiPoint25D:
      case QGis::WKBMultiPoint:
        baseCoordElem = doc.createElement( "gml:pos" );;
        break;
      default:
        baseCoordElem = doc.createElement( "gml:posList" );;
        break;
    }
    baseCoordElem.setAttribute( "srsDimension", "2" );
    cs = " ";
  }
  else
  {
    baseCoordElem = doc.createElement( "gml:coordinates" );;
    baseCoordElem.setAttribute( "cs", cs );
    baseCoordElem.setAttribute( "ts", ts );
  }

  switch ( geometry->wkbType() )
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
    {
      QDomElement pointElem = doc.createElement( "gml:Point" );
      QDomElement coordElem = baseCoordElem.cloneNode().toElement();

      double x, y;
      wkbPtr >> x >> y;
      QDomText coordText = doc.createTextNode( qgsDoubleToString( x, precision ) + cs + qgsDoubleToString( y, precision ) );

      coordElem.appendChild( coordText );
      pointElem.appendChild( coordElem );
      return pointElem;
    }
    case QGis::WKBMultiPoint25D:
      hasZValue = true;
      //intentional fall-through
    case QGis::WKBMultiPoint:
    {
      QDomElement multiPointElem = doc.createElement( "gml:MultiPoint" );

      int nPoints;
      wkbPtr >> nPoints;

      for ( int idx = 0; idx < nPoints; ++idx )
      {
        wkbPtr += 1 + sizeof( int );
        QDomElement pointMemberElem = doc.createElement( "gml:pointMember" );
        QDomElement pointElem = doc.createElement( "gml:Point" );
        QDomElement coordElem = baseCoordElem.cloneNode().toElement();

        double x, y;
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
    case QGis::WKBLineString:
    {
      QDomElement lineStringElem = doc.createElement( "gml:LineString" );
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
    case QGis::WKBMultiLineString:
    {
      QDomElement multiLineStringElem = doc.createElement( "gml:MultiLineString" );

      int nLines;
      wkbPtr >> nLines;

      for ( int jdx = 0; jdx < nLines; jdx++ )
      {
        QDomElement lineStringMemberElem = doc.createElement( "gml:lineStringMember" );
        QDomElement lineStringElem = doc.createElement( "gml:LineString" );
        wkbPtr += 1 + sizeof( int ); // skip type since we know its 2

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
    case QGis::WKBPolygon:
    {
      QDomElement polygonElem = doc.createElement( "gml:Polygon" );

      // get number of rings in the polygon
      int numRings;
      wkbPtr >> numRings;

      if ( numRings == 0 ) // sanity check for zero rings in polygon
        return QDomElement();

      int *ringNumPoints = new int[numRings]; // number of points in each ring

      for ( int idx = 0; idx < numRings; idx++ )
      {
        QString boundaryName = "gml:outerBoundaryIs";
        if ( idx != 0 )
        {
          boundaryName = "gml:innerBoundaryIs";
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
    case QGis::WKBMultiPolygon:
    {
      QDomElement multiPolygonElem = doc.createElement( "gml:MultiPolygon" );

      int numPolygons;
      wkbPtr >> numPolygons;

      for ( int kdx = 0; kdx < numPolygons; kdx++ )
      {
        QDomElement polygonMemberElem = doc.createElement( "gml:polygonMember" );
        QDomElement polygonElem = doc.createElement( "gml:Polygon" );

        wkbPtr += 1 + sizeof( int );

        int numRings;
        wkbPtr >> numRings;

        for ( int idx = 0; idx < numRings; idx++ )
        {
          QString boundaryName = "gml:outerBoundaryIs";
          if ( idx != 0 )
          {
            boundaryName = "gml:innerBoundaryIs";
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

QDomElement QgsOgcUtils::geometryToGML( const QgsGeometry *geometry, QDomDocument &doc, const int &precision )
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
      coordString += " ";
    }
    coordString += qgsDoubleToString( pointIt->x() );
    coordString += ",";
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
      coordString += " ";
    }
    coordString += qgsDoubleToString( pointIt->x() );
    coordString += " ";
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
    return NULL;

  QgsExpression *expr = new QgsExpression();

  QDomElement childElem = element.firstChildElement();
  while ( !childElem.isNull() )
  {
    QString errorMsg;
    QgsExpression::Node *node = nodeFromOgcFilter( childElem, errorMsg );
    if ( !node )
    {
      // invalid expression, parser error
      expr->mParserErrorString = errorMsg;
      return expr;
    }

    // use the concat binary operator to append to the root node
    if ( !expr->mRootNode )
    {
      expr->mRootNode = node;
    }
    else
    {
      expr->mRootNode = new QgsExpression::NodeBinaryOperator( QgsExpression::boConcat, expr->mRootNode, node );
    }

    childElem = childElem.nextSiblingElement();
  }

  // update expression string
  expr->mExp = expr->dump();

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
    spatialOps << "BBOX" << "Intersects" << "Contians" << "Crosses" << "Equals"
    << "Disjoint" << "Overlaps" << "Touches" << "Within";
  }

  return spatialOps.contains( tagName );
}



QgsExpression::Node* QgsOgcUtils::nodeFromOgcFilter( QDomElement &element, QString &errorMessage )
{
  if ( element.isNull() )
    return NULL;

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

  errorMessage += QString( "unable to convert '%1' element to a valid expression: it is not supported yet or it has invalid arguments" ).arg( element.tagName() );
  return NULL;
}



QgsExpression::NodeBinaryOperator* QgsOgcUtils::nodeBinaryOperatorFromOgcFilter( QDomElement &element, QString &errorMessage )
{
  if ( element.isNull() )
    return NULL;

  int op = binaryOperatorFromTagName( element.tagName() );
  if ( op < 0 )
  {
    if ( errorMessage.isEmpty() )
      errorMessage = QString( "'%1' binary operator not supported." ).arg( element.tagName() );
    return NULL;
  }

  QDomElement operandElem = element.firstChildElement();
  QgsExpression::Node *expr = nodeFromOgcFilter( operandElem, errorMessage ), *leftOp = expr;
  if ( !expr )
  {
    if ( errorMessage.isEmpty() )
      errorMessage = QString( "invalid left operand for '%1' binary operator" ).arg( element.tagName() );
    return NULL;
  }

  for ( operandElem = operandElem.nextSiblingElement(); !operandElem.isNull(); operandElem = operandElem.nextSiblingElement() )
  {
    QgsExpression::Node* opRight = nodeFromOgcFilter( operandElem, errorMessage );
    if ( !opRight )
    {
      if ( errorMessage.isEmpty() )
        errorMessage = QString( "invalid right operand for '%1' binary operator" ).arg( element.tagName() );
      delete expr;
      return NULL;
    }

    expr = new QgsExpression::NodeBinaryOperator(( QgsExpression::BinaryOperator ) op, expr, opRight );
  }

  if ( expr == leftOp )
  {
    if ( errorMessage.isEmpty() )
      errorMessage = QString( "only one operand for '%1' binary operator" ).arg( element.tagName() );
    delete expr;
    return NULL;
  }

  QgsExpression::NodeBinaryOperator *ret = dynamic_cast< QgsExpression::NodeBinaryOperator * >( expr );
  if ( !ret )
    delete expr;

  return ret;
}


QgsExpression::NodeFunction* QgsOgcUtils::nodeSpatialOperatorFromOgcFilter( QDomElement& element, QString& errorMessage )
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
    gml2Args->append( new QgsExpression::NodeLiteral( QVariant( gml2Str.remove( "\n" ) ) ) );
  }
  else
  {
    errorMessage = "No OGC Geometry found";
    delete gml2Args;
    return NULL;
  }

  QgsExpression::NodeList *opArgs = new QgsExpression::NodeList();
  opArgs->append( new QgsExpression::NodeFunction( QgsExpression::functionIndex( "$geometry" ), new QgsExpression::NodeList() ) );
  opArgs->append( new QgsExpression::NodeFunction( QgsExpression::functionIndex( "geomFromGML" ), gml2Args ) );

  return new QgsExpression::NodeFunction( opIdx, opArgs );
}


QgsExpression::NodeUnaryOperator* QgsOgcUtils::nodeNotFromOgcFilter( QDomElement &element, QString &errorMessage )
{
  if ( element.tagName() != "Not" )
    return NULL;

  QDomElement operandElem = element.firstChildElement();
  QgsExpression::Node* operand = nodeFromOgcFilter( operandElem, errorMessage );
  if ( !operand )
  {
    if ( errorMessage.isEmpty() )
      errorMessage = QString( "invalid operand for '%1' unary operator" ).arg( element.tagName() );
    return NULL;
  }

  return new QgsExpression::NodeUnaryOperator( QgsExpression::uoNot, operand );
}


QgsExpression::NodeFunction* QgsOgcUtils::nodeFunctionFromOgcFilter( QDomElement &element, QString &errorMessage )
{
  if ( element.isNull() || element.tagName() != "Function" )
  {
    errorMessage = QString( "ogc:Function expected, got %1" ).arg( element.tagName() );
    return NULL;
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
        return NULL;
      }
      args->append( op );

      operandElem = operandElem.nextSiblingElement();
    }

    return new QgsExpression::NodeFunction( i, args );
  }

  return NULL;
}



QgsExpression::Node* QgsOgcUtils::nodeLiteralFromOgcFilter( QDomElement &element, QString &errorMessage )
{
  if ( element.isNull() || element.tagName() != "Literal" )
  {
    errorMessage = QString( "ogc:Literal expected, got %1" ).arg( element.tagName() );
    return NULL;
  }

  QgsExpression::Node *root = 0;

  // the literal content can have more children (e.g. CDATA section, text, ...)
  QDomNode childNode = element.firstChild();
  while ( !childNode.isNull() )
  {
    QgsExpression::Node* operand = 0;

    if ( childNode.nodeType() == QDomNode::ElementNode )
    {
      // found a element node (e.g. PropertyName), convert it
      QDomElement operandElem = childNode.toElement();
      operand = nodeFromOgcFilter( operandElem, errorMessage );
      if ( !operand )
      {
        if ( root )
          delete root;

        errorMessage = QString( "'%1' is an invalid or not supported content for ogc:Literal" ).arg( operandElem.tagName() );
        return NULL;
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

  return NULL;
}


QgsExpression::NodeColumnRef* QgsOgcUtils::nodeColumnRefFromOgcFilter( QDomElement &element, QString &errorMessage )
{
  if ( element.isNull() || element.tagName() != "PropertyName" )
  {
    errorMessage = QString( "ogc:PropertyName expected, got %1" ).arg( element.tagName() );
    return NULL;
  }

  return new QgsExpression::NodeColumnRef( element.firstChild().nodeValue() );
}


QgsExpression::Node* QgsOgcUtils::nodeIsBetweenFromOgcFilter( QDomElement& element, QString& errorMessage )
{
  // <ogc:PropertyIsBetween> encode a Range check
  QgsExpression::Node *operand = 0, *lowerBound = 0;
  QgsExpression::Node *operand2 = 0, *upperBound = 0;

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

    errorMessage = "missing some required sub-elements in ogc:PropertyIsBetween";
    return NULL;
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
    return NULL;
  }

  QDomElement operandElem = element.firstChildElement();
  QgsExpression::Node* opLeft = nodeFromOgcFilter( operandElem, errorMessage );
  if ( !opLeft )
    return NULL;

  QgsExpression::Node* opRight = new QgsExpression::NodeLiteral( QVariant() );
  return new QgsExpression::NodeBinaryOperator( QgsExpression::boIs, opLeft, opRight );
}


/////////////////


QDomElement QgsOgcUtils::expressionToOgcFilter( const QgsExpression& exp, QDomDocument& doc, QString* errorMessage )
{
  if ( !exp.rootNode() )
    return QDomElement();

  QString localErrorMessage; // temporary that will be thrown away unused
  QString& refErrorMessage = ( errorMessage ? *errorMessage : localErrorMessage );
  refErrorMessage.clear();

  QDomElement exprRootElem = expressionNodeToOgcFilter( exp.rootNode(), doc, refErrorMessage );
  if ( exprRootElem.isNull() )
    return QDomElement();

  QDomElement filterElem = doc.createElementNS( OGC_NAMESPACE, "ogc:Filter" );
  filterElem.appendChild( exprRootElem );
  return filterElem;
}


QDomElement QgsOgcUtils::expressionNodeToOgcFilter( const QgsExpression::Node* node, QDomDocument& doc, QString& errorMessage )
{
  switch ( node->nodeType() )
  {
    case QgsExpression::ntUnaryOperator:
      return expressionUnaryOperatorToOgcFilter( static_cast<const QgsExpression::NodeUnaryOperator*>( node ), doc, errorMessage );
    case QgsExpression::ntBinaryOperator:
      return expressionBinaryOperatorToOgcFilter( static_cast<const QgsExpression::NodeBinaryOperator*>( node ), doc, errorMessage );
    case QgsExpression::ntInOperator:
      return expressionInOperatorToOgcFilter( static_cast<const QgsExpression::NodeInOperator*>( node ), doc, errorMessage );
    case QgsExpression::ntFunction:
      return expressionFunctionToOgcFilter( static_cast<const QgsExpression::NodeFunction*>( node ), doc, errorMessage );
    case QgsExpression::ntLiteral:
      return expressionLiteralToOgcFilter( static_cast<const QgsExpression::NodeLiteral*>( node ), doc, errorMessage );
    case QgsExpression::ntColumnRef:
      return expressionColumnRefToOgcFilter( static_cast<const QgsExpression::NodeColumnRef*>( node ), doc, errorMessage );

    default:
      errorMessage = QString( "Node type not supported: %1" ).arg( node->nodeType() );
      return QDomElement();
  }
}


QDomElement QgsOgcUtils::expressionUnaryOperatorToOgcFilter( const QgsExpression::NodeUnaryOperator* node, QDomDocument& doc, QString& errorMessage )
{

  QDomElement operandElem = expressionNodeToOgcFilter( node->operand(), doc, errorMessage );
  if ( !errorMessage.isEmpty() )
    return QDomElement();

  QDomElement uoElem;
  switch ( node->op() )
  {
    case QgsExpression::uoMinus:
      uoElem = doc.createElement( "ogc:Literal" );
      if ( node->operand()->nodeType() == QgsExpression::ntLiteral )
      {
        // operand expression already created a Literal node:
        // take the literal value, prepend - and remove old literal node
        uoElem.appendChild( doc.createTextNode( "-" + operandElem.text() ) );
        doc.removeChild( operandElem );
      }
      else
      {
        errorMessage = QString( "This use of unary operator not implemented yet" );
        return QDomElement();
      }
      break;
    case QgsExpression::uoNot:
      uoElem = doc.createElement( "ogc:Not" );
      uoElem.appendChild( operandElem );
      break;

    default:
      errorMessage = QString( "Unary operator %1 not implemented yet" ).arg( QgsExpression::UnaryOperatorText[node->op()] );
      return QDomElement();
  }

  return uoElem;
}


QDomElement QgsOgcUtils::expressionBinaryOperatorToOgcFilter( const QgsExpression::NodeBinaryOperator* node, QDomDocument& doc, QString& errorMessage )
{
  QDomElement leftElem = expressionNodeToOgcFilter( node->opLeft(), doc, errorMessage );
  if ( !errorMessage.isEmpty() )
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

        QDomElement elem = doc.createElement( "ogc:PropertyIsNull" );
        elem.appendChild( leftElem );

        if ( op == QgsExpression::boIsNot )
        {
          QDomElement notElem = doc.createElement( "ogc:Not" );
          notElem.appendChild( elem );
          return notElem;
        }

        return elem;
      }

      // continue with equal / not equal operator once the null case is handled
      op = ( op == QgsExpression::boIs ? QgsExpression::boEQ : QgsExpression::boNE );
    }

  }

  QDomElement rightElem = expressionNodeToOgcFilter( node->opRight(), doc, errorMessage );
  if ( !errorMessage.isEmpty() )
    return QDomElement();


  QString opText = binaryOperatorToTagName( op );
  if ( opText.isEmpty() )
  {
    // not implemented binary operators
    // TODO: regex, % (mod), ^ (pow) are not supported yet
    errorMessage = QString( "Binary operator %1 not implemented yet" ).arg( QgsExpression::BinaryOperatorText[op] );
    return QDomElement();
  }

  QDomElement boElem = doc.createElement( "ogc:" + opText );

  if ( op == QgsExpression::boLike || op == QgsExpression::boILike )
  {
    if ( op == QgsExpression::boILike )
      boElem.setAttribute( "matchCase", "false" );

    // setup wildcards to <ogc:PropertyIsLike>
    boElem.setAttribute( "wildCard", "%" );
    boElem.setAttribute( "singleChar", "?" );
    boElem.setAttribute( "escapeChar", "!" );
  }

  boElem.appendChild( leftElem );
  boElem.appendChild( rightElem );
  return boElem;
}


QDomElement QgsOgcUtils::expressionLiteralToOgcFilter( const QgsExpression::NodeLiteral* node, QDomDocument& doc, QString& errorMessage )
{
  QString value;
  switch ( node->value().type() )
  {
    case QVariant::Int:
      value = QString::number( node->value().toInt() );
      break;
    case QVariant::Double:
      value = QString::number( node->value().toDouble() );
      break;
    case QVariant::String:
      value = node->value().toString();
      break;

    default:
      errorMessage = QString( "Literal type not supported: %1" ).arg( node->value().type() );
      return QDomElement();
  }

  QDomElement litElem = doc.createElement( "ogc:Literal" );
  litElem.appendChild( doc.createTextNode( value ) );
  return litElem;
}


QDomElement QgsOgcUtils::expressionColumnRefToOgcFilter( const QgsExpression::NodeColumnRef* node, QDomDocument& doc, QString& /*errorMessage*/ )
{
  QDomElement propElem = doc.createElement( "ogc:PropertyName" );
  propElem.appendChild( doc.createTextNode( node->name() ) );
  return propElem;
}



QDomElement QgsOgcUtils::expressionInOperatorToOgcFilter( const QgsExpression::NodeInOperator* node, QDomDocument& doc, QString& errorMessage )
{
  if ( node->list()->list().size() == 1 )
    return expressionNodeToOgcFilter( node->list()->list()[0], doc, errorMessage );

  QDomElement orElem = doc.createElement( "ogc:Or" );
  QDomElement leftNode = expressionNodeToOgcFilter( node->node(), doc, errorMessage );

  Q_FOREACH ( QgsExpression::Node* n, node->list()->list() )
  {
    QDomElement listNode = expressionNodeToOgcFilter( n, doc, errorMessage );
    if ( !errorMessage.isEmpty() )
      return QDomElement();

    QDomElement eqElem = doc.createElement( "ogc:PropertyIsEqualTo" );
    eqElem.appendChild( leftNode.cloneNode() );
    eqElem.appendChild( listNode );

    orElem.appendChild( eqElem );
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
  return 0;
}


QDomElement QgsOgcUtils::expressionFunctionToOgcFilter( const QgsExpression::NodeFunction* node, QDomDocument& doc, QString& errorMessage )
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

      QDomElement elemBox = rectangleToGMLBox( &rect, doc );

      QDomElement geomProperty = doc.createElement( "ogc:PropertyName" );
      geomProperty.appendChild( doc.createTextNode( "geometry" ) );

      QDomElement funcElem = doc.createElement( "ogr:BBOX" );
      funcElem.appendChild( geomProperty );
      funcElem.appendChild( elemBox );
      return funcElem;
    }
    else
    {
      delete geom;

      errorMessage = QString( "<BBOX> is currently supported only in form: bbox($geometry, geomFromWKT('...'))" );
      return QDomElement();
    }
  }

  if ( isBinarySpatialOperator( fd->name() ) )
  {
    QList<QgsExpression::Node*> argNodes = node->args()->list();
    Q_ASSERT( argNodes.count() == 2 ); // binary spatial ops must have two args

    QgsExpression::Node* otherNode = 0;
    if ( isGeometryColumn( argNodes[0] ) )
      otherNode = argNodes[1];
    else if ( isGeometryColumn( argNodes[1] ) )
      otherNode = argNodes[0];
    else
    {
      errorMessage = QString( "Unable to translate spatial operator: at least one must refer to geometry." );
      return QDomElement();
    }

    QDomElement otherGeomElem;

    // the other node must be a geometry constructor
    if ( otherNode->nodeType() != QgsExpression::ntFunction )
    {
      errorMessage = "spatial operator: the other operator must be a geometry constructor function";
      return QDomElement();
    }

    const QgsExpression::NodeFunction* otherFn = static_cast<const QgsExpression::NodeFunction*>( otherNode );
    QgsExpression::Function* otherFnDef = QgsExpression::Functions()[otherFn->fnIndex()];
    if ( otherFnDef->name() == "geom_from_wkt" )
    {
      QgsExpression::Node* firstFnArg = otherFn->args()->list()[0];
      if ( firstFnArg->nodeType() != QgsExpression::ntLiteral )
      {
        errorMessage = "geom_from_wkt: argument must be string literal";
        return QDomElement();
      }
      QString wkt = static_cast<const QgsExpression::NodeLiteral*>( firstFnArg )->value().toString();
      QgsGeometry* geom = QgsGeometry::fromWkt( wkt );
      otherGeomElem = QgsOgcUtils::geometryToGML( geom, doc );
      delete geom;
    }
    else if ( otherFnDef->name() == "geom_from_gml" )
    {
      QgsExpression::Node* firstFnArg = otherFn->args()->list()[0];
      if ( firstFnArg->nodeType() != QgsExpression::ntLiteral )
      {
        errorMessage = "geom_from_gml: argument must be string literal";
        return QDomElement();
      }

      QDomDocument geomDoc;
      QString gml = static_cast<const QgsExpression::NodeLiteral*>( firstFnArg )->value().toString();
      if ( !geomDoc.setContent( gml, true ) )
      {
        errorMessage = "geom_from_gml: unable to parse XML";
        return QDomElement();
      }

      QDomNode geomNode = doc.importNode( geomDoc.documentElement(), true );
      otherGeomElem = geomNode.toElement();
    }
    else
    {
      errorMessage = "spatial operator: unknown geometry constructor function";
      return QDomElement();
    }

    QDomElement funcElem = doc.createElement( "ogc:" + tagNameForSpatialOperator( fd->name() ) );
    QDomElement geomProperty = doc.createElement( "ogc:PropertyName" );
    geomProperty.appendChild( doc.createTextNode( "geometry" ) );
    funcElem.appendChild( geomProperty );
    funcElem.appendChild( otherGeomElem );
    return funcElem;
  }

  if ( fd->params() == 0 )
  {
    errorMessage = QString( "Special columns / constants are not supported." );
    return QDomElement();
  }

  // this is somehow wrong - we are just hoping that the other side supports the same functions as we do...
  QDomElement funcElem = doc.createElement( "ogc:Function" );
  funcElem.setAttribute( "name", fd->name() );
  Q_FOREACH ( QgsExpression::Node* n, node->args()->list() )
  {
    QDomElement childElem = expressionNodeToOgcFilter( n, doc, errorMessage );
    if ( !errorMessage.isEmpty() )
      return QDomElement();

    funcElem.appendChild( childElem );
  }

  return funcElem;
}
