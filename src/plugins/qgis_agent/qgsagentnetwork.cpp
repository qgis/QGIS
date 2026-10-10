#include "qgsagentplugin.h"

#include "qgsapplication.h"
#include "qgsnetworkaccessmanager.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QEventLoop>
#include <QFileInfo>
#include <QHostAddress>
#include <QHostInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

#include <algorithm>
#include <cmath>
#include <limits>

using namespace Qt::StringLiterals;

namespace
{
  constexpr qint64 MAX_DOWNLOAD_LIMIT = 512LL * 1024 * 1024;
  constexpr qint64 JSON_RESPONSE_LIMIT = 25LL * 1024 * 1024;

  struct NetworkResponse
  {
    bool ok = false;
    int statusCode = 0;
    QByteArray data;
    QString contentType;
    QString error;
    QUrl finalUrl;
  };

  bool isPublicNetworkUrl( const QUrl &url, QString &error )
  {
    if ( !url.isValid() || ( url.scheme() != u"https"_s && url.scheme() != u"http"_s ) )
    {
      error = QObject::tr( "Only valid HTTP or HTTPS URLs are allowed." );
      return false;
    }
    if ( url.host().isEmpty() || !url.userInfo().isEmpty() )
    {
      error = QObject::tr( "The URL must contain a public host and must not embed credentials." );
      return false;
    }

    const QString host = url.host().toLower();
    if ( host == u"localhost"_s || host.endsWith( u".localhost"_s ) || host.endsWith( u".local"_s ) )
    {
      error = QObject::tr( "Local network hosts are not allowed." );
      return false;
    }

    QHostAddress literalAddress;
    QList<QHostAddress> addresses;
    if ( literalAddress.setAddress( host ) )
    {
      addresses.append( literalAddress );
    }
    else
    {
      const QHostInfo hostInfo = QHostInfo::fromName( host );
      if ( hostInfo.error() != QHostInfo::NoError || hostInfo.addresses().isEmpty() )
      {
        error = QObject::tr( "Could not resolve the remote host: %1" ).arg( host );
        return false;
      }
      addresses = hostInfo.addresses();
    }

    for ( const QHostAddress &address : std::as_const( addresses ) )
    {
      if ( address.isNull() || address.isLoopback() || address.isLinkLocal() || address.isSiteLocal()
           || address.isUniqueLocalUnicast() || address.isPrivateUse() || address.isMulticast() || address.isBroadcast()
           || !address.isGlobal() )
      {
        error = QObject::tr( "The remote host resolves to a non-public address, which is not allowed." );
        return false;
      }
    }
    return true;
  }

  NetworkResponse requestPublicUrl(
    const QUrl &initialUrl,
    const QByteArray &method,
    const QByteArray &body,
    const QByteArray &contentType,
    qint64 maximumBytes,
    int timeoutSeconds
  )
  {
    QUrl currentUrl = initialUrl;
    for ( int redirectCount = 0; redirectCount <= 5; ++redirectCount )
    {
      NetworkResponse response;
      response.finalUrl = currentUrl;
      if ( !isPublicNetworkUrl( currentUrl, response.error ) )
        return response;

      QNetworkRequest request( currentUrl );
      request.setAttribute( QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy );
      request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork );
      request.setTransferTimeout( timeoutSeconds * 1000 );
      request.setRawHeader( "Accept", method == "POST" ? "application/json, text/plain;q=0.9, */*;q=0.5" : "*/*" );
      request.setRawHeader( "User-Agent", "QGIS-Agent/0.1 (user-initiated GIS data request)" );
      if ( !contentType.isEmpty() )
        request.setRawHeader( "Content-Type", contentType );

      QNetworkReply *reply = method == "POST"
                               ? QgsNetworkAccessManager::instance()->post( request, body )
                               : QgsNetworkAccessManager::instance()->get( request );
      QByteArray responseData;
      bool tooLarge = false;
      bool timedOut = false;
      QEventLoop loop;
      QTimer timer;
      timer.setSingleShot( true );
      QObject::connect( reply, &QNetworkReply::readyRead, &loop, [&] {
        responseData.append( reply->readAll() );
        if ( responseData.size() > maximumBytes )
        {
          tooLarge = true;
          reply->abort();
        }
      } );
      QObject::connect( reply, &QNetworkReply::metaDataChanged, &loop, [&] {
        const qint64 declaredSize = reply->header( QNetworkRequest::ContentLengthHeader ).toLongLong();
        if ( declaredSize > maximumBytes )
        {
          tooLarge = true;
          reply->abort();
        }
      } );
      QObject::connect( reply, &QNetworkReply::finished, &loop, &QEventLoop::quit );
      QObject::connect( &timer, &QTimer::timeout, &loop, [&] {
        timedOut = true;
        reply->abort();
      } );
      timer.start( timeoutSeconds * 1000 );
      loop.exec();
      timer.stop();
      responseData.append( reply->readAll() );

      response.statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
      response.contentType = QString::fromUtf8( reply->rawHeader( "Content-Type" ) );
      const QUrl redirectUrl = reply->attribute( QNetworkRequest::RedirectionTargetAttribute ).toUrl();
      const QNetworkReply::NetworkError networkError = reply->error();
      const QString networkErrorText = reply->errorString();
      reply->deleteLater();

      if ( tooLarge )
      {
        response.error = QObject::tr( "The response exceeded the %1 MB download limit." ).arg( maximumBytes / 1024 / 1024 );
        return response;
      }
      if ( timedOut )
      {
        response.error = QObject::tr( "The network request timed out after %1 seconds." ).arg( timeoutSeconds );
        return response;
      }
      if ( redirectUrl.isValid() && !redirectUrl.isEmpty() )
      {
        if ( redirectCount == 5 )
        {
          response.error = QObject::tr( "The request exceeded the redirect limit." );
          return response;
        }
        currentUrl = currentUrl.resolved( redirectUrl );
        continue;
      }
      if ( networkError != QNetworkReply::NoError )
      {
        const QString responseDetail = QString::fromUtf8( responseData.left( 4096 ) ).simplified();
        response.error = responseDetail.isEmpty() ? networkErrorText : u"%1 Response: %2"_s.arg( networkErrorText, responseDetail );
        return response;
      }
      if ( response.statusCode < 200 || response.statusCode >= 300 )
      {
        response.error = QObject::tr( "The server returned HTTP %1." ).arg( response.statusCode );
        return response;
      }
      if ( responseData.isEmpty() )
      {
        response.error = QObject::tr( "The server returned an empty response." );
        return response;
      }

      response.ok = true;
      response.data = responseData;
      response.finalUrl = currentUrl;
      return response;
    }
    return NetworkResponse();
  }

  QString safeFileName( QString name )
  {
    name = QFileInfo( name ).fileName();
    name.replace( QRegularExpression( u"[^A-Za-z0-9._-]+"_s ), u"_"_s );
    return name.left( 160 );
  }

  QString defaultDownloadPath( const QUrl &url, const QString &fallbackSuffix )
  {
    QString fileName = safeFileName( QFileInfo( url.path() ).fileName() );
    if ( fileName.isEmpty() )
      fileName = u"download_%1.%2"_s.arg( QDateTime::currentDateTime().toString( u"yyyyMMdd_HHmmss"_s ), fallbackSuffix );
    QString directory = QStandardPaths::writableLocation( QStandardPaths::DownloadLocation );
    if ( directory.isEmpty() )
      directory = QStandardPaths::writableLocation( QStandardPaths::TempLocation );
    directory = QDir( directory ).filePath( u"QGIS-Agent"_s );
    return QDir( directory ).filePath( fileName );
  }

  bool hasSupportedDownloadExtension( const QString &path )
  {
    const QString lower = path.toLower();
    const QStringList suffixes{
      u".geojson"_s, u".json"_s, u".gpkg"_s, u".shp.zip"_s, u".zip"_s, u".tif"_s, u".tiff"_s,
      u".vrt"_s, u".img"_s, u".asc"_s, u".csv"_s, u".kml"_s, u".gml"_s, u".fgb"_s,
      u".parquet"_s, u".osm"_s, u".pbf"_s, u".las"_s, u".laz"_s, u".nc"_s, u".grib"_s,
      u".png"_s, u".jpg"_s, u".jpeg"_s,
    };
    for ( const QString &suffix : suffixes )
    {
      if ( lower.endsWith( suffix ) )
        return true;
    }
    return false;
  }

  bool writeNewFile( const QString &path, const QByteArray &data, QString &error )
  {
    const QFileInfo info( path );
    if ( !QDir::isAbsolutePath( path ) || info.exists() )
    {
      error = info.exists() ? QObject::tr( "The output already exists; network tools never overwrite existing files." )
                            : QObject::tr( "An absolute output path is required." );
      return false;
    }
    if ( !QDir().mkpath( info.absolutePath() ) )
    {
      error = QObject::tr( "Could not create the output directory." );
      return false;
    }
    QSaveFile file( path );
    if ( !file.open( QIODevice::WriteOnly ) || file.write( data ) != data.size() || !file.commit() )
    {
      error = file.errorString();
      return false;
    }
    return true;
  }

  QJsonArray coordinateArray( const QJsonArray &geometry )
  {
    QJsonArray coordinates;
    for ( const QJsonValue &pointValue : geometry )
    {
      const QJsonObject point = pointValue.toObject();
      if ( point.contains( u"lon"_s ) && point.contains( u"lat"_s ) )
        coordinates.append( QJsonArray{ point.value( u"lon"_s ).toDouble(), point.value( u"lat"_s ).toDouble() } );
    }
    return coordinates;
  }

  bool osmAreaTags( const QJsonObject &tags )
  {
    if ( tags.value( u"area"_s ).toString() == u"yes"_s )
      return true;
    for ( const QString &key : { u"building"_s, u"landuse"_s, u"leisure"_s, u"amenity"_s, u"shop"_s, u"tourism"_s } )
    {
      if ( tags.contains( key ) )
        return true;
    }
    return tags.contains( u"natural"_s ) && tags.value( u"natural"_s ).toString() != u"coastline"_s;
  }

  QJsonObject overpassGeoJson( const QJsonObject &response, bool centersOnly, int &skipped )
  {
    QJsonArray features;
    skipped = 0;
    for ( const QJsonValue &elementValue : response.value( u"elements"_s ).toArray() )
    {
      const QJsonObject element = elementValue.toObject();
      const QString type = element.value( u"type"_s ).toString();
      const QJsonObject tags = element.value( u"tags"_s ).toObject();
      QJsonObject geometry;

      const QJsonObject center = element.value( u"center"_s ).toObject();
      if ( centersOnly || type == u"node"_s )
      {
        const double longitude = type == u"node"_s ? element.value( u"lon"_s ).toDouble( std::numeric_limits<double>::quiet_NaN() )
                                                   : center.value( u"lon"_s ).toDouble( std::numeric_limits<double>::quiet_NaN() );
        const double latitude = type == u"node"_s ? element.value( u"lat"_s ).toDouble( std::numeric_limits<double>::quiet_NaN() )
                                                  : center.value( u"lat"_s ).toDouble( std::numeric_limits<double>::quiet_NaN() );
        if ( std::isfinite( longitude ) && std::isfinite( latitude ) )
          geometry = QJsonObject{ { u"type"_s, u"Point"_s }, { u"coordinates"_s, QJsonArray{ longitude, latitude } } };
      }
      else if ( type == u"way"_s )
      {
        const QJsonArray coordinates = coordinateArray( element.value( u"geometry"_s ).toArray() );
        if ( coordinates.size() >= 2 )
        {
          const bool closed = coordinates.size() >= 4 && coordinates.first().toArray() == coordinates.last().toArray();
          if ( closed && osmAreaTags( tags ) )
            geometry = QJsonObject{ { u"type"_s, u"Polygon"_s }, { u"coordinates"_s, QJsonArray{ coordinates } } };
          else
            geometry = QJsonObject{ { u"type"_s, u"LineString"_s }, { u"coordinates"_s, coordinates } };
        }
      }
      else if ( type == u"relation"_s )
      {
        QJsonArray lines;
        for ( const QJsonValue &memberValue : element.value( u"members"_s ).toArray() )
        {
          const QJsonArray line = coordinateArray( memberValue.toObject().value( u"geometry"_s ).toArray() );
          if ( line.size() >= 2 )
            lines.append( line );
        }
        if ( !lines.isEmpty() )
          geometry = QJsonObject{ { u"type"_s, u"MultiLineString"_s }, { u"coordinates"_s, lines } };
      }

      if ( geometry.isEmpty() )
      {
        ++skipped;
        continue;
      }
      QJsonObject properties = tags;
      properties.insert( u"osm_id"_s, QString::number( element.value( u"id"_s ).toInteger() ) );
      properties.insert( u"osm_type"_s, type );
      features.append( QJsonObject{
        { u"type"_s, u"Feature"_s },
        { u"id"_s, u"%1/%2"_s.arg( type, QString::number( element.value( u"id"_s ).toInteger() ) ) },
        { u"properties"_s, properties },
        { u"geometry"_s, geometry },
      } );
    }
    return QJsonObject{
      { u"type"_s, u"FeatureCollection"_s },
      { u"name"_s, u"OpenStreetMap Overpass result"_s },
      { u"attribution"_s, u"© OpenStreetMap contributors, ODbL 1.0"_s },
      { u"features"_s, features },
    };
  }

  bool validOverpassSelector( const QString &selector )
  {
    static const QRegularExpression pattern( u"^(?:node|way|relation|nwr)(?:\\[[^\\[\\];{}\\r\\n]+\\])+$"_s );
    return selector.size() <= 300 && pattern.match( selector.trimmed() ).hasMatch();
  }

  QString normalizedStacDatetime( const QString &value )
  {
    static const QRegularExpression datePattern( u"^\\d{4}-\\d{2}-\\d{2}$"_s );
    const QStringList parts = value.split( QLatin1Char( '/' ) );
    if ( parts.size() == 2 )
    {
      const QString start = datePattern.match( parts.at( 0 ) ).hasMatch() ? parts.at( 0 ) + u"T00:00:00Z"_s : parts.at( 0 );
      const QString end = datePattern.match( parts.at( 1 ) ).hasMatch() ? parts.at( 1 ) + u"T23:59:59Z"_s : parts.at( 1 );
      return start + QLatin1Char( '/' ) + end;
    }
    if ( datePattern.match( value ).hasMatch() )
      return value + u"T00:00:00Z/"_s + value + u"T23:59:59Z"_s;
    return value;
  }
}

QJsonObject QgsAgentServer::downloadRemoteFile( const QJsonObject &arguments )
{
  const QUrl url( arguments.value( u"url"_s ).toString() );
  QString validationError;
  if ( !isPublicNetworkUrl( url, validationError ) )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, validationError } };

  QString outputPath = arguments.value( u"output_path"_s ).toString();
  if ( outputPath.isEmpty() )
    outputPath = defaultDownloadPath( url, u"dat"_s );
  if ( !QDir::isAbsolutePath( outputPath ) || QFileInfo::exists( outputPath ) )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, QFileInfo::exists( outputPath ) ? tr( "The output already exists; choose a new path." ) : tr( "An absolute output path is required." ) } };
  if ( !hasSupportedDownloadExtension( outputPath ) )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "The output extension is not in the supported geospatial data allowlist." ) } };

  const qint64 maximumBytes = std::clamp<qint64>(
    static_cast<qint64>( arguments.value( u"max_size_mb"_s ).toInt( 100 ) ) * 1024 * 1024,
    1024 * 1024,
    MAX_DOWNLOAD_LIMIT
  );
  const bool load = arguments.value( u"load"_s ).toBool( true );
  if ( !confirmWriteAction(
         tr( "Download remote GIS data?" ),
         tr( "Source: %1\nOutput: %2\nMaximum size: %3 MB\nLoad into project: %4" )
           .arg( url.toDisplayString(), outputPath )
           .arg( maximumBytes / 1024 / 1024 )
           .arg( load ? tr( "Yes" ) : tr( "No" ) )
       ) )
    return QJsonObject{ { u"ok"_s, false }, { u"cancelled"_s, true } };

  const NetworkResponse response = requestPublicUrl(
    url,
    "GET",
    QByteArray(),
    QByteArray(),
    maximumBytes,
    std::clamp( arguments.value( u"timeout_seconds"_s ).toInt( 120 ), 10, 600 )
  );
  if ( !response.ok )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, response.error }, { u"status_code"_s, response.statusCode } };

  const QString expectedHash = arguments.value( u"sha256"_s ).toString().trimmed().toLower();
  const QString actualHash = QString::fromLatin1( QCryptographicHash::hash( response.data, QCryptographicHash::Sha256 ).toHex() );
  if ( !expectedHash.isEmpty() && expectedHash != actualHash )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "The downloaded file did not match the requested SHA-256 checksum." ) }, { u"sha256"_s, actualHash } };

  QString writeError;
  if ( !writeNewFile( outputPath, response.data, writeError ) )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, writeError } };
  emit fileCreated( outputPath );

  QJsonObject result{
    { u"ok"_s, true },
    { u"path"_s, outputPath },
    { u"source_url"_s, response.finalUrl.toString() },
    { u"bytes"_s, response.data.size() },
    { u"content_type"_s, response.contentType },
    { u"sha256"_s, actualHash },
  };
  if ( load )
  {
    QJsonObject loadArguments{
      { u"path"_s, outputPath },
      { u"name"_s, arguments.value( u"name"_s ).toString() },
      { u"type"_s, arguments.value( u"type"_s ).toString( u"auto"_s ) },
    };
    const QJsonObject loadResult = loadLayer( loadArguments );
    result.insert( u"load_result"_s, loadResult );
    result.insert( u"ok"_s, loadResult.value( u"ok"_s ).toBool() );
  }
  return result;
}

QJsonObject QgsAgentServer::queryOverpass( const QJsonObject &arguments )
{
  const QJsonArray bbox = arguments.value( u"bbox"_s ).toArray();
  if ( bbox.size() != 4 )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "bbox must be [south, west, north, east]." ) } };
  const double south = bbox.at( 0 ).toDouble();
  const double west = bbox.at( 1 ).toDouble();
  const double north = bbox.at( 2 ).toDouble();
  const double east = bbox.at( 3 ).toDouble();
  if ( south < -90 || north > 90 || west < -180 || east > 180 || south >= north || west >= east )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "The bounding box is invalid." ) } };
  if ( north - south > 5 || east - west > 5 )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "The Overpass bounding box is too large. Split it into areas no wider or taller than 5 degrees." ) } };

  QStringList selectors;
  for ( const QJsonValue &value : arguments.value( u"selectors"_s ).toArray() )
  {
    const QString selector = value.toString().trimmed();
    if ( !validOverpassSelector( selector ) )
      return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Invalid Overpass selector: %1" ).arg( selector ) } };
    selectors.append( selector );
  }
  if ( selectors.isEmpty() || selectors.size() > 20 )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Provide between 1 and 20 Overpass selectors." ) } };

  const QString geometryMode = arguments.value( u"geometry_mode"_s ).toString( u"center"_s );
  if ( geometryMode != u"center"_s && geometryMode != u"geometry"_s )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "geometry_mode must be center or geometry." ) } };
  const int timeoutSeconds = std::clamp( arguments.value( u"timeout_seconds"_s ).toInt( 90 ), 10, 180 );
  const QString bboxText = u"%1,%2,%3,%4"_s.arg( south, 0, 'f', 8 ).arg( west, 0, 'f', 8 ).arg( north, 0, 'f', 8 ).arg( east, 0, 'f', 8 );
  QStringList statements;
  for ( const QString &selector : std::as_const( selectors ) )
    statements.append( u"%1(%2);"_s.arg( selector, bboxText ) );
  const QString query = u"[out:json][timeout:%1];(%2);out body %3;"_s
                          .arg( timeoutSeconds )
                          .arg( statements.join( QString() ), geometryMode == u"center"_s ? u"center"_s : u"geom"_s );
  QString outputPath = arguments.value( u"output_path"_s ).toString();
  if ( outputPath.isEmpty() )
    outputPath = defaultDownloadPath( QUrl( u"https://overpass-api.de/osm_%1.geojson"_s.arg( QDateTime::currentDateTime().toString( u"yyyyMMdd_HHmmss"_s ) ) ), u"geojson"_s );
  if ( !outputPath.toLower().endsWith( u".geojson"_s ) || !QDir::isAbsolutePath( outputPath ) || QFileInfo::exists( outputPath ) )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Overpass output must be a new absolute .geojson path." ) } };

  if ( !confirmWriteAction(
         tr( "Download OpenStreetMap data?" ),
         tr( "Source: OpenStreetMap via Overpass API\nBounding box: %1\nSelectors: %2\nOutput: %3\nLicense: ODbL 1.0" ).arg( bboxText, selectors.join( u", "_s ), outputPath )
       ) )
    return QJsonObject{ { u"ok"_s, false }, { u"cancelled"_s, true } };

  const QByteArray body = "data=" + QUrl::toPercentEncoding( query );
  const NetworkResponse response = requestPublicUrl(
    QUrl( u"https://overpass-api.de/api/interpreter"_s ),
    "POST",
    body,
    "application/x-www-form-urlencoded",
    JSON_RESPONSE_LIMIT,
    timeoutSeconds + 15
  );
  if ( !response.ok )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, response.error }, { u"status_code"_s, response.statusCode } };

  QJsonParseError parseError;
  const QJsonDocument overpassDocument = QJsonDocument::fromJson( response.data, &parseError );
  if ( parseError.error != QJsonParseError::NoError || !overpassDocument.isObject() )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Overpass returned invalid JSON: %1" ).arg( parseError.errorString() ) } };
  int skipped = 0;
  const QJsonObject geoJson = overpassGeoJson( overpassDocument.object(), geometryMode == u"center"_s, skipped );
  const int featureCount = geoJson.value( u"features"_s ).toArray().size();
  if ( featureCount == 0 )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "The Overpass query returned no loadable features." ) }, { u"skipped"_s, skipped } };

  QString writeError;
  if ( !writeNewFile( outputPath, QJsonDocument( geoJson ).toJson( QJsonDocument::Compact ), writeError ) )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, writeError } };
  emit fileCreated( outputPath );
  const QJsonObject loadResult = loadLayer( QJsonObject{
    { u"path"_s, outputPath },
    { u"name"_s, arguments.value( u"name"_s ).toString( u"OpenStreetMap"_s ) },
    { u"type"_s, u"vector"_s },
  } );
  return QJsonObject{
    { u"ok"_s, loadResult.value( u"ok"_s ).toBool() },
    { u"path"_s, outputPath },
    { u"feature_count"_s, featureCount },
    { u"skipped_elements"_s, skipped },
    { u"geometry_mode"_s, geometryMode },
    { u"bbox"_s, bbox },
    { u"selectors"_s, QJsonArray::fromStringList( selectors ) },
    { u"source"_s, u"https://overpass-api.de/api/interpreter"_s },
    { u"attribution"_s, u"© OpenStreetMap contributors"_s },
    { u"license"_s, u"Open Database License (ODbL) 1.0"_s },
    { u"load_result"_s, loadResult },
  };
}

QJsonObject QgsAgentServer::geocodePlace( const QJsonObject &arguments )
{
  const QString queryText = arguments.value( u"query"_s ).toString().trimmed();
  if ( queryText.size() < 2 || queryText.size() > 300 )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Provide a place name between 2 and 300 characters." ) } };
  const QDateTime now = QDateTime::currentDateTimeUtc();
  if ( mLastGeocodeRequest.isValid() && mLastGeocodeRequest.msecsTo( now ) < 1100 )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Nominatim requests are rate limited. Wait at least one second before retrying." ) } };
  mLastGeocodeRequest = now;

  const int limit = std::clamp( arguments.value( u"limit"_s ).toInt( 5 ), 1, 10 );
  QUrl url( u"https://nominatim.openstreetmap.org/search"_s );
  QUrlQuery query;
  query.addQueryItem( u"q"_s, queryText );
  query.addQueryItem( u"format"_s, u"jsonv2"_s );
  query.addQueryItem( u"addressdetails"_s, u"1"_s );
  query.addQueryItem( u"limit"_s, QString::number( limit ) );
  const QString countryCodes = arguments.value( u"country_codes"_s ).toString().trimmed().toLower();
  if ( !countryCodes.isEmpty() )
  {
    static const QRegularExpression countryPattern( u"^[a-z]{2}(?:,[a-z]{2})*$"_s );
    if ( !countryPattern.match( countryCodes ).hasMatch() )
      return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "country_codes must be comma-separated ISO 3166-1 alpha-2 codes." ) } };
    query.addQueryItem( u"countrycodes"_s, countryCodes );
  }
  url.setQuery( query );

  const NetworkResponse response = requestPublicUrl( url, "GET", QByteArray(), QByteArray(), 2LL * 1024 * 1024, 30 );
  if ( !response.ok )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, response.error }, { u"status_code"_s, response.statusCode } };
  QJsonParseError parseError;
  const QJsonDocument document = QJsonDocument::fromJson( response.data, &parseError );
  if ( parseError.error != QJsonParseError::NoError || !document.isArray() )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Nominatim returned invalid JSON: %1" ).arg( parseError.errorString() ) } };

  QJsonArray results;
  for ( const QJsonValue &value : document.array() )
  {
    const QJsonObject item = value.toObject();
    QJsonArray outputBbox;
    const QJsonArray sourceBbox = item.value( u"boundingbox"_s ).toArray();
    if ( sourceBbox.size() == 4 )
    {
      // Nominatim: south, north, west, east. Overpass: south, west, north, east.
      outputBbox = QJsonArray{
        sourceBbox.at( 0 ).toString().toDouble(),
        sourceBbox.at( 2 ).toString().toDouble(),
        sourceBbox.at( 1 ).toString().toDouble(),
        sourceBbox.at( 3 ).toString().toDouble(),
      };
    }
    results.append( QJsonObject{
      { u"display_name"_s, item.value( u"display_name"_s ) },
      { u"type"_s, item.value( u"type"_s ) },
      { u"category"_s, item.value( u"category"_s ) },
      { u"latitude"_s, item.value( u"lat"_s ).toString().toDouble() },
      { u"longitude"_s, item.value( u"lon"_s ).toString().toDouble() },
      { u"overpass_bbox"_s, outputBbox },
      { u"address"_s, item.value( u"address"_s ) },
    } );
  }
  return QJsonObject{
    { u"ok"_s, true },
    { u"query"_s, queryText },
    { u"results"_s, results },
    { u"source"_s, u"https://nominatim.openstreetmap.org/"_s },
    { u"attribution"_s, u"© OpenStreetMap contributors"_s },
    { u"license"_s, u"Open Database License (ODbL) 1.0"_s },
  };
}

QJsonObject QgsAgentServer::searchStac( const QJsonObject &arguments )
{
  QUrl apiUrl( arguments.value( u"api_url"_s ).toString( u"https://earth-search.aws.element84.com/v1"_s ) );
  QString validationError;
  if ( !isPublicNetworkUrl( apiUrl, validationError ) )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, validationError } };
  QString path = apiUrl.path();
  if ( path.endsWith( QLatin1Char( '/' ) ) )
    path.chop( 1 );
  if ( !path.endsWith( u"/search"_s ) )
    path += u"/search"_s;
  apiUrl.setPath( path );

  const QJsonArray collections = arguments.value( u"collections"_s ).toArray();
  if ( collections.isEmpty() || collections.size() > 10 )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Provide between 1 and 10 STAC collection IDs." ) } };
  const QJsonArray bbox = arguments.value( u"bbox"_s ).toArray();
  if ( bbox.size() != 4 )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "bbox must be [west, south, east, north]." ) } };
  const double west = bbox.at( 0 ).toDouble();
  const double south = bbox.at( 1 ).toDouble();
  const double east = bbox.at( 2 ).toDouble();
  const double north = bbox.at( 3 ).toDouble();
  if ( west < -180 || east > 180 || south < -90 || north > 90 || west >= east || south >= north || east - west > 10 || north - south > 10 )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "The STAC bounding box is invalid or exceeds 10 degrees in width or height." ) } };

  QJsonObject requestBody{
    { u"collections"_s, collections },
    { u"bbox"_s, bbox },
  };
  const QString datetime = normalizedStacDatetime( arguments.value( u"datetime"_s ).toString() );
  if ( !datetime.isEmpty() )
    requestBody.insert( u"datetime"_s, datetime );
  if ( arguments.value( u"query"_s ).isObject() )
    requestBody.insert( u"query"_s, arguments.value( u"query"_s ) );
  const int limit = std::clamp( arguments.value( u"limit"_s ).toInt( 10 ), 1, 100 );
  requestBody.insert( u"limit"_s, limit );

  const NetworkResponse response = requestPublicUrl(
    apiUrl,
    "POST",
    QJsonDocument( requestBody ).toJson( QJsonDocument::Compact ),
    "application/json",
    JSON_RESPONSE_LIMIT,
    std::clamp( arguments.value( u"timeout_seconds"_s ).toInt( 60 ), 10, 180 )
  );
  if ( !response.ok )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, response.error }, { u"status_code"_s, response.statusCode } };

  QJsonParseError parseError;
  const QJsonDocument document = QJsonDocument::fromJson( response.data, &parseError );
  if ( parseError.error != QJsonParseError::NoError || !document.isObject() )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "The STAC API returned invalid JSON: %1" ).arg( parseError.errorString() ) } };

  const QSet<QString> requestedAssets = [&arguments] {
    QSet<QString> keys;
    for ( const QJsonValue &value : arguments.value( u"asset_keys"_s ).toArray() )
      keys.insert( value.toString() );
    return keys;
  }();
  QJsonArray items;
  for ( const QJsonValue &featureValue : document.object().value( u"features"_s ).toArray() )
  {
    const QJsonObject feature = featureValue.toObject();
    const QJsonObject properties = feature.value( u"properties"_s ).toObject();
    QJsonObject assets;
    const QJsonObject sourceAssets = feature.value( u"assets"_s ).toObject();
    for ( auto it = sourceAssets.constBegin(); it != sourceAssets.constEnd(); ++it )
    {
      if ( !requestedAssets.isEmpty() && !requestedAssets.contains( it.key() ) )
        continue;
      const QJsonObject asset = it.value().toObject();
      const QUrl href( asset.value( u"href"_s ).toString() );
      if ( href.scheme() != u"https"_s && href.scheme() != u"http"_s )
        continue;
      assets.insert( it.key(), QJsonObject{
        { u"href"_s, href.toString() },
        { u"type"_s, asset.value( u"type"_s ) },
        { u"title"_s, asset.value( u"title"_s ) },
        { u"roles"_s, asset.value( u"roles"_s ) },
      } );
    }
    items.append( QJsonObject{
      { u"id"_s, feature.value( u"id"_s ) },
      { u"collection"_s, feature.value( u"collection"_s ) },
      { u"bbox"_s, feature.value( u"bbox"_s ) },
      { u"datetime"_s, properties.value( u"datetime"_s ) },
      { u"start_datetime"_s, properties.value( u"start_datetime"_s ) },
      { u"end_datetime"_s, properties.value( u"end_datetime"_s ) },
      { u"cloud_cover"_s, properties.value( u"eo:cloud_cover"_s ) },
      { u"assets"_s, assets },
    } );
    if ( items.size() >= limit )
      break;
  }
  return QJsonObject{
    { u"ok"_s, true },
    { u"api_url"_s, response.finalUrl.toString() },
    { u"returned"_s, items.size() },
    { u"items"_s, items },
    { u"guidance"_s, tr( "Review each collection and asset license, resolution, CRS, acquisition date, and cloud cover before downloading. Use download_remote_file for the selected public asset." ) },
  };
}
