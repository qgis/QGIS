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

QgsPointCloudLayer::QgsPointCloudLayer( const QString &path,
                                        const QString &baseName,
                                        const QString &providerLib,
                                        const QgsPointCloudLayer::LayerOptions &options )
  : QgsMapLayer( QgsMapLayerType::PointCloudLayer, baseName, path )
{
  Q_UNUSED(providerLib)
  Q_UNUSED(options)
  setValid( loadDataSource() );
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
  return new QgsPointCloudRenderer( this, rendererContext );
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
  setValid( loadDataSource() );

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

  writeStyleManager( layerNode, doc );

  QString errorMsg;
  return writeSymbology( layerNode, doc, errorMsg, context );
}

bool QgsPointCloudLayer::readSymbology( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories )
{
  QDomElement elem = node.toElement();

  readCommonStyle( elem, context, categories );

  return true;
}

bool QgsPointCloudLayer::writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage,
    const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{

}

void QgsPointCloudLayer::setTransformContext( const QgsCoordinateTransformContext &transformContext )
{

}

QString QgsPointCloudLayer::loadDefaultStyle( bool &resultFlag )
{
  return QString();
}


bool QgsPointCloudLayer::loadDataSource()
{

  mDataProvider.reset( new QgsPointCloudDataProvider( source(), QgsDataProvider::ProviderOptions() ) );

  if ( mDataProvider->isValid() )
    setCrs( mDataProvider->crs() );

  return mDataProvider->isValid();
}
