/***************************************************************************
    qgsgml.cpp
    ---------------------
    begin                : February 2013
    copyright            : (C) 2013 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsgml.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include <QBuffer>
#include <QList>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QProgressDialog>
#include <QSet>
#include <QSettings>
#include <QUrl>

#include <limits>

const char NS_SEPARATOR = '?';
const QString GML_NAMESPACE = "http://www.opengis.net/gml";

QgsGml::QgsGml(
  const QString& typeName,
  const QString& geometryAttribute,
  const QgsFields & fields )
    : QObject()
    , mTypeName( typeName )
    , mGeometryAttribute( geometryAttribute )
    , mFinished( false )
    , mCurrentFeature( 0 )
    , mFeatureCount( 0 )
    , mCurrentWKBSize( 0 )
{
  mThematicAttributes.clear();
  for ( int i = 0; i < fields.size(); i++ )
  {
    mThematicAttributes.insert( fields[i].name(), qMakePair( i, fields[i] ) );
  }

  mEndian = QgsApplication::endian();

  int index = mTypeName.indexOf( ":" );
  if ( index != -1 && index < mTypeName.length() )
  {
    mTypeName = mTypeName.mid( index + 1 );
  }
}

QgsGml::~QgsGml()
{
}

int QgsGml::getFeatures( const QString& uri, QGis::WkbType* wkbType, QgsRectangle* extent )
{
  mUri = uri;
  mWkbType = wkbType;

  XML_Parser p = XML_ParserCreateNS( NULL, NS_SEPARATOR );
  XML_SetUserData( p, this );
  XML_SetElementHandler( p, QgsGml::start, QgsGml::end );
  XML_SetCharacterDataHandler( p, QgsGml::chars );

  //start with empty extent
  mExtent.setMinimal();

  QNetworkRequest request( mUri );
  QNetworkReply* reply = QgsNetworkAccessManager::instance()->get( request );

  connect( reply, SIGNAL( finished() ), this, SLOT( setFinished() ) );
  connect( reply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( handleProgressEvent( qint64, qint64 ) ) );

  //find out if there is a QGIS main window. If yes, display a progress dialog
  QProgressDialog* progressDialog = 0;
  QWidget* mainWindow = 0;
  QWidgetList topLevelWidgets = qApp->topLevelWidgets();
  for ( QWidgetList::iterator it = topLevelWidgets.begin(); it != topLevelWidgets.end(); ++it )
  {
    if (( *it )->objectName() == "QgisApp" )
    {
      mainWindow = *it;
      break;
    }
  }
  if ( mainWindow )
  {
    progressDialog = new QProgressDialog( tr( "Loading GML data\n%1" ).arg( mTypeName ), tr( "Abort" ), 0, 0, mainWindow );
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
      if ( XML_Parse( p, readData.constData(), readData.size(), atEnd ) == 0 )
      {
        XML_Error errorCode = XML_GetErrorCode( p );
        QString errorString = tr( "Error: %1 on line %2, column %3" )
                              .arg( XML_ErrorString( errorCode ) )
                              .arg( XML_GetCurrentLineNumber( p ) )
                              .arg( XML_GetCurrentColumnNumber( p ) );
        QgsMessageLog::logMessage( errorString, tr( "WFS" ) );
      }
    }
    QCoreApplication::processEvents();
  }

  delete reply;
  delete progressDialog;

  if ( *mWkbType != QGis::WKBNoGeometry )
  {
    if ( mExtent.isEmpty() )
    {
      //reading of bbox from the server failed, so we calculate it less efficiently by evaluating the features
      calculateExtentFromFeatures();
    }
  }

  XML_ParserFree( p );

  if ( extent )
    *extent = mExtent;

  return 0;
}

int QgsGml::getFeatures( const QByteArray &data, QGis::WkbType* wkbType, QgsRectangle* extent )
{
  mWkbType = wkbType;
  mExtent.setMinimal();

  XML_Parser p = XML_ParserCreateNS( NULL, NS_SEPARATOR );
  XML_SetUserData( p, this );
  XML_SetElementHandler( p, QgsGml::start, QgsGml::end );
  XML_SetCharacterDataHandler( p, QgsGml::chars );
  int atEnd = 1;
  XML_Parse( p, data.constData(), data.size(), atEnd );

  if ( extent )
    *extent = mExtent;

  return 0;
}

void QgsGml::setFinished( )
{
  mFinished = true;
}

void QgsGml::handleProgressEvent( qint64 progress, qint64 totalSteps )
{
  emit dataReadProgress( progress );
  if ( totalSteps < 0 )
  {
    totalSteps = 0;
  }
  emit totalStepsUpdate( totalSteps );
  emit dataProgressAndSteps( progress, totalSteps );
}

void QgsGml::startElement( const XML_Char* el, const XML_Char** attr )
{
  QString elementName( el );
  ParseMode theParseMode( mParseModeStack.isEmpty() ? none : mParseModeStack.top() );
  QStringList splitName =  elementName.split( NS_SEPARATOR );
  QString localName = splitName.last();
  QString ns = splitName.size() > 1 ? splitName.first() : "";

  if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "coordinates" )
  {
    mParseModeStack.push( QgsGml::coordinate );
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
    mParseModeStack.push( QgsGml::geometry );
  }
  //else if ( mParseModeStack.size() == 0 && elementName == GML_NAMESPACE + NS_SEPARATOR + "boundedBy" )
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "boundedBy" )
  {
    mParseModeStack.push( QgsGml::boundingBox );
  }
  else if ( theParseMode == none && localName == mTypeName )
  {
    Q_ASSERT( !mCurrentFeature );
    mCurrentFeature = new QgsFeature( mFeatureCount );
    QgsAttributes attributes( mThematicAttributes.size() ); //add empty attributes
    mCurrentFeature->setAttributes( attributes );
    mParseModeStack.push( QgsGml::feature );
    mCurrentFeatureId = readAttribute( "fid", attr );
  }

  else if ( theParseMode == boundingBox && elementName == GML_NAMESPACE + NS_SEPARATOR + "Box" )
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
    mCurrentWKBFragments.push_back( QList<unsigned char*>() );
    mCurrentWKBFragmentSizes.push_back( QList<int>() );
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "MultiPoint" )
  {
    mParseModeStack.push( QgsGml::multiPoint );
    //we need one nested list for intermediate WKB
    mCurrentWKBFragments.push_back( QList<unsigned char*>() );
    mCurrentWKBFragmentSizes.push_back( QList<int>() );
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "MultiLineString" )
  {
    mParseModeStack.push( QgsGml::multiLine );
    //we need one nested list for intermediate WKB
    mCurrentWKBFragments.push_back( QList<unsigned char*>() );
    mCurrentWKBFragmentSizes.push_back( QList<int>() );
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "MultiPolygon" )
  {
    mParseModeStack.push( QgsGml::multiPolygon );
  }
  else if ( theParseMode == feature && mThematicAttributes.contains( localName ) )
  {
    mParseModeStack.push( QgsGml::attribute );
    mAttributeName = localName;
    mStringCash.clear();
  }
}

void QgsGml::endElement( const XML_Char* el )
{
  QString elementName( el );
  ParseMode theParseMode( mParseModeStack.isEmpty() ? none : mParseModeStack.top() );
  QStringList splitName =  elementName.split( NS_SEPARATOR );
  QString localName = splitName.last();
  QString ns = splitName.size() > 1 ? splitName.first() : "";

  if ( theParseMode == coordinate && elementName == GML_NAMESPACE + NS_SEPARATOR + "coordinates" )
  {
    mParseModeStack.pop();
  }
  else if ( theParseMode == attribute && localName == mAttributeName ) //add a thematic attribute to the feature
  {
    mParseModeStack.pop();

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
      Q_ASSERT( mCurrentFeature );
      mCurrentFeature->setAttribute( att_it.value().first, QVariant( mStringCash ) );
    }
  }
  else if ( theParseMode == geometry && localName == mGeometryAttribute )
  {
    mParseModeStack.pop();
  }
  else if ( theParseMode == boundingBox && elementName == GML_NAMESPACE + NS_SEPARATOR + "boundedBy" )
  {
    //create bounding box from mStringCash
    if ( createBBoxFromCoordinateString( mCurrentExtent, mStringCash ) != 0 )
    {
      QgsDebugMsg( "creation of bounding box failed" );
    }

    mParseModeStack.pop();
  }
  else if ( theParseMode == feature && localName == mTypeName )
  {
    Q_ASSERT( mCurrentFeature );
    if ( mCurrentWKBSize > 0 )
    {
      mCurrentFeature->setGeometryAndOwnership( mCurrentWKB, mCurrentWKBSize );
    }
    else if ( !mCurrentExtent.isEmpty() )
    {
      mCurrentFeature->setGeometry( QgsGeometry::fromRect( mCurrentExtent ) );
    }
    else
    {
      mCurrentFeature->setGeometry( 0 );
    }
    mCurrentFeature->setValid( true );

    mFeatures.insert( mCurrentFeature->id(), mCurrentFeature );
    if ( !mCurrentFeatureId.isEmpty() )
    {
      mIdMap.insert( mCurrentFeature->id(), mCurrentFeatureId );
    }
    mCurrentFeature = 0;
    ++mFeatureCount;
    mParseModeStack.pop();
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "Point" )
  {
    QList<QgsPoint> pointList;
    if ( pointsFromCoordinateString( pointList, mStringCash ) != 0 )
    {
      //error
    }

    if ( theParseMode == QgsGml::geometry )
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
      QList<unsigned char*> wkbList;
      QList<int> wkbSizeList;
      if ( getPointWKB( &wkb, &wkbSize, *( pointList.begin() ) ) != 0 )
      {
        //error
      }
      if ( !mCurrentWKBFragments.isEmpty() )
      {
        mCurrentWKBFragments.last().push_back( wkb );
        mCurrentWKBFragmentSizes.last().push_back( wkbSize );
      }
      else
      {
        QgsDebugMsg( "No wkb fragments" );
      }
    }
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "LineString" )
  {
    //add WKB point to the feature

    QList<QgsPoint> pointList;
    if ( pointsFromCoordinateString( pointList, mStringCash ) != 0 )
    {
      //error
    }
    if ( theParseMode == QgsGml::geometry )
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
      QList<unsigned char*> wkbList;
      QList<int> wkbSizeList;
      if ( getLineWKB( &wkb, &wkbSize, pointList ) != 0 )
      {
        //error
      }
      if ( !mCurrentWKBFragments.isEmpty() )
      {
        mCurrentWKBFragments.last().push_back( wkb );
        mCurrentWKBFragmentSizes.last().push_back( wkbSize );
      }
      else
      {
        QgsDebugMsg( "no wkb fragments" );
      }
    }
  }
  else if (( theParseMode == geometry || theParseMode == multiPolygon ) && elementName == GML_NAMESPACE + NS_SEPARATOR + "LinearRing" )
  {
    QList<QgsPoint> pointList;
    if ( pointsFromCoordinateString( pointList, mStringCash ) != 0 )
    {
      //error
    }
    unsigned char* wkb = 0;
    int wkbSize = 0;
    if ( getRingWKB( &wkb, &wkbSize, pointList ) != 0 )
    {
      //error
    }
    if ( !mCurrentWKBFragments.isEmpty() )
    {
      mCurrentWKBFragments.last().push_back( wkb );
      mCurrentWKBFragmentSizes.last().push_back( wkbSize );
    }
    else
    {
      QgsDebugMsg( "no wkb fragments" );
    }
  }
  else if (( theParseMode == geometry || theParseMode == multiPolygon ) && elementName == GML_NAMESPACE + NS_SEPARATOR + "Polygon" )
  {
    if ( *mWkbType != QGis::WKBMultiPolygon )//keep multitype in case of geometry type mix
    {
      *mWkbType = QGis::WKBPolygon;
    }

    if ( theParseMode == geometry )
    {
      createPolygonFromFragments();
    }
  }
  else if ( theParseMode == multiPoint && elementName == GML_NAMESPACE + NS_SEPARATOR + "MultiPoint" )
  {
    *mWkbType = QGis::WKBMultiPoint;
    mParseModeStack.pop();
    createMultiPointFromFragments();
  }
  else if ( theParseMode == multiLine && elementName == GML_NAMESPACE + NS_SEPARATOR + "MultiLineString" )
  {
    *mWkbType = QGis::WKBMultiLineString;
    mParseModeStack.pop();
    createMultiLineFromFragments();
  }
  else if ( theParseMode == multiPolygon && elementName == GML_NAMESPACE + NS_SEPARATOR + "MultiPolygon" )
  {
    *mWkbType = QGis::WKBMultiPolygon;
    mParseModeStack.pop();
    createMultiPolygonFromFragments();
  }
}

void QgsGml::characters( const XML_Char* chars, int len )
{
  //save chars in mStringCash attribute mode or coordinate mode
  if ( mParseModeStack.size() == 0 )
  {
    return;
  }

  QgsGml::ParseMode theParseMode = mParseModeStack.top();
  if ( theParseMode == QgsGml::attribute || theParseMode == QgsGml::coordinate )
  {
    mStringCash.append( QString::fromUtf8( chars, len ) );
  }
}

int QgsGml::readEpsgFromAttribute( int& epsgNr, const XML_Char** attr ) const
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

QString QgsGml::readAttribute( const QString& attributeName, const XML_Char** attr ) const
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

int QgsGml::createBBoxFromCoordinateString( QgsRectangle &r, const QString& coordString ) const
{
  QList<QgsPoint> points;
  if ( pointsFromCoordinateString( points, coordString ) != 0 )
  {
    return 2;
  }

  if ( points.size() < 2 )
  {
    return 3;
  }

  r.set( points[0], points[1] );

  return 0;
}

int QgsGml::pointsFromCoordinateString( QList<QgsPoint>& points, const QString& coordString ) const
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

int QgsGml::getPointWKB( unsigned char** wkb, int* size, const QgsPoint& point ) const
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

int QgsGml::getLineWKB( unsigned char** wkb, int* size, const QList<QgsPoint>& lineCoordinates ) const
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

  QList<QgsPoint>::const_iterator iter;
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

int QgsGml::getRingWKB( unsigned char** wkb, int* size, const QList<QgsPoint>& ringCoordinates ) const
{
  int wkbSize = sizeof( int ) + ringCoordinates.size() * 2 * sizeof( double );
  *size = wkbSize;
  *wkb = new unsigned char[wkbSize];
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  double x, y;
  int nPoints = ringCoordinates.size();
  memcpy( &( *wkb )[wkbPosition], &nPoints, sizeof( int ) );
  wkbPosition += sizeof( int );

  QList<QgsPoint>::const_iterator iter;
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

int QgsGml::createMultiLineFromFragments()
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
  QList<unsigned char*>::iterator wkbIt = mCurrentWKBFragments.begin()->begin();
  QList<int>::iterator sizeIt = mCurrentWKBFragmentSizes.begin()->begin();

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

int QgsGml::createMultiPointFromFragments()
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

  QList<unsigned char*>::iterator wkbIt = mCurrentWKBFragments.begin()->begin();
  QList<int>::iterator sizeIt = mCurrentWKBFragmentSizes.begin()->begin();

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


int QgsGml::createPolygonFromFragments()
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

  QList<unsigned char*>::iterator wkbIt = mCurrentWKBFragments.begin()->begin();
  QList<int>::iterator sizeIt = mCurrentWKBFragmentSizes.begin()->begin();
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

int QgsGml::createMultiPolygonFromFragments()
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
  QList< QList<unsigned char*> >::iterator outerWkbIt;
  QList< QList<int> >::iterator outerSizeIt;
  QList< unsigned char* >::iterator innerWkbIt;
  QList< int >::iterator innerSizeIt;

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

int QgsGml::totalWKBFragmentSize() const
{
  int result = 0;
  foreach ( const QList<int> &list, mCurrentWKBFragmentSizes )
  {
    foreach ( int i, list )
    {
      result += i;
    }
  }
  return result;
}

void QgsGml::calculateExtentFromFeatures()
{
  if ( mFeatures.size() < 1 )
  {
    return;
  }

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
        mExtent = currentGeometry->boundingBox();
        bboxInitialised = true;
      }
      else
      {
        mExtent.unionRect( currentGeometry->boundingBox() );
      }
    }
  }
}
