/***************************************************************************
      qgsgpxprovider.cpp  -  Data provider for GPS eXchange files
                             -------------------
    begin                : 2004-04-14
    copyright            : (C) 2004 by Lars Luthman
    email                : larsl@users.sourceforge.net

    Partly based on qgsdelimitedtextprovider.cpp, (C) 2004 Gary E. Sherman
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <algorithm>
#include <iostream>
#include <limits>
#include <cstring>
#include <cmath>

// Changed #include <qapp.h> to <qapplication.h>. Apparently some
// debian distros do not include the qapp.h wrapper and the compilation
// fails. [gsherman]
#include <QApplication>
#include <QFile>
#include <QTextCodec>
#include <QTextStream>
#include <QObject>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsdataprovider.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsrectangle.h"
#include "qgsgpxprovider.h"
#include "gpsdata.h"
#include "qgslogger.h"

const char* QgsGPXProvider::attr[] = { "name", "elevation", "symbol", "number",
                                       "comment", "description", "source",
                                       "url", "url name"
                                     };


const QString GPX_KEY = "gpx";

const QString GPX_DESCRIPTION = QObject::tr( "GPS eXchange format provider" );


QgsGPXProvider::QgsGPXProvider( QString uri ) :
    QgsVectorDataProvider( uri )
{
  // assume that it won't work
  mValid = false;

  // we always use UTF-8
  mEncoding = QTextCodec::codecForName( "utf8" );

  // get the file name and the type parameter from the URI
  int fileNameEnd = uri.indexOf( '?' );
  if ( fileNameEnd == -1 || uri.mid( fileNameEnd + 1, 5 ) != "type=" )
  {
    QgsLogger::warning( tr( "Bad URI - you need to specify the feature type." ) );
    return;
  }
  QString typeStr = uri.mid( fileNameEnd + 6 );
  mFeatureType = ( typeStr == "waypoint" ? WaypointType :
                   ( typeStr == "route" ? RouteType : TrackType ) );

  // set up the attributes and the geometry type depending on the feature type
  attributeFields[NameAttr] = QgsField( attr[NameAttr], QVariant::String, "text" );
  if ( mFeatureType == WaypointType )
  {
    attributeFields[EleAttr] = QgsField( attr[EleAttr], QVariant::Double, "double" );
    attributeFields[SymAttr] = QgsField( attr[SymAttr], QVariant::String, "text" );
  }
  else if ( mFeatureType == RouteType || mFeatureType == TrackType )
  {
    attributeFields[NumAttr] = QgsField( attr[NumAttr], QVariant::Int, "int" );
  }
  attributeFields[CmtAttr] = QgsField( attr[CmtAttr], QVariant::String, "text" );
  attributeFields[DscAttr] = QgsField( attr[DscAttr], QVariant::String, "text" );
  attributeFields[SrcAttr] = QgsField( attr[SrcAttr], QVariant::String, "text" );
  attributeFields[URLAttr] = QgsField( attr[URLAttr], QVariant::String, "text" );
  attributeFields[URLNameAttr] = QgsField( attr[URLNameAttr], QVariant::String, "text" );
  mFileName = uri.left( fileNameEnd );

  // set the selection rectangle to null
  mSelectionRectangle = 0;

  // parse the file
  data = QgsGPSData::getData( mFileName );
  if ( data == 0 )
  {
    return;
  }

  mValid = true;
}


QgsGPXProvider::~QgsGPXProvider()
{
  QgsGPSData::releaseData( mFileName );
  delete mSelectionRectangle;
}


QString QgsGPXProvider::storageType() const
{
  return tr( "GPS eXchange file" );
}

int QgsGPXProvider::capabilities() const
{
  return QgsVectorDataProvider::AddFeatures |
         QgsVectorDataProvider::DeleteFeatures |
         QgsVectorDataProvider::ChangeAttributeValues;
}

bool QgsGPXProvider::nextFeature( QgsFeature& feature )
{
  feature.setValid( false );
  bool result = false;

  QgsAttributeList::const_iterator iter;

  if ( mFeatureType == WaypointType )
  {
    // go through the list of waypoints and return the first one that is in
    // the bounds rectangle
    for ( ; mWptIter != data->waypointsEnd(); ++mWptIter )
    {
      const QgsWaypoint* wpt;
      wpt = &( *mWptIter );
      if ( boundsCheck( wpt->lon, wpt->lat ) )
      {
        feature.setFeatureId( wpt->id );
        result = true;

        // some wkb voodoo
        if ( mFetchGeom )
        {
          char* geo = new char[21];
          std::memset( geo, 0, 21 );
          geo[0] = QgsApplication::endian();
          geo[geo[0] == QgsApplication::NDR ? 1 : 4] = QGis::WKBPoint;
          std::memcpy( geo + 5, &wpt->lon, sizeof( double ) );
          std::memcpy( geo + 13, &wpt->lat, sizeof( double ) );
          feature.setGeometryAndOwnership(( unsigned char * )geo, sizeof( wkbPoint ) );
        }
        feature.setValid( true );

        // add attributes if they are wanted
        for ( iter = mAttributesToFetch.begin(); iter != mAttributesToFetch.end(); ++iter )
        {
          switch ( *iter )
          {
            case NameAttr:
              feature.addAttribute( NameAttr, QVariant( wpt->name ) );
              break;
            case EleAttr:
              if ( wpt->ele != -std::numeric_limits<double>::max() )
                feature.addAttribute( EleAttr, QVariant( wpt->ele ) );
              break;
            case SymAttr:
              feature.addAttribute( SymAttr, QVariant( wpt->sym ) );
              break;
            case CmtAttr:
              feature.addAttribute( CmtAttr, QVariant( wpt->cmt ) );
              break;
            case DscAttr:
              feature.addAttribute( DscAttr, QVariant( wpt->desc ) );
              break;
            case SrcAttr:
              feature.addAttribute( SrcAttr, QVariant( wpt->src ) );
              break;
            case URLAttr:
              feature.addAttribute( URLAttr, QVariant( wpt->url ) );
              break;
            case URLNameAttr:
              feature.addAttribute( URLNameAttr, QVariant( wpt->urlname ) );
              break;
          }
        }

        ++mWptIter;
        break;
      }
    }
  }

  else if ( mFeatureType == RouteType )
  {
    // go through the routes and return the first one that is in the bounds
    // rectangle
    for ( ; mRteIter != data->routesEnd(); ++mRteIter )
    {
      const QgsRoute* rte;
      rte = &( *mRteIter );

      if ( rte->points.size() == 0 )
        continue;
      const QgsRectangle& b( *mSelectionRectangle );
      if (( rte->xMax >= b.xMinimum() ) && ( rte->xMin <= b.xMaximum() ) &&
          ( rte->yMax >= b.yMinimum() ) && ( rte->yMin <= b.yMaximum() ) )
      {
        // some wkb voodoo
        int nPoints = rte->points.size();
        char* geo = new char[9 + 16 * nPoints];
        std::memset( geo, 0, 9 + 16 * nPoints );
        geo[0] = QgsApplication::endian();
        geo[geo[0] == QgsApplication::NDR ? 1 : 4] = QGis::WKBLineString;
        std::memcpy( geo + 5, &nPoints, 4 );
        for ( uint i = 0; i < rte->points.size(); ++i )
        {
          std::memcpy( geo + 9 + 16 * i, &rte->points[i].lon, sizeof( double ) );
          std::memcpy( geo + 9 + 16 * i + 8, &rte->points[i].lat, sizeof( double ) );
        }

        //create QgsGeometry and use it for intersection test
        //if geometry is to be fetched, it is attached to the feature, otherwise we delete it
        QgsGeometry* theGeometry = new QgsGeometry();
        theGeometry->fromWkb(( unsigned char * )geo, 9 + 16 * nPoints );
        bool intersection = theGeometry->intersects( b );//use geos for precise intersection test

        if ( !intersection )
        {
          delete theGeometry;
        }
        else
        {
          if ( mFetchGeom )
          {
            feature.setGeometry( theGeometry );
          }
          else
          {
            delete theGeometry;
          }
          feature.setFeatureId( rte->id );
          result = true;
          feature.setValid( true );

          // add attributes if they are wanted
          for ( iter = mAttributesToFetch.begin(); iter != mAttributesToFetch.end(); ++iter )
          {
            switch ( *iter )
            {
              case NameAttr:
                feature.addAttribute( NameAttr, QVariant( rte->name ) );
                break;
              case NumAttr:
                if ( rte->number != std::numeric_limits<int>::max() )
                  feature.addAttribute( NumAttr, QVariant( rte->number ) );
                break;
              case CmtAttr:
                feature.addAttribute( CmtAttr, QVariant( rte->cmt ) );
                break;
              case DscAttr:
                feature.addAttribute( DscAttr, QVariant( rte->desc ) );
                break;
              case SrcAttr:
                feature.addAttribute( SrcAttr, QVariant( rte->src ) );
                break;
              case URLAttr:
                feature.addAttribute( URLAttr, QVariant( rte->url ) );
                break;
              case URLNameAttr:
                feature.addAttribute( URLNameAttr, QVariant( rte->urlname ) );
                break;
            }
          }

          ++mRteIter;
          break;

        }

        //++mRteIter;
        //xbreak;
      }
    }
  }

  else if ( mFeatureType == TrackType )
  {
    // go through the tracks and return the first one that is in the bounds
    // rectangle
    for ( ; mTrkIter != data->tracksEnd(); ++mTrkIter )
    {
      const QgsTrack* trk;
      trk = &( *mTrkIter );

      QgsDebugMsg( QString( "GPX feature track segments: %1" ).arg( trk->segments.size() ) );
      if ( trk->segments.size() == 0 )
        continue;

      // A track consists of several segments. Add all those segments into one.
      int totalPoints = 0;;
      for ( std::vector<QgsTrackSegment>::size_type i = 0; i < trk->segments.size(); i ++ )
      {
        totalPoints += trk->segments[i].points.size();
      }
      if ( totalPoints == 0 )
        continue;
      QgsDebugMsg( "GPX feature track total points: " + QString::number( totalPoints ) );
      const QgsRectangle& b( *mSelectionRectangle );
      if (( trk->xMax >= b.xMinimum() ) && ( trk->xMin <= b.xMaximum() ) &&
          ( trk->yMax >= b.yMinimum() ) && ( trk->yMin <= b.yMaximum() ) )
      {
        // some wkb voodoo
        char* geo = new char[9 + 16 * totalPoints];
        if ( !geo )
        {
          QgsDebugMsg( "Too large track!!!" );
          return false;
        }
        std::memset( geo, 0, 9 + 16 * totalPoints );
        geo[0] = QgsApplication::endian();
        geo[geo[0] == QgsApplication::NDR ? 1 : 4] = QGis::WKBLineString;
        std::memcpy( geo + 5, &totalPoints, 4 );

        int thisPoint = 0;
        for ( std::vector<QgsTrackSegment>::size_type k = 0; k < trk->segments.size(); k++ )
        {
          int nPoints = trk->segments[k].points.size();
          for ( int i = 0; i < nPoints; ++i )
          {
            std::memcpy( geo + 9 + 16 * thisPoint,     &trk->segments[k].points[i].lon, sizeof( double ) );
            std::memcpy( geo + 9 + 16 * thisPoint + 8, &trk->segments[k].points[i].lat, sizeof( double ) );
            thisPoint++;
          }
        }

        //create QgsGeometry and use it for intersection test
        //if geometry is to be fetched, it is attached to the feature, otherwise we delete it
        QgsGeometry* theGeometry = new QgsGeometry();
        theGeometry->fromWkb(( unsigned char * )geo, 9 + 16 * totalPoints );
        bool intersection = theGeometry->intersects( b );//use geos for precise intersection test

        if ( !intersection ) //no intersection, delete geometry and move on
        {
          delete theGeometry;
        }
        else //intersection
        {
          if ( mFetchGeom )
          {
            feature.setGeometry( theGeometry );
          }
          else
          {
            delete theGeometry;
          }
          feature.setFeatureId( trk->id );
          result = true;

          feature.setValid( true );

          // add attributes if they are wanted
          for ( iter = mAttributesToFetch.begin(); iter != mAttributesToFetch.end(); ++iter )
          {
            switch ( *iter )
            {
              case NameAttr:
                feature.addAttribute( NameAttr, QVariant( trk->name ) );
                break;
              case NumAttr:
                if ( trk->number != std::numeric_limits<int>::max() )
                  feature.addAttribute( NumAttr, QVariant( trk->number ) );
                break;
              case CmtAttr:
                feature.addAttribute( CmtAttr, QVariant( trk->cmt ) );
                break;
              case DscAttr:
                feature.addAttribute( DscAttr, QVariant( trk->desc ) );
                break;
              case SrcAttr:
                feature.addAttribute( SrcAttr, QVariant( trk->src ) );
                break;
              case URLAttr:
                feature.addAttribute( URLAttr, QVariant( trk->url ) );
                break;
              case URLNameAttr:
                feature.addAttribute( URLNameAttr, QVariant( trk->urlname ) );
                break;
            }
          }

          ++mTrkIter;
          break;
        }
      }

    }
  }
  if ( result )
  {
    feature.setValid( true );
  }
  return result;
}

void QgsGPXProvider::select( QgsAttributeList fetchAttributes,
                             QgsRectangle rect,
                             bool fetchGeometry,
                             bool useIntersect )
{
  delete mSelectionRectangle;
  mSelectionRectangle = 0;

  if ( rect.isEmpty() )
  {
    mSelectionRectangle = new QgsRectangle( extent() );
  }
  else
  {
    mSelectionRectangle = new QgsRectangle( rect );
  }
  mAttributesToFetch = fetchAttributes;
  mFetchGeom = fetchGeometry;

  rewind();
}



// Return the extent of the layer
QgsRectangle QgsGPXProvider::extent()
{
  return data->getExtent();
}


/**
 * Return the feature type
 */
QGis::WkbType QgsGPXProvider::geometryType() const
{
  if ( mFeatureType == WaypointType )
    return QGis::WKBPoint;

  if ( mFeatureType == RouteType || mFeatureType == TrackType )
    return QGis::WKBLineString;

  return QGis::WKBUnknown;
}


/**
 * Return the feature type
 */
long QgsGPXProvider::featureCount() const
{
  if ( mFeatureType == WaypointType )
    return data->getNumberOfWaypoints();
  if ( mFeatureType == RouteType )
    return data->getNumberOfRoutes();
  if ( mFeatureType == TrackType )
    return data->getNumberOfTracks();
  return 0;
}


/**
 * Return the number of fields
 */
uint QgsGPXProvider::fieldCount() const
{
  return attributeFields.size();
}


const QgsFieldMap& QgsGPXProvider::fields() const
{
  return attributeFields;
}


void QgsGPXProvider::rewind()
{
  if ( mFeatureType == WaypointType )
    mWptIter = data->waypointsBegin();
  else if ( mFeatureType == RouteType )
    mRteIter = data->routesBegin();
  else if ( mFeatureType == TrackType )
    mTrkIter = data->tracksBegin();
}


bool QgsGPXProvider::isValid()
{
  return mValid;
}


bool QgsGPXProvider::addFeatures( QgsFeatureList & flist )
{

  // add all the features
  for ( QgsFeatureList::iterator iter = flist.begin();
        iter != flist.end(); ++iter )
  {
    if ( !addFeature( *iter ) )
      return false;
  }

  // write back to file
  QFile file( mFileName );
  if ( !file.open( QIODevice::WriteOnly ) )
    return false;
  QTextStream ostr( &file );
  data->writeXML( ostr );
  return true;
}


bool QgsGPXProvider::addFeature( QgsFeature& f )
{
  unsigned char* geo = f.geometry()->asWkb();
  QGis::WkbType wkbType = f.geometry()->wkbType();
  bool success = false;
  QgsGPSObject* obj = NULL;
  const QgsAttributeMap& attrs( f.attributeMap() );
  QgsAttributeMap::const_iterator it;

  // is it a waypoint?
  if ( mFeatureType == WaypointType && geo != NULL && wkbType == QGis::WKBPoint )
  {

    // add geometry
    QgsWaypoint wpt;
    std::memcpy( &wpt.lon, geo + 5, sizeof( double ) );
    std::memcpy( &wpt.lat, geo + 13, sizeof( double ) );

    // add waypoint-specific attributes
    for ( it = attrs.begin(); it != attrs.end(); ++it )
    {
      if ( it.key() == EleAttr )
      {
        bool eleIsOK;
        double ele = it->toDouble( &eleIsOK );
        if ( eleIsOK )
          wpt.ele = ele;
      }
      else if ( it.key() == SymAttr )
      {
        wpt.sym = it->toString();
      }
    }

    QgsGPSData::WaypointIterator iter = data->addWaypoint( wpt );
    success = true;
    obj = &( *iter );
  }

  // is it a route?
  if ( mFeatureType == RouteType && geo != NULL && wkbType == QGis::WKBLineString )
  {

    QgsRoute rte;

    // reset bounds
    rte.xMin = std::numeric_limits<double>::max();
    rte.xMax = -std::numeric_limits<double>::max();
    rte.yMin = std::numeric_limits<double>::max();
    rte.yMax = -std::numeric_limits<double>::max();

    // add geometry
    int nPoints;
    std::memcpy( &nPoints, geo + 5, 4 );
    for ( int i = 0; i < nPoints; ++i )
    {
      double lat, lon;
      std::memcpy( &lon, geo + 9 + 16 * i, sizeof( double ) );
      std::memcpy( &lat, geo + 9 + 16 * i + 8, sizeof( double ) );
      QgsRoutepoint rtept;
      rtept.lat = lat;
      rtept.lon = lon;
      rte.points.push_back( rtept );
      rte.xMin = rte.xMin < lon ? rte.xMin : lon;
      rte.xMax = rte.xMax > lon ? rte.xMax : lon;
      rte.yMin = rte.yMin < lat ? rte.yMin : lat;
      rte.yMax = rte.yMax > lat ? rte.yMax : lat;
    }

    // add route-specific attributes
    for ( it = attrs.begin(); it != attrs.end(); ++it )
    {
      if ( it.key() == NumAttr )
      {
        bool numIsOK;
        long num = it->toInt( &numIsOK );
        if ( numIsOK )
          rte.number = num;
      }
    }

    QgsGPSData::RouteIterator iter = data->addRoute( rte );
    success = true;
    obj = &( *iter );
  }

  // is it a track?
  if ( mFeatureType == TrackType && geo != NULL && wkbType == QGis::WKBLineString )
  {

    QgsTrack trk;
    QgsTrackSegment trkseg;

    // reset bounds
    trk.xMin = std::numeric_limits<double>::max();
    trk.xMax = -std::numeric_limits<double>::max();
    trk.yMin = std::numeric_limits<double>::max();
    trk.yMax = -std::numeric_limits<double>::max();

    // add geometry
    int nPoints;
    std::memcpy( &nPoints, geo + 5, 4 );
    for ( int i = 0; i < nPoints; ++i )
    {
      double lat, lon;
      std::memcpy( &lon, geo + 9 + 16 * i, sizeof( double ) );
      std::memcpy( &lat, geo + 9 + 16 * i + 8, sizeof( double ) );
      QgsTrackpoint trkpt;
      trkpt.lat = lat;
      trkpt.lon = lon;
      trkseg.points.push_back( trkpt );
      trk.xMin = trk.xMin < lon ? trk.xMin : lon;
      trk.xMax = trk.xMax > lon ? trk.xMax : lon;
      trk.yMin = trk.yMin < lat ? trk.yMin : lat;
      trk.yMax = trk.yMax > lat ? trk.yMax : lat;
    }

    // add track-specific attributes
    for ( it = attrs.begin(); it != attrs.end(); ++it )
    {
      if ( it.key() == NumAttr )
      {
        bool numIsOK;
        long num = it->toInt( &numIsOK );
        if ( numIsOK )
          trk.number = num;
      }
    }

    trk.segments.push_back( trkseg );
    QgsGPSData::TrackIterator iter = data->addTrack( trk );
    success = true;
    obj = &( *iter );
  }


  // add common attributes
  if ( obj )
  {
    for ( it = attrs.begin(); it != attrs.end(); ++it )
    {
      if ( it.key() == NameAttr )
      {
        obj->name = it->toString();
      }
      else if ( it.key() == CmtAttr )
      {
        obj->cmt = it->toString();
      }
      else if ( it.key() == DscAttr )
      {
        obj->desc = it->toString();
      }
      else if ( it.key() == SrcAttr )
      {
        obj->src = it->toString();
      }
      else if ( it.key() == URLAttr )
      {
        obj->url = it->toString();
      }
      else if ( it.key() == URLNameAttr )
      {
        obj->urlname = it->toString();
      }
    }
  }

  return success;
}


bool QgsGPXProvider::deleteFeatures( const QgsFeatureIds & id )
{
  if ( mFeatureType == WaypointType )
    data->removeWaypoints( id );
  else if ( mFeatureType == RouteType )
    data->removeRoutes( id );
  else if ( mFeatureType == TrackType )
    data->removeTracks( id );

  // write back to file
  QFile file( mFileName );
  if ( !file.open( QIODevice::WriteOnly ) )
    return false;
  QTextStream ostr( &file );
  data->writeXML( ostr );
  return true;
}


bool QgsGPXProvider::changeAttributeValues( const QgsChangedAttributesMap & attr_map )
{
  QgsChangedAttributesMap::const_iterator aIter = attr_map.begin();
  if ( mFeatureType == WaypointType )
  {
    QgsGPSData::WaypointIterator wIter = data->waypointsBegin();
    for ( ; wIter != data->waypointsEnd() && aIter != attr_map.end(); ++wIter )
    {
      if ( wIter->id == aIter.key() )
      {
        changeAttributeValues( *wIter, aIter.value() );
        ++aIter;
      }
    }
  }
  else if ( mFeatureType == RouteType )
  {
    QgsGPSData::RouteIterator rIter = data->routesBegin();
    for ( ; rIter != data->routesEnd() && aIter != attr_map.end(); ++rIter )
    {
      if ( rIter->id == aIter.key() )
      {
        changeAttributeValues( *rIter, aIter.value() );
        ++aIter;
      }
    }
  }
  if ( mFeatureType == TrackType )
  {
    QgsGPSData::TrackIterator tIter = data->tracksBegin();
    for ( ; tIter != data->tracksEnd() && aIter != attr_map.end(); ++tIter )
    {
      if ( tIter->id == aIter.key() )
      {
        changeAttributeValues( *tIter, aIter.value() );
        ++aIter;
      }
    }
  }

  // write back to file
  QFile file( mFileName );
  if ( !file.open( QIODevice::WriteOnly ) )
    return false;
  QTextStream ostr( &file );
  data->writeXML( ostr );
  return true;
}


void QgsGPXProvider::changeAttributeValues( QgsGPSObject& obj, const QgsAttributeMap& attrs )
{
  QgsAttributeMap::const_iterator aIter;

  // TODO:
  if ( attrs.contains( NameAttr ) )
    obj.name = attrs[NameAttr].toString();
  if ( attrs.contains( CmtAttr ) )
    obj.cmt = attrs[CmtAttr].toString();
  if ( attrs.contains( DscAttr ) )
    obj.desc = attrs[DscAttr].toString();
  if ( attrs.contains( SrcAttr ) )
    obj.src = attrs[SrcAttr].toString();
  if ( attrs.contains( URLAttr ) )
    obj.url = attrs[URLAttr].toString();
  if ( attrs.contains( URLNameAttr ) )
    obj.urlname = attrs[URLNameAttr].toString();

  // waypoint-specific attributes
  QgsWaypoint* wpt = dynamic_cast<QgsWaypoint*>( &obj );
  if ( wpt != NULL )
  {
    if ( attrs.contains( SymAttr ) )
      wpt->sym = attrs[SymAttr].toString();
    if ( attrs.contains( EleAttr ) )
    {
      bool eleIsOK;
      double ele = attrs[EleAttr].toDouble( &eleIsOK );
      if ( eleIsOK )
        wpt->ele = ele;
    }
  }

  // route- and track-specific attributes
  QgsGPSExtended* ext = dynamic_cast<QgsGPSExtended*>( &obj );
  if ( ext != NULL )
  {
    if ( attrs.contains( NumAttr ) )
    {
      bool eleIsOK;
      double ele = attrs[NumAttr].toDouble( &eleIsOK );
      if ( eleIsOK )
        wpt->ele = ele;
    }
  }
}


QVariant QgsGPXProvider::defaultValue( int fieldId )
{
  if ( fieldId == SrcAttr )
    return tr( "Digitized in QGIS" );
  return QVariant();
}


/**
 * Check to see if the point is within the selection rectangle
 */
bool QgsGPXProvider::boundsCheck( double x, double y )
{
  bool inBounds = ((( x <= mSelectionRectangle->xMaximum() ) &&
                    ( x >= mSelectionRectangle->xMinimum() ) ) &&
                   (( y <= mSelectionRectangle->yMaximum() ) &&
                    ( y >= mSelectionRectangle->yMinimum() ) ) );
  QString hit = inBounds ? "true" : "false";
  return inBounds;
}


QString QgsGPXProvider::name() const
{
  return GPX_KEY;
} // QgsGPXProvider::name()



QString QgsGPXProvider::description() const
{
  return GPX_DESCRIPTION;
} // QgsGPXProvider::description()

QgsCoordinateReferenceSystem QgsGPXProvider::crs()
{
  return QgsCoordinateReferenceSystem( GEOSRID, QgsCoordinateReferenceSystem::PostgisCrsId ); // use WGS84
}






/**
 * Class factory to return a pointer to a newly created
 * QgsGPXProvider object
 */
QGISEXTERN QgsGPXProvider * classFactory( const QString *uri )
{
  return new QgsGPXProvider( *uri );
}


/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return GPX_KEY;
}


/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return GPX_DESCRIPTION;
}


/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}


