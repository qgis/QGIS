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
#include "qgsvectortilebasiclabeling.h"
#include "qgsvectortilebasicrenderer.h"
#include "qgsvectortilelabeling.h"
#include "qgsvectortileloader.h"
#include "qgsvectortileutils.h"
#include "qgsnetworkaccessmanager.h"
#include "qgssetrequestinitiator_p.h"
#include "qgsdatasourceuri.h"
#include "qgslayermetadataformatter.h"
#include "qgsblockingnetworkrequest.h"
#include "qgsmapboxglstyleconverter.h"
#include "qgsjsonutils.h"
#include "qgspainting.h"
#include "qgsmaplayerfactory.h"
#include "qgsselectioncontext.h"
#include "qgsgeometryengine.h"
#include "qgsvectortilemvtdecoder.h"
#include "qgsthreadingutils.h"
#include "qgsproviderregistry.h"
#include "qgsvectortiledataprovider.h"

#include <QUrl>
#include <QUrlQuery>

QgsVectorTileLayer::QgsVectorTileLayer( const QString &uri, const QString &baseName, const LayerOptions &options )
  : QgsMapLayer( Qgis::LayerType::VectorTile, baseName )
  , mTransformContext( options.transformContext )
{
  mDataSource = uri;

  if ( !uri.isEmpty() )
    setValid( loadDataSource() );

  // set a default renderer
  QgsVectorTileBasicRenderer *renderer = new QgsVectorTileBasicRenderer;
  renderer->setStyles( QgsVectorTileBasicRenderer::simpleStyleWithRandomColors() );
  setRenderer( renderer );

  connect( this, &QgsVectorTileLayer::selectionChanged, this, [this] { triggerRepaint(); } );
}

void QgsVectorTileLayer::setDataSourcePrivate( const QString &dataSource, const QString &baseName, const QString &, const QgsDataProvider::ProviderOptions &, QgsDataProvider::ReadFlags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mDataSource = dataSource;
  mLayerName = baseName;
  mDataProvider.reset();

  setValid( loadDataSource() );
}

bool QgsVectorTileLayer::loadDataSource()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( mDataSource );

  setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3857" ) ) );

  const QgsDataProvider::ProviderOptions providerOptions { mTransformContext };
  const QgsDataProvider::ReadFlags flags;

  mSourceType = dsUri.param( QStringLiteral( "type" ) );
  QString providerKey;
  if ( mSourceType == QLatin1String( "xyz" ) && dsUri.param( QStringLiteral( "serviceType" ) ) == QLatin1String( "arcgis" ) )
  {
    providerKey = QStringLiteral( "arcgisvectortileservice" );
  }
  else if ( mSourceType == QLatin1String( "xyz" ) )
  {
    providerKey = QStringLiteral( "xyzvectortiles" );
  }
  else if ( mSourceType == QLatin1String( "mbtiles" ) )
  {
    providerKey = QStringLiteral( "mbtilesvectortiles" );
  }
  else if ( mSourceType == QLatin1String( "vtpk" ) )
  {
    providerKey = QStringLiteral( "vtpkvectortiles" );
  }
  else
  {
    QgsDebugError( QStringLiteral( "Unknown source type: " ) + mSourceType );
    return false;
  }

  mDataProvider.reset( qobject_cast<QgsVectorTileDataProvider *>( QgsProviderRegistry::instance()->createProvider( providerKey, mDataSource, providerOptions, flags ) ) );
  mProviderKey = mDataProvider->name();

  if ( mDataProvider )
  {
    mMatrixSet = qgis::down_cast< QgsVectorTileDataProvider * >( mDataProvider.get() )->tileMatrixSet();
    setCrs( mDataProvider->crs() );
    setExtent( mDataProvider->extent() );
  }

  return mDataProvider && mDataProvider->isValid();
}

QgsVectorTileLayer::~QgsVectorTileLayer() = default;

QgsVectorTileLayer *QgsVectorTileLayer::clone() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QgsVectorTileLayer::LayerOptions options( mTransformContext );
  QgsVectorTileLayer *layer = new QgsVectorTileLayer( source(), name(), options );
  layer->setRenderer( renderer() ? renderer()->clone() : nullptr );
  return layer;
}

QgsDataProvider *QgsVectorTileLayer::dataProvider()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDataProvider.get();
}

const QgsDataProvider *QgsVectorTileLayer::dataProvider() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDataProvider.get();
}

QgsMapLayerRenderer *QgsVectorTileLayer::createMapRenderer( QgsRenderContext &rendererContext )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return new QgsVectorTileLayerRenderer( this, rendererContext );
}

bool QgsVectorTileLayer::readXml( const QDomNode &layerNode, QgsReadWriteContext &context )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  setValid( loadDataSource() );

  if ( !mDataProvider || !( qobject_cast< QgsVectorTileDataProvider * >( mDataProvider.get() )->providerFlags() & Qgis::VectorTileProviderFlag::AlwaysUseTileMatrixSetFromProvider ) )
  {
    const QDomElement matrixSetElement = layerNode.firstChildElement( QStringLiteral( "matrixSet" ) );
    if ( !matrixSetElement.isNull() )
    {
      mMatrixSet.readXml( matrixSetElement, context );
    }
  }
  setCrs( mMatrixSet.crs() );

  QString errorMsg;
  if ( !readSymbology( layerNode, errorMsg, context ) )
    return false;

  readStyleManager( layerNode );
  return true;
}

bool QgsVectorTileLayer::writeXml( QDomNode &layerNode, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QDomElement mapLayerNode = layerNode.toElement();
  mapLayerNode.setAttribute( QStringLiteral( "type" ), QgsMapLayerFactory::typeToString( Qgis::LayerType::VectorTile ) );

  if ( !mDataProvider || !( qobject_cast< QgsVectorTileDataProvider * >( mDataProvider.get() )->providerFlags() & Qgis::VectorTileProviderFlag::AlwaysUseTileMatrixSetFromProvider ) )
  {
    mapLayerNode.appendChild( mMatrixSet.writeXml( doc, context ) );
  }

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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

      if ( elemLabeling.hasAttribute( QStringLiteral( "labelsEnabled" ) ) )
        mLabelsEnabled = elemLabeling.attribute( QStringLiteral( "labelsEnabled" ) ).toInt();
      else
        mLabelsEnabled = true;

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
      setBlendMode( QgsPainting::getCompositionMode( static_cast< Qgis::BlendMode >( e.text().toInt() ) ) );
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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
    elemLabeling.setAttribute( QStringLiteral( "labelsEnabled" ), mLabelsEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
    mLabeling->writeXml( elemLabeling, context );
    elem.appendChild( elemLabeling );
  }

  if ( categories.testFlag( Symbology ) )
  {
    // add the blend mode field
    QDomElement blendModeElem  = doc.createElement( QStringLiteral( "blendMode" ) );
    const QDomText blendModeText = doc.createTextNode( QString::number( static_cast< int >( QgsPainting::getBlendModeEnum( blendMode() ) ) ) );
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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mDataProvider )
    mDataProvider->setTransformContext( transformContext );

  mTransformContext = transformContext;
  invalidateWgs84Extent();
}

QString QgsVectorTileLayer::loadDefaultStyle( bool &resultFlag )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QString error;
  QStringList warnings;
  resultFlag = loadDefaultStyle( error, warnings );
  return error;
}

Qgis::MapLayerProperties QgsVectorTileLayer::properties() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return loadDefaultStyleAndSubLayersPrivate( error, warnings, nullptr );
}

bool QgsVectorTileLayer::loadDefaultStyleAndSubLayers( QString &error, QStringList &warnings, QList<QgsMapLayer *> &subLayers )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return loadDefaultStyleAndSubLayersPrivate( error, warnings, &subLayers );
}

bool QgsVectorTileLayer::loadDefaultStyleAndSubLayersPrivate( QString &error, QStringList &warnings, QList<QgsMapLayer *> *subLayers )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  QgsVectorTileDataProvider *vtProvider = qgis::down_cast< QgsVectorTileDataProvider *> ( mDataProvider.get() );
  if ( !vtProvider )
    return false;

  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( mDataSource );

  QVariantMap styleDefinition;
  QgsMapBoxGlStyleConversionContext context;
  QString styleUrl;
  if ( !dsUri.param( QStringLiteral( "styleUrl" ) ).isEmpty() )
  {
    styleUrl = dsUri.param( QStringLiteral( "styleUrl" ) );
  }
  else
  {
    styleUrl = vtProvider->styleUrl();
  }

  styleDefinition = vtProvider->styleDefinition();
  const QVariantMap spriteDefinition = vtProvider->spriteDefinition();
  if ( !spriteDefinition.isEmpty() )
  {
    const QImage spriteImage = vtProvider->spriteImage();
    context.setSprites( spriteImage, spriteDefinition );
  }

  if ( !styleDefinition.isEmpty() || !styleUrl.isEmpty() )
  {
    if ( styleDefinition.isEmpty() )
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
    }

    QgsVectorTileUtils::loadSprites( styleDefinition, context, styleUrl );
  }

  if ( !styleDefinition.isEmpty() )
  {
    // convert automatically from pixel sizes to millimeters, because pixel sizes
    // are a VERY edge case in QGIS and don't play nice with hidpi map renders or print layouts
    context.setTargetUnit( Qgis::RenderUnit::Millimeters );
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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  resultFlag = false;
  if ( !mDataProvider || !mDataProvider->isValid() )
    return QString();

  if ( qgis::down_cast< QgsVectorTileDataProvider * >( mDataProvider.get() )->providerCapabilities() & Qgis::VectorTileProviderCapability::ReadLayerMetadata )
  {
    setMetadata( mDataProvider->layerMetadata() );
  }
  else
  {
    QgsMapLayer::loadDefaultMetadata( resultFlag );
  }
  resultFlag = true;
  return QString();
}

QString QgsVectorTileLayer::encodedSource( const QString &source, const QgsReadWriteContext &context ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsProviderRegistry::instance()->absoluteToRelativeUri( mProviderKey, source, context );
}

QString QgsVectorTileLayer::decodedSource( const QString &source, const QString &provider, const QgsReadWriteContext &context ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsProviderRegistry::instance()->relativeToAbsoluteUri( provider, source, context );
}

QString QgsVectorTileLayer::htmlMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QgsLayerMetadataFormatter htmlFormatter( metadata() );

  QString info = QStringLiteral( "<html><head></head>\n<body>\n" );

  info += generalHtmlMetadata();

  info += QStringLiteral( "<h1>" ) + tr( "Information from provider" ) + QStringLiteral( "</h1>\n<hr>\n" ) %
          QStringLiteral( "<table class=\"list-view\">\n" );

  info += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Source type" ) % QStringLiteral( "</td><td>" ) % sourceType() % QStringLiteral( "</td></tr>\n" );

  info += QStringLiteral( "<tr><td class=\"highlight\">" ) % tr( "Zoom levels" ) % QStringLiteral( "</td><td>" ) % QStringLiteral( "%1 - %2" ).arg( sourceMinZoom() ).arg( sourceMaxZoom() ) % QStringLiteral( "</td></tr>\n" );

  if ( mDataProvider )
    info += qobject_cast< const QgsVectorTileDataProvider * >( mDataProvider.get() )->htmlMetadata();

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

QString QgsVectorTileLayer::sourcePath() const
{
  if ( QgsVectorTileDataProvider *vtProvider = qobject_cast< QgsVectorTileDataProvider * >( mDataProvider.get() ) )
    return vtProvider->sourcePath();

  return QString();
}

QgsVectorTileRawData QgsVectorTileLayer::getRawTile( QgsTileXYZ tileID )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsVectorTileDataProvider *vtProvider = qobject_cast< QgsVectorTileDataProvider * >( mDataProvider.get() );
  if ( !vtProvider )
    return QgsVectorTileRawData();

  return vtProvider->readTile( mMatrixSet, tileID );
}

void QgsVectorTileLayer::setRenderer( QgsVectorTileRenderer *r )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mRenderer.reset( r );
  triggerRepaint();
}

QgsVectorTileRenderer *QgsVectorTileLayer::renderer() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mRenderer.get();
}

void QgsVectorTileLayer::setLabeling( QgsVectorTileLabeling *labeling )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mLabeling.reset( labeling );
  triggerRepaint();
}

QgsVectorTileLabeling *QgsVectorTileLayer::labeling() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mLabeling.get();
}

bool QgsVectorTileLayer::labelsEnabled() const
{
  // non fatal for now -- the "rasterize" processing algorithm is not thread safe and calls this
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mLabelsEnabled && static_cast< bool >( mLabeling );
}

void QgsVectorTileLayer::setLabelsEnabled( bool enabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mLabelsEnabled = enabled;
}

QList<QgsFeature> QgsVectorTileLayer::selectedFeatures() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QList< QgsFeature > res;
  res.reserve( mSelectedFeatures.size() );
  for ( auto it = mSelectedFeatures.begin(); it != mSelectedFeatures.end(); ++it )
    res.append( it.value() );

  return res;
}

int QgsVectorTileLayer::selectedFeatureCount() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mSelectedFeatures.size();
}

void QgsVectorTileLayer::selectByGeometry( const QgsGeometry &geometry, const QgsSelectionContext &context, Qgis::SelectBehavior behavior, Qgis::SelectGeometryRelationship relationship, Qgis::SelectionFlags flags, QgsRenderContext *renderContext )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  bool isSinglePoint = selectionGeom.type() == Qgis::GeometryType::Point;
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
      const QVector< QgsTileXYZ> tiles = tileMatrixSet().tilesInRange( tileRange, tileZoom );

      for ( const QgsTileXYZ &tileID : tiles )
      {
        const QgsVectorTileRawData data = getRawTile( tileID );
        if ( data.data.isEmpty() )
          continue;  // failed to get data

        QgsVectorTileMVTDecoder decoder( tileMatrixSet() );
        if ( !decoder.decode( data ) )
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
            if ( renderContext && mRenderer && !mRenderer->willRenderFeature( f, tileID.zoomLevel(), layerName, *renderContext ) )
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
                addDerivedFields( derivedFeature, tileID.zoomLevel(), layerName );
                if ( flags & Qgis::SelectionFlag::SingleFeatureSelection )
                  singleSelectCandidates << derivedFeature;
                else
                  mSelectedFeatures.insert( derivedFeature.id(), derivedFeature );
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
          case Qgis::GeometryType::Point:
            bestCandidate = candidate;
            break;
          case Qgis::GeometryType::Line:
          {
            const double length = candidate.geometry().length();
            if ( length < smallestLength && bestCandidate.geometry().type() != Qgis::GeometryType::Point )
            {
              bestCandidate = candidate;
              smallestLength = length;
            }
            break;
          }
          case Qgis::GeometryType::Polygon:
          {
            const double area = candidate.geometry().area();
            if ( area < smallestArea && bestCandidate.geometry().type() != Qgis::GeometryType::Point && bestCandidate.geometry().type() != Qgis::GeometryType::Line )
            {
              bestCandidate = candidate;
              smallestArea = area;
            }
            break;
          }
          case Qgis::GeometryType::Unknown:
          case Qgis::GeometryType::Null:
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
          case Qgis::GeometryType::Point:
            bestCandidate = candidate;
            break;
          case Qgis::GeometryType::Line:
          {
            const double length = candidate.geometry().length();
            if ( length < smallestLength && bestCandidate.geometry().type() != Qgis::GeometryType::Point )
            {
              bestCandidate = candidate;
              smallestLength = length;
            }
            break;
          }
          case Qgis::GeometryType::Polygon:
          {
            const double area = candidate.geometry().area();
            if ( area < smallestArea && bestCandidate.geometry().type() != Qgis::GeometryType::Point && bestCandidate.geometry().type() != Qgis::GeometryType::Line )
            {
              bestCandidate = candidate;
              smallestArea = area;
            }
            break;
          }
          case Qgis::GeometryType::Unknown:
          case Qgis::GeometryType::Null:
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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mSelectedFeatures.empty() )
    return;

  mSelectedFeatures.clear();
  emit selectionChanged();
}


