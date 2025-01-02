/***************************************************************************
  qgswindow3dengine.cpp
  --------------------------------------
  Date                 : July 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswindow3dengine.h"
#include "moc_qgswindow3dengine.cpp"

#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DRender/QRenderSettings>

#include "qgs3dmapcanvas.h"
#include "qgsframegraph.h"


QgsWindow3DEngine::QgsWindow3DEngine( Qgs3DMapCanvas *parent )
  : QgsAbstract3DEngine( parent )
{
  mMapCanvas3D = parent;

  mRoot = new Qt3DCore::QEntity;
  mMapCanvas3D->setRootEntity( mRoot );

  mFrameGraph = new QgsFrameGraph( mMapCanvas3D, QSize( 1024, 768 ), mMapCanvas3D->camera(), mRoot );
  mMapCanvas3D->setActiveFrameGraph( mFrameGraph->frameGraphRoot() );

  // force switching to no shadow rendering
  setShadowRenderingEnabled( false );
}

QWindow *QgsWindow3DEngine::window()
{
  return mMapCanvas3D;
}

Qt3DCore::QEntity *QgsWindow3DEngine::root() const
{
  return mRoot;
}

void QgsWindow3DEngine::setShadowRenderingEnabled( bool enabled )
{
  mShadowRenderingEnabled = enabled;
  mFrameGraph->setShadowRenderingEnabled( mShadowRenderingEnabled );
}

void QgsWindow3DEngine::setClearColor( const QColor &color )
{
  mFrameGraph->setClearColor( color );
}

void QgsWindow3DEngine::setFrustumCullingEnabled( bool enabled )
{
  // Not sure if this works properly
  mFrameGraph->setFrustumCullingEnabled( enabled );
}

void QgsWindow3DEngine::setRootEntity( Qt3DCore::QEntity *root )
{
  mSceneRoot = root;
  mSceneRoot->setParent( mRoot );
  mSceneRoot->addComponent( mFrameGraph->forwardRenderLayer() );
  mSceneRoot->addComponent( mFrameGraph->castShadowsLayer() );
}

Qt3DRender::QRenderSettings *QgsWindow3DEngine::renderSettings()
{
  return mMapCanvas3D->renderSettings();
}

Qt3DRender::QCamera *QgsWindow3DEngine::camera()
{
  return mMapCanvas3D->camera();
}

QSize QgsWindow3DEngine::size() const
{
  return mMapCanvas3D->size();
}

QSurface *QgsWindow3DEngine::surface() const
{
  return mMapCanvas3D;
}

void QgsWindow3DEngine::setSize( QSize s )
{
  mSize = s;

  mMapCanvas3D->setWidth( mSize.width() );
  mMapCanvas3D->setHeight( mSize.height() );
  mFrameGraph->setSize( mSize );
  camera()->setAspectRatio( float( mSize.width() ) / float( mSize.height() ) );
  emit sizeChanged();
}
