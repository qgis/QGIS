/***************************************************************************
    qgswmscapabilities.cpp
    ---------------------
    begin                : January 2014
    copyright            : (C) 2014 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswmscapabilities.h"
#include "qgswmsprovider.h"

#include <QFile>
#include <QDir>
#include <QNetworkCacheMetaData>
#include <QRegularExpression>
#include <QUrlQuery>

#include "qgis.h"
#include "qgssettings.h"
#include "qgscoordinatetransform.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssetrequestinitiator_p.h"
#include "qgsexception.h"
#include "qgstemporalutils.h"
#include "qgsunittypes.h"

// %%% copied from qgswmsprovider.cpp
static QString DEFAULT_LATLON_CRS = QStringLiteral( "CRS:84" );



bool QgsWmsSettings::parseUri( const QString &uriString )
{
  QgsDebugMsgLevel( "uriString = " + uriString, 2 );
  QgsDataSourceUri uri;
  uri.setEncodedUri( uriString );

  // Setup authentication
  mAuth.mUserName = uri.username();
  mAuth.mPassword = uri.password();

  if ( !uri.authConfigId().isEmpty() )
  {
    mAuth.mAuthCfg = uri.authConfigId();
  }

  mAuth.mHttpHeaders = uri.httpHeaders();
  mXyz = false;  // assume WMS / WMTS

  if ( uri.hasParam( QStringLiteral( "interpretation" ) ) )
  {
    mInterpretation = uri.param( QStringLiteral( "interpretation" ) );
  }

  if ( uri.param( QStringLiteral( "type" ) ) == QLatin1String( "xyz" ) ||
       uri.param( QStringLiteral( "type" ) ) == QLatin1String( "mbtiles" ) )
  {
    // for XYZ tiles most of the things do not apply
    mTiled = true;
    mXyz = true;
    mTileDimensionValues.clear();
    mTileMatrixSetId = QStringLiteral( "tms0" );
    mMaxWidth = 0;
    mMaxHeight = 0;
    mHttpUri = uri.param( QStringLiteral( "url" ) );

    // automatically replace outdated references to http OpenStreetMap tiles with correct https URI
    const thread_local QRegularExpression sRegExOSMTiles( QStringLiteral( "^https?://(?:[a-z]\\.)?tile\\.openstreetmap\\.org" ) );
    const QRegularExpressionMatch match = sRegExOSMTiles.match( mHttpUri );
    if ( match.hasMatch() )
    {
      mHttpUri = QStringLiteral( "https://tile.openstreetmap.org" ) + mHttpUri.mid( match.captured( 0 ).length() );
    }

    mBaseUrl = mHttpUri;

    mIgnoreGetMapUrl = false;
    mIgnoreGetFeatureInfoUrl = false;
    mSmoothPixmapTransform = mInterpretation.isEmpty();
    mDpiMode = DpiNone; // does not matter what we set here
    mActiveSubLayers = QStringList( QStringLiteral( "xyz" ) );  // just a placeholder to have one sub-layer
    mActiveSubStyles = QStringList( QStringLiteral( "xyz" ) );  // just a placeholder to have one sub-style
    mActiveSubLayerVisibility.clear();
    mFeatureCount = 0;
    mImageMimeType.clear();
    mCrsId = QStringLiteral( "EPSG:3857" );
    mEnableContextualLegend = false;

    mIsMBTiles = uri.param( QStringLiteral( "type" ) ) == QLatin1String( "mbtiles" );

    return true;
  }

  if ( uri.param( QStringLiteral( "type" ) ) == QLatin1String( "wmst" ) )
  {
    mIsTemporal = true;
    mTemporalExtent = uri.param( QStringLiteral( "timeDimensionExtent" ) );
    mTimeDimensionExtent = parseTemporalExtent( mTemporalExtent );

    if ( !mTimeDimensionExtent.datesResolutionList.isEmpty() &&
         !mTimeDimensionExtent.datesResolutionList.constFirst().dates.dateTimes.empty() )
    {
      QDateTime begin = mTimeDimensionExtent.datesResolutionList.constFirst().dates.dateTimes.first();
      QDateTime end = mTimeDimensionExtent.datesResolutionList.constLast().dates.dateTimes.last();

      mFixedRange = QgsDateTimeRange( begin, end );
    }
    else
      mFixedRange = QgsDateTimeRange();

    mAllRanges.clear();
    mAllRanges.reserve( mTimeDimensionExtent.datesResolutionList.size() );
    for ( const QgsWmstExtentPair &extent : std::as_const( mTimeDimensionExtent.datesResolutionList ) )
    {
      if ( extent.dates.dateTimes.empty() )
        continue;

      const QDateTime begin = extent.dates.dateTimes.first();
      const QDateTime end = extent.dates.dateTimes.last();

      if ( !extent.resolution.isNull() )
      {
        bool maxValuesExceeded = false;
        const QList< QDateTime > dates = QgsTemporalUtils::calculateDateTimesUsingDuration( begin, end, extent.resolution, maxValuesExceeded, 1000 );
        // if we have a manageable number of distinct dates, then we'll use those. If not we just use the overall range.
        // (some servers eg may have data for every minute for decades!)
        if ( !maxValuesExceeded )
        {
          for ( const QDateTime &dt : dates )
            mAllRanges.append( QgsDateTimeRange( dt, dt ) );
        }
        else
        {
          mAllRanges.append( QgsDateTimeRange( begin, end ) );
        }

        mDefaultInterval = extent.resolution.toInterval();
      }
      else
      {
        mAllRanges.append( QgsDateTimeRange( begin, end ) );
        mDefaultInterval = QgsInterval( 1, Qgis::TemporalUnit::IrregularStep );
      }
    }

    if ( !uri.param( QStringLiteral( "referenceTimeDimensionExtent" ) ).isEmpty() )
    {
      QString referenceExtent = uri.param( QStringLiteral( "referenceTimeDimensionExtent" ) );

      mReferenceTimeDimensionExtent = parseTemporalExtent( referenceExtent );

      if ( mReferenceTimeDimensionExtent.datesResolutionList.constFirst().dates.dateTimes.size() > 0 )
      {
        QDateTime begin = mReferenceTimeDimensionExtent.datesResolutionList.constFirst().dates.dateTimes.first();
        QDateTime end = mReferenceTimeDimensionExtent.datesResolutionList.constLast().dates.dateTimes.last();

        mFixedReferenceRange =  QgsDateTimeRange( begin, end );
      }
      else
        mFixedReferenceRange = QgsDateTimeRange();

      mIsBiTemporal = true;
    }
  }

  mTiled = false;
  mTileDimensionValues.clear();

  mHttpUri = uri.param( QStringLiteral( "url" ) );
  mBaseUrl = QgsWmsProvider::prepareUri( mHttpUri ); // must set here, setImageCrs is using that
  QgsDebugMsgLevel( "mBaseUrl = " + mBaseUrl, 2 );

  mIgnoreGetMapUrl = uri.hasParam( QStringLiteral( "IgnoreGetMapUrl" ) );
  mIgnoreGetFeatureInfoUrl = uri.hasParam( QStringLiteral( "IgnoreGetFeatureInfoUrl" ) );
  mIgnoreReportedLayerExtents = uri.hasParam( QStringLiteral( "IgnoreReportedLayerExtents" ) );
  mParserSettings.ignoreAxisOrientation = uri.hasParam( QStringLiteral( "IgnoreAxisOrientation" ) ); // must be before parsing!
  mParserSettings.invertAxisOrientation = uri.hasParam( QStringLiteral( "InvertAxisOrientation" ) ); // must be before parsing!
  mSmoothPixmapTransform = uri.hasParam( QStringLiteral( "SmoothPixmapTransform" ) );

  mDpiMode = uri.hasParam( QStringLiteral( "dpiMode" ) ) ? static_cast< QgsWmsDpiMode >( uri.param( QStringLiteral( "dpiMode" ) ).toInt() ) : DpiAll;

  mActiveSubLayers = uri.params( QStringLiteral( "layers" ) );
  mActiveSubStyles = uri.params( QStringLiteral( "styles" ) );
  QgsDebugMsgLevel( "Entering: layers:" + mActiveSubLayers.join( ", " ) + ", styles:" + mActiveSubStyles.join( ", " ), 2 );

  //opacities
  if ( uri.hasParam( QStringLiteral( "opacities" ) ) )
  {
    mOpacities.clear();
    const QStringList opacities = uri.params( QStringLiteral( "opacities" ) );
    for ( const QString &opacity : opacities )
    {
      bool ok = false;
      ( void )opacity.toInt( &ok );
      if ( ok )
      {
        mOpacities.append( opacity );
      }
      else
      {
        mOpacities.append( QStringLiteral( "255" ) );
      }
    }
  }

  if ( uri.hasParam( QStringLiteral( "filter" ) ) )
  {
    mFilter = uri.param( QStringLiteral( "filter" ) );
  }

  mImageMimeType = uri.param( QStringLiteral( "format" ) );
  QgsDebugMsgLevel( "Setting image encoding to " + mImageMimeType + '.', 2 );

  mMaxWidth = 0;
  mMaxHeight = 0;
  if ( uri.hasParam( QStringLiteral( "maxWidth" ) ) && uri.hasParam( QStringLiteral( "maxHeight" ) ) )
  {
    mMaxWidth = uri.param( QStringLiteral( "maxWidth" ) ).toInt();
    mMaxHeight = uri.param( QStringLiteral( "maxHeight" ) ).toInt();
  }

  mStepWidth = 2000;
  mStepHeight = 2000;
  if ( uri.hasParam( QStringLiteral( "stepWidth" ) ) && uri.hasParam( QStringLiteral( "stepHeight" ) ) )
  {
    mStepWidth = uri.param( QStringLiteral( "stepWidth" ) ).toInt();
    mStepHeight = uri.param( QStringLiteral( "stepHeight" ) ).toInt();
  }

  if ( uri.hasParam( QStringLiteral( "tileMatrixSet" ) ) )
  {
    mTiled = true;
    // tileMatrixSet may be empty if URI was converted from < 1.9 project file URI
    // in that case it means that the source is WMS-C
    mTileMatrixSetId = uri.param( QStringLiteral( "tileMatrixSet" ) );
    mTilePixelRatio = uri.hasParam( QStringLiteral( "tilePixelRatio" ) ) ? static_cast< Qgis::TilePixelRatio >( uri.param( QStringLiteral( "tilePixelRatio" ) ).toInt() ) : Qgis::TilePixelRatio::Undefined;
  }

  if ( uri.hasParam( QStringLiteral( "tileDimensions" ) ) )
  {
    mTiled = true;
    const auto tileDimensions = uri.param( QStringLiteral( "tileDimensions" ) ).split( ';' );
    for ( const QString &param : tileDimensions )
    {
      QStringList kv = param.split( '=' );
      if ( kv.size() == 1 )
      {
        mTileDimensionValues.insert( kv[0], QString() );
      }
      else if ( kv.size() == 2 )
      {
        mTileDimensionValues.insert( kv[0], kv[1] );
      }
      else
      {
        QgsDebugMsgLevel( QStringLiteral( "skipped dimension %1" ).arg( param ), 2 );
      }
    }
  }

  mCrsId = uri.param( QStringLiteral( "crs" ) );

  mEnableContextualLegend = uri.param( QStringLiteral( "contextualWMSLegend" ) ).toInt();
  QgsDebugMsgLevel( QStringLiteral( "Contextual legend: %1" ).arg( mEnableContextualLegend ), 2 );

  mFeatureCount = uri.param( QStringLiteral( "featureCount" ) ).toInt(); // default to 0

  return true;
}

QgsWmstDimensionExtent QgsWmsSettings::parseTemporalExtent( const QString &extent )
{
  QgsWmstDimensionExtent dimensionExtent;
  if ( extent.isEmpty() )
    return dimensionExtent;

  const QStringList parts = extent.split( ',' );

  for ( const QString &part : parts )
  {
    const QString item = part.trimmed();

    // If item contain '/' content separator, it is an interval
    if ( item.contains( '/' ) )
    {
      const QStringList itemParts = item.split( '/' );

      QgsTimeDuration itemResolution;
      QgsWmstDates itemDatesList;

      for ( const QString &itemPart : itemParts )
      {
        QString itemContent = itemPart.trimmed();

        if ( itemContent.startsWith( 'P' ) )
        {
          itemResolution = parseWmstResolution( itemContent );
        }
        else
        {
          itemDatesList.dateTimes.append( parseWmstDateTimes( itemContent ) );
        }
      }

      dimensionExtent.datesResolutionList.append( QgsWmstExtentPair( itemDatesList, itemResolution ) );
    }
    else
    {
      QgsTimeDuration resolution;
      QgsWmstDates datesList;
      if ( item.startsWith( 'P' ) )
      {
        resolution = parseWmstResolution( item );
      }
      else
      {
        datesList.dateTimes.append( parseWmstDateTimes( item ) );
      }

      dimensionExtent.datesResolutionList.append( QgsWmstExtentPair( datesList, resolution ) );
    }
  }

  return dimensionExtent;
}

void QgsWmsSettings::setTimeDimensionExtent( const QgsWmstDimensionExtent &timeDimensionExtent )
{
  mTimeDimensionExtent = timeDimensionExtent;
}

QgsWmstDimensionExtent QgsWmsSettings::timeDimensionExtent() const
{
  return mTimeDimensionExtent;
}

QDateTime QgsWmsSettings::findLeastClosestDateTime( const QDateTime &dateTime, bool dateOnly ) const
{
  QDateTime closest = dateTime;

  long long seconds;

  if ( dateOnly )
    seconds = QDateTime( closest.date(), QTime( 0, 0, 0 ) ).toSecsSinceEpoch();
  else
    seconds = closest.toSecsSinceEpoch();

  for ( const QgsWmstExtentPair &pair : mTimeDimensionExtent.datesResolutionList )
  {
    if ( pair.dates.dateTimes.empty() )
    {
      continue;
    }
    else if ( pair.dates.dateTimes.size() == 1 )
    {
      long long startSeconds = pair.dates.dateTimes.at( 0 ).toSecsSinceEpoch();

      // if out of bounds
      if ( seconds < startSeconds )
        continue;

      closest = pair.dates.dateTimes.at( 0 );
    }
    else
    {
      long long startSeconds = pair.dates.dateTimes.at( 0 ).toSecsSinceEpoch();
      long long endSeconds = pair.dates.dateTimes.at( 1 ).toSecsSinceEpoch();

      // if out of bounds
      if ( seconds < startSeconds || seconds > endSeconds )
        continue;
      if ( seconds == endSeconds )
        break;

      long long resolutionSeconds = pair.resolution.toSeconds();

      if ( resolutionSeconds <= 0 )
        continue;
      long long step = std::floor( ( seconds - startSeconds ) / resolutionSeconds );
      long long resultSeconds = startSeconds + ( step * resolutionSeconds );

      closest.setSecsSinceEpoch( resultSeconds );
    }
  }

  return closest;
}

QgsTimeDuration QgsWmsSettings::parseWmstResolution( const QString &itemText )
{
  bool ok = false;
  QgsTimeDuration resolution = QgsTimeDuration::fromString( itemText, ok );
  return resolution;
}

QDateTime QgsWmsSettings::parseWmstDateTimes( const QString &item )
{
  // Standard item will have YYYY-MM-DDTHH:mm:ss.SSSZ
  //  format a Qt::ISODateWithMs

  // Check if it does not have time part
  if ( !item.contains( 'T' ) )
    return QDateTime::fromString( item, QStringLiteral( "yyyy-MM-dd" ) );
  else if ( item.contains( '.' ) )
    return QDateTime::fromString( item, Qt::ISODateWithMs );
  else
    return QDateTime::fromString( item, Qt::ISODate );
}

QgsDateTimeRange QgsWmsSettings::parseWmtsTimeValue( const QString &value, QgsWmtsTileLayer::WmtsTimeFormat &format )
{
  // no standards here, we just have to be flexible..!
  // because we have to reconstruct values in the same exact formats later, each format match
  // must be specific to ONE SINGULAR format only! (ie. we can't make these formats tolerant to - vs /, etc --
  // each one must be handled individually).

  // YYYYMMDD format, eg
  // 20210101
  const thread_local QRegularExpression rxYYYYMMDD( QStringLiteral( "^\\s*(\\d{4})(\\d{2})(\\d{2})\\s*$" ) );
  QRegularExpressionMatch match = rxYYYYMMDD.match( value );
  if ( match.hasMatch() )
  {
    const QDate date( match.captured( 1 ).toInt(), match.captured( 2 ).toInt(), match.captured( 3 ).toInt() );
    format = QgsWmtsTileLayer::WmtsTimeFormat::yyyyMMdd;

    return QgsDateTimeRange( date.startOfDay(), date.endOfDay( ) );
  }

  // YYYY-MM-DD format, eg
  // 2021-01-01
  const thread_local QRegularExpression rxYYYY_MM_DD( QStringLiteral( "^\\s*(\\d{4})-(\\d{2})-(\\d{2})\\s*$" ) );
  match = rxYYYY_MM_DD.match( value );
  if ( match.hasMatch() )
  {
    const QDate date( match.captured( 1 ).toInt(), match.captured( 2 ).toInt(), match.captured( 3 ).toInt() );
    format = QgsWmtsTileLayer::WmtsTimeFormat::yyyy_MM_dd;

    return QgsDateTimeRange( date.startOfDay(), date.endOfDay( ) );
  }

  // YYYY format, eg
  // 2021
  const thread_local QRegularExpression rxYYYY( QStringLiteral( "^\\s*(\\d{4})\\s*$" ) );
  match = rxYYYY.match( value );
  if ( match.hasMatch() )
  {
    const QDate startDate( match.captured( 1 ).toInt(), 1, 1 );
    const QDate endDate( match.captured( 1 ).toInt(), 12, 31 );
    format = QgsWmtsTileLayer::WmtsTimeFormat::yyyy;
    return QgsDateTimeRange( startDate.startOfDay(), endDate.endOfDay( ) );
  }

  // YYYY-MM-DDTHH:mm:ss.SSSZ
  const thread_local QRegularExpression rxYYYYMMDDHHmmssSSSz( QStringLiteral( "^\\s*(\\d{4})-(\\d{2})-(\\d{2})T(\\d{2}):(\\d{2}):(\\d{2})Z\\s*$" ) );
  match = rxYYYYMMDDHHmmssSSSz.match( value );
  if ( match.hasMatch() )
  {
    const QDate date( match.captured( 1 ).toInt(), match.captured( 2 ).toInt(), match.captured( 3 ).toInt() );
    const QTime time( match.captured( 4 ).toInt(), match.captured( 5 ).toInt(), match.captured( 6 ).toInt() );
    format = QgsWmtsTileLayer::WmtsTimeFormat::yyyyMMddThhmmssZ;
    return QgsDateTimeRange( QDateTime( date, time ), QDateTime( date, time ) );
  }

  return QgsDateTimeRange();
}


// ----------------------


QgsWmsCapabilities::QgsWmsCapabilities( const QgsCoordinateTransformContext &coordinateTransformContext, const QString &baseUrl ):
  mCoordinateTransformContext( coordinateTransformContext ),
  mBaseUrl( baseUrl )
{

}

bool QgsWmsCapabilities::parseResponse( const QByteArray &response, QgsWmsParserSettings settings )
{
  mParserSettings = settings;
  mValid = false;

  if ( response.isEmpty() )
  {
    if ( mError.isEmpty() )
    {
      mErrorFormat = QStringLiteral( "text/plain" );
      mError = QObject::tr( "empty capabilities document" );
    }
    QgsDebugError( QStringLiteral( "response is empty" ) );
    return false;
  }

  if ( response.startsWith( "<html>" ) ||
       response.startsWith( "<HTML>" ) )
  {
    mErrorFormat = QStringLiteral( "text/html" );
    mError = response;
    QgsDebugError( QStringLiteral( "starts with <html>" ) );
    return false;
  }


  QgsDebugMsgLevel( QStringLiteral( "Converting to Dom." ), 2 );

  bool domOK;
  domOK = parseCapabilitiesDom( response, mCapabilities );

  if ( !domOK )
  {
    // We had an Dom exception -
    // mErrorCaption and mError are pre-filled by parseCapabilitiesDom

    // TODO[MD] mError += QObject::tr( "\nTried URL: %1" ).arg( url );

    QgsDebugError( "!domOK: " + mError );

    return false;
  }

  // get identify formats
  for ( const QString &f : std::as_const( mCapabilities.capability.request.getFeatureInfo.format ) )
  {
    // Don't use mSupportedGetFeatureFormats, there are too many possibilities
    QgsDebugMsgLevel( "supported format = " + f, 2 );
    // 1.0: MIME - server shall choose format, we presume it to be plain text
    //      GML.1, GML.2, or GML.3
    // 1.1.0, 1.3.0 - mime types, GML should use application/vnd.ogc.gml
    //      but in UMN Mapserver it may be also OUTPUTFORMAT, e.g. OGRGML
    Qgis::RasterIdentifyFormat format = Qgis::RasterIdentifyFormat::Undefined;
    if ( ( f == QLatin1String( "MIME" ) ) // 1.0
         || ( f == QLatin1String( "text/plain" ) ) )
      format = Qgis::RasterIdentifyFormat::Text;
    else if ( f == QLatin1String( "text/html" ) )
      format = Qgis::RasterIdentifyFormat::Html;
    else if ( f.startsWith( QLatin1String( "GML." ) ) || f == QLatin1String( "application/vnd.ogc.gml" ) || f == QLatin1String( "application/json" ) || f == QLatin1String( "application/geojson" ) || f == QLatin1String( "application/geo+json" ) || f.contains( QLatin1String( "gml" ), Qt::CaseInsensitive ) || ( f == QLatin1String( "text/xml" ) && !mBaseUrl.contains( "MapServer" ) ) )
      format = Qgis::RasterIdentifyFormat::Feature;

    mIdentifyFormats.insert( format, f );
  }

  mValid = mError.isEmpty();
  return mValid;
}


bool QgsWmsCapabilities::parseCapabilitiesDom( const QByteArray &xml, QgsWmsCapabilitiesProperty &capabilitiesProperty )
{

#ifdef QGISDEBUG
  QFile file( QDir::tempPath() + "/qgis-wmsprovider-capabilities.xml" );
  if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    file.write( xml );
    file.close();
  }
#endif

  // Convert completed document into a Dom
  QDomDocument capabilitiesDom;
  QString errorMsg;
  int errorLine;
  int errorColumn;
  bool contentSuccess = capabilitiesDom.setContent( xml, false, &errorMsg, &errorLine, &errorColumn );

  if ( !contentSuccess )
  {
    mErrorCaption = QObject::tr( "Dom Exception" );
    mErrorFormat = QStringLiteral( "text/plain" );
    mError = QObject::tr( "Could not get WMS capabilities: %1 at line %2 column %3\nThis is probably due to an incorrect WMS Server URL.\nResponse was:\n\n%4" )
             .arg( errorMsg )
             .arg( errorLine )
             .arg( errorColumn )
             .arg( QString( xml ) );

    QgsLogger::debug( "Dom Exception: " + mError );

    return false;
  }

  QDomElement docElement = capabilitiesDom.documentElement();

  // Assert that the DTD is what we expected (i.e. a WMS Capabilities document)
  QgsDebugMsgLevel( "testing tagName " + docElement.tagName(), 2 );

  if (
    docElement.tagName() != QLatin1String( "WMS_Capabilities" )  && // (1.3 vintage)
    docElement.tagName() != QLatin1String( "WMT_MS_Capabilities" ) && // (1.1.1 vintage)
    docElement.tagName() != QLatin1String( "Capabilities" )         // WMTS
  )
  {
    mErrorCaption = QObject::tr( "Dom Exception" );
    mErrorFormat = QStringLiteral( "text/plain" );
    mError = QObject::tr( "Could not get WMS capabilities in the expected format (DTD): no %1 or %2 found.\nThis might be due to an incorrect WMS Server URL.\nTag: %3\nResponse was:\n%4" )
             .arg( QStringLiteral( "WMS_Capabilities" ),
                   QStringLiteral( "WMT_MS_Capabilities" ),
                   docElement.tagName(),
                   QString( xml ) );

    QgsLogger::debug( "Dom Exception: " + mError );

    return false;
  }

  capabilitiesProperty.version = docElement.attribute( QStringLiteral( "version" ) );

  // Start walking through XML.
  QDomNode node = docElement.firstChild();

  while ( !node.isNull() )
  {
    QDomElement element = node.toElement();
    if ( !element.isNull() )
    {
      QgsDebugMsgLevel( element.tagName(), 2 ); // the node really is an element.

      if ( element.tagName() == QLatin1String( "Service" ) || element.tagName() == QLatin1String( "ows:ServiceProvider" ) || element.tagName() == QLatin1String( "ows:ServiceIdentification" ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "  Service." ), 2 );
        parseService( element, capabilitiesProperty.service );
      }
      else if ( element.tagName() == QLatin1String( "Capability" ) || element.tagName() == QLatin1String( "ows:OperationsMetadata" ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "  Capability." ), 2 );
        parseCapability( element, capabilitiesProperty.capability );
      }
      else if ( element.tagName() == QLatin1String( "Contents" ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "  Contents." ), 2 );
        parseWMTSContents( element );
      }
    }
    node = node.nextSibling();
  }

  return true;
}


void QgsWmsCapabilities::parseService( const QDomElement &element, QgsWmsServiceProperty &serviceProperty )
{

  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    if ( !nodeElement.isNull() )
    {
      QString tagName = nodeElement.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );
      if ( tagName.startsWith( QLatin1String( "ows:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "Title" ) )
      {
        serviceProperty.title = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "Abstract" ) )
      {
        serviceProperty.abstract = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "KeywordList" ) || tagName == QLatin1String( "Keywords" ) )
      {
        parseKeywordList( nodeElement, serviceProperty.keywordList );
      }
      else if ( tagName == QLatin1String( "OnlineResource" ) )
      {
        parseOnlineResource( nodeElement, serviceProperty.onlineResource );
      }
      else if ( tagName == QLatin1String( "ContactInformation" ) || tagName == QLatin1String( "ServiceContact" ) )
      {
        parseContactInformation( nodeElement, serviceProperty.contactInformation );
      }
      else if ( tagName == QLatin1String( "Fees" ) )
      {
        serviceProperty.fees = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "AccessConstraints" ) )
      {
        serviceProperty.accessConstraints = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "LayerLimit" ) )
      {
        serviceProperty.layerLimit = nodeElement.text().toUInt();
      }
      else if ( tagName == QLatin1String( "MaxWidth" ) )
      {
        serviceProperty.maxWidth = nodeElement.text().toUInt();
      }
      else if ( tagName == QLatin1String( "MaxHeight" ) )
      {
        serviceProperty.maxHeight = nodeElement.text().toUInt();
      }
    }
    node = node.nextSibling();
  }
}


void QgsWmsCapabilities::parseOnlineResource( const QDomElement &element, QgsWmsOnlineResourceAttribute &onlineResourceAttribute )
{
  QUrl url = QUrl::fromEncoded( element.attribute( QStringLiteral( "xlink:href" ) ).toUtf8() );
  if ( url.isRelative() )
  {
    const QUrl baseUrl = QUrl( mBaseUrl );
    url = baseUrl.resolved( url );
  }
  onlineResourceAttribute.xlinkHref = url.toString();
}


void QgsWmsCapabilities::parseKeywordList( const QDomElement &element, QStringList &keywordListProperty )
{

  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    if ( !nodeElement.isNull() )
    {
      QString tagName = nodeElement.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );
      if ( tagName.startsWith( QLatin1String( "ows:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "Keyword" ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "      Keyword." ), 2 );
        keywordListProperty += nodeElement.text();
      }
    }
    node = node.nextSibling();
  }
}

void QgsWmsCapabilities::parseContactInformation( const QDomElement &element, QgsWmsContactInformationProperty &contactInformationProperty )
{

  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    if ( !nodeElement.isNull() )
    {
      QString tagName = nodeElement.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "ContactPersonPrimary" ) )
      {
        parseContactPersonPrimary( nodeElement, contactInformationProperty.contactPersonPrimary );
      }
      else if ( tagName == QLatin1String( "ContactPosition" ) || tagName == QLatin1String( "ows:PositionName" ) )
      {
        contactInformationProperty.contactPosition = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "ContactAddress" ) )
      {
        parseContactAddress( nodeElement, contactInformationProperty.contactAddress );
      }
      else if ( tagName == QLatin1String( "ContactVoiceTelephone" ) )
      {
        contactInformationProperty.contactVoiceTelephone = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "ContactFacsimileTelephone" ) )
      {
        contactInformationProperty.contactFacsimileTelephone = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "ContactElectronicMailAddress" ) )
      {
        contactInformationProperty.contactElectronicMailAddress = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "ows:IndividualName" ) )
      {
        contactInformationProperty.contactPersonPrimary.contactPerson = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "ows:ProviderName" ) )
      {
        contactInformationProperty.contactPersonPrimary.contactOrganization = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "ows:ContactInfo" ) )
      {
        QDomNode tempNode = node.firstChildElement( QStringLiteral( "ows:Phone" ) );
        contactInformationProperty.contactVoiceTelephone        = tempNode.firstChildElement( QStringLiteral( "ows:Voice" ) ).toElement().text();
        contactInformationProperty.contactFacsimileTelephone    = tempNode.firstChildElement( QStringLiteral( "ows:Facsimile" ) ).toElement().text();

        tempNode = node.firstChildElement( QStringLiteral( "ows:Address" ) );
        contactInformationProperty.contactElectronicMailAddress   = tempNode.firstChildElement( QStringLiteral( "ows:ElectronicMailAddress" ) ).toElement().text();
        contactInformationProperty.contactAddress.address         = tempNode.firstChildElement( QStringLiteral( "ows:DeliveryPoint" ) ).toElement().text();
        contactInformationProperty.contactAddress.city            = tempNode.firstChildElement( QStringLiteral( "ows:City" ) ).toElement().text();
        contactInformationProperty.contactAddress.stateOrProvince = tempNode.firstChildElement( QStringLiteral( "ows:AdministrativeArea" ) ).toElement().text();
        contactInformationProperty.contactAddress.postCode        = tempNode.firstChildElement( QStringLiteral( "ows:PostalCode" ) ).toElement().text();
        contactInformationProperty.contactAddress.country         = tempNode.firstChildElement( QStringLiteral( "ows:Country" ) ).toElement().text();
      }
    }
    node = node.nextSibling();
  }
}

void QgsWmsCapabilities::parseContactPersonPrimary( const QDomElement &element, QgsWmsContactPersonPrimaryProperty &contactPersonPrimaryProperty )
{

  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    {
      QString tagName = nodeElement.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "ContactPerson" ) )
      {
        contactPersonPrimaryProperty.contactPerson = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "ContactOrganization" ) )
      {
        contactPersonPrimaryProperty.contactOrganization = nodeElement.text();
      }
    }
    node = node.nextSibling();
  }
}


void QgsWmsCapabilities::parseContactAddress( const QDomElement &element, QgsWmsContactAddressProperty &contactAddressProperty )
{

  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    if ( !nodeElement.isNull() )
    {
      QString tagName = nodeElement.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "AddressType" ) )
      {
        contactAddressProperty.addressType = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "Address" ) )
      {
        contactAddressProperty.address = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "City" ) )
      {
        contactAddressProperty.city = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "StateOrProvince" ) )
      {
        contactAddressProperty.stateOrProvince = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "PostCode" ) )
      {
        contactAddressProperty.postCode = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "Country" ) )
      {
        contactAddressProperty.country = nodeElement.text();
      }
    }
    node = node.nextSibling();
  }
}


void QgsWmsCapabilities::parseCapability( const QDomElement &element, QgsWmsCapabilityProperty &capabilityProperty )
{

  for ( QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling() )
  {
    QDomElement nodeElement = node.toElement();
    if ( nodeElement.isNull() )
      continue;

    QString tagName = nodeElement.tagName();
    if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
      tagName = tagName.mid( 4 );

    QgsDebugMsgLevel( "  "  + nodeElement.tagName(), 2 ); // the node really is an element.

    if ( tagName == QLatin1String( "Request" ) )
    {
      parseRequest( nodeElement, capabilityProperty.request );
    }
    else if ( tagName == QLatin1String( "Layer" ) )
    {
      QgsWmsLayerProperty layer;
      parseLayer( nodeElement, layer );
      capabilityProperty.layers.push_back( layer );
    }
    else if ( tagName == QLatin1String( "VendorSpecificCapabilities" ) )
    {
      for ( int i = 0; i < nodeElement.childNodes().size(); i++ )
      {
        QDomNode childNode = nodeElement.childNodes().item( i );
        QDomElement childNodeElement = childNode.toElement();

        QString tagName = childNodeElement.tagName();
        if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
          tagName = tagName.mid( 4 );

        if ( tagName == QLatin1String( "TileSet" ) )
        {
          parseTileSetProfile( childNodeElement );
        }
      }
    }
    else if ( tagName == QLatin1String( "ows:Operation" ) )
    {
      QString name = nodeElement.attribute( QStringLiteral( "name" ) );
      QDomElement get = node.firstChildElement( QStringLiteral( "ows:DCP" ) )
                        .firstChildElement( QStringLiteral( "ows:HTTP" ) )
                        .firstChildElement( QStringLiteral( "ows:Get" ) );

      QString href = get.attribute( QStringLiteral( "xlink:href" ) );

      QgsWmsDcpTypeProperty dcp;
      dcp.http.get.onlineResource.xlinkHref = href;

      QgsWmsOperationType *operationType = nullptr;
      if ( href.isNull() )
      {
        QgsDebugError( QStringLiteral( "http get missing from ows:Operation '%1'" ).arg( name ) );
      }
      else if ( name == QLatin1String( "GetTile" ) )
      {
        operationType = &capabilityProperty.request.getTile;
      }
      else if ( name == QLatin1String( "GetFeatureInfo" ) )
      {
        operationType = &capabilityProperty.request.getFeatureInfo;
      }
      else if ( name == QLatin1String( "GetLegendGraphic" ) || name == QLatin1String( "sld:GetLegendGraphic" ) )
      {
        operationType = &capabilityProperty.request.getLegendGraphic;
      }
      else
      {
        QgsDebugError( QStringLiteral( "ows:Operation %1 ignored" ).arg( name ) );
      }

      if ( operationType )
      {
        operationType->dcpType << dcp;
        operationType->allowedEncodings.clear();
        for ( QDomElement childNodeElement = get.firstChildElement( QStringLiteral( "ows:Constraint" ) ).firstChildElement( QStringLiteral( "ows:AllowedValues" ) ).firstChildElement( QStringLiteral( "ows:Value" ) );
              !childNodeElement.isNull();
              childNodeElement = nodeElement.nextSiblingElement( QStringLiteral( "ows:Value" ) ) )
        {
          operationType->allowedEncodings << childNodeElement.text();
        }
      }
    }
  }

  // Layers in a WMS-C Capabilities TileSet do not have layer title or abstract
  // still those could be present in a Layer list.
  if ( !mTileLayersSupported.isEmpty() )
  {
    QHash<QString, QString> titles;
    QHash<QString, QString> abstracts;

    // Build layer identifier - title|abstract mapping
    for ( const QgsWmsLayerProperty &layer : std::as_const( mLayersSupported ) )
    {
      if ( !layer.name.isEmpty() )
      {
        if ( !layer.title.isEmpty() )
        {
          titles.insert( layer.name, layer.title );
        }

        if ( !layer.abstract.isEmpty() )
        {
          abstracts.insert( layer.name, layer.abstract );
        }
      }
    }

    // If tile layer title|abstract is empty, try to use one from a matching layer
    for ( QgsWmtsTileLayer &tileLayer : mTileLayersSupported )
    {
      if ( tileLayer.title.isEmpty() && titles.contains( tileLayer.identifier ) )
      {
        tileLayer.title = titles.value( tileLayer.identifier );
      }

      if ( tileLayer.abstract.isEmpty() && abstracts.contains( tileLayer.identifier ) )
      {
        tileLayer.abstract = abstracts.value( tileLayer.identifier );
      }
    }
  }
}


void QgsWmsCapabilities::parseRequest( const QDomElement &element, QgsWmsRequestProperty &requestProperty )
{

  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    {
      QString operation = nodeElement.tagName();
      if ( operation == QLatin1String( "Operation" ) )
      {
        operation = nodeElement.attribute( QStringLiteral( "name" ) );
      }

      if ( operation == QLatin1String( "GetMap" ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "      GetMap." ), 2 );
        parseOperationType( nodeElement, requestProperty.getMap );
      }
      else if ( operation == QLatin1String( "GetFeatureInfo" ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "      GetFeatureInfo." ), 2 );
        parseOperationType( nodeElement, requestProperty.getFeatureInfo );
      }
      else if ( operation == QLatin1String( "GetLegendGraphic" ) || operation == QLatin1String( "sld:GetLegendGraphic" ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "      GetLegendGraphic." ), 2 );
        parseOperationType( nodeElement, requestProperty.getLegendGraphic );
      }
    }
    node = node.nextSibling();
  }
}



void QgsWmsCapabilities::parseLegendUrl( const QDomElement &element, QgsWmsLegendUrlProperty &legendUrlProperty )
{

  legendUrlProperty.width  = element.attribute( QStringLiteral( "width" ) ).toUInt();
  legendUrlProperty.height = element.attribute( QStringLiteral( "height" ) ).toUInt();

  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    if ( !nodeElement.isNull() )
    {
      QString tagName = nodeElement.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "Format" ) )
      {
        legendUrlProperty.format = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "OnlineResource" ) )
      {
        parseOnlineResource( nodeElement, legendUrlProperty.onlineResource );
      }
    }
    node = node.nextSibling();
  }
}

void QgsWmsCapabilities::parseDimension( const QDomElement &element, QgsWmsDimensionProperty &dimensionProperty )
{
  dimensionProperty.name = element.attribute( QStringLiteral( "name" ) );
  dimensionProperty.units = element.attribute( QStringLiteral( "units" ) );
  dimensionProperty.unitSymbol = element.attribute( QStringLiteral( "unitSymbol" ) );
  dimensionProperty.defaultValue = element.attribute( QStringLiteral( "default" ) );

  if ( !element.attribute( QStringLiteral( "multipleValues" ) ).isNull() )
  {
    QString multipleValuesAttribute = element.attribute( QStringLiteral( "multipleValues" ) );
    dimensionProperty.multipleValues = ( multipleValuesAttribute == QLatin1String( "1" ) || multipleValuesAttribute == QLatin1String( "true" ) );
  }

  if ( !element.attribute( QStringLiteral( "nearestValue" ) ).isNull() )
  {
    QString nearestValueAttribute = element.attribute( QStringLiteral( "nearestValue" ) );
    dimensionProperty.nearestValue = ( nearestValueAttribute == QLatin1String( "1" ) || nearestValueAttribute == QLatin1String( "true" ) );
  }

  if ( !element.attribute( QStringLiteral( "current" ) ).isNull() )
  {
    QString currentAttribute = element.attribute( QStringLiteral( "current" ) );
    dimensionProperty.current = ( currentAttribute == QLatin1String( "1" ) || currentAttribute == QLatin1String( "true" ) );
  }

  dimensionProperty.extent = element.text().simplified();
}

void QgsWmsCapabilities::parseExtent( const QDomElement &element, QVector<QgsWmsDimensionProperty> &dimensionProperties )
{
  const QString name = element.attribute( QStringLiteral( "name" ) );
  // try to find corresponding dimension property -- i.e. we upgrade the WMS 1.1 split of Dimension and Extent to 1.3 style where Dimension holds the extent information
  for ( auto it = dimensionProperties.begin(); it != dimensionProperties.end(); ++it )
  {
    if ( it->name == name )
    {
      it->extent = element.text().simplified();

      it->defaultValue = element.attribute( QStringLiteral( "default" ) );

      if ( !element.attribute( QStringLiteral( "multipleValues" ) ).isNull() )
      {
        QString multipleValuesAttribute = element.attribute( QStringLiteral( "multipleValues" ) );
        it->multipleValues = ( multipleValuesAttribute == QLatin1String( "1" ) || multipleValuesAttribute == QLatin1String( "true" ) );
      }

      if ( !element.attribute( QStringLiteral( "nearestValue" ) ).isNull() )
      {
        QString nearestValueAttribute = element.attribute( QStringLiteral( "nearestValue" ) );
        it->nearestValue = ( nearestValueAttribute == QLatin1String( "1" ) || nearestValueAttribute == QLatin1String( "true" ) );
      }

      if ( !element.attribute( QStringLiteral( "current" ) ).isNull() )
      {
        QString currentAttribute = element.attribute( QStringLiteral( "current" ) );
        it->current = ( currentAttribute == QLatin1String( "1" ) || currentAttribute == QLatin1String( "true" ) );
      }
    }
  }
}

void QgsWmsCapabilities::parseMetadataUrl( const QDomElement &element, QgsWmsMetadataUrlProperty &metadataUrlProperty )
{

  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    if ( !nodeElement.isNull() )
    {
      QString tagName = nodeElement.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ), Qt::CaseInsensitive ) )
        tagName = tagName.mid( 4 );

      if ( tagName.compare( QLatin1String( "Format" ), Qt::CaseInsensitive ) == 0 )
      {
        metadataUrlProperty.format = nodeElement.text();
      }
      else if ( tagName.compare( QLatin1String( "OnlineResource" ), Qt::CaseInsensitive ) == 0 )
      {
        parseOnlineResource( nodeElement, metadataUrlProperty.onlineResource );
      }
    }
    node = node.nextSibling();
  }
}


void QgsWmsCapabilities::parseLayer( const QDomElement &element, QgsWmsLayerProperty &layerProperty,
                                     QgsWmsLayerProperty *parentProperty )
{

// TODO: Delete this stanza completely, depending on success of "Inherit things into the sublayer" below.
#if 0
  // enforce WMS non-inheritance rules
  layerProperty.name = QString();
  layerProperty.title = QString();
  layerProperty.abstract = QString();
  layerProperty.keywordList.clear();
#endif

  layerProperty.orderId     = ++mLayerCount;

  QString queryableAttribute = element.attribute( QStringLiteral( "queryable" ) );
  layerProperty.queryable = queryableAttribute == QLatin1String( "1" ) || queryableAttribute.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0;

  layerProperty.cascaded    = element.attribute( QStringLiteral( "cascaded" ) ).toUInt();

  QString opaqueAttribute = element.attribute( QStringLiteral( "opaque" ) );
  layerProperty.opaque = opaqueAttribute == QLatin1String( "1" ) || opaqueAttribute.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0;

  QString noSubsetsAttribute = element.attribute( QStringLiteral( "noSubsets" ) );
  layerProperty.noSubsets = noSubsetsAttribute == QLatin1String( "1" ) || noSubsetsAttribute.compare( QLatin1String( "true" ), Qt::CaseInsensitive ) == 0;

  layerProperty.fixedWidth  = element.attribute( QStringLiteral( "fixedWidth" ) ).toUInt();
  layerProperty.fixedHeight = element.attribute( QStringLiteral( "fixedHeight" ) ).toUInt();

  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    if ( !nodeElement.isNull() )
    {
      QString tagName = nodeElement.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "Layer" ) )
      {
        QgsWmsLayerProperty subLayerProperty;

        // Inherit things into the sublayer
        //   Ref: 7.2.4.8 Inheritance of layer properties

        // Cleanup inherited styles, replace layer name with current layer name
        QVector<QgsWmsStyleProperty> inheritedStyles { layerProperty.style };
        if ( ! nodeElement.firstChildElement( QStringLiteral( "Name" ) ).isNull() )
        {
          const QString layerName { nodeElement.firstChildElement( QStringLiteral( "Name" ) ).text() };
          for ( auto stylesIt = inheritedStyles.begin(); stylesIt != inheritedStyles.end(); ++stylesIt )
          {
            for ( auto legendUriIt = stylesIt->legendUrl.begin(); legendUriIt != stylesIt->legendUrl.end(); ++legendUriIt )
            {
              QUrl legendUrl { legendUriIt->onlineResource.xlinkHref };
              if ( legendUrl.hasQuery() )
              {
                QUrlQuery query { legendUrl.query() };
                if ( query.hasQueryItem( QStringLiteral( "LAYER" ) ) )
                {
                  query.removeQueryItem( QStringLiteral( "LAYER" ) );
                  query.addQueryItem( QStringLiteral( "LAYER" ), layerName );
                }
                else if ( query.hasQueryItem( QStringLiteral( "layer" ) ) )
                {
                  query.removeQueryItem( QStringLiteral( "layer" ) );
                  query.addQueryItem( QStringLiteral( "layer" ), layerName );
                }
                legendUrl.setQuery( query );
                legendUriIt->onlineResource.xlinkHref = legendUrl.url( );
              }
            }
          }
        }

        subLayerProperty.style                    = inheritedStyles;
        subLayerProperty.crs                      = layerProperty.crs;
        subLayerProperty.boundingBoxes            = layerProperty.boundingBoxes;
        subLayerProperty.ex_GeographicBoundingBox = layerProperty.ex_GeographicBoundingBox;
        // TODO

        parseLayer( nodeElement, subLayerProperty, &layerProperty );

        layerProperty.layer.push_back( subLayerProperty );
      }
      else if ( tagName == QLatin1String( "Name" ) )
      {
        layerProperty.name = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "Title" ) )
      {
        layerProperty.title = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "Abstract" ) )
      {
        layerProperty.abstract = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "KeywordList" ) )
      {
        parseKeywordList( nodeElement, layerProperty.keywordList );
      }
      else if ( tagName == QLatin1String( "SRS" ) || tagName == QLatin1String( "CRS" ) )
      {
        // CRS can contain several definitions separated by whitespace
        // though this was deprecated in WMS 1.1.1
        const QStringList crsList = nodeElement.text().split( QRegularExpression( "\\s+" ) );
        for ( const QString &srs : crsList )
        {
          if ( !layerProperty.crs.contains( srs ) )
            layerProperty.crs.push_back( srs );
        }
      }
      else if ( tagName == QLatin1String( "LatLonBoundingBox" ) )    // legacy from earlier versions of WMS
      {
        // boundingBox element can contain comma as decimal separator and layer extent is not
        // calculated at all. Fixing by replacing comma with point.
        layerProperty.ex_GeographicBoundingBox = QgsRectangle(
              nodeElement.attribute( QStringLiteral( "minx" ) ).replace( ',', '.' ).toDouble(),
              nodeElement.attribute( QStringLiteral( "miny" ) ).replace( ',', '.' ).toDouble(),
              nodeElement.attribute( QStringLiteral( "maxx" ) ).replace( ',', '.' ).toDouble(),
              nodeElement.attribute( QStringLiteral( "maxy" ) ).replace( ',', '.' ).toDouble()
            );

        if ( nodeElement.hasAttribute( QStringLiteral( "SRS" ) ) && nodeElement.attribute( QStringLiteral( "SRS" ) ) != DEFAULT_LATLON_CRS )
        {
          try
          {
            QgsCoordinateReferenceSystem src = QgsCoordinateReferenceSystem::fromOgcWmsCrs( nodeElement.attribute( QStringLiteral( "SRS" ) ) );
            QgsCoordinateReferenceSystem dst = QgsCoordinateReferenceSystem::fromOgcWmsCrs( DEFAULT_LATLON_CRS );
            QgsCoordinateTransform ct( src, dst, mCoordinateTransformContext );
            ct.setBallparkTransformsAreAppropriate( true );
            layerProperty.ex_GeographicBoundingBox = ct.transformBoundingBox( layerProperty.ex_GeographicBoundingBox );
          }
          catch ( QgsCsException &cse )
          {
            Q_UNUSED( cse )
          }
        }
      }
      else if ( tagName == QLatin1String( "EX_GeographicBoundingBox" ) ) //for WMS 1.3
      {
        QDomElement wBoundLongitudeElem, eBoundLongitudeElem, sBoundLatitudeElem, nBoundLatitudeElem;

        if ( nodeElement.tagName() == QLatin1String( "wms:EX_GeographicBoundingBox" ) )
        {
          wBoundLongitudeElem = node.namedItem( QStringLiteral( "wms:westBoundLongitude" ) ).toElement();
          eBoundLongitudeElem = node.namedItem( QStringLiteral( "wms:eastBoundLongitude" ) ).toElement();
          sBoundLatitudeElem = node.namedItem( QStringLiteral( "wms:southBoundLatitude" ) ).toElement();
          nBoundLatitudeElem = node.namedItem( QStringLiteral( "wms:northBoundLatitude" ) ).toElement();
        }
        else
        {
          wBoundLongitudeElem = node.namedItem( QStringLiteral( "westBoundLongitude" ) ).toElement();
          eBoundLongitudeElem = node.namedItem( QStringLiteral( "eastBoundLongitude" ) ).toElement();
          sBoundLatitudeElem = node.namedItem( QStringLiteral( "southBoundLatitude" ) ).toElement();
          nBoundLatitudeElem = node.namedItem( QStringLiteral( "northBoundLatitude" ) ).toElement();
        }

        double wBLong, eBLong, sBLat, nBLat;
        bool wBOk, eBOk, sBOk, nBOk;
        // boundingBox element can contain comma as decimal separator and layer extent is not
        // calculated at all. Fixing by replacing comma with point.
        wBLong = wBoundLongitudeElem.text().replace( ',', '.' ).toDouble( &wBOk );
        eBLong = eBoundLongitudeElem.text().replace( ',', '.' ).toDouble( &eBOk );
        sBLat = sBoundLatitudeElem.text().replace( ',', '.' ).toDouble( &sBOk );
        nBLat = nBoundLatitudeElem.text().replace( ',', '.' ).toDouble( &nBOk );
        if ( wBOk && eBOk && sBOk && nBOk )
        {
          layerProperty.ex_GeographicBoundingBox = QgsRectangle( wBLong, sBLat, eBLong, nBLat );
        }
      }
      else if ( tagName == QLatin1String( "BoundingBox" ) )
      {
        QgsWmsBoundingBoxProperty bbox;
        bbox.box = QgsRectangle( nodeElement.attribute( QStringLiteral( "minx" ) ).replace( ',', '.' ).toDouble(),
                                 nodeElement.attribute( QStringLiteral( "miny" ) ).replace( ',', '.' ).toDouble(),
                                 nodeElement.attribute( QStringLiteral( "maxx" ) ).replace( ',', '.' ).toDouble(),
                                 nodeElement.attribute( QStringLiteral( "maxy" ) ).replace( ',', '.' ).toDouble()
                               );
        if ( nodeElement.hasAttribute( QStringLiteral( "CRS" ) ) || nodeElement.hasAttribute( QStringLiteral( "SRS" ) ) )
        {
          if ( nodeElement.hasAttribute( QStringLiteral( "CRS" ) ) )
            bbox.crs = nodeElement.attribute( QStringLiteral( "CRS" ) );
          else if ( nodeElement.hasAttribute( QStringLiteral( "SRS" ) ) )
            bbox.crs = nodeElement.attribute( QStringLiteral( "SRS" ) );

          if ( shouldInvertAxisOrientation( bbox.crs ) )
          {
            QgsRectangle invAxisBbox( bbox.box.yMinimum(), bbox.box.xMinimum(),
                                      bbox.box.yMaximum(), bbox.box.xMaximum() );
            bbox.box = invAxisBbox;
          }

          // Overwrite existing bounding boxes with identical CRS
          bool inheritedOverwritten = false;
          for ( int i = 0; i < layerProperty.boundingBoxes.size(); i++ )
          {
            if ( layerProperty.boundingBoxes[i].crs == bbox.crs )
            {
              layerProperty.boundingBoxes[i] = bbox;
              inheritedOverwritten = true;
            }
          }
          if ( ! inheritedOverwritten )
            layerProperty.boundingBoxes << bbox;
        }
        else
        {
          QgsDebugError( QStringLiteral( "CRS/SRS attribute not found in BoundingBox" ) );
        }
      }
      else if ( tagName == QLatin1String( "Dimension" ) )
      {
        layerProperty.dimensions << QgsWmsDimensionProperty();
        parseDimension( nodeElement, layerProperty.dimensions.last() );
      }
      else if ( tagName == QLatin1String( "Extent" ) )
      {
        // upgrade WMS 1.1 style Extent/Dimension handling to WMS 1.3
        parseExtent( nodeElement, layerProperty.dimensions );
      }
      else if ( tagName == QLatin1String( "Attribution" ) )
      {
        // TODO
      }
      else if ( tagName == QLatin1String( "AuthorityURL" ) )
      {
        // TODO
      }
      else if ( tagName == QLatin1String( "Identifier" ) )
      {
        // TODO
      }
      else if ( tagName == QLatin1String( "MetadataURL" ) )
      {
        layerProperty.metadataUrl << QgsWmsMetadataUrlProperty();
        parseMetadataUrl( nodeElement, layerProperty.metadataUrl.last() );
      }
      else if ( tagName == QLatin1String( "DataURL" ) )
      {
        // TODO
      }
      else if ( tagName == QLatin1String( "FeatureListURL" ) )
      {
        // TODO
      }
      else if ( tagName == QLatin1String( "Style" ) )
      {
        QgsWmsStyleProperty styleProperty;

        parseStyle( nodeElement, styleProperty );

        for ( int i = 0; i < layerProperty.style.size(); ++i )
        {
          if ( layerProperty.style.at( i ).name == styleProperty.name )
          {
            // override inherited parent's style if it has the same name
            // according to the WMS spec, it should not happen, but Mapserver
            // does it all the time.
            layerProperty.style.remove( i );
            break;
          }
        }
        layerProperty.style.push_back( styleProperty );
      }
      else if ( tagName == QLatin1String( "MinScaleDenominator" ) )
      {
        // TODO
      }
      else if ( tagName == QLatin1String( "MaxScaleDenominator" ) )
      {
        // TODO
      }
      // If we got here then it's not in the WMS 1.3 standard

    }
    node = node.nextSibling();
  }

  if ( parentProperty )
  {
    mLayerParents[ layerProperty.orderId ] = parentProperty->orderId;
  }

  if ( !layerProperty.name.isEmpty() )
  {
    // We have all the information we need to properly evaluate a layer definition
    // TODO: Save this somewhere

    // Store if the layer is queryable
    mQueryableForLayer[ layerProperty.name ] = layerProperty.queryable;

    // Insert into the local class' registry
    mLayersSupported.push_back( layerProperty );

    //if there are several <Layer> elements without a parent layer, the style list needs to be cleared
    if ( layerProperty.layer.empty() )
    {
      layerProperty.style.clear();
    }
  }

  if ( !layerProperty.layer.empty() )
  {
    mLayerParentNames[ layerProperty.orderId ] = QStringList() << layerProperty.name << layerProperty.title << layerProperty.abstract;
  }

}


void QgsWmsCapabilities::parseStyle( const QDomElement &element, QgsWmsStyleProperty &styleProperty )
{

  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    if ( !nodeElement.isNull() )
    {
      QString tagName = nodeElement.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "Name" ) )
      {
        styleProperty.name = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "Title" ) )
      {
        styleProperty.title = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "Abstract" ) )
      {
        styleProperty.abstract = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "LegendURL" ) )
      {
        styleProperty.legendUrl << QgsWmsLegendUrlProperty();
        parseLegendUrl( nodeElement, styleProperty.legendUrl.last() );
      }
      else if ( tagName == QLatin1String( "StyleSheetURL" ) )
      {
        // TODO
      }
      else if ( tagName == QLatin1String( "StyleURL" ) )
      {
        // TODO
      }
    }
    node = node.nextSibling();
  }
}


void QgsWmsCapabilities::parseOperationType( const QDomElement &element, QgsWmsOperationType &operationType )
{

  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    if ( !nodeElement.isNull() )
    {
      QString tagName = nodeElement.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "Format" ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "      Format." ), 2 );
        operationType.format += nodeElement.text();
      }
      else if ( tagName == QLatin1String( "DCPType" ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "      DCPType." ), 2 );
        QgsWmsDcpTypeProperty dcp;
        parseDcpType( nodeElement, dcp );
        operationType.dcpType.push_back( dcp );
      }
    }
    node = node.nextSibling();
  }
}


void QgsWmsCapabilities::parseDcpType( const QDomElement &element, QgsWmsDcpTypeProperty &dcpType )
{

  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    if ( !nodeElement.isNull() )
    {
      if ( nodeElement.tagName() == QLatin1String( "HTTP" ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "      HTTP." ), 2 );
        parseHttp( nodeElement, dcpType.http );
      }
    }
    node = node.nextSibling();
  }
}

void QgsWmsCapabilities::parseHttp( const QDomElement &element, QgsWmsHttpProperty &httpProperty )
{

  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    if ( !nodeElement.isNull() )
    {
      QString tagName = nodeElement.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "Get" ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "      Get." ), 2 );
        parseGet( nodeElement, httpProperty.get );
      }
      else if ( tagName == QLatin1String( "Post" ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "      Post." ), 2 );
        parsePost( nodeElement, httpProperty.post );
      }
    }
    node = node.nextSibling();
  }
}

void QgsWmsCapabilities::parseGet( const QDomElement &element, QgsWmsGetProperty &getProperty )
{

  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    if ( !nodeElement.isNull() )
    {
      QString tagName = nodeElement.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "OnlineResource" ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "      OnlineResource." ), 2 );
        parseOnlineResource( nodeElement, getProperty.onlineResource );
      }
    }
    node = node.nextSibling();
  }
}

void QgsWmsCapabilities::parsePost( const QDomElement &element, QgsWmsPostProperty &postProperty )
{

  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    if ( !nodeElement.isNull() )
    {
      QString tagName = nodeElement.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "OnlineResource" ) )
      {
        QgsDebugMsgLevel( QStringLiteral( "      OnlineResource." ), 2 );
        parseOnlineResource( nodeElement, postProperty.onlineResource );
      }
    }
    node = node.nextSibling();
  }
}

void QgsWmsCapabilities::parseTileSetProfile( const QDomElement &element )
{
  QStringList resolutions, layers, styles;
  QgsWmsBoundingBoxProperty boundingBox;
  QgsWmtsTileMatrixSet matrixSet;
  QgsWmtsTileMatrix tileMatrix;
  QgsWmtsTileLayer tileLayer;
  tileLayer.dpi = -1;
  tileLayer.timeFormat = QgsWmtsTileLayer::WmtsTimeFormat::yyyyMMdd;

  // don't allow duplicate format/style/ strings
  QSet< QString > uniqueFormats;
  QSet< QString > uniqueStyles;

  tileLayer.tileMode = WMSC;

  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    {
      QgsDebugMsgLevel( "    "  + nodeElement.tagName(), 2 ); // the node really is an element.

      QString tagName = nodeElement.tagName();
      if ( tagName.startsWith( QLatin1String( "wms:" ) ) )
        tagName = tagName.mid( 4 );

      if ( tagName == QLatin1String( "Layers" ) )
      {
        layers << nodeElement.text();
      }
      else if ( tagName == QLatin1String( "Styles" ) )
      {
        const QString style = nodeElement.text();
        if ( !uniqueStyles.contains( style ) )
        {
          styles << style;
          uniqueStyles.insert( style );
        }
      }
      else if ( tagName == QLatin1String( "Width" ) )
      {
        tileMatrix.tileWidth = nodeElement.text().toInt();
      }
      else if ( tagName == QLatin1String( "Height" ) )
      {
        tileMatrix.tileHeight = nodeElement.text().toInt();
      }
      else if ( tagName == QLatin1String( "SRS" ) )
      {
        matrixSet.crs = nodeElement.text();
      }
      else if ( tagName == QLatin1String( "Format" ) )
      {
        const QString format = nodeElement.text();
        if ( !uniqueFormats.contains( format ) )
        {
          tileLayer.formats << format;
          uniqueFormats.insert( format );
        }
      }
      else if ( tagName == QLatin1String( "BoundingBox" ) )
      {
        QgsWmsBoundingBoxProperty boundingBoxProperty;
        // boundingBox element can contain comma as decimal separator and layer extent is not
        // calculated at all. Fixing by replacing comma with point.
        boundingBoxProperty.box = QgsRectangle(
                                    nodeElement.attribute( QStringLiteral( "minx" ) ).replace( ',', '.' ).toDouble(),
                                    nodeElement.attribute( QStringLiteral( "miny" ) ).replace( ',', '.' ).toDouble(),
                                    nodeElement.attribute( QStringLiteral( "maxx" ) ).replace( ',', '.' ).toDouble(),
                                    nodeElement.attribute( QStringLiteral( "maxy" ) ).replace( ',', '.' ).toDouble()
                                  );
        if ( nodeElement.hasAttribute( QStringLiteral( "SRS" ) ) )
          boundingBoxProperty.crs = nodeElement.attribute( QStringLiteral( "SRS" ) );
        else if ( nodeElement.hasAttribute( QStringLiteral( "srs" ) ) )
          boundingBoxProperty.crs = nodeElement.attribute( QStringLiteral( "srs" ) );
        else if ( nodeElement.hasAttribute( QStringLiteral( "CRS" ) ) )
          boundingBoxProperty.crs = nodeElement.attribute( QStringLiteral( "CRS" ) );
        else if ( nodeElement.hasAttribute( QStringLiteral( "crs" ) ) )
          boundingBoxProperty.crs = nodeElement.attribute( QStringLiteral( "crs" ) );
        else
        {
          QgsDebugError( QStringLiteral( "crs of bounding box undefined" ) );
        }

        if ( !boundingBoxProperty.crs.isEmpty() )
        {
          QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( boundingBoxProperty.crs );
          if ( crs.isValid() )
            boundingBoxProperty.crs = crs.authid();

          tileLayer.boundingBoxes << boundingBoxProperty;
        }
      }
      else if ( tagName == QLatin1String( "Resolutions" ) )
      {
        resolutions = nodeElement.text().trimmed().split( ' ', Qt::SkipEmptyParts );
      }
      else
      {
        QgsDebugError( QStringLiteral( "tileset tag %1 ignored" ).arg( nodeElement.tagName() ) );
      }
    }
    node = node.nextSibling();
  }

  matrixSet.identifier = QStringLiteral( "%1-wmsc-%2" ).arg( layers.join( QLatin1Char( '_' ) ) ).arg( mTileLayersSupported.size() );

  tileLayer.identifier = layers.join( QLatin1Char( ',' ) );
  QgsWmtsStyle style;
  style.identifier = styles.join( QLatin1Char( ',' ) );
  tileLayer.styles.insert( style.identifier, style );
  tileLayer.defaultStyle = style.identifier;

  QgsWmtsTileMatrixSetLink setLink;
  setLink.tileMatrixSet = matrixSet.identifier;
  tileLayer.setLinks.insert( matrixSet.identifier, setLink );
  mTileLayersSupported.append( tileLayer );

  int i = 0;
  for ( const QString &rS : std::as_const( resolutions ) )
  {
    double r = rS.toDouble();
    tileMatrix.identifier = QString::number( i );
    Q_ASSERT( tileLayer.boundingBoxes.size() == 1 );
    tileMatrix.matrixWidth  = std::ceil( tileLayer.boundingBoxes.at( 0 ).box.width() / tileMatrix.tileWidth / r );
    tileMatrix.matrixHeight = std::ceil( tileLayer.boundingBoxes.at( 0 ).box.height() / tileMatrix.tileHeight / r );
    tileMatrix.topLeft = QgsPointXY( tileLayer.boundingBoxes.at( 0 ).box.xMinimum(), tileLayer.boundingBoxes.at( 0 ).box.yMinimum() + tileMatrix.matrixHeight * tileMatrix.tileHeight * r );
    tileMatrix.tres = r;
    matrixSet.tileMatrices.insert( r, tileMatrix );
    i++;
  }

  mTileMatrixSets.insert( matrixSet.identifier, matrixSet );
}

void QgsWmsCapabilities::parseWMTSContents( const QDomElement &element )
{
  //
  // tile matrix sets
  //

  mTileMatrixSets.clear();
  for ( QDomElement childElement = element.firstChildElement( QStringLiteral( "TileMatrixSet" ) );
        !childElement.isNull();
        childElement = childElement.nextSiblingElement( QStringLiteral( "TileMatrixSet" ) ) )
  {
    QgsWmtsTileMatrixSet set;
    set.identifier = childElement.firstChildElement( QStringLiteral( "ows:Identifier" ) ).text();
    set.title      = childElement.firstChildElement( QStringLiteral( "ows:Title" ) ).text();
    set.abstract   = childElement.firstChildElement( QStringLiteral( "ows:Abstract" ) ).text();
    parseKeywords( childElement, set.keywords );

    QString supportedCRS = childElement.firstChildElement( QStringLiteral( "ows:SupportedCRS" ) ).text();

    QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( supportedCRS );

    set.wkScaleSet = childElement.firstChildElement( QStringLiteral( "WellKnownScaleSet" ) ).text();

    double metersPerUnit = QgsUnitTypes::fromUnitToUnitFactor( crs.mapUnits(), Qgis::DistanceUnit::Meters );

    set.crs = crs.authid();

    bool invert = !mParserSettings.ignoreAxisOrientation && crs.hasAxisInverted();
    if ( mParserSettings.invertAxisOrientation )
      invert = !invert;

    QgsDebugMsgLevel( QStringLiteral( "tilematrix set: %1 (supportedCRS:%2 crs:%3; metersPerUnit:%4 axisInverted:%5)" )
                      .arg( set.identifier,
                            supportedCRS,
                            set.crs )
                      .arg( metersPerUnit, 0, 'f' )
                      .arg( invert ? "yes" : "no" ), 2
                    );

    for ( QDomElement secondChildElement = childElement.firstChildElement( QStringLiteral( "TileMatrix" ) );
          !secondChildElement.isNull();
          secondChildElement = secondChildElement.nextSiblingElement( QStringLiteral( "TileMatrix" ) ) )
    {
      QgsWmtsTileMatrix tileMatrix;

      tileMatrix.identifier = secondChildElement.firstChildElement( QStringLiteral( "ows:Identifier" ) ).text();
      tileMatrix.title      = secondChildElement.firstChildElement( QStringLiteral( "ows:Title" ) ).text();
      tileMatrix.abstract   = secondChildElement.firstChildElement( QStringLiteral( "ows:Abstract" ) ).text();
      parseKeywords( secondChildElement, tileMatrix.keywords );

      tileMatrix.scaleDenom = secondChildElement.firstChildElement( QStringLiteral( "ScaleDenominator" ) ).text().toDouble();

      QStringList topLeft = secondChildElement.firstChildElement( QStringLiteral( "TopLeftCorner" ) ).text().split( ' ' );
      if ( topLeft.size() == 2 )
      {
        if ( invert )
        {
          tileMatrix.topLeft.set( topLeft[1].toDouble(), topLeft[0].toDouble() );
        }
        else
        {
          tileMatrix.topLeft.set( topLeft[0].toDouble(), topLeft[1].toDouble() );
        }
      }
      else
      {
        QgsDebugError( QStringLiteral( "Could not parse topLeft" ) );
        continue;
      }

      tileMatrix.tileWidth    = secondChildElement.firstChildElement( QStringLiteral( "TileWidth" ) ).text().toInt();
      tileMatrix.tileHeight   = secondChildElement.firstChildElement( QStringLiteral( "TileHeight" ) ).text().toInt();
      tileMatrix.matrixWidth  = secondChildElement.firstChildElement( QStringLiteral( "MatrixWidth" ) ).text().toInt();
      tileMatrix.matrixHeight = secondChildElement.firstChildElement( QStringLiteral( "MatrixHeight" ) ).text().toInt();

      // the magic number below is "standardized rendering pixel size" defined
      // in WMTS (and WMS 1.3) standard, being 0.28 pixel
      tileMatrix.tres = tileMatrix.scaleDenom * 0.00028 / metersPerUnit;

      QgsDebugMsgLevel( QStringLiteral( " %1: scale=%2 res=%3 tile=%4x%5 matrix=%6x%7 topLeft=%8" )
                        .arg( tileMatrix.identifier )
                        .arg( tileMatrix.scaleDenom ).arg( tileMatrix.tres )
                        .arg( tileMatrix.tileWidth ).arg( tileMatrix.tileHeight )
                        .arg( tileMatrix.matrixWidth ).arg( tileMatrix.matrixHeight )
                        .arg( tileMatrix.topLeft.toString() ), 2
                      );

      set.tileMatrices.insert( tileMatrix.tres, tileMatrix );
    }

    mTileMatrixSets.insert( set.identifier, set );
  }

  //
  // layers
  //

  mTileLayersSupported.clear();
  for ( QDomElement childElement = element.firstChildElement( QStringLiteral( "Layer" ) );
        !childElement.isNull();
        childElement = childElement.nextSiblingElement( QStringLiteral( "Layer" ) ) )
  {
#ifdef QGISDEBUG
    QString id = childElement.firstChildElement( QStringLiteral( "ows:Identifier" ) ).text();  // clazy:exclude=unused-non-trivial-variable
    QgsDebugMsgLevel( QStringLiteral( "Layer %1" ).arg( id ), 2 );
#endif

    QgsWmtsTileLayer tileLayer;
    tileLayer.tileMode   = WMTS;
    tileLayer.identifier = childElement.firstChildElement( QStringLiteral( "ows:Identifier" ) ).text();
    tileLayer.title      = childElement.firstChildElement( QStringLiteral( "ows:Title" ) ).text();
    tileLayer.abstract   = childElement.firstChildElement( QStringLiteral( "ows:Abstract" ) ).text();
    parseKeywords( childElement, tileLayer.keywords );

    QgsWmsBoundingBoxProperty boundingBoxProperty;

    QDomElement bbox = childElement.firstChildElement( QStringLiteral( "ows:WGS84BoundingBox" ) );
    if ( !bbox.isNull() )
    {
      QStringList ll = bbox.firstChildElement( QStringLiteral( "ows:LowerCorner" ) ).text().split( ' ' );
      QStringList ur = bbox.firstChildElement( QStringLiteral( "ows:UpperCorner" ) ).text().split( ' ' );

      if ( ll.size() == 2 && ur.size() == 2 )
      {
        boundingBoxProperty.crs = DEFAULT_LATLON_CRS;
        boundingBoxProperty.box = QgsRectangle( QgsPointXY( ll[0].toDouble(), ll[1].toDouble() ),
                                                QgsPointXY( ur[0].toDouble(), ur[1].toDouble() ) );

        tileLayer.boundingBoxes << boundingBoxProperty;
      }
    }

    for ( bbox = childElement.firstChildElement( QStringLiteral( "ows:BoundingBox" ) );
          !bbox.isNull();
          bbox = bbox.nextSiblingElement( QStringLiteral( "ows:BoundingBox" ) ) )
    {
      QStringList ll = bbox.firstChildElement( QStringLiteral( "ows:LowerCorner" ) ).text().split( ' ' );
      QStringList ur = bbox.firstChildElement( QStringLiteral( "ows:UpperCorner" ) ).text().split( ' ' );

      if ( ll.size() == 2 && ur.size() == 2 )
      {
        boundingBoxProperty.box = QgsRectangle( QgsPointXY( ll[0].toDouble(), ll[1].toDouble() ),
                                                QgsPointXY( ur[0].toDouble(), ur[1].toDouble() ) );

        if ( bbox.hasAttribute( QStringLiteral( "SRS" ) ) )
          boundingBoxProperty.crs = bbox.attribute( QStringLiteral( "SRS" ) );
        else if ( bbox.hasAttribute( QStringLiteral( "srs" ) ) )
          boundingBoxProperty.crs = bbox.attribute( QStringLiteral( "srs" ) );
        else if ( bbox.hasAttribute( QStringLiteral( "CRS" ) ) )
          boundingBoxProperty.crs = bbox.attribute( QStringLiteral( "CRS" ) );
        else if ( bbox.hasAttribute( QStringLiteral( "crs" ) ) )
          boundingBoxProperty.crs = bbox.attribute( QStringLiteral( "crs" ) );
        else
        {
          QgsDebugError( QStringLiteral( "crs of bounding box undefined" ) );
        }

        if ( !boundingBoxProperty.crs.isEmpty() )
        {
          QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( boundingBoxProperty.crs );
          if ( crs.isValid() )
          {
            boundingBoxProperty.crs = crs.authid();

            bool invert = !mParserSettings.ignoreAxisOrientation && crs.hasAxisInverted();
            if ( mParserSettings.invertAxisOrientation )
              invert = !invert;

            if ( invert )
              boundingBoxProperty.box.invert();

            tileLayer.boundingBoxes << boundingBoxProperty;
          }
        }
      }
    }

    for ( QDomElement secondChildElement = childElement.firstChildElement( QStringLiteral( "Style" ) );
          !secondChildElement.isNull();
          secondChildElement = secondChildElement.nextSiblingElement( QStringLiteral( "Style" ) ) )
    {
      QgsWmtsStyle style;
      style.identifier = secondChildElement.firstChildElement( QStringLiteral( "ows:Identifier" ) ).text();
      style.title      = secondChildElement.firstChildElement( QStringLiteral( "ows:Title" ) ).text();
      style.abstract   = secondChildElement.firstChildElement( QStringLiteral( "ows:Abstract" ) ).text();
      parseKeywords( secondChildElement, style.keywords );

      for ( QDomElement thirdChildElement = secondChildElement.firstChildElement( QStringLiteral( "ows:legendURL" ) );
            !thirdChildElement.isNull();
            thirdChildElement = thirdChildElement.nextSiblingElement( QStringLiteral( "ows:legendURL" ) ) )
      {
        QgsWmtsLegendURL legendURL;

        legendURL.format   = thirdChildElement.firstChildElement( QStringLiteral( "format" ) ).text();
        legendURL.minScale = thirdChildElement.firstChildElement( QStringLiteral( "minScale" ) ).text().toDouble();
        legendURL.maxScale = thirdChildElement.firstChildElement( QStringLiteral( "maxScale" ) ).text().toDouble();
        legendURL.href     = thirdChildElement.firstChildElement( QStringLiteral( "href" ) ).text();
        legendURL.width    = thirdChildElement.firstChildElement( QStringLiteral( "width" ) ).text().toInt();
        legendURL.height   = thirdChildElement.firstChildElement( QStringLiteral( "height" ) ).text().toInt();

        style.legendURLs << legendURL;
      }
      QDomElement thirdChildElement = secondChildElement.firstChildElement( QStringLiteral( "LegendURL" ) );
      if ( !thirdChildElement.isNull() )
      {
        QgsWmtsLegendURL legendURL;

        legendURL.format   = thirdChildElement.attribute( QStringLiteral( "format" ) );
        legendURL.minScale = thirdChildElement.attribute( QStringLiteral( "minScaleDenominator" ) ).toDouble();
        legendURL.maxScale = thirdChildElement.attribute( QStringLiteral( "maxScaleDenominator" ) ).toDouble();
        legendURL.href     = thirdChildElement.attribute( QStringLiteral( "xlink:href" ) );
        legendURL.width    = thirdChildElement.attribute( QStringLiteral( "width" ) ).toInt();
        legendURL.height   = thirdChildElement.attribute( QStringLiteral( "height" ) ).toInt();

        style.legendURLs << legendURL;
      }

      style.isDefault = secondChildElement.attribute( QStringLiteral( "isDefault" ) ) == QLatin1String( "true" );

      tileLayer.styles.insert( style.identifier, style );

      if ( style.isDefault )
        tileLayer.defaultStyle = style.identifier;
    }

    if ( tileLayer.styles.isEmpty() )
    {
      QgsWmtsStyle style;
      style.identifier = QStringLiteral( "default" );
      style.title      = QObject::tr( "Generated default style" );
      style.abstract   = QObject::tr( "Style was missing in capabilities" );
      tileLayer.styles.insert( style.identifier, style );
    }

    {
      QSet< QString > uniqueFormats;
      for ( QDomElement secondChildElement = childElement.firstChildElement( QStringLiteral( "Format" ) ); !secondChildElement.isNull(); secondChildElement = secondChildElement.nextSiblingElement( QStringLiteral( "Format" ) ) )
      {
        const QString format = secondChildElement.text();
        if ( !uniqueFormats.contains( format ) )
        {
          tileLayer.formats << format;
          uniqueFormats.insert( format );
        }
      }
    }

    for ( QDomElement secondChildElement = childElement.firstChildElement( QStringLiteral( "InfoFormat" ) ); !secondChildElement.isNull(); secondChildElement = secondChildElement.nextSiblingElement( QStringLiteral( "InfoFormat" ) ) )
    {
      QString format = secondChildElement.text();

      tileLayer.infoFormats << secondChildElement.text();

      Qgis::RasterIdentifyFormat fmt = Qgis::RasterIdentifyFormat::Undefined;

      QgsDebugMsgLevel( QStringLiteral( "format=%1" ).arg( format ), 2 );

      if ( ( format == QLatin1String( "MIME" ) ) // 1.0
           || ( format == QLatin1String( "text/plain" ) ) )
        fmt = Qgis::RasterIdentifyFormat::Text;
      else if ( format == QLatin1String( "text/html" ) )
        fmt = Qgis::RasterIdentifyFormat::Html;
      else if ( format.startsWith( QLatin1String( "GML." ) ) // 1.0
                || ( format == QLatin1String( "application/vnd.ogc.gml" ) )
                || ( format.contains( QLatin1String( "gml" ), Qt::CaseInsensitive ) )
                || ( format == QLatin1String( "application/json" ) )
                || ( format == QLatin1String( "application/geojson" ) )
                || ( format == QLatin1String( "application/geo+json" ) ) )
        fmt = Qgis::RasterIdentifyFormat::Feature;
      else
      {
        QgsDebugError( QStringLiteral( "Unsupported featureInfoUrl format: %1" ).arg( format ) );
        continue;
      }

      QgsDebugMsgLevel( QStringLiteral( "fmt=%1" ).arg( qgsEnumValueToKey( fmt ) ), 2 );
      mIdentifyFormats.insert( fmt, format );
    }

    for ( QDomElement secondChildElement = childElement.firstChildElement( QStringLiteral( "Dimension" ) ); !secondChildElement.isNull(); secondChildElement = secondChildElement.nextSiblingElement( QStringLiteral( "Dimension" ) ) )
    {
      QgsWmtsDimension dimension;

      dimension.identifier   = secondChildElement.firstChildElement( QStringLiteral( "ows:Identifier" ) ).text();
      if ( dimension.identifier.isEmpty() )
        continue;

      dimension.title        = secondChildElement.firstChildElement( QStringLiteral( "ows:Title" ) ).text();
      dimension.abstract     = secondChildElement.firstChildElement( QStringLiteral( "ows:Abstract" ) ).text();
      parseKeywords( secondChildElement, dimension.keywords );

      dimension.UOM          = secondChildElement.firstChildElement( QStringLiteral( "UOM" ) ).text();
      dimension.unitSymbol   = secondChildElement.firstChildElement( QStringLiteral( "unitSymbol" ) ).text();
      dimension.defaultValue = secondChildElement.firstChildElement( QStringLiteral( "Default" ) ).text();
      dimension.current      = secondChildElement.firstChildElement( QStringLiteral( "current" ) ).text() == QLatin1String( "true" );

      for ( QDomElement thirdChildElement = secondChildElement.firstChildElement( QStringLiteral( "Value" ) );
            !thirdChildElement.isNull();
            thirdChildElement = thirdChildElement.nextSiblingElement( QStringLiteral( "Value" ) ) )
      {
        dimension.values << thirdChildElement.text();
      }

      tileLayer.dimensions.insert( dimension.identifier, dimension );

      if ( ( dimension.title.compare( QLatin1String( "time" ), Qt::CaseInsensitive ) == 0 ||
             dimension.identifier.compare( QLatin1String( "time" ), Qt::CaseInsensitive ) == 0 )
           && !dimension.values.empty() )
      {
        // we will use temporal framework if there's multiple time dimension values, OR if a single time dimension value is itself an interval
        bool useTemporalFramework = dimension.values.size() > 1;
        if ( !useTemporalFramework )
        {
          const thread_local QRegularExpression rxPeriod( QStringLiteral( ".*/P.*" ) );
          const QRegularExpressionMatch match = rxPeriod.match( dimension.values.constFirst() );
          useTemporalFramework = match.hasMatch();
        }

        if ( useTemporalFramework )
        {
          tileLayer.timeDimensionIdentifier = dimension.identifier;
          // populate temporal information
          QDateTime minTime;
          QDateTime maxTime;
          QgsInterval defaultInterval = QgsInterval( 1, Qgis::TemporalUnit::IrregularStep );
          bool hasPeriodValue = false;
          for ( const QString &value : std::as_const( dimension.values ) )
          {
            // unfortunately there's NO standard way of specifying time values for WMTS. So we have to be flexible here...

            // check first if it's a WMST style YYYY-MM-DD/YYYY-MM-DD/Pxx format
            const thread_local QRegularExpression rxPeriod( QStringLiteral( ".*/P.*" ) );
            const QRegularExpressionMatch match = rxPeriod.match( value );
            if ( match.hasMatch() )
            {
              const QStringList valueParts = value.split( '/' );
              if ( valueParts.size() == 3 )
              {
                const QDateTime begin = QgsWmsSettings::parseWmstDateTimes( valueParts.at( 0 ) );
                const QDateTime end = QgsWmsSettings::parseWmstDateTimes( valueParts.at( 1 ) );
                const QgsTimeDuration itemResolution = QgsWmsSettings::parseWmstResolution( valueParts.at( 2 ) );

                bool maxValuesExceeded = false;
                const QList< QDateTime > dates = QgsTemporalUtils::calculateDateTimesUsingDuration( begin, end, itemResolution, maxValuesExceeded, 1000 );
                // if we have a manageable number of distinct dates, then we'll use those. If not we just use the overall range.
                // (some servers eg may have data for every minute for decades!)
                if ( !maxValuesExceeded )
                {
                  for ( const QDateTime &dt : dates )
                  {
                    tileLayer.allTimeRanges.append( QgsDateTimeRange( dt, dt ) );
                    if ( !minTime.isValid() || dt < minTime )
                      minTime = dt;
                    if ( !maxTime.isValid() || dt > maxTime )
                      maxTime = dt;
                  }
                }
                else
                {
                  tileLayer.allTimeRanges.append( QgsDateTimeRange( begin, end ) );
                  if ( !minTime.isValid() || begin < minTime )
                    minTime = begin;
                  if ( !maxTime.isValid() || end > maxTime )
                    maxTime = end;
                }

                defaultInterval = itemResolution.toInterval();
                tileLayer.timeFormat = QgsWmtsTileLayer::WmtsTimeFormat::yyyyMMddyyyyMMddPxx;
                hasPeriodValue = true;
              }
              else
              {
                QgsMessageLog::logMessage( QObject::tr( "Could not interpret TIME dimension value %1 as a time range" ).arg( value ) );
              }
              continue;
            }

            const QgsDateTimeRange range = QgsWmsSettings::parseWmtsTimeValue( value, tileLayer.timeFormat );
            if ( !range.isEmpty() )
            {
              tileLayer.allTimeRanges.append( range );
              if ( !minTime.isValid() || range.begin() < minTime )
                minTime = range.begin();
              if ( !maxTime.isValid() || range.end() > maxTime )
                maxTime = range.end();
            }
            else
            {
              QgsMessageLog::logMessage( QObject::tr( "Could not interpret TIME dimension value %1 as a time range" ).arg( value ) );
            }
          }
          if ( minTime.isValid() )
            tileLayer.temporalExtent = QgsDateTimeRange( minTime, maxTime );
          tileLayer.temporalInterval = defaultInterval;
          tileLayer.defaultTimeDimensionValue = dimension.defaultValue;

          if ( !hasPeriodValue )
          {
            // when the wmts isn't exposing a period value for the dimension, then we have to be careful to only ever pass exact time matches for the actual values
            // present on the service
            tileLayer.temporalCapabilityFlags |= Qgis::RasterTemporalCapabilityFlag::RequestedTimesMustExactlyMatchAllAvailableTemporalRanges;
          }
        }
      }
    }

    for ( QDomElement secondChildElement = childElement.firstChildElement( QStringLiteral( "TileMatrixSetLink" ) ); !secondChildElement.isNull(); secondChildElement = secondChildElement.nextSiblingElement( QStringLiteral( "TileMatrixSetLink" ) ) )
    {
      QgsWmtsTileMatrixSetLink setLink;

      setLink.tileMatrixSet = secondChildElement.firstChildElement( QStringLiteral( "TileMatrixSet" ) ).text();

      if ( !mTileMatrixSets.contains( setLink.tileMatrixSet ) )
      {
        QgsDebugError( QStringLiteral( "  TileMatrixSet %1 not found." ).arg( setLink.tileMatrixSet ) );
        continue;
      }

      const QgsWmtsTileMatrixSet &tms = mTileMatrixSets[ setLink.tileMatrixSet ];

      for ( QDomElement thirdChildElement = secondChildElement.firstChildElement( QStringLiteral( "TileMatrixSetLimits" ) ); !thirdChildElement.isNull(); thirdChildElement = thirdChildElement.nextSiblingElement( QStringLiteral( "TileMatrixSetLimits" ) ) )
      {
        for ( QDomElement fourthChildElement = thirdChildElement.firstChildElement( QStringLiteral( "TileMatrixLimits" ) ); !fourthChildElement.isNull(); fourthChildElement = fourthChildElement.nextSiblingElement( QStringLiteral( "TileMatrixLimits" ) ) )
        {
          QgsWmtsTileMatrixLimits limit;

          QString id = fourthChildElement.firstChildElement( QStringLiteral( "TileMatrix" ) ).text();

          bool isValid = false;
          int matrixWidth = -1, matrixHeight = -1;
          for ( const QgsWmtsTileMatrix &m : tms.tileMatrices )
          {
            isValid = m.identifier == id;
            if ( isValid )
            {
              matrixWidth = m.matrixWidth;
              matrixHeight = m.matrixHeight;
              break;
            }
          }

          if ( isValid )
          {
            limit.minTileRow = fourthChildElement.firstChildElement( QStringLiteral( "MinTileRow" ) ).text().toInt();
            limit.maxTileRow = fourthChildElement.firstChildElement( QStringLiteral( "MaxTileRow" ) ).text().toInt();
            limit.minTileCol = fourthChildElement.firstChildElement( QStringLiteral( "MinTileCol" ) ).text().toInt();
            limit.maxTileCol = fourthChildElement.firstChildElement( QStringLiteral( "MaxTileCol" ) ).text().toInt();

            isValid =
              limit.minTileCol >= 0 && limit.minTileCol < matrixWidth &&
              limit.maxTileCol >= 0 && limit.maxTileCol < matrixWidth &&
              limit.minTileCol <= limit.maxTileCol &&
              limit.minTileRow >= 0 && limit.minTileRow < matrixHeight &&
              limit.maxTileRow >= 0 && limit.maxTileRow < matrixHeight &&
              limit.minTileRow <= limit.maxTileRow;
          }
          else
          {
            QgsDebugError( QStringLiteral( "   TileMatrix id:%1 not found." ).arg( id ) );
          }

          QgsDebugMsgLevel( QStringLiteral( "   TileMatrixLimit id:%1 row:%2-%3 col:%4-%5 matrix:%6x%7 %8" )
                            .arg( id )
                            .arg( limit.minTileRow ).arg( limit.maxTileRow )
                            .arg( limit.minTileCol ).arg( limit.maxTileCol )
                            .arg( matrixWidth ).arg( matrixHeight )
                            .arg( isValid ? "valid" : "INVALID" ), 2
                          );

          if ( isValid )
          {
            setLink.limits.insert( id, limit );
          }
        }
      }

      tileLayer.setLinks.insert( setLink.tileMatrixSet, setLink );
    }

    for ( QDomElement secondChildElement = childElement.firstChildElement( QStringLiteral( "ResourceURL" ) ); !secondChildElement.isNull(); secondChildElement = secondChildElement.nextSiblingElement( QStringLiteral( "ResourceURL" ) ) )
    {
      QString format       = nodeAttribute( secondChildElement, QStringLiteral( "format" ) );
      QString resourceType = nodeAttribute( secondChildElement, QStringLiteral( "resourceType" ) );
      QString tmpl         = nodeAttribute( secondChildElement, QStringLiteral( "template" ) );

      if ( format.isEmpty() || resourceType.isEmpty() || tmpl.isEmpty() )
      {
        QgsDebugMsgLevel( QStringLiteral( "SKIPPING ResourceURL format=%1 resourceType=%2 template=%3" )
                          .arg( format,
                                resourceType,
                                tmpl ), 2 );
        continue;
      }

      if ( resourceType == QLatin1String( "tile" ) )
      {
        tileLayer.getTileURLs.insert( format, tmpl );
      }
      else if ( resourceType == QLatin1String( "FeatureInfo" ) )
      {
        tileLayer.getFeatureInfoURLs.insert( format, tmpl );

        Qgis::RasterIdentifyFormat fmt = Qgis::RasterIdentifyFormat::Undefined;

        QgsDebugMsgLevel( QStringLiteral( "format=%1" ).arg( format ), 2 );

        if ( ( format == QLatin1String( "MIME" ) ) // 1.0
             || ( format == QLatin1String( "text/plain" ) ) )
          fmt = Qgis::RasterIdentifyFormat::Text;
        else if ( format == QLatin1String( "text/html" ) )
          fmt = Qgis::RasterIdentifyFormat::Html;
        else if ( format.startsWith( QLatin1String( "GML." ) )  // 1.0
                  || ( format == QLatin1String( "application/vnd.ogc.gml" ) )
                  || ( format.contains( QLatin1String( "gml" ), Qt::CaseInsensitive ) )
                  || ( format == QLatin1String( "application/json" ) )
                  || ( format == QLatin1String( "application/geojson" ) )
                  || ( format == QLatin1String( "application/geo+json" ) ) )
          fmt = Qgis::RasterIdentifyFormat::Feature;
        else
        {
          QgsDebugError( QStringLiteral( "Unsupported featureInfoUrl format: %1" ).arg( format ) );
          continue;
        }

        QgsDebugMsgLevel( QStringLiteral( "fmt=%1" ).arg( qgsEnumValueToKey( fmt ) ), 2 );
        mIdentifyFormats.insert( fmt, format );
      }
      else
      {
        QgsDebugError( QStringLiteral( "UNEXPECTED resourceType in ResourcURL format=%1 resourceType=%2 template=%3" )
                       .arg( format,
                             resourceType,
                             tmpl ) );
      }
    }

    QgsDebugMsgLevel( QStringLiteral( "add layer %1" ).arg( id ), 2 );
    mTileLayersSupported << tileLayer;
  }

  //
  // themes
  //
  mTileThemes.clear();
  for ( QDomElement childElement = element.firstChildElement( QStringLiteral( "Themes" ) ).firstChildElement( QStringLiteral( "Theme" ) );
        !childElement.isNull();
        childElement = childElement.nextSiblingElement( QStringLiteral( "Theme" ) ) )
  {
    mTileThemes << QgsWmtsTheme();
    parseTheme( childElement, mTileThemes.back() );
  }

  // make sure that all layers have a bounding box
  for ( QList<QgsWmtsTileLayer>::iterator it = mTileLayersSupported.begin(); it != mTileLayersSupported.end(); ++it )
  {
    QgsWmtsTileLayer &tileLayer = *it;

    if ( tileLayer.boundingBoxes.isEmpty() )
    {
      if ( !detectTileLayerBoundingBox( tileLayer ) )
      {
        QgsDebugError( "failed to detect bounding box for " + tileLayer.identifier + " - using extent of the whole world" );

        QgsWmsBoundingBoxProperty boundingBoxProperty;
        boundingBoxProperty.crs = DEFAULT_LATLON_CRS;
        boundingBoxProperty.box = QgsRectangle( -180.0, -90.0, 180.0, 90.0 );
        tileLayer.boundingBoxes << boundingBoxProperty;
      }
    }
  }
}


void QgsWmsCapabilities::parseKeywords( const QDomNode &node, QStringList &keywords )
{
  keywords.clear();

  for ( QDomElement nodeElement = node.firstChildElement( QStringLiteral( "ows:Keywords" ) ).firstChildElement( QStringLiteral( "ows:Keyword" ) );
        !nodeElement.isNull();
        nodeElement = nodeElement.nextSiblingElement( QStringLiteral( "ows:Keyword" ) ) )
  {
    keywords << nodeElement.text();
  }
}

void QgsWmsCapabilities::parseTheme( const QDomElement &element, QgsWmtsTheme &theme )
{
  theme.identifier = element.firstChildElement( QStringLiteral( "ows:Identifier" ) ).text();
  theme.title      = element.firstChildElement( QStringLiteral( "ows:Title" ) ).text();
  theme.abstract   = element.firstChildElement( QStringLiteral( "ows:Abstract" ) ).text();
  parseKeywords( element, theme.keywords );

  QDomElement themeElement = element.firstChildElement( QStringLiteral( "ows:Theme" ) );
  if ( !themeElement.isNull() )
  {
    theme.subTheme = new QgsWmtsTheme;
    parseTheme( themeElement, *theme.subTheme );
  }
  else
  {
    theme.subTheme = nullptr;
  }

  theme.layerRefs.clear();
  for ( QDomElement childElement = element.firstChildElement( QStringLiteral( "ows:LayerRef" ) );
        !childElement.isNull();
        childElement = childElement.nextSiblingElement( QStringLiteral( "ows:LayerRef" ) ) )
  {
    theme.layerRefs << childElement.text();
  }
}

QString QgsWmsCapabilities::nodeAttribute( const QDomElement &element, const QString &name, const QString &defValue )
{
  if ( element.hasAttribute( name ) )
    return element.attribute( name );

  QDomNamedNodeMap map( element.attributes() );
  for ( int i = 0; i < map.size(); i++ )
  {
    QDomAttr attr( map.item( i ).toElement().toAttr() );
    if ( attr.name().compare( name, Qt::CaseInsensitive ) == 0 )
      return attr.value();
  }

  return defValue;
}


bool QgsWmsCapabilities::detectTileLayerBoundingBox( QgsWmtsTileLayer &tileLayer )
{
  if ( tileLayer.setLinks.isEmpty() )
    return false;

  // add valid bounding boxes for all linked tile matrix set
  const QList<QgsWmtsTileMatrixSetLink> links = tileLayer.setLinks.values();
  for ( QgsWmtsTileMatrixSetLink setLink : links )
  {
    QHash<QString, QgsWmtsTileMatrixSet>::const_iterator tmsIt = mTileMatrixSets.constFind( setLink.tileMatrixSet );
    if ( tmsIt == mTileMatrixSets.constEnd() )
      continue;

    QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( tmsIt->crs );
    if ( !crs.isValid() )
      continue;

    // take most coarse tile matrix ...
    QMap<double, QgsWmtsTileMatrix>::const_iterator tmIt = --tmsIt->tileMatrices.constEnd();
    if ( tmIt == tmsIt->tileMatrices.constEnd() )
      continue;

    const QgsWmtsTileMatrix &tm = *tmIt;
    double metersPerUnit = QgsUnitTypes::fromUnitToUnitFactor( crs.mapUnits(), Qgis::DistanceUnit::Meters );
    // the magic number below is "standardized rendering pixel size" defined
    // in WMTS (and WMS 1.3) standard, being 0.28 pixel
    double res = tm.scaleDenom * 0.00028 / metersPerUnit;
    QgsPointXY bottomRight( tm.topLeft.x() + res * tm.tileWidth * tm.matrixWidth,
                            tm.topLeft.y() - res * tm.tileHeight * tm.matrixHeight );

    QgsDebugMsgLevel( QStringLiteral( "detecting WMTS layer bounding box: tileset %1 matrix %2 crs %3 res %4" )
                      .arg( tmsIt->identifier, tm.identifier, tmsIt->crs ).arg( res ), 2 );

    QgsRectangle extent( tm.topLeft, bottomRight );
    extent.normalize();

    QgsWmsBoundingBoxProperty boundingBoxProperty;
    boundingBoxProperty.box = extent;
    boundingBoxProperty.crs = crs.authid();
    tileLayer.boundingBoxes << boundingBoxProperty;
  }

  return !tileLayer.boundingBoxes.isEmpty();
}


bool QgsWmsCapabilities::shouldInvertAxisOrientation( const QString &ogcCrs )
{
  //according to the WMS spec for 1.3, some CRS have inverted axis
  bool changeXY = false;
  if ( !mParserSettings.ignoreAxisOrientation && ( mCapabilities.version == QLatin1String( "1.3.0" ) || mCapabilities.version == QLatin1String( "1.3" ) ) )
  {
    //have we already checked this crs?
    if ( mCrsInvertAxis.contains( ogcCrs ) )
    {
      //if so, return previous result to save time
      return mCrsInvertAxis[ ogcCrs ];
    }

    //create CRS from string
    QgsCoordinateReferenceSystem srs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( ogcCrs );
    if ( srs.isValid() && srs.hasAxisInverted() )
    {
      changeXY = true;
    }

    //cache result to speed up future checks
    mCrsInvertAxis[ ogcCrs ] = changeXY;
  }

  if ( mParserSettings.invertAxisOrientation )
    changeXY = !changeXY;

  return changeXY;
}

int QgsWmsCapabilities::identifyCapabilities() const
{
  int capability = QgsRasterInterface::NoCapabilities;

  for ( auto it = mIdentifyFormats.constBegin(); it != mIdentifyFormats.constEnd(); ++it )
  {
    capability |= QgsRasterDataProvider::identifyFormatToCapability( it.key() );
  }

  return capability;
}



// -----------------

QgsWmsCapabilitiesDownload::QgsWmsCapabilitiesDownload( bool forceRefresh, QObject *parent )
  : QObject( parent )
  , mIsAborted( false )
  , mForceRefresh( forceRefresh )
{
}

QgsWmsCapabilitiesDownload::QgsWmsCapabilitiesDownload( const QString &baseUrl, const QgsWmsAuthorization &auth, bool forceRefresh, QObject *parent )
  : QObject( parent )
  , mBaseUrl( baseUrl )
  , mAuth( auth )
  , mIsAborted( false )
  , mForceRefresh( forceRefresh )
{
}

QgsWmsCapabilitiesDownload::~QgsWmsCapabilitiesDownload()
{
  abort();
}

bool QgsWmsCapabilitiesDownload::forceRefresh()
{
  return mForceRefresh;
}

void QgsWmsCapabilitiesDownload::setForceRefresh( bool forceRefresh )
{
  mForceRefresh = forceRefresh;
}

bool QgsWmsCapabilitiesDownload::downloadCapabilities( const QString &baseUrl, const QgsWmsAuthorization &auth )
{
  mBaseUrl = baseUrl;
  mAuth = auth;
  return downloadCapabilities();
}

bool QgsWmsCapabilitiesDownload::downloadCapabilities()
{
  QgsDebugMsgLevel( QStringLiteral( "entering: forceRefresh=%1" ).arg( mForceRefresh ), 2 );
  abort(); // cancel previous
  mIsAborted = false;

  QString url = mBaseUrl;
  if ( !QgsWmsProvider::isUrlForWMTS( url ) )
  {
    url += QLatin1String( "SERVICE=WMS&REQUEST=GetCapabilities" );
  }
  QgsDebugMsgLevel( QStringLiteral( "url = %1" ).arg( url ), 2 );

  mError.clear();

  QNetworkRequest request( url );
  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsWmsCapabilitiesDownload" ) );
  if ( !mAuth.setAuthorization( request ) )
  {
    mError = tr( "Download of capabilities failed:\nnetwork request update failed for authentication config" );
    QgsMessageLog::logMessage( mError, tr( "WMS" ) );
    return false;
  }
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, mForceRefresh ? QNetworkRequest::AlwaysNetwork : QNetworkRequest::PreferCache );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

  mCapabilitiesReply = QgsNetworkAccessManager::instance()->get( request );
  if ( !mAuth.setAuthorizationReply( mCapabilitiesReply ) )
  {
    mCapabilitiesReply->deleteLater();
    mCapabilitiesReply = nullptr;
    mError = tr( "Download of capabilities failed:\nnetwork reply update failed for authentication config" );
    QgsMessageLog::logMessage( mError, tr( "WMS" ) );
    return false;
  }
  connect( mCapabilitiesReply, &QNetworkReply::finished, this, &QgsWmsCapabilitiesDownload::capabilitiesReplyFinished, Qt::DirectConnection );
  connect( mCapabilitiesReply, &QNetworkReply::downloadProgress, this, &QgsWmsCapabilitiesDownload::capabilitiesReplyProgress, Qt::DirectConnection );

  QEventLoop loop;
  connect( this, &QgsWmsCapabilitiesDownload::downloadFinished, &loop, &QEventLoop::quit );
  loop.exec( QEventLoop::ExcludeUserInputEvents );

  return mError.isEmpty();
}

void QgsWmsCapabilitiesDownload::abort()
{
  mIsAborted = true;
  if ( mCapabilitiesReply )
  {
    mCapabilitiesReply->deleteLater();
    mCapabilitiesReply = nullptr;
  }
}

void QgsWmsCapabilitiesDownload::capabilitiesReplyProgress( qint64 bytesReceived, qint64 bytesTotal )
{
  QString message = tr( "%1 of %2 bytes of capabilities downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? QStringLiteral( "unknown number of" ) : QString::number( bytesTotal ) );
  QgsDebugMsgLevel( message, 2 );
  emit statusChanged( message );
}

void QgsWmsCapabilitiesDownload::capabilitiesReplyFinished()
{
  if ( !mIsAborted && mCapabilitiesReply )
  {
    if ( mCapabilitiesReply->error() == QNetworkReply::NoError )
    {
      QgsDebugMsgLevel( QStringLiteral( "reply OK" ), 2 );
      QVariant redirect = mCapabilitiesReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
      if ( !QgsVariantUtils::isNull( redirect ) )
      {
        emit statusChanged( tr( "Capabilities request redirected." ) );

        const QUrl &toUrl = redirect.toUrl();
        mCapabilitiesReply->request();
        if ( toUrl == mCapabilitiesReply->url() )
        {
          mError = tr( "Redirect loop detected:\n%1" ).arg( toUrl.toString() );
          QgsMessageLog::logMessage( mError, tr( "WMS" ) );
          mHttpCapabilitiesResponse.clear();
        }
        else
        {
          QNetworkRequest request( toUrl );
          QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsWmsCapabilitiesDownload" ) );
          if ( !mAuth.setAuthorization( request ) )
          {
            mHttpCapabilitiesResponse.clear();
            mError = tr( "Download of capabilities failed:\nnetwork request update failed for authentication config" );
            QgsMessageLog::logMessage( mError, tr( "WMS" ) );
            emit downloadFinished();
            return;
          }
          request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, mForceRefresh ? QNetworkRequest::AlwaysNetwork : QNetworkRequest::PreferCache );
          request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

          mCapabilitiesReply->deleteLater();
          mCapabilitiesReply = nullptr;

          QgsDebugMsgLevel( QStringLiteral( "redirected getcapabilities:\n%1 forceRefresh=%2" ).arg( redirect.toString() ).arg( mForceRefresh ), 2 );
          mCapabilitiesReply = QgsNetworkAccessManager::instance()->get( request );

          if ( !mAuth.setAuthorizationReply( mCapabilitiesReply ) )
          {
            mHttpCapabilitiesResponse.clear();
            mCapabilitiesReply->deleteLater();
            mCapabilitiesReply = nullptr;
            mError = tr( "Download of capabilities failed:\nnetwork reply update failed for authentication config" );
            QgsMessageLog::logMessage( mError, tr( "WMS" ) );
            emit downloadFinished();
            return;
          }

          connect( mCapabilitiesReply, &QNetworkReply::finished, this, &QgsWmsCapabilitiesDownload::capabilitiesReplyFinished, Qt::DirectConnection );
          connect( mCapabilitiesReply, &QNetworkReply::downloadProgress, this, &QgsWmsCapabilitiesDownload::capabilitiesReplyProgress, Qt::DirectConnection );
          return;
        }
      }
      else
      {
        const QgsNetworkAccessManager *nam = QgsNetworkAccessManager::instance();

        if ( nam->cache() )
        {
          QNetworkCacheMetaData cmd = nam->cache()->metaData( mCapabilitiesReply->request().url() );

          QNetworkCacheMetaData::RawHeaderList hl;
          const auto constRawHeaders = cmd.rawHeaders();
          for ( const QNetworkCacheMetaData::RawHeader &h : constRawHeaders )
          {
            if ( h.first != "Cache-Control" )
              hl.append( h );
          }
          cmd.setRawHeaders( hl );

          QgsDebugMsgLevel( QStringLiteral( "expirationDate:%1" ).arg( cmd.expirationDate().toString() ), 2 );
          if ( cmd.expirationDate().isNull() )
          {
            QgsSettings s;
            cmd.setExpirationDate( QDateTime::currentDateTime().addSecs( s.value( QStringLiteral( "qgis/defaultCapabilitiesExpiry" ), "24" ).toInt() * 60 * 60 ) );
          }

          nam->cache()->updateMetaData( cmd );
        }
        else
        {
          QgsDebugMsgLevel( QStringLiteral( "No cache for capabilities!" ), 2 );
        }

#ifdef QGISDEBUG
        bool fromCache = mCapabilitiesReply->attribute( QNetworkRequest::SourceIsFromCacheAttribute ).toBool();
        QgsDebugMsgLevel( QStringLiteral( "Capabilities reply was cached:\n%1" ).arg( fromCache ), 2 );
#endif

        mHttpCapabilitiesResponse = mCapabilitiesReply->readAll();

        if ( mHttpCapabilitiesResponse.isEmpty() )
        {
          mError = tr( "Capabilities are empty:\n%1" ).arg( mCapabilitiesReply->errorString() );
        }
      }
    }
    else
    {
      const QString contentType = mCapabilitiesReply->header( QNetworkRequest::ContentTypeHeader ).toString();

      QString errorMessage;
      if ( contentType.startsWith( QLatin1String( "text/plain" ) ) )
        errorMessage = mCapabilitiesReply->readAll();
      else
        errorMessage = mCapabilitiesReply->attribute( QNetworkRequest::HttpReasonPhraseAttribute ).toString();

      if ( errorMessage.isEmpty() )
        errorMessage = mCapabilitiesReply->errorString();

      mError = tr( "Download of capabilities failed:\n%1" ).arg( errorMessage );
      QgsMessageLog::logMessage( mError, tr( "WMS" ) );
      mHttpCapabilitiesResponse.clear();
    }
  }

  if ( mCapabilitiesReply )
  {
    mCapabilitiesReply->deleteLater();
    mCapabilitiesReply = nullptr;
  }

  emit downloadFinished();
}

QRectF QgsWmtsTileMatrix::tileRect( int col, int row ) const
{
  double twMap = tileWidth * tres;
  double thMap = tileHeight * tres;
  return QRectF( topLeft.x() + col * twMap, topLeft.y() - ( row + 1 ) * thMap, twMap, thMap );
}

QgsRectangle QgsWmtsTileMatrix::tileBBox( int col, int row ) const
{
  double twMap = tileWidth * tres;
  double thMap = tileHeight * tres;
  return QgsRectangle(
           topLeft.x() +         col * twMap,
           topLeft.y() - ( row + 1 ) * thMap,
           topLeft.x() + ( col + 1 ) * twMap,
           topLeft.y() -         row * thMap );
}

void QgsWmtsTileMatrix::viewExtentIntersection( const QgsRectangle &viewExtent, const QgsWmtsTileMatrixLimits *tml, int &col0, int &row0, int &col1, int &row1 ) const
{
  double twMap = tileWidth * tres;
  double thMap = tileHeight * tres;

  int minTileCol = 0;
  int maxTileCol = matrixWidth - 1;
  int minTileRow = 0;
  int maxTileRow = matrixHeight - 1;

  if ( tml )
  {
    minTileCol = tml->minTileCol;
    maxTileCol = tml->maxTileCol;
    minTileRow = tml->minTileRow;
    maxTileRow = tml->maxTileRow;
  }

  col0 = std::clamp( ( int ) std::floor( ( viewExtent.xMinimum() - topLeft.x() ) / twMap ),  minTileCol, maxTileCol );
  row0 = std::clamp( ( int ) std::floor( ( topLeft.y() - viewExtent.yMaximum() ) / thMap ),  minTileRow, maxTileRow );
  col1 = std::clamp( ( int ) std::floor( ( viewExtent.xMaximum() - topLeft.x() ) / twMap ),  minTileCol, maxTileCol );
  row1 = std::clamp( ( int ) std::floor( ( topLeft.y() - viewExtent.yMinimum() ) / thMap ),  minTileRow, maxTileRow );
}

const QgsWmtsTileMatrix *QgsWmtsTileMatrixSet::findNearestResolution( double vres ) const
{
  QMap<double, QgsWmtsTileMatrix>::const_iterator prev, it = tileMatrices.constBegin();
  while ( it != tileMatrices.constEnd() && it.key() < vres )
  {
    prev = it;
    ++it;
  }

  if ( it == tileMatrices.constEnd() ||
       ( it != tileMatrices.constBegin() && vres - prev.key() < it.key() - vres ) )
  {
    it = prev;
  }

  return &it.value();
}

const QgsWmtsTileMatrix *QgsWmtsTileMatrixSet::findOtherResolution( double tres, int offset ) const
{
  QMap<double, QgsWmtsTileMatrix>::const_iterator it = tileMatrices.constFind( tres );
  if ( it == tileMatrices.constEnd() )
    return nullptr;
  while ( true )
  {
    if ( offset > 0 )
    {
      ++it;
      --offset;
    }
    else if ( offset < 0 )
    {
      if ( it == tileMatrices.constBegin() )
        return nullptr;
      --it;
      ++offset;
    }
    else
      break;

    if ( it == tileMatrices.constEnd() )
      return nullptr;
  }
  return &it.value();
}
