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
#include "qgspointcloudrenderer.h"
#include "qgspointcloudindex.h"
#include "qgsrectangle.h"
#include "qgspointclouddataprovider.h"
#include "qgsproviderregistry.h"
#include "qgslogger.h"
#include "qgslayermetadataformatter.h"

QgsPointCloudLayer::QgsPointCloudLayer( const QString &path,
                                        const QString &baseName,
                                        const QString &providerLib,
                                        const QgsPointCloudLayer::LayerOptions &options )
  : QgsMapLayer( QgsMapLayerType::PointCloudLayer, baseName, path )
{
  if ( !path.isEmpty() && !providerLib.isEmpty() )
  {
    QgsDataProvider::ProviderOptions providerOptions { options.transformContext };
    setDataSource( path, baseName, providerLib, providerOptions, options.loadDefaultStyle );
  }
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
  return layer;
}

QgsRectangle QgsPointCloudLayer::extent() const
{
  if ( !mDataProvider )
    return QgsRectangle();

  return mDataProvider->index()->extent();
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
  QDomNode pkeyNode = layerNode.namedItem( QStringLiteral( "provider" ) );
  mProviderKey = pkeyNode.toElement().text();

  if ( !( mReadFlags & QgsMapLayer::FlagDontResolveLayers ) )
  {
    QgsDataProvider::ProviderOptions providerOptions { context.transformContext() };
    setDataSource( mDataSource, mLayerName, mProviderKey, providerOptions, false );
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
  mapLayerNode.setAttribute( QStringLiteral( "type" ), QStringLiteral( "point-cloud" ) );

  if ( mDataProvider )
  {
    QDomElement provider  = doc.createElement( QStringLiteral( "provider" ) );
    QDomText providerText = doc.createTextNode( providerType() );
    provider.appendChild( providerText );
    layerNode.appendChild( provider );
  }

  writeStyleManager( layerNode, doc );

  QString errorMsg;
  return writeSymbology( layerNode, doc, errorMsg, context );
}

bool QgsPointCloudLayer::readSymbology( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories )
{
  QDomElement elem = node.toElement();

  readCommonStyle( elem, context, categories );

  // hack for now !!
  if ( categories.testFlag( Symbology ) )
  {
    const QDomElement elemRenderer = elem.firstChildElement( QStringLiteral( "renderer" ) );
    if ( elemRenderer.isNull() )
    {
      errorMessage = tr( "Missing <renderer> tag" );
      //  return false;
    }
    setCustomProperty( QStringLiteral( "pcMin" ), elemRenderer.attribute( QStringLiteral( "pcMin" ), QStringLiteral( "400" ) ).toInt() );
    setCustomProperty( QStringLiteral( "pcMax" ), elemRenderer.attribute( QStringLiteral( "pcMax" ), QStringLiteral( "600" ) ).toInt() );
    setCustomProperty( QStringLiteral( "pcRamp" ), elemRenderer.attribute( QStringLiteral( "pcRamp" ), QStringLiteral( "Viridis" ) ) );
  }

  return true;
}

bool QgsPointCloudLayer::writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage,
    const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  Q_UNUSED( errorMessage )

  QDomElement elem = node.toElement();
  writeCommonStyle( elem, doc, context, categories );

  // hack for now !!
  if ( categories.testFlag( Symbology ) )
  {
    QDomElement elemRenderer = doc.createElement( QStringLiteral( "renderer" ) );
    elemRenderer.setAttribute( QStringLiteral( "pcMin" ), customProperty( QStringLiteral( "pcMin" ), 400 ).toInt() );
    elemRenderer.setAttribute( QStringLiteral( "pcMax" ), customProperty( QStringLiteral( "pcMax" ), 600 ).toInt() );
    elemRenderer.setAttribute( QStringLiteral( "pcRamp" ), customProperty( QStringLiteral( "pcRamp" ),  QStringLiteral( "Viridis" ) ).toString() );
    elem.appendChild( elemRenderer );
  }

  return true;
}

void QgsPointCloudLayer::setTransformContext( const QgsCoordinateTransformContext &transformContext )
{
  if ( mDataProvider )
    mDataProvider->setTransformContext( transformContext );
}

void QgsPointCloudLayer::setDataSource( const QString &dataSource, const QString &baseName, const QString &provider, const QgsDataProvider::ProviderOptions &options, bool loadDefaultStyleFlag )
{
  if ( mDataProvider )
    disconnect( mDataProvider.get(), &QgsPointCloudDataProvider::dataChanged, this, &QgsPointCloudLayer::dataChanged );

  setName( baseName );
  mProviderKey = provider;
  mDataSource = dataSource;

  QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags();
  if ( mReadFlags & QgsMapLayer::FlagTrustLayerMetadata )
  {
    flags |= QgsDataProvider::FlagTrustDataSource;
  }

  mDataProvider.reset( qobject_cast<QgsPointCloudDataProvider *>( QgsProviderRegistry::instance()->createProvider( provider, dataSource, options, flags ) ) );
  if ( !mDataProvider )
  {
    QgsDebugMsg( QStringLiteral( "Unable to get point cloud data provider" ) );
    setValid( false );
    emit dataSourceChanged();
    return;
  }

  mDataProvider->setParent( this );
  QgsDebugMsgLevel( QStringLiteral( "Instantiated the point cloud data provider plugin" ), 2 );

  setValid( mDataProvider->isValid() );
  if ( !isValid() )
  {
    QgsDebugMsg( QStringLiteral( "Invalid point cloud provider plugin %1" ).arg( QString( mDataSource.toUtf8() ) ) );
    emit dataSourceChanged();
    return;
  }

  setCrs( mDataProvider->crs() );

  if ( loadDefaultStyleFlag )
  {
    bool defaultLoadedFlag = false;
    loadDefaultStyle( defaultLoadedFlag );
  }

  connect( mDataProvider.get(), &QgsPointCloudDataProvider::dataChanged, this, &QgsPointCloudLayer::dataChanged );

  emit dataSourceChanged();
  triggerRepaint();
}

QString QgsPointCloudLayer::htmlMetadata() const
{
  QgsLayerMetadataFormatter htmlFormatter( metadata() );
  QString myMetadata = QStringLiteral( "<html>\n<body>\n" );

  // Begin Provider section
  myMetadata += QStringLiteral( "<h1>" ) + tr( "Information from provider" ) + QStringLiteral( "</h1>\n<hr>\n" );
  myMetadata += QLatin1String( "<table class=\"list-view\">\n" );

  // name
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Name" ) + QStringLiteral( "</td><td>" ) + name() + QStringLiteral( "</td></tr>\n" );

  // local path
  QVariantMap uriComponents = QgsProviderRegistry::instance()->decodeUri( mProviderKey, publicSource() );
  QString path;
  if ( uriComponents.contains( QStringLiteral( "path" ) ) )
  {
    path = uriComponents[QStringLiteral( "path" )].toString();
    if ( QFile::exists( path ) )
      myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Path" ) + QStringLiteral( "</td><td>%1" ).arg( QStringLiteral( "<a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( path ).toString(), QDir::toNativeSeparators( path ) ) ) + QStringLiteral( "</td></tr>\n" );
  }
  if ( uriComponents.contains( QStringLiteral( "url" ) ) )
  {
    const QString url = uriComponents[QStringLiteral( "url" )].toString();
    myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "URL" ) + QStringLiteral( "</td><td>%1" ).arg( QStringLiteral( "<a href=\"%1\">%2</a>" ).arg( QUrl( url ).toString(), url ) ) + QStringLiteral( "</td></tr>\n" );
  }

  // data source
  if ( publicSource() != path )
    myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Source" ) + QStringLiteral( "</td><td>%1" ).arg( publicSource() ) + QStringLiteral( "</td></tr>\n" );

  // EPSG
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "CRS" ) + QStringLiteral( "</td><td>" );
  if ( crs().isValid() )
  {
    myMetadata += crs().userFriendlyIdentifier( QgsCoordinateReferenceSystem::FullString ) + QStringLiteral( " - " );
    if ( crs().isGeographic() )
      myMetadata += tr( "Geographic" );
    else
      myMetadata += tr( "Projected" );
  }
  myMetadata += QLatin1String( "</td></tr>\n" );

  // Extent
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Extent" ) + QStringLiteral( "</td><td>" ) + extent().toString() + QStringLiteral( "</td></tr>\n" );

  // unit
  myMetadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Unit" ) + QStringLiteral( "</td><td>" ) + QgsUnitTypes::toString( crs().mapUnits() ) + QStringLiteral( "</td></tr>\n" );

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
