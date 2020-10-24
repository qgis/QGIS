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
  QgsPointCloudLayer *layer = new QgsPointCloudLayer( source(), name() );
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
  Q_UNUSED( errorMessage )

  QDomElement elem = node.toElement();

  readCommonStyle( elem, context, categories );

  return true;
}

bool QgsPointCloudLayer::writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage,
    const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  Q_UNUSED( errorMessage )
  Q_UNUSED( context )
  Q_UNUSED( categories )
  Q_UNUSED( node )
  Q_UNUSED( doc )
  return false;
}

void QgsPointCloudLayer::setTransformContext( const QgsCoordinateTransformContext &transformContext )
{
  if ( mDataProvider )
    mDataProvider->setTransformContext( transformContext );
}

QString QgsPointCloudLayer::loadDefaultStyle( bool &resultFlag )
{
  Q_UNUSED( resultFlag )
  return QString();
}

void QgsPointCloudLayer::setDataSource( const QString &dataSource, const QString &baseName, const QString &provider, const QgsDataProvider::ProviderOptions &options, bool loadDefaultStyleFlag )
{
  Q_UNUSED( loadDefaultStyleFlag )

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

  connect( mDataProvider.get(), &QgsPointCloudDataProvider::dataChanged, this, &QgsPointCloudLayer::dataChanged );

  emit dataSourceChanged();
  triggerRepaint();
}
