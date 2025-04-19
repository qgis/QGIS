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
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QRenderTargetSelector>
#include <Qt3DRender/QRenderTarget>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/qsubtreeenabler.h>
#include <Qt3DRender/QCameraSelector>
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
  Qt3DRender::QLayerFilter *peviewLayerFilter = new Qt3DRender::QLayerFilter( mRendererEnabler );
  peviewLayerFilter->addLayer( mLayer );

  Qt3DRender::QRenderStateSet *mPreviewRenderStateSet = new Qt3DRender::QRenderStateSet( peviewLayerFilter );
  Qt3DRender::QDepthTest *mPreviewDepthTest = new Qt3DRender::QDepthTest;
  mPreviewDepthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );
  mPreviewRenderStateSet->addRenderState( mPreviewDepthTest );
  Qt3DRender::QCullFace *mPreviewCullFace = new Qt3DRender::QCullFace;
  mPreviewCullFace->setMode( Qt3DRender::QCullFace::NoCulling );
  mPreviewRenderStateSet->addRenderState( mPreviewCullFace );
}
