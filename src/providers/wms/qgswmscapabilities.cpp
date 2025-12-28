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

#include "qgis.h"
#include "qgscoordinatetransform.h"
#include "qgsdatasourceuri.h"
#include "qgsexception.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssetrequestinitiator_p.h"
#include "qgssettings.h"
#include "qgstemporalutils.h"
#include "qgsunittypes.h"
#include "qgswmsprovider.h"

#include <QDir>
#include <QFile>
#include <QNetworkCacheMetaData>
#include <QRegularExpression>
#include <QUrlQuery>

#include "moc_qgswmscapabilities.cpp"

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
  mXyz = false; // assume WMS / WMTS

  if ( uri.hasParam( u"interpretation"_s ) )
  {
    mInterpretation = uri.param( u"interpretation"_s );
  }

  mTiled = false;
  if ( uri.param( u"type"_s ) == "xyz"_L1 || uri.param( u"type"_s ) == "mbtiles"_L1 )
  {
    // for XYZ tiles most of the things do not apply
    mTiled = true;
    mXyz = true;
    mTileDimensionValues.clear();
    mTileMatrixSetId = u"tms0"_s;
    mMaxWidth = 0;
    mMaxHeight = 0;
    mHttpUri = uri.param( u"url"_s );

    // automatically replace outdated references to http OpenStreetMap tiles with correct https URI
    const thread_local QRegularExpression sRegExOSMTiles( u"^https?://(?:[a-z]\\.)?tile\\.openstreetmap\\.org"_s );
    const QRegularExpressionMatch match = sRegExOSMTiles.match( mHttpUri );
    if ( match.hasMatch() )
    {
      mHttpUri = u"https://tile.openstreetmap.org"_s + mHttpUri.mid( match.captured( 0 ).length() );
    }

    mBaseUrl = mHttpUri;

    mIgnoreGetMapUrl = false;
    mIgnoreGetFeatureInfoUrl = false;
    mSmoothPixmapTransform = mInterpretation.isEmpty();
    mDpiMode = DpiNone;                         // does not matter what we set here
    mActiveSubLayers = QStringList( u"xyz"_s ); // just a placeholder to have one sub-layer
    mActiveSubStyles = QStringList( u"xyz"_s ); // just a placeholder to have one sub-style
    mActiveSubLayerVisibility.clear();
    mFeatureCount = 0;
    mImageMimeType.clear();
    mCrsId = u"EPSG:3857"_s;
    mEnableContextualLegend = false;

    mIsMBTiles = uri.param( u"type"_s ) == "mbtiles"_L1;

    return true;
  }
  else if ( uri.param( u"type"_s ) == "wmst"_L1 )
  {
    mIsTemporal = true;
    mTemporalExtent = uri.param( u"timeDimensionExtent"_s );
    mTimeDimensionExtent = parseTemporalExtent( mTemporalExtent );

    if ( !mTimeDimensionExtent.datesResolutionList.isEmpty() && !mTimeDimensionExtent.datesResolutionList.constFirst().dates.dateTimes.empty() )
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
        const QList<QDateTime> dates = QgsTemporalUtils::calculateDateTimesUsingDuration( begin, end, extent.resolution, maxValuesExceeded, 1000 );
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

    if ( !uri.param( u"referenceTimeDimensionExtent"_s ).isEmpty() )
    {
      QString referenceExtent = uri.param( u"referenceTimeDimensionExtent"_s );

      mReferenceTimeDimensionExtent = parseTemporalExtent( referenceExtent );

      if ( mReferenceTimeDimensionExtent.datesResolutionList.constFirst().dates.dateTimes.size() > 0 )
      {
        QDateTime begin = mReferenceTimeDimensionExtent.datesResolutionList.constFirst().dates.dateTimes.first();
        QDateTime end = mReferenceTimeDimensionExtent.datesResolutionList.constLast().dates.dateTimes.last();

        mFixedReferenceRange = QgsDateTimeRange( begin, end );
      }
      else
        mFixedReferenceRange = QgsDateTimeRange();

      mIsBiTemporal = true;
    }
  }
  else if ( uri.param( u"type"_s ) == "wmts"_L1 )
  {
    mTiled = true;
  }

  mTileDimensionValues.clear();

  mHttpUri = uri.param( u"url"_s );
  mBaseUrl = QgsWmsProvider::prepareUri( mHttpUri ); // must set here, setImageCrs is using that
  QgsDebugMsgLevel( "mBaseUrl = " + mBaseUrl, 2 );

  mIgnoreGetMapUrl = uri.hasParam( u"IgnoreGetMapUrl"_s );
  mIgnoreGetFeatureInfoUrl = uri.hasParam( u"IgnoreGetFeatureInfoUrl"_s );
  mIgnoreReportedLayerExtents = uri.hasParam( u"IgnoreReportedLayerExtents"_s );
  mParserSettings.ignoreAxisOrientation = uri.hasParam( u"IgnoreAxisOrientation"_s ); // must be before parsing!
  mParserSettings.invertAxisOrientation = uri.hasParam( u"InvertAxisOrientation"_s ); // must be before parsing!
  mSmoothPixmapTransform = uri.hasParam( u"SmoothPixmapTransform"_s );

  mDpiMode = uri.hasParam( u"dpiMode"_s ) ? static_cast<QgsWmsDpiMode>( uri.param( u"dpiMode"_s ).toInt() ) : DpiAll;

  mActiveSubLayers = uri.params( u"layers"_s );
  mActiveSubStyles = uri.params( u"styles"_s );
  QgsDebugMsgLevel( "Entering: layers:" + mActiveSubLayers.join( ", " ) + ", styles:" + mActiveSubStyles.join( ", " ), 2 );

  //opacities
  if ( uri.hasParam( u"opacities"_s ) )
  {
    mOpacities.clear();
    const QStringList opacities = uri.params( u"opacities"_s );
    for ( const QString &opacity : opacities )
    {
      bool ok = false;
      ( void ) opacity.toInt( &ok );
      if ( ok )
      {
        mOpacities.append( opacity );
      }
      else
      {
        mOpacities.append( u"255"_s );
      }
    }
  }

  if ( uri.hasParam( u"filter"_s ) )
  {
    mFilter = uri.param( u"filter"_s );
  }

  mImageMimeType = uri.param( u"format"_s );
  QgsDebugMsgLevel( "Setting image encoding to " + mImageMimeType + '.', 2 );

  mMaxWidth = 0;
  mMaxHeight = 0;
  if ( uri.hasParam( u"maxWidth"_s ) && uri.hasParam( u"maxHeight"_s ) )
  {
    mMaxWidth = uri.param( u"maxWidth"_s ).toInt();
    mMaxHeight = uri.param( u"maxHeight"_s ).toInt();
  }

  mStepWidth = 2000;
  mStepHeight = 2000;
  if ( uri.hasParam( u"stepWidth"_s ) && uri.hasParam( u"stepHeight"_s ) )
  {
    mStepWidth = uri.param( u"stepWidth"_s ).toInt();
    mStepHeight = uri.param( u"stepHeight"_s ).toInt();
  }

  if ( uri.hasParam( u"tileMatrixSet"_s ) )
  {
    mTiled = true;
    // tileMatrixSet may be empty if URI was converted from < 1.9 project file URI
    // in that case it means that the source is WMS-C
    mTileMatrixSetId = uri.param( u"tileMatrixSet"_s );
    mTilePixelRatio = uri.hasParam( u"tilePixelRatio"_s ) ? static_cast<Qgis::TilePixelRatio>( uri.param( u"tilePixelRatio"_s ).toInt() ) : Qgis::TilePixelRatio::Undefined;
  }

  if ( uri.hasParam( u"tileDimensions"_s ) )
  {
    mTiled = true;
    const auto tileDimensions = uri.param( u"tileDimensions"_s ).split( ';' );
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
        QgsDebugMsgLevel( u"skipped dimension %1"_s.arg( param ), 2 );
      }
    }
  }

  mCrsId = uri.param( u"crs"_s );

  mEnableContextualLegend = uri.param( u"contextualWMSLegend"_s ).toInt();
  QgsDebugMsgLevel( u"Contextual legend: %1"_s.arg( mEnableContextualLegend ), 2 );

  mFeatureCount = uri.param( u"featureCount"_s ).toInt(); // default to 0

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
  {
    if ( item.size() == 4 )
      return QDateTime::fromString( item, u"yyyy"_s );
    else
      return QDateTime::fromString( item, u"yyyy-MM-dd"_s );
  }
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
  const thread_local QRegularExpression rxYYYYMMDD( u"^\\s*(\\d{4})(\\d{2})(\\d{2})\\s*$"_s );
  QRegularExpressionMatch match = rxYYYYMMDD.match( value );
  if ( match.hasMatch() )
  {
    const QDate date( match.captured( 1 ).toInt(), match.captured( 2 ).toInt(), match.captured( 3 ).toInt() );
    format = QgsWmtsTileLayer::WmtsTimeFormat::yyyyMMdd;

    return QgsDateTimeRange( date.startOfDay(), date.endOfDay() );
  }

  // YYYY-MM-DD format, eg
  // 2021-01-01
  const thread_local QRegularExpression rxYYYY_MM_DD( u"^\\s*(\\d{4})-(\\d{2})-(\\d{2})\\s*$"_s );
  match = rxYYYY_MM_DD.match( value );
  if ( match.hasMatch() )
  {
    const QDate date( match.captured( 1 ).toInt(), match.captured( 2 ).toInt(), match.captured( 3 ).toInt() );
    format = QgsWmtsTileLayer::WmtsTimeFormat::yyyy_MM_dd;

    return QgsDateTimeRange( date.startOfDay(), date.endOfDay() );
  }

  // YYYY format, eg
  // 2021
  const thread_local QRegularExpression rxYYYY( u"^\\s*(\\d{4})\\s*$"_s );
  match = rxYYYY.match( value );
  if ( match.hasMatch() )
  {
    const QDate startDate( match.captured( 1 ).toInt(), 1, 1 );
    const QDate endDate( match.captured( 1 ).toInt(), 12, 31 );
    format = QgsWmtsTileLayer::WmtsTimeFormat::yyyy;
    return QgsDateTimeRange( startDate.startOfDay(), endDate.endOfDay() );
  }

  // YYYY-MM-DDTHH:mm:ss.SSSZ
  const thread_local QRegularExpression rxYYYYMMDDHHmmssSSSz( u"^\\s*(\\d{4})-(\\d{2})-(\\d{2})T(\\d{2}):(\\d{2}):(\\d{2})Z\\s*$"_s );
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


QgsWmsCapabilities::QgsWmsCapabilities( const QgsCoordinateTransformContext &coordinateTransformContext, const QString &baseUrl )
  : mCoordinateTransformContext( coordinateTransformContext ), mBaseUrl( baseUrl )
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
      mErrorFormat = u"text/plain"_s;
      mError = QObject::tr( "empty capabilities document" );
    }
    QgsDebugError( u"response is empty"_s );
    return false;
  }

  if ( response.startsWith( "<html>" ) || response.startsWith( "<HTML>" ) )
  {
    mErrorFormat = u"text/html"_s;
    mError = response;
    QgsDebugError( u"starts with <html>"_s );
    return false;
  }


  QgsDebugMsgLevel( u"Converting to Dom."_s, 2 );

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
    if ( ( f == "MIME"_L1 ) // 1.0
         || ( f == "text/plain"_L1 ) )
      format = Qgis::RasterIdentifyFormat::Text;
    else if ( f == "text/html"_L1 )
      format = Qgis::RasterIdentifyFormat::Html;
    else if ( f.startsWith( "GML."_L1 ) || f == "application/vnd.ogc.gml"_L1 || f == "application/json"_L1 || f == "application/geojson"_L1 || f == "application/geo+json"_L1 || f.contains( "gml"_L1, Qt::CaseInsensitive ) || ( f == "text/xml"_L1 && !mBaseUrl.contains( "MapServer" ) ) )
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
    mErrorFormat = u"text/plain"_s;
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
    docElement.tagName() != "WMS_Capabilities"_L1 &&    // (1.3 vintage)
    docElement.tagName() != "WMT_MS_Capabilities"_L1 && // (1.1.1 vintage)
    docElement.tagName() != "Capabilities"_L1           // WMTS
  )
  {
    mErrorCaption = QObject::tr( "Dom Exception" );
    mErrorFormat = u"text/plain"_s;
    mError = QObject::tr( "Could not get WMS capabilities in the expected format (DTD): no %1 or %2 found.\nThis might be due to an incorrect WMS Server URL.\nTag: %3\nResponse was:\n%4" )
               .arg( u"WMS_Capabilities"_s, u"WMT_MS_Capabilities"_s, docElement.tagName(), QString( xml ) );

    QgsLogger::debug( "Dom Exception: " + mError );

    return false;
  }

  capabilitiesProperty.version = docElement.attribute( u"version"_s );

  // Start walking through XML.
  QDomNode node = docElement.firstChild();

  while ( !node.isNull() )
  {
    QDomElement element = node.toElement();
    if ( !element.isNull() )
    {
      QgsDebugMsgLevel( element.tagName(), 2 ); // the node really is an element.

      if ( element.tagName() == "Service"_L1 || element.tagName() == "ows:ServiceProvider"_L1 || element.tagName() == "ows:ServiceIdentification"_L1 )
      {
        QgsDebugMsgLevel( u"  Service."_s, 2 );
        parseService( element, capabilitiesProperty.service );
      }
      else if ( element.tagName() == "Capability"_L1 || element.tagName() == "ows:OperationsMetadata"_L1 )
      {
        QgsDebugMsgLevel( u"  Capability."_s, 2 );
        parseCapability( element, capabilitiesProperty.capability );
      }
      else if ( element.tagName() == "Contents"_L1 )
      {
        QgsDebugMsgLevel( u"  Contents."_s, 2 );
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
      if ( tagName.startsWith( "wms:"_L1 ) )
        tagName = tagName.mid( 4 );
      if ( tagName.startsWith( "ows:"_L1 ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Title"_L1 )
      {
        serviceProperty.title = nodeElement.text();
      }
      else if ( tagName == "Abstract"_L1 )
      {
        serviceProperty.abstract = nodeElement.text();
      }
      else if ( tagName == "KeywordList"_L1 || tagName == "Keywords"_L1 )
      {
        parseKeywordList( nodeElement, serviceProperty.keywordList );
      }
      else if ( tagName == "OnlineResource"_L1 )
      {
        parseOnlineResource( nodeElement, serviceProperty.onlineResource );
      }
      else if ( tagName == "ContactInformation"_L1 || tagName == "ServiceContact"_L1 )
      {
        parseContactInformation( nodeElement, serviceProperty.contactInformation );
      }
      else if ( tagName == "Fees"_L1 )
      {
        serviceProperty.fees = nodeElement.text();
      }
      else if ( tagName == "AccessConstraints"_L1 )
      {
        serviceProperty.accessConstraints = nodeElement.text();
      }
      else if ( tagName == "LayerLimit"_L1 )
      {
        serviceProperty.layerLimit = nodeElement.text().toUInt();
      }
      else if ( tagName == "MaxWidth"_L1 )
      {
        serviceProperty.maxWidth = nodeElement.text().toUInt();
      }
      else if ( tagName == "MaxHeight"_L1 )
      {
        serviceProperty.maxHeight = nodeElement.text().toUInt();
      }
    }
    node = node.nextSibling();
  }
}


void QgsWmsCapabilities::parseOnlineResource( const QDomElement &element, QgsWmsOnlineResourceAttribute &onlineResourceAttribute )
{
  QUrl url = QUrl::fromEncoded( element.attribute( u"xlink:href"_s ).toUtf8() );
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
      if ( tagName.startsWith( "wms:"_L1 ) )
        tagName = tagName.mid( 4 );
      if ( tagName.startsWith( "ows:"_L1 ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Keyword"_L1 )
      {
        QgsDebugMsgLevel( u"      Keyword."_s, 2 );
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
      if ( tagName.startsWith( "wms:"_L1 ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "ContactPersonPrimary"_L1 )
      {
        parseContactPersonPrimary( nodeElement, contactInformationProperty.contactPersonPrimary );
      }
      else if ( tagName == "ContactPosition"_L1 || tagName == "ows:PositionName"_L1 )
      {
        contactInformationProperty.contactPosition = nodeElement.text();
      }
      else if ( tagName == "ContactAddress"_L1 )
      {
        parseContactAddress( nodeElement, contactInformationProperty.contactAddress );
      }
      else if ( tagName == "ContactVoiceTelephone"_L1 )
      {
        contactInformationProperty.contactVoiceTelephone = nodeElement.text();
      }
      else if ( tagName == "ContactFacsimileTelephone"_L1 )
      {
        contactInformationProperty.contactFacsimileTelephone = nodeElement.text();
      }
      else if ( tagName == "ContactElectronicMailAddress"_L1 )
      {
        contactInformationProperty.contactElectronicMailAddress = nodeElement.text();
      }
      else if ( tagName == "ows:IndividualName"_L1 )
      {
        contactInformationProperty.contactPersonPrimary.contactPerson = nodeElement.text();
      }
      else if ( tagName == "ows:ProviderName"_L1 )
      {
        contactInformationProperty.contactPersonPrimary.contactOrganization = nodeElement.text();
      }
      else if ( tagName == "ows:ContactInfo"_L1 )
      {
        QDomNode tempNode = node.firstChildElement( u"ows:Phone"_s );
        contactInformationProperty.contactVoiceTelephone = tempNode.firstChildElement( u"ows:Voice"_s ).toElement().text();
        contactInformationProperty.contactFacsimileTelephone = tempNode.firstChildElement( u"ows:Facsimile"_s ).toElement().text();

        tempNode = node.firstChildElement( u"ows:Address"_s );
        contactInformationProperty.contactElectronicMailAddress = tempNode.firstChildElement( u"ows:ElectronicMailAddress"_s ).toElement().text();
        contactInformationProperty.contactAddress.address = tempNode.firstChildElement( u"ows:DeliveryPoint"_s ).toElement().text();
        contactInformationProperty.contactAddress.city = tempNode.firstChildElement( u"ows:City"_s ).toElement().text();
        contactInformationProperty.contactAddress.stateOrProvince = tempNode.firstChildElement( u"ows:AdministrativeArea"_s ).toElement().text();
        contactInformationProperty.contactAddress.postCode = tempNode.firstChildElement( u"ows:PostalCode"_s ).toElement().text();
        contactInformationProperty.contactAddress.country = tempNode.firstChildElement( u"ows:Country"_s ).toElement().text();
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
      if ( tagName.startsWith( "wms:"_L1 ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "ContactPerson"_L1 )
      {
        contactPersonPrimaryProperty.contactPerson = nodeElement.text();
      }
      else if ( tagName == "ContactOrganization"_L1 )
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
      if ( tagName.startsWith( "wms:"_L1 ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "AddressType"_L1 )
      {
        contactAddressProperty.addressType = nodeElement.text();
      }
      else if ( tagName == "Address"_L1 )
      {
        contactAddressProperty.address = nodeElement.text();
      }
      else if ( tagName == "City"_L1 )
      {
        contactAddressProperty.city = nodeElement.text();
      }
      else if ( tagName == "StateOrProvince"_L1 )
      {
        contactAddressProperty.stateOrProvince = nodeElement.text();
      }
      else if ( tagName == "PostCode"_L1 )
      {
        contactAddressProperty.postCode = nodeElement.text();
      }
      else if ( tagName == "Country"_L1 )
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
    if ( tagName.startsWith( "wms:"_L1 ) )
      tagName = tagName.mid( 4 );

    QgsDebugMsgLevel( "  " + nodeElement.tagName(), 2 ); // the node really is an element.

    if ( tagName == "Request"_L1 )
    {
      parseRequest( nodeElement, capabilityProperty.request );
    }
    else if ( tagName == "Layer"_L1 )
    {
      QgsWmsLayerProperty layer;
      parseLayer( nodeElement, layer );
      capabilityProperty.layers.push_back( layer );
    }
    else if ( tagName == "VendorSpecificCapabilities"_L1 )
    {
      for ( int i = 0; i < nodeElement.childNodes().size(); i++ )
      {
        QDomNode childNode = nodeElement.childNodes().item( i );
        QDomElement childNodeElement = childNode.toElement();

        QString tagName = childNodeElement.tagName();
        if ( tagName.startsWith( "wms:"_L1 ) )
          tagName = tagName.mid( 4 );

        if ( tagName == "TileSet"_L1 )
        {
          parseTileSetProfile( childNodeElement );
        }
      }
    }
    else if ( tagName == "ows:Operation"_L1 )
    {
      QString name = nodeElement.attribute( u"name"_s );
      QDomElement get = node.firstChildElement( u"ows:DCP"_s )
                          .firstChildElement( u"ows:HTTP"_s )
                          .firstChildElement( u"ows:Get"_s );

      QString href = get.attribute( u"xlink:href"_s );

      QgsWmsDcpTypeProperty dcp;
      dcp.http.get.onlineResource.xlinkHref = href;

      QgsWmsOperationType *operationType = nullptr;
      if ( href.isNull() )
      {
        QgsDebugError( u"http get missing from ows:Operation '%1'"_s.arg( name ) );
      }
      else if ( name == "GetTile"_L1 )
      {
        operationType = &capabilityProperty.request.getTile;
      }
      else if ( name == "GetFeatureInfo"_L1 )
      {
        operationType = &capabilityProperty.request.getFeatureInfo;
      }
      else if ( name == "GetLegendGraphic"_L1 || name == "sld:GetLegendGraphic"_L1 )
      {
        operationType = &capabilityProperty.request.getLegendGraphic;
      }
      else
      {
        QgsDebugError( u"ows:Operation %1 ignored"_s.arg( name ) );
      }

      if ( operationType )
      {
        operationType->dcpType << dcp;
        operationType->allowedEncodings.clear();
        for ( QDomElement childNodeElement = get.firstChildElement( u"ows:Constraint"_s ).firstChildElement( u"ows:AllowedValues"_s ).firstChildElement( u"ows:Value"_s );
              !childNodeElement.isNull();
              childNodeElement = nodeElement.nextSiblingElement( u"ows:Value"_s ) )
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
      if ( operation == "Operation"_L1 )
      {
        operation = nodeElement.attribute( u"name"_s );
      }

      if ( operation == "GetMap"_L1 )
      {
        QgsDebugMsgLevel( u"      GetMap."_s, 2 );
        parseOperationType( nodeElement, requestProperty.getMap );
      }
      else if ( operation == "GetFeatureInfo"_L1 )
      {
        QgsDebugMsgLevel( u"      GetFeatureInfo."_s, 2 );
        parseOperationType( nodeElement, requestProperty.getFeatureInfo );
      }
      else if ( operation == "GetLegendGraphic"_L1 || operation == "sld:GetLegendGraphic"_L1 )
      {
        QgsDebugMsgLevel( u"      GetLegendGraphic."_s, 2 );
        parseOperationType( nodeElement, requestProperty.getLegendGraphic );
      }
    }
    node = node.nextSibling();
  }
}


void QgsWmsCapabilities::parseLegendUrl( const QDomElement &element, QgsWmsLegendUrlProperty &legendUrlProperty )
{
  legendUrlProperty.width = element.attribute( u"width"_s ).toUInt();
  legendUrlProperty.height = element.attribute( u"height"_s ).toUInt();

  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    if ( !nodeElement.isNull() )
    {
      QString tagName = nodeElement.tagName();
      if ( tagName.startsWith( "wms:"_L1 ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Format"_L1 )
      {
        legendUrlProperty.format = nodeElement.text();
      }
      else if ( tagName == "OnlineResource"_L1 )
      {
        parseOnlineResource( nodeElement, legendUrlProperty.onlineResource );
      }
    }
    node = node.nextSibling();
  }
}

void QgsWmsCapabilities::parseDimension( const QDomElement &element, QgsWmsDimensionProperty &dimensionProperty )
{
  dimensionProperty.name = element.attribute( u"name"_s );
  dimensionProperty.units = element.attribute( u"units"_s );
  dimensionProperty.unitSymbol = element.attribute( u"unitSymbol"_s );
  dimensionProperty.defaultValue = element.attribute( u"default"_s );

  if ( !element.attribute( u"multipleValues"_s ).isNull() )
  {
    QString multipleValuesAttribute = element.attribute( u"multipleValues"_s );
    dimensionProperty.multipleValues = ( multipleValuesAttribute == "1"_L1 || multipleValuesAttribute == "true"_L1 );
  }

  if ( !element.attribute( u"nearestValue"_s ).isNull() )
  {
    QString nearestValueAttribute = element.attribute( u"nearestValue"_s );
    dimensionProperty.nearestValue = ( nearestValueAttribute == "1"_L1 || nearestValueAttribute == "true"_L1 );
  }

  if ( !element.attribute( u"current"_s ).isNull() )
  {
    QString currentAttribute = element.attribute( u"current"_s );
    dimensionProperty.current = ( currentAttribute == "1"_L1 || currentAttribute == "true"_L1 );
  }

  dimensionProperty.extent = element.text().simplified();
}

void QgsWmsCapabilities::parseExtent( const QDomElement &element, QVector<QgsWmsDimensionProperty> &dimensionProperties )
{
  const QString name = element.attribute( u"name"_s );
  // try to find corresponding dimension property -- i.e. we upgrade the WMS 1.1 split of Dimension and Extent to 1.3 style where Dimension holds the extent information
  for ( auto it = dimensionProperties.begin(); it != dimensionProperties.end(); ++it )
  {
    if ( it->name == name )
    {
      it->extent = element.text().simplified();

      it->defaultValue = element.attribute( u"default"_s );

      if ( !element.attribute( u"multipleValues"_s ).isNull() )
      {
        QString multipleValuesAttribute = element.attribute( u"multipleValues"_s );
        it->multipleValues = ( multipleValuesAttribute == "1"_L1 || multipleValuesAttribute == "true"_L1 );
      }

      if ( !element.attribute( u"nearestValue"_s ).isNull() )
      {
        QString nearestValueAttribute = element.attribute( u"nearestValue"_s );
        it->nearestValue = ( nearestValueAttribute == "1"_L1 || nearestValueAttribute == "true"_L1 );
      }

      if ( !element.attribute( u"current"_s ).isNull() )
      {
        QString currentAttribute = element.attribute( u"current"_s );
        it->current = ( currentAttribute == "1"_L1 || currentAttribute == "true"_L1 );
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
      if ( tagName.startsWith( "wms:"_L1, Qt::CaseInsensitive ) )
        tagName = tagName.mid( 4 );

      if ( tagName.compare( "Format"_L1, Qt::CaseInsensitive ) == 0 )
      {
        metadataUrlProperty.format = nodeElement.text();
      }
      else if ( tagName.compare( "OnlineResource"_L1, Qt::CaseInsensitive ) == 0 )
      {
        parseOnlineResource( nodeElement, metadataUrlProperty.onlineResource );
      }
    }
    node = node.nextSibling();
  }
}


void QgsWmsCapabilities::parseLayer( const QDomElement &element, QgsWmsLayerProperty &layerProperty, QgsWmsLayerProperty *parentProperty )
{
// TODO: Delete this stanza completely, depending on success of "Inherit things into the sublayer" below.
#if 0
  // enforce WMS non-inheritance rules
  layerProperty.name = QString();
  layerProperty.title = QString();
  layerProperty.abstract = QString();
  layerProperty.keywordList.clear();
#endif

  layerProperty.orderId = ++mLayerCount;

  QString queryableAttribute = element.attribute( u"queryable"_s );
  layerProperty.queryable = queryableAttribute == "1"_L1 || queryableAttribute.compare( "true"_L1, Qt::CaseInsensitive ) == 0;

  layerProperty.cascaded = element.attribute( u"cascaded"_s ).toUInt();

  QString opaqueAttribute = element.attribute( u"opaque"_s );
  layerProperty.opaque = opaqueAttribute == "1"_L1 || opaqueAttribute.compare( "true"_L1, Qt::CaseInsensitive ) == 0;

  QString noSubsetsAttribute = element.attribute( u"noSubsets"_s );
  layerProperty.noSubsets = noSubsetsAttribute == "1"_L1 || noSubsetsAttribute.compare( "true"_L1, Qt::CaseInsensitive ) == 0;

  layerProperty.fixedWidth = element.attribute( u"fixedWidth"_s ).toUInt();
  layerProperty.fixedHeight = element.attribute( u"fixedHeight"_s ).toUInt();

  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    if ( !nodeElement.isNull() )
    {
      QString tagName = nodeElement.tagName();
      if ( tagName.startsWith( "wms:"_L1 ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Layer"_L1 )
      {
        QgsWmsLayerProperty subLayerProperty;

        // Inherit things into the sublayer
        //   Ref: 7.2.4.8 Inheritance of layer properties

        // Cleanup inherited styles, replace layer name with current layer name
        QVector<QgsWmsStyleProperty> inheritedStyles { layerProperty.style };
        if ( !nodeElement.firstChildElement( u"Name"_s ).isNull() )
        {
          const QString layerName { nodeElement.firstChildElement( u"Name"_s ).text() };
          for ( auto stylesIt = inheritedStyles.begin(); stylesIt != inheritedStyles.end(); ++stylesIt )
          {
            for ( auto legendUriIt = stylesIt->legendUrl.begin(); legendUriIt != stylesIt->legendUrl.end(); ++legendUriIt )
            {
              QUrl legendUrl { legendUriIt->onlineResource.xlinkHref };
              if ( legendUrl.hasQuery() )
              {
                QUrlQuery query { legendUrl.query() };
                if ( query.hasQueryItem( u"LAYER"_s ) )
                {
                  query.removeQueryItem( u"LAYER"_s );
                  query.addQueryItem( u"LAYER"_s, layerName );
                }
                else if ( query.hasQueryItem( u"layer"_s ) )
                {
                  query.removeQueryItem( u"layer"_s );
                  query.addQueryItem( u"layer"_s, layerName );
                }
                legendUrl.setQuery( query );
                legendUriIt->onlineResource.xlinkHref = legendUrl.url();
              }
            }
          }
        }

        subLayerProperty.style = inheritedStyles;
        subLayerProperty.crs = layerProperty.crs;
        subLayerProperty.boundingBoxes = layerProperty.boundingBoxes;
        subLayerProperty.ex_GeographicBoundingBox = layerProperty.ex_GeographicBoundingBox;
        // TODO

        parseLayer( nodeElement, subLayerProperty, &layerProperty );

        layerProperty.layer.push_back( subLayerProperty );
      }
      else if ( tagName == "Name"_L1 )
      {
        layerProperty.name = nodeElement.text();
      }
      else if ( tagName == "Title"_L1 )
      {
        layerProperty.title = nodeElement.text();
      }
      else if ( tagName == "Abstract"_L1 )
      {
        layerProperty.abstract = nodeElement.text();
      }
      else if ( tagName == "KeywordList"_L1 )
      {
        parseKeywordList( nodeElement, layerProperty.keywordList );
      }
      else if ( tagName == "SRS"_L1 || tagName == "CRS"_L1 )
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
      else if ( tagName == "LatLonBoundingBox"_L1 ) // legacy from earlier versions of WMS
      {
        // boundingBox element can contain comma as decimal separator and layer extent is not
        // calculated at all. Fixing by replacing comma with point.
        layerProperty.ex_GeographicBoundingBox = QgsRectangle(
          nodeElement.attribute( u"minx"_s ).replace( ',', '.' ).toDouble(),
          nodeElement.attribute( u"miny"_s ).replace( ',', '.' ).toDouble(),
          nodeElement.attribute( u"maxx"_s ).replace( ',', '.' ).toDouble(),
          nodeElement.attribute( u"maxy"_s ).replace( ',', '.' ).toDouble()
        );

        if ( nodeElement.hasAttribute( u"SRS"_s ) && nodeElement.attribute( u"SRS"_s ) != QgsWmsProvider::DEFAULT_LATLON_CRS )
        {
          try
          {
            QgsCoordinateReferenceSystem src = QgsCoordinateReferenceSystem::fromOgcWmsCrs( nodeElement.attribute( u"SRS"_s ) );
            QgsCoordinateReferenceSystem dst = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QgsWmsProvider::DEFAULT_LATLON_CRS );
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
      else if ( tagName == "EX_GeographicBoundingBox"_L1 ) //for WMS 1.3
      {
        QDomElement wBoundLongitudeElem, eBoundLongitudeElem, sBoundLatitudeElem, nBoundLatitudeElem;

        if ( nodeElement.tagName() == "wms:EX_GeographicBoundingBox"_L1 )
        {
          wBoundLongitudeElem = node.namedItem( u"wms:westBoundLongitude"_s ).toElement();
          eBoundLongitudeElem = node.namedItem( u"wms:eastBoundLongitude"_s ).toElement();
          sBoundLatitudeElem = node.namedItem( u"wms:southBoundLatitude"_s ).toElement();
          nBoundLatitudeElem = node.namedItem( u"wms:northBoundLatitude"_s ).toElement();
        }
        else
        {
          wBoundLongitudeElem = node.namedItem( u"westBoundLongitude"_s ).toElement();
          eBoundLongitudeElem = node.namedItem( u"eastBoundLongitude"_s ).toElement();
          sBoundLatitudeElem = node.namedItem( u"southBoundLatitude"_s ).toElement();
          nBoundLatitudeElem = node.namedItem( u"northBoundLatitude"_s ).toElement();
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
      else if ( tagName == "BoundingBox"_L1 )
      {
        QgsWmsBoundingBoxProperty bbox;
        bbox.box = QgsRectangle( nodeElement.attribute( u"minx"_s ).replace( ',', '.' ).toDouble(), nodeElement.attribute( u"miny"_s ).replace( ',', '.' ).toDouble(), nodeElement.attribute( u"maxx"_s ).replace( ',', '.' ).toDouble(), nodeElement.attribute( u"maxy"_s ).replace( ',', '.' ).toDouble() );
        if ( nodeElement.hasAttribute( u"CRS"_s ) || nodeElement.hasAttribute( u"SRS"_s ) )
        {
          if ( nodeElement.hasAttribute( u"CRS"_s ) )
            bbox.crs = nodeElement.attribute( u"CRS"_s );
          else if ( nodeElement.hasAttribute( u"SRS"_s ) )
            bbox.crs = nodeElement.attribute( u"SRS"_s );

          if ( shouldInvertAxisOrientation( bbox.crs ) )
          {
            QgsRectangle invAxisBbox( bbox.box.yMinimum(), bbox.box.xMinimum(), bbox.box.yMaximum(), bbox.box.xMaximum() );
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
          if ( !inheritedOverwritten )
            layerProperty.boundingBoxes << bbox;
        }
        else
        {
          QgsDebugError( u"CRS/SRS attribute not found in BoundingBox"_s );
        }
      }
      else if ( tagName == "Dimension"_L1 )
      {
        layerProperty.dimensions << QgsWmsDimensionProperty();
        parseDimension( nodeElement, layerProperty.dimensions.last() );
      }
      else if ( tagName == "Extent"_L1 )
      {
        // upgrade WMS 1.1 style Extent/Dimension handling to WMS 1.3
        parseExtent( nodeElement, layerProperty.dimensions );
      }
      else if ( tagName == "Attribution"_L1 )
      {
        // TODO
      }
      else if ( tagName == "AuthorityURL"_L1 )
      {
        // TODO
      }
      else if ( tagName == "Identifier"_L1 )
      {
        // TODO
      }
      else if ( tagName == "MetadataURL"_L1 )
      {
        layerProperty.metadataUrl << QgsWmsMetadataUrlProperty();
        parseMetadataUrl( nodeElement, layerProperty.metadataUrl.last() );
      }
      else if ( tagName == "DataURL"_L1 )
      {
        // TODO
      }
      else if ( tagName == "FeatureListURL"_L1 )
      {
        // TODO
      }
      else if ( tagName == "Style"_L1 )
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
      else if ( tagName == "MinScaleDenominator"_L1 )
      {
        // TODO
      }
      else if ( tagName == "MaxScaleDenominator"_L1 )
      {
        // TODO
      }
      // If we got here then it's not in the WMS 1.3 standard
    }
    node = node.nextSibling();
  }

  if ( parentProperty )
  {
    mLayerParents[layerProperty.orderId] = parentProperty->orderId;
  }

  if ( !layerProperty.name.isEmpty() )
  {
    // We have all the information we need to properly evaluate a layer definition
    // TODO: Save this somewhere

    // Store if the layer is queryable
    mQueryableForLayer[layerProperty.name] = layerProperty.queryable;

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
    mLayerParentNames[layerProperty.orderId] = QStringList() << layerProperty.name << layerProperty.title << layerProperty.abstract;
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
      if ( tagName.startsWith( "wms:"_L1 ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Name"_L1 )
      {
        styleProperty.name = nodeElement.text();
      }
      else if ( tagName == "Title"_L1 )
      {
        styleProperty.title = nodeElement.text();
      }
      else if ( tagName == "Abstract"_L1 )
      {
        styleProperty.abstract = nodeElement.text();
      }
      else if ( tagName == "LegendURL"_L1 )
      {
        styleProperty.legendUrl << QgsWmsLegendUrlProperty();
        parseLegendUrl( nodeElement, styleProperty.legendUrl.last() );
      }
      else if ( tagName == "StyleSheetURL"_L1 )
      {
        // TODO
      }
      else if ( tagName == "StyleURL"_L1 )
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
      if ( tagName.startsWith( "wms:"_L1 ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Format"_L1 )
      {
        QgsDebugMsgLevel( u"      Format."_s, 2 );
        operationType.format += nodeElement.text();
      }
      else if ( tagName == "DCPType"_L1 )
      {
        QgsDebugMsgLevel( u"      DCPType."_s, 2 );
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
      if ( nodeElement.tagName() == "HTTP"_L1 )
      {
        QgsDebugMsgLevel( u"      HTTP."_s, 2 );
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
      if ( tagName.startsWith( "wms:"_L1 ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Get"_L1 )
      {
        QgsDebugMsgLevel( u"      Get."_s, 2 );
        parseGet( nodeElement, httpProperty.get );
      }
      else if ( tagName == "Post"_L1 )
      {
        QgsDebugMsgLevel( u"      Post."_s, 2 );
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
      if ( tagName.startsWith( "wms:"_L1 ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "OnlineResource"_L1 )
      {
        QgsDebugMsgLevel( u"      OnlineResource."_s, 2 );
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
      if ( tagName.startsWith( "wms:"_L1 ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "OnlineResource"_L1 )
      {
        QgsDebugMsgLevel( u"      OnlineResource."_s, 2 );
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
  QSet<QString> uniqueFormats;
  QSet<QString> uniqueStyles;

  tileLayer.tileMode = WMSC;

  QDomNode node = element.firstChild();
  while ( !node.isNull() )
  {
    QDomElement nodeElement = node.toElement();
    {
      QgsDebugMsgLevel( "    " + nodeElement.tagName(), 2 ); // the node really is an element.

      QString tagName = nodeElement.tagName();
      if ( tagName.startsWith( "wms:"_L1 ) )
        tagName = tagName.mid( 4 );

      if ( tagName == "Layers"_L1 )
      {
        layers << nodeElement.text();
      }
      else if ( tagName == "Styles"_L1 )
      {
        const QString style = nodeElement.text();
        if ( !uniqueStyles.contains( style ) )
        {
          styles << style;
          uniqueStyles.insert( style );
        }
      }
      else if ( tagName == "Width"_L1 )
      {
        tileMatrix.tileWidth = nodeElement.text().toInt();
      }
      else if ( tagName == "Height"_L1 )
      {
        tileMatrix.tileHeight = nodeElement.text().toInt();
      }
      else if ( tagName == "SRS"_L1 )
      {
        matrixSet.crs = nodeElement.text();
      }
      else if ( tagName == "Format"_L1 )
      {
        const QString format = nodeElement.text();
        if ( !uniqueFormats.contains( format ) )
        {
          tileLayer.formats << format;
          uniqueFormats.insert( format );
        }
      }
      else if ( tagName == "BoundingBox"_L1 )
      {
        QgsWmsBoundingBoxProperty boundingBoxProperty;
        // boundingBox element can contain comma as decimal separator and layer extent is not
        // calculated at all. Fixing by replacing comma with point.
        boundingBoxProperty.box = QgsRectangle(
          nodeElement.attribute( u"minx"_s ).replace( ',', '.' ).toDouble(),
          nodeElement.attribute( u"miny"_s ).replace( ',', '.' ).toDouble(),
          nodeElement.attribute( u"maxx"_s ).replace( ',', '.' ).toDouble(),
          nodeElement.attribute( u"maxy"_s ).replace( ',', '.' ).toDouble()
        );
        if ( nodeElement.hasAttribute( u"SRS"_s ) )
          boundingBoxProperty.crs = nodeElement.attribute( u"SRS"_s );
        else if ( nodeElement.hasAttribute( u"srs"_s ) )
          boundingBoxProperty.crs = nodeElement.attribute( u"srs"_s );
        else if ( nodeElement.hasAttribute( u"CRS"_s ) )
          boundingBoxProperty.crs = nodeElement.attribute( u"CRS"_s );
        else if ( nodeElement.hasAttribute( u"crs"_s ) )
          boundingBoxProperty.crs = nodeElement.attribute( u"crs"_s );
        else
        {
          QgsDebugError( u"crs of bounding box undefined"_s );
        }

        if ( !boundingBoxProperty.crs.isEmpty() )
        {
          QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( boundingBoxProperty.crs );
          if ( crs.isValid() )
            boundingBoxProperty.crs = crs.authid();

          tileLayer.boundingBoxes << boundingBoxProperty;
        }
      }
      else if ( tagName == "Resolutions"_L1 )
      {
        resolutions = nodeElement.text().trimmed().split( ' ', Qt::SkipEmptyParts );
      }
      else
      {
        QgsDebugError( u"tileset tag %1 ignored"_s.arg( nodeElement.tagName() ) );
      }
    }
    node = node.nextSibling();
  }

  matrixSet.identifier = u"%1-wmsc-%2"_s.arg( layers.join( QLatin1Char( '_' ) ) ).arg( mTileLayersSupported.size() );

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
    tileMatrix.matrixWidth = std::ceil( tileLayer.boundingBoxes.at( 0 ).box.width() / tileMatrix.tileWidth / r );
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
  for ( QDomElement childElement = element.firstChildElement( u"TileMatrixSet"_s );
        !childElement.isNull();
        childElement = childElement.nextSiblingElement( u"TileMatrixSet"_s ) )
  {
    QgsWmtsTileMatrixSet set;
    set.identifier = childElement.firstChildElement( u"ows:Identifier"_s ).text();
    set.title = childElement.firstChildElement( u"ows:Title"_s ).text();
    set.abstract = childElement.firstChildElement( u"ows:Abstract"_s ).text();
    parseKeywords( childElement, set.keywords );

    QString supportedCRS = childElement.firstChildElement( u"ows:SupportedCRS"_s ).text();

    QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( supportedCRS );

    set.wkScaleSet = childElement.firstChildElement( u"WellKnownScaleSet"_s ).text();

    double metersPerUnit = QgsUnitTypes::fromUnitToUnitFactor( crs.mapUnits(), Qgis::DistanceUnit::Meters );

    set.crs = crs.authid();

    bool invert = !mParserSettings.ignoreAxisOrientation && crs.hasAxisInverted();
    if ( mParserSettings.invertAxisOrientation )
      invert = !invert;

    QgsDebugMsgLevel( u"tilematrix set: %1 (supportedCRS:%2 crs:%3; metersPerUnit:%4 axisInverted:%5)"_s.arg( set.identifier, supportedCRS, set.crs ).arg( metersPerUnit, 0, 'f' ).arg( invert ? "yes" : "no" ), 2 );

    for ( QDomElement secondChildElement = childElement.firstChildElement( u"TileMatrix"_s );
          !secondChildElement.isNull();
          secondChildElement = secondChildElement.nextSiblingElement( u"TileMatrix"_s ) )
    {
      QgsWmtsTileMatrix tileMatrix;

      tileMatrix.identifier = secondChildElement.firstChildElement( u"ows:Identifier"_s ).text();
      tileMatrix.title = secondChildElement.firstChildElement( u"ows:Title"_s ).text();
      tileMatrix.abstract = secondChildElement.firstChildElement( u"ows:Abstract"_s ).text();
      parseKeywords( secondChildElement, tileMatrix.keywords );

      tileMatrix.scaleDenom = secondChildElement.firstChildElement( u"ScaleDenominator"_s ).text().toDouble();

      QStringList topLeft = secondChildElement.firstChildElement( u"TopLeftCorner"_s ).text().split( ' ', Qt::SkipEmptyParts );
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
        QgsDebugError( u"Could not parse topLeft"_s );
        continue;
      }

      tileMatrix.tileWidth = secondChildElement.firstChildElement( u"TileWidth"_s ).text().toInt();
      tileMatrix.tileHeight = secondChildElement.firstChildElement( u"TileHeight"_s ).text().toInt();
      tileMatrix.matrixWidth = secondChildElement.firstChildElement( u"MatrixWidth"_s ).text().toInt();
      tileMatrix.matrixHeight = secondChildElement.firstChildElement( u"MatrixHeight"_s ).text().toInt();

      // the magic number below is "standardized rendering pixel size" defined
      // in WMTS (and WMS 1.3) standard, being 0.28 pixel
      tileMatrix.tres = tileMatrix.scaleDenom * 0.00028 / metersPerUnit;

      QgsDebugMsgLevel( u" %1: scale=%2 res=%3 tile=%4x%5 matrix=%6x%7 topLeft=%8"_s.arg( tileMatrix.identifier ).arg( tileMatrix.scaleDenom ).arg( tileMatrix.tres ).arg( tileMatrix.tileWidth ).arg( tileMatrix.tileHeight ).arg( tileMatrix.matrixWidth ).arg( tileMatrix.matrixHeight ).arg( tileMatrix.topLeft.toString() ), 2 );

      set.tileMatrices.insert( tileMatrix.tres, tileMatrix );
    }

    mTileMatrixSets.insert( set.identifier, set );
    if ( mFirstTileMatrixSetId.isEmpty() )
      mFirstTileMatrixSetId = set.identifier;
  }

  //
  // layers
  //

  mTileLayersSupported.clear();
  for ( QDomElement childElement = element.firstChildElement( u"Layer"_s );
        !childElement.isNull();
        childElement = childElement.nextSiblingElement( u"Layer"_s ) )
  {
#ifdef QGISDEBUG
    QString id = childElement.firstChildElement( u"ows:Identifier"_s ).text(); // clazy:exclude=unused-non-trivial-variable
    QgsDebugMsgLevel( u"Layer %1"_s.arg( id ), 2 );
#endif

    QgsWmtsTileLayer tileLayer;
    tileLayer.tileMode = WMTS;
    tileLayer.identifier = childElement.firstChildElement( u"ows:Identifier"_s ).text();
    tileLayer.title = childElement.firstChildElement( u"ows:Title"_s ).text();
    tileLayer.abstract = childElement.firstChildElement( u"ows:Abstract"_s ).text();
    parseKeywords( childElement, tileLayer.keywords );

    QgsWmsBoundingBoxProperty boundingBoxProperty;

    QDomElement bbox = childElement.firstChildElement( u"ows:WGS84BoundingBox"_s );
    if ( !bbox.isNull() )
    {
      QStringList ll = bbox.firstChildElement( u"ows:LowerCorner"_s ).text().split( ' ', Qt::SkipEmptyParts );
      QStringList ur = bbox.firstChildElement( u"ows:UpperCorner"_s ).text().split( ' ', Qt::SkipEmptyParts );

      if ( ll.size() == 2 && ur.size() == 2 )
      {
        boundingBoxProperty.crs = QgsWmsProvider::DEFAULT_LATLON_CRS;
        boundingBoxProperty.box = QgsRectangle( QgsPointXY( ll[0].toDouble(), ll[1].toDouble() ), QgsPointXY( ur[0].toDouble(), ur[1].toDouble() ) );

        tileLayer.boundingBoxes << boundingBoxProperty;
      }
    }

    for ( bbox = childElement.firstChildElement( u"ows:BoundingBox"_s );
          !bbox.isNull();
          bbox = bbox.nextSiblingElement( u"ows:BoundingBox"_s ) )
    {
      QStringList ll = bbox.firstChildElement( u"ows:LowerCorner"_s ).text().split( ' ', Qt::SkipEmptyParts );
      QStringList ur = bbox.firstChildElement( u"ows:UpperCorner"_s ).text().split( ' ', Qt::SkipEmptyParts );

      if ( ll.size() == 2 && ur.size() == 2 )
      {
        boundingBoxProperty.box = QgsRectangle( QgsPointXY( ll[0].toDouble(), ll[1].toDouble() ), QgsPointXY( ur[0].toDouble(), ur[1].toDouble() ) );

        if ( bbox.hasAttribute( u"SRS"_s ) )
          boundingBoxProperty.crs = bbox.attribute( u"SRS"_s );
        else if ( bbox.hasAttribute( u"srs"_s ) )
          boundingBoxProperty.crs = bbox.attribute( u"srs"_s );
        else if ( bbox.hasAttribute( u"CRS"_s ) )
          boundingBoxProperty.crs = bbox.attribute( u"CRS"_s );
        else if ( bbox.hasAttribute( u"crs"_s ) )
          boundingBoxProperty.crs = bbox.attribute( u"crs"_s );
        else
        {
          QgsDebugError( u"crs of bounding box undefined"_s );
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

    for ( QDomElement secondChildElement = childElement.firstChildElement( u"Style"_s );
          !secondChildElement.isNull();
          secondChildElement = secondChildElement.nextSiblingElement( u"Style"_s ) )
    {
      QgsWmtsStyle style;
      style.identifier = secondChildElement.firstChildElement( u"ows:Identifier"_s ).text();
      style.title = secondChildElement.firstChildElement( u"ows:Title"_s ).text();
      style.abstract = secondChildElement.firstChildElement( u"ows:Abstract"_s ).text();
      parseKeywords( secondChildElement, style.keywords );

      for ( QDomElement thirdChildElement = secondChildElement.firstChildElement( u"ows:legendURL"_s );
            !thirdChildElement.isNull();
            thirdChildElement = thirdChildElement.nextSiblingElement( u"ows:legendURL"_s ) )
      {
        QgsWmtsLegendURL legendURL;

        legendURL.format = thirdChildElement.firstChildElement( u"format"_s ).text();
        legendURL.minScale = thirdChildElement.firstChildElement( u"minScale"_s ).text().toDouble();
        legendURL.maxScale = thirdChildElement.firstChildElement( u"maxScale"_s ).text().toDouble();
        legendURL.href = thirdChildElement.firstChildElement( u"href"_s ).text();
        legendURL.width = thirdChildElement.firstChildElement( u"width"_s ).text().toInt();
        legendURL.height = thirdChildElement.firstChildElement( u"height"_s ).text().toInt();

        style.legendURLs << legendURL;
      }
      QDomElement thirdChildElement = secondChildElement.firstChildElement( u"LegendURL"_s );
      if ( !thirdChildElement.isNull() )
      {
        QgsWmtsLegendURL legendURL;

        legendURL.format = thirdChildElement.attribute( u"format"_s );
        legendURL.minScale = thirdChildElement.attribute( u"minScaleDenominator"_s ).toDouble();
        legendURL.maxScale = thirdChildElement.attribute( u"maxScaleDenominator"_s ).toDouble();
        legendURL.href = thirdChildElement.attribute( u"xlink:href"_s );
        legendURL.width = thirdChildElement.attribute( u"width"_s ).toInt();
        legendURL.height = thirdChildElement.attribute( u"height"_s ).toInt();

        style.legendURLs << legendURL;
      }

      style.isDefault = secondChildElement.attribute( u"isDefault"_s ) == "true"_L1;

      tileLayer.styles.insert( style.identifier, style );

      if ( style.isDefault )
        tileLayer.defaultStyle = style.identifier;
    }

    if ( tileLayer.styles.isEmpty() )
    {
      QgsWmtsStyle style;
      style.identifier = u"default"_s;
      style.title = QObject::tr( "Generated default style" );
      style.abstract = QObject::tr( "Style was missing in capabilities" );
      tileLayer.styles.insert( style.identifier, style );
    }

    {
      QSet<QString> uniqueFormats;
      for ( QDomElement secondChildElement = childElement.firstChildElement( u"Format"_s ); !secondChildElement.isNull(); secondChildElement = secondChildElement.nextSiblingElement( u"Format"_s ) )
      {
        const QString format = secondChildElement.text();
        if ( !uniqueFormats.contains( format ) )
        {
          tileLayer.formats << format;
          uniqueFormats.insert( format );
        }
      }
    }

    for ( QDomElement secondChildElement = childElement.firstChildElement( u"InfoFormat"_s ); !secondChildElement.isNull(); secondChildElement = secondChildElement.nextSiblingElement( u"InfoFormat"_s ) )
    {
      QString format = secondChildElement.text();

      tileLayer.infoFormats << secondChildElement.text();

      Qgis::RasterIdentifyFormat fmt = Qgis::RasterIdentifyFormat::Undefined;

      QgsDebugMsgLevel( u"format=%1"_s.arg( format ), 2 );

      if ( ( format == "MIME"_L1 ) // 1.0
           || ( format == "text/plain"_L1 ) )
        fmt = Qgis::RasterIdentifyFormat::Text;
      else if ( format == "text/html"_L1 )
        fmt = Qgis::RasterIdentifyFormat::Html;
      else if ( format.startsWith( "GML."_L1 ) // 1.0
                || ( format == "application/vnd.ogc.gml"_L1 )
                || ( format.contains( "gml"_L1, Qt::CaseInsensitive ) )
                || ( format == "application/json"_L1 )
                || ( format == "application/geojson"_L1 )
                || ( format == "application/geo+json"_L1 ) )
        fmt = Qgis::RasterIdentifyFormat::Feature;
      else
      {
        QgsDebugError( u"Unsupported featureInfoUrl format: %1"_s.arg( format ) );
        continue;
      }

      QgsDebugMsgLevel( u"fmt=%1"_s.arg( qgsEnumValueToKey( fmt ) ), 2 );
      mIdentifyFormats.insert( fmt, format );
    }

    for ( QDomElement secondChildElement = childElement.firstChildElement( u"Dimension"_s ); !secondChildElement.isNull(); secondChildElement = secondChildElement.nextSiblingElement( u"Dimension"_s ) )
    {
      QgsWmtsDimension dimension;

      dimension.identifier = secondChildElement.firstChildElement( u"ows:Identifier"_s ).text();
      if ( dimension.identifier.isEmpty() )
        continue;

      dimension.title = secondChildElement.firstChildElement( u"ows:Title"_s ).text();
      dimension.abstract = secondChildElement.firstChildElement( u"ows:Abstract"_s ).text();
      parseKeywords( secondChildElement, dimension.keywords );

      dimension.UOM = secondChildElement.firstChildElement( u"UOM"_s ).text();
      dimension.unitSymbol = secondChildElement.firstChildElement( u"unitSymbol"_s ).text();
      dimension.defaultValue = secondChildElement.firstChildElement( u"Default"_s ).text();
      dimension.current = secondChildElement.firstChildElement( u"current"_s ).text() == "true"_L1;

      for ( QDomElement thirdChildElement = secondChildElement.firstChildElement( u"Value"_s );
            !thirdChildElement.isNull();
            thirdChildElement = thirdChildElement.nextSiblingElement( u"Value"_s ) )
      {
        dimension.values << thirdChildElement.text();
      }

      tileLayer.dimensions.insert( dimension.identifier, dimension );

      if ( ( dimension.title.compare( "time"_L1, Qt::CaseInsensitive ) == 0 || dimension.identifier.compare( "time"_L1, Qt::CaseInsensitive ) == 0 )
           && !dimension.values.empty() )
      {
        // we will use temporal framework if there's multiple time dimension values, OR if a single time dimension value is itself an interval
        bool useTemporalFramework = dimension.values.size() > 1;
        if ( !useTemporalFramework )
        {
          const thread_local QRegularExpression rxPeriod( u".*/P.*"_s );
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
            const thread_local QRegularExpression rxPeriod( u".*/P.*"_s );
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
                const QList<QDateTime> dates = QgsTemporalUtils::calculateDateTimesUsingDuration( begin, end, itemResolution, maxValuesExceeded, 1000 );
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

    for ( QDomElement secondChildElement = childElement.firstChildElement( u"TileMatrixSetLink"_s ); !secondChildElement.isNull(); secondChildElement = secondChildElement.nextSiblingElement( u"TileMatrixSetLink"_s ) )
    {
      QgsWmtsTileMatrixSetLink setLink;

      setLink.tileMatrixSet = secondChildElement.firstChildElement( u"TileMatrixSet"_s ).text();

      if ( !mTileMatrixSets.contains( setLink.tileMatrixSet ) )
      {
        QgsDebugError( u"  TileMatrixSet %1 not found."_s.arg( setLink.tileMatrixSet ) );
        continue;
      }

      const QgsWmtsTileMatrixSet &tms = mTileMatrixSets[setLink.tileMatrixSet];

      for ( QDomElement thirdChildElement = secondChildElement.firstChildElement( u"TileMatrixSetLimits"_s ); !thirdChildElement.isNull(); thirdChildElement = thirdChildElement.nextSiblingElement( u"TileMatrixSetLimits"_s ) )
      {
        for ( QDomElement fourthChildElement = thirdChildElement.firstChildElement( u"TileMatrixLimits"_s ); !fourthChildElement.isNull(); fourthChildElement = fourthChildElement.nextSiblingElement( u"TileMatrixLimits"_s ) )
        {
          QgsWmtsTileMatrixLimits limit;

          QString id = fourthChildElement.firstChildElement( u"TileMatrix"_s ).text();

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
            limit.minTileRow = fourthChildElement.firstChildElement( u"MinTileRow"_s ).text().toInt();
            limit.maxTileRow = fourthChildElement.firstChildElement( u"MaxTileRow"_s ).text().toInt();
            limit.minTileCol = fourthChildElement.firstChildElement( u"MinTileCol"_s ).text().toInt();
            limit.maxTileCol = fourthChildElement.firstChildElement( u"MaxTileCol"_s ).text().toInt();

            isValid = limit.minTileCol >= 0 && limit.minTileCol < matrixWidth && limit.maxTileCol >= 0 && limit.maxTileCol < matrixWidth && limit.minTileCol <= limit.maxTileCol && limit.minTileRow >= 0 && limit.minTileRow < matrixHeight && limit.maxTileRow >= 0 && limit.maxTileRow < matrixHeight && limit.minTileRow <= limit.maxTileRow;
          }
          else
          {
            QgsDebugError( u"   TileMatrix id:%1 not found."_s.arg( id ) );
          }

          QgsDebugMsgLevel( u"   TileMatrixLimit id:%1 row:%2-%3 col:%4-%5 matrix:%6x%7 %8"_s.arg( id ).arg( limit.minTileRow ).arg( limit.maxTileRow ).arg( limit.minTileCol ).arg( limit.maxTileCol ).arg( matrixWidth ).arg( matrixHeight ).arg( isValid ? "valid" : "INVALID" ), 2 );

          if ( isValid )
          {
            setLink.limits.insert( id, limit );
          }
        }
      }

      tileLayer.setLinks.insert( setLink.tileMatrixSet, setLink );
    }

    for ( QDomElement secondChildElement = childElement.firstChildElement( u"ResourceURL"_s ); !secondChildElement.isNull(); secondChildElement = secondChildElement.nextSiblingElement( u"ResourceURL"_s ) )
    {
      QString format = nodeAttribute( secondChildElement, u"format"_s );
      QString resourceType = nodeAttribute( secondChildElement, u"resourceType"_s );
      QString tmpl = nodeAttribute( secondChildElement, u"template"_s );

      if ( format.isEmpty() || resourceType.isEmpty() || tmpl.isEmpty() )
      {
        QgsDebugMsgLevel( u"SKIPPING ResourceURL format=%1 resourceType=%2 template=%3"_s.arg( format, resourceType, tmpl ), 2 );
        continue;
      }

      if ( resourceType == "tile"_L1 )
      {
        tileLayer.getTileURLs.insert( format, tmpl );
      }
      else if ( resourceType == "FeatureInfo"_L1 )
      {
        tileLayer.getFeatureInfoURLs.insert( format, tmpl );

        Qgis::RasterIdentifyFormat fmt = Qgis::RasterIdentifyFormat::Undefined;

        QgsDebugMsgLevel( u"format=%1"_s.arg( format ), 2 );

        if ( ( format == "MIME"_L1 ) // 1.0
             || ( format == "text/plain"_L1 ) )
          fmt = Qgis::RasterIdentifyFormat::Text;
        else if ( format == "text/html"_L1 )
          fmt = Qgis::RasterIdentifyFormat::Html;
        else if ( format.startsWith( "GML."_L1 ) // 1.0
                  || ( format == "application/vnd.ogc.gml"_L1 )
                  || ( format.contains( "gml"_L1, Qt::CaseInsensitive ) )
                  || ( format == "application/json"_L1 )
                  || ( format == "application/geojson"_L1 )
                  || ( format == "application/geo+json"_L1 ) )
          fmt = Qgis::RasterIdentifyFormat::Feature;
        else
        {
          QgsDebugError( u"Unsupported featureInfoUrl format: %1"_s.arg( format ) );
          continue;
        }

        QgsDebugMsgLevel( u"fmt=%1"_s.arg( qgsEnumValueToKey( fmt ) ), 2 );
        mIdentifyFormats.insert( fmt, format );
      }
      else
      {
        QgsDebugError( u"UNEXPECTED resourceType in ResourcURL format=%1 resourceType=%2 template=%3"_s
                         .arg( format, resourceType, tmpl ) );
      }
    }

    QgsDebugMsgLevel( u"add layer %1"_s.arg( id ), 2 );
    mTileLayersSupported << tileLayer;
  }

  //
  // themes
  //
  mTileThemes.clear();
  for ( QDomElement childElement = element.firstChildElement( u"Themes"_s ).firstChildElement( u"Theme"_s );
        !childElement.isNull();
        childElement = childElement.nextSiblingElement( u"Theme"_s ) )
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
        boundingBoxProperty.crs = QgsWmsProvider::DEFAULT_LATLON_CRS;
        boundingBoxProperty.box = QgsRectangle( -180.0, -90.0, 180.0, 90.0 );
        tileLayer.boundingBoxes << boundingBoxProperty;
      }
    }
  }
}


void QgsWmsCapabilities::parseKeywords( const QDomNode &node, QStringList &keywords )
{
  keywords.clear();

  for ( QDomElement nodeElement = node.firstChildElement( u"ows:Keywords"_s ).firstChildElement( u"ows:Keyword"_s );
        !nodeElement.isNull();
        nodeElement = nodeElement.nextSiblingElement( u"ows:Keyword"_s ) )
  {
    keywords << nodeElement.text();
  }
}

void QgsWmsCapabilities::parseTheme( const QDomElement &element, QgsWmtsTheme &theme )
{
  theme.identifier = element.firstChildElement( u"ows:Identifier"_s ).text();
  theme.title = element.firstChildElement( u"ows:Title"_s ).text();
  theme.abstract = element.firstChildElement( u"ows:Abstract"_s ).text();
  parseKeywords( element, theme.keywords );

  QDomElement themeElement = element.firstChildElement( u"ows:Theme"_s );
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
  for ( QDomElement childElement = element.firstChildElement( u"ows:LayerRef"_s );
        !childElement.isNull();
        childElement = childElement.nextSiblingElement( u"ows:LayerRef"_s ) )
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
    QgsPointXY bottomRight( tm.topLeft.x() + res * tm.tileWidth * tm.matrixWidth, tm.topLeft.y() - res * tm.tileHeight * tm.matrixHeight );

    QgsDebugMsgLevel( u"detecting WMTS layer bounding box: tileset %1 matrix %2 crs %3 res %4"_s.arg( tmsIt->identifier, tm.identifier, tmsIt->crs ).arg( res ), 2 );

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
  if ( !mParserSettings.ignoreAxisOrientation && ( mCapabilities.version == "1.3.0"_L1 || mCapabilities.version == "1.3"_L1 ) )
  {
    //have we already checked this crs?
    if ( mCrsInvertAxis.contains( ogcCrs ) )
    {
      //if so, return previous result to save time
      return mCrsInvertAxis[ogcCrs];
    }

    //create CRS from string
    QgsCoordinateReferenceSystem srs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( ogcCrs );
    if ( srs.isValid() && srs.hasAxisInverted() )
    {
      changeXY = true;
    }

    //cache result to speed up future checks
    mCrsInvertAxis[ogcCrs] = changeXY;
  }

  if ( mParserSettings.invertAxisOrientation )
    changeXY = !changeXY;

  return changeXY;
}

Qgis::RasterInterfaceCapabilities QgsWmsCapabilities::identifyCapabilities() const
{
  Qgis::RasterInterfaceCapabilities capability = Qgis::RasterInterfaceCapability::NoCapabilities;

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

QgsWmsCapabilitiesDownload::QgsWmsCapabilitiesDownload( const QString &baseUrl, const QgsAuthorizationSettings &auth, bool forceRefresh, QObject *parent )
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

bool QgsWmsCapabilitiesDownload::downloadCapabilities( const QString &baseUrl, const QgsAuthorizationSettings &auth )
{
  mBaseUrl = baseUrl;
  mAuth = auth;
  return downloadCapabilities();
}

bool QgsWmsCapabilitiesDownload::downloadCapabilities()
{
  QgsDebugMsgLevel( u"entering: forceRefresh=%1"_s.arg( mForceRefresh ), 2 );
  abort(); // cancel previous
  mIsAborted = false;

  QString url = mBaseUrl;


  if ( !QgsWmsProvider::isUrlForWMTS( url ) )
  {
    url += "SERVICE=WMS&REQUEST=GetCapabilities"_L1;
  }

  QgsDebugMsgLevel( u"url = %1"_s.arg( url ), 2 );

  mError.clear();


  QNetworkRequest request( url );
  QgsSetRequestInitiatorClass( request, u"QgsWmsCapabilitiesDownload"_s );
  if ( !mAuth.setAuthorization( request ) )
  {
    mError = tr( "Download of capabilities failed:\nnetwork request update failed for authentication config" );
    QgsMessageLog::logMessage( mError, tr( "WMS" ) );
    return false;
  }
  request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy );
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
  QString message = tr( "%1 of %2 bytes of capabilities downloaded." ).arg( bytesReceived ).arg( bytesTotal < 0 ? u"unknown number of"_s : QString::number( bytesTotal ) );
  QgsDebugMsgLevel( message, 2 );
  emit statusChanged( message );
}

void QgsWmsCapabilitiesDownload::capabilitiesReplyFinished()
{
  if ( !mIsAborted && mCapabilitiesReply )
  {
    if ( mCapabilitiesReply->error() == QNetworkReply::NoError )
    {
      QgsDebugMsgLevel( u"reply OK"_s, 2 );
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
          QgsSetRequestInitiatorClass( request, u"QgsWmsCapabilitiesDownload"_s );
          if ( !mAuth.setAuthorization( request ) )
          {
            mHttpCapabilitiesResponse.clear();
            mError = tr( "Download of capabilities failed:\nnetwork request update failed for authentication config" );
            QgsMessageLog::logMessage( mError, tr( "WMS" ) );
            emit downloadFinished();
            return;
          }
          request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy );
          request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, mForceRefresh ? QNetworkRequest::AlwaysNetwork : QNetworkRequest::PreferCache );
          request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

          mCapabilitiesReply->deleteLater();
          mCapabilitiesReply = nullptr;

          QgsDebugMsgLevel( u"redirected getcapabilities:\n%1 forceRefresh=%2"_s.arg( redirect.toString() ).arg( mForceRefresh ), 2 );
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

          QgsDebugMsgLevel( u"expirationDate:%1"_s.arg( cmd.expirationDate().toString() ), 2 );
          if ( cmd.expirationDate().isNull() )
          {
            QgsSettings s;
            cmd.setExpirationDate( QDateTime::currentDateTime().addSecs( s.value( u"qgis/defaultCapabilitiesExpiry"_s, "24" ).toInt() * 60 * 60 ) );
          }

          nam->cache()->updateMetaData( cmd );
        }
        else
        {
          QgsDebugMsgLevel( u"No cache for capabilities!"_s, 2 );
        }

#ifdef QGISDEBUG
        bool fromCache = mCapabilitiesReply->attribute( QNetworkRequest::SourceIsFromCacheAttribute ).toBool();
        QgsDebugMsgLevel( u"Capabilities reply was cached:\n%1"_s.arg( fromCache ), 2 );
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
      if ( contentType.startsWith( "text/plain"_L1 ) )
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
    topLeft.x() + col * twMap,
    topLeft.y() - ( row + 1 ) * thMap,
    topLeft.x() + ( col + 1 ) * twMap,
    topLeft.y() - row * thMap
  );
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

  col0 = std::clamp( ( int ) std::floor( ( viewExtent.xMinimum() - topLeft.x() ) / twMap ), minTileCol, maxTileCol );
  row0 = std::clamp( ( int ) std::floor( ( topLeft.y() - viewExtent.yMaximum() ) / thMap ), minTileRow, maxTileRow );
  col1 = std::clamp( ( int ) std::floor( ( viewExtent.xMaximum() - topLeft.x() ) / twMap ), minTileCol, maxTileCol );
  row1 = std::clamp( ( int ) std::floor( ( topLeft.y() - viewExtent.yMinimum() ) / thMap ), minTileRow, maxTileRow );
}

const QgsWmtsTileMatrix *QgsWmtsTileMatrixSet::findNearestResolution( double vres ) const
{
  QMap<double, QgsWmtsTileMatrix>::const_iterator prev, it = tileMatrices.constBegin();
  while ( it != tileMatrices.constEnd() && it.key() < vres )
  {
    prev = it;
    ++it;
  }

  if ( it == tileMatrices.constEnd() || ( it != tileMatrices.constBegin() && vres - prev.key() < it.key() - vres ) )
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
