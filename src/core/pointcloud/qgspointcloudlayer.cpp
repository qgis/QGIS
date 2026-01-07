/***************************************************************************
                         qgspointcloudlayer.cpp
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudlayer.h"

#include "qgsabstractpointcloud3drenderer.h"
#include "qgsapplication.h"
#include "qgseventtracing.h"
#include "qgslayermetadataformatter.h"
#include "qgslogger.h"
#include "qgsmaplayerfactory.h"
#include "qgsmaplayerlegend.h"
#include "qgsmessagelog.h"
#include "qgspainting.h"
#include "qgspointclouddataprovider.h"
#include "qgspointcloudeditingindex.h"
#include "qgspointcloudindex.h"
#include "qgspointcloudlayereditutils.h"
#include "qgspointcloudlayerelevationproperties.h"
#include "qgspointcloudlayerprofilegenerator.h"
#include "qgspointcloudlayerrenderer.h"
#include "qgspointcloudlayerundocommand.h"
#include "qgspointcloudrenderer.h"
#include "qgspointcloudrendererregistry.h"
#include "qgspointcloudstatistics.h"
#include "qgspointcloudstatscalculationtask.h"
#include "qgspointcloudsubindex.h"
#include "qgsproviderregistry.h"
#include "qgsrectangle.h"
#include "qgsruntimeprofiler.h"
#include "qgstaskmanager.h"
#include "qgsthreadingutils.h"
#include "qgsvirtualpointcloudprovider.h"
#include "qgsxmlutils.h"

#include <QUrl>

#include "moc_qgspointcloudlayer.cpp"

QgsPointCloudLayer::QgsPointCloudLayer( const QString &uri,
                                        const QString &baseName,
                                        const QString &providerLib,
                                        const QgsPointCloudLayer::LayerOptions &options )
  : QgsMapLayer( Qgis::LayerType::PointCloud, baseName, uri )
  , mElevationProperties( new QgsPointCloudLayerElevationProperties( this ) )
  , mLayerOptions( options )
{
  if ( !uri.isEmpty() && !providerLib.isEmpty() )
  {
    const QgsDataProvider::ProviderOptions providerOptions { options.transformContext };
    Qgis::DataProviderReadFlags providerFlags;
    if ( options.loadDefaultStyle )
    {
      providerFlags |= Qgis::DataProviderReadFlag::LoadDefaultStyle;
    }
    setDataSource( uri, baseName, providerLib, providerOptions, providerFlags );
  }

  setLegend( QgsMapLayerLegend::defaultPointCloudLegend( this ) );
  connect( this, &QgsPointCloudLayer::subsetStringChanged, this, &QgsMapLayer::configChanged );
  connect( undoStack(), &QUndoStack::indexChanged, this, &QgsMapLayer::layerModified );
  connect( this, &QgsMapLayer::layerModified, this, [this] { triggerRepaint(); } );
}

QgsPointCloudLayer::~QgsPointCloudLayer()
{
  if ( QgsTask *task = QgsApplication::taskManager()->task( mStatsCalculationTask ) )
  {
    mStatsCalculationTask = 0;
    task->cancel();
    task->waitForFinished();
  }
}

QgsPointCloudLayer *QgsPointCloudLayer::clone() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsPointCloudLayer *layer = new QgsPointCloudLayer( source(), name(), mProviderKey, mLayerOptions );
  QgsMapLayer::clone( layer );

  if ( mRenderer )
    layer->setRenderer( mRenderer->clone() );

  layer->mElevationProperties = mElevationProperties->clone();
  layer->mElevationProperties->setParent( layer );

  layer->mLayerOptions = mLayerOptions;
  layer->mSync3DRendererTo2DRenderer = mSync3DRendererTo2DRenderer;

  return layer;
}

QgsRectangle QgsPointCloudLayer::extent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !mDataProvider )
    return QgsRectangle();

  return mDataProvider->extent();
}

QgsMapLayerRenderer *QgsPointCloudLayer::createMapRenderer( QgsRenderContext &rendererContext )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mRenderer->type() != "extent"_L1 )
    loadIndexesForRenderContext( rendererContext );

  return new QgsPointCloudLayerRenderer( this, rendererContext );
}

QgsAbstractProfileGenerator *QgsPointCloudLayer::createProfileGenerator( const QgsProfileRequest &request )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return new QgsPointCloudLayerProfileGenerator( this, request );
}

QgsPointCloudDataProvider *QgsPointCloudLayer::dataProvider()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDataProvider.get();
}

const QgsPointCloudDataProvider *QgsPointCloudLayer::dataProvider() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDataProvider.get();
}

bool QgsPointCloudLayer::readXml( const QDomNode &layerNode, QgsReadWriteContext &context )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  // create provider
  const QDomNode pkeyNode = layerNode.namedItem( u"provider"_s );
  mProviderKey = pkeyNode.toElement().text();

  if ( !( mReadFlags & QgsMapLayer::FlagDontResolveLayers ) )
  {
    const QgsDataProvider::ProviderOptions providerOptions { context.transformContext() };
    Qgis::DataProviderReadFlags flags = providerReadFlags( layerNode, mReadFlags );
    // read extent
    if ( mReadFlags & QgsMapLayer::FlagReadExtentFromXml )
    {
      const QDomNode extentNode = layerNode.namedItem( u"extent"_s );
      if ( !extentNode.isNull() )
      {
        // get the extent
        const QgsRectangle mbr = QgsXmlUtils::readRectangle( extentNode.toElement() );

        // store the extent
        setExtent( mbr );
      }
    }

    setDataSource( mDataSource, mLayerName, mProviderKey, providerOptions, flags );
    const QDomNode subset = layerNode.namedItem( u"subset"_s );
    const QString subsetText = subset.toElement().text();
    if ( !subsetText.isEmpty() )
      setSubsetString( subsetText );
  }

  if ( !isValid() )
  {
    return false;
  }

  QString errorMsg;
  if ( !readSymbology( layerNode, errorMsg, context ) )
    return false;

  readStyleManager( layerNode );
  return true;
}

bool QgsPointCloudLayer::writeXml( QDomNode &layerNode, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QDomElement mapLayerNode = layerNode.toElement();
  mapLayerNode.setAttribute( u"type"_s, QgsMapLayerFactory::typeToString( Qgis::LayerType::PointCloud ) );

  if ( !subsetString().isEmpty() )
  {
    QDomElement subset = doc.createElement( u"subset"_s );
    const QDomText subsetText = doc.createTextNode( subsetString() );
    subset.appendChild( subsetText );
    layerNode.appendChild( subset );
  }
  if ( mDataProvider )
  {
    QDomElement provider  = doc.createElement( u"provider"_s );
    const QDomText providerText = doc.createTextNode( providerType() );
    provider.appendChild( providerText );
    layerNode.appendChild( provider );
  }

  writeStyleManager( layerNode, doc );

  QString errorMsg;
  return writeSymbology( layerNode, doc, errorMsg, context );
}

bool QgsPointCloudLayer::readSymbology( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QDomElement elem = node.toElement();

  readCommonStyle( elem, context, categories );

  readStyle( node, errorMessage, context, categories );

  if ( categories.testFlag( CustomProperties ) )
    readCustomProperties( node, u"variable"_s );

  if ( categories.testFlag( Legend ) )
  {
    QgsReadWriteContextCategoryPopper p = context.enterCategory( tr( "Legend" ) );

    const QDomElement legendElem = node.firstChildElement( u"legend"_s );
    if ( QgsMapLayerLegend *l = legend(); !legendElem.isNull() )
    {
      l->readXml( legendElem, context );
    }
  }

  return true;
}

bool QgsPointCloudLayer::readStyle( const QDomNode &node, QString &, QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  bool result = true;

  if ( categories.testFlag( Symbology3D ) )
  {
    bool ok;
    bool sync = node.attributes().namedItem( u"sync3DRendererTo2DRenderer"_s ).nodeValue().toInt( &ok );
    if ( ok )
      setSync3DRendererTo2DRenderer( sync );
  }

  if ( categories.testFlag( Symbology ) )
  {
    QDomElement rendererElement = node.firstChildElement( u"renderer"_s );
    if ( !rendererElement.isNull() )
    {
      std::unique_ptr< QgsPointCloudRenderer > r( QgsPointCloudRenderer::load( rendererElement, context ) );
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
      setRenderer( QgsPointCloudRendererRegistry::defaultRenderer( this ) );
    }
  }

  if ( categories.testFlag( Symbology ) )
  {
    // get and set the blend mode if it exists
    const QDomNode blendModeNode = node.namedItem( u"blendMode"_s );
    if ( !blendModeNode.isNull() )
    {
      const QDomElement e = blendModeNode.toElement();
      setBlendMode( QgsPainting::getCompositionMode( static_cast< Qgis::BlendMode >( e.text().toInt() ) ) );
    }
  }

  // get and set the layer transparency and scale visibility if they exists
  if ( categories.testFlag( Rendering ) )
  {
    const QDomNode layerOpacityNode = node.namedItem( u"layerOpacity"_s );
    if ( !layerOpacityNode.isNull() )
    {
      const QDomElement e = layerOpacityNode.toElement();
      setOpacity( e.text().toDouble() );
    }

    const bool hasScaleBasedVisibiliy { node.attributes().namedItem( u"hasScaleBasedVisibilityFlag"_s ).nodeValue() == '1' };
    setScaleBasedVisibility( hasScaleBasedVisibiliy );
    bool ok;
    const double maxScale { node.attributes().namedItem( u"maxScale"_s ).nodeValue().toDouble( &ok ) };
    if ( ok )
    {
      setMaximumScale( maxScale );
    }
    const double minScale { node.attributes().namedItem( u"minScale"_s ).nodeValue().toDouble( &ok ) };
    if ( ok )
    {
      setMinimumScale( minScale );
    }
  }
  return result;
}

bool QgsPointCloudLayer::writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage,
    const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( errorMessage )

  QDomElement elem = node.toElement();
  writeCommonStyle( elem, doc, context, categories );

  ( void )writeStyle( node, doc, errorMessage, context, categories );

  if ( categories.testFlag( Legend ) && legend() )
  {
    QDomElement legendElement = legend()->writeXml( doc, context );
    if ( !legendElement.isNull() )
      node.appendChild( legendElement );
  }

  return true;
}

bool QgsPointCloudLayer::writeStyle( QDomNode &node, QDomDocument &doc, QString &, const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QDomElement mapLayerNode = node.toElement();

  if ( categories.testFlag( Symbology3D ) )
  {
    mapLayerNode.setAttribute( u"sync3DRendererTo2DRenderer"_s, mSync3DRendererTo2DRenderer ? 1 : 0 );
  }

  if ( categories.testFlag( Symbology ) )
  {
    if ( mRenderer )
    {
      const QDomElement rendererElement = mRenderer->save( doc, context );
      node.appendChild( rendererElement );
    }
  }

  //save customproperties
  if ( categories.testFlag( CustomProperties ) )
  {
    writeCustomProperties( node, doc );
  }

  if ( categories.testFlag( Symbology ) )
  {
    // add the blend mode field
    QDomElement blendModeElem  = doc.createElement( u"blendMode"_s );
    const QDomText blendModeText = doc.createTextNode( QString::number( static_cast< int >( QgsPainting::getBlendModeEnum( blendMode() ) ) ) );
    blendModeElem.appendChild( blendModeText );
    node.appendChild( blendModeElem );
  }

  // add the layer opacity and scale visibility
  if ( categories.testFlag( Rendering ) )
  {
    QDomElement layerOpacityElem = doc.createElement( u"layerOpacity"_s );
    const QDomText layerOpacityText = doc.createTextNode( QString::number( opacity() ) );
    layerOpacityElem.appendChild( layerOpacityText );
    node.appendChild( layerOpacityElem );

    mapLayerNode.setAttribute( u"hasScaleBasedVisibilityFlag"_s, hasScaleBasedVisibility() ? 1 : 0 );
    mapLayerNode.setAttribute( u"maxScale"_s, maximumScale() );
    mapLayerNode.setAttribute( u"minScale"_s, minimumScale() );
  }
  return true;
}

void QgsPointCloudLayer::setTransformContext( const QgsCoordinateTransformContext &transformContext )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mDataProvider )
    mDataProvider->setTransformContext( transformContext );
  invalidateWgs84Extent();
}

void QgsPointCloudLayer::setDataSourcePrivate( const QString &dataSource, const QString &baseName, const QString &provider,
    const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mDataProvider )
  {
    disconnect( mDataProvider.get(), &QgsPointCloudDataProvider::dataChanged, this, &QgsPointCloudLayer::dataChanged );
    disconnect( mDataProvider.get(), &QgsPointCloudDataProvider::indexGenerationStateChanged, this, &QgsPointCloudLayer::onPointCloudIndexGenerationStateChanged );
  }

  setName( baseName );
  mProviderKey = provider;
  mDataSource = dataSource;

  if ( mPreloadedProvider )
  {
    mDataProvider.reset( qobject_cast< QgsPointCloudDataProvider * >( mPreloadedProvider.release() ) );
  }
  else
  {
    std::unique_ptr< QgsScopedRuntimeProfile > profile;
    if ( QgsApplication::profiler()->groupIsActive( u"projectload"_s ) )
      profile = std::make_unique< QgsScopedRuntimeProfile >( tr( "Create %1 provider" ).arg( provider ), u"projectload"_s );
    mDataProvider.reset( qobject_cast<QgsPointCloudDataProvider *>( QgsProviderRegistry::instance()->createProvider( provider, dataSource, options, flags ) ) );
  }

  if ( !mDataProvider )
  {
    QgsDebugError( u"Unable to get point cloud data provider"_s );
    setValid( false );
    return;
  }

  mDataProvider->setParent( this );
  QgsDebugMsgLevel( u"Instantiated the point cloud data provider plugin"_s, 2 );

  setValid( mDataProvider->isValid() );
  if ( !isValid() )
  {
    QgsDebugError( u"Invalid point cloud provider plugin %1"_s.arg( QString( mDataSource.toUtf8() ) ) );
    setError( mDataProvider->error() );
    return;
  }

  connect( mDataProvider.get(), &QgsPointCloudDataProvider::indexGenerationStateChanged, this, &QgsPointCloudLayer::onPointCloudIndexGenerationStateChanged );
  connect( mDataProvider.get(), &QgsPointCloudDataProvider::dataChanged, this, &QgsPointCloudLayer::dataChanged );

  // Load initial extent, crs and renderer
  setCrs( mDataProvider->crs() );
  if ( !( flags & Qgis::DataProviderReadFlag::SkipGetExtent ) )
  {
    setExtent3D( mDataProvider->extent3D() );
  }

  bool loadDefaultStyleFlag = false;
  if ( flags & Qgis::DataProviderReadFlag::LoadDefaultStyle )
  {
    loadDefaultStyleFlag = true;
  }

  if ( !mLayerOptions.skipIndexGeneration &&
       mDataProvider &&
       mDataProvider->indexingState() != QgsPointCloudDataProvider::PointCloudIndexGenerationState::Indexed &&
       mDataProvider->pointCount() > 0 )
  {
    mDataProvider->generateIndex();
  }

  if ( !mLayerOptions.skipStatisticsCalculation &&
       mDataProvider &&
       mDataProvider->indexingState() == QgsPointCloudDataProvider::PointCloudIndexGenerationState::Indexed &&
       mDataProvider->pointCount() > 0 )
  {
    calculateStatistics();
  }

  if ( !mRenderer || loadDefaultStyleFlag )
  {
    std::unique_ptr< QgsScopedRuntimeProfile > profile;
    if ( QgsApplication::profiler()->groupIsActive( u"projectload"_s ) )
      profile = std::make_unique< QgsScopedRuntimeProfile >( tr( "Load layer style" ), u"projectload"_s );

    bool defaultLoadedFlag = false;

    if ( loadDefaultStyleFlag && isSpatial() && mDataProvider->capabilities() & QgsPointCloudDataProvider::CreateRenderer )
    {
      // first try to create a renderer directly from the data provider
      std::unique_ptr< QgsPointCloudRenderer > defaultRenderer( mDataProvider->createRenderer() );
      if ( defaultRenderer )
      {
        defaultLoadedFlag = true;
        setRenderer( defaultRenderer.release() );
      }
    }

    if ( !defaultLoadedFlag && loadDefaultStyleFlag )
    {
      loadDefaultStyle( defaultLoadedFlag );
    }

    if ( !defaultLoadedFlag )
    {
      // all else failed, create default renderer
      setRenderer( QgsPointCloudRendererRegistry::defaultRenderer( this ) );
    }
  }
}

QString QgsPointCloudLayer::encodedSource( const QString &source, const QgsReadWriteContext &context ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsProviderRegistry::instance()->absoluteToRelativeUri( mProviderKey, source, context );
}

QString QgsPointCloudLayer::decodedSource( const QString &source, const QString &dataProvider, const QgsReadWriteContext &context ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsProviderRegistry::instance()->relativeToAbsoluteUri( dataProvider, source, context );
}

void QgsPointCloudLayer::onPointCloudIndexGenerationStateChanged( QgsPointCloudDataProvider::PointCloudIndexGenerationState state )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  switch ( state )
  {
    case QgsPointCloudDataProvider::Indexed:
    {
      resetRenderer();
      break;
    }
    case QgsPointCloudDataProvider::NotIndexed:
    {
      QgsError providerError = mDataProvider->error();
      if ( !providerError.isEmpty() )
      {
        setError( providerError );
        emit raiseError( providerError.summary() );
      }
      break;
    }
    case QgsPointCloudDataProvider::Indexing:
      break;
  }
}


QString QgsPointCloudLayer::loadDefaultStyle( bool &resultFlag )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mDataProvider->capabilities() & QgsPointCloudDataProvider::CreateRenderer )
  {
    // first try to create a renderer directly from the data provider
    std::unique_ptr< QgsPointCloudRenderer > defaultRenderer( mDataProvider->createRenderer() );
    if ( defaultRenderer )
    {
      resultFlag = true;
      setRenderer( defaultRenderer.release() );
      return QString();
    }
  }

  return QgsMapLayer::loadDefaultStyle( resultFlag );
}

QString QgsPointCloudLayer::htmlMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QgsLayerMetadataFormatter htmlFormatter( metadata() );
  QString myMetadata = u"<html>\n<body>\n"_s;

  myMetadata += generalHtmlMetadata();

  // Begin Provider section
  myMetadata += u"<h1>"_s + tr( "Information from provider" ) + u"</h1>\n<hr>\n"_s;
  myMetadata += "<table class=\"list-view\">\n"_L1;

  // Extent
  myMetadata += u"<tr><td class=\"highlight\">"_s + tr( "Extent" ) + u"</td><td>"_s + extent().toString() + u"</td></tr>\n"_s;

  // feature count
  QLocale locale = QLocale();
  locale.setNumberOptions( locale.numberOptions() &= ~QLocale::NumberOption::OmitGroupSeparator );
  const qint64 pointCount = mDataProvider ? mDataProvider->pointCount() : -1;
  myMetadata += u"<tr><td class=\"highlight\">"_s
                + tr( "Point count" ) + u"</td><td>"_s
                + ( pointCount < 0 ? tr( "unknown" ) : locale.toString( static_cast<qlonglong>( pointCount ) ) )
                + u"</td></tr>\n"_s;

  if ( const QgsPointCloudDataProvider *provider = dataProvider() )
  {
    myMetadata += provider->htmlMetadata();
  }

  myMetadata += "</table>\n<br><br>"_L1;

  // CRS
  myMetadata += crsHtmlMetadata();

  // provider metadata section
  myMetadata += u"<h1>"_s + tr( "Metadata" ) + u"</h1>\n<hr>\n"_s + u"<table class=\"list-view\">\n"_s;
  const QVariantMap originalMetadata = mDataProvider ? mDataProvider->originalMetadata() : QVariantMap();

  if ( originalMetadata.value( u"creation_year"_s ).toInt() > 0 && originalMetadata.contains( u"creation_doy"_s ) )
  {
    QDate creationDate( originalMetadata.value( u"creation_year"_s ).toInt(), 1, 1 );
    creationDate = creationDate.addDays( originalMetadata.value( u"creation_doy"_s ).toInt() );

    myMetadata += u"<tr><td class=\"highlight\">"_s
                  + tr( "Creation date" ) + u"</td><td>"_s
                  + creationDate.toString( Qt::ISODate )
                  + u"</td></tr>\n"_s;
  }
  if ( originalMetadata.contains( u"major_version"_s ) && originalMetadata.contains( u"minor_version"_s ) )
  {
    myMetadata += u"<tr><td class=\"highlight\">"_s
                  + tr( "Version" ) + u"</td><td>"_s
                  + u"%1.%2"_s.arg( originalMetadata.value( u"major_version"_s ).toString(),
                                    originalMetadata.value( u"minor_version"_s ).toString() )
                  + u"</td></tr>\n"_s;
  }

  if ( !originalMetadata.value( u"dataformat_id"_s ).toString().isEmpty() )
  {
    myMetadata += u"<tr><td class=\"highlight\">"_s
                  + tr( "Data format" ) + u"</td><td>"_s
                  + u"%1 (%2)"_s.arg( QgsPointCloudDataProvider::translatedDataFormatIds().value( originalMetadata.value( u"dataformat_id"_s ).toInt() ),
                                      originalMetadata.value( u"dataformat_id"_s ).toString() ).trimmed()
                  + u"</td></tr>\n"_s;
  }

  myMetadata += u"<tr><td class=\"highlight\">"_s
                + tr( "Scale X" ) + u"</td><td>"_s
                + QString::number( originalMetadata.value( u"scale_x"_s ).toDouble() )
                + u"</td></tr>\n"_s;
  myMetadata += u"<tr><td class=\"highlight\">"_s
                + tr( "Scale Y" ) + u"</td><td>"_s
                + QString::number( originalMetadata.value( u"scale_y"_s ).toDouble() )
                + u"</td></tr>\n"_s;
  myMetadata += u"<tr><td class=\"highlight\">"_s
                + tr( "Scale Z" ) + u"</td><td>"_s
                + QString::number( originalMetadata.value( u"scale_z"_s ).toDouble() )
                + u"</td></tr>\n"_s;

  myMetadata += u"<tr><td class=\"highlight\">"_s
                + tr( "Offset X" ) + u"</td><td>"_s
                + QString::number( originalMetadata.value( u"offset_x"_s ).toDouble() )
                + u"</td></tr>\n"_s;
  myMetadata += u"<tr><td class=\"highlight\">"_s
                + tr( "Offset Y" ) + u"</td><td>"_s
                + QString::number( originalMetadata.value( u"offset_y"_s ).toDouble() )
                + u"</td></tr>\n"_s;
  myMetadata += u"<tr><td class=\"highlight\">"_s
                + tr( "Offset Z" ) + u"</td><td>"_s
                + QString::number( originalMetadata.value( u"offset_z"_s ).toDouble() )
                + u"</td></tr>\n"_s;

  if ( !originalMetadata.value( u"project_id"_s ).toString().isEmpty() )
  {
    myMetadata += u"<tr><td class=\"highlight\">"_s
                  + tr( "Project ID" ) + u"</td><td>"_s
                  + originalMetadata.value( u"project_id"_s ).toString()
                  + u"</td></tr>\n"_s;
  }

  if ( !originalMetadata.value( u"system_id"_s ).toString().isEmpty() )
  {
    myMetadata += u"<tr><td class=\"highlight\">"_s
                  + tr( "System ID" ) + u"</td><td>"_s
                  + originalMetadata.value( u"system_id"_s ).toString()
                  + u"</td></tr>\n"_s;
  }

  if ( !originalMetadata.value( u"software_id"_s ).toString().isEmpty() )
  {
    myMetadata += u"<tr><td class=\"highlight\">"_s
                  + tr( "Software ID" ) + u"</td><td>"_s
                  + originalMetadata.value( u"software_id"_s ).toString()
                  + u"</td></tr>\n"_s;
  }

  // End Provider section
  myMetadata += "</table>\n<br><br>"_L1;

  // identification section
  myMetadata += u"<h1>"_s + tr( "Identification" ) + u"</h1>\n<hr>\n"_s;
  myMetadata += htmlFormatter.identificationSectionHtml( );
  myMetadata += "<br><br>\n"_L1;

  // extent section
  myMetadata += u"<h1>"_s + tr( "Extent" ) + u"</h1>\n<hr>\n"_s;
  myMetadata += htmlFormatter.extentSectionHtml( isSpatial() );
  myMetadata += "<br><br>\n"_L1;

  // Start the Access section
  myMetadata += u"<h1>"_s + tr( "Access" ) + u"</h1>\n<hr>\n"_s;
  myMetadata += htmlFormatter.accessSectionHtml( );
  myMetadata += "<br><br>\n"_L1;

  // Attributes section
  myMetadata += u"<h1>"_s + tr( "Attributes" ) + u"</h1>\n<hr>\n<table class=\"list-view\">\n"_s;

  const QgsPointCloudAttributeCollection attrs = attributes();

  // count attributes
  myMetadata += u"<tr><td class=\"highlight\">"_s + tr( "Count" ) + u"</td><td>"_s + QString::number( attrs.count() ) + u"</td></tr>\n"_s;

  myMetadata += "</table>\n<br><table width=\"100%\" class=\"tabular-view\">\n"_L1;
  myMetadata += "<tr><th>"_L1 + tr( "Attribute" ) + "</th><th>"_L1 + tr( "Type" ) + "</th></tr>\n"_L1;

  for ( int i = 0; i < attrs.count(); ++i )
  {
    const QgsPointCloudAttribute attribute = attrs.at( i );
    QString rowClass;
    if ( i % 2 )
      rowClass = u"class=\"odd-row\""_s;
    myMetadata += "<tr "_L1 + rowClass + "><td>"_L1 + attribute.name() + "</td><td>"_L1 + attribute.displayType() + "</td></tr>\n"_L1;
  }

  //close field list
  myMetadata += "</table>\n<br><br>"_L1;


  // Start the contacts section
  myMetadata += u"<h1>"_s + tr( "Contacts" ) + u"</h1>\n<hr>\n"_s;
  myMetadata += htmlFormatter.contactsSectionHtml( );
  myMetadata += "<br><br>\n"_L1;

  // Start the links section
  myMetadata += u"<h1>"_s + tr( "Links" ) + u"</h1>\n<hr>\n"_s;
  myMetadata += htmlFormatter.linksSectionHtml( );
  myMetadata += "<br><br>\n"_L1;

  // Start the history section
  myMetadata += u"<h1>"_s + tr( "History" ) + u"</h1>\n<hr>\n"_s;
  myMetadata += htmlFormatter.historySectionHtml( );
  myMetadata += "<br><br>\n"_L1;

  myMetadata += customPropertyHtmlMetadata();

  myMetadata += "\n</body>\n</html>\n"_L1;
  return myMetadata;
}

QgsMapLayerElevationProperties *QgsPointCloudLayer::elevationProperties()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mElevationProperties;
}

QgsPointCloudAttributeCollection QgsPointCloudLayer::attributes() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDataProvider ? mDataProvider->attributes() : QgsPointCloudAttributeCollection();
}

qint64 QgsPointCloudLayer::pointCount() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDataProvider ? mDataProvider->pointCount() : 0;
}

QgsPointCloudRenderer *QgsPointCloudLayer::renderer()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mRenderer.get();
}

const QgsPointCloudRenderer *QgsPointCloudLayer::renderer() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mRenderer.get();
}

void QgsPointCloudLayer::setRenderer( QgsPointCloudRenderer *renderer )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( renderer == mRenderer.get() )
    return;

  mRenderer.reset( renderer );
  emit rendererChanged();
  emitStyleChanged();

  if ( mSync3DRendererTo2DRenderer )
    convertRenderer3DFromRenderer2D();
}

bool QgsPointCloudLayer::setSubsetString( const QString &subset )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !isValid() || !mDataProvider )
  {
    QgsDebugMsgLevel( u"invoked with invalid layer or null mDataProvider"_s, 3 );
    setCustomProperty( u"storedSubsetString"_s, subset );
    return false;
  }
  else if ( subset == mDataProvider->subsetString() )
    return true;

  bool res = mDataProvider->setSubsetString( subset );
  if ( res )
  {
    emit subsetStringChanged();
    triggerRepaint();
  }
  return res;
}

QString QgsPointCloudLayer::subsetString() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !isValid() || !mDataProvider )
  {
    QgsDebugMsgLevel( u"invoked with invalid layer or null mDataProvider"_s, 3 );
    return customProperty( u"storedSubsetString"_s ).toString();
  }
  return mDataProvider->subsetString();
}

bool QgsPointCloudLayer::convertRenderer3DFromRenderer2D()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  bool result = false;
  if ( QgsAbstractPointCloud3DRenderer *r = static_cast<QgsAbstractPointCloud3DRenderer *>( renderer3D() ) )
  {
    std::unique_ptr< QgsAbstractPointCloud3DRenderer > newRenderer( static_cast< QgsAbstractPointCloud3DRenderer * >( r->clone() ) );
    result = newRenderer->convertFrom2DRenderer( renderer() );
    setRenderer3D( newRenderer.release() );
    trigger3DUpdate();
  }
  return result;
}

bool QgsPointCloudLayer::sync3DRendererTo2DRenderer() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mSync3DRendererTo2DRenderer;
}

void QgsPointCloudLayer::setSync3DRendererTo2DRenderer( bool sync )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mSync3DRendererTo2DRenderer = sync;
  if ( sync )
    convertRenderer3DFromRenderer2D();
}

void QgsPointCloudLayer::calculateStatistics()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !mDataProvider.get() || !mDataProvider->hasValidIndex() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Failed to calculate statistics of the point cloud %1" ).arg( this->name() ) );
    return;
  }
  if ( mStatsCalculationTask )
  {
    QgsMessageLog::logMessage( QObject::tr( "A statistics calculation task for the point cloud %1 is already in progress" ).arg( this->name() ) );
    return;
  }

  QgsPointCloudStatistics indexStats = mDataProvider->metadataStatistics();
  QList<QString> indexStatsAttributes = indexStats.statisticsMap().keys();
  QVector<QgsPointCloudAttribute> attributes = mDataProvider->attributes().attributes();
  // Do not calculate stats for attributes that the index gives us stats for
  for ( int i = 0; i < attributes.size(); ++i )
  {
    if ( indexStatsAttributes.contains( attributes[i].name() ) )
    {
      attributes.remove( i );
      --i;
    }
  }

  // Use the layer statistics for now, until we can calculate complete ones
  mStatistics = indexStats;
  if ( attributes.empty() && indexStats.sampledPointsCount() > 0 )
  {
    // All attributes are covered by the saved stats, skip calculating anything
    mStatisticsCalculationState = QgsPointCloudLayer::PointCloudStatisticsCalculationState::Calculated;
    emit statisticsCalculationStateChanged( mStatisticsCalculationState );
    resetRenderer();
    return;
  }

  QgsPointCloudStatsCalculationTask *task = new QgsPointCloudStatsCalculationTask( mDataProvider->index(), attributes, 1000000 );
  connect( task, &QgsTask::taskCompleted, this, [this, task, indexStats, indexStatsAttributes]()
  {
    mStatistics = task->calculationResults();

    // Fetch what we can directly from the index
    QMap<QString, QgsPointCloudAttributeStatistics> statsMap = mStatistics.statisticsMap();
    for ( const QString &attribute : indexStatsAttributes )
    {
      statsMap[ attribute ] = indexStats.statisticsOf( attribute );
    }
    mStatistics = QgsPointCloudStatistics( mStatistics.sampledPointsCount(), statsMap );

    mStatisticsCalculationState = QgsPointCloudLayer::PointCloudStatisticsCalculationState::Calculated;
    emit statisticsCalculationStateChanged( mStatisticsCalculationState );
    resetRenderer();
    mStatsCalculationTask = 0;
#ifdef HAVE_COPC
    if ( mDataProvider && mDataProvider->index() && mDataProvider->index().isValid() && mDataProvider->name() == "pdal"_L1 && mStatistics.sampledPointsCount() != 0 )
    {
      mDataProvider->index().writeStatistics( mStatistics );
    }
#endif
  } );

  // In case the statistics calculation fails, QgsTask::taskTerminated will be called
  connect( task, &QgsTask::taskTerminated, this, [this]()
  {
    if ( mStatsCalculationTask )
    {
      QgsMessageLog::logMessage( QObject::tr( "Failed to calculate statistics of the point cloud %1" ).arg( this->name() ) );
      mStatsCalculationTask = 0;
    }
  } );

  mStatsCalculationTask = QgsApplication::taskManager()->addTask( task );

  mStatisticsCalculationState = QgsPointCloudLayer::PointCloudStatisticsCalculationState::Calculating;
  emit statisticsCalculationStateChanged( mStatisticsCalculationState );
}

void QgsPointCloudLayer::resetRenderer()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mDataProvider->loadIndex();
  if ( !mLayerOptions.skipStatisticsCalculation && statisticsCalculationState() == QgsPointCloudLayer::PointCloudStatisticsCalculationState::NotStarted )
  {
    calculateStatistics();
  }
  if ( !mRenderer || mRenderer->type() == "extent"_L1 )
  {
    setRenderer( QgsPointCloudRendererRegistry::defaultRenderer( this ) );
  }
  triggerRepaint();

  emit rendererChanged();
}

void QgsPointCloudLayer::loadIndexesForRenderContext( QgsRenderContext &rendererContext ) const
{
  if ( mDataProvider->capabilities() & QgsPointCloudDataProvider::ContainSubIndexes )
  {
    QgsRectangle renderExtent;
    try
    {
      renderExtent = rendererContext.coordinateTransform().transformBoundingBox( rendererContext.mapExtent(), Qgis::TransformDirection::Reverse );
    }
    catch ( QgsCsException & )
    {
      QgsDebugError( u"Transformation of extent failed!"_s );
    }

    const QVector<QgsPointCloudSubIndex> subIndex = mDataProvider->subIndexes();
    if ( const QgsVirtualPointCloudProvider *vpcProvider = dynamic_cast<QgsVirtualPointCloudProvider *>( mDataProvider.get() ) )
    {
      for ( int i = 0; i < subIndex.size(); ++i )
      {
        // no need to load as it's there
        if ( subIndex.at( i ).index() )
          continue;

        if ( subIndex.at( i ).extent().intersects( renderExtent ) &&
             ( renderExtent.width() < vpcProvider->averageSubIndexWidth() ||
               renderExtent.height() < vpcProvider->averageSubIndexHeight() ) )
        {
          mDataProvider->loadSubIndex( i );
        }
      }
    }
  }
}

bool QgsPointCloudLayer::startEditing()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  if ( mEditIndex )
    return false;

  mEditIndex = QgsPointCloudIndex( new QgsPointCloudEditingIndex( this ) );

  if ( !mEditIndex.isValid() )
  {
    mEditIndex = QgsPointCloudIndex();
    return false;
  }

  emit editingStarted();
  return true;
}

bool QgsPointCloudLayer::commitChanges( bool stopEditing )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  if ( !mEditIndex )
    return false;

  if ( mEditIndex.isModified() )
  {
    if ( !mEditIndex.commitChanges( &mCommitError ) )
      return false;

    // emitting layerModified() is not required as that's done automatically
    // when undo stack index changes
  }

  undoStack()->clear();

  if ( stopEditing )
  {
    mEditIndex = QgsPointCloudIndex();
    emit editingStopped();
  }

  return true;
}

QString QgsPointCloudLayer::commitError() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  return mCommitError;
}

bool QgsPointCloudLayer::rollBack()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  if ( !mEditIndex )
    return false;

  const QList<QgsPointCloudNodeId> updatedNodes = mEditIndex.updatedNodes();

  undoStack()->clear();

  mEditIndex = QgsPointCloudIndex();
  emit editingStopped();

  if ( !updatedNodes.isEmpty() )
  {
    for ( const QgsPointCloudNodeId &n : updatedNodes )
      emit chunkAttributeValuesChanged( n );

    // emitting layerModified() is not required as that's done automatically
    // when undo stack index changes
  }

  return true;
}

bool QgsPointCloudLayer::supportsEditing() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  return mDataProvider && mDataProvider->capabilities() & QgsPointCloudDataProvider::Capability::ChangeAttributeValues;
}

bool QgsPointCloudLayer::isEditable() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  if ( mEditIndex )
    return true;

  return false;
}

bool QgsPointCloudLayer::isModified() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  if ( !mEditIndex )
    return false;

  return mEditIndex.isModified();
}

bool QgsPointCloudLayer::changeAttributeValue( const QgsPointCloudNodeId &n, const QVector<int> &points, const QgsPointCloudAttribute &attribute, double value )
{
  return this->changeAttributeValue( { { n, points } }, attribute, value );
}

bool QgsPointCloudLayer::changeAttributeValue( const QHash<QgsPointCloudNodeId, QVector<int>> &nodesAndPoints, const QgsPointCloudAttribute &attribute, double value )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsEventTracing::ScopedEvent _trace( u"PointCloud"_s, u"QgsPointCloudLayer::changeAttributeValue"_s );

  if ( !mEditIndex )
    return false;

  // Cannot allow x,y,z editing as points may get moved outside the node extents
  if ( attribute.name().compare( 'X'_L1, Qt::CaseInsensitive ) == 0 ||
       attribute.name().compare( 'Y'_L1, Qt::CaseInsensitive ) == 0 ||
       attribute.name().compare( 'Z'_L1, Qt::CaseInsensitive ) == 0 )
    return false;

  const QgsPointCloudAttributeCollection attributeCollection = mEditIndex.attributes();

  int attributeOffset;
  const QgsPointCloudAttribute *at = attributeCollection.find( attribute.name(), attributeOffset );

  if ( !at ||
       at->size() != attribute.size() ||
       at->type() != attribute.type() )
  {
    return false;
  }

  if ( !QgsPointCloudLayerEditUtils::isAttributeValueValid( attribute, value ) )
  {
    return false;
  }

  for ( auto it = nodesAndPoints.constBegin(); it != nodesAndPoints.constEnd(); it++ )
  {
    QgsPointCloudNodeId n = it.key();
    QVector<int> points = it.value();

    if ( !n.isValid() || !mEditIndex.hasNode( n ) ) // todo: should not have to check if n.isValid
      return false;

    if ( points.isEmpty() )
      continue;

    int pointsMin = std::numeric_limits<int>::max();
    int pointsMax = std::numeric_limits<int>::min();
    for ( int pt : std::as_const( points ) )
    {
      if ( pt < pointsMin )
        pointsMin = pt;
      if ( pt > pointsMax )
        pointsMax = pt;
    }

    if ( pointsMin < 0 || pointsMax >= mEditIndex.getNode( n ).pointCount() )
      return false;

  }

  undoStack()->push( new QgsPointCloudLayerUndoCommandChangeAttribute( this, nodesAndPoints, attribute, value ) );

  return true;
}

QgsPointCloudIndex QgsPointCloudLayer::index() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  if ( mEditIndex )
    return mEditIndex;

  if ( mDataProvider )
    return mDataProvider->index();

  return QgsPointCloudIndex();
}
