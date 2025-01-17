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
#include "moc_qgspointcloudlayer.cpp"
#include "qgspointcloudeditingindex.h"
#include "qgspointcloudlayereditutils.h"
#include "qgspointcloudlayerrenderer.h"
#include "qgspointcloudindex.h"
#include "qgspointcloudstatistics.h"
#include "qgspointcloudsubindex.h"
#include "qgsrectangle.h"
#include "qgspointclouddataprovider.h"
#include "qgsproviderregistry.h"
#include "qgslogger.h"
#include "qgslayermetadataformatter.h"
#include "qgspointcloudrenderer.h"
#include "qgsruntimeprofiler.h"
#include "qgsapplication.h"
#include "qgspainting.h"
#include "qgspointcloudrendererregistry.h"
#include "qgspointcloudlayerelevationproperties.h"
#include "qgsmaplayerlegend.h"
#include "qgsxmlutils.h"
#include "qgsmaplayerfactory.h"
#include "qgsabstractpointcloud3drenderer.h"
#include "qgspointcloudstatscalculationtask.h"
#include "qgsmessagelog.h"
#include "qgstaskmanager.h"
#include "qgsthreadingutils.h"
#include "qgspointcloudlayerprofilegenerator.h"
#include "qgspointcloudlayerundocommand.h"
#ifdef HAVE_COPC
#include "qgscopcpointcloudindex.h"
#endif

#include "qgsvirtualpointcloudprovider.h"


#include <QUrl>

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

  if ( mRenderer->type() != QLatin1String( "extent" ) )
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
  // BAD! 2D rendering of point clouds is NOT thread safe
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

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
    const QDomNode subset = layerNode.namedItem( QStringLiteral( "subset" ) );
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
  mapLayerNode.setAttribute( QStringLiteral( "type" ), QgsMapLayerFactory::typeToString( Qgis::LayerType::PointCloud ) );

  if ( !subsetString().isEmpty() )
  {
    QDomElement subset = doc.createElement( QStringLiteral( "subset" ) );
    const QDomText subsetText = doc.createTextNode( subsetString() );
    subset.appendChild( subsetText );
    layerNode.appendChild( subset );
  }
  if ( mDataProvider )
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

bool QgsPointCloudLayer::readSymbology( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QDomElement elem = node.toElement();

  readCommonStyle( elem, context, categories );

  readStyle( node, errorMessage, context, categories );

  if ( categories.testFlag( CustomProperties ) )
    readCustomProperties( node, QStringLiteral( "variable" ) );

  return true;
}

bool QgsPointCloudLayer::readStyle( const QDomNode &node, QString &, QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  bool result = true;

  if ( categories.testFlag( Symbology3D ) )
  {
    bool ok;
    bool sync = node.attributes().namedItem( QStringLiteral( "sync3DRendererTo2DRenderer" ) ).nodeValue().toInt( &ok );
    if ( ok )
      setSync3DRendererTo2DRenderer( sync );
  }

  if ( categories.testFlag( Symbology ) )
  {
    QDomElement rendererElement = node.firstChildElement( QStringLiteral( "renderer" ) );
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
    const QDomNode blendModeNode = node.namedItem( QStringLiteral( "blendMode" ) );
    if ( !blendModeNode.isNull() )
    {
      const QDomElement e = blendModeNode.toElement();
      setBlendMode( QgsPainting::getCompositionMode( static_cast< Qgis::BlendMode >( e.text().toInt() ) ) );
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

bool QgsPointCloudLayer::writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage,
    const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( errorMessage )

  QDomElement elem = node.toElement();
  writeCommonStyle( elem, doc, context, categories );

  ( void )writeStyle( node, doc, errorMessage, context, categories );

  return true;
}

bool QgsPointCloudLayer::writeStyle( QDomNode &node, QDomDocument &doc, QString &, const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QDomElement mapLayerNode = node.toElement();

  if ( categories.testFlag( Symbology3D ) )
  {
    mapLayerNode.setAttribute( QStringLiteral( "sync3DRendererTo2DRenderer" ), mSync3DRendererTo2DRenderer ? 1 : 0 );
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
    QDomElement blendModeElem  = doc.createElement( QStringLiteral( "blendMode" ) );
    const QDomText blendModeText = doc.createTextNode( QString::number( static_cast< int >( QgsPainting::getBlendModeEnum( blendMode() ) ) ) );
    blendModeElem.appendChild( blendModeText );
    node.appendChild( blendModeElem );
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
    if ( QgsApplication::profiler()->groupIsActive( QStringLiteral( "projectload" ) ) )
      profile = std::make_unique< QgsScopedRuntimeProfile >( tr( "Create %1 provider" ).arg( provider ), QStringLiteral( "projectload" ) );
    mDataProvider.reset( qobject_cast<QgsPointCloudDataProvider *>( QgsProviderRegistry::instance()->createProvider( provider, dataSource, options, flags ) ) );
  }

  if ( !mDataProvider )
  {
    QgsDebugError( QStringLiteral( "Unable to get point cloud data provider" ) );
    setValid( false );
    return;
  }

  mDataProvider->setParent( this );
  QgsDebugMsgLevel( QStringLiteral( "Instantiated the point cloud data provider plugin" ), 2 );

  setValid( mDataProvider->isValid() );
  if ( !isValid() )
  {
    QgsDebugError( QStringLiteral( "Invalid point cloud provider plugin %1" ).arg( QString( mDataSource.toUtf8() ) ) );
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
    if ( QgsApplication::profiler()->groupIsActive( QStringLiteral( "projectload" ) ) )
      profile = std::make_unique< QgsScopedRuntimeProfile >( tr( "Load layer style" ), QStringLiteral( "projectload" ) );

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
  QString myMetadata = QStringLiteral( "<html>\n<body>\n" );

  myMetadata += generalHtmlMetadata();

  // Begin Provider section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Information from provider" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += QLatin1String( "<table class=\"list-view\">\n" );

  // Extent
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Extent" ) + QStringLiteral( "</td><td>" ) + extent().toString() + QStringLiteral( "</td></tr>\n" );

  // feature count
  QLocale locale = QLocale();
  locale.setNumberOptions( locale.numberOptions() &= ~QLocale::NumberOption::OmitGroupSeparator );
  const qint64 pointCount = mDataProvider ? mDataProvider->pointCount() : -1;
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" )
                + tr( "Point count" ) + QStringLiteral( "</td><td>" )
                + ( pointCount < 0 ? tr( "unknown" ) : locale.toString( static_cast<qlonglong>( pointCount ) ) )
                + QStringLiteral( "</td></tr>\n" );

  if ( const QgsPointCloudDataProvider *provider = dataProvider() )
  {
    myMetadata += provider->htmlMetadata();
  }

  myMetadata += QLatin1String( "</table>\n<br><br>" );

  // CRS
  myMetadata += crsHtmlMetadata();

  // provider metadata section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Metadata" ) + QStringLiteral( "</h1>\n<hr>\n" ) + QStringLiteral( "<table class=\"list-view\">\n" );
  const QVariantMap originalMetadata = mDataProvider ? mDataProvider->originalMetadata() : QVariantMap();

  if ( originalMetadata.value( QStringLiteral( "creation_year" ) ).toInt() > 0 && originalMetadata.contains( QStringLiteral( "creation_doy" ) ) )
  {
    QDate creationDate( originalMetadata.value( QStringLiteral( "creation_year" ) ).toInt(), 1, 1 );
    creationDate = creationDate.addDays( originalMetadata.value( QStringLiteral( "creation_doy" ) ).toInt() );

    myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" )
                  + tr( "Creation date" ) + QStringLiteral( "</td><td>" )
                  + creationDate.toString( Qt::ISODate )
                  + QStringLiteral( "</td></tr>\n" );
  }
  if ( originalMetadata.contains( QStringLiteral( "major_version" ) ) && originalMetadata.contains( QStringLiteral( "minor_version" ) ) )
  {
    myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" )
                  + tr( "Version" ) + QStringLiteral( "</td><td>" )
                  + QStringLiteral( "%1.%2" ).arg( originalMetadata.value( QStringLiteral( "major_version" ) ).toString(),
                      originalMetadata.value( QStringLiteral( "minor_version" ) ).toString() )
                  + QStringLiteral( "</td></tr>\n" );
  }

  if ( !originalMetadata.value( QStringLiteral( "dataformat_id" ) ).toString().isEmpty() )
  {
    myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" )
                  + tr( "Data format" ) + QStringLiteral( "</td><td>" )
                  + QStringLiteral( "%1 (%2)" ).arg( QgsPointCloudDataProvider::translatedDataFormatIds().value( originalMetadata.value( QStringLiteral( "dataformat_id" ) ).toInt() ),
                      originalMetadata.value( QStringLiteral( "dataformat_id" ) ).toString() ).trimmed()
                  + QStringLiteral( "</td></tr>\n" );
  }

  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" )
                + tr( "Scale X" ) + QStringLiteral( "</td><td>" )
                + QString::number( originalMetadata.value( QStringLiteral( "scale_x" ) ).toDouble() )
                + QStringLiteral( "</td></tr>\n" );
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" )
                + tr( "Scale Y" ) + QStringLiteral( "</td><td>" )
                + QString::number( originalMetadata.value( QStringLiteral( "scale_y" ) ).toDouble() )
                + QStringLiteral( "</td></tr>\n" );
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" )
                + tr( "Scale Z" ) + QStringLiteral( "</td><td>" )
                + QString::number( originalMetadata.value( QStringLiteral( "scale_z" ) ).toDouble() )
                + QStringLiteral( "</td></tr>\n" );

  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" )
                + tr( "Offset X" ) + QStringLiteral( "</td><td>" )
                + QString::number( originalMetadata.value( QStringLiteral( "offset_x" ) ).toDouble() )
                + QStringLiteral( "</td></tr>\n" );
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" )
                + tr( "Offset Y" ) + QStringLiteral( "</td><td>" )
                + QString::number( originalMetadata.value( QStringLiteral( "offset_y" ) ).toDouble() )
                + QStringLiteral( "</td></tr>\n" );
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" )
                + tr( "Offset Z" ) + QStringLiteral( "</td><td>" )
                + QString::number( originalMetadata.value( QStringLiteral( "offset_z" ) ).toDouble() )
                + QStringLiteral( "</td></tr>\n" );

  if ( !originalMetadata.value( QStringLiteral( "project_id" ) ).toString().isEmpty() )
  {
    myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" )
                  + tr( "Project ID" ) + QStringLiteral( "</td><td>" )
                  + originalMetadata.value( QStringLiteral( "project_id" ) ).toString()
                  + QStringLiteral( "</td></tr>\n" );
  }

  if ( !originalMetadata.value( QStringLiteral( "system_id" ) ).toString().isEmpty() )
  {
    myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" )
                  + tr( "System ID" ) + QStringLiteral( "</td><td>" )
                  + originalMetadata.value( QStringLiteral( "system_id" ) ).toString()
                  + QStringLiteral( "</td></tr>\n" );
  }

  if ( !originalMetadata.value( QStringLiteral( "software_id" ) ).toString().isEmpty() )
  {
    myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" )
                  + tr( "Software ID" ) + QStringLiteral( "</td><td>" )
                  + originalMetadata.value( QStringLiteral( "software_id" ) ).toString()
                  + QStringLiteral( "</td></tr>\n" );
  }

  // End Provider section
  myMetadata += QLatin1String( "</table>\n<br><br>" );

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

  // Attributes section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Attributes" ) + QStringLiteral( "</h1>\n<hr>\n<table class=\"list-view\">\n" );

  const QgsPointCloudAttributeCollection attrs = attributes();

  // count attributes
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Count" ) + QStringLiteral( "</td><td>" ) + QString::number( attrs.count() ) + QStringLiteral( "</td></tr>\n" );

  myMetadata += QLatin1String( "</table>\n<br><table width=\"100%\" class=\"tabular-view\">\n" );
  myMetadata += QLatin1String( "<tr><th>" ) + tr( "Attribute" ) + QLatin1String( "</th><th>" ) + tr( "Type" ) + QLatin1String( "</th></tr>\n" );

  for ( int i = 0; i < attrs.count(); ++i )
  {
    const QgsPointCloudAttribute attribute = attrs.at( i );
    QString rowClass;
    if ( i % 2 )
      rowClass = QStringLiteral( "class=\"odd-row\"" );
    myMetadata += QLatin1String( "<tr " ) + rowClass + QLatin1String( "><td>" ) + attribute.name() + QLatin1String( "</td><td>" ) + attribute.displayType() + QLatin1String( "</td></tr>\n" );
  }

  //close field list
  myMetadata += QLatin1String( "</table>\n<br><br>" );


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
    QgsDebugMsgLevel( QStringLiteral( "invoked with invalid layer or null mDataProvider" ), 3 );
    setCustomProperty( QStringLiteral( "storedSubsetString" ), subset );
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
    QgsDebugMsgLevel( QStringLiteral( "invoked with invalid layer or null mDataProvider" ), 3 );
    return customProperty( QStringLiteral( "storedSubsetString" ) ).toString();
  }
  return mDataProvider->subsetString();
}

bool QgsPointCloudLayer::convertRenderer3DFromRenderer2D()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  bool result = false;
  QgsAbstractPointCloud3DRenderer *r = static_cast<QgsAbstractPointCloud3DRenderer *>( renderer3D() );
  if ( r )
  {
    result = r->convertFrom2DRenderer( renderer() );
    setRenderer3D( r );
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
    if ( mDataProvider && mDataProvider->index() && mDataProvider->index().isValid() && mDataProvider->name() == QLatin1String( "pdal" ) && mStatistics.sampledPointsCount() != 0 )
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
  if ( !mRenderer || mRenderer->type() == QLatin1String( "extent" ) )
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
      QgsDebugError( QStringLiteral( "Transformation of extent failed!" ) );
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

    emit layerModified();
    triggerRepaint();
  }

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

  undoStack()->clear();

  mEditIndex = QgsPointCloudIndex();
  emit editingStopped();

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  if ( !mEditIndex )
    return false;

  // Cannot allow x,y,z editing as points may get moved outside the node extents
  if ( attribute.name().compare( QLatin1String( "X" ), Qt::CaseInsensitive ) == 0 ||
       attribute.name().compare( QLatin1String( "Y" ), Qt::CaseInsensitive ) == 0 ||
       attribute.name().compare( QLatin1String( "Z" ), Qt::CaseInsensitive ) == 0 )
    return false;

  if ( !n.isValid() || !mEditIndex.hasNode( n ) ) // todo: should not have to check if n.isValid
    return false;

  if ( points.isEmpty() )
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

  QVector<int> sortedPoints( points.constBegin(), points.constEnd() );
  std::sort( sortedPoints.begin(), sortedPoints.end() );
  sortedPoints.erase( std::unique( sortedPoints.begin(), sortedPoints.end() ), sortedPoints.end() );

  if ( sortedPoints.constFirst() < 0 ||
       sortedPoints.constLast() >= mEditIndex.getNode( n ).pointCount() )
    return false;

  undoStack()->push( new QgsPointCloudLayerUndoCommandChangeAttribute( mEditIndex, n, sortedPoints, attribute, value ) );

  return true;
}

QgsPointCloudIndex QgsPointCloudLayer::index() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL
  if ( mEditIndex )
    return mEditIndex;

  if ( mDataProvider )
    return mDataProvider->index();

  return QgsPointCloudIndex();
}
