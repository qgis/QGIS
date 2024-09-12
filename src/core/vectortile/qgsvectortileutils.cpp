/***************************************************************************
  qgsvectortileutils.cpp
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectortileutils.h"

#include <math.h>

#include <QPolygon>
#include <QJsonDocument>
#include <QJsonArray>

#include "qgscoordinatetransform.h"
#include "qgsgeometrycollection.h"
#include "qgsfields.h"
#include "qgslogger.h"
#include "qgsmaptopixel.h"
#include "qgsrectangle.h"
#include "qgsvectorlayer.h"

#include "qgsvectortileloader.h"
#include "qgsvectortilemvtdecoder.h"
#include "qgsvectortilelayer.h"
#include "qgsvectortilerenderer.h"
#include "qgsmapboxglstyleconverter.h"
#include "qgssetrequestinitiator_p.h"
#include "qgsblockingnetworkrequest.h"
#include "qgsjsonutils.h"
#include "qgsvectortileconnection.h"


void QgsVectorTileUtils::updateUriSources( QString &uri, bool forceUpdate )
{
  QgsVectorTileProviderConnection::Data data = QgsVectorTileProviderConnection::decodedUri( uri );
  if ( forceUpdate || ( data.url.isEmpty() && !data.styleUrl.isEmpty() ) )
  {
    const QMap<QString, QString> sources = QgsVectorTileUtils::parseStyleSourceUrl( data.styleUrl, data.httpHeaders, data.authCfg );
    QMap<QString, QString>::const_iterator it = sources.constBegin();
    int i = 0;
    for ( ; it != sources.constEnd(); ++it )
    {
      i += 1;
      QString urlKey = QStringLiteral( "url" );
      QString nameKey = QStringLiteral( "urlName" );
      if ( i > 1 )
      {
        urlKey.append( QString( "_%1" ).arg( i ) );
        nameKey.append( QString( "_%1" ).arg( i ) );
      }
      uri.append( QString( "&%1=%2" ).arg( nameKey, it.key() ) );
      uri.append( QString( "&%1=%2" ).arg( urlKey, it.value() ) );
    }
  }
}

QMap<QString, QString> QgsVectorTileUtils::parseStyleSourceUrl( const QString &styleUrl, const QgsHttpHeaders &headers, const QString &authCfg )
{
  QNetworkRequest nr;
  nr.setUrl( QUrl( styleUrl ) );
  headers.updateNetworkRequest( nr );

  QgsBlockingNetworkRequest req;
  req.setAuthCfg( authCfg );
  QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr, false );
  if ( errCode != QgsBlockingNetworkRequest::NoError )
  {
    QgsDebugError( QStringLiteral( "Request failed: " ) + styleUrl );
    return QMap<QString, QString>();
  }
  QgsNetworkReplyContent reply = req.reply();

  QJsonParseError err;
  const QJsonDocument doc = QJsonDocument::fromJson( reply.content(), &err );
  if ( doc.isNull() )
  {
    QgsDebugError( QStringLiteral( "Could not load style: %1" ).arg( err.errorString() ) );
  }
  else if ( !doc.isObject() )
  {
    QgsDebugError( QStringLiteral( "Could not read style, JSON object is expected" ) );
  }
  else
  {
    QMap<QString, QString> sources;
    QJsonObject sourcesData = doc.object().value( QStringLiteral( "sources" ) ).toObject();
    if ( sourcesData.count() == 0 )
    {
      QgsDebugError( QStringLiteral( "Could not read sources in the style" ) );
    }
    else
    {
      QJsonObject::const_iterator it = sourcesData.constBegin();
      for ( ; it != sourcesData.constEnd(); ++it )
      {
        const QString sourceName = it.key();
        const QJsonObject sourceData = it.value().toObject();
        if ( sourceData.value( QStringLiteral( "type" ) ).toString() != QLatin1String( "vector" ) )
        {
          // raster layers are handled separately
          // ideally we should handle the sources here also, the same way than for vector
          continue;
        }
        QVariantList tiles;
        if ( sourceData.contains( QStringLiteral( "tiles" ) ) )
        {
          tiles = sourceData["tiles"].toArray().toVariantList();
        }
        else if ( sourceData.contains( QStringLiteral( "url" ) ) )
        {
          tiles = parseStyleSourceContentUrl( sourceData.value( QStringLiteral( "url" ) ).toString(), headers, authCfg );
        }
        else
        {
          QgsDebugError( QStringLiteral( "Could not read source %1" ).arg( sourceName ) );
        }
        if ( tiles.count() == 0 )
        {
          QgsDebugError( QStringLiteral( "Could not read source %1, not tiles found" ).arg( sourceName ) );
        }
        else
        {
          // take a random one from the list
          // we might want to save the alternatives for a fallback later
          sources.insert( sourceName, tiles[rand() % tiles.count()].toString() );
        }
      }
      return sources;
    }
  }
  return QMap<QString, QString>();
}

QVariantList QgsVectorTileUtils::parseStyleSourceContentUrl( const QString &sourceUrl, const QgsHttpHeaders &headers, const QString &authCfg )
{
  QNetworkRequest nr;
  nr.setUrl( QUrl( sourceUrl ) );
  headers.updateNetworkRequest( nr );

  QgsBlockingNetworkRequest req;
  req.setAuthCfg( authCfg );
  QgsBlockingNetworkRequest::ErrorCode errCode = req.get( nr, false );
  if ( errCode != QgsBlockingNetworkRequest::NoError )
  {
    QgsDebugError( QStringLiteral( "Request failed: " ) + sourceUrl );
    return QVariantList();
  }
  QgsNetworkReplyContent reply = req.reply();

  QJsonParseError err;
  const QJsonDocument doc = QJsonDocument::fromJson( reply.content(), &err );
  if ( doc.isNull() )
  {
    QgsDebugError( QStringLiteral( "Could not load style: %1" ).arg( err.errorString() ) );
  }
  else if ( !doc.isObject() )
  {
    QgsDebugError( QStringLiteral( "Could not read style, JSON object is expected" ) );
  }
  else
  {
    return doc.object().value( QStringLiteral( "tiles" ) ).toArray().toVariantList();
  }
  return QVariantList();
}



QPolygon QgsVectorTileUtils::tilePolygon( QgsTileXYZ id, const QgsCoordinateTransform &ct, const QgsTileMatrix &tm, const QgsMapToPixel &mtp )
{
  QgsRectangle r = tm.tileExtent( id );
  QgsPointXY p00a = mtp.transform( ct.transform( r.xMinimum(), r.yMinimum() ) );
  QgsPointXY p11a = mtp.transform( ct.transform( r.xMaximum(), r.yMaximum() ) );
  QgsPointXY p01a = mtp.transform( ct.transform( r.xMinimum(), r.yMaximum() ) );
  QgsPointXY p10a = mtp.transform( ct.transform( r.xMaximum(), r.yMinimum() ) );
  QPolygon path;
  path << p00a.toQPointF().toPoint();
  path << p01a.toQPointF().toPoint();
  path << p11a.toQPointF().toPoint();
  path << p10a.toQPointF().toPoint();
  return path;
}

QgsFields QgsVectorTileUtils::makeQgisFields( const QSet<QString> &flds )
{
  QgsFields fields;
  QStringList fieldsSorted = qgis::setToList( flds );
  std::sort( fieldsSorted.begin(), fieldsSorted.end() );
  for ( const QString &fieldName : std::as_const( fieldsSorted ) )
  {
    fields.append( QgsField( fieldName, QMetaType::Type::QString ) );
  }
  return fields;
}

double QgsVectorTileUtils::scaleToZoom( double mapScale, double z0Scale )
{
  double s0 = z0Scale;
  double tileZoom2 = log( s0 / mapScale ) / log( 2 );
  tileZoom2 -= 1;   // TODO: it seems that map scale is double (is that because of high-dpi screen?)
  return tileZoom2;
}

int QgsVectorTileUtils::scaleToZoomLevel( double mapScale, int sourceMinZoom, int sourceMaxZoom, double z0Scale )
{
  int tileZoom = static_cast<int>( round( scaleToZoom( mapScale, z0Scale ) ) );

  if ( tileZoom < sourceMinZoom )
    tileZoom = sourceMinZoom;
  if ( tileZoom > sourceMaxZoom )
    tileZoom = sourceMaxZoom;

  return tileZoom;
}

QgsVectorLayer *QgsVectorTileUtils::makeVectorLayerForTile( QgsVectorTileLayer *mvt, QgsTileXYZ tileID, const QString &layerName )
{
  QgsVectorTileMVTDecoder decoder( mvt->tileMatrixSet() );
  const QgsVectorTileRawData rawTile = mvt->getRawTile( tileID );
  decoder.decode( rawTile );
  QSet<QString> fieldNames = qgis::listToSet( decoder.layerFieldNames( layerName ) );
  fieldNames << QStringLiteral( "_geom_type" );
  QMap<QString, QgsFields> perLayerFields;
  QgsFields fields = QgsVectorTileUtils::makeQgisFields( fieldNames );
  perLayerFields[layerName] = fields;
  QgsVectorTileFeatures data = decoder.layerFeatures( perLayerFields, QgsCoordinateTransform() );
  QgsFeatureList featuresList = data[layerName].toList();

  // turn all geometries to geom. collections (otherwise they won't be accepted by memory provider)
  for ( int i = 0; i < featuresList.count(); ++i )
  {
    QgsGeometry g = featuresList[i].geometry();
    QgsGeometryCollection *gc = new QgsGeometryCollection;
    const QgsAbstractGeometry *gg = g.constGet();
    if ( const QgsGeometryCollection *ggc = qgsgeometry_cast<const QgsGeometryCollection *>( gg ) )
    {
      for ( int k = 0; k < ggc->numGeometries(); ++k )
        gc->addGeometry( ggc->geometryN( k )->clone() );
    }
    else
      gc->addGeometry( gg->clone() );
    featuresList[i].setGeometry( QgsGeometry( gc ) );
  }

  QgsVectorLayer *vl = new QgsVectorLayer( QStringLiteral( "GeometryCollection" ), layerName, QStringLiteral( "memory" ) );
  vl->dataProvider()->addAttributes( fields.toList() );
  vl->updateFields();
  bool res = vl->dataProvider()->addFeatures( featuresList );
  Q_UNUSED( res )
  Q_ASSERT( res );
  Q_ASSERT( featuresList.count() == vl->featureCount() );
  vl->updateExtents();
  QgsDebugMsgLevel( QStringLiteral( "Layer %1 features %2" ).arg( layerName ).arg( vl->featureCount() ), 2 );
  return vl;
}


QString QgsVectorTileUtils::formatXYZUrlTemplate( const QString &url, QgsTileXYZ tile, const QgsTileMatrix &tileMatrix )
{
  QString turl( url );

  turl.replace( QLatin1String( "{x}" ), QString::number( tile.column() ), Qt::CaseInsensitive );
  if ( turl.contains( QLatin1String( "{-y}" ) ) )
  {
    turl.replace( QLatin1String( "{-y}" ), QString::number( tileMatrix.matrixHeight() - tile.row() - 1 ), Qt::CaseInsensitive );
  }
  else
  {
    turl.replace( QLatin1String( "{y}" ), QString::number( tile.row() ), Qt::CaseInsensitive );
  }
  turl.replace( QLatin1String( "{z}" ), QString::number( tile.zoomLevel() ), Qt::CaseInsensitive );
  return turl;
}

bool QgsVectorTileUtils::checkXYZUrlTemplate( const QString &url )
{
  return url.contains( QStringLiteral( "{x}" ) ) &&
         ( url.contains( QStringLiteral( "{y}" ) ) || url.contains( QStringLiteral( "{-y}" ) ) ) &&
         url.contains( QStringLiteral( "{z}" ) );
}

//! a helper class for ordering tile requests according to the distance from view center
struct LessThanTileRequest
{
  QPointF center;  //!< Center in tile matrix (!) coordinates
  bool operator()( QgsTileXYZ req1, QgsTileXYZ req2 )
  {
    QPointF p1( req1.column() + 0.5, req1.row() + 0.5 );
    QPointF p2( req2.column() + 0.5, req2.row() + 0.5 );
    // using chessboard distance (loading order more natural than euclidean/manhattan distance)
    double d1 = std::max( std::fabs( center.x() - p1.x() ), std::fabs( center.y() - p1.y() ) );
    double d2 = std::max( std::fabs( center.x() - p2.x() ), std::fabs( center.y() - p2.y() ) );
    return d1 < d2;
  }
};

void QgsVectorTileUtils::sortTilesByDistanceFromCenter( QVector<QgsTileXYZ> &tiles, QPointF center )
{
  LessThanTileRequest cmp;
  cmp.center = center;
  std::sort( tiles.begin(), tiles.end(), cmp );
}

void QgsVectorTileUtils::loadSprites( const QVariantMap &styleDefinition, QgsMapBoxGlStyleConversionContext &context, const QString &styleUrl )
{
  if ( styleDefinition.contains( QStringLiteral( "sprite" ) ) && ( context.spriteDefinitions().empty() || context.spriteImage().isNull() ) )
  {
    // retrieve sprite definition
    QString spriteUriBase;
    if ( styleDefinition.value( QStringLiteral( "sprite" ) ).toString().startsWith( QLatin1String( "http" ) ) )
    {
      spriteUriBase = styleDefinition.value( QStringLiteral( "sprite" ) ).toString();
    }
    else
    {
      spriteUriBase = styleUrl + '/' + styleDefinition.value( QStringLiteral( "sprite" ) ).toString();
    }

    for ( int resolution = 2; resolution > 0; resolution-- )
    {
      QUrl spriteUrl = QUrl( spriteUriBase );
      spriteUrl.setPath( spriteUrl.path() + QStringLiteral( "%1.json" ).arg( resolution > 1 ? QStringLiteral( "@%1x" ).arg( resolution ) : QString() ) );
      QNetworkRequest request = QNetworkRequest( spriteUrl );
      QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsVectorTileLayer" ) )
      QgsBlockingNetworkRequest networkRequest;
      switch ( networkRequest.get( request ) )
      {
        case QgsBlockingNetworkRequest::NoError:
        {
          const QgsNetworkReplyContent content = networkRequest.reply();
          const QVariantMap spriteDefinition = QgsJsonUtils::parseJson( content.content() ).toMap();

          // retrieve sprite images
          QUrl spriteUrl = QUrl( spriteUriBase );
          spriteUrl.setPath( spriteUrl.path() + QStringLiteral( "%1.png" ).arg( resolution > 1 ? QStringLiteral( "@%1x" ).arg( resolution ) : QString() ) );
          QNetworkRequest request = QNetworkRequest( spriteUrl );
          QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsVectorTileLayer" ) )
          QgsBlockingNetworkRequest networkRequest;
          switch ( networkRequest.get( request ) )
          {
            case QgsBlockingNetworkRequest::NoError:
            {
              const QgsNetworkReplyContent imageContent = networkRequest.reply();
              const QImage spriteImage( QImage::fromData( imageContent.content() ) );
              context.setSprites( spriteImage, spriteDefinition );
              break;
            }

            case QgsBlockingNetworkRequest::NetworkError:
            case QgsBlockingNetworkRequest::TimeoutError:
            case QgsBlockingNetworkRequest::ServerExceptionError:
              break;
          }

          break;
        }

        case QgsBlockingNetworkRequest::NetworkError:
        case QgsBlockingNetworkRequest::TimeoutError:
        case QgsBlockingNetworkRequest::ServerExceptionError:
          break;
      }

      if ( !context.spriteDefinitions().isEmpty() )
        break;
    }
  }
}

