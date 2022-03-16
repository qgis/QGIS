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
#include "qgspointcloudlayerrenderer.h"
#include "qgspointcloudindex.h"
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
#include "qgsmaplayerutils.h"

#include <QUrl>

QgsPointCloudLayer::QgsPointCloudLayer( const QString &uri,
                                        const QString &baseName,
                                        const QString &providerLib,
                                        const QgsPointCloudLayer::LayerOptions &options )
  : QgsMapLayer( QgsMapLayerType::PointCloudLayer, baseName, uri )
  , mElevationProperties( new QgsPointCloudLayerElevationProperties( this ) )
{
  if ( !uri.isEmpty() && !providerLib.isEmpty() )
  {
    const QgsDataProvider::ProviderOptions providerOptions { options.transformContext };
    QgsDataProvider::ReadFlags providerFlags = QgsDataProvider::ReadFlags();
    if ( options.loadDefaultStyle )
    {
      providerFlags |= QgsDataProvider::FlagLoadDefaultStyle;
    }
    setDataSource( uri, baseName, providerLib, providerOptions, providerFlags );

    if ( !options.skipIndexGeneration && mDataProvider && mDataProvider->isValid() )
      mDataProvider.get()->generateIndex();
  }

  setLegend( QgsMapLayerLegend::defaultPointCloudLegend( this ) );
  connect( this, &QgsPointCloudLayer::subsetStringChanged, this, &QgsMapLayer::configChanged );
}

QgsPointCloudLayer::~QgsPointCloudLayer() = default;

QgsPointCloudLayer *QgsPointCloudLayer::clone() const
{
  LayerOptions options;
  options.loadDefaultStyle = false;
  options.transformContext = transformContext();
  options.skipCrsValidation = true;

  QgsPointCloudLayer *layer = new QgsPointCloudLayer( source(), name(), mProviderKey, options );
  QgsMapLayer::clone( layer );

  if ( mRenderer )
    layer->setRenderer( mRenderer->clone() );

  return layer;
}

QgsRectangle QgsPointCloudLayer::extent() const
{
  if ( !mDataProvider )
    return QgsRectangle();

  return mDataProvider->extent();
}

QgsMapLayerRenderer *QgsPointCloudLayer::createMapRenderer( QgsRenderContext &rendererContext )
{
  return new QgsPointCloudLayerRenderer( this, rendererContext );
}

QgsPointCloudDataProvider *QgsPointCloudLayer::dataProvider()
{
  return mDataProvider.get();
}

const QgsPointCloudDataProvider *QgsPointCloudLayer::dataProvider() const
{
  return mDataProvider.get();
}

bool QgsPointCloudLayer::readXml( const QDomNode &layerNode, QgsReadWriteContext &context )
{
  // create provider
  const QDomNode pkeyNode = layerNode.namedItem( QStringLiteral( "provider" ) );
  mProviderKey = pkeyNode.toElement().text();

  if ( !( mReadFlags & QgsMapLayer::FlagDontResolveLayers ) )
  {
    const QgsDataProvider::ProviderOptions providerOptions { context.transformContext() };
    QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags();
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

        // skip get extent
        flags |= QgsDataProvider::SkipGetExtent;
      }
    }
    if ( mReadFlags & QgsMapLayer::FlagTrustLayerMetadata )
    {
      flags |= QgsDataProvider::FlagTrustDataSource;
    }
    setDataSource( mDataSource, mLayerName, mProviderKey, providerOptions, flags );
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
  QDomElement mapLayerNode = layerNode.toElement();
  mapLayerNode.setAttribute( QStringLiteral( "type" ), QgsMapLayerFactory::typeToString( QgsMapLayerType::PointCloudLayer ) );

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
  const QDomElement elem = node.toElement();

  readCommonStyle( elem, context, categories );

  readStyle( node, errorMessage, context, categories );

  if ( categories.testFlag( CustomProperties ) )
    readCustomProperties( node, QStringLiteral( "variable" ) );

  return true;
}

bool QgsPointCloudLayer::readStyle( const QDomNode &node, QString &, QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories )
{
  bool result = true;

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
      setRenderer( QgsPointCloudRendererRegistry::defaultRenderer( mDataProvider.get() ) );
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
  Q_UNUSED( errorMessage )

  QDomElement elem = node.toElement();
  writeCommonStyle( elem, doc, context, categories );

  ( void )writeStyle( node, doc, errorMessage, context, categories );

  return true;
}

bool QgsPointCloudLayer::writeStyle( QDomNode &node, QDomDocument &doc, QString &, const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  QDomElement mapLayerNode = node.toElement();

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
    const QDomText blendModeText = doc.createTextNode( QString::number( QgsPainting::getBlendModeEnum( blendMode() ) ) );
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
  if ( mDataProvider )
    mDataProvider->setTransformContext( transformContext );
  invalidateWgs84Extent();
}

void QgsPointCloudLayer::setDataSourcePrivate( const QString &dataSource, const QString &baseName, const QString &provider,
    const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  if ( mDataProvider )
  {
    disconnect( mDataProvider.get(), &QgsPointCloudDataProvider::dataChanged, this, &QgsPointCloudLayer::dataChanged );
    disconnect( mDataProvider.get(), &QgsPointCloudDataProvider::indexGenerationStateChanged, this, &QgsPointCloudLayer::onPointCloudIndexGenerationStateChanged );
  }

  setName( baseName );
  mProviderKey = provider;
  mDataSource = dataSource;

  mDataProvider.reset( qobject_cast<QgsPointCloudDataProvider *>( QgsProviderRegistry::instance()->createProvider( provider, dataSource, options, flags ) ) );
  if ( !mDataProvider )
  {
    QgsDebugMsg( QStringLiteral( "Unable to get point cloud data provider" ) );
    setValid( false );
    return;
  }

  mDataProvider->setParent( this );
  QgsDebugMsgLevel( QStringLiteral( "Instantiated the point cloud data provider plugin" ), 2 );

  setValid( mDataProvider->isValid() );
  if ( !isValid() )
  {
    QgsDebugMsg( QStringLiteral( "Invalid point cloud provider plugin %1" ).arg( QString( mDataSource.toUtf8() ) ) );
    return;
  }

  connect( mDataProvider.get(), &QgsPointCloudDataProvider::indexGenerationStateChanged, this, &QgsPointCloudLayer::onPointCloudIndexGenerationStateChanged );
  connect( mDataProvider.get(), &QgsPointCloudDataProvider::dataChanged, this, &QgsPointCloudLayer::dataChanged );

  // Load initial extent, crs and renderer
  setCrs( mDataProvider->crs() );
  if ( !( flags & QgsDataProvider::SkipGetExtent ) )
  {
    setExtent( mDataProvider->extent() );
  }

  bool loadDefaultStyleFlag = false;
  if ( flags & QgsDataProvider::FlagLoadDefaultStyle )
  {
    loadDefaultStyleFlag = true;
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
      setRenderer( QgsPointCloudRendererRegistry::defaultRenderer( mDataProvider.get() ) );
    }
  }
}

QString QgsPointCloudLayer::encodedSource( const QString &source, const QgsReadWriteContext &context ) const
{
  QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( providerType(), source );
  if ( parts.contains( QStringLiteral( "path" ) ) )
  {
    parts.insert( QStringLiteral( "path" ), context.pathResolver().writePath( parts.value( QStringLiteral( "path" ) ).toString() ) );
    return QgsProviderRegistry::instance()->encodeUri( providerType(), parts );
  }
  else
  {
    return source;
  }
}

QString QgsPointCloudLayer::decodedSource( const QString &source, const QString &dataProvider, const QgsReadWriteContext &context ) const
{
  QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( dataProvider, source );
  if ( parts.contains( QStringLiteral( "path" ) ) )
  {
    parts.insert( QStringLiteral( "path" ), context.pathResolver().readPath( parts.value( QStringLiteral( "path" ) ).toString() ) );
    return QgsProviderRegistry::instance()->encodeUri( dataProvider, parts );
  }
  else
  {
    return source;
  }
}

void QgsPointCloudLayer::onPointCloudIndexGenerationStateChanged( QgsPointCloudDataProvider::PointCloudIndexGenerationState state )
{
  if ( state == QgsPointCloudDataProvider::Indexed )
  {
    mDataProvider.get()->loadIndex();
    if ( mRenderer->type() == QLatin1String( "extent" ) )
    {
      setRenderer( QgsPointCloudRendererRegistry::defaultRenderer( mDataProvider.get() ) );
    }
    triggerRepaint();

    emit rendererChanged();
  }
}

QString QgsPointCloudLayer::loadDefaultStyle( bool &resultFlag )
{
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

  myMetadata += QLatin1String( "\n</body>\n</html>\n" );
  return myMetadata;
}

QgsMapLayerElevationProperties *QgsPointCloudLayer::elevationProperties()
{
  return mElevationProperties;
}

QgsPointCloudAttributeCollection QgsPointCloudLayer::attributes() const
{
  return mDataProvider ? mDataProvider->attributes() : QgsPointCloudAttributeCollection();
}

qint64 QgsPointCloudLayer::pointCount() const
{
  return mDataProvider ? mDataProvider->pointCount() : 0;
}

QgsPointCloudRenderer *QgsPointCloudLayer::renderer()
{
  return mRenderer.get();
}

const QgsPointCloudRenderer *QgsPointCloudLayer::renderer() const
{
  return mRenderer.get();
}

void QgsPointCloudLayer::setRenderer( QgsPointCloudRenderer *renderer )
{
  if ( renderer == mRenderer.get() )
    return;

  mRenderer.reset( renderer );
  emit rendererChanged();
  emitStyleChanged();
}

bool QgsPointCloudLayer::setSubsetString( const QString &subset )
{
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
  if ( !isValid() || !mDataProvider )
  {
    QgsDebugMsgLevel( QStringLiteral( "invoked with invalid layer or null mDataProvider" ), 3 );
    return customProperty( QStringLiteral( "storedSubsetString" ) ).toString();
  }
  return mDataProvider->subsetString();
}
