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
#include <QIcon>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsdataprovider.h"
#include "qgsfeature.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsrectangle.h"

#include "qgsgpxfeatureiterator.h"
#include "qgsgpxprovider.h"
#include "gpsdata.h"

const QStringList QgsGPXProvider::sAttributeNames = { "name", "elevation", "symbol", "number",
                                                      "comment", "description", "source",
                                                      "url", "url name"
                                                    };
const QList< QVariant::Type > QgsGPXProvider::sAttributeTypes = { QVariant::String, QVariant::Double, QVariant::String, QVariant::Int,
                                                                  QVariant::String, QVariant::String, QVariant::String,
                                                                  QVariant::String, QVariant::String
                                                                };
const QList< QgsGPXProvider::DataType > QgsGPXProvider::sAttributedUsedForLayerType =
{
  QgsGPXProvider::AllType, QgsGPXProvider::WaypointType, QgsGPXProvider::TrkRteType, QgsGPXProvider::TrkRteType,
  QgsGPXProvider::AllType, QgsGPXProvider::AllType, QgsGPXProvider::AllType, QgsGPXProvider::AllType,
  QgsGPXProvider::AllType, QgsGPXProvider::AllType
};

const QString GPX_KEY = QStringLiteral( "gpx" );

const QString GPX_DESCRIPTION = QObject::tr( "GPS eXchange format provider" );


QgsGPXProvider::QgsGPXProvider( const QString &uri, const ProviderOptions &options, QgsDataProvider::ReadFlags flags )
  : QgsVectorDataProvider( uri, options, flags )
{
  // we always use UTF-8
  setEncoding( QStringLiteral( "utf8" ) );

  const QVariantMap uriParts = decodeUri( uri );
  const QString typeStr = uriParts.value( QStringLiteral( "layerName" ) ).toString();
  if ( typeStr.isEmpty() )
  {
    QgsLogger::warning( tr( "Bad URI - you need to specify the feature type." ) );
    return;
  }
  if ( typeStr.compare( QLatin1String( "waypoint" ), Qt::CaseInsensitive ) == 0 )
    mFeatureType = WaypointType;
  else if ( typeStr.compare( QLatin1String( "route" ), Qt::CaseInsensitive ) == 0 )
    mFeatureType = RouteType;
  else
    mFeatureType = TrackType;

  mFileName = uriParts.value( QStringLiteral( "path" ) ).toString();

  // set up the attributes and the geometry type depending on the feature type
  for ( int i = 0; i < sAttributeNames.size(); ++i )
  {
    if ( sAttributedUsedForLayerType[i] & mFeatureType )
    {
      const QString attrTypeName = ( sAttributeTypes[i] == QVariant::Int ? "int" : ( sAttributeTypes[i] == QVariant::Double ? "double" : "text" ) );
      mAttributeFields.append( QgsField( sAttributeNames[i], sAttributeTypes[i], attrTypeName ) );
      mIndexToAttr.append( i );
    }
  }

  // parse the file
  mData = QgsGpsData::getData( mFileName );
  if ( !mData )
    return;

  mValid = true;
}

QgsGPXProvider::~QgsGPXProvider()
{
  QgsGpsData::releaseData( mFileName );
}

QgsAbstractFeatureSource *QgsGPXProvider::featureSource() const
{
  return new QgsGPXFeatureSource( this );
}

QString QgsGPXProvider::storageType() const
{
  return tr( "GPS eXchange file" );
}

QgsVectorDataProvider::Capabilities QgsGPXProvider::capabilities() const
{
  return QgsVectorDataProvider::AddFeatures |
         QgsVectorDataProvider::DeleteFeatures |
         QgsVectorDataProvider::ChangeAttributeValues;
}

QgsRectangle QgsGPXProvider::extent() const
{
  if ( mData )
    return mData->getExtent();
  return QgsRectangle();
}

QgsWkbTypes::Type QgsGPXProvider::wkbType() const
{
  if ( mFeatureType == WaypointType )
    return QgsWkbTypes::Point;

  if ( mFeatureType == RouteType || mFeatureType == TrackType )
    return QgsWkbTypes::LineString;

  return QgsWkbTypes::Unknown;
}

long long QgsGPXProvider::featureCount() const
{
  if ( !mData )
    return static_cast< long long >( Qgis::FeatureCountState::UnknownCount );

  if ( mFeatureType == WaypointType )
    return mData->getNumberOfWaypoints();
  if ( mFeatureType == RouteType )
    return mData->getNumberOfRoutes();
  if ( mFeatureType == TrackType )
    return mData->getNumberOfTracks();
  return 0;
}

QgsFields QgsGPXProvider::fields() const
{
  return mAttributeFields;
}

bool QgsGPXProvider::isValid() const
{
  return mValid;
}

QgsFeatureIterator QgsGPXProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  return QgsFeatureIterator( new QgsGPXFeatureIterator( new QgsGPXFeatureSource( this ), true, request ) );
}

bool QgsGPXProvider::addFeatures( QgsFeatureList &flist, Flags flags )
{
  if ( !mData )
    return false;

  // add all the features
  for ( QgsFeatureList::iterator iter = flist.begin();
        iter != flist.end(); ++iter )
  {
    if ( !addFeature( *iter, flags ) )
      return false;
  }

  // write back to file
  QFile file( mFileName );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    return false;
  QTextStream ostr( &file );
  mData->writeXml( ostr );
  return true;
}

bool QgsGPXProvider::addFeature( QgsFeature &f, Flags )
{
  if ( !mData )
    return false;

  const QByteArray wkb( f.geometry().asWkb() );
  const char *geo = wkb.constData();
  const QgsWkbTypes::Type wkbType = f.geometry().wkbType();
  bool success = false;
  QgsGpsObject *obj = nullptr;
  const QgsAttributes attrs = f.attributes();

  // is it a waypoint?
  if ( mFeatureType == WaypointType && geo && wkbType == QgsWkbTypes::Point )
  {

    // add geometry
    QgsWaypoint wpt;
    std::memcpy( &wpt.lon, geo + 5, sizeof( double ) );
    std::memcpy( &wpt.lat, geo + 13, sizeof( double ) );

    // add waypoint-specific attributes
    for ( int i = 0; i < attrs.count(); ++i )
    {
      if ( mIndexToAttr.at( i ) == EleAttr )
      {
        bool eleIsOK;
        const double ele = attrs.at( i ).toDouble( &eleIsOK );
        if ( eleIsOK )
          wpt.ele = ele;
      }
      else if ( mIndexToAttr.at( i ) == SymAttr )
      {
        wpt.sym = attrs.at( i ).toString();
      }
    }

    const QgsGpsData::WaypointIterator iter = mData->addWaypoint( wpt );
    success = true;
    obj = &( *iter );
  }

  // is it a route?
  if ( mFeatureType == RouteType && geo && wkbType == QgsWkbTypes::LineString )
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
      if ( mIndexToAttr.at( i ) == NumAttr )
      {
        bool numIsOK;
        const long num = attrs.at( i ).toInt( &numIsOK );
        if ( numIsOK )
          rte.number = num;
      }
    }

    const QgsGpsData::RouteIterator iter = mData->addRoute( rte );
    success = true;
    obj = &( *iter );
  }

  // is it a track?
  if ( mFeatureType == TrackType && geo && wkbType == QgsWkbTypes::LineString )
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
      if ( mIndexToAttr.at( i ) == NumAttr )
      {
        bool numIsOK;
        const long num = attrs.at( i ).toInt( &numIsOK );
        if ( numIsOK )
          trk.number = num;
      }
    }

    trk.segments.push_back( trkseg );
    const QgsGpsData::TrackIterator iter = mData->addTrack( trk );
    success = true;
    obj = &( *iter );
  }


  // add common attributes
  if ( obj )
  {
    for ( int i = 0; i < attrs.count(); ++i )
    {
      switch ( mIndexToAttr.at( i ) )
      {
        case NameAttr:
          obj->name    = attrs.at( i ).toString();
          break;
        case CmtAttr:
          obj->cmt     = attrs.at( i ).toString();
          break;
        case DscAttr:
          obj->desc    = attrs.at( i ).toString();
          break;
        case SrcAttr:
          obj->src     = attrs.at( i ).toString();
          break;
        case URLAttr:
          obj->url     = attrs.at( i ).toString();
          break;
        case URLNameAttr:
          obj->urlname = attrs.at( i ).toString();
          break;
      }
    }
  }

  return success;
}

bool QgsGPXProvider::deleteFeatures( const QgsFeatureIds &id )
{
  if ( !mData )
    return false;

  if ( mFeatureType == WaypointType )
    mData->removeWaypoints( id );
  else if ( mFeatureType == RouteType )
    mData->removeRoutes( id );
  else if ( mFeatureType == TrackType )
    mData->removeTracks( id );

  // write back to file
  QFile file( mFileName );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    return false;
  QTextStream ostr( &file );
  mData->writeXml( ostr );
  return true;
}

bool QgsGPXProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  if ( !mData )
    return false;

  QgsChangedAttributesMap::const_iterator aIter = attr_map.begin();
  if ( mFeatureType == WaypointType )
  {
    QgsGpsData::WaypointIterator wIter = mData->waypointsBegin();
    for ( ; wIter != mData->waypointsEnd() && aIter != attr_map.end(); ++wIter )
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
    QgsGpsData::RouteIterator rIter = mData->routesBegin();
    for ( ; rIter != mData->routesEnd() && aIter != attr_map.end(); ++rIter )
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
    QgsGpsData::TrackIterator tIter = mData->tracksBegin();
    for ( ; tIter != mData->tracksEnd() && aIter != attr_map.end(); ++tIter )
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
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    return false;
  QTextStream ostr( &file );
  mData->writeXml( ostr );
  return true;
}

void QgsGPXProvider::changeAttributeValues( QgsGpsObject &obj, const QgsAttributeMap &attrs )
{

  QgsWaypoint *wpt = dynamic_cast<QgsWaypoint *>( &obj );
  QgsGpsExtended *ext = dynamic_cast<QgsGpsExtended *>( &obj );

  QgsAttributeMap::const_iterator aIter = attrs.begin();
  for ( ; aIter != attrs.end(); ++aIter )
  {
    const int i = aIter.key();
    const QVariant v = aIter.value();

    // common attributes
    switch ( mIndexToAttr.at( i ) )
    {
      case NameAttr:
        obj.name    = v.toString();
        break;
      case CmtAttr:
        obj.cmt     = v.toString();
        break;
      case DscAttr:
        obj.desc    = v.toString();
        break;
      case SrcAttr:
        obj.src     = v.toString();
        break;
      case URLAttr:
        obj.url     = v.toString();
        break;
      case URLNameAttr:
        obj.urlname = v.toString();
        break;
    }

    // waypoint-specific attributes
    if ( wpt )
    {
      if ( mIndexToAttr.at( i ) == SymAttr )
        wpt->sym = v.toString();
      else if ( mIndexToAttr.at( i ) == EleAttr )
      {
        bool eleIsOK;
        const double ele = v.toDouble( &eleIsOK );
        if ( eleIsOK )
          wpt->ele = ele;
      }
    }

    // route- and track-specific attributes
    if ( ext )
    {
      if ( mIndexToAttr.at( i ) == NumAttr )
      {
        bool numIsOK;
        const int num = v.toInt( &numIsOK );
        if ( numIsOK )
          ext->number = num;
      }
    }

  }

}

QVariant QgsGPXProvider::defaultValue( int fieldId ) const
{
  if ( fieldId == SrcAttr )
    return tr( "Digitized in QGIS" );
  return QVariant();
}

QString QgsGPXProvider::name() const
{
  return GPX_KEY;
}

QString QgsGPXProvider::description() const
{
  return GPX_DESCRIPTION;
}

QgsCoordinateReferenceSystem QgsGPXProvider::crs() const
{
  return QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) );
}

QString QgsGPXProvider::encodeUri( const QVariantMap &parts )
{
  if ( parts.value( QStringLiteral( "layerName" ) ).toString().isEmpty() )
    return parts.value( QStringLiteral( "path" ) ).toString();
  else
    return QStringLiteral( "%1?type=%2" ).arg( parts.value( QStringLiteral( "path" ) ).toString(),
           parts.value( QStringLiteral( "layerName" ) ).toString() );
}

QVariantMap QgsGPXProvider::decodeUri( const QString &uri )
{
  QVariantMap res;
  const int fileNameEnd = uri.indexOf( '?' );
  if ( fileNameEnd != -1 && uri.mid( fileNameEnd + 1, 5 ) == QLatin1String( "type=" ) )
  {
    res.insert( QStringLiteral( "layerName" ), uri.mid( fileNameEnd + 6 ) );
    res.insert( QStringLiteral( "path" ), uri.left( fileNameEnd ) );
  }
  else if ( !uri.isEmpty() )
  {
    res.insert( QStringLiteral( "path" ), uri );
  }
  return res;
}

QgsGpxProviderMetadata::QgsGpxProviderMetadata():
  QgsProviderMetadata( GPX_KEY, GPX_DESCRIPTION )
{
}

QIcon QgsGpxProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "mIconGpx.svg" ) );
}

QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsGpxProviderMetadata();
}

QgsDataProvider *QgsGpxProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  return new QgsGPXProvider( uri, options, flags );
}

QgsProviderMetadata::ProviderCapabilities QgsGpxProviderMetadata::providerCapabilities() const
{
  return QgsProviderMetadata::ProviderCapability::FileBasedUris;
}

QString QgsGpxProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  return QgsGPXProvider::encodeUri( parts );
}

QVariantMap QgsGpxProviderMetadata::decodeUri( const QString &uri ) const
{
  return QgsGPXProvider::decodeUri( uri );
}

QList<QgsMapLayerType> QgsGpxProviderMetadata::supportedLayerTypes() const
{
  return { QgsMapLayerType::VectorLayer };
}
