/***************************************************************************
  qgsvectortilelayer.cpp
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

#include "qgsvectortilelayer.h"

#include "qgslogger.h"
#include "qgsvectortilelayerrenderer.h"
#include "qgsmbtiles.h"
#include "qgsvectortilebasiclabeling.h"
#include "qgsvectortilebasicrenderer.h"
#include "qgsvectortilelabeling.h"
#include "qgsvectortileloader.h"
#include "qgsvectortileutils.h"

#include "qgsdatasourceuri.h"


QgsVectorTileLayer::QgsVectorTileLayer( const QString &uri, const QString &baseName )
  : QgsMapLayer( QgsMapLayerType::VectorTileLayer, baseName )
{
  mDataSource = uri;

  mValid = loadDataSource();

  // set a default renderer
  QgsVectorTileBasicRenderer *renderer = new QgsVectorTileBasicRenderer;
  renderer->setStyles( QgsVectorTileBasicRenderer::simpleStyleWithRandomColors() );
  setRenderer( renderer );
}

bool QgsVectorTileLayer::loadDataSource()
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( mDataSource );

  mSourceType = dsUri.param( QStringLiteral( "type" ) );
  mSourcePath = dsUri.param( QStringLiteral( "url" ) );
  if ( mSourceType == QStringLiteral( "xyz" ) )
  {
    if ( !QgsVectorTileUtils::checkXYZUrlTemplate( mSourcePath ) )
    {
      QgsDebugMsg( QStringLiteral( "Invalid format of URL for XYZ source: " ) + mSourcePath );
      return false;
    }

    // online tiles
    mSourceMinZoom = 0;
    mSourceMaxZoom = 14;

    if ( dsUri.hasParam( QStringLiteral( "zmin" ) ) )
      mSourceMinZoom = dsUri.param( QStringLiteral( "zmin" ) ).toInt();
    if ( dsUri.hasParam( QStringLiteral( "zmax" ) ) )
      mSourceMaxZoom = dsUri.param( QStringLiteral( "zmax" ) ).toInt();

    setExtent( QgsRectangle( -20037508.3427892, -20037508.3427892, 20037508.3427892, 20037508.3427892 ) );
  }
  else if ( mSourceType == QStringLiteral( "mbtiles" ) )
  {
    QgsMbTiles reader( mSourcePath );
    if ( !reader.open() )
    {
      QgsDebugMsg( QStringLiteral( "failed to open MBTiles file: " ) + mSourcePath );
      return false;
    }

    QString format = reader.metadataValue( QStringLiteral( "format" ) );
    if ( format != QStringLiteral( "pbf" ) )
    {
      QgsDebugMsg( QStringLiteral( "Cannot open MBTiles for vector tiles. Format = " ) + format );
      return false;
    }

    QgsDebugMsgLevel( QStringLiteral( "name: " ) + reader.metadataValue( QStringLiteral( "name" ) ), 2 );
    bool minZoomOk, maxZoomOk;
    int minZoom = reader.metadataValue( QStringLiteral( "minzoom" ) ).toInt( &minZoomOk );
    int maxZoom = reader.metadataValue( QStringLiteral( "maxzoom" ) ).toInt( &maxZoomOk );
    if ( minZoomOk )
      mSourceMinZoom = minZoom;
    if ( maxZoomOk )
      mSourceMaxZoom = maxZoom;
    QgsDebugMsgLevel( QStringLiteral( "zoom range: %1 - %2" ).arg( mSourceMinZoom ).arg( mSourceMaxZoom ), 2 );

    QgsRectangle r = reader.extent();
    QgsCoordinateTransform ct( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ),
                               QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ), transformContext() );
    r = ct.transformBoundingBox( r );
    setExtent( r );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "Unknown source type: " ) + mSourceType );
    return false;
  }

  setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) );
  return true;
}

QgsVectorTileLayer::~QgsVectorTileLayer() = default;


QgsVectorTileLayer *QgsVectorTileLayer::clone() const
{
  QgsVectorTileLayer *layer = new QgsVectorTileLayer( source(), name() );
  layer->setRenderer( renderer() ? renderer()->clone() : nullptr );
  return layer;
}

QgsMapLayerRenderer *QgsVectorTileLayer::createMapRenderer( QgsRenderContext &rendererContext )
{
  return new QgsVectorTileLayerRenderer( this, rendererContext );
}

bool QgsVectorTileLayer::readXml( const QDomNode &layerNode, QgsReadWriteContext &context )
{
  mValid = loadDataSource();

  QString errorMsg;
  if ( !readSymbology( layerNode, errorMsg, context ) )
    return false;

  readStyleManager( layerNode );
  return true;
}

bool QgsVectorTileLayer::writeXml( QDomNode &layerNode, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement mapLayerNode = layerNode.toElement();
  mapLayerNode.setAttribute( QStringLiteral( "type" ), QStringLiteral( "vector-tile" ) );

  writeStyleManager( layerNode, doc );

  QString errorMsg;
  return writeSymbology( layerNode, doc, errorMsg, context );
}

bool QgsVectorTileLayer::readSymbology( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories )
{
  QDomElement elem = node.toElement();

  readCommonStyle( elem, context, categories );

  QDomElement elemRenderer = elem.firstChildElement( QStringLiteral( "renderer" ) );
  if ( elemRenderer.isNull() )
  {
    errorMessage = tr( "Missing <renderer> tag" );
    return false;
  }
  QString rendererType = elemRenderer.attribute( QStringLiteral( "type" ) );
  QgsVectorTileRenderer *r = nullptr;
  if ( rendererType == QStringLiteral( "basic" ) )
    r = new QgsVectorTileBasicRenderer;
  else
  {
    errorMessage = tr( "Unknown renderer type: " ) + rendererType;
    return false;
  }

  r->readXml( elemRenderer, context );
  setRenderer( r );

  setLabeling( nullptr );
  QDomElement elemLabeling = elem.firstChildElement( QStringLiteral( "labeling" ) );
  if ( !elemLabeling.isNull() )
  {
    QString labelingType = elemLabeling.attribute( QStringLiteral( "type" ) );
    QgsVectorTileLabeling *labeling = nullptr;
    if ( labelingType == QStringLiteral( "basic" ) )
      labeling = new QgsVectorTileBasicLabeling;
    else
    {
      errorMessage = tr( "Unknown labeling type: " ) + rendererType;
    }

    if ( labeling )
    {
      labeling->readXml( elemLabeling, context );
      setLabeling( labeling );
    }
  }

  return true;
}

bool QgsVectorTileLayer::writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  Q_UNUSED( errorMessage )
  QDomElement elem = node.toElement();

  writeCommonStyle( elem, doc, context, categories );

  if ( mRenderer )
  {
    QDomElement elemRenderer = doc.createElement( QStringLiteral( "renderer" ) );
    elemRenderer.setAttribute( QStringLiteral( "type" ), mRenderer->type() );
    mRenderer->writeXml( elemRenderer, context );
    elem.appendChild( elemRenderer );
  }

  if ( mLabeling )
  {
    QDomElement elemLabeling = doc.createElement( QStringLiteral( "labeling" ) );
    elemLabeling.setAttribute( QStringLiteral( "type" ), mLabeling->type() );
    mLabeling->writeXml( elemLabeling, context );
    elem.appendChild( elemLabeling );
  }

  return true;
}

void QgsVectorTileLayer::setTransformContext( const QgsCoordinateTransformContext &transformContext )
{
  Q_UNUSED( transformContext )
}

QString QgsVectorTileLayer::encodedSource( const QString &source, const QgsReadWriteContext &context ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( source );

  QString sourceType = dsUri.param( QStringLiteral( "type" ) );
  QString sourcePath = dsUri.param( QStringLiteral( "url" ) );
  if ( sourceType == QStringLiteral( "xyz" ) )
  {
    QUrl sourceUrl( sourcePath );
    if ( sourceUrl.isLocalFile() )
    {
      // relative path will become "file:./x.txt"
      QString relSrcUrl = context.pathResolver().writePath( sourceUrl.toLocalFile() );
      dsUri.removeParam( QStringLiteral( "url" ) );  // needed because setParam() would insert second "url" key
      dsUri.setParam( QStringLiteral( "url" ), QUrl::fromLocalFile( relSrcUrl ).toString() );
      return dsUri.encodedUri();
    }
  }
  else if ( sourceType == QStringLiteral( "mbtiles" ) )
  {
    sourcePath = context.pathResolver().writePath( sourcePath );
    dsUri.removeParam( QStringLiteral( "url" ) );  // needed because setParam() would insert second "url" key
    dsUri.setParam( QStringLiteral( "url" ), sourcePath );
    return dsUri.encodedUri();
  }

  return source;
}

QString QgsVectorTileLayer::decodedSource( const QString &source, const QString &provider, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( provider )

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( source );

  QString sourceType = dsUri.param( QStringLiteral( "type" ) );
  QString sourcePath = dsUri.param( QStringLiteral( "url" ) );
  if ( sourceType == QStringLiteral( "xyz" ) )
  {
    QUrl sourceUrl( sourcePath );
    if ( sourceUrl.isLocalFile() )  // file-based URL? convert to relative path
    {
      QString absSrcUrl = context.pathResolver().readPath( sourceUrl.toLocalFile() );
      dsUri.removeParam( QStringLiteral( "url" ) );  // needed because setParam() would insert second "url" key
      dsUri.setParam( QStringLiteral( "url" ), QUrl::fromLocalFile( absSrcUrl ).toString() );
      return dsUri.encodedUri();
    }
  }
  else if ( sourceType == QStringLiteral( "mbtiles" ) )
  {
    sourcePath = context.pathResolver().readPath( sourcePath );
    dsUri.removeParam( QStringLiteral( "url" ) );  // needed because setParam() would insert second "url" key
    dsUri.setParam( QStringLiteral( "url" ), sourcePath );
    return dsUri.encodedUri();
  }

  return source;
}

QByteArray QgsVectorTileLayer::getRawTile( QgsTileXYZ tileID )
{
  QgsTileMatrix tileMatrix = QgsTileMatrix::fromWebMercator( tileID.zoomLevel() );
  QgsTileRange tileRange( tileID.column(), tileID.column(), tileID.row(), tileID.row() );
  QList<QgsVectorTileRawData> rawTiles = QgsVectorTileLoader::blockingFetchTileRawData( mSourceType, mSourcePath, tileMatrix, QPointF(), tileRange );
  if ( rawTiles.isEmpty() )
    return QByteArray();
  return rawTiles.first().data;
}

void QgsVectorTileLayer::setRenderer( QgsVectorTileRenderer *r )
{
  mRenderer.reset( r );
  triggerRepaint();
}

QgsVectorTileRenderer *QgsVectorTileLayer::renderer() const
{
  return mRenderer.get();
}

void QgsVectorTileLayer::setLabeling( QgsVectorTileLabeling *labeling )
{
  mLabeling.reset( labeling );
  triggerRepaint();
}

QgsVectorTileLabeling *QgsVectorTileLayer::labeling() const
{
  return mLabeling.get();
}
