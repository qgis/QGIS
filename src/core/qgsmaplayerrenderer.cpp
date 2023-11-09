/***************************************************************************
  qgsmaplayerrenderer.cpp
  --------------------------------------
  Date                 : August 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayerrenderer.h"
#include "qgsrendereditemdetails.h"

QgsMapLayerRenderer::~QgsMapLayerRenderer() = default;


bool QgsMapLayerRenderer::forceRasterRender() const
{
  return false;
}

Qgis::MapLayerRendererFlags QgsMapLayerRenderer::flags() const
{
  return Qgis::MapLayerRendererFlags();
}

QgsFeedback *QgsMapLayerRenderer::feedback() const
{
  return nullptr;
}

QList<QgsRenderedItemDetails *> QgsMapLayerRenderer::takeRenderedItemDetails()
{
  return std::move( mRenderedItemDetails );
}

void QgsMapLayerRenderer::appendRenderedItemDetails( QgsRenderedItemDetails *details )
{
  mRenderedItemDetails.append( details );
}
