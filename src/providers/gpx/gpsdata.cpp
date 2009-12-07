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

#include "gpsdata.h"
#include <qgslogger.h>

#define OUTPUT_PRECISION 12

QString QgsGPSObject::xmlify( const QString& str )
{
  QString tmp = str;
  tmp.replace( "&", "&amp;" );
  tmp.replace( "<", "&lt;" );
  tmp.replace( ">", "&gt;" );
  tmp.replace( "\"", "&quot;" );
  tmp.replace( "\'", "&apos;" );
  return tmp;
}


void QgsGPSObject::writeXML( QTextStream& stream )
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


QgsGPSPoint::QgsGPSPoint()
{
  ele = -std::numeric_limits<double>::max();
}


void QgsGPSPoint::writeXML( QTextStream& stream )
{
  QgsGPSObject::writeXML( stream );
  if ( ele != -std::numeric_limits<double>::max() )
    stream << "<ele>" << ele << "</ele>\n";
  if ( !sym.isEmpty() )
    stream << "<sym>" << xmlify( sym ) << "</sym>\n";
}


QgsGPSExtended::QgsGPSExtended()
    : xMin( std::numeric_limits<double>::max() ),
    xMax( -std::numeric_limits<double>::max() ),
    yMin( std::numeric_limits<double>::max() ),
    yMax( -std::numeric_limits<double>::max() ),
    number( std::numeric_limits<int>::max() )
{

}


void QgsGPSExtended::writeXML( QTextStream& stream )
{
  QgsGPSObject::writeXML( stream );
  if ( number != std::numeric_limits<int>::max() )
    stream << "<number>" << number << "</number>\n";
}


void QgsWaypoint::writeXML( QTextStream& stream )
{
  stream << "<wpt lat=\"" << QString::number( lat, 'f', OUTPUT_PRECISION ) <<
  "\" lon=\"" << QString::number( lon, 'f', OUTPUT_PRECISION ) << "\">\n";
  QgsGPSPoint::writeXML( stream );
  stream << "</wpt>\n";
}


void QgsRoute::writeXML( QTextStream& stream )
{
  stream << "<rte>\n";
  QgsGPSExtended::writeXML( stream );
  for ( unsigned int i = 0; i < points.size(); ++i )
  {
    stream << "<rtept lat=\"" << QString::number( points[i].lat, 'f', OUTPUT_PRECISION )
    << "\" lon=\"" << QString::number( points[i].lon, 'f', OUTPUT_PRECISION ) << "\">\n";
    points[i].writeXML( stream );
    stream << "</rtept>\n";
  }
  stream << "</rte>\n";
}


void QgsTrack::writeXML( QTextStream& stream )
{
  stream << "<trk>\n";
  QgsGPSExtended::writeXML( stream );
  for ( unsigned int i = 0; i < segments.size(); ++i )
  {
    stream << "<trkseg>\n";
    for ( unsigned int j = 0; j < segments[i].points.size(); ++j )
    {
      stream << "<trkpt lat=\"" <<
      QString::number( segments[i].points[j].lat, 'f', OUTPUT_PRECISION ) <<
      "\" lon=\"" << QString::number( segments[i].points[j].lon, 'f', OUTPUT_PRECISION ) <<
      "\">\n";
      segments[i].points[j].writeXML( stream );
      stream << "</trkpt>\n";
    }
    stream << "</trkseg>\n";
  }
  stream << "</trk>\n";
}


QgsGPSData::QgsGPSData()
{
  xMin = std::numeric_limits<double>::max();
  xMax = -std::numeric_limits<double>::max();
  yMin = std::numeric_limits<double>::max();
  yMax = -std::numeric_limits<double>::max();
  nextWaypoint = 0;
  nextRoute = 0;
  nextTrack = 0;
}


QgsRectangle QgsGPSData::getExtent() const
{
  return QgsRectangle( xMin, yMin, xMax, yMax );
}

void QgsGPSData::setNoDataExtent()
{
  if ( getNumberOfWaypoints() + getNumberOfRoutes() + getNumberOfTracks() == 0 )
  {
    xMin = -1.0;
    xMax =  1.0;
    yMin = -1.0;
    yMax =  1.0;
  }
}

int QgsGPSData::getNumberOfWaypoints() const
{
  return waypoints.size();
}


int QgsGPSData::getNumberOfRoutes() const
{
  return routes.size();
}


int QgsGPSData::getNumberOfTracks() const
{
  return tracks.size();
}


QgsGPSData::WaypointIterator QgsGPSData::waypointsBegin()
{
  return waypoints.begin();
}


QgsGPSData::RouteIterator QgsGPSData::routesBegin()
{
  return routes.begin();
}


QgsGPSData::TrackIterator QgsGPSData::tracksBegin()
{
  return tracks.begin();
}


QgsGPSData::WaypointIterator QgsGPSData::waypointsEnd()
{
  return waypoints.end();
}


QgsGPSData::RouteIterator QgsGPSData::routesEnd()
{
  return routes.end();
}


QgsGPSData::TrackIterator QgsGPSData::tracksEnd()
{
  return tracks.end();
}


QgsGPSData::WaypointIterator QgsGPSData::addWaypoint( double lat, double lon,
    QString name, double ele )
{
  QgsWaypoint wpt;
  wpt.lat = lat;
  wpt.lon = lon;
  wpt.name = name;
  wpt.ele = ele;
  return addWaypoint( wpt );
}


QgsGPSData::WaypointIterator QgsGPSData::addWaypoint( const QgsWaypoint& wpt )
{
  xMax = xMax > wpt.lon ? xMax : wpt.lon;
  xMin = xMin < wpt.lon ? xMin : wpt.lon;
  yMax = yMax > wpt.lat ? yMax : wpt.lat;
  yMin = yMin < wpt.lat ? yMin : wpt.lat;
  WaypointIterator iter = waypoints.insert( waypoints.end(), wpt );
  iter->id = nextWaypoint++;
  return iter;
}


QgsGPSData::RouteIterator QgsGPSData::addRoute( QString name )
{
  QgsRoute rte;
  rte.name = name;
  return addRoute( rte );
}


QgsGPSData::RouteIterator QgsGPSData::addRoute( const QgsRoute& rte )
{
  xMax = xMax > rte.xMax ? xMax : rte.xMax;
  xMin = xMin < rte.xMin ? xMin : rte.xMin;
  yMax = yMax > rte.yMax ? yMax : rte.yMax;
  yMin = yMin < rte.yMin ? yMin : rte.yMin;
  RouteIterator iter = routes.insert( routes.end(), rte );
  iter->id = nextRoute++;
  return iter;
}


QgsGPSData::TrackIterator QgsGPSData::addTrack( QString name )
{
  QgsTrack trk;
  trk.name = name;
  return addTrack( trk );
}


QgsGPSData::TrackIterator QgsGPSData::addTrack( const QgsTrack& trk )
{
  xMax = xMax > trk.xMax ? xMax : trk.xMax;
  xMin = xMin < trk.xMin ? xMin : trk.xMin;
  yMax = yMax > trk.yMax ? yMax : trk.yMax;
  yMin = yMin < trk.yMin ? yMin : trk.yMin;
  TrackIterator iter = tracks.insert( tracks.end(), trk );
  iter->id = nextTrack++;
  return iter;
}


void QgsGPSData::removeWaypoints( const QgsFeatureIds & ids )
{
  QList<int> ids2 = ids.toList();
  qSort( ids2 );
  QList<int>::const_iterator iter = ids2.begin();
  WaypointIterator wIter;
  for ( wIter = waypoints.begin();
        wIter != waypoints.end() && iter != ids2.end(); )
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


void QgsGPSData::removeRoutes( const QgsFeatureIds & ids )
{
  QList<int> ids2 = ids.toList();
  qSort( ids2 );
  QList<int>::const_iterator iter = ids2.begin();
  RouteIterator rIter;
  for ( rIter = routes.begin(); rIter != routes.end() && iter != ids2.end(); )
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


void QgsGPSData::removeTracks( const QgsFeatureIds & ids )
{
  QList<int> ids2 = ids.toList();
  qSort( ids2 );
  QList<int>::const_iterator iter = ids2.begin();
  TrackIterator tIter;
  for ( tIter = tracks.begin(); tIter != tracks.end() && iter != ids2.end(); )
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


void QgsGPSData::writeXML( QTextStream& stream )
{
  stream.setCodec( QTextCodec::codecForName( "UTF8" ) );
  stream << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
  << "<gpx version=\"1.0\" creator=\"Quantum GIS\">\n";
  for ( WaypointIterator wIter = waypoints.begin();
        wIter != waypoints.end(); ++wIter )
    wIter->writeXML( stream );
  for ( RouteIterator rIter = routes.begin(); rIter != routes.end(); ++rIter )
    rIter->writeXML( stream );
  for ( TrackIterator tIter = tracks.begin(); tIter != tracks.end(); ++tIter )
    tIter->writeXML( stream );
  stream << "</gpx>\n";
  stream << flush;
}


QgsGPSData* QgsGPSData::getData( const QString& fileName )
{
  // if the data isn't there already, try to load it
  if ( dataObjects.find( fileName ) == dataObjects.end() )
  {
    QFile file( fileName );
    if ( !file.open( QIODevice::ReadOnly ) )
    {
      QgsLogger::warning( QObject::tr( "Couldn't open the data source: %1" ).arg( fileName ) );
      return 0;
    }
    QgsGPSData* data = new QgsGPSData;
    QgsLogger::debug( "Loading file " + fileName );
    QgsGPXHandler handler( *data );
    bool failed = false;

    // SAX parsing
    XML_Parser p = XML_ParserCreate( NULL );
    XML_SetUserData( p, &handler );
    XML_SetElementHandler( p, QgsGPXHandler::start, QgsGPXHandler::end );
    XML_SetCharacterDataHandler( p, QgsGPXHandler::chars );
    long int bufsize = 10 * 1024 * 1024;
    char* buffer = new char[bufsize];
    int atEnd = 0;
    while ( !file.atEnd() )
    {
      long int readBytes = file.read( buffer, bufsize );
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
      return 0;

    data->setNoDataExtent();

    dataObjects[fileName] = std::pair<QgsGPSData*, unsigned>( data, 0 );
  }
  else
    QgsLogger::debug( fileName + " is already loaded" );

  // return a pointer and increase the reference count for that file name
  DataMap::iterator iter = dataObjects.find( fileName );
  ++( iter->second.second );
  return ( QgsGPSData* )( iter->second.first );
}


void QgsGPSData::releaseData( const QString& fileName )
{

  /* decrease the reference count for the file name (if it is used), and erase
     it if the reference count becomes 0 */
  DataMap::iterator iter = dataObjects.find( fileName );
  if ( iter != dataObjects.end() )
  {
    QgsLogger::debug( "unrefing " + fileName );
    if ( --( iter->second.second ) == 0 )
    {
      QgsLogger::debug( "No one's using " + fileName + ", I'll erase it" );
      delete iter->second.first;
      dataObjects.erase( iter );
    }
  }
}


// we have to initialize the static member
QgsGPSData::DataMap QgsGPSData::dataObjects;




bool QgsGPXHandler::startElement( const XML_Char* qName, const XML_Char** attr )
{

  if ( !std::strcmp( qName, "gpx" ) )
  {
    parseModes.push( ParsingDocument );
    mData = QgsGPSData();
  }

  // top level objects
  else if ( !std::strcmp( qName, "wpt" ) )
  {
    parseModes.push( ParsingWaypoint );
    mWpt = QgsWaypoint();
    for ( int i = 0; attr[2*i] != NULL; ++i )
    {
      if ( !std::strcmp( attr[2*i], "lat" ) )
        mWpt.lat = QString( attr[2*i+1] ).toDouble();
      else if ( !std::strcmp( attr[2*i], "lon" ) )
        mWpt.lon = QString( attr[2*i+1] ).toDouble();
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
      mCharBuffer = "";
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
      mCharBuffer = "";
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
      mCharBuffer = "";
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
      mCharBuffer = "";
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
      mCharBuffer = "";
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
      mCharBuffer = "";
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
      mCharBuffer = "";
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
      mCharBuffer = "";
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
      mCharBuffer = "";
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
      for ( int i = 0; attr[2*i] != NULL; ++i )
      {
        if ( !std::strcmp( attr[2*i], "lat" ) )
          mRtept.lat = QString( attr[2*i+1] ).toDouble();
        else if ( !std::strcmp( attr[2*i], "lon" ) )
          mRtept.lon = QString( attr[2*i+1] ).toDouble();
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
      for ( int i = 0; attr[2*i] != NULL; ++i )
      {
        if ( !std::strcmp( attr[2*i], "lat" ) )
          mTrkpt.lat = QString( attr[2*i+1] ).toDouble();
        else if ( !std::strcmp( attr[2*i], "lon" ) )
          mTrkpt.lon = QString( attr[2*i+1] ).toDouble();
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


void QgsGPXHandler::characters( const XML_Char* chars, int len )
{
  // This is horrible.
#ifdef XML_UNICODE
  for ( int i = 0; i < len; ++i )
    mCharBuffer += QChar( chars[i] );
#else
  mCharBuffer += QString::fromUtf8( chars, len );
#endif
}


bool QgsGPXHandler::endElement( const std::string& qName )
{
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
    mCharBuffer = "";
  }
  else if ( parseModes.top() == ParsingInt )
  {
    *mInt = QString( mCharBuffer ).toInt();
    mCharBuffer = "";
  }
  else if ( parseModes.top() == ParsingString )
  {
    *mString = mCharBuffer;
    mCharBuffer = "";
  }
  parseModes.pop();

  return true;
}

