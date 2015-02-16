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
#include "qgscoordinatereferencesystem.h"
#include "qgsdataprovider.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsrectangle.h"

#include "qgsgpxfeatureiterator.h"
#include "qgsgpxprovider.h"
#include "gpsdata.h"

const char* QgsGPXProvider::attr[] = { "name", "elevation", "symbol", "number",
                                       "comment", "description", "source",
                                       "url", "url name"
                                     };
QVariant::Type QgsGPXProvider::attrType[] = { QVariant::String, QVariant::Double, QVariant::String, QVariant::Int,
    QVariant::String, QVariant::String, QVariant::String,
    QVariant::String, QVariant::String
                                            };
QgsGPXProvider::DataType QgsGPXProvider::attrUsed[] =
{
  QgsGPXProvider::AllType, QgsGPXProvider::WaypointType, QgsGPXProvider::TrkRteType, QgsGPXProvider::TrkRteType,
  QgsGPXProvider::AllType, QgsGPXProvider::AllType, QgsGPXProvider::AllType, QgsGPXProvider::AllType,
  QgsGPXProvider::AllType, QgsGPXProvider::AllType
};

const int QgsGPXProvider::attrCount = sizeof( QgsGPXProvider::attr ) / sizeof( const char* );

const QString GPX_KEY = "gpx";

const QString GPX_DESCRIPTION = QObject::tr( "GPS eXchange format provider" );


QgsGPXProvider::QgsGPXProvider( QString uri )
    : QgsVectorDataProvider( uri )
    , data( 0 )
    , mFeatureType( WaypointType )
    , mValid( false ) // assume that it won't work
{
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
  for ( int i = 0; i < attrCount; ++i )
  {
    if ( attrUsed[i] & mFeatureType )
    {
      QString attrTypeName = ( attrType[i] == QVariant::Int ? "int" : ( attrType[i] == QVariant::Double ? "double" : "text" ) );
      attributeFields.append( QgsField( attr[i], attrType[i], attrTypeName ) );
      indexToAttr.append( i );
    }
  }

  mFileName = uri.left( fileNameEnd );

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
}

QgsAbstractFeatureSource* QgsGPXProvider::featureSource() const
{
  return new QgsGPXFeatureSource( this );
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


const QgsFields& QgsGPXProvider::fields() const
{
  return attributeFields;
}



bool QgsGPXProvider::isValid()
{
  return mValid;
}


QgsFeatureIterator QgsGPXProvider::getFeatures( const QgsFeatureRequest& request )
{
  return QgsFeatureIterator( new QgsGPXFeatureIterator( new QgsGPXFeatureSource( this ), true, request ) );
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
  const unsigned char* geo = f.geometry()->asWkb();
  QGis::WkbType wkbType = f.geometry()->wkbType();
  bool success = false;
  QgsGPSObject* obj = NULL;
  const QgsAttributes& attrs = f.attributes();
  QgsAttributeMap::const_iterator it;

  // is it a waypoint?
  if ( mFeatureType == WaypointType && geo != NULL && wkbType == QGis::WKBPoint )
  {

    // add geometry
    QgsWaypoint wpt;
    std::memcpy( &wpt.lon, geo + 5, sizeof( double ) );
    std::memcpy( &wpt.lat, geo + 13, sizeof( double ) );

    // add waypoint-specific attributes
    for ( int i = 0; i < attrs.count(); ++i )
    {
      if ( indexToAttr[i] == EleAttr )
      {
        bool eleIsOK;
        double ele = attrs[i].toDouble( &eleIsOK );
        if ( eleIsOK )
          wpt.ele = ele;
      }
      else if ( indexToAttr[i] == SymAttr )
      {
        wpt.sym = attrs[i].toString();
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
    for ( int i = 0; i < attrs.count(); ++i )
    {
      if ( indexToAttr[i] == NumAttr )
      {
        bool numIsOK;
        long num = attrs[i].toInt( &numIsOK );
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
    for ( int i = 0; i < attrs.count(); ++i )
    {
      if ( indexToAttr[i] == NumAttr )
      {
        bool numIsOK;
        long num = attrs[i].toInt( &numIsOK );
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
    for ( int i = 0; i < attrs.count(); ++i )
    {
      switch ( indexToAttr[i] )
      {
        case NameAttr:    obj->name    = attrs[i].toString(); break;
        case CmtAttr:     obj->cmt     = attrs[i].toString(); break;
        case DscAttr:     obj->desc    = attrs[i].toString(); break;
        case SrcAttr:     obj->src     = attrs[i].toString(); break;
        case URLAttr:     obj->url     = attrs[i].toString(); break;
        case URLNameAttr: obj->urlname = attrs[i].toString(); break;
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


bool QgsGPXProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
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

  QgsWaypoint* wpt = dynamic_cast<QgsWaypoint*>( &obj );
  QgsGPSExtended* ext = dynamic_cast<QgsGPSExtended*>( &obj );

  QgsAttributeMap::const_iterator aIter = attrs.begin();
  for ( ; aIter != attrs.end(); ++aIter )
  {
    int i = aIter.key();
    QVariant v = aIter.value();

    // common attributes
    switch ( indexToAttr[i] )
    {
      case NameAttr:    obj.name    = v.toString(); break;
      case CmtAttr:     obj.cmt     = v.toString(); break;
      case DscAttr:     obj.desc    = v.toString(); break;
      case SrcAttr:     obj.src     = v.toString(); break;
      case URLAttr:     obj.url     = v.toString(); break;
      case URLNameAttr: obj.urlname = v.toString(); break;
    }

    // waypoint-specific attributes
    if ( wpt != NULL )
    {
      if ( indexToAttr[i] == SymAttr )
        wpt->sym = v.toString();
      else if ( indexToAttr[i] == EleAttr )
      {
        bool eleIsOK;
        double ele = v.toDouble( &eleIsOK );
        if ( eleIsOK )
          wpt->ele = ele;
      }
    }

    // route- and track-specific attributes
    if ( ext != NULL )
    {
      if ( indexToAttr[i] == NumAttr )
      {
        bool numIsOK;
        int num = v.toInt( &numIsOK );
        if ( numIsOK )
          ext->number = num;
      }
    }

  }

}


QVariant QgsGPXProvider::defaultValue( int fieldId )
{
  if ( fieldId == SrcAttr )
    return tr( "Digitized in QGIS" );
  return QVariant();
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


