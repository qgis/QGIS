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
#include "qgsauthmanager.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgswkbptr.h"

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
    , mWkbType( nullptr )
    , mFinished( false )
    , mCurrentFeature( nullptr )
    , mFeatureCount( 0 )
    , mCurrentWKB( nullptr, 0 )
    , mDimension( 2 )
    , mCoorMode( QgsGml::coordinate )
    , mEpsg( 0 )
{
  mThematicAttributes.clear();
  for ( int i = 0; i < fields.size(); i++ )
  {
    mThematicAttributes.insert( fields[i].name(), qMakePair( i, fields[i] ) );
  }

  mEndian = QgsApplication::endian();

  int index = mTypeName.indexOf( ':' );
  if ( index != -1 && index < mTypeName.length() )
  {
    mTypeName = mTypeName.mid( index + 1 );
  }
}

QgsGml::~QgsGml()
{
}

int QgsGml::getFeatures( const QString& uri, QGis::WkbType* wkbType, QgsRectangle* extent, const QString& userName, const QString& password , const QString& authcfg )
{
  mUri = uri;
  mWkbType = wkbType;

  XML_Parser p = XML_ParserCreateNS( nullptr, NS_SEPARATOR );
  XML_SetUserData( p, this );
  XML_SetElementHandler( p, QgsGml::start, QgsGml::end );
  XML_SetCharacterDataHandler( p, QgsGml::chars );

  //start with empty extent
  mExtent.setMinimal();

  QNetworkRequest request( mUri );
  if ( !authcfg.isEmpty() )
  {
    if ( !QgsAuthManager::instance()->updateNetworkRequest( request, authcfg ) )
    {
      QgsMessageLog::logMessage(
        tr( "GML Getfeature network request update failed for authcfg %1" ).arg( authcfg ),
        tr( "Network" ),
        QgsMessageLog::CRITICAL
      );
      return 1;
    }
  }
  else if ( !userName.isNull() || !password.isNull() )
  {
    request.setRawHeader( "Authorization", "Basic " + QString( "%1:%2" ).arg( userName, password ).toAscii().toBase64() );
  }
  QNetworkReply* reply = QgsNetworkAccessManager::instance()->get( request );

  connect( reply, SIGNAL( finished() ), this, SLOT( setFinished() ) );
  connect( reply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( handleProgressEvent( qint64, qint64 ) ) );

  //find out if there is a QGIS main window. If yes, display a progress dialog
  QProgressDialog* progressDialog = nullptr;
  QWidget* mainWindow = nullptr;
  QWidgetList topLevelWidgets = qApp->topLevelWidgets();
  for ( QWidgetList::const_iterator it = topLevelWidgets.constBegin(); it != topLevelWidgets.constEnd(); ++it )
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
    if ( !readData.isEmpty() )
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

  QNetworkReply::NetworkError replyError = reply->error();
  QString replyErrorString = reply->errorString();

  delete reply;
  delete progressDialog;

  if ( replyError )
  {
    QgsMessageLog::logMessage(
      tr( "GML Getfeature network request failed with error: %1" ).arg( replyErrorString ),
      tr( "Network" ),
      QgsMessageLog::CRITICAL
    );
    return 1;
  }

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

  XML_Parser p = XML_ParserCreateNS( nullptr, NS_SEPARATOR );
  XML_SetUserData( p, this );
  XML_SetElementHandler( p, QgsGml::start, QgsGml::end );
  XML_SetCharacterDataHandler( p, QgsGml::chars );
  int atEnd = 1;
  XML_Parse( p, data.constData(), data.size(), atEnd );

  if ( extent )
    *extent = mExtent;

  return 0;
}

void QgsGml::setFinished()
{
  mFinished = true;
}

void QgsGml::handleProgressEvent( qint64 progress, qint64 totalSteps )
{
  if ( totalSteps < 0 )
  {
    totalSteps = 0;
    progress = 0;
  }
  emit totalStepsUpdate( totalSteps );
  emit dataReadProgress( progress );
  emit dataProgressAndSteps( progress, totalSteps );
}

void QgsGml::startElement( const XML_Char* el, const XML_Char** attr )
{
  QString elementName( QString::fromUtf8( el ) );
  ParseMode theParseMode( mParseModeStack.isEmpty() ? none : mParseModeStack.top() );
  QStringList splitName =  elementName.split( NS_SEPARATOR );
  QString localName = splitName.last();
  QString ns = splitName.size() > 1 ? splitName.first() : "";

  if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "coordinates" )
  {
    mParseModeStack.push( QgsGml::coordinate );
    mCoorMode = QgsGml::coordinate;
    mStringCash.clear();
    mCoordinateSeparator = readAttribute( "cs", attr );
    if ( mCoordinateSeparator.isEmpty() )
    {
      mCoordinateSeparator = ',';
    }
    mTupleSeparator = readAttribute( "ts", attr );
    if ( mTupleSeparator.isEmpty() )
    {
      mTupleSeparator = ' ';
    }
  }
  if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "pos"
       || elementName == GML_NAMESPACE + NS_SEPARATOR + "posList" )
  {
    mParseModeStack.push( QgsGml::posList );
    mCoorMode = QgsGml::posList;
    mStringCash.clear();
    QString dimension = readAttribute( "srsDimension", attr );
    bool ok;
    mDimension = dimension.toInt( &ok );
    if ( dimension.isEmpty() || !ok )
    {
      mDimension = 2;
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
    mCurrentWKBFragments.push_back( QList<QgsWkbPtr>() );
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "MultiPoint" )
  {
    mParseModeStack.push( QgsGml::multiPoint );
    //we need one nested list for intermediate WKB
    mCurrentWKBFragments.push_back( QList<QgsWkbPtr>() );
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "MultiLineString" )
  {
    mParseModeStack.push( QgsGml::multiLine );
    //we need one nested list for intermediate WKB
    mCurrentWKBFragments.push_back( QList<QgsWkbPtr>() );
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
  // QGIS server (2.2) is using:
  // <Attribute value="My description" name="desc"/>
  else if ( theParseMode == feature
            && localName.compare( "attribute", Qt::CaseInsensitive ) == 0 )
  {
    QString name = readAttribute( "name", attr );
    if ( mThematicAttributes.contains( name ) )
    {
      QString value = readAttribute( "value", attr );
      setAttribute( name, value );
    }
  }

  if ( mEpsg == 0 && ( localName == "Point" || localName == "MultiPoint" ||
                       localName == "LineString" || localName == "MultiLineString" ||
                       localName == "Polygon" || localName == "MultiPolygon" ) )
  {
    if ( readEpsgFromAttribute( mEpsg, attr ) != 0 )
    {
      QgsDebugMsg( "error, could not get epsg id" );
    }
    else
    {
      QgsDebugMsg( QString( "mEpsg = %1" ).arg( mEpsg ) );
    }
  }
}

void QgsGml::endElement( const XML_Char* el )
{
  QString elementName( QString::fromUtf8( el ) );
  ParseMode theParseMode( mParseModeStack.isEmpty() ? none : mParseModeStack.top() );
  QStringList splitName =  elementName.split( NS_SEPARATOR );
  QString localName = splitName.last();
  QString ns = splitName.size() > 1 ? splitName.first() : "";

  if (( theParseMode == coordinate && elementName == GML_NAMESPACE + NS_SEPARATOR + "coordinates" )
      || ( theParseMode == posList && (
             elementName == GML_NAMESPACE + NS_SEPARATOR + "pos"
             || elementName == GML_NAMESPACE + NS_SEPARATOR + "posList" ) ) )
  {
    mParseModeStack.pop();
  }
  else if ( theParseMode == attribute && localName == mAttributeName ) //add a thematic attribute to the feature
  {
    mParseModeStack.pop();

    setAttribute( mAttributeName, mStringCash );
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
    if ( mCurrentWKB.size() > 0 )
    {
      QgsGeometry *g = new QgsGeometry();
      g->fromWkb( mCurrentWKB, mCurrentWKB.size() );
      mCurrentFeature->setGeometry( g );
      mCurrentWKB = QgsWkbPtr( nullptr, 0 );
    }
    else if ( !mCurrentExtent.isEmpty() )
    {
      mCurrentFeature->setGeometry( QgsGeometry::fromRect( mCurrentExtent ) );
    }
    else
    {
      mCurrentFeature->setGeometry( nullptr );
    }
    mCurrentFeature->setValid( true );

    mFeatures.insert( mCurrentFeature->id(), mCurrentFeature );
    if ( !mCurrentFeatureId.isEmpty() )
    {
      mIdMap.insert( mCurrentFeature->id(), mCurrentFeatureId );
    }
    mCurrentFeature = nullptr;
    ++mFeatureCount;
    mParseModeStack.pop();
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "Point" )
  {
    QList<QgsPoint> pointList;
    if ( pointsFromString( pointList, mStringCash ) != 0 )
    {
      //error
    }

    if ( pointList.isEmpty() )
      return;  // error

    if ( theParseMode == QgsGml::geometry )
    {
      //directly add WKB point to the feature
      if ( getPointWKB( mCurrentWKB, *( pointList.constBegin() ) ) != 0 )
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
      QgsWkbPtr wkbPtr( nullptr, 0 );
      if ( getPointWKB( wkbPtr, *( pointList.constBegin() ) ) != 0 )
      {
        //error
      }
      if ( !mCurrentWKBFragments.isEmpty() )
      {
        mCurrentWKBFragments.last().push_back( wkbPtr );
      }
      else
      {
        QgsDebugMsg( "No wkb fragments" );
        delete [] wkbPtr;
      }
    }
  }
  else if ( elementName == GML_NAMESPACE + NS_SEPARATOR + "LineString" )
  {
    //add WKB point to the feature

    QList<QgsPoint> pointList;
    if ( pointsFromString( pointList, mStringCash ) != 0 )
    {
      //error
    }
    if ( theParseMode == QgsGml::geometry )
    {
      if ( getLineWKB( mCurrentWKB, pointList ) != 0 )
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
      QgsWkbPtr wkbPtr( nullptr, 0 );
      if ( getLineWKB( wkbPtr, pointList ) != 0 )
      {
        //error
      }
      if ( !mCurrentWKBFragments.isEmpty() )
      {
        mCurrentWKBFragments.last().push_back( wkbPtr );
      }
      else
      {
        QgsDebugMsg( "no wkb fragments" );
        delete [] wkbPtr;
      }
    }
  }
  else if (( theParseMode == geometry || theParseMode == multiPolygon ) && elementName == GML_NAMESPACE + NS_SEPARATOR + "LinearRing" )
  {
    QList<QgsPoint> pointList;
    if ( pointsFromString( pointList, mStringCash ) != 0 )
    {
      //error
    }

    QgsWkbPtr wkbPtr( nullptr, 0 );
    if ( getRingWKB( wkbPtr, pointList ) != 0 )
    {
      //error
    }

    if ( !mCurrentWKBFragments.isEmpty() )
    {
      mCurrentWKBFragments.last().push_back( wkbPtr );
    }
    else
    {
      delete[] wkbPtr;
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
  if ( mParseModeStack.isEmpty() )
  {
    return;
  }

  QgsGml::ParseMode theParseMode = mParseModeStack.top();
  if ( theParseMode == QgsGml::attribute || theParseMode == QgsGml::coordinate || theParseMode == QgsGml::posList )
  {
    mStringCash.append( QString::fromUtf8( chars, len ) );
  }
}

void QgsGml::setAttribute( const QString& name, const QString& value )
{
  //find index with attribute name
  QMap<QString, QPair<int, QgsField> >::const_iterator att_it = mThematicAttributes.constFind( name );
  if ( att_it != mThematicAttributes.constEnd() )
  {
    QVariant var;
    switch ( att_it.value().second.type() )
    {
      case QVariant::Double:
        var = QVariant( value.toDouble() );
        break;
      case QVariant::Int:
        var = QVariant( value.toInt() );
        break;
      case QVariant::LongLong:
        var = QVariant( value.toLongLong() );
        break;
      default: //string type is default
        var = QVariant( value );
        break;
    }
    Q_ASSERT( mCurrentFeature );
    mCurrentFeature->setAttribute( att_it.value().first, var );
  }
}

int QgsGml::readEpsgFromAttribute( int& epsgNr, const XML_Char** attr ) const
{
  int i = 0;
  while ( attr[i] )
  {
    if ( strcmp( attr[i], "srsName" ) == 0 )
    {
      QString epsgString( attr[i+1] );
      QString epsgNrString;
      if ( epsgString.startsWith( "http" ) ) //e.g. geoserver: "http://www.opengis.net/gml/srs/epsg.xml#4326"
      {
        epsgNrString = epsgString.section( '#', 1, 1 );
      }
      else //e.g. umn mapserver: "EPSG:4326">
      {
        epsgNrString = epsgString.section( ':', 1, 1 );
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
  while ( attr[i] )
  {
    if ( attributeName.compare( attr[i] ) == 0 )
    {
      return QString::fromUtf8( attr[i+1] );
    }
    i += 2;
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

int QgsGml::pointsFromPosListString( QList<QgsPoint>& points, const QString& coordString, int dimension ) const
{
  // coordinates separated by spaces
  QStringList coordinates = coordString.split( ' ', QString::SkipEmptyParts );

  if ( coordinates.size() % dimension != 0 )
  {
    QgsDebugMsg( "Wrong number of coordinates" );
  }

  int ncoor = coordinates.size() / dimension;
  for ( int i = 0; i < ncoor; i++ )
  {
    bool conversionSuccess;
    double x = coordinates.value( i * dimension ).toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      continue;
    }
    double y = coordinates.value( i * dimension + 1 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      continue;
    }
    points.append( QgsPoint( x, y ) );
  }
  return 0;
}

int QgsGml::pointsFromString( QList<QgsPoint>& points, const QString& coordString ) const
{
  if ( mCoorMode == QgsGml::coordinate )
  {
    return pointsFromCoordinateString( points, coordString );
  }
  else if ( mCoorMode == QgsGml::posList )
  {
    return pointsFromPosListString( points, coordString, mDimension );
  }
  return 1;
}

int QgsGml::getPointWKB( QgsWkbPtr &wkbPtr, const QgsPoint& point ) const
{
  int wkbSize = 1 + sizeof( int ) + 2 * sizeof( double );
  wkbPtr = QgsWkbPtr( new unsigned char[wkbSize], wkbSize );

  QgsWkbPtr fillPtr( wkbPtr );
  fillPtr << mEndian << QGis::WKBPoint << point.x() << point.y();

  return 0;
}

int QgsGml::getLineWKB( QgsWkbPtr &wkbPtr, const QList<QgsPoint>& lineCoordinates ) const
{
  int wkbSize = 1 + 2 * sizeof( int ) + lineCoordinates.size() * 2 * sizeof( double );
  wkbPtr = QgsWkbPtr( new unsigned char[wkbSize], wkbSize );

  QgsWkbPtr fillPtr( wkbPtr );

  fillPtr << mEndian << QGis::WKBLineString << lineCoordinates.size();

  QList<QgsPoint>::const_iterator iter;
  for ( iter = lineCoordinates.constBegin(); iter != lineCoordinates.constEnd(); ++iter )
  {
    fillPtr << iter->x() << iter->y();
  }

  return 0;
}

int QgsGml::getRingWKB( QgsWkbPtr &wkbPtr, const QList<QgsPoint>& ringCoordinates ) const
{
  int wkbSize = sizeof( int ) + ringCoordinates.size() * 2 * sizeof( double );
  wkbPtr = QgsWkbPtr( new unsigned char[wkbSize], wkbSize );

  QgsWkbPtr fillPtr( wkbPtr );

  fillPtr << ringCoordinates.size();

  QList<QgsPoint>::const_iterator iter;
  for ( iter = ringCoordinates.constBegin(); iter != ringCoordinates.constEnd(); ++iter )
  {
    fillPtr << iter->x() << iter->y();
  }

  return 0;
}

int QgsGml::createMultiLineFromFragments()
{
  int size = 1 + 2 * sizeof( int ) + totalWKBFragmentSize();
  mCurrentWKB = QgsWkbPtr( new unsigned char[size], size );

  QgsWkbPtr wkbPtr( mCurrentWKB );

  wkbPtr << mEndian << QGis::WKBMultiLineString << mCurrentWKBFragments.constBegin()->size();

  //copy (and delete) all the wkb fragments
  QList<QgsWkbPtr>::const_iterator wkbIt = mCurrentWKBFragments.constBegin()->constBegin();
  for ( ; wkbIt != mCurrentWKBFragments.constBegin()->constEnd(); ++wkbIt )
  {
    memcpy( wkbPtr, *wkbIt, wkbIt->size() );
    wkbPtr += wkbIt->size();
    delete[] *wkbIt;
  }

  mCurrentWKBFragments.clear();
  *mWkbType = QGis::WKBMultiLineString;
  return 0;
}

int QgsGml::createMultiPointFromFragments()
{
  int size = 1 + 2 * sizeof( int ) + totalWKBFragmentSize();
  mCurrentWKB = QgsWkbPtr( new unsigned char[size], size );

  QgsWkbPtr wkbPtr( mCurrentWKB );
  wkbPtr << mEndian << QGis::WKBMultiPoint << mCurrentWKBFragments.constBegin()->size();

  QList<QgsWkbPtr>::const_iterator wkbIt = mCurrentWKBFragments.constBegin()->constBegin();
  for ( ; wkbIt != mCurrentWKBFragments.constBegin()->constEnd(); ++wkbIt )
  {
    memcpy( wkbPtr, *wkbIt, wkbIt->size() );
    wkbPtr += wkbIt->size();
    delete[] *wkbIt;
  }

  mCurrentWKBFragments.clear();
  *mWkbType = QGis::WKBMultiPoint;
  return 0;
}


int QgsGml::createPolygonFromFragments()
{
  int size = 1 + 2 * sizeof( int ) + totalWKBFragmentSize();
  mCurrentWKB = QgsWkbPtr( new unsigned char[size], size );

  QgsWkbPtr wkbPtr( mCurrentWKB );
  wkbPtr << mEndian << QGis::WKBPolygon << mCurrentWKBFragments.constBegin()->size();

  QList<QgsWkbPtr>::const_iterator wkbIt = mCurrentWKBFragments.constBegin()->constBegin();
  for ( ; wkbIt != mCurrentWKBFragments.constBegin()->constEnd(); ++wkbIt )
  {
    memcpy( wkbPtr, *wkbIt, wkbIt->size() );
    wkbPtr += wkbIt->size();
    delete[] *wkbIt;
  }

  mCurrentWKBFragments.clear();
  *mWkbType = QGis::WKBPolygon;
  return 0;
}

int QgsGml::createMultiPolygonFromFragments()
{
  int size = 0;
  size += 1 + 2 * sizeof( int );
  size += totalWKBFragmentSize();
  size += mCurrentWKBFragments.size() * ( 1 + 2 * sizeof( int ) ); //fragments are just the rings

  mCurrentWKB = QgsWkbPtr( new unsigned char[size], size );

  QgsWkbPtr wkbPtr( mCurrentWKB );
  wkbPtr << ( char ) mEndian << QGis::WKBMultiPolygon << mCurrentWKBFragments.size();

  //have outer and inner iterators
  QList< QList<QgsWkbPtr> >::const_iterator outerWkbIt = mCurrentWKBFragments.constBegin();

  for ( ; outerWkbIt != mCurrentWKBFragments.constEnd(); ++outerWkbIt )
  {
    //new polygon
    wkbPtr << ( char ) mEndian << QGis::WKBPolygon << outerWkbIt->size();

    QList<QgsWkbPtr>::const_iterator innerWkbIt = outerWkbIt->constBegin();
    for ( ; innerWkbIt != outerWkbIt->constEnd(); ++innerWkbIt )
    {
      memcpy( wkbPtr, *innerWkbIt, innerWkbIt->size() );
      wkbPtr += innerWkbIt->size();
      delete[] *innerWkbIt;
    }
  }

  mCurrentWKBFragments.clear();
  *mWkbType = QGis::WKBMultiPolygon;
  return 0;
}

int QgsGml::totalWKBFragmentSize() const
{
  int result = 0;
  Q_FOREACH ( const QList<QgsWkbPtr> &list, mCurrentWKBFragments )
  {
    Q_FOREACH ( const QgsWkbPtr &i, list )
    {
      result += i.size();
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

  QgsFeature* currentFeature = nullptr;
  const QgsGeometry* currentGeometry = nullptr;
  bool bboxInitialised = false; //gets true once bbox has been set to the first geometry

  for ( int i = 0; i < mFeatures.size(); ++i )
  {
    currentFeature = mFeatures[i];
    if ( !currentFeature )
    {
      continue;
    }
    currentGeometry = currentFeature->constGeometry();
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

QgsCoordinateReferenceSystem QgsGml::crs() const
{
  QgsCoordinateReferenceSystem crs;
  if ( mEpsg != 0 )
  {
    crs.createFromOgcWmsCrs( QString( "EPSG:%1" ).arg( mEpsg ) );
  }
  return crs;
}
