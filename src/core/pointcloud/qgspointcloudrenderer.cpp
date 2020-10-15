/***************************************************************************
                         qgspointcloudrenderer.cpp
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

#include "qgspointcloudrenderer.h"
#include "qgspointcloudlayer.h"
#include "qgsrendercontext.h"

QgsPointCloudRenderer::QgsPointCloudRenderer( QgsPointCloudLayer *layer, QgsRenderContext &context )
  : QgsMapLayerRenderer( layer->id(), &context )
  , mLayer( layer )
{

}

bool QgsPointCloudRenderer::render()
{
  return true;
}


QgsPointCloudRenderer::~QgsPointCloudRenderer() = default;


void QgsPointCloudRenderer::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{

}

void QgsPointCloudRenderer::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{

}
