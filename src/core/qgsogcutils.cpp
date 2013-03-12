#include "qgsogcutils.h"

#include "qgsgeometry.h"

#include <QStringList>

#ifndef Q_WS_WIN
#include <netinet/in.h>
#else
#include <winsock.h>
#endif


static const QString GML_NAMESPACE = "http://www.opengis.net/gml";

QgsGeometry* QgsOgcUtils::geometryFromGML2( const QDomNode& geometryNode )
{
  QDomElement geometryTypeElement = geometryNode.toElement();
  QString geomType = geometryTypeElement.tagName();

  if ( !( geomType == "Point" || geomType == "LineString" || geomType == "Polygon" || geomType == "MultiPoint" || geomType == "MultiLineString" || geomType == "MultiPolygon" || geomType == "Box" ) )
  {
    QDomNode geometryChild = geometryNode.firstChild();
    if ( geometryChild.isNull() )
    {
      return 0;
    }
    geometryTypeElement = geometryChild.toElement();
    geomType = geometryTypeElement.tagName();
  }

  if ( !( geomType == "Point" || geomType == "LineString" || geomType == "Polygon" || geomType == "MultiPoint" || geomType == "MultiLineString" || geomType == "MultiPolygon" || geomType == "Box" ) )
    return 0;

  if ( geomType == "Point" )
  {
    return geometryFromGML2Point( geometryTypeElement );
  }
  else if ( geomType == "LineString" )
  {
    return geometryFromGML2LineString( geometryTypeElement );
  }
  else if ( geomType == "Polygon" )
  {
    return geometryFromGML2Polygon( geometryTypeElement );
  }
  else if ( geomType == "MultiPoint" )
  {
    return geometryFromGML2MultiPoint( geometryTypeElement );
  }
  else if ( geomType == "MultiLineString" )
  {
    return geometryFromGML2MultiLineString( geometryTypeElement );
  }
  else if ( geomType == "MultiPolygon" )
  {
    return geometryFromGML2MultiPolygon( geometryTypeElement );
  }
  else if ( geomType == "Box" )
  {
    return QgsGeometry::fromRect( rectangleFromGMLBox( geometryTypeElement ) );
  }
  else //unknown type
  {
    return 0;
  }
}

QgsGeometry* QgsOgcUtils::geometryFromGML2( const QString& xmlString )
{
  // wrap the string into a root tag to have "gml" namespace (and also as a default namespace)
  QString xml = QString( "<tmp xmlns=\"%1\" xmlns:gml=\"%1\">%2</tmp>").arg( GML_NAMESPACE ).arg( xmlString );
  QDomDocument doc;
  if ( !doc.setContent( xml, true ) )
    return 0;

  return geometryFromGML2( doc.documentElement().firstChildElement() );
}


QgsGeometry* QgsOgcUtils::geometryFromGML2Point( const QDomElement& geometryElement )
{
  QDomNodeList coordList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "coordinates" );
  if ( coordList.size() < 1 )
  {
    return 0;
  }
  QDomElement coordElement = coordList.at( 0 ).toElement();
  std::list<QgsPoint> pointCoordinate;
  if ( readGML2Coordinates( pointCoordinate, coordElement ) != 0 )
  {
    return 0;
  }

  if ( pointCoordinate.size() < 1 )
  {
    return 0;
  }

  std::list<QgsPoint>::const_iterator point_it = pointCoordinate.begin();
  //char e = QgsApplication::endian();
  char e = ( htonl( 1 ) == 1 ) ? 0 : 1 ;
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

QgsGeometry* QgsOgcUtils::geometryFromGML2LineString( const QDomElement& geometryElement )
{
  QDomNodeList coordinatesList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "coordinates" );
  if ( coordinatesList.size() < 1 )
  {
    return 0;
  }
  QDomElement coordinatesElement = coordinatesList.at( 0 ).toElement();
  std::list<QgsPoint> lineCoordinates;
  if ( readGML2Coordinates( lineCoordinates, coordinatesElement ) != 0 )
  {
    return 0;
  }

  //char e = QgsApplication::endian();
  char e = ( htonl( 1 ) == 1 ) ? 0 : 1 ;
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

  std::list<QgsPoint>::const_iterator iter;
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

QgsGeometry* QgsOgcUtils::geometryFromGML2Polygon( const QDomElement& geometryElement )
{
  //read all the coordinates (as QgsPoint) into memory. Each linear ring has an entry in the vector
  std::vector<std::list<QgsPoint> > ringCoordinates;

  //read coordinates for outer boundary
  QDomNodeList outerBoundaryList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "outerBoundaryIs" );
  if ( outerBoundaryList.size() < 1 ) //outer ring is necessary
  {
    return 0;
  }
  QDomElement coordinatesElement = outerBoundaryList.at( 0 ).firstChild().firstChild().toElement();
  if ( coordinatesElement.isNull() )
  {
    return 0;
  }
  std::list<QgsPoint> exteriorPointList;
  if ( readGML2Coordinates( exteriorPointList, coordinatesElement ) != 0 )
  {
    return 0;
  }
  ringCoordinates.push_back( exteriorPointList );

  //read coordinates for inner boundary
  QDomNodeList innerBoundaryList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "innerBoundaryIs" );
  for ( int i = 0; i < innerBoundaryList.size(); ++i )
  {
    std::list<QgsPoint> interiorPointList;
    QDomElement coordinatesElement = innerBoundaryList.at( i ).firstChild().firstChild().toElement();
    if ( coordinatesElement.isNull() )
    {
      return 0;
    }
    if ( readGML2Coordinates( interiorPointList, coordinatesElement ) != 0 )
    {
      return 0;
    }
    ringCoordinates.push_back( interiorPointList );
  }

  //calculate number of bytes to allocate
  int nrings = ringCoordinates.size();
  int npoints = 0;//total number of points
  for ( std::vector<std::list<QgsPoint> >::const_iterator it = ringCoordinates.begin(); it != ringCoordinates.end(); ++it )
  {
    npoints += it->size();
  }
  int size = 1 + 2 * sizeof( int ) + nrings * sizeof( int ) + 2 * npoints * sizeof( double );

  QGis::WkbType type = QGis::WKBPolygon;
  unsigned char* wkb = new unsigned char[size];

  //char e = QgsApplication::endian();
  char e = ( htonl( 1 ) == 1 ) ? 0 : 1 ;
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
  for ( std::vector<std::list<QgsPoint> >::const_iterator it = ringCoordinates.begin(); it != ringCoordinates.end(); ++it )
  {
    nPointsInRing = it->size();
    memcpy( &( wkb )[wkbPosition], &nPointsInRing, sizeof( int ) );
    wkbPosition += sizeof( int );
    //iterate through the string list converting the strings to x-/y- doubles
    std::list<QgsPoint>::const_iterator iter;
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

QgsGeometry* QgsOgcUtils::geometryFromGML2MultiPoint( const QDomElement& geometryElement )
{
  std::list<QgsPoint> pointList;
  std::list<QgsPoint> currentPoint;
  QDomNodeList pointMemberList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "pointMember" );
  if ( pointMemberList.size() < 1 )
  {
    return 0;
  }
  QDomNodeList pointNodeList;
  QDomNodeList coordinatesList;
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
    if ( coordinatesList.size() < 1 )
    {
      continue;
    }
    currentPoint.clear();
    if ( readGML2Coordinates( currentPoint, coordinatesList.at( 0 ).toElement() ) != 0 )
    {
      continue;
    }
    if ( currentPoint.size() < 1 )
    {
      continue;
    }
    pointList.push_back(( *currentPoint.begin() ) );
  }

  //calculate the required wkb size
  int size = 1 + 2 * sizeof( int ) + pointList.size() * ( 2 * sizeof( double ) + 1 + sizeof( int ) );

  QGis::WkbType type = QGis::WKBMultiPoint;
  unsigned char* wkb = new unsigned char[size];

  //fill the wkb content
  //char e = QgsApplication::endian();
  char e = ( htonl( 1 ) == 1 ) ? 0 : 1 ;
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  int nPoints = pointList.size(); //number of points
  double x, y;
  memcpy( &( wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( wkb )[wkbPosition], &nPoints, sizeof( int ) );
  wkbPosition += sizeof( int );
  for ( std::list<QgsPoint>::const_iterator it = pointList.begin(); it != pointList.end(); ++it )
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

QgsGeometry* QgsOgcUtils::geometryFromGML2MultiLineString( const QDomElement& geometryElement )
{
  //geoserver has
  //<gml:MultiLineString>
  //<gml:lineStringMember>
  //<gml:LineString>

  //mapserver has directly
  //<gml:MultiLineString
  //<gml:LineString

  std::list<std::list<QgsPoint> > lineCoordinates; //first list: lines, second list: points of one line
  QDomElement currentLineStringElement;
  QDomNodeList currentCoordList;

  QDomNodeList lineStringMemberList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "lineStringMember" );
  if ( lineStringMemberList.size() > 0 ) //geoserver
  {
    for ( int i = 0; i < lineStringMemberList.size(); ++i )
    {
      QDomNodeList lineStringNodeList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "LineString" );
      if ( lineStringNodeList.size() < 1 )
      {
        return 0;
      }
      currentLineStringElement = lineStringNodeList.at( 0 ).toElement();
      currentCoordList = currentLineStringElement.elementsByTagNameNS( GML_NAMESPACE, "coordinates" );
      if ( currentCoordList.size() < 1 )
      {
        return 0;
      }
      std::list<QgsPoint> currentPointList;
      if ( readGML2Coordinates( currentPointList, currentCoordList.at( 0 ).toElement() ) != 0 )
      {
        return 0;
      }
      lineCoordinates.push_back( currentPointList );
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
        if ( currentCoordList.size() < 1 )
        {
          return 0;
        }
        std::list<QgsPoint> currentPointList;
        if ( readGML2Coordinates( currentPointList, currentCoordList.at( 0 ).toElement() ) != 0 )
        {
          return 0;
        }
        lineCoordinates.push_back( currentPointList );
      }
    }
    else
    {
      return 0;
    }
  }


  //calculate the required wkb size
  int size = ( lineCoordinates.size() + 1 ) * ( 1 + 2 * sizeof( int ) );
  for ( std::list<std::list<QgsPoint> >::const_iterator it = lineCoordinates.begin(); it != lineCoordinates.end(); ++it )
  {
    size += it->size() * 2 * sizeof( double );
  }

  QGis::WkbType type = QGis::WKBMultiLineString;
  unsigned char* wkb = new unsigned char[size];

  //fill the wkb content
  //char e = QgsApplication::endian();
  char e = ( htonl( 1 ) == 1 ) ? 0 : 1 ;
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  int nLines = lineCoordinates.size();
  int nPoints; //number of points in a line
  double x, y;
  memcpy( &( wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( wkb )[wkbPosition], &nLines, sizeof( int ) );
  wkbPosition += sizeof( int );
  for ( std::list<std::list<QgsPoint> >::const_iterator it = lineCoordinates.begin(); it != lineCoordinates.end(); ++it )
  {
    memcpy( &( wkb )[wkbPosition], &e, 1 );
    wkbPosition += 1;
    memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
    wkbPosition += sizeof( int );
    nPoints = it->size();
    memcpy( &( wkb )[wkbPosition], &nPoints, sizeof( int ) );
    wkbPosition += sizeof( int );
    for ( std::list<QgsPoint>::const_iterator iter = it->begin(); iter != it->end(); ++iter )
    {
      x = iter->x();
      //qWarning("x is: " + QString::number(x));
      y = iter->y();
      //qWarning("y is: " + QString::number(y));
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

QgsGeometry* QgsOgcUtils::geometryFromGML2MultiPolygon( const QDomElement& geometryElement )
{
  //first list: different polygons, second list: different rings, third list: different points
  std::list<std::list<std::list<QgsPoint> > > multiPolygonPoints;
  QDomElement currentPolygonMemberElement;
  QDomNodeList polygonList;
  QDomElement currentPolygonElement;
  QDomNodeList outerBoundaryList;
  QDomElement currentOuterBoundaryElement;
  QDomElement currentInnerBoundaryElement;
  QDomNodeList innerBoundaryList;
  QDomNodeList linearRingNodeList;
  QDomElement currentLinearRingElement;
  QDomNodeList currentCoordinateList;

  QDomNodeList polygonMemberList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "polygonMember" );
  for ( int i = 0; i < polygonMemberList.size(); ++i )
  {
    std::list<std::list<QgsPoint> > currentPolygonList;
    currentPolygonMemberElement = polygonMemberList.at( i ).toElement();
    polygonList = currentPolygonMemberElement.elementsByTagNameNS( GML_NAMESPACE, "Polygon" );
    if ( polygonList.size() < 1 )
    {
      continue;
    }
    currentPolygonElement = polygonList.at( 0 ).toElement();

    //find exterior ring
    outerBoundaryList = currentPolygonElement.elementsByTagNameNS( GML_NAMESPACE, "outerBoundaryIs" );
    if ( outerBoundaryList.size() < 1 )
    {
      continue;
    }

    currentOuterBoundaryElement = outerBoundaryList.at( 0 ).toElement();
    std::list<QgsPoint> ringCoordinates;

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
    if ( readGML2Coordinates( ringCoordinates, currentCoordinateList.at( 0 ).toElement() ) != 0 )
    {
      continue;
    }
    currentPolygonList.push_back( ringCoordinates );

    //find interior rings
    QDomNodeList innerBoundaryList = currentPolygonElement.elementsByTagNameNS( GML_NAMESPACE, "innerBoundaryIs" );
    for ( int j = 0; j < innerBoundaryList.size(); ++j )
    {
      std::list<QgsPoint> ringCoordinates;
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
      if ( readGML2Coordinates( ringCoordinates, currentCoordinateList.at( 0 ).toElement() ) != 0 )
      {
        continue;
      }
      currentPolygonList.push_back( ringCoordinates );
    }
    multiPolygonPoints.push_back( currentPolygonList );
  }

  int size = 1 + 2 * sizeof( int );
  //calculate the wkb size
  for ( std::list<std::list<std::list<QgsPoint> > >::const_iterator it = multiPolygonPoints.begin(); it != multiPolygonPoints.end(); ++it )
  {
    size += 1 + 2 * sizeof( int );
    for ( std::list<std::list<QgsPoint> >::const_iterator iter = it->begin(); iter != it->end(); ++iter )
    {
      size += sizeof( int ) + 2 * iter->size() * sizeof( double );
    }
  }

  QGis::WkbType type = QGis::WKBMultiPolygon;
  unsigned char* wkb = new unsigned char[size];

  int polygonType = QGis::WKBPolygon;
  //char e = QgsApplication::endian();
  char e = ( htonl( 1 ) == 1 ) ? 0 : 1 ;
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  double x, y;
  int nPolygons = multiPolygonPoints.size();
  int nRings;
  int nPointsInRing;

  //fill the contents into *wkb
  memcpy( &( wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( wkb )[wkbPosition], &nPolygons, sizeof( int ) );
  wkbPosition += sizeof( int );

  for ( std::list<std::list<std::list<QgsPoint> > >::const_iterator it = multiPolygonPoints.begin(); it != multiPolygonPoints.end(); ++it )
  {
    memcpy( &( wkb )[wkbPosition], &e, 1 );
    wkbPosition += 1;
    memcpy( &( wkb )[wkbPosition], &polygonType, sizeof( int ) );
    wkbPosition += sizeof( int );
    nRings = it->size();
    memcpy( &( wkb )[wkbPosition], &nRings, sizeof( int ) );
    wkbPosition += sizeof( int );
    for ( std::list<std::list<QgsPoint> >::const_iterator iter = it->begin(); iter != it->end(); ++iter )
    {
      nPointsInRing = iter->size();
      memcpy( &( wkb )[wkbPosition], &nPointsInRing, sizeof( int ) );
      wkbPosition += sizeof( int );
      for ( std::list<QgsPoint>::const_iterator iterator = iter->begin(); iterator != iter->end(); ++iterator )
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

bool QgsOgcUtils::readGML2Coordinates( std::list<QgsPoint>& coords, const QDomElement elem )
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




QDomElement QgsOgcUtils::geometryToGML2( QgsGeometry* geometry, QDomDocument& doc )
{
  if ( !geometry || !geometry->asWkb() )
    return QDomElement();

  bool hasZValue = false;
  double *x, *y;  
  unsigned char* wkb = geometry->asWkb();

  switch ( geometry->wkbType() )
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
    {
      QDomElement pointElem = doc.createElement( "gml:Point" );
      QDomElement coordElem = doc.createElement( "gml:coordinates" );
      coordElem.setAttribute( "cs", "," );
      coordElem.setAttribute( "ts", " " );
      QString coordString;
      x = ( double * )( wkb + 5 );
      coordString += QString::number( *x, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
      coordString += ",";
      y = ( double * )( wkb + 5 + sizeof( double ) );
      coordString += QString::number( *y, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
      QDomText coordText = doc.createTextNode( coordString );
      coordElem.appendChild( coordText );
      pointElem.appendChild( coordElem );
      return pointElem;
    }
    case QGis::WKBMultiPoint25D:
      hasZValue = true;
    case QGis::WKBMultiPoint:
    {
      unsigned char *ptr;
      int idx;
      int *nPoints;

      QDomElement multiPointElem = doc.createElement( "gml:MultiPoint" );
      nPoints = ( int* )( wkb + 5 );
      ptr = wkb + 5 + sizeof( int );
      for ( idx = 0; idx < *nPoints; ++idx )
      {
        ptr += ( 1 + sizeof( int ) );
        QDomElement pointMemberElem = doc.createElement( "gml:pointMember" );
        QDomElement pointElem = doc.createElement( "gml:Point" );
        QDomElement coordElem = doc.createElement( "gml:coordinates" );
        coordElem.setAttribute( "cs", "," );
        coordElem.setAttribute( "ts", " " );
        QString coordString;
        x = ( double * )( ptr );
        coordString += QString::number( *x, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
        coordString += ",";
        ptr += sizeof( double );
        y = ( double * )( ptr );
        coordString += QString::number( *y, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
        QDomText coordText = doc.createTextNode( coordString );
        coordElem.appendChild( coordText );
        pointElem.appendChild( coordElem );

        ptr += sizeof( double );
        if ( hasZValue )
        {
          ptr += sizeof( double );
        }
        pointMemberElem.appendChild( pointElem );
        multiPointElem.appendChild( pointMemberElem );
      }
      return multiPointElem;
    }
    case QGis::WKBLineString25D:
      hasZValue = true;
    case QGis::WKBLineString:
    {
      unsigned char *ptr;
      int *nPoints;
      int idx;

      QDomElement lineStringElem = doc.createElement( "gml:LineString" );
      // get number of points in the line
      ptr = wkb + 5;
      nPoints = ( int * ) ptr;
      ptr = wkb + 1 + 2 * sizeof( int );
      QDomElement coordElem = doc.createElement( "gml:coordinates" );
      coordElem.setAttribute( "cs", "," );
      coordElem.setAttribute( "ts", " " );
      QString coordString;
      for ( idx = 0; idx < *nPoints; ++idx )
      {
        if ( idx != 0 )
        {
          coordString += " ";
        }
        x = ( double * ) ptr;
        coordString += QString::number( *x, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
        coordString += ",";
        ptr += sizeof( double );
        y = ( double * ) ptr;
        coordString += QString::number( *y, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
        ptr += sizeof( double );
        if ( hasZValue )
        {
          ptr += sizeof( double );
        }
      }
      QDomText coordText = doc.createTextNode( coordString );
      coordElem.appendChild( coordText );
      lineStringElem.appendChild( coordElem );
      return lineStringElem;
    }
    case QGis::WKBMultiLineString25D:
      hasZValue = true;
    case QGis::WKBMultiLineString:
    {
      unsigned char *ptr;
      int idx, jdx, numLineStrings;
      int *nPoints;

      QDomElement multiLineStringElem = doc.createElement( "gml:MultiLineString" );
      numLineStrings = ( int )( wkb[5] );
      ptr = wkb + 9;
      for ( jdx = 0; jdx < numLineStrings; jdx++ )
      {
        QDomElement lineStringMemberElem = doc.createElement( "gml:lineStringMember" );
        QDomElement lineStringElem = doc.createElement( "gml:LineString" );
        ptr += 5; // skip type since we know its 2
        nPoints = ( int * ) ptr;
        ptr += sizeof( int );
        QDomElement coordElem = doc.createElement( "gml:coordinates" );
        coordElem.setAttribute( "cs", "," );
        coordElem.setAttribute( "ts", " " );
        QString coordString;
        for ( idx = 0; idx < *nPoints; idx++ )
        {
          if ( idx != 0 )
          {
            coordString += " ";
          }
          x = ( double * ) ptr;
          coordString += QString::number( *x, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
          ptr += sizeof( double );
          coordString += ",";
          y = ( double * ) ptr;
          coordString += QString::number( *y, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
          ptr += sizeof( double );
          if ( hasZValue )
          {
            ptr += sizeof( double );
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
    case QGis::WKBPolygon:
    {
      unsigned char *ptr;
      int idx, jdx;
      int *numRings, *nPoints;

      QDomElement polygonElem = doc.createElement( "gml:Polygon" );
      // get number of rings in the polygon
      numRings = ( int * )( wkb + 1 + sizeof( int ) );
      if ( !( *numRings ) )  // sanity check for zero rings in polygon
      {
        return QDomElement();
      }
      int *ringStart; // index of first point for each ring
      int *ringNumPoints; // number of points in each ring
      ringStart = new int[*numRings];
      ringNumPoints = new int[*numRings];
      ptr = wkb + 1 + 2 * sizeof( int ); // set pointer to the first ring
      for ( idx = 0; idx < *numRings; idx++ )
      {
        QString boundaryName = "gml:outerBoundaryIs";
        if ( idx != 0 )
        {
          boundaryName = "gml:innerBoundaryIs";
        }
        QDomElement boundaryElem = doc.createElement( boundaryName );
        QDomElement ringElem = doc.createElement( "gml:LinearRing" );
        // get number of points in the ring
        nPoints = ( int * ) ptr;
        ringNumPoints[idx] = *nPoints;
        ptr += 4;
        QDomElement coordElem = doc.createElement( "gml:coordinates" );
        coordElem.setAttribute( "cs", "," );
        coordElem.setAttribute( "ts", " " );
        QString coordString;
        for ( jdx = 0; jdx < *nPoints; jdx++ )
        {
          if ( jdx != 0 )
          {
            coordString += " ";
          }
          x = ( double * ) ptr;
          coordString += QString::number( *x, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
          coordString += ",";
          ptr += sizeof( double );
          y = ( double * ) ptr;
          coordString += QString::number( *y, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
          ptr += sizeof( double );
          if ( hasZValue )
          {
            ptr += sizeof( double );
          }
        }
        QDomText coordText = doc.createTextNode( coordString );
        coordElem.appendChild( coordText );
        ringElem.appendChild( coordElem );
        boundaryElem.appendChild( ringElem );
        polygonElem.appendChild( boundaryElem );
      }
      delete [] ringStart;
      delete [] ringNumPoints;
      return polygonElem;
    }
    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
    case QGis::WKBMultiPolygon:
    {
      unsigned char *ptr;
      int idx, jdx, kdx;
      int *numPolygons, *numRings, *nPoints;

      QDomElement multiPolygonElem = doc.createElement( "gml:MultiPolygon" );
      ptr = wkb + 5;
      numPolygons = ( int * ) ptr;
      ptr = wkb + 9;
      for ( kdx = 0; kdx < *numPolygons; kdx++ )
      {
        QDomElement polygonMemberElem = doc.createElement( "gml:polygonMember" );
        QDomElement polygonElem = doc.createElement( "gml:Polygon" );
        ptr += 5;
        numRings = ( int * ) ptr;
        ptr += 4;
        for ( idx = 0; idx < *numRings; idx++ )
        {
          QString boundaryName = "gml:outerBoundaryIs";
          if ( idx != 0 )
          {
            boundaryName = "gml:innerBoundaryIs";
          }
          QDomElement boundaryElem = doc.createElement( boundaryName );
          QDomElement ringElem = doc.createElement( "gml:LinearRing" );
          nPoints = ( int * ) ptr;
          ptr += 4;
          QDomElement coordElem = doc.createElement( "gml:coordinates" );
          coordElem.setAttribute( "cs", "," );
          coordElem.setAttribute( "ts", " " );
          QString coordString;
          for ( jdx = 0; jdx < *nPoints; jdx++ )
          {
            if ( jdx != 0 )
            {
              coordString += " ";
            }
            x = ( double * ) ptr;
            coordString += QString::number( *x, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
            ptr += sizeof( double );
            coordString += ",";
            y = ( double * ) ptr;
            coordString += QString::number( *y, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
            ptr += sizeof( double );
            if ( hasZValue )
            {
              ptr += sizeof( double );
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


QgsGeometry* QgsOgcUtils::geometryFromGML3( const QDomNode& geometryNode )
{
  QDomElement geometryTypeElement = geometryNode.toElement();
  QString geomType = geometryTypeElement.tagName();

  if ( !( geomType == "Point" || geomType == "LineString" || geomType == "Polygon" || geomType == "MultiPoint" || geomType == "MultiLineString" || geomType == "MultiPolygon" || geomType == "Envelope" ) )
  {
    QDomNode geometryChild = geometryNode.firstChild();
    if ( geometryChild.isNull() )
    {
      return 0;
    }
    geometryTypeElement = geometryChild.toElement();
    geomType = geometryTypeElement.tagName();
  }

  if ( !( geomType == "Point" || geomType == "LineString" || geomType == "Polygon" || geomType == "MultiPoint" || geomType == "MultiLineString" || geomType == "MultiPolygon" || geomType == "Envelope" ) )
    return 0;

  if ( geomType == "Point" )
  {
    return geometryFromGML3Point( geometryTypeElement );
  }
  else if ( geomType == "LineString" )
  {
    return geometryFromGML3LineString( geometryTypeElement );
  }
  else if ( geomType == "Polygon" )
  {
    return geometryFromGML3Polygon( geometryTypeElement );
  }
  else if ( geomType == "MultiPoint" )
  {
    return geometryFromGML3MultiPoint( geometryTypeElement );
  }
  else if ( geomType == "MultiLineString" )
  {
    return geometryFromGML3MultiLineString( geometryTypeElement );
  }
  else if ( geomType == "MultiPolygon" )
  {
    return geometryFromGML3MultiPolygon( geometryTypeElement );
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

QgsGeometry* QgsOgcUtils::geometryFromGML3( const QString& xmlString )
{
  // wrap the string into a root tag to have "gml" namespace (and also as a default namespace)
  QString xml = QString( "<tmp xmlns=\"%1\" xmlns:gml=\"%1\">%2</tmp>").arg( GML_NAMESPACE ).arg( xmlString );
  QDomDocument doc;
  if ( !doc.setContent( xml, true ) )
    return 0;

  return geometryFromGML3( doc.documentElement().firstChildElement() );
}


QDomElement QgsOgcUtils::geometryToGML3( QgsGeometry* geometry, QDomDocument& doc )
{
  if ( !geometry || !geometry->asWkb() )
    return QDomElement();

  bool hasZValue = false;
  double *x, *y;  
  unsigned char* wkb = geometry->asWkb();

  switch ( geometry->wkbType() )
  {
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
    {
      QDomElement pointElem = doc.createElement( "gml:Point" );
      QDomElement coordElem = doc.createElement( "gml:pos" );
      coordElem.setAttribute( "srsDimension", "2" );
      QString coordString;
      x = ( double * )( wkb + 5 );
      coordString += QString::number( *x, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
      coordString += " ";
      y = ( double * )( wkb + 5 + sizeof( double ) );
      coordString += QString::number( *y, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
      QDomText coordText = doc.createTextNode( coordString );
      coordElem.appendChild( coordText );
      pointElem.appendChild( coordElem );
      return pointElem;
    }
    case QGis::WKBMultiPoint25D:
      hasZValue = true;
    case QGis::WKBMultiPoint:
    {
      unsigned char *ptr;
      int idx;
      int *nPoints;

      QDomElement multiPointElem = doc.createElement( "gml:MultiPoint" );
      nPoints = ( int* )( wkb + 5 );
      ptr = wkb + 5 + sizeof( int );
      for ( idx = 0; idx < *nPoints; ++idx )
      {
        ptr += ( 1 + sizeof( int ) );
        QDomElement pointMemberElem = doc.createElement( "gml:pointMember" );
        QDomElement pointElem = doc.createElement( "gml:Point" );
        QDomElement coordElem = doc.createElement( "gml:pos" );
        coordElem.setAttribute( "srsDimension", "2" );
        QString coordString;
        x = ( double * )( ptr );
        coordString += QString::number( *x, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
        coordString += " ";
        ptr += sizeof( double );
        y = ( double * )( ptr );
        coordString += QString::number( *y, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
        QDomText coordText = doc.createTextNode( coordString );
        coordElem.appendChild( coordText );
        pointElem.appendChild( coordElem );

        ptr += sizeof( double );
        if ( hasZValue )
        {
          ptr += sizeof( double );
        }
        pointMemberElem.appendChild( pointElem );
        multiPointElem.appendChild( pointMemberElem );
      }
      return multiPointElem;
    }
    case QGis::WKBLineString25D:
      hasZValue = true;
    case QGis::WKBLineString:
    {
      unsigned char *ptr;
      int *nPoints;
      int idx;

      QDomElement lineStringElem = doc.createElement( "gml:LineString" );
      // get number of points in the line
      ptr = wkb + 5;
      nPoints = ( int * ) ptr;
      ptr = wkb + 1 + 2 * sizeof( int );
      QDomElement coordElem = doc.createElement( "gml:posList" );
      coordElem.setAttribute( "srsDimension", "2" );
      QString coordString;
      for ( idx = 0; idx < *nPoints; ++idx )
      {
        if ( idx != 0 )
        {
          coordString += " ";
        }
        x = ( double * ) ptr;
        coordString += QString::number( *x, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
        coordString += " ";
        ptr += sizeof( double );
        y = ( double * ) ptr;
        coordString += QString::number( *y, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
        ptr += sizeof( double );
        if ( hasZValue )
        {
          ptr += sizeof( double );
        }
      }
      QDomText coordText = doc.createTextNode( coordString );
      coordElem.appendChild( coordText );
      lineStringElem.appendChild( coordElem );
      return lineStringElem;
    }
    case QGis::WKBMultiLineString25D:
      hasZValue = true;
    case QGis::WKBMultiLineString:
    {
      unsigned char *ptr;
      int idx, jdx, numLineStrings;
      int *nPoints;

      QDomElement multiLineStringElem = doc.createElement( "gml:MultiLineString" );
      numLineStrings = ( int )( wkb[5] );
      ptr = wkb + 9;
      for ( jdx = 0; jdx < numLineStrings; jdx++ )
      {
        QDomElement lineStringMemberElem = doc.createElement( "gml:lineStringMember" );
        QDomElement lineStringElem = doc.createElement( "gml:LineString" );
        ptr += 5; // skip type since we know its 2
        nPoints = ( int * ) ptr;
        ptr += sizeof( int );
        QDomElement coordElem = doc.createElement( "gml:posList" );
        coordElem.setAttribute( "srsDimension", "2" );
        QString coordString;
        for ( idx = 0; idx < *nPoints; idx++ )
        {
          if ( idx != 0 )
          {
            coordString += " ";
          }
          x = ( double * ) ptr;
          coordString += QString::number( *x, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
          ptr += sizeof( double );
          coordString += " ";
          y = ( double * ) ptr;
          coordString += QString::number( *y, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
          ptr += sizeof( double );
          if ( hasZValue )
          {
            ptr += sizeof( double );
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
    case QGis::WKBPolygon:
    {
      unsigned char *ptr;
      int idx, jdx;
      int *numRings, *nPoints;

      QDomElement polygonElem = doc.createElement( "gml:Polygon" );
      // get number of rings in the polygon
      numRings = ( int * )( wkb + 1 + sizeof( int ) );
      if ( !( *numRings ) )  // sanity check for zero rings in polygon
      {
        return QDomElement();
      }
      int *ringStart; // index of first point for each ring
      int *ringNumPoints; // number of points in each ring
      ringStart = new int[*numRings];
      ringNumPoints = new int[*numRings];
      ptr = wkb + 1 + 2 * sizeof( int ); // set pointer to the first ring
      for ( idx = 0; idx < *numRings; idx++ )
      {
        QString boundaryName = "gml:exterior";
        if ( idx != 0 )
        {
          boundaryName = "gml:interior";
        }
        QDomElement boundaryElem = doc.createElement( boundaryName );
        QDomElement ringElem = doc.createElement( "gml:LinearRing" );
        // get number of points in the ring
        nPoints = ( int * ) ptr;
        ringNumPoints[idx] = *nPoints;
        ptr += 4;
        QDomElement coordElem = doc.createElement( "gml:posList" );
        coordElem.setAttribute( "srsDimension", "2" );
        QString coordString;
        for ( jdx = 0; jdx < *nPoints; jdx++ )
        {
          if ( jdx != 0 )
          {
            coordString += " ";
          }
          x = ( double * ) ptr;
          coordString += QString::number( *x, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
          coordString += " ";
          ptr += sizeof( double );
          y = ( double * ) ptr;
          coordString += QString::number( *y, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
          ptr += sizeof( double );
          if ( hasZValue )
          {
            ptr += sizeof( double );
          }
        }
        QDomText coordText = doc.createTextNode( coordString );
        coordElem.appendChild( coordText );
        ringElem.appendChild( coordElem );
        boundaryElem.appendChild( ringElem );
        polygonElem.appendChild( boundaryElem );
      }
      delete [] ringStart;
      delete [] ringNumPoints;
      return polygonElem;
    }
    case QGis::WKBMultiPolygon25D:
      hasZValue = true;
    case QGis::WKBMultiPolygon:
    {
      unsigned char *ptr;
      int idx, jdx, kdx;
      int *numPolygons, *numRings, *nPoints;

      QDomElement multiPolygonElem = doc.createElement( "gml:MultiPolygon" );
      ptr = wkb + 5;
      numPolygons = ( int * ) ptr;
      ptr = wkb + 9;
      for ( kdx = 0; kdx < *numPolygons; kdx++ )
      {
        QDomElement polygonMemberElem = doc.createElement( "gml:polygonMember" );
        QDomElement polygonElem = doc.createElement( "gml:Polygon" );
        ptr += 5;
        numRings = ( int * ) ptr;
        ptr += 4;
        for ( idx = 0; idx < *numRings; idx++ )
        {
          QString boundaryName = "gml:exterior";
          if ( idx != 0 )
          {
            boundaryName = "gml:interior";
          }
          QDomElement boundaryElem = doc.createElement( boundaryName );
          QDomElement ringElem = doc.createElement( "gml:LinearRing" );
          nPoints = ( int * ) ptr;
          ptr += 4;
          QDomElement coordElem = doc.createElement( "gml:posList" );
          coordElem.setAttribute( "srsDimension", "2" );
          QString coordString;
          for ( jdx = 0; jdx < *nPoints; jdx++ )
          {
            if ( jdx != 0 )
            {
              coordString += " ";
            }
            x = ( double * ) ptr;
            coordString += QString::number( *x, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
            ptr += sizeof( double );
            coordString += " ";
            y = ( double * ) ptr;
            coordString += QString::number( *y, 'f', 8 ).remove( QRegExp( "[0]{1,7}$" ) );
            ptr += sizeof( double );
            if ( hasZValue )
            {
              ptr += sizeof( double );
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


QgsGeometry* QgsOgcUtils::geometryFromGML3Point( const QDomElement& geometryElement )
{
  QDomNodeList posList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "pos" );
  if ( posList.size() < 1 )
  {
    return 0;
  }
  QDomElement posElement = posList.at( 0 ).toElement();
  std::list<QgsPoint> pointPosition;
  if ( readGML3Positions( pointPosition, posElement ) != 0 )
  {
    return 0;
  }

  if ( pointPosition.size() < 1 )
  {
    return 0;
  }

  std::list<QgsPoint>::const_iterator point_it = pointPosition.begin();
  //char e = QgsApplication::endian();
  char e = ( htonl( 1 ) == 1 ) ? 0 : 1 ;
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

QgsGeometry* QgsOgcUtils::geometryFromGML3LineString( const QDomElement& geometryElement )
{
  QDomNodeList posList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "posList" );
  if ( posList.size() < 1 )
  {
    return 0;
  }
  QDomElement posElement = posList.at( 0 ).toElement();
  std::list<QgsPoint> linePositions;
  if ( readGML2Coordinates( linePositions, posElement ) != 0 )
  {
    return 0;
  }

  //char e = QgsApplication::endian();
  char e = ( htonl( 1 ) == 1 ) ? 0 : 1 ;
  int size = 1 + 2 * sizeof( int ) + linePositions.size() * 2 * sizeof( double );

  QGis::WkbType type = QGis::WKBLineString;
  unsigned char* wkb = new unsigned char[size];

  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  double x, y;
  int nPoints = linePositions.size();

  //fill the contents into *wkb
  memcpy( &( wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( wkb )[wkbPosition], &nPoints, sizeof( int ) );
  wkbPosition += sizeof( int );

  std::list<QgsPoint>::const_iterator iter;
  for ( iter = linePositions.begin(); iter != linePositions.end(); ++iter )
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

QgsGeometry* QgsOgcUtils::geometryFromGML3Polygon( const QDomElement& geometryElement )
{
  //read all the coordinates (as QgsPoint) into memory. Each linear ring has an entry in the vector
  std::vector<std::list<QgsPoint> > ringPositions;

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
  std::list<QgsPoint> exteriorPointList;
  if ( readGML3Positions( exteriorPointList, posElement ) != 0 )
  {
    return 0;
  }
  ringPositions.push_back( exteriorPointList );

  //read coordinates for inner boundary
  QDomNodeList interiorList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "interior" );
  for ( int i = 0; i < interiorList.size(); ++i )
  {
    std::list<QgsPoint> interiorPointList;
    QDomElement posElement = interiorList.at( i ).firstChild().firstChild().toElement();
    if ( posElement.isNull() )
    {
      return 0;
    }
    if ( readGML3Positions( interiorPointList, posElement ) != 0 )
    {
      return 0;
    }
    ringPositions.push_back( interiorPointList );
  }

  //calculate number of bytes to allocate
  int nrings = ringPositions.size();
  int npoints = 0;//total number of points
  for ( std::vector<std::list<QgsPoint> >::const_iterator it = ringPositions.begin(); it != ringPositions.end(); ++it )
  {
    npoints += it->size();
  }
  int size = 1 + 2 * sizeof( int ) + nrings * sizeof( int ) + 2 * npoints * sizeof( double );

  QGis::WkbType type = QGis::WKBPolygon;
  unsigned char* wkb = new unsigned char[size];

  //char e = QgsApplication::endian();
  char e = ( htonl( 1 ) == 1 ) ? 0 : 1 ;
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
  for ( std::vector<std::list<QgsPoint> >::const_iterator it = ringPositions.begin(); it != ringPositions.end(); ++it )
  {
    nPointsInRing = it->size();
    memcpy( &( wkb )[wkbPosition], &nPointsInRing, sizeof( int ) );
    wkbPosition += sizeof( int );
    //iterate through the string list converting the strings to x-/y- doubles
    std::list<QgsPoint>::const_iterator iter;
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

QgsGeometry* QgsOgcUtils::geometryFromGML3MultiPoint( const QDomElement& geometryElement )
{
  std::list<QgsPoint> pointList;
  std::list<QgsPoint> currentPoint;
  QDomNodeList pointMemberList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "pointMember" );
  if ( pointMemberList.size() < 1 )
  {
    return 0;
  }
  QDomNodeList pointNodeList;
  QDomNodeList posList;
  for ( int i = 0; i < pointMemberList.size(); ++i )
  {
    //<Point> element
    pointNodeList = pointMemberList.at( i ).toElement().elementsByTagNameNS( GML_NAMESPACE, "Point" );
    if ( pointNodeList.size() < 1 )
    {
      continue;
    }
    //<pos> element
    posList = pointNodeList.at( 0 ).toElement().elementsByTagNameNS( GML_NAMESPACE, "pos" );
    if ( posList.size() < 1 )
    {
      continue;
    }
    currentPoint.clear();
    if ( readGML3Positions( currentPoint, posList.at( 0 ).toElement() ) != 0 )
    {
      continue;
    }
    if ( currentPoint.size() < 1 )
    {
      continue;
    }
    pointList.push_back(( *currentPoint.begin() ) );
  }

  //calculate the required wkb size
  int size = 1 + 2 * sizeof( int ) + pointList.size() * ( 2 * sizeof( double ) + 1 + sizeof( int ) );

  QGis::WkbType type = QGis::WKBMultiPoint;
  unsigned char* wkb = new unsigned char[size];

  //fill the wkb content
  //char e = QgsApplication::endian();
  char e = ( htonl( 1 ) == 1 ) ? 0 : 1 ;
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  int nPoints = pointList.size(); //number of points
  double x, y;
  memcpy( &( wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( wkb )[wkbPosition], &nPoints, sizeof( int ) );
  wkbPosition += sizeof( int );
  for ( std::list<QgsPoint>::const_iterator it = pointList.begin(); it != pointList.end(); ++it )
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

QgsGeometry* QgsOgcUtils::geometryFromGML3MultiLineString( const QDomElement& geometryElement )
{
  //geoserver has
  //<gml:MultiLineString>
  //<gml:lineStringMember>
  //<gml:LineString>

  //mapserver has directly
  //<gml:MultiLineString
  //<gml:LineString

  std::list<std::list<QgsPoint> > linePositions; //first list: lines, second list: points of one line
  QDomElement currentLineStringElement;
  QDomNodeList currentPosList;

  QDomNodeList lineStringMemberList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "lineStringMember" );
  if ( lineStringMemberList.size() > 0 ) //geoserver
  {
    for ( int i = 0; i < lineStringMemberList.size(); ++i )
    {
      QDomNodeList lineStringNodeList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "LineString" );
      if ( lineStringNodeList.size() < 1 )
      {
        return 0;
      }
      currentLineStringElement = lineStringNodeList.at( 0 ).toElement();
      currentPosList = currentLineStringElement.elementsByTagNameNS( GML_NAMESPACE, "posList" );
      if ( currentPosList.size() < 1 )
      {
        return 0;
      }
      std::list<QgsPoint> currentPointList;
      if ( readGML3Positions( currentPointList, currentPosList.at( 0 ).toElement() ) != 0 )
      {
        return 0;
      }
      linePositions.push_back( currentPointList );
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
        currentPosList = currentLineStringElement.elementsByTagNameNS( GML_NAMESPACE, "posList" );
        if ( currentPosList.size() < 1 )
        {
          return 0;
        }
        std::list<QgsPoint> currentPointList;
        if ( readGML3Positions( currentPointList, currentPosList.at( 0 ).toElement() ) != 0 )
        {
          return 0;
        }
        linePositions.push_back( currentPointList );
      }
    }
    else
    {
      return 0;
    }
  }


  //calculate the required wkb size
  int size = ( linePositions.size() + 1 ) * ( 1 + 2 * sizeof( int ) );
  for ( std::list<std::list<QgsPoint> >::const_iterator it = linePositions.begin(); it != linePositions.end(); ++it )
  {
    size += it->size() * 2 * sizeof( double );
  }

  QGis::WkbType type = QGis::WKBMultiLineString;
  unsigned char* wkb = new unsigned char[size];

  //fill the wkb content
  //char e = QgsApplication::endian();
  char e = ( htonl( 1 ) == 1 ) ? 0 : 1 ;
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  int nLines = linePositions.size();
  int nPoints; //number of points in a line
  double x, y;
  memcpy( &( wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( wkb )[wkbPosition], &nLines, sizeof( int ) );
  wkbPosition += sizeof( int );
  for ( std::list<std::list<QgsPoint> >::const_iterator it = linePositions.begin(); it != linePositions.end(); ++it )
  {
    memcpy( &( wkb )[wkbPosition], &e, 1 );
    wkbPosition += 1;
    memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
    wkbPosition += sizeof( int );
    nPoints = it->size();
    memcpy( &( wkb )[wkbPosition], &nPoints, sizeof( int ) );
    wkbPosition += sizeof( int );
    for ( std::list<QgsPoint>::const_iterator iter = it->begin(); iter != it->end(); ++iter )
    {
      x = iter->x();
      //qWarning("x is: " + QString::number(x));
      y = iter->y();
      //qWarning("y is: " + QString::number(y));
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

QgsGeometry* QgsOgcUtils::geometryFromGML3MultiPolygon( const QDomElement& geometryElement )
{
  //first list: different polygons, second list: different rings, third list: different points
  std::list<std::list<std::list<QgsPoint> > > multiPolygonPoints;
  QDomElement currentPolygonMemberElement;
  QDomNodeList polygonList;
  QDomElement currentPolygonElement;
  QDomNodeList exteriorList;
  QDomElement currentExteriorElement;
  QDomElement currentInteriorElement;
  QDomNodeList interiorList;
  QDomNodeList linearRingNodeList;
  QDomElement currentLinearRingElement;
  QDomNodeList currentPosList;

  QDomNodeList polygonMemberList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "polygonMember" );
  for ( int i = 0; i < polygonMemberList.size(); ++i )
  {
    std::list<std::list<QgsPoint> > currentPolygonList;
    currentPolygonMemberElement = polygonMemberList.at( i ).toElement();
    polygonList = currentPolygonMemberElement.elementsByTagNameNS( GML_NAMESPACE, "Polygon" );
    if ( polygonList.size() < 1 )
    {
      continue;
    }
    currentPolygonElement = polygonList.at( 0 ).toElement();

    //find exterior ring
    exteriorList = currentPolygonElement.elementsByTagNameNS( GML_NAMESPACE, "exterior" );
    if ( exteriorList.size() < 1 )
    {
      continue;
    }

    currentExteriorElement = exteriorList.at( 0 ).toElement();
    std::list<QgsPoint> ringPositions;

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
    if ( readGML3Positions( ringPositions, currentPosList.at( 0 ).toElement() ) != 0 )
    {
      continue;
    }
    currentPolygonList.push_back( ringPositions );

    //find interior rings
    QDomNodeList interiorList = currentPolygonElement.elementsByTagNameNS( GML_NAMESPACE, "interior" );
    for ( int j = 0; j < interiorList.size(); ++j )
    {
      std::list<QgsPoint> ringPositions;
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
      if ( readGML3Positions( ringPositions, currentPosList.at( 0 ).toElement() ) != 0 )
      {
        continue;
      }
      currentPolygonList.push_back( ringPositions );
    }
    multiPolygonPoints.push_back( currentPolygonList );
  }

  int size = 1 + 2 * sizeof( int );
  //calculate the wkb size
  for ( std::list<std::list<std::list<QgsPoint> > >::const_iterator it = multiPolygonPoints.begin(); it != multiPolygonPoints.end(); ++it )
  {
    size += 1 + 2 * sizeof( int );
    for ( std::list<std::list<QgsPoint> >::const_iterator iter = it->begin(); iter != it->end(); ++iter )
    {
      size += sizeof( int ) + 2 * iter->size() * sizeof( double );
    }
  }

  QGis::WkbType type = QGis::WKBMultiPolygon;
  unsigned char* wkb = new unsigned char[size];

  int polygonType = QGis::WKBPolygon;
  //char e = QgsApplication::endian();
  char e = ( htonl( 1 ) == 1 ) ? 0 : 1 ;
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  double x, y;
  int nPolygons = multiPolygonPoints.size();
  int nRings;
  int nPointsInRing;

  //fill the contents into *wkb
  memcpy( &( wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( wkb )[wkbPosition], &type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( wkb )[wkbPosition], &nPolygons, sizeof( int ) );
  wkbPosition += sizeof( int );

  for ( std::list<std::list<std::list<QgsPoint> > >::const_iterator it = multiPolygonPoints.begin(); it != multiPolygonPoints.end(); ++it )
  {
    memcpy( &( wkb )[wkbPosition], &e, 1 );
    wkbPosition += 1;
    memcpy( &( wkb )[wkbPosition], &polygonType, sizeof( int ) );
    wkbPosition += sizeof( int );
    nRings = it->size();
    memcpy( &( wkb )[wkbPosition], &nRings, sizeof( int ) );
    wkbPosition += sizeof( int );
    for ( std::list<std::list<QgsPoint> >::const_iterator iter = it->begin(); iter != it->end(); ++iter )
    {
      nPointsInRing = iter->size();
      memcpy( &( wkb )[wkbPosition], &nPointsInRing, sizeof( int ) );
      wkbPosition += sizeof( int );
      for ( std::list<QgsPoint>::const_iterator iterator = iter->begin(); iterator != iter->end(); ++iterator )
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


bool QgsOgcUtils::readGML3Positions( std::list<QgsPoint>& coords, const QDomElement elem )
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

  for (int i=0; i<posSize/srsDimension; i++) {
    x = pos.at( i*srsDimension ).toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      return 1;
    }
    y = pos.at( i*srsDimension+1 ).toDouble( &conversionSuccess );
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

  elem = lowerCornerList.at( 0 ).toElement();
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




