/***************************************************************************
  qgsdebugtexturerenderview.cpp
  --------------------------------------
  Date                 : June 2024
  Copyright            : (C) 2024 by Benoit De Mezzo
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdebugtexturerenderview.h"
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/qsubtreeenabler.h>
#include <Qt3DRender/QRenderStateSet>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QCullFace>


QgsDebugTextureRenderView::QgsDebugTextureRenderView( const QString &viewName )
  : QgsAbstractRenderView( viewName )
{
  mLayer = new Qt3DRender::QLayer;
  mLayer->setRecursive( true );
  mLayer->setObjectName( mViewName + "::Layer" );

  // debug rendering pass
  buildRenderPass();
}

Qt3DRender::QLayer *QgsDebugTextureRenderView::debugLayer() const
{
  return mLayer;
}

void QgsDebugTextureRenderView::buildRenderPass()
{
  Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter( mRendererEnabler );
  layerFilter->addLayer( mLayer );

  Qt3DRender::QRenderStateSet *renderStateSet = new Qt3DRender::QRenderStateSet( layerFilter );
  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest;
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );
  renderStateSet->addRenderState( depthTest );
  Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
  cullFace->setMode( Qt3DRender::QCullFace::NoCulling );
  renderStateSet->addRenderState( cullFace );
}
