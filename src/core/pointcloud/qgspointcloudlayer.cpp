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

QgsPointCloudLayer::QgsPointCloudLayer( const QString &path, const QString &baseName )
  : QgsMapLayer( QgsMapLayerType::PointCloudLayer, path, baseName )
{

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
  return QgsRectangle();
}

QgsMapLayerRenderer *QgsPointCloudLayer::createMapRenderer( QgsRenderContext &rendererContext )
{
  return new QgsPointCloudRenderer( this, rendererContext );
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

QgsPointCloudIndex *QgsPointCloudLayer::pointCloudIndex() const
{
  return mPointCloudIndex;
}

bool QgsPointCloudLayer::loadDataSource()
{
  return false;
}
