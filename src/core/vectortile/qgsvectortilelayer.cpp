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
#include "qgsvtpktiles.h"
#include "qgsvectortilebasiclabeling.h"
#include "qgsvectortilebasicrenderer.h"
#include "qgsvectortilelabeling.h"
#include "qgsvectortileloader.h"
#include "qgsvectortileutils.h"
#include "qgsnetworkaccessmanager.h"

#include "qgsdatasourceuri.h"
#include "qgslayermetadataformatter.h"
#include "qgsblockingnetworkrequest.h"
#include "qgsmapboxglstyleconverter.h"
#include "qgsjsonutils.h"
#include "qgspainting.h"
#include "qgsmaplayerfactory.h"
#include "qgsarcgisrestutils.h"
#include "qgsselectioncontext.h"
#include "qgsgeometryengine.h"
#include "qgsvectortilemvtdecoder.h"
#include "qgsrasterlayer.h"

#include <QUrl>
#include <QUrlQuery>

QgsVectorTileLayer::QgsVectorTileLayer( const QString &uri, const QString &baseName, const LayerOptions &options )
  : QgsMapLayer( QgsMapLayerType::VectorTileLayer, baseName )
  , mTransformContext( options.transformContext )
{
  mMatrixSet = QgsVectorTileMatrixSet::fromWebMercator();

  mDataSource = uri;

  setValid( loadDataSource() );

  // set a default renderer
  QgsVectorTileBasicRenderer *renderer = new QgsVectorTileBasicRenderer;
  renderer->setStyles( QgsVectorTileBasicRenderer::simpleStyleWithRandomColors() );
  setRenderer( renderer );

  connect( this, &QgsVectorTileLayer::selectionChanged, this, [ = ] { triggerRepaint(); } );
}

void QgsVectorTileLayer::setDataSourcePrivate( const QString &dataSource, const QString &baseName, const QString &, const QgsDataProvider::ProviderOptions &, QgsDataProvider::ReadFlags )
{
  mDataSource = dataSource;
  mLayerName = baseName;
  mDataProvider.reset();

  setValid( loadDataSource() );
}

bool QgsVectorTileLayer::loadDataSource()
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( mDataSource );

  setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) );

  mSourceType = dsUri.param( QStringLiteral( "type" ) );
  mSourcePath = dsUri.param( QStringLiteral( "url" ) );
  if ( mSourceType == QLatin1String( "xyz" ) && dsUri.param( QStringLiteral( "serviceType" ) ) == QLatin1String( "arcgis" ) )
  {
    if ( !setupArcgisVectorTileServiceConnection( mSourcePath, dsUri ) )
      return false;
  }
  else if ( mSourceType == QLatin1String( "xyz" ) )
  {
    if ( !QgsVectorTileUtils::checkXYZUrlTemplate( mSourcePath ) )
    {
      QgsDebugMsg( QStringLiteral( "Invalid format of URL for XYZ source: " ) + mSourcePath );
      return false;
    }

    // online tiles
    int zMin = 0;
    if ( dsUri.hasParam( QStringLiteral( "zmin" ) ) )
      zMin = dsUri.param( QStringLiteral( "zmin" ) ).toInt();

    int zMax = 14;
    if ( dsUri.hasParam( QStringLiteral( "zmax" ) ) )
      zMax = dsUri.param( QStringLiteral( "zmax" ) ).toInt();

    mMatrixSet = QgsVectorTileMatrixSet::fromWebMercator( zMin, zMax );
    setExtent( QgsRectangle( -20037508.3427892, -20037508.3427892, 20037508.3427892, 20037508.3427892 ) );
  }
  else if ( mSourceType == QLatin1String( "mbtiles" ) )
  {
    QgsMbTiles reader( mSourcePath );
    if ( !reader.open() )
    {
      QgsDebugMsg( QStringLiteral( "failed to open MBTiles file: " ) + mSourcePath );
      return false;
    }

    const QString format = reader.metadataValue( QStringLiteral( "format" ) );
    if ( format != QLatin1String( "pbf" ) )
    {
      QgsDebugMsg( QStringLiteral( "Cannot open MBTiles for vector tiles. Format = " ) + format );
      return false;
    }

    QgsDebugMsgLevel( QStringLiteral( "name: " ) + reader.metadataValue( QStringLiteral( "name" ) ), 2 );
    bool minZoomOk, maxZoomOk;
    const int minZoom = reader.metadataValue( QStringLiteral( "minzoom" ) ).toInt( &minZoomOk );
    const int maxZoom = reader.metadataValue( QStringLiteral( "maxzoom" ) ).toInt( &maxZoomOk );
    if ( minZoomOk )
      mMatrixSet.dropMatricesOutsideZoomRange( minZoom, 99 );
    if ( maxZoomOk )
      mMatrixSet.dropMatricesOutsideZoomRange( 0, maxZoom );
    QgsDebugMsgLevel( QStringLiteral( "zoom range: %1 - %2" ).arg( mMatrixSet.minimumZoom() ).arg( mMatrixSet.maximumZoom() ), 2 );

    QgsRectangle r = reader.extent();
    QgsCoordinateTransform ct( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ),
                               QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ), transformContext() );
    ct.setBallparkTransformsAreAppropriate( true );
    r = ct.transformBoundingBox( r );
    setExtent( r );
  }
  else if ( mSourceType == QLatin1String( "vtpk" ) )
  {
    QgsVtpkTiles reader( mSourcePath );
    if ( !reader.open() )
    {
      QgsDebugMsg( QStringLiteral( "failed to open VTPK file: " ) + mSourcePath );
      return false;
    }

    const QVariantMap metadata = reader.metadata();
    const QString format = metadata.value( QStringLiteral( "tileInfo" ) ).toMap().value( QStringLiteral( "format" ) ).toString();
    if ( format != QLatin1String( "pbf" ) )
    {
      QgsDebugMsg( QStringLiteral( "Cannot open VTPK for vector tiles. Format = " ) + format );
      return false;
    }

    mMatrixSet = reader.matrixSet();
    setCrs( mMatrixSet.crs() );
    setExtent( reader.extent( transformContext() ) );
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "Unknown source type: " ) + mSourceType );
    return false;
  }

  const QgsDataProvider::ProviderOptions providerOptions { mTransformContext };
  const QgsDataProvider::ReadFlags flags;
  mDataProvider.reset( new QgsVectorTileDataProvider( providerOptions, flags ) );
  mProviderKey = mDataProvider->name();

  return true;
}

bool QgsVectorTileLayer::setupArcgisVectorTileServiceConnection( const QString &uri, const QgsDataSourceUri &dataSourceUri )
{
  QUrl url( uri );
  // some services don't default to json format, while others do... so let's explicitly request it!
  // (refs https://github.com/qgis/QGIS/issues/4231)
  QUrlQuery query;
  query.addQueryItem( QStringLiteral( "f" ), QStringLiteral( "pjson" ) );
  url.setQuery( query );

  QNetworkRequest request = QNetworkRequest( url );

  QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsVectorTileLayer" ) )

  QgsBlockingNetworkRequest networkRequest;
  switch ( networkRequest.get( request ) )
  {
    case QgsBlockingNetworkRequest::NoError:
      break;

    case QgsBlockingNetworkRequest::NetworkError:
    case QgsBlockingNetworkRequest::TimeoutError:
    case QgsBlockingNetworkRequest::ServerExceptionError:
      return false;
  }

  const QgsNetworkReplyContent content = networkRequest.reply();
  const QByteArray raw = content.content();

  // Parse data
  QJsonParseError err;
  const QJsonDocument doc = QJsonDocument::fromJson( raw, &err );
  if ( doc.isNull() )
  {
    return false;
  }
  mArcgisLayerConfiguration = doc.object().toVariantMap();
  if ( mArcgisLayerConfiguration.contains( QStringLiteral( "error" ) ) )
  {
    return false;
  }

  mArcgisLayerConfiguration.insert( QStringLiteral( "serviceUri" ), uri );
  mSourcePath = uri + '/' + mArcgisLayerConfiguration.value( QStringLiteral( "tiles" ) ).toList().value( 0 ).toString();
  if ( !QgsVectorTileUtils::checkXYZUrlTemplate( mSourcePath ) )
  {
    QgsDebugMsg( QStringLiteral( "Invalid format of URL for XYZ source: " ) + mSourcePath );
    return false;
  }

  mMatrixSet.fromEsriJson( mArcgisLayerConfiguration );
  setCrs( mMatrixSet.crs() );

  // if hardcoded zoom limits aren't specified, take them from the server
  if ( dataSourceUri.hasParam( QStringLiteral( "zmin" ) ) )
    mMatrixSet.dropMatricesOutsideZoomRange( dataSourceUri.param( QStringLiteral( "zmin" ) ).toInt(), 99 );

  if ( dataSourceUri.hasParam( QStringLiteral( "zmax" ) ) )
    mMatrixSet.dropMatricesOutsideZoomRange( 0, dataSourceUri.param( QStringLiteral( "zmax" ) ).toInt() );

  const QVariantMap fullExtent = mArcgisLayerConfiguration.value( QStringLiteral( "fullExtent" ) ).toMap();
  if ( !fullExtent.isEmpty() )
  {
    const QgsRectangle fullExtentRect(
      fullExtent.value( QStringLiteral( "xmin" ) ).toDouble(),
      fullExtent.value( QStringLiteral( "ymin" ) ).toDouble(),
      fullExtent.value( QStringLiteral( "xmax" ) ).toDouble(),
      fullExtent.value( QStringLiteral( "ymax" ) ).toDouble()
    );

    const QgsCoordinateReferenceSystem fullExtentCrs = QgsArcGisRestUtils::convertSpatialReference( fullExtent.value( QStringLiteral( "spatialReference" ) ).toMap() );
    const QgsCoordinateTransform extentTransform( fullExtentCrs, crs(), transformContext() );
    try
    {
      setExtent( extentTransform.transformBoundingBox( fullExtentRect ) );
    }
    catch ( QgsCsException & )
    {
      QgsDebugMsg( QStringLiteral( "Could not transform layer fullExtent to layer CRS" ) );
    }
  }
  else
  {
    // if no fullExtent specified in JSON, default to web mercator specs full extent
    const QgsCoordinateTransform extentTransform( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ), crs(), transformContext() );
    try
    {
      setExtent( extentTransform.transformBoundingBox( QgsRectangle( -20037508.3427892, -20037508.3427892, 20037508.3427892, 20037508.3427892 ) ) );
    }
    catch ( QgsCsException & )
    {
      QgsDebugMsg( QStringLiteral( "Could not transform layer extent to layer CRS" ) );
    }
  }

  return true;
}

QgsVectorTileLayer::~QgsVectorTileLayer() = default;


QgsVectorTileLayer *QgsVectorTileLayer::clone() const
{
  const QgsVectorTileLayer::LayerOptions options( mTransformContext );
  QgsVectorTileLayer *layer = new QgsVectorTileLayer( source(), name(), options );
  layer->setRenderer( renderer() ? renderer()->clone() : nullptr );
  return layer;
}

QgsDataProvider *QgsVectorTileLayer::dataProvider()
{
  return mDataProvider.get();
}

const QgsDataProvider *QgsVectorTileLayer::dataProvider() const
{
  return mDataProvider.get();
}

QgsMapLayerRenderer *QgsVectorTileLayer::createMapRenderer( QgsRenderContext &rendererContext )
{
  return new QgsVectorTileLayerRenderer( this, rendererContext );
}

bool QgsVectorTileLayer::readXml( const QDomNode &layerNode, QgsReadWriteContext &context )
{
  setValid( loadDataSource() );

  const QDomElement matrixSetElement = layerNode.firstChildElement( QStringLiteral( "matrixSet" ) );
  if ( !matrixSetElement.isNull() )
  {
    mMatrixSet.readXml( matrixSetElement, context );
    setCrs( mMatrixSet.crs() );
  }

  QString errorMsg;
  if ( !readSymbology( layerNode, errorMsg, context ) )
    return false;

  readStyleManager( layerNode );
  return true;
}

bool QgsVectorTileLayer::writeXml( QDomNode &layerNode, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement mapLayerNode = layerNode.toElement();
  mapLayerNode.setAttribute( QStringLiteral( "type" ), QgsMapLayerFactory::typeToString( QgsMapLayerType::VectorTileLayer ) );

  mapLayerNode.appendChild( mMatrixSet.writeXml( doc, context ) );

  // add provider node
  if ( mDataProvider )
  {
    QDomElement provider  = doc.createElement( QStringLiteral( "provider" ) );
    const QDomText providerText = doc.createTextNode( providerType() );
    provider.appendChild( providerText );
    mapLayerNode.appendChild( provider );
  }

  writeStyleManager( layerNode, doc );

  QString errorMsg;
  return writeSymbology( layerNode, doc, errorMsg, context );
}

bool QgsVectorTileLayer::readSymbology( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories )
{
  const QDomElement elem = node.toElement();

  readCommonStyle( elem, context, categories );

  const QDomElement elemRenderer = elem.firstChildElement( QStringLiteral( "renderer" ) );
  if ( elemRenderer.isNull() )
  {
    errorMessage = tr( "Missing <renderer> tag" );
    return false;
  }
  const QString rendererType = elemRenderer.attribute( QStringLiteral( "type" ) );

  if ( categories.testFlag( Symbology ) )
  {
    QgsVectorTileRenderer *r = nullptr;
    if ( rendererType == QLatin1String( "basic" ) )
      r = new QgsVectorTileBasicRenderer;
    else
    {
      errorMessage = tr( "Unknown renderer type: " ) + rendererType;
      return false;
    }

    r->readXml( elemRenderer, context );
    setRenderer( r );
  }

  if ( categories.testFlag( Labeling ) )
  {
    setLabeling( nullptr );
    const QDomElement elemLabeling = elem.firstChildElement( QStringLiteral( "labeling" ) );
    if ( !elemLabeling.isNull() )
    {
      const QString labelingType = elemLabeling.attribute( QStringLiteral( "type" ) );
      QgsVectorTileLabeling *labeling = nullptr;
      if ( labelingType == QLatin1String( "basic" ) )
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
  }

  if ( categories.testFlag( Symbology ) )
  {
    // get and set the blend mode if it exists
    const QDomNode blendModeNode = node.namedItem( QStringLiteral( "blendMode" ) );
    if ( !blendModeNode.isNull() )
    {
      const QDomElement e = blendModeNode.toElement();
      setBlendMode( QgsPainting::getCompositionMode( static_cast< QgsPainting::BlendMode >( e.text().toInt() ) ) );
    }
  }

  // get and set the layer transparency
  if ( categories.testFlag( Rendering ) )
  {
    const QDomNode layerOpacityNode = node.namedItem( QStringLiteral( "layerOpacity" ) );
    if ( !layerOpacityNode.isNull() )
    {
      const QDomElement e = layerOpacityNode.toElement();
      setOpacity( e.text().toDouble() );
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
    if ( categories.testFlag( Symbology ) )
    {
      mRenderer->writeXml( elemRenderer, context );
    }
    elem.appendChild( elemRenderer );
  }

  if ( mLabeling && categories.testFlag( Labeling ) )
  {
    QDomElement elemLabeling = doc.createElement( QStringLiteral( "labeling" ) );
    elemLabeling.setAttribute( QStringLiteral( "type" ), mLabeling->type() );
    mLabeling->writeXml( elemLabeling, context );
    elem.appendChild( elemLabeling );
  }

  if ( categories.testFlag( Symbology ) )
  {
    // add the blend mode field
    QDomElement blendModeElem  = doc.createElement( QStringLiteral( "blendMode" ) );
    const QDomText blendModeText = doc.createTextNode( QString::number( QgsPainting::getBlendModeEnum( blendMode() ) ) );
    blendModeElem.appendChild( blendModeText );
    node.appendChild( blendModeElem );
  }

  // add the layer opacity
  if ( categories.testFlag( Rendering ) )
  {
    QDomElement layerOpacityElem  = doc.createElement( QStringLiteral( "layerOpacity" ) );
    const QDomText layerOpacityText = doc.createTextNode( QString::number( opacity() ) );
    layerOpacityElem.appendChild( layerOpacityText );
    node.appendChild( layerOpacityElem );
  }

  return true;
}

void QgsVectorTileLayer::setTransformContext( const QgsCoordinateTransformContext &transformContext )
{
  if ( mDataProvider )
    mDataProvider->setTransformContext( transformContext );

  mTransformContext = transformContext;
  invalidateWgs84Extent();
}

QString QgsVectorTileLayer::loadDefaultStyle( bool &resultFlag )
{
  QString error;
  QStringList warnings;
  resultFlag = loadDefaultStyle( error, warnings );
  return error;
}

Qgis::MapLayerProperties QgsVectorTileLayer::properties() const
{
  Qgis::MapLayerProperties res;
  if ( mSourceType == QLatin1String( "xyz" ) )
  {
    // always consider xyz vector tiles as basemap layers
    res |= Qgis::MapLayerProperty::IsBasemapLayer;
  }
  else
  {
    // TODO when should we consider mbtiles layers as basemap layers? potentially if their extent is "large"?
  }

  return res;
}

bool QgsVectorTileLayer::loadDefaultStyle( QString &error, QStringList &warnings )
{
  return loadDefaultStyleAndSubLayersPrivate( error, warnings, nullptr );
}

bool QgsVectorTileLayer::loadDefaultStyleAndSubLayers( QString &error, QStringList &warnings, QList<QgsMapLayer *> &subLayers )
{
  return loadDefaultStyleAndSubLayersPrivate( error, warnings, &subLayers );
}

bool QgsVectorTileLayer::loadDefaultStyleAndSubLayersPrivate( QString &error, QStringList &warnings, QList<QgsMapLayer *> *subLayers )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( mDataSource );

  QVariantMap styleDefinition;
  QgsMapBoxGlStyleConversionContext context;
  QString styleUrl;
  if ( !dsUri.param( QStringLiteral( "styleUrl" ) ).isEmpty() )
  {
    styleUrl = dsUri.param( QStringLiteral( "styleUrl" ) );
  }
  else if ( mSourceType == QLatin1String( "xyz" ) && dsUri.param( QStringLiteral( "serviceType" ) ) == QLatin1String( "arcgis" ) )
  {
    // for ArcMap VectorTileServices we default to the defaultStyles URL from the layer configuration
    styleUrl = mArcgisLayerConfiguration.value( QStringLiteral( "serviceUri" ) ).toString()
               + '/' + mArcgisLayerConfiguration.value( QStringLiteral( "defaultStyles" ) ).toString();
  }

  if ( mSourceType == QLatin1String( "vtpk" ) )
  {
    QgsVtpkTiles reader( mSourcePath );
    if ( !reader.open() )
    {
      QgsDebugMsg( QStringLiteral( "failed to open VTPK file: " ) + mSourcePath );
      return false;
    }

    styleDefinition = reader.styleDefinition();

    const QVariantMap spriteDefinition = reader.spriteDefinition();
    if ( !spriteDefinition.isEmpty() )
    {
      const QImage spriteImage = reader.spriteImage();
      context.setSprites( spriteImage, spriteDefinition );
    }
  }
  else if ( !styleUrl.isEmpty() )
  {
    QNetworkRequest request = QNetworkRequest( QUrl( styleUrl ) );

    QgsSetRequestInitiatorClass( request, QStringLiteral( "QgsVectorTileLayer" ) );

    QgsBlockingNetworkRequest networkRequest;
    switch ( networkRequest.get( request ) )
    {
      case QgsBlockingNetworkRequest::NoError:
        break;

      case QgsBlockingNetworkRequest::NetworkError:
      case QgsBlockingNetworkRequest::TimeoutError:
      case QgsBlockingNetworkRequest::ServerExceptionError:
        error = QObject::tr( "Error retrieving default style" );
        return false;
    }

    const QgsNetworkReplyContent content = networkRequest.reply();
    styleDefinition = QgsJsonUtils::parseJson( content.content() ).toMap();

    if ( styleDefinition.contains( QStringLiteral( "sprite" ) ) )
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

  if ( !styleDefinition.isEmpty() )
  {
    // convert automatically from pixel sizes to millimeters, because pixel sizes
    // are a VERY edge case in QGIS and don't play nice with hidpi map renders or print layouts
    context.setTargetUnit( QgsUnitTypes::RenderMillimeters );
    //assume source uses 96 dpi
    context.setPixelSizeConversionFactor( 25.4 / 96.0 );

    QgsMapBoxGlStyleConverter converter;
    if ( converter.convert( styleDefinition, &context ) != QgsMapBoxGlStyleConverter::Success )
    {
      warnings = converter.warnings();
      error = converter.errorMessage();
      return false;
    }

    setRenderer( converter.renderer() );
    setLabeling( converter.labeling() );
    warnings = converter.warnings();

    if ( subLayers )
    {
      *subLayers = converter.createSubLayers();
    }

    return true;
  }
  else
  {
    bool resultFlag = false;
    error = QgsMapLayer::loadDefaultStyle( resultFlag );
    return resultFlag;
  }
}

QString QgsVectorTileLayer::loadDefaultMetadata( bool &resultFlag )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( mDataSource );
  if ( mSourceType == QLatin1String( "xyz" ) && dsUri.param( QStringLiteral( "serviceType" ) ) == QLatin1String( "arcgis" ) )
  {
    // populate default metadata
    QgsLayerMetadata metadata;
    metadata.setIdentifier( mArcgisLayerConfiguration.value( QStringLiteral( "serviceUri" ) ).toString() );
    const QString parentIdentifier = mArcgisLayerConfiguration.value( QStringLiteral( "serviceItemId" ) ).toString();
    if ( !parentIdentifier.isEmpty() )
    {
      metadata.setParentIdentifier( parentIdentifier );
    }
    metadata.setType( QStringLiteral( "dataset" ) );
    metadata.setTitle( mArcgisLayerConfiguration.value( QStringLiteral( "name" ) ).toString() );
    const QString copyright = mArcgisLayerConfiguration.value( QStringLiteral( "copyrightText" ) ).toString();
    if ( !copyright.isEmpty() )
      metadata.setRights( QStringList() << copyright );
    metadata.addLink( QgsAbstractMetadataBase::Link( tr( "Source" ), QStringLiteral( "WWW:LINK" ), mArcgisLayerConfiguration.value( QStringLiteral( "serviceUri" ) ).toString() ) );

    setMetadata( metadata );

    resultFlag = true;
    return QString();
  }
  else if ( mSourceType == QLatin1String( "vtpk" ) )
  {
    QgsVtpkTiles reader( mSourcePath );
    if ( !reader.open() )
    {
      QgsDebugMsg( QStringLiteral( "failed to open VTPK file: " ) + mSourcePath );
      resultFlag = false;
    }
    else
    {
      setMetadata( reader.layerMetadata() );
      resultFlag = true;
    }
    return QString();
  }
  else
  {
    QgsMapLayer::loadDefaultMetadata( resultFlag );
    resultFlag = true;
    return QString();
  }
}

QString QgsVectorTileLayer::encodedSource( const QString &source, const QgsReadWriteContext &context ) const
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( source );

  const QString sourceType = dsUri.param( QStringLiteral( "type" ) );
  QString sourcePath = dsUri.param( QStringLiteral( "url" ) );
  if ( sourceType == QLatin1String( "xyz" ) )
  {
    const QUrl sourceUrl( sourcePath );
    if ( sourceUrl.isLocalFile() )
    {
      // relative path will become "file:./x.txt"
      const QString relSrcUrl = context.pathResolver().writePath( sourceUrl.toLocalFile() );
      dsUri.removeParam( QStringLiteral( "url" ) );  // needed because setParam() would insert second "url" key
      dsUri.setParam( QStringLiteral( "url" ), QUrl::fromLocalFile( relSrcUrl ).toString() );
      return dsUri.encodedUri();
    }
  }
  else if ( sourceType == QLatin1String( "mbtiles" ) )
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

  const QString sourceType = dsUri.param( QStringLiteral( "type" ) );
  QString sourcePath = dsUri.param( QStringLiteral( "url" ) );
  if ( sourceType == QLatin1String( "xyz" ) )
  {
    const QUrl sourceUrl( sourcePath );
    if ( sourceUrl.isLocalFile() )  // file-based URL? convert to relative path
    {
      const QString absSrcUrl = context.pathResolver().readPath( sourceUrl.toLocalFile() );
      dsUri.removeParam( QStringLiteral( "url" ) );  // needed because setParam() would insert second "url" key
      dsUri.setParam( QStringLiteral( "url" ), QUrl::fromLocalFile( absSrcUrl ).toString() );
      return dsUri.encodedUri();
    }
  }
  else if ( sourceType == QLatin1String( "mbtiles" ) )
  {
    sourcePath = context.pathResolver().readPath( sourcePath );
    dsUri.removeParam( QStringLiteral( "url" ) );  // needed because setParam() would insert second "url" key
    dsUri.setParam( QStringLiteral( "url" ), sourcePath );
    return dsUri.encodedUri();
  }

  return source;
}

QString QgsVectorTileLayer::htmlMetadata() const
{
  const QgsLayerMetadataFormatter htmlFormatter( metadata() );

  QString info = QStringLiteral( "<html><head></head>\n<body>\n" );

  info += generalHtmlMetadata();

  info += QStringLiteral( "<h1>" ) + tr( "Information from provider" ) + QStringLiteral( "</h1>\n<hr>\n" ) %
          QStringLiteral( "<table class=\"list-view\">\n" );

  info += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Source type" ) % QStringLiteral( "</td><td>" ) % sourceType() % QStringLiteral( "</td></tr>\n" );

  info += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Zoom levels" ) % QStringLiteral( "</td><td>" ) % QStringLiteral( "%1 - %2" ).arg( sourceMinZoom() ).arg( sourceMaxZoom() ) % QStringLiteral( "</td></tr>\n" );

  info += QLatin1String( "</table>\n<br>" );

  // CRS
  info += crsHtmlMetadata();

  // Identification section
  info += QStringLiteral( "<h1>" ) % tr( "Identification" ) % QStringLiteral( "</h1>\n<hr>\n" ) %
          htmlFormatter.identificationSectionHtml() %
          QStringLiteral( "<br>\n" ) %

          // extent section
          QStringLiteral( "<h1>" ) % tr( "Extent" ) % QStringLiteral( "</h1>\n<hr>\n" ) %
          htmlFormatter.extentSectionHtml( ) %
          QStringLiteral( "<br>\n" ) %

          // Start the Access section
          QStringLiteral( "<h1>" ) % tr( "Access" ) % QStringLiteral( "</h1>\n<hr>\n" ) %
          htmlFormatter.accessSectionHtml( ) %
          QStringLiteral( "<br>\n" ) %


          // Start the contacts section
          QStringLiteral( "<h1>" ) % tr( "Contacts" ) % QStringLiteral( "</h1>\n<hr>\n" ) %
          htmlFormatter.contactsSectionHtml( ) %
          QStringLiteral( "<br><br>\n" ) %

          // Start the links section
          QStringLiteral( "<h1>" ) % tr( "References" ) % QStringLiteral( "</h1>\n<hr>\n" ) %
          htmlFormatter.linksSectionHtml( ) %
          QStringLiteral( "<br>\n" ) %

          // Start the history section
          QStringLiteral( "<h1>" ) % tr( "History" ) % QStringLiteral( "</h1>\n<hr>\n" ) %
          htmlFormatter.historySectionHtml( ) %
          QStringLiteral( "<br>\n" ) %

          QStringLiteral( "\n</body>\n</html>\n" );

  return info;
}

QByteArray QgsVectorTileLayer::getRawTile( QgsTileXYZ tileID )
{
  const QgsTileMatrix tileMatrix = mMatrixSet.tileMatrix( tileID.zoomLevel() );
  const QgsTileRange tileRange( tileID.column(), tileID.column(), tileID.row(), tileID.row() );

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( mDataSource );
  const QString authcfg = dsUri.authConfigId();

  QList<QgsVectorTileRawData> rawTiles = QgsVectorTileLoader::blockingFetchTileRawData( mSourceType, mSourcePath, tileMatrix, QPointF(), tileRange, authcfg, dsUri.httpHeaders() );
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

QList<QgsFeature> QgsVectorTileLayer::selectedFeatures() const
{
  QList< QgsFeature > res;
  res.reserve( mSelectedFeatures.size() );
  for ( auto it = mSelectedFeatures.begin(); it != mSelectedFeatures.end(); ++it )
    res.append( it.value() );

  return res;
}

int QgsVectorTileLayer::selectedFeatureCount() const
{
  return mSelectedFeatures.size();
}

void QgsVectorTileLayer::selectByGeometry( const QgsGeometry &geometry, const QgsSelectionContext &context, Qgis::SelectBehavior behavior, Qgis::SelectGeometryRelationship relationship, Qgis::SelectionFlags flags, QgsRenderContext *renderContext )
{
  if ( !isInScaleRange( context.scale() ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "Out of scale limits" ), 2 );
    return;
  }

  QSet< QgsFeatureId > prevSelection;
  prevSelection.reserve( mSelectedFeatures.size() );
  for ( auto it = mSelectedFeatures.begin(); it != mSelectedFeatures.end(); ++it )
    prevSelection.insert( it.key() );

  if ( !( flags & Qgis::SelectionFlag::ToggleSelection ) )
  {
    switch ( behavior )
    {
      case Qgis::SelectBehavior::SetSelection:
        mSelectedFeatures.clear();
        break;

      case Qgis::SelectBehavior::IntersectSelection:
      case Qgis::SelectBehavior::AddToSelection:
      case Qgis::SelectBehavior::RemoveFromSelection:
        break;
    }
  }

  QgsGeometry selectionGeom = geometry;
  bool isPointOrRectangle;
  QgsPointXY point;
  bool isSinglePoint = selectionGeom.type() == QgsWkbTypes::PointGeometry;
  if ( isSinglePoint )
  {
    isPointOrRectangle = true;
    point = selectionGeom.asPoint();
    relationship = Qgis::SelectGeometryRelationship::Intersect;
  }
  else
  {
    // we have a polygon - maybe it is a rectangle - in such case we can avoid costly instersection tests later
    isPointOrRectangle = QgsGeometry::fromRect( selectionGeom.boundingBox() ).isGeosEqual( selectionGeom );
  }

  auto addDerivedFields = []( QgsFeature & feature, const int tileZoom, const QString & layer )
  {
    QgsFields fields = feature.fields();
    fields.append( QgsField( QStringLiteral( "tile_zoom" ), QVariant::Int ) );
    fields.append( QgsField( QStringLiteral( "tile_layer" ), QVariant::String ) );
    QgsAttributes attributes = feature.attributes();
    attributes << tileZoom << layer;
    feature.setFields( fields );
    feature.setAttributes( attributes );
  };

  std::unique_ptr<QgsGeometryEngine> selectionGeomPrepared;
  QList< QgsFeature > singleSelectCandidates;

  QgsRectangle r;
  if ( isSinglePoint )
  {
    r = QgsRectangle( point.x(), point.y(), point.x(), point.y() );
  }
  else
  {
    r = selectionGeom.boundingBox();

    if ( !isPointOrRectangle || relationship == Qgis::SelectGeometryRelationship::Within )
    {
      // use prepared geometry for faster intersection test
      selectionGeomPrepared.reset( QgsGeometry::createGeometryEngine( selectionGeom.constGet() ) );
    }
  }

  switch ( behavior )
  {
    case Qgis::SelectBehavior::SetSelection:
    case Qgis::SelectBehavior::AddToSelection:
    {
      // when adding to or setting a selection, we retrieve the tile data for the current scale
      const int tileZoom = tileMatrixSet().scaleToZoomLevel( context.scale() );
      const QgsTileMatrix tileMatrix = tileMatrixSet().tileMatrix( tileZoom );
      const QgsTileRange tileRange = tileMatrix.tileRangeFromExtent( r );

      for ( int row = tileRange.startRow(); row <= tileRange.endRow(); ++row )
      {
        for ( int col = tileRange.startColumn(); col <= tileRange.endColumn(); ++col )
        {
          QgsTileXYZ tileID( col, row, tileZoom );
          QByteArray data = getRawTile( tileID );
          if ( data.isEmpty() )
            continue;  // failed to get data

          QgsVectorTileMVTDecoder decoder( tileMatrixSet() );
          if ( !decoder.decode( tileID, data ) )
            continue;  // failed to decode

          QMap<QString, QgsFields> perLayerFields;
          const QStringList layerNames = decoder.layers();
          for ( const QString &layerName : layerNames )
          {
            QSet<QString> fieldNames = qgis::listToSet( decoder.layerFieldNames( layerName ) );
            perLayerFields[layerName] = QgsVectorTileUtils::makeQgisFields( fieldNames );
          }

          const QgsVectorTileFeatures features = decoder.layerFeatures( perLayerFields, QgsCoordinateTransform() );
          const QStringList featuresLayerNames = features.keys();
          for ( const QString &layerName : featuresLayerNames )
          {
            const QgsFields fFields = perLayerFields[layerName];
            const QVector<QgsFeature> &layerFeatures = features[layerName];
            for ( const QgsFeature &f : layerFeatures )
            {
              if ( renderContext && mRenderer && !mRenderer->willRenderFeature( f, tileZoom, layerName, *renderContext ) )
                continue;

              if ( f.geometry().intersects( r ) )
              {
                bool selectFeature = true;
                if ( selectionGeomPrepared )
                {
                  switch ( relationship )
                  {
                    case Qgis::SelectGeometryRelationship::Intersect:
                      selectFeature = selectionGeomPrepared->intersects( f.geometry().constGet() );
                      break;
                    case Qgis::SelectGeometryRelationship::Within:
                      selectFeature = selectionGeomPrepared->contains( f.geometry().constGet() );
                      break;
                  }
                }

                if ( selectFeature )
                {
                  QgsFeature derivedFeature = f;
                  addDerivedFields( derivedFeature, tileZoom, layerName );
                  if ( flags & Qgis::SelectionFlag::SingleFeatureSelection )
                    singleSelectCandidates << derivedFeature;
                  else
                    mSelectedFeatures.insert( derivedFeature.id(), derivedFeature );
                }
              }
            }
          }
        }
      }
      break;
    }

    case Qgis::SelectBehavior::IntersectSelection:
    case Qgis::SelectBehavior::RemoveFromSelection:
    {
      // when removing from the selection, we instead just iterate over the current selection and test against the geometry
      // we do this as we don't want the selection removal operation to depend at all on the tile zoom
      for ( auto it = mSelectedFeatures.begin(); it != mSelectedFeatures.end(); )
      {
        bool matchesGeometry = false;
        if ( selectionGeomPrepared )
        {
          switch ( relationship )
          {
            case Qgis::SelectGeometryRelationship::Intersect:
              matchesGeometry = selectionGeomPrepared->intersects( it->geometry().constGet() );
              break;
            case Qgis::SelectGeometryRelationship::Within:
              matchesGeometry = selectionGeomPrepared->contains( it->geometry().constGet() );
              break;
          }
        }
        else
        {
          switch ( relationship )
          {
            case Qgis::SelectGeometryRelationship::Intersect:
              matchesGeometry = it->geometry().intersects( r );
              break;
            case Qgis::SelectGeometryRelationship::Within:
              matchesGeometry = r.contains( it->geometry().boundingBox() );
              break;
          }
        }

        if ( flags & Qgis::SelectionFlag::SingleFeatureSelection )
        {
          singleSelectCandidates << it.value();
          it++;
        }
        else if ( ( matchesGeometry && behavior == Qgis::SelectBehavior::IntersectSelection )
                  || ( !matchesGeometry && behavior == Qgis::SelectBehavior::RemoveFromSelection ) )
        {
          it++;
        }
        else
        {
          it = mSelectedFeatures.erase( it );
        }
      }
      break;
    }
  }

  if ( ( flags & Qgis::SelectionFlag::SingleFeatureSelection ) && !singleSelectCandidates.empty() )
  {
    QgsFeature bestCandidate;

    if ( flags & Qgis::SelectionFlag::ToggleSelection )
    {
      // when toggling a selection, we first check to see if we can remove a feature from the current selection -- that takes precedence over adding new features to the selection

      // find smallest feature in the current selection
      double smallestArea = std::numeric_limits< double >::max();
      double smallestLength = std::numeric_limits< double >::max();
      for ( const QgsFeature &candidate : std::as_const( singleSelectCandidates ) )
      {
        if ( !mSelectedFeatures.contains( candidate.id() ) )
          continue;

        switch ( candidate.geometry().type() )
        {
          case QgsWkbTypes::PointGeometry:
            bestCandidate = candidate;
            break;
          case QgsWkbTypes::LineGeometry:
          {
            const double length = candidate.geometry().length();
            if ( length < smallestLength && bestCandidate.geometry().type() != QgsWkbTypes::PointGeometry )
            {
              bestCandidate = candidate;
              smallestLength = length;
            }
            break;
          }
          case QgsWkbTypes::PolygonGeometry:
          {
            const double area = candidate.geometry().area();
            if ( area < smallestArea && bestCandidate.geometry().type() != QgsWkbTypes::PointGeometry && bestCandidate.geometry().type() != QgsWkbTypes::LineGeometry )
            {
              bestCandidate = candidate;
              smallestArea = area;
            }
            break;
          }
          case QgsWkbTypes::UnknownGeometry:
          case QgsWkbTypes::NullGeometry:
            break;
        }
      }
    }

    if ( !bestCandidate.isValid() )
    {
      // find smallest feature (ie. pick the "hardest" one to click on)
      double smallestArea = std::numeric_limits< double >::max();
      double smallestLength = std::numeric_limits< double >::max();
      for ( const QgsFeature &candidate : std::as_const( singleSelectCandidates ) )
      {
        switch ( candidate.geometry().type() )
        {
          case QgsWkbTypes::PointGeometry:
            bestCandidate = candidate;
            break;
          case QgsWkbTypes::LineGeometry:
          {
            const double length = candidate.geometry().length();
            if ( length < smallestLength && bestCandidate.geometry().type() != QgsWkbTypes::PointGeometry )
            {
              bestCandidate = candidate;
              smallestLength = length;
            }
            break;
          }
          case QgsWkbTypes::PolygonGeometry:
          {
            const double area = candidate.geometry().area();
            if ( area < smallestArea && bestCandidate.geometry().type() != QgsWkbTypes::PointGeometry && bestCandidate.geometry().type() != QgsWkbTypes::LineGeometry )
            {
              bestCandidate = candidate;
              smallestArea = area;
            }
            break;
          }
          case QgsWkbTypes::UnknownGeometry:
          case QgsWkbTypes::NullGeometry:
            break;
        }
      }
    }

    if ( flags & Qgis::SelectionFlag::ToggleSelection )
    {
      if ( prevSelection.contains( bestCandidate.id() ) )
        mSelectedFeatures.remove( bestCandidate.id() );
      else
        mSelectedFeatures.insert( bestCandidate.id(), bestCandidate );
    }
    else
    {
      switch ( behavior )
      {
        case Qgis::SelectBehavior::SetSelection:
        case Qgis::SelectBehavior::AddToSelection:
          mSelectedFeatures.insert( bestCandidate.id(), bestCandidate );
          break;

        case Qgis::SelectBehavior::IntersectSelection:
        {
          if ( mSelectedFeatures.contains( bestCandidate.id() ) )
          {
            mSelectedFeatures.clear();
            mSelectedFeatures.insert( bestCandidate.id(), bestCandidate );
          }
          else
          {
            mSelectedFeatures.clear();
          }
          break;
        }

        case Qgis::SelectBehavior::RemoveFromSelection:
        {
          mSelectedFeatures.remove( bestCandidate.id() );
          break;
        }
      }
    }
  }

  QSet< QgsFeatureId > newSelection;
  newSelection.reserve( mSelectedFeatures.size() );
  for ( auto it = mSelectedFeatures.begin(); it != mSelectedFeatures.end(); ++it )
    newSelection.insert( it.key() );

  // signal
  if ( prevSelection != newSelection )
    emit selectionChanged();
}

void QgsVectorTileLayer::removeSelection()
{
  if ( mSelectedFeatures.empty() )
    return;

  mSelectedFeatures.clear();
  emit selectionChanged();
}


//
// QgsVectorTileDataProvider
//
///@cond PRIVATE
QgsVectorTileDataProvider::QgsVectorTileDataProvider(
  const ProviderOptions &options,
  QgsDataProvider::ReadFlags flags )
  : QgsDataProvider( QString(), options, flags )
{}

QgsCoordinateReferenceSystem QgsVectorTileDataProvider::crs() const
{
  return QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) );
}

QString QgsVectorTileDataProvider::name() const
{
  return QStringLiteral( "vectortile" );
}

QString QgsVectorTileDataProvider::description() const
{
  return QString();
}

QgsRectangle QgsVectorTileDataProvider::extent() const
{
  return QgsRectangle();
}

bool QgsVectorTileDataProvider::isValid() const
{
  return true;
}

bool QgsVectorTileDataProvider::renderInPreview( const PreviewContext &context )
{
  // Vector tiles by design are very CPU light to render, so we are much more permissive here compared
  // with other layer types. (Generally if a vector tile layer has taken more than a few milliseconds to render it's
  // a result of network requests, and the tile manager class handles these gracefully for us)
  return context.lastRenderingTimeMs <= 1000;
}

///@endcond
