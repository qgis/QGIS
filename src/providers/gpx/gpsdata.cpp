/***************************************************************************
      gpsdata.cpp  -  Data structures for GPS data
                             -------------------
    begin                : 2004-04-14
    copyright            : (C) 2004 by Lars Luthman
    email                : larsl@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <limits>
#include <stdexcept>
#include <cstring>

#include <QFile>
#include <QTextCodec>
#include <QTextStream>
#include <QObject>
#include <QSet>
#include <QMutexLocker>

#include "gpsdata.h"
#include "qgslogger.h"

#define OUTPUT_PRECISION 12

QString QgsGpsObject::xmlify( const QString &str )
{
  QString tmp = str;
  tmp.replace( '&', QLatin1String( "&amp;" ) );
  tmp.replace( '<', QLatin1String( "&lt;" ) );
  tmp.replace( '>', QLatin1String( "&gt;" ) );
  tmp.replace( '\"', QLatin1String( "&quot;" ) );
  tmp.replace( '\'', QLatin1String( "&apos;" ) );
  return tmp;
}


void QgsGpsObject::writeXml( QTextStream &stream )
{
  if ( !name.isEmpty() )
    stream << "<name>" << xmlify( name ) << "</name>\n";
  if ( !cmt.isEmpty() )
    stream << "<cmt>" << xmlify( cmt ) << "</cmt>\n";
  if ( !desc.isEmpty() )
    stream << "<desc>" << xmlify( desc ) << "</desc>\n";
  if ( !src.isEmpty() )
    stream << "<src>" << xmlify( src ) << "</src>\n";
  if ( !url.isEmpty() )
    stream << "<url>" << xmlify( url ) << "</url>\n";
  if ( !urlname.isEmpty() )
    stream << "<urlname>" << xmlify( urlname ) << "</urlname>\n";
}


QgsGpsPoint::QgsGpsPoint()
{
  ele = -std::numeric_limits<double>::max();
}


void QgsGpsPoint::writeXml( QTextStream &stream )
{
  QgsGpsObject::writeXml( stream );
  if ( ele != -std::numeric_limits<double>::max() )
    stream << "<ele>" << ele << "</ele>\n";
  if ( !sym.isEmpty() )
    stream << "<sym>" << xmlify( sym ) << "</sym>\n";
}


QgsGpsExtended::QgsGpsExtended()
  : xMin( std::numeric_limits<double>::max() )
  , xMax( -std::numeric_limits<double>::max() )
  , yMin( std::numeric_limits<double>::max() )
  , yMax( -std::numeric_limits<double>::max() )
  , number( std::numeric_limits<int>::max() )
{

}


void QgsGpsExtended::writeXml( QTextStream &stream )
{
  QgsGpsObject::writeXml( stream );
  if ( number != std::numeric_limits<int>::max() )
    stream << "<number>" << number << "</number>\n";
}


void QgsWaypoint::writeXml( QTextStream &stream )
{
  stream << "<wpt lat=\"" << QString::number( lat, 'f', OUTPUT_PRECISION ) <<
         "\" lon=\"" << QString::number( lon, 'f', OUTPUT_PRECISION ) << "\">\n";
  QgsGpsPoint::writeXml( stream );
  stream << "</wpt>\n";
}


void QgsRoute::writeXml( QTextStream &stream )
{
  stream << "<rte>\n";
  QgsGpsExtended::writeXml( stream );
  for ( int i = 0; i < points.size(); ++i )
  {
    stream << "<rtept lat=\"" << QString::number( points[i].lat, 'f', OUTPUT_PRECISION )
           << "\" lon=\"" << QString::number( points[i].lon, 'f', OUTPUT_PRECISION ) << "\">\n";
    points[i].writeXml( stream );
    stream << "</rtept>\n";
  }
  stream << "</rte>\n";
}


void QgsTrack::writeXml( QTextStream &stream )
{
  stream << "<trk>\n";
  QgsGpsExtended::writeXml( stream );
  for ( int i = 0; i < segments.size(); ++i )
  {
    stream << "<trkseg>\n";
    for ( int j = 0; j < segments.at( i ).points.size(); ++j )
    {
      stream << "<trkpt lat=\"" <<
             QString::number( segments.at( i ).points.at( j ).lat, 'f', OUTPUT_PRECISION ) <<
             "\" lon=\"" << QString::number( segments.at( i ).points.at( j ).lon, 'f', OUTPUT_PRECISION ) <<
             "\">\n";
      segments[i].points[j].writeXml( stream );
      stream << "</trkpt>\n";
    }
    stream << "</trkseg>\n";
  }
  stream << "</trk>\n";
}


//
// QgsGpsData
//

QgsGpsData::DataMap QgsGpsData::sDataObjects;


#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
QMutex QgsGpsData::sDataObjectsMutex { QMutex::Recursive };
#else
QRecursiveMutex QgsGpsData::sDataObjectsMutex;
#endif

QgsGpsData::QgsGpsData()
{
  xMin = std::numeric_limits<double>::max();
  xMax = -std::numeric_limits<double>::max();
  yMin = std::numeric_limits<double>::max();
  yMax = -std::numeric_limits<double>::max();
  nextWaypoint = 0;
  nextRoute = 0;
  nextTrack = 0;
}


QgsRectangle QgsGpsData::getExtent() const
{
  return QgsRectangle( xMin, yMin, xMax, yMax );
}

void QgsGpsData::setNoDataExtent()
{
  if ( getNumberOfWaypoints() + getNumberOfRoutes() + getNumberOfTracks() == 0 )
  {
    xMin = -1.0;
    xMax = 1.0;
    yMin = -1.0;
    yMax = 1.0;
  }
}

int QgsGpsData::getNumberOfWaypoints() const
{
  return waypoints.size();
}


int QgsGpsData::getNumberOfRoutes() const
{
  return routes.size();
}


int QgsGpsData::getNumberOfTracks() const
{
  return tracks.size();
}


QgsGpsData::WaypointIterator QgsGpsData::waypointsBegin()
{
  return waypoints.begin();
}


QgsGpsData::RouteIterator QgsGpsData::routesBegin()
{
  return routes.begin();
}


QgsGpsData::TrackIterator QgsGpsData::tracksBegin()
{
  return tracks.begin();
}


QgsGpsData::WaypointIterator QgsGpsData::waypointsEnd()
{
  return waypoints.end();
}


QgsGpsData::RouteIterator QgsGpsData::routesEnd()
{
  return routes.end();
}


QgsGpsData::TrackIterator QgsGpsData::tracksEnd()
{
  return tracks.end();
}


QgsGpsData::WaypointIterator QgsGpsData::addWaypoint( double lat, double lon,
    const QString &name, double ele )
{
  QgsWaypoint wpt;
  wpt.lat = lat;
  wpt.lon = lon;
  wpt.name = name;
  wpt.ele = ele;
  return addWaypoint( wpt );
}


QgsGpsData::WaypointIterator QgsGpsData::addWaypoint( const QgsWaypoint &wpt )
{
  xMax = xMax > wpt.lon ? xMax : wpt.lon;
  xMin = xMin < wpt.lon ? xMin : wpt.lon;
  yMax = yMax > wpt.lat ? yMax : wpt.lat;
  yMin = yMin < wpt.lat ? yMin : wpt.lat;
  const WaypointIterator iter = waypoints.insert( waypoints.end(), wpt );
  iter->id = nextWaypoint++;
  return iter;
}


QgsGpsData::RouteIterator QgsGpsData::addRoute( const QString &name )
{
  QgsRoute rte;
  rte.name = name;
  return addRoute( rte );
}


QgsGpsData::RouteIterator QgsGpsData::addRoute( const QgsRoute &rte )
{
  xMax = xMax > rte.xMax ? xMax : rte.xMax;
  xMin = xMin < rte.xMin ? xMin : rte.xMin;
  yMax = yMax > rte.yMax ? yMax : rte.yMax;
  yMin = yMin < rte.yMin ? yMin : rte.yMin;
  const RouteIterator iter = routes.insert( routes.end(), rte );
  iter->id = nextRoute++;
  return iter;
}


QgsGpsData::TrackIterator QgsGpsData::addTrack( const QString &name )
{
  QgsTrack trk;
  trk.name = name;
  return addTrack( trk );
}


QgsGpsData::TrackIterator QgsGpsData::addTrack( const QgsTrack &trk )
{
  xMax = xMax > trk.xMax ? xMax : trk.xMax;
  xMin = xMin < trk.xMin ? xMin : trk.xMin;
  yMax = yMax > trk.yMax ? yMax : trk.yMax;
  yMin = yMin < trk.yMin ? yMin : trk.yMin;
  const TrackIterator iter = tracks.insert( tracks.end(), trk );
  iter->id = nextTrack++;
  return iter;
}


void QgsGpsData::removeWaypoints( const QgsFeatureIds &ids )
{
  QList<QgsFeatureId> ids2 = qgis::setToList( ids );
  std::sort( ids2.begin(), ids2.end() );
  QList<QgsFeatureId>::const_iterator iter = ids2.constBegin();
  WaypointIterator wIter;
  for ( wIter = waypoints.begin();
        wIter != waypoints.end() && iter != ids2.constEnd(); )
  {
    WaypointIterator tmpIter = wIter;
    ++tmpIter;
    if ( wIter->id == *iter )
    {
      waypoints.erase( wIter );
      ++iter;
    }
    wIter = tmpIter;
  }
}


void QgsGpsData::removeRoutes( const QgsFeatureIds &ids )
{
  QList<QgsFeatureId> ids2 = qgis::setToList( ids );
  std::sort( ids2.begin(), ids2.end() );
  QList<QgsFeatureId>::const_iterator iter = ids2.constBegin();
  RouteIterator rIter;
  for ( rIter = routes.begin(); rIter != routes.end() && iter != ids2.constEnd(); )
  {
    RouteIterator tmpIter = rIter;
    ++tmpIter;
    if ( rIter->id == *iter )
    {
      routes.erase( rIter );
      ++iter;
    }
    rIter = tmpIter;
  }
}


void QgsGpsData::removeTracks( const QgsFeatureIds &ids )
{
  QList<QgsFeatureId> ids2 = qgis::setToList( ids );
  std::sort( ids2.begin(), ids2.end() );
  QList<QgsFeatureId>::const_iterator iter = ids2.constBegin();
  TrackIterator tIter;
  for ( tIter = tracks.begin(); tIter != tracks.end() && iter != ids2.constEnd(); )
  {
    TrackIterator tmpIter = tIter;
    ++tmpIter;
    if ( tIter->id == *iter )
    {
      tracks.erase( tIter );
      ++iter;
    }
    tIter = tmpIter;
  }
}


void QgsGpsData::writeXml( QTextStream &stream )
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  stream.setCodec( QTextCodec::codecForName( "UTF8" ) );
#endif
  stream << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
         << "<gpx version=\"1.0\" creator=\"QGIS\">\n";
  for ( WaypointIterator wIter = waypoints.begin();
        wIter != waypoints.end(); ++wIter )
    wIter->writeXml( stream );
  for ( RouteIterator rIter = routes.begin(); rIter != routes.end(); ++rIter )
    rIter->writeXml( stream );
  for ( TrackIterator tIter = tracks.begin(); tIter != tracks.end(); ++tIter )
    tIter->writeXml( stream );
  stream << "</gpx>\n";
  stream.flush();
}


QgsGpsData *QgsGpsData::getData( const QString &fileName )
{
  // if the data isn't there already, try to load it
  const QMutexLocker lock( &sDataObjectsMutex );

  if ( sDataObjects.find( fileName ) == sDataObjects.end() )
  {
    QFile file( fileName );
    if ( !file.open( QIODevice::ReadOnly ) )
    {
      QgsLogger::warning( QObject::tr( "Couldn't open the data source: %1" ).arg( fileName ) );
      return nullptr;
    }
    QgsGpsData *data = new QgsGpsData;
    QgsDebugMsg( "Loading file " + fileName );
    QgsGPXHandler handler( *data );
    bool failed = false;

    // SAX parsing
    XML_Parser p = XML_ParserCreate( nullptr );
    XML_SetUserData( p, &handler );
    XML_SetElementHandler( p, QgsGPXHandler::start, QgsGPXHandler::end );
    XML_SetCharacterDataHandler( p, QgsGPXHandler::chars );
    const long int bufsize = 10 * 1024 * 1024;
    char *buffer = new char[bufsize];
    int atEnd = 0;
    while ( !file.atEnd() )
    {
      const long int readBytes = file.read( buffer, bufsize );
      if ( file.atEnd() )
        atEnd = 1;
      if ( !XML_Parse( p, buffer, readBytes, atEnd ) )
      {
        QgsLogger::warning( QObject::tr( "Parse error at line %1 : %2" )
                            .arg( XML_GetCurrentLineNumber( p ) )
                            .arg( XML_ErrorString( XML_GetErrorCode( p ) ) ) );
        failed = true;
        break;
      }
    }
    delete [] buffer;
    XML_ParserFree( p );
    if ( failed )
      return nullptr;

    data->setNoDataExtent();

    sDataObjects[fileName] = qMakePair( data, 0 );
  }
  else
  {
    QgsDebugMsg( fileName + " is already loaded" );
  }

  // return a pointer and increase the reference count for that file name
  const DataMap::iterator iter = sDataObjects.find( fileName );
  ++( iter.value().second );
  return ( QgsGpsData * )( iter.value().first );
}


void QgsGpsData::releaseData( const QString &fileName )
{
  const QMutexLocker lock( &sDataObjectsMutex );

  /* decrease the reference count for the file name (if it is used), and erase
     it if the reference count becomes 0 */
  const DataMap::iterator iter = sDataObjects.find( fileName );
  if ( iter != sDataObjects.end() )
  {
    QgsDebugMsg( "unrefing " + fileName );
    if ( --( iter.value().second ) == 0 )
    {
      QgsDebugMsg( "No one's using " + fileName + ", I'll erase it" );
      delete iter.value().first;
      sDataObjects.erase( iter );
    }
  }
}


bool QgsGPXHandler::startElement( const XML_Char *qName, const XML_Char **attr )
{

  if ( !std::strcmp( qName, "gpx" ) )
  {
    parseModes.push( ParsingDocument );
    mData = QgsGpsData();
  }

  // top level objects
  else if ( !std::strcmp( qName, "wpt" ) )
  {
    parseModes.push( ParsingWaypoint );
    mWpt = QgsWaypoint();
    for ( int i = 0; attr[2 * i]; ++i )
    {
      if ( !std::strcmp( attr[2 * i], "lat" ) )
        mWpt.lat = QString( attr[2 * i + 1] ).toDouble();
      else if ( !std::strcmp( attr[2 * i], "lon" ) )
        mWpt.lon = QString( attr[2 * i + 1] ).toDouble();
    }
    mObj = &mWpt;
  }
  else if ( !std::strcmp( qName, "rte" ) )
  {
    parseModes.push( ParsingRoute );
    mRte = QgsRoute();
    mObj = &mRte;
  }
  else if ( !std::strcmp( qName, "trk" ) )
  {
    parseModes.push( ParsingTrack );
    mTrk = QgsTrack();
    mObj = &mTrk;
  }

  // common properties
  else if ( !std::strcmp( qName, "name" ) )
  {
    if ( parseModes.top() == ParsingWaypoint ||
         parseModes.top() == ParsingRoute ||
         parseModes.top() == ParsingTrack )
    {
      mString = &mObj->name;
      mCharBuffer.clear();
      parseModes.push( ParsingString );
    }
    else
      parseModes.push( ParsingUnknown );
  }
  else if ( !std::strcmp( qName, "cmt" ) )
  {
    if ( parseModes.top() == ParsingWaypoint ||
         parseModes.top() == ParsingRoute ||
         parseModes.top() == ParsingTrack )
    {
      mString = &mObj->cmt;
      mCharBuffer.clear();
      parseModes.push( ParsingString );
    }
    else
      parseModes.push( ParsingUnknown );
  }
  else if ( !std::strcmp( qName, "desc" ) )
  {
    if ( parseModes.top() == ParsingWaypoint ||
         parseModes.top() == ParsingRoute ||
         parseModes.top() == ParsingTrack )
    {
      mString = &mObj->desc;
      mCharBuffer.clear();
      parseModes.push( ParsingString );
    }
    else
      parseModes.push( ParsingUnknown );
  }
  else if ( !std::strcmp( qName, "src" ) )
  {
    if ( parseModes.top() == ParsingWaypoint ||
         parseModes.top() == ParsingRoute ||
         parseModes.top() == ParsingTrack )
    {
      mString = &mObj->src;
      mCharBuffer.clear();
      parseModes.push( ParsingString );
    }
    else
      parseModes.push( ParsingUnknown );
  }
  else if ( !std::strcmp( qName, "url" ) )
  {
    if ( parseModes.top() == ParsingWaypoint ||
         parseModes.top() == ParsingRoute ||
         parseModes.top() == ParsingTrack )
    {
      mString = &mObj->url;
      mCharBuffer.clear();
      parseModes.push( ParsingString );
    }
    else
      parseModes.push( ParsingUnknown );
  }
  else if ( !std::strcmp( qName, "urlname" ) )
  {
    if ( parseModes.top() == ParsingWaypoint ||
         parseModes.top() == ParsingRoute ||
         parseModes.top() == ParsingTrack )
    {
      mString = &mObj->urlname;
      mCharBuffer.clear();
      parseModes.push( ParsingString );
    }
    else
      parseModes.push( ParsingUnknown );
  }

  // waypoint-specific attributes
  else if ( !std::strcmp( qName, "ele" ) )
  {
    if ( parseModes.top() == ParsingWaypoint )
    {
      mDouble = &mWpt.ele;
      mCharBuffer.clear();
      parseModes.push( ParsingDouble );
    }
    else
      parseModes.push( ParsingUnknown );
  }
  else if ( !std::strcmp( qName, "sym" ) )
  {
    if ( parseModes.top() == ParsingWaypoint )
    {
      mString = &mWpt.sym;
      mCharBuffer.clear();
      parseModes.push( ParsingString );
    }
    else
      parseModes.push( ParsingUnknown );
  }

  // route/track-specific attributes
  else if ( !std::strcmp( qName, "number" ) )
  {
    if ( parseModes.top() == ParsingRoute )
    {
      mInt = &mRte.number;
      mCharBuffer.clear();
      parseModes.push( ParsingInt );
    }
    else if ( parseModes.top() == ParsingTrack )
    {
      mInt = &mTrk.number;
      parseModes.push( ParsingInt );
    }
    else
      parseModes.push( ParsingUnknown );
  }

  // route points
  else if ( !std::strcmp( qName, "rtept" ) )
  {
    if ( parseModes.top() == ParsingRoute )
    {
      mRtept = QgsRoutepoint();
      for ( int i = 0; attr[2 * i]; ++i )
      {
        if ( !std::strcmp( attr[2 * i], "lat" ) )
          mRtept.lat = QString( attr[2 * i + 1] ).toDouble();
        else if ( !std::strcmp( attr[2 * i], "lon" ) )
          mRtept.lon = QString( attr[2 * i + 1] ).toDouble();
      }
      parseModes.push( ParsingRoutepoint );
    }
    else
      parseModes.push( ParsingUnknown );
  }

  // track segments and points
  else if ( !std::strcmp( qName, "trkseg" ) )
  {
    if ( parseModes.top() == ParsingTrack )
    {
      mTrkseg = QgsTrackSegment();
      parseModes.push( ParsingTrackSegment );
    }
    else
      parseModes.push( ParsingUnknown );
  }
  else if ( !std::strcmp( qName, "trkpt" ) )
  {
    if ( parseModes.top() == ParsingTrackSegment )
    {
      mTrkpt = QgsTrackpoint();
      for ( int i = 0; attr[2 * i]; ++i )
      {
        if ( !std::strcmp( attr[2 * i], "lat" ) )
          mTrkpt.lat = QString( attr[2 * i + 1] ).toDouble();
        else if ( !std::strcmp( attr[2 * i], "lon" ) )
          mTrkpt.lon = QString( attr[2 * i + 1] ).toDouble();
      }
      parseModes.push( ParsingTrackpoint );
    }
    else
      parseModes.push( ParsingUnknown );
  }

  // unknown
  else
    parseModes.push( ParsingUnknown );

  return true;
}


void QgsGPXHandler::characters( const XML_Char *chars, int len )
{
  // This is horrible.
#ifdef XML_UNICODE
  for ( int i = 0; i < len; ++i )
    mCharBuffer += QChar( chars[i] );
#else
  mCharBuffer += QString::fromUtf8( chars, len );
#endif
}


bool QgsGPXHandler::endElement( const std::string &qName )
{
  Q_UNUSED( qName )

  if ( parseModes.top() == ParsingWaypoint )
  {
    mData.addWaypoint( mWpt );
  }
  else if ( parseModes.top() == ParsingRoute )
  {
    mData.addRoute( mRte );
  }
  else if ( parseModes.top() == ParsingTrack )
  {
    mData.addTrack( mTrk );
  }
  else if ( parseModes.top() == ParsingRoutepoint )
  {
    mRte.points.push_back( mRtept );
    mRte.xMin = ( mRte.xMin < mRtept.lon ? mRte.xMin : mRtept.lon );
    mRte.xMax = ( mRte.xMax > mRtept.lon ? mRte.xMax : mRtept.lon );
    mRte.yMin = ( mRte.yMin < mRtept.lat ? mRte.yMin : mRtept.lat );
    mRte.yMax = ( mRte.yMax > mRtept.lat ? mRte.yMax : mRtept.lat );
  }
  else if ( parseModes.top() == ParsingTrackSegment )
  {
    mTrk.segments.push_back( mTrkseg );
  }
  else if ( parseModes.top() == ParsingTrackpoint )
  {
    mTrkseg.points.push_back( mTrkpt );
    mTrk.xMin = ( mTrk.xMin < mTrkpt.lon ? mTrk.xMin : mTrkpt.lon );
    mTrk.xMax = ( mTrk.xMax > mTrkpt.lon ? mTrk.xMax : mTrkpt.lon );
    mTrk.yMin = ( mTrk.yMin < mTrkpt.lat ? mTrk.yMin : mTrkpt.lat );
    mTrk.yMax = ( mTrk.yMax > mTrkpt.lat ? mTrk.yMax : mTrkpt.lat );
  }
  else if ( parseModes.top() == ParsingDouble )
  {
    *mDouble = QString( mCharBuffer ).toDouble();
    mCharBuffer.clear();
  }
  else if ( parseModes.top() == ParsingInt )
  {
    *mInt = QString( mCharBuffer ).toInt();
    mCharBuffer.clear();
  }
  else if ( parseModes.top() == ParsingString )
  {
    *mString = mCharBuffer;
    mCharBuffer.clear();
  }
  parseModes.pop();

  return true;
}

