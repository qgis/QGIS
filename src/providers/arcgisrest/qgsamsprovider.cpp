/***************************************************************************
    qgsamsprovider.cpp - ArcGIS MapServer Raster Provider
     ----------------------------------------------------
    Date                 : Nov 24, 2015
    Copyright            : (C) 2015 by Sandro Mani
    email                : manisandro@gmail.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsamsprovider.h"
#include "qgsarcgisrestutils.h"
#include "qgscrscache.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgsrasteridentifyresult.h"
#include "qgsfeaturestore.h"
#include "qgsgeometry.h"

#include <cstring>
#include <qjson/parser.h>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPainter>
#include <qmath.h>

QgsAmsLegendFetcher::QgsAmsLegendFetcher( QgsAmsProvider *provider )
    : QgsImageFetcher( provider ), mProvider( provider )
{
  mQuery = new QgsArcGisAsyncQuery( this );
  connect( mQuery, SIGNAL( finished() ), this, SLOT( handleFinished() ) );
  connect( mQuery, SIGNAL( failed( QString, QString ) ), this, SLOT( handleError( QString, QString ) ) );
}

void QgsAmsLegendFetcher::start()
{
  // http://resources.arcgis.com/en/help/rest/apiref/mslegend.html
  // http://sampleserver5.arcgisonline.com/arcgis/rest/services/CommunityAddressing/MapServer/legend?f=pjson
  QgsDataSourceURI dataSource( mProvider->dataSourceUri() );
  QUrl queryUrl( dataSource.param( "url" ) + "/legend" );
  queryUrl.addQueryItem( "f", "json" );
  mQuery->start( queryUrl, &mQueryReply );
}

void QgsAmsLegendFetcher::handleError( QString errorTitle, QString errorMsg )
{
  emit error( errorTitle + ": " + errorMsg );
}

void QgsAmsLegendFetcher::handleFinished()
{
  // Parse result
  QJson::Parser parser;
  bool ok = false;
  QVariantMap queryResults = parser.parse( mQueryReply, &ok ).toMap();
  if ( !ok )
  {
    emit error( QString( "Parsing error at line %1: %2" ).arg( parser.errorLine() ).arg( parser.errorString() ) );
  }
  QgsDataSourceURI dataSource( mProvider->dataSourceUri() );
  QList< QPair<QString, QImage> > legendEntries;
  foreach ( const QVariant& result, queryResults["layers"].toList() )
  {
    QVariantMap queryResultMap = result.toMap();
    QString layerId = queryResultMap["layerId"].toString();
    if ( layerId != dataSource.param( "layer" ) && !mProvider->subLayers().contains( layerId ) )
    {
      continue;
    }
    QVariantList legendSymbols = queryResultMap["legend"].toList();
    foreach ( const QVariant& legendEntry, legendSymbols )
    {
      QVariantMap legendEntryMap = legendEntry.toMap();
      QString label = legendEntryMap["label"].toString();
      if ( label.isEmpty() && legendSymbols.size() == 1 )
        label = queryResultMap["layerName"].toString();
      QByteArray imageData = QByteArray::fromBase64( legendEntryMap["imageData"].toByteArray() );
      legendEntries.append( qMakePair( label, QImage::fromData( imageData ) ) );
    }
  }
  if ( !legendEntries.isEmpty() )
  {
    int padding = 5;
    int vpadding = 1;
    int imageSize = 20;
    int textWidth = 175;

    typedef QPair<QString, QImage> LegendEntry_t;
    QSize maxImageSize( 0, 0 );
    foreach ( const LegendEntry_t& legendEntry, legendEntries )
    {
      maxImageSize.setWidth( qMax( maxImageSize.width(), legendEntry.second.width() ) );
      maxImageSize.setHeight( qMax( maxImageSize.height(), legendEntry.second.height() ) );
    }
    double scaleFactor = maxImageSize.width() == 0 || maxImageSize.height() == 0 ? 1.0 :
                         qMin( 1., qMin( double( imageSize ) / maxImageSize.width(), double( imageSize ) / maxImageSize.height() ) );

    mLegendImage = QImage( imageSize + padding + textWidth, vpadding + legendEntries.size() * ( imageSize + vpadding ), QImage::Format_ARGB32 );
    mLegendImage.fill( Qt::transparent );
    QPainter painter( &mLegendImage );
    int i = 0;
    foreach ( const LegendEntry_t& legendEntry, legendEntries )
    {
      QImage symbol = legendEntry.second.scaled( legendEntry.second.width() * scaleFactor, legendEntry.second.height() * scaleFactor, Qt::KeepAspectRatio, Qt::SmoothTransformation );
      painter.drawImage( 0, vpadding + i * ( imageSize + vpadding ) + ( imageSize - symbol.height() ), symbol );
      painter.drawText( imageSize + padding, vpadding + i * ( imageSize + vpadding ), textWidth, imageSize, Qt::AlignLeft | Qt::AlignVCenter, legendEntry.first );
      ++i;
    }
  }
  emit finish( mLegendImage );
}

///////////////////////////////////////////////////////////////////////////////

QgsAmsProvider::QgsAmsProvider( const QString & uri )
    : QgsRasterDataProvider( uri ), mValid( false )
{
  mLegendFetcher = new QgsAmsLegendFetcher( this );

  QgsDataSourceURI dataSource( dataSourceUri() );
  mServiceInfo = QgsArcGisRestUtils::getServiceInfo( dataSource.param( "url" ), mErrorTitle, mError );
  mLayerInfo = QgsArcGisRestUtils::getLayerInfo( dataSource.param( "url" ) + "/" + dataSource.param( "layer" ), mErrorTitle, mError );

  QVariantMap extentData = mLayerInfo["extent"].toMap();
  mExtent.setXMinimum( extentData["xmin"].toDouble() );
  mExtent.setYMinimum( extentData["ymin"].toDouble() );
  mExtent.setXMaximum( extentData["xmax"].toDouble() );
  mExtent.setYMaximum( extentData["ymax"].toDouble() );
  mCrs = QgsArcGisRestUtils::parseSpatialReference( extentData["spatialReference"].toMap() );
  if ( !mCrs.isValid() )
  {
    appendError( QgsErrorMessage( tr( "Could not parse spatial reference" ), "AMSProvider" ) );
    return;
  }
  foreach ( const QVariant& sublayer, mLayerInfo["subLayers"].toList() )
  {
    mSubLayers.append( sublayer.toMap()["id"].toString() );
    mSubLayerVisibilities.append( true );
  }

  mTimestamp = QDateTime::currentDateTime();
  mValid = true;
}

QStringList QgsAmsProvider::subLayerStyles() const
{
  QStringList styles;
  for ( int i = 0, n = mSubLayers.size(); i < n; ++i )
  {
    styles.append( QString() );
  }
  return styles;
}

void QgsAmsProvider::setLayerOrder( const QStringList &layers )
{
  QStringList oldSubLayers = mSubLayers;
  QList<bool> oldSubLayerVisibilities = mSubLayerVisibilities;
  mSubLayers.clear();
  mSubLayerVisibilities.clear();
  foreach ( const QString& layer, layers )
  {
    // Search for match
    for ( int i = 0, n = oldSubLayers.size(); i < n; ++i )
    {
      if ( oldSubLayers[i] == layer )
      {
        mSubLayers.append( layer );
        oldSubLayers.removeAt( i );
        mSubLayerVisibilities.append( oldSubLayerVisibilities[i] );
        oldSubLayerVisibilities.removeAt( i );
        break;
      }
    }
  }
  // Add remaining at bottom
  mSubLayers.append( oldSubLayers );
  mSubLayerVisibilities.append( oldSubLayerVisibilities );
}

void QgsAmsProvider::setSubLayerVisibility( const QString &name, bool vis )
{
  for ( int i = 0, n = mSubLayers.size(); i < n; ++i )
  {
    if ( mSubLayers[i] == name )
    {
      mSubLayerVisibilities[i] = vis;
      break;
    }
  }
}

void QgsAmsProvider::reloadData()
{
  mCachedImage = QImage();
}

QgsRasterInterface * QgsAmsProvider::clone() const
{
  QgsAmsProvider* provider = new QgsAmsProvider( dataSourceUri() );
  provider->copyBaseSettings( *this );
  return provider;
}

static inline QString dumpVariantMap( const QVariantMap& variantMap, const QString& title = QString() )
{
  QString result = "<table>";
  if ( !title.isEmpty() )
  {
    result += QString( "<tr><td class=\"glossy\" colspan=\"2\">%1</td></tr>" ).arg( title );
  }
  foreach ( const QString& key, variantMap.keys() )
  {
    QVariantMap childMap = variantMap[key].toMap();
    if ( childMap.isEmpty() )
    {
      result += QString( "<tr><td>%1</td><td>%2</td></tr>" ).arg( key ).arg( variantMap[key].toString() );
    }
    else
    {
      result += QString( "<tr><td>%1</td><td>%2</td></tr>" ).arg( key ).arg( dumpVariantMap( childMap ) );
    }
  }
  result += "</table>";
  return result;
}

QString QgsAmsProvider::metadata()
{
  return dumpVariantMap( mServiceInfo, tr( "Service Info" ) ) + dumpVariantMap( mLayerInfo, tr( "Layer Info" ) );
}

QImage* QgsAmsProvider::draw( const QgsRectangle & viewExtent, int pixelWidth, int pixelHeight )
{
  if ( !mCachedImage.isNull() && mCachedImageExtent == viewExtent )
  {
    return &mCachedImage;
  }
  QgsDataSourceURI dataSource( dataSourceUri() );

  // Use of tiles currently only implemented if service CRS is meter based
  if ( mServiceInfo["singleFusedMapCache"].toBool() && mCrs.mapUnits() == QGis::Meters )
  {
    // Compute ideal resolution
    // - Measure distance in meters along lower and upper edge of bounding box
    // - Target resolution is the coarser resolution (resolution = distMeters / pixelWidth)
    double width = viewExtent.xMaximum() - viewExtent.xMinimum();
    double targetRes = width / ( pixelWidth );

    // Tiles available, assemble image from tiles
    QVariantMap tileInfo = mServiceInfo["tileInfo"].toMap();
    int tileWidth = tileInfo["cols"].toInt();
    int tileHeight = tileInfo["rows"].toInt();
    QVariantMap origin = tileInfo["origin"].toMap();
    double ox = origin["x"].toDouble();
    double oy = origin["y"].toDouble();

    // Search matching resolution (tile resolution <= targetRes)
    QList<QVariant> lodEntries = tileInfo["lods"].toList();
    if ( lodEntries.isEmpty() )
    {
      mCachedImage = QImage();
      mCachedImage.fill( Qt::transparent );
      return &mCachedImage;
    }
    int level = 0;
    double resolution = lodEntries.front().toMap()["resolution"].toDouble();
    foreach ( const QVariant& lodEntry, lodEntries )
    {
      QVariantMap lodEntryMap = lodEntry.toMap();
      level = lodEntryMap["level"].toInt();
      resolution = lodEntryMap["resolution"].toDouble();
      if ( lodEntryMap["resolution"].toDouble() <= 1.5*targetRes )
      {
        break;
      }
    }

    // Get necessary tiles to fill extent
    // tile_x = ox + i * (resolution * tileWidth)
    // tile_y = oy - j * (resolution * tileHeight)
    int ixStart = qFloor(( viewExtent.xMinimum() - ox ) / ( tileWidth * resolution ) );
    int iyStart = qFloor(( oy - viewExtent.yMaximum() ) / ( tileHeight * resolution ) );
    int ixEnd = qCeil(( viewExtent.xMaximum() - ox ) / ( tileWidth * resolution ) );
    int iyEnd = qCeil(( oy - viewExtent.yMinimum() ) / ( tileHeight * resolution ) );
    double imX = ( viewExtent.xMinimum() - ox ) / resolution;
    double imY = ( oy - viewExtent.yMaximum() ) / resolution;

    // Query tiles
    int ixCount = ( ixEnd - ixStart + 1 );
    QVector<QByteArray> results( ixCount * ( iyEnd - iyStart + 1 ) );
    QVector<QUrl> queries( ixCount * ( iyEnd - iyStart + 1 ) );
    for ( int iy = iyStart; iy <= iyEnd; ++iy )
    {
      for ( int ix = ixStart; ix <= ixEnd; ++ix )
      {
        queries[( iy - iyStart ) * ixCount + ( ix - ixStart )] = QUrl( dataSource.param( "url" ) + QString( "/tile/%1/%2/%3" ).arg( level ).arg( iy ).arg( ix ) );
      }
    }
    QgsArcGisAsyncParallelQuery query;
    QEventLoop evLoop;
    connect( &query, SIGNAL( finished( QStringList ) ), &evLoop, SLOT( quit() ) );
    query.start( queries, &results, true );
    evLoop.exec( QEventLoop::ExcludeUserInputEvents );

    // Fill image
    mCachedImage = QImage( pixelWidth, pixelHeight, QImage::Format_ARGB32 );
    mCachedImage.fill( Qt::transparent );
    QPainter painter( &mCachedImage );
    painter.setRenderHint( QPainter::SmoothPixmapTransform, true );
    double resScale = resolution / targetRes;
    painter.scale( resScale, resScale );
    for ( int iy = iyStart; iy <= iyEnd; ++iy )
    {
      for ( int ix = ixStart; ix <= ixEnd; ++ix )
      {
        QImage image = QImage::fromData( results[( iy - iyStart ) * ixCount + ( ix - ixStart )], tileInfo["format"].toByteArray() );
        painter.drawImage( QPointF( ix * tileWidth - imX, iy * tileHeight - imY ), image );
      }
    }
  }
  else
  {
    QUrl requestUrl( dataSource.param( "url" ) + "/export" );
    requestUrl.addQueryItem( "bbox", QString( "%1,%2,%3,%4" ).arg( viewExtent.xMinimum(), 0, 'f', -1 ).arg( viewExtent.yMinimum(), 0, 'f', -1 ).arg( viewExtent.xMaximum(), 0, 'f', -1 ).arg( viewExtent.yMaximum(), 0, 'f', -1 ) );
    requestUrl.addQueryItem( "size", QString( "%1,%2" ).arg( pixelWidth ).arg( pixelHeight ) );
    requestUrl.addQueryItem( "format", dataSource.param( "format" ) );
    requestUrl.addQueryItem( "layers", QString( "show:%1" ).arg( dataSource.param( "layer" ) ) );
    requestUrl.addQueryItem( "transparent", "true" );
    requestUrl.addQueryItem( "f", "image" );
    QByteArray reply = QgsArcGisRestUtils::queryService( requestUrl, mErrorTitle, mError );
    mCachedImage = QImage::fromData( reply, dataSource.param( "format" ).toAscii() );
    if ( mCachedImage.format() != QImage::Format_ARGB32 )
    {
      mCachedImage = mCachedImage.convertToFormat( QImage::Format_ARGB32 );
    }
  }
  return &mCachedImage;
}

QImage QgsAmsProvider::getLegendGraphic( double /*scale*/, bool forceRefresh, const QgsRectangle * /*visibleExtent*/ )
{
  if ( mLegendFetcher->haveImage() && !forceRefresh )
  {
    return mLegendFetcher->getImage();
  }
  QEventLoop evLoop;
  connect( mLegendFetcher, SIGNAL( finish( QImage ) ), &evLoop, SLOT( quit() ) );
  connect( mLegendFetcher, SIGNAL( error( QString ) ), &evLoop, SLOT( quit() ) );
  mLegendFetcher->start();
  evLoop.exec( QEventLoop::ExcludeUserInputEvents );
  if ( !mLegendFetcher->errorTitle().isEmpty() )
  {
    mErrorTitle = mLegendFetcher->errorTitle();
    mError = mLegendFetcher->errorMessage();
    return QImage();
  }
  else
  {
    return mLegendFetcher->getImage();
  }
}

QgsImageFetcher* QgsAmsProvider::getLegendGraphicFetcher( const QgsMapSettings* /*mapSettings*/ )
{
  return new QgsAmsLegendFetcher( this );
}

QgsRasterIdentifyResult QgsAmsProvider::identify( const QgsPoint & thePoint, QgsRaster::IdentifyFormat theFormat, const QgsRectangle &theExtent, int theWidth, int theHeight, int theDpi )
{
  // http://resources.arcgis.com/en/help/rest/apiref/identify.html
  QgsDataSourceURI dataSource( dataSourceUri() );
  QUrl queryUrl( dataSource.param( "url" ) + "/identify" );
  queryUrl.addQueryItem( "f", "json" );
  queryUrl.addQueryItem( "geometryType", "esriGeometryPoint" );
  queryUrl.addQueryItem( "geometry", QString( "{x: %1, y: %2}" ).arg( thePoint.x(), 0, 'f' ).arg( thePoint.y(), 0, 'f' ) );
//  queryUrl.addQueryItem( "sr", mCrs.postgisSrid() );
  queryUrl.addQueryItem( "layers", QString( "all:%1" ).arg( dataSource.param( "layer" ) ) );
  queryUrl.addQueryItem( "imageDisplay", QString( "%1,%2,%3" ).arg( theWidth ).arg( theHeight ).arg( theDpi ) );
  queryUrl.addQueryItem( "mapExtent", QString( "%1,%2,%3,%4" ).arg( theExtent.xMinimum(), 0, 'f' ).arg( theExtent.yMinimum(), 0, 'f' ).arg( theExtent.xMaximum(), 0, 'f' ).arg( theExtent.yMaximum(), 0, 'f' ) );
  queryUrl.addQueryItem( "tolerance", "10" );
  QVariantList queryResults = QgsArcGisRestUtils::queryServiceJSON( queryUrl, mErrorTitle, mError )["results"].toList();

  QMap<int, QVariant> entries;

  if ( theFormat == QgsRaster::IdentifyFormatText )
  {
    foreach ( const QVariant& result, queryResults )
    {
      QVariantMap resultMap = result.toMap();
      QVariantMap attributesMap = resultMap["attributes"].toMap();
      QString valueStr;
      foreach ( const QString& attribute, attributesMap.keys() )
      {
        valueStr += QString( "%1 = %2\n" ).arg( attribute ).arg( attributesMap[attribute].toString() );
      }
      entries.insert( entries.size(), valueStr );
    }
  }
  else if ( theFormat == QgsRaster::IdentifyFormatFeature )
  {
    foreach ( const QVariant& result, queryResults )
    {
      QVariantMap resultMap = result.toMap();

      QgsFields fields;
      QVariantMap attributesMap = resultMap["attributes"].toMap();
      QgsAttributes featureAttributes;
      foreach ( const QString& attribute, attributesMap.keys() )
      {
        fields.append( QgsField( attribute, QVariant::String, "string" ) );
        featureAttributes.append( attributesMap[attribute].toString() );
      }
      QgsCoordinateReferenceSystem crs;
      QgsAbstractGeometryV2* geometry = QgsArcGisRestUtils::parseEsriGeoJSON( resultMap["geometry"].toMap(), resultMap["geometryType"].toString(), false, false, &crs );
      QgsFeature feature( fields );
      feature.setGeometry( new QgsGeometry( geometry ) );
      feature.setAttributes( featureAttributes );
      feature.setValid( true );
      QgsFeatureStore store( fields, crs );
      QMap<QString, QVariant> params;
      params["sublayer"] = resultMap["layerName"].toString();
      params["featureType"] = attributesMap[resultMap["displayFieldName"].toString()].toString();
      store.setParams( params );
      store.features().append( feature );
      entries.insert( entries.size(), qVariantFromValue( QList<QgsFeatureStore>() << store ) );
    }
  }
  return QgsRasterIdentifyResult( theFormat, entries );
}

void QgsAmsProvider::readBlock( int /*bandNo*/, const QgsRectangle & viewExtent, int width, int height, void *data )
{
  // TODO: optimize to avoid writing to QImage
  // returned image is actually mCachedImage, no need to delete
  QImage *image = draw( viewExtent, width, height );
  if ( image->width() != width || image->height() != height )
  {
    QgsDebugMsg( "Unexpected image size for block" );
    return;
  }
  std::memcpy( data, image->bits(), image->bytesPerLine() * image->height() );
}
