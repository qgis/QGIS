/***************************************************************************
                         qgstiledscenelayer.cpp
                         --------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstiledscenelayer.h"
#include "moc_qgstiledscenelayer.cpp"
#include "qgsthreadingutils.h"
#include "qgsmaplayerfactory.h"
#include "qgspainting.h"
#include "qgsproviderregistry.h"
#include "qgslayermetadataformatter.h"
#include "qgstiledscenerenderer.h"
#include "qgsxmlutils.h"
#include "qgsruntimeprofiler.h"
#include "qgsapplication.h"
#include "qgstiledscenelayerrenderer.h"
#include "qgstiledscenerendererregistry.h"
#include "qgstiledscenelayerelevationproperties.h"

QgsTiledSceneLayer::QgsTiledSceneLayer( const QString &uri,
                                        const QString &baseName,
                                        const QString &provider,
                                        const QgsTiledSceneLayer::LayerOptions &options )
  : QgsMapLayer( Qgis::LayerType::TiledScene, baseName, uri )
  , mElevationProperties( new QgsTiledSceneLayerElevationProperties( this ) )
  , mLayerOptions( options )
{
  if ( !uri.isEmpty() && !provider.isEmpty() )
  {
    const QgsDataProvider::ProviderOptions providerOptions { options.transformContext };
    Qgis::DataProviderReadFlags providerFlags;
    if ( options.loadDefaultStyle )
    {
      providerFlags |= Qgis::DataProviderReadFlag::LoadDefaultStyle;
    }
    setDataSource( uri, baseName, provider, providerOptions, providerFlags );
  }

  // TODO: temporary, for removal
  if ( provider == QLatin1String( "test_tiled_scene_provider" ) )
    mValid = true;

#if 0
  setLegend( QgsMapLayerLegend::defaultTiledSceneLegend( this ) );
#endif
}

QgsTiledSceneLayer::~QgsTiledSceneLayer() = default;

QgsTiledSceneLayer *QgsTiledSceneLayer::clone() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsTiledSceneLayer *layer = new QgsTiledSceneLayer( source(), name(), mProviderKey, mLayerOptions );
  QgsMapLayer::clone( layer );

  if ( mRenderer )
    layer->setRenderer( mRenderer->clone() );

  layer->mElevationProperties = mElevationProperties->clone();
  layer->mElevationProperties->setParent( layer );

  layer->mLayerOptions = mLayerOptions;

  return layer;
}

QgsRectangle QgsTiledSceneLayer::extent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !mDataProvider )
    return QgsRectangle();

  return mDataProvider->extent();
}

QString QgsTiledSceneLayer::loadDefaultMetadata( bool &resultFlag )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  resultFlag = false;
  if ( !mDataProvider || !mDataProvider->isValid() )
    return QString();

  if ( qgis::down_cast< QgsTiledSceneDataProvider * >( mDataProvider.get() )->capabilities() & Qgis::TiledSceneProviderCapability::ReadLayerMetadata )
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

QgsMapLayerElevationProperties *QgsTiledSceneLayer::elevationProperties()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mElevationProperties;
}

QgsMapLayerRenderer *QgsTiledSceneLayer::createMapRenderer( QgsRenderContext &context )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return new QgsTiledSceneLayerRenderer( this, context );
}

QgsTiledSceneRenderer *QgsTiledSceneLayer::renderer()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mRenderer.get();
}

const QgsTiledSceneRenderer *QgsTiledSceneLayer::renderer() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mRenderer.get();
}

void QgsTiledSceneLayer::setRenderer( QgsTiledSceneRenderer *renderer )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( renderer == mRenderer.get() )
    return;

  mRenderer.reset( renderer );
  emit rendererChanged();
  emitStyleChanged();
}

QgsTiledSceneDataProvider *QgsTiledSceneLayer::dataProvider()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDataProvider.get();
}

const QgsTiledSceneDataProvider *QgsTiledSceneLayer::dataProvider() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDataProvider.get();
}

bool QgsTiledSceneLayer::readXml( const QDomNode &layerNode, QgsReadWriteContext &context )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  // create provider
  const QDomNode pkeyNode = layerNode.namedItem( QStringLiteral( "provider" ) );
  mProviderKey = pkeyNode.toElement().text();

  if ( !( mReadFlags & QgsMapLayer::FlagDontResolveLayers ) )
  {
    const QgsDataProvider::ProviderOptions providerOptions { context.transformContext() };
    Qgis::DataProviderReadFlags flags = providerReadFlags( layerNode, mReadFlags );
    // read extent
    if ( mReadFlags & QgsMapLayer::FlagReadExtentFromXml )
    {
      const QDomNode extentNode = layerNode.namedItem( QStringLiteral( "extent" ) );
      if ( !extentNode.isNull() )
      {
        // get the extent
        const QgsRectangle mbr = QgsXmlUtils::readRectangle( extentNode.toElement() );

        // store the extent
        setExtent( mbr );
      }
    }

    setDataSource( mDataSource, mLayerName, mProviderKey, providerOptions, flags );
  }

  QString errorMsg;
  if ( !readSymbology( layerNode, errorMsg, context ) )
    return false;

  readStyleManager( layerNode );

  return isValid();
}

bool QgsTiledSceneLayer::writeXml( QDomNode &layerNode, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QDomElement mapLayerNode = layerNode.toElement();
  mapLayerNode.setAttribute( QStringLiteral( "type" ), QgsMapLayerFactory::typeToString( Qgis::LayerType::TiledScene ) );

  {
    QDomElement provider  = doc.createElement( QStringLiteral( "provider" ) );
    const QDomText providerText = doc.createTextNode( providerType() );
    provider.appendChild( providerText );
    layerNode.appendChild( provider );
  }

  writeStyleManager( layerNode, doc );

  QString errorMsg;
  return writeSymbology( layerNode, doc, errorMsg, context );
}

bool QgsTiledSceneLayer::readSymbology( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QDomElement elem = node.toElement();

  readCommonStyle( elem, context, categories );

  readStyle( node, errorMessage, context, categories );

  if ( categories.testFlag( CustomProperties ) )
    readCustomProperties( node, QStringLiteral( "variable" ) );

  return true;
}

bool QgsTiledSceneLayer::readStyle( const QDomNode &node, QString &, QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  bool result = true;

  if ( categories.testFlag( Symbology ) )
  {
    // get and set the blend mode if it exists
    const QDomNode blendModeNode = node.namedItem( QStringLiteral( "blendMode" ) );
    if ( !blendModeNode.isNull() )
    {
      const QDomElement e = blendModeNode.toElement();
      setBlendMode( QgsPainting::getCompositionMode( static_cast< Qgis::BlendMode >( e.text().toInt() ) ) );
    }

    QDomElement rendererElement = node.firstChildElement( QStringLiteral( "renderer" ) );
    if ( !rendererElement.isNull() )
    {
      std::unique_ptr< QgsTiledSceneRenderer > r( QgsTiledSceneRenderer::load( rendererElement, context ) );
      if ( r )
      {
        setRenderer( r.release() );
      }
      else
      {
        result = false;
      }
    }
    // make sure layer has a renderer - if none exists, fallback to a default renderer
    if ( !mRenderer )
    {
      setRenderer( QgsTiledSceneRendererRegistry::defaultRenderer( this ) );
    }
  }

  // get and set the layer transparency and scale visibility if they exists
  if ( categories.testFlag( Rendering ) )
  {
    const QDomNode layerOpacityNode = node.namedItem( QStringLiteral( "layerOpacity" ) );
    if ( !layerOpacityNode.isNull() )
    {
      const QDomElement e = layerOpacityNode.toElement();
      setOpacity( e.text().toDouble() );
    }

    const bool hasScaleBasedVisibiliy { node.attributes().namedItem( QStringLiteral( "hasScaleBasedVisibilityFlag" ) ).nodeValue() == '1' };
    setScaleBasedVisibility( hasScaleBasedVisibiliy );
    bool ok;
    const double maxScale { node.attributes().namedItem( QStringLiteral( "maxScale" ) ).nodeValue().toDouble( &ok ) };
    if ( ok )
    {
      setMaximumScale( maxScale );
    }
    const double minScale { node.attributes().namedItem( QStringLiteral( "minScale" ) ).nodeValue().toDouble( &ok ) };
    if ( ok )
    {
      setMinimumScale( minScale );
    }
  }
  return result;
}

bool QgsTiledSceneLayer::writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage,
    const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( errorMessage )

  QDomElement elem = node.toElement();
  writeCommonStyle( elem, doc, context, categories );

  ( void )writeStyle( node, doc, errorMessage, context, categories );

  return true;
}

bool QgsTiledSceneLayer::writeStyle( QDomNode &node, QDomDocument &doc, QString &, const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QDomElement mapLayerNode = node.toElement();

  //save custom properties
  if ( categories.testFlag( CustomProperties ) )
  {
    writeCustomProperties( node, doc );
  }

  if ( categories.testFlag( Symbology ) )
  {
    // add the blend mode field
    QDomElement blendModeElem  = doc.createElement( QStringLiteral( "blendMode" ) );
    const QDomText blendModeText = doc.createTextNode( QString::number( static_cast< int >( QgsPainting::getBlendModeEnum( blendMode() ) ) ) );
    blendModeElem.appendChild( blendModeText );
    node.appendChild( blendModeElem );

    if ( mRenderer )
    {
      const QDomElement rendererElement = mRenderer->save( doc, context );
      node.appendChild( rendererElement );
    }
  }

  // add the layer opacity and scale visibility
  if ( categories.testFlag( Rendering ) )
  {
    QDomElement layerOpacityElem = doc.createElement( QStringLiteral( "layerOpacity" ) );
    const QDomText layerOpacityText = doc.createTextNode( QString::number( opacity() ) );
    layerOpacityElem.appendChild( layerOpacityText );
    node.appendChild( layerOpacityElem );

    mapLayerNode.setAttribute( QStringLiteral( "hasScaleBasedVisibilityFlag" ), hasScaleBasedVisibility() ? 1 : 0 );
    mapLayerNode.setAttribute( QStringLiteral( "maxScale" ), maximumScale() );
    mapLayerNode.setAttribute( QStringLiteral( "minScale" ), minimumScale() );
  }
  return true;
}

void QgsTiledSceneLayer::setTransformContext( const QgsCoordinateTransformContext &transformContext )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mDataProvider )
    mDataProvider->setTransformContext( transformContext );
  invalidateWgs84Extent();
}

void QgsTiledSceneLayer::setDataSourcePrivate( const QString &dataSource, const QString &baseName, const QString &provider,
    const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  setName( baseName );
  mProviderKey = provider;
  mDataSource = dataSource;

  if ( mPreloadedProvider )
  {
    mDataProvider.reset( qobject_cast< QgsTiledSceneDataProvider * >( mPreloadedProvider.release() ) );
  }
  else
  {
    std::unique_ptr< QgsScopedRuntimeProfile > profile;
    if ( QgsApplication::profiler()->groupIsActive( QStringLiteral( "projectload" ) ) )
      profile = std::make_unique< QgsScopedRuntimeProfile >( tr( "Create %1 provider" ).arg( provider ), QStringLiteral( "projectload" ) );
    mDataProvider.reset( qobject_cast<QgsTiledSceneDataProvider *>( QgsProviderRegistry::instance()->createProvider( provider, dataSource, options, flags ) ) );
  }

  if ( !mDataProvider )
  {
    QgsDebugError( QStringLiteral( "Unable to get tiled scene data provider" ) );
    setValid( false );
    return;
  }

  mDataProvider->setParent( this );
  QgsDebugMsgLevel( QStringLiteral( "Instantiated the tiled scene data provider plugin" ), 2 );

  setValid( mDataProvider->isValid() );
  if ( !isValid() )
  {
    QgsDebugError( QStringLiteral( "Invalid tiled scene provider plugin %1" ).arg( QString( mDataSource.toUtf8() ) ) );
    setError( mDataProvider->error() );
    return;
  }

  // Load initial extent, crs and renderer
  setCrs( mDataProvider->crs() );
  if ( !( flags & Qgis::DataProviderReadFlag::SkipGetExtent ) )
  {
    setExtent( mDataProvider->extent() );
  }

  bool loadDefaultStyleFlag = false;
  if ( flags & Qgis::DataProviderReadFlag::LoadDefaultStyle )
  {
    loadDefaultStyleFlag = true;
  }

  if ( !mRenderer || loadDefaultStyleFlag )
  {
    std::unique_ptr< QgsScopedRuntimeProfile > profile;
    if ( QgsApplication::profiler()->groupIsActive( QStringLiteral( "projectload" ) ) )
      profile = std::make_unique< QgsScopedRuntimeProfile >( tr( "Load layer style" ), QStringLiteral( "projectload" ) );

    bool defaultLoadedFlag = false;

    if ( !defaultLoadedFlag && loadDefaultStyleFlag )
    {
      loadDefaultStyle( defaultLoadedFlag );
    }

    if ( !defaultLoadedFlag )
    {
      // all else failed, create default renderer
      setRenderer( QgsTiledSceneRendererRegistry::defaultRenderer( this ) );
    }
  }
}

QString QgsTiledSceneLayer::encodedSource( const QString &source, const QgsReadWriteContext &context ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsProviderRegistry::instance()->absoluteToRelativeUri( mProviderKey, source, context );
}

QString QgsTiledSceneLayer::decodedSource( const QString &source, const QString &dataProvider, const QgsReadWriteContext &context ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsProviderRegistry::instance()->relativeToAbsoluteUri( dataProvider, source, context );
}

QString QgsTiledSceneLayer::loadDefaultStyle( bool &resultFlag )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsMapLayer::loadDefaultStyle( resultFlag );
}

QString QgsTiledSceneLayer::htmlMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QgsLayerMetadataFormatter htmlFormatter( metadata() );
  QString myMetadata = QStringLiteral( "<html>\n<body>\n" );

  myMetadata += generalHtmlMetadata();

  // Begin Provider section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Information from provider" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += QLatin1String( "<table class=\"list-view\">\n" );

  if ( mDataProvider )
    myMetadata += mDataProvider->htmlMetadata();

  // Extent
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Extent" ) + QStringLiteral( "</td><td>" ) + extent().toString() + QStringLiteral( "</td></tr>\n" );

  myMetadata += QLatin1String( "</table>\n<br><br>" );

  // CRS
  myMetadata += crsHtmlMetadata();

  // identification section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Identification" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.identificationSectionHtml( );
  myMetadata += QLatin1String( "<br><br>\n" );

  // extent section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Extent" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.extentSectionHtml( isSpatial() );
  myMetadata += QLatin1String( "<br><br>\n" );

  // Start the Access section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Access" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.accessSectionHtml( );
  myMetadata += QLatin1String( "<br><br>\n" );

  // Start the contacts section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Contacts" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.contactsSectionHtml( );
  myMetadata += QLatin1String( "<br><br>\n" );

  // Start the links section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Links" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.linksSectionHtml( );
  myMetadata += QLatin1String( "<br><br>\n" );

  // Start the history section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "History" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += htmlFormatter.historySectionHtml( );
  myMetadata += QLatin1String( "<br><br>\n" );

  myMetadata += customPropertyHtmlMetadata();

  myMetadata += QLatin1String( "\n</body>\n</html>\n" );
  return myMetadata;
}

bool QgsTiledSceneLayer::isReadOnly() const
{
  return true;
}
