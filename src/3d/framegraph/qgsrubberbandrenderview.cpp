/***************************************************************************
  qgsrubberbandrenderview.cpp
  --------------------------------------
  Date                 : June 2026
  Copyright            : (C) 2026 by Benoit De Mezzo and (C) 2020 by Belgacem Nedjima
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrubberbandrenderview.h"

#include <Qt3DRender/QBlendEquation>
#include <Qt3DRender/QBlendEquationArguments>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QRenderStateSet>
#include <Qt3DRender/QRenderTargetSelector>
#include <Qt3DRender/qsubtreeenabler.h>

QgsRubberBandRenderView::QgsRubberBandRenderView( const QString &viewName, Qt3DRender::QCamera *mainCamera, Qt3DCore::QEntity *rootSceneEntity, Qt3DRender::QRenderTarget *fwdRenderTarget )
  : QgsAbstractRenderView( viewName )
  , mMainCamera( mainCamera )
{
  mLayer = new Qt3DRender::QLayer;
  mLayer->setRecursive( true );
  mLayer->setObjectName( mViewName + "::Layer" );

  // rubberband rendering pass
  constructRenderPass( fwdRenderTarget ); //#spellok

  mRubberBandsRootEntity = new Qt3DCore::QEntity( rootSceneEntity );
  mRubberBandsRootEntity->setObjectName( "mRubberBandsRootEntity" );
  mRubberBandsRootEntity->addComponent( mLayer );
}

void QgsRubberBandRenderView::constructRenderPass( Qt3DRender::QRenderTarget *fwdRenderTarget ) //#spellok
{
  Qt3DRender::QCameraSelector *cameraSelector = new Qt3DRender::QCameraSelector( mRendererEnabler );
  cameraSelector->setObjectName( mViewName + "::CameraSelector" );
  cameraSelector->setCamera( mMainCamera );

  Qt3DRender::QLayerFilter *layerFilter = new Qt3DRender::QLayerFilter( cameraSelector );
  layerFilter->addLayer( mLayer );

  Qt3DRender::QRenderStateSet *stateSet = new Qt3DRender::QRenderStateSet( layerFilter );

  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest;
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );
  stateSet->addRenderState( depthTest );

  Qt3DRender::QBlendEquationArguments *blendState = new Qt3DRender::QBlendEquationArguments;
  blendState->setSourceRgb( Qt3DRender::QBlendEquationArguments::SourceAlpha );
  blendState->setDestinationRgb( Qt3DRender::QBlendEquationArguments::OneMinusSourceAlpha );
  stateSet->addRenderState( blendState );

  Qt3DRender::QBlendEquation *blendEquation = new Qt3DRender::QBlendEquation;
  blendEquation->setBlendFunction( Qt3DRender::QBlendEquation::Add );
  stateSet->addRenderState( blendEquation );

  // Here we attach our drawings to the render target also used by forward pass.
  // This is kind of okay, but as a result, post-processing effects get applied
  // to rubber bands too. Ideally we would want them on top of everything.
  mRenderTargetSelector = new Qt3DRender::QRenderTargetSelector( stateSet );

  mRenderTargetSelector->setTarget( fwdRenderTarget );
}

Qt3DCore::QEntity *QgsRubberBandRenderView::rubberBandEntity() const
{
  return mRubberBandsRootEntity;
}

Qt3DRender::QRenderTargetSelector *QgsRubberBandRenderView::renderTargetSelector() const
{
  return mRenderTargetSelector;
};
