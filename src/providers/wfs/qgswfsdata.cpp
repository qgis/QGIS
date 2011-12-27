/***************************************************************************
     qgswfsdata.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:19:51 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswfsdata.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"
#include <QBuffer>
#include <QList>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QProgressDialog>
#include <QSet>
#include <QSettings>
#include <QUrl>

const char NS_SEPARATOR = '?';
const QString GML_NAMESPACE = "http://www.opengis.net/gml";

QgsWFSData::QgsWFSData(
  const QString& uri,
  QgsRectangle* extent,
  QMap<QgsFeatureId, QgsFeature*> &features,
  QMap<QgsFeatureId, QString > &idMap,
  const QString& geometryAttribute,
  const QMap<QString, QPair<int, QgsField> >& thematicAttributes,
  QGis::WkbType* wkbType )
    : QObject(),
    mUri( uri ),
    mExtent( extent ),
    mFeatures( features ),
    mIdMap( idMap ),
    mGeometryAttribute( geometryAttribute ),
    mThematicAttributes( thematicAttributes ),
    mWkbType( wkbType ),
    mFinished( false ),
    mFeatureCount( 0 )
{
  //find out mTypeName from uri
  QStringList arguments = uri.split( "&" );
  QStringList::const_iterator it;
  for ( it = arguments.constBegin(); it != arguments.constEnd(); ++it )
  {
    if ( it->startsWith( "TYPENAME", Qt::CaseInsensitive ) )
    {
      mTypeName = it->section( "=", 1, 1 );
      //and strip away namespace prefix
      QStringList splitList = mTypeName.split( ":" );
      if ( splitList.size() > 1 )
      {
        mTypeName = splitList.at( 1 );
      }
      QgsDebugMsg( QString( "mTypeName is: %1" ).arg( mTypeName ) );
    }
  }

  mEndian = QgsApplication::endian();
}

QgsWFSData::~QgsWFSData()
{

}

int QgsWFSData::getWFSData()
{
  XML_Parser p = XML_ParserCreateNS( NULL, NS_SEPARATOR );
  XML_SetUserData( p, this );
  XML_SetElementHandler( p, QgsWFSData::start, QgsWFSData::end );
  XML_SetCharacterDataHandler( p, QgsWFSData::chars );

  //start with empty extent
  if ( mExtent )
  {
    mExtent->set( 0, 0, 0, 0 );
  }

  //QUrl requestUrl( mUri );
  QNetworkRequest request( mUri );
  QNetworkReply* reply = QgsNetworkAccessManager::instance()->get( request );

  connect( reply, SIGNAL( finished() ), this, SLOT( setFinished() ) );
  connect( reply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( handleProgressEvent( qint64, qint64 ) ) );

  //find out if there is a QGIS main window. If yes, display a progress dialog
  QProgressDialog* progressDialog = 0;
  QWidget* mainWindow = findMainWindow();

  if ( mainWindow )
  {
    progressDialog = new QProgressDialog( tr( "Loading WFS data\n%1" ).arg( mTypeName ), tr( "Abort" ), 0, 0, mainWindow );
    progressDialog->setWindowModality( Qt::ApplicationModal );
    connect( this, SIGNAL( dataReadProgress( int ) ), progressDialog, SLOT( setValue( int ) ) );
    connect( this, SIGNAL( totalStepsUpdate( int ) ), progressDialog, SLOT( setMaximum( int ) ) );
    connect( progressDialog, SIGNAL( canceled() ), this, SLOT( setFinished() ) );
    progressDialog->show();
  }

  int atEnd = 0;
  while ( !atEnd )
  {
    if ( mFinished )
    {
      atEnd = 1;
    }
    QByteArray readData = reply->readAll();
    if ( readData.size() > 0 )
    {
      XML_Parse( p, readData.constData(), readData.size(), atEnd );
    }
    QCoreApplication::processEvents();
  }

  delete reply;
  delete progressDialog;

  if ( mExtent )
  {
    if ( mExtent->isEmpty() )
    {
      //reading of bbox from the server failed, so we calculate it less efficiently by evaluating the features
      calculateExtentFromFeatures();
    }
  }

  XML_ParserFree( p );
  return 0;
}

void QgsWFSData::setFinished( )
{
  mFinished = true;
}

void QgsWFSData::handleProgressEvent( qint64 progress, qint64 totalSteps )
{
  emit dataReadProgress( progress );
  if ( totalSteps < 0 )
  {
    totalSteps = 0;
  }
  emit totalStepsUpdate( totalSteps );
  emit dataProgressAndSteps( progress, totalSteps );
}

void QgsWFSData::startElement( const XML_Char* el, const XML_Char** attr )
{
  QString elementName( el );
  QString localName = elementName.section( NS_SEPARATOR, 1, 1 );
  if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "coordinates" )
  {
    mParseModeStack.push( QgsWFSData::coordinate );
    mStringCash.clear();
    mCoordinateSeparator = readAttribute( "cs", attr );
    if ( mCoordinateSeparator.isEmpty() )
    {
      mCoordinateSeparator = ",";
    }
    mTupleSeparator = readAttribute( "ts", attr );
    if ( mTupleSeparator.isEmpty() )
    {
      mTupleSeparator = " ";
    }
  }
  else if ( localName == mGeometryAttribute )
  {
    mParseModeStack.push( QgsWFSData::geometry );
  }
  else if ( mParseModeStack.size() == 0 && elementName == GML_NAMESPACE + NS_SEPARATOR + "boundedBy" )
  {
    mParseModeStack.push( QgsWFSData::boundingBox );
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "featureMember" )
  {
    mCurrentFeature = new QgsFeature( mFeatureCount );
    mParseModeStack.push( QgsWFSData::featureMember );
  }
  else if ( localName == mTypeName )
  {
    mCurrentFeatureId = readAttribute( "fid", attr );
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "Box" && mParseModeStack.top() == QgsWFSData::boundingBox )
  {
    //read attribute srsName="EPSG:26910"
    int epsgNr;
    if ( readEpsgFromAttribute( epsgNr, attr ) != 0 )
    {
      QgsDebugMsg( "error, could not get epsg id" );
    }
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "Polygon" )
  {
    std::list<unsigned char*> wkbList;
    std::list<int> wkbSizeList;
    mCurrentWKBFragments.push_back( wkbList );
    mCurrentWKBFragmentSizes.push_back( wkbSizeList );
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "MultiPoint" )
  {
    mParseModeStack.push( QgsWFSData::multiPoint );
    //we need one nested list for intermediate WKB
    std::list<unsigned char*> wkbList;
    std::list<int> wkbSizeList;
    mCurrentWKBFragments.push_back( wkbList );
    mCurrentWKBFragmentSizes.push_back( wkbSizeList );
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "MultiLineString" )
  {
    mParseModeStack.push( QgsWFSData::multiLine );
    //we need one nested list for intermediate WKB
    std::list<unsigned char*> wkbList;
    std::list<int> wkbSizeList;
    mCurrentWKBFragments.push_back( wkbList );
    mCurrentWKBFragmentSizes.push_back( wkbSizeList );
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "MultiPolygon" )
  {
    mParseModeStack.push( QgsWFSData::multiPolygon );
  }

  else if ( mParseModeStack.size() == 1 && mParseModeStack.top() == QgsWFSData::featureMember && mThematicAttributes.find( localName ) != mThematicAttributes.end() )
  {
    mParseModeStack.push( QgsWFSData::attribute );
    mAttributeName = localName;
    mStringCash.clear();
  }
}

void QgsWFSData::endElement( const XML_Char* el )
{
  QString elementName( el );
  QString localName = elementName.section( NS_SEPARATOR, 1, 1 );
  if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "coordinates" )
  {
    if ( !mParseModeStack.empty() )
    {
      mParseModeStack.pop();
    }
  }
  else if ( localName == mAttributeName ) //add a thematic attribute to the feature
  {
    if ( !mParseModeStack.empty() )
    {
      mParseModeStack.pop();
    }

    //find index with attribute name
    QMap<QString, QPair<int, QgsField> >::const_iterator att_it = mThematicAttributes.find( mAttributeName );
    if ( att_it != mThematicAttributes.constEnd() )
    {
      QVariant var;
      switch ( att_it.value().second.type() )
      {
        case QVariant::Double:
          var = QVariant( mStringCash.toDouble() );
          break;
        case QVariant::Int:
          var = QVariant( mStringCash.toInt() );
          break;
        case QVariant::LongLong:
          var = QVariant( mStringCash.toLongLong() );
          break;
        default: //string type is default
          var = QVariant( mStringCash );
          break;
      }
      mCurrentFeature->addAttribute( att_it.value().first, QVariant( mStringCash ) );
    }
  }
  else if ( localName == mGeometryAttribute )
  {
    if ( !mParseModeStack.empty() )
    {
      mParseModeStack.pop();
    }
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "boundedBy" && mParseModeStack.top() == QgsWFSData::boundingBox )
  {
    //create bounding box from mStringCash
    if ( createBBoxFromCoordinateString( mExtent, mStringCash ) != 0 )
    {
      QgsDebugMsg( "creation of bounding box failed" );
    }

    if ( !mParseModeStack.empty() )
    {
      mParseModeStack.pop();
    }
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "featureMember" )
  {
    //MH090531: Check if all feature attributes are initialised, sometimes attribute values are missing.
    //We fill the not initialized ones with empty strings, otherwise the feature cannot be exported to shp later
    QgsAttributeMap currentFeatureAttributes = mCurrentFeature->attributeMap();
    QMap<QString, QPair<int, QgsField> >::const_iterator att_it = mThematicAttributes.constBegin();
    for ( ; att_it != mThematicAttributes.constEnd(); ++att_it )
    {
      int attIndex = att_it.value().first;
      QgsAttributeMap::const_iterator findIt = currentFeatureAttributes.find( attIndex );
      if ( findIt == currentFeatureAttributes.constEnd() )
      {
        mCurrentFeature->addAttribute( attIndex, QVariant( "" ) );
      }
    }


    mCurrentFeature->setGeometryAndOwnership( mCurrentWKB, mCurrentWKBSize );
    mFeatures.insert( mCurrentFeature->id(), mCurrentFeature );
    if ( !mCurrentFeatureId.isEmpty() )
    {
      mIdMap.insert( mCurrentFeature->id(), mCurrentFeatureId );
    }
    ++mFeatureCount;
    mParseModeStack.pop();
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "Point" )
  {
    std::list<QgsPoint> pointList;
    if ( pointsFromCoordinateString( pointList, mStringCash ) != 0 )
    {
      //error
    }

    if ( mParseModeStack.top() != QgsWFSData::multiPoint )
    {
      //directly add WKB point to the feature
      if ( getPointWKB( &mCurrentWKB, &mCurrentWKBSize, *( pointList.begin() ) ) != 0 )
      {
        //error
      }

      if ( *mWkbType != QGis::WKBMultiPoint ) //keep multitype in case of geometry type mix
      {
        *mWkbType = QGis::WKBPoint;
      }
    }
    else //multipoint, add WKB as fragment
    {
      unsigned char* wkb = 0;
      int wkbSize = 0;
      std::list<unsigned char*> wkbList;
      std::list<int> wkbSizeList;
      if ( getPointWKB( &wkb, &wkbSize, *( pointList.begin() ) ) != 0 )
      {
        //error
      }
      mCurrentWKBFragments.rbegin()->push_back( wkb );
      mCurrentWKBFragmentSizes.rbegin()->push_back( wkbSize );
      //wkbList.push_back(wkb);
      //wkbSizeList.push_back(wkbSize);
      //mCurrentWKBFragments.push_back(wkbList);
      //mCurrentWKBFragmentSizes.push_back(wkbSizeList);
    }
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "LineString" )
  {
    //add WKB point to the feature

    std::list<QgsPoint> pointList;
    if ( pointsFromCoordinateString( pointList, mStringCash ) != 0 )
    {
      //error
    }
    if ( mParseModeStack.top() != QgsWFSData::multiLine )
    {
      if ( getLineWKB( &mCurrentWKB, &mCurrentWKBSize, pointList ) != 0 )
      {
        //error
      }

      if ( *mWkbType != QGis::WKBMultiLineString )//keep multitype in case of geometry type mix
      {
        *mWkbType = QGis::WKBLineString;
      }
    }
    else //multiline, add WKB as fragment
    {
      unsigned char* wkb = 0;
      int wkbSize = 0;
      std::list<unsigned char*> wkbList;
      std::list<int> wkbSizeList;
      if ( getLineWKB( &wkb, &wkbSize, pointList ) != 0 )
      {
        //error
      }
      mCurrentWKBFragments.rbegin()->push_back( wkb );
      mCurrentWKBFragmentSizes.rbegin()->push_back( wkbSize );
      //wkbList.push_back(wkb);
      //wkbSizeList.push_back(wkbSize);
      //mCurrentWKBFragments.push_back(wkbList);
      //mCurrentWKBFragmentSizes.push_back(wkbSizeList);
    }
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "LinearRing" )
  {
    std::list<QgsPoint> pointList;
    if ( pointsFromCoordinateString( pointList, mStringCash ) != 0 )
    {
      //error
    }
    unsigned char* wkb;
    int wkbSize;
    if ( getRingWKB( &wkb, &wkbSize, pointList ) != 0 )
    {
      //error
    }
    mCurrentWKBFragments.rbegin()->push_back( wkb );
    mCurrentWKBFragmentSizes.rbegin()->push_back( wkbSize );
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "Polygon" )
  {
    if ( *mWkbType != QGis::WKBMultiPolygon )//keep multitype in case of geometry type mix
    {
      *mWkbType = QGis::WKBPolygon;
    }
    if ( mParseModeStack.top() != QgsWFSData::multiPolygon )
    {
      createPolygonFromFragments();
    }
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "MultiPoint" )
  {
    *mWkbType = QGis::WKBMultiPoint;
    if ( !mParseModeStack.empty() )
    {
      mParseModeStack.pop();
    }
    createMultiPointFromFragments();
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "MultiLineString" )
  {
    *mWkbType = QGis::WKBMultiLineString;
    if ( !mParseModeStack.empty() )
    {
      mParseModeStack.pop();
    }
    createMultiLineFromFragments();
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "MultiPolygon" )
  {
    *mWkbType = QGis::WKBMultiPolygon;
    if ( !mParseModeStack.empty() )
    {
      mParseModeStack.pop();
    }
    createMultiPolygonFromFragments();
  }
}

void QgsWFSData::characters( const XML_Char* chars, int len )
{
  //save chars in mStringCash attribute mode or coordinate mode
  if ( mParseModeStack.size() == 0 )
  {
    return;
  }

  QgsWFSData::parseMode theParseMode = mParseModeStack.top();
  if ( theParseMode == QgsWFSData::attribute || theParseMode == QgsWFSData::coordinate )
  {
    mStringCash.append( QString::fromUtf8( chars, len ) );
  }
}


int QgsWFSData::readEpsgFromAttribute( int& epsgNr, const XML_Char** attr ) const
{
  int i = 0;
  while ( attr[i] != NULL )
  {
    if ( strcmp( attr[i], "srsName" ) == 0 )
    {
      QString epsgString( attr[i+1] );
      QString epsgNrString;
      if ( epsgString.startsWith( "http" ) ) //e.g. geoserver: "http://www.opengis.net/gml/srs/epsg.xml#4326"
      {
        epsgNrString = epsgString.section( "#", 1, 1 );
      }
      else //e.g. umn mapserver: "EPSG:4326">
      {
        epsgNrString = epsgString.section( ":", 1, 1 );
      }
      bool conversionOk;
      int eNr = epsgNrString.toInt( &conversionOk );
      if ( !conversionOk )
      {
        return 1;
      }
      epsgNr = eNr;
      return 0;
    }
    ++i;
  }
  return 2;
}

QString QgsWFSData::readAttribute( const QString& attributeName, const XML_Char** attr ) const
{
  int i = 0;
  while ( attr[i] != NULL )
  {
    if ( attributeName.compare( attr[i] ) == 0 )
    {
      return QString( attr[i+1] );
    }
    ++i;
  }
  return QString();
}

int QgsWFSData::createBBoxFromCoordinateString( QgsRectangle* bb, const QString& coordString ) const
{
  if ( !bb )
  {
    return 1;
  }

  std::list<QgsPoint> points;
  if ( pointsFromCoordinateString( points, coordString ) != 0 )
  {
    return 2;
  }
  if ( points.size() < 2 )
  {
    return 3;
  }

  std::list<QgsPoint>::const_iterator firstPointIt = points.begin();
  std::list<QgsPoint>::const_iterator secondPointIt = points.begin();
  ++secondPointIt;
  bb->set( *firstPointIt, *secondPointIt );
  return 0;
}

int QgsWFSData::pointsFromCoordinateString( std::list<QgsPoint>& points, const QString& coordString ) const
{
  //tuples are separated by space, x/y by ','
  QStringList tuples = coordString.split( mTupleSeparator, QString::SkipEmptyParts );
  QStringList tuples_coordinates;
  double x, y;
  bool conversionSuccess;

  QStringList::const_iterator tupleIterator;
  for ( tupleIterator = tuples.constBegin(); tupleIterator != tuples.constEnd(); ++tupleIterator )
  {
    tuples_coordinates = tupleIterator->split( mCoordinateSeparator, QString::SkipEmptyParts );
    if ( tuples_coordinates.size() < 2 )
    {
      continue;
    }
    x = tuples_coordinates.at( 0 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      continue;
    }
    y = tuples_coordinates.at( 1 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      continue;
    }
    points.push_back( QgsPoint( x, y ) );
  }
  return 0;
}

int QgsWFSData::getPointWKB( unsigned char** wkb, int* size, const QgsPoint& point ) const
{
  int wkbSize = 1 + sizeof( int ) + 2 * sizeof( double );
  *size = wkbSize;
  *wkb = new unsigned char[wkbSize];
  QGis::WkbType type = QGis::WKBPoint;
  double x = point.x();
  double y = point.y();
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)

  memcpy( &( *wkb )[wkbPosition], &mEndian, 1 );
  wkbPosition += 1;
  memcpy( &( *wkb )[wkbPosition], &type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( *wkb )[wkbPosition], &x, sizeof( double ) );
  wkbPosition += sizeof( double );
  memcpy( &( *wkb )[wkbPosition], &y, sizeof( double ) );
  return 0;
}

int QgsWFSData::getLineWKB( unsigned char** wkb, int* size, const std::list<QgsPoint>& lineCoordinates ) const
{
  int wkbSize = 1 + 2 * sizeof( int ) + lineCoordinates.size() * 2 * sizeof( double );
  *size = wkbSize;
  *wkb = new unsigned char[wkbSize];
  QGis::WkbType type = QGis::WKBLineString;
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  double x, y;
  int nPoints = lineCoordinates.size();

  //fill the contents into *wkb
  memcpy( &( *wkb )[wkbPosition], &mEndian, 1 );
  wkbPosition += 1;
  memcpy( &( *wkb )[wkbPosition], &type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( *wkb )[wkbPosition], &nPoints, sizeof( int ) );
  wkbPosition += sizeof( int );

  std::list<QgsPoint>::const_iterator iter;
  for ( iter = lineCoordinates.begin(); iter != lineCoordinates.end(); ++iter )
  {
    x = iter->x();
    y = iter->y();
    memcpy( &( *wkb )[wkbPosition], &x, sizeof( double ) );
    wkbPosition += sizeof( double );
    memcpy( &( *wkb )[wkbPosition], &y, sizeof( double ) );
    wkbPosition += sizeof( double );
  }
  return 0;
}

int QgsWFSData::getRingWKB( unsigned char** wkb, int* size, const std::list<QgsPoint>& ringCoordinates ) const
{
  int wkbSize = sizeof( int ) + ringCoordinates.size() * 2 * sizeof( double );
  *size = wkbSize;
  *wkb = new unsigned char[wkbSize];
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  double x, y;
  int nPoints = ringCoordinates.size();
  memcpy( &( *wkb )[wkbPosition], &nPoints, sizeof( int ) );
  wkbPosition += sizeof( int );

  std::list<QgsPoint>::const_iterator iter;
  for ( iter = ringCoordinates.begin(); iter != ringCoordinates.end(); ++iter )
  {
    x = iter->x();
    y = iter->y();
    memcpy( &( *wkb )[wkbPosition], &x, sizeof( double ) );
    wkbPosition += sizeof( double );
    memcpy( &( *wkb )[wkbPosition], &y, sizeof( double ) );
    wkbPosition += sizeof( double );
  }
  return 0;
}

int QgsWFSData::createMultiLineFromFragments()
{
  mCurrentWKBSize = 0;
  mCurrentWKBSize += 1 + 2 * sizeof( int );
  mCurrentWKBSize += totalWKBFragmentSize();

  mCurrentWKB = new unsigned char[mCurrentWKBSize];
  int pos = 0;
  QGis::WkbType type = QGis::WKBMultiLineString;
  int numLines = mCurrentWKBFragments.begin()->size();
  //add endian
  memcpy( &( mCurrentWKB[pos] ), &mEndian, 1 );
  pos += 1;
  memcpy( &( mCurrentWKB[pos] ), &type, sizeof( int ) );
  pos += sizeof( int );
  memcpy( &( mCurrentWKB[pos] ), &numLines, sizeof( int ) );
  pos += sizeof( int );
  std::list<unsigned char*>::iterator wkbIt = mCurrentWKBFragments.begin()->begin();
  std::list<int>::iterator sizeIt = mCurrentWKBFragmentSizes.begin()->begin();

  //copy (and delete) all the wkb fragments
  for ( ; wkbIt != mCurrentWKBFragments.begin()->end(); ++wkbIt, ++sizeIt )
  {
    memcpy( &( mCurrentWKB[pos] ), *wkbIt, *sizeIt );
    pos += *sizeIt;
    delete[] *wkbIt;
  }

  mCurrentWKBFragments.clear();
  mCurrentWKBFragmentSizes.clear();
  *mWkbType = QGis::WKBMultiLineString;
  return 0;
}

int QgsWFSData::createMultiPointFromFragments()
{
  mCurrentWKBSize = 0;
  mCurrentWKBSize += 1 + 2 * sizeof( int );
  mCurrentWKBSize += totalWKBFragmentSize();
  mCurrentWKB = new unsigned char[mCurrentWKBSize];

  int pos = 0;
  QGis::WkbType type = QGis::WKBMultiPoint;
  int numPoints = mCurrentWKBFragments.begin()->size();

  memcpy( &( mCurrentWKB[pos] ), &mEndian, 1 );
  pos += 1;
  memcpy( &( mCurrentWKB[pos] ), &type, sizeof( int ) );
  pos += sizeof( int );
  memcpy( &( mCurrentWKB[pos] ), &numPoints, sizeof( int ) );
  pos += sizeof( int );

  std::list<unsigned char*>::iterator wkbIt = mCurrentWKBFragments.begin()->begin();
  std::list<int>::iterator sizeIt = mCurrentWKBFragmentSizes.begin()->begin();

  for ( ; wkbIt != mCurrentWKBFragments.begin()->end(); ++wkbIt, ++sizeIt )
  {
    memcpy( &( mCurrentWKB[pos] ), *wkbIt, *sizeIt );
    pos += *sizeIt;
    delete[] *wkbIt;
  }

  mCurrentWKBFragments.clear();
  mCurrentWKBFragmentSizes.clear();
  *mWkbType = QGis::WKBMultiPoint;
  return 0;
}


int QgsWFSData::createPolygonFromFragments()
{
  mCurrentWKBSize = 0;
  mCurrentWKBSize += 1 + 2 * sizeof( int );
  mCurrentWKBSize += totalWKBFragmentSize();

  mCurrentWKB = new unsigned char[mCurrentWKBSize];
  int pos = 0;
  QGis::WkbType type = QGis::WKBPolygon;
  int numRings = mCurrentWKBFragments.begin()->size();
  memcpy( &( mCurrentWKB[pos] ), &mEndian, 1 );
  pos += 1;
  memcpy( &( mCurrentWKB[pos] ), &type, sizeof( int ) );
  pos += sizeof( int );
  memcpy( &( mCurrentWKB[pos] ), &numRings, sizeof( int ) );
  pos += sizeof( int );

  std::list<unsigned char*>::iterator wkbIt = mCurrentWKBFragments.begin()->begin();
  std::list<int>::iterator sizeIt = mCurrentWKBFragmentSizes.begin()->begin();
  for ( ; wkbIt != mCurrentWKBFragments.begin()->end(); ++wkbIt, ++sizeIt )
  {
    memcpy( &( mCurrentWKB[pos] ), *wkbIt, *sizeIt );
    pos += *sizeIt;
    delete[] *wkbIt;
  }

  mCurrentWKBFragments.clear();
  mCurrentWKBFragmentSizes.clear();
  *mWkbType = QGis::WKBPolygon;
  return 0;
}

int QgsWFSData::createMultiPolygonFromFragments()
{
  mCurrentWKBSize = 0;
  mCurrentWKBSize += 1 + 2 * sizeof( int );
  mCurrentWKBSize += totalWKBFragmentSize();
  mCurrentWKBSize += mCurrentWKBFragments.size() * ( 1 + 2 * sizeof( int ) ); //fragments are just the rings

  mCurrentWKB = new unsigned char[mCurrentWKBSize];
  int pos = 0;
  QGis::WkbType type = QGis::WKBMultiPolygon;
  QGis::WkbType polygonType = QGis::WKBPolygon;
  int numPolys = mCurrentWKBFragments.size();
  int numRings;
  memcpy( &( mCurrentWKB[pos] ), &mEndian, 1 );
  pos += 1;
  memcpy( &( mCurrentWKB[pos] ), &type, sizeof( int ) );
  pos += sizeof( int );
  memcpy( &( mCurrentWKB[pos] ), &numPolys, sizeof( int ) );
  pos += sizeof( int );

  //have outer and inner iterators
  std::list<std::list<unsigned char*> >::iterator outerWkbIt;
  std::list<std::list<int> >::iterator outerSizeIt;
  std::list<unsigned char*>::iterator innerWkbIt;
  std::list<int>::iterator innerSizeIt;

  outerWkbIt = mCurrentWKBFragments.begin();
  outerSizeIt = mCurrentWKBFragmentSizes.begin();

  for ( ; outerWkbIt != mCurrentWKBFragments.end(); ++outerWkbIt, ++outerSizeIt )
  {
    //new polygon
    memcpy( &( mCurrentWKB[pos] ), &mEndian, 1 );
    pos += 1;
    memcpy( &( mCurrentWKB[pos] ), &polygonType, sizeof( int ) );
    pos += sizeof( int );
    numRings = outerWkbIt->size();
    memcpy( &( mCurrentWKB[pos] ), &numRings, sizeof( int ) );
    pos += sizeof( int );

    innerWkbIt = outerWkbIt->begin();
    innerSizeIt = outerSizeIt->begin();
    for ( ; innerWkbIt != outerWkbIt->end(); ++innerWkbIt, ++innerSizeIt )
    {
      memcpy( &( mCurrentWKB[pos] ), *innerWkbIt, *innerSizeIt );
      pos += *innerSizeIt;
      delete[] *innerWkbIt;
    }
  }

  mCurrentWKBFragments.clear();
  mCurrentWKBFragmentSizes.clear();
  *mWkbType = QGis::WKBMultiPolygon;
  return 0;
}

int QgsWFSData::totalWKBFragmentSize() const
{
  int result = 0;
  for ( std::list<std::list<int> >::const_iterator it = mCurrentWKBFragmentSizes.begin(); it != mCurrentWKBFragmentSizes.end(); ++it )
  {
    for ( std::list<int>::const_iterator iter = it->begin(); iter != it->end(); ++iter )
    {
      result += *iter;
    }
  }
  return result;
}

QWidget* QgsWFSData::findMainWindow() const
{
  QWidget* mainWindow = 0;

  QWidgetList topLevelWidgets = qApp->topLevelWidgets();
  QWidgetList::iterator it = topLevelWidgets.begin();
  for ( ; it != topLevelWidgets.end(); ++it )
  {
    if (( *it )->objectName() == "QgisApp" )
    {
      mainWindow = *it;
      break;
    }
  }
  return mainWindow;
}

void QgsWFSData::calculateExtentFromFeatures() const
{
  if ( mFeatures.size() < 1 )
  {
    return;
  }

  QgsRectangle bbox;

  QgsFeature* currentFeature = 0;
  QgsGeometry* currentGeometry = 0;
  bool bboxInitialised = false; //gets true once bbox has been set to the first geometry

  for ( int i = 0; i < mFeatures.size(); ++i )
  {
    currentFeature = mFeatures[i];
    if ( !currentFeature )
    {
      continue;
    }
    currentGeometry = currentFeature->geometry();
    if ( currentGeometry )
    {
      if ( !bboxInitialised )
      {
        bbox = currentGeometry->boundingBox();
        bboxInitialised = true;
      }
      else
      {
        bbox.unionRect( currentGeometry->boundingBox() );
      }
    }
  }
  ( *mExtent ) = bbox;
}
