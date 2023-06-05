/***************************************************************************
  qgs3dwindow.cpp
  --------------------------------------
  Date                 : May 2023
  Copyright            : (C) 2023 by Jean-Baptiste Peter
  Email                : jbpeter at outlook dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <Qt3DCore/QAspectEngine>
#include <Qt3DCore/QEntity>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <Qt3DCore/QCoreAspect>
#endif
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DRender/QRenderSettings>
#include <Qt3DRender/QRenderAspect>
#include <Qt3DInput/QInputAspect>
#include <Qt3DInput/QInputSettings>
#include <Qt3DLogic/QLogicAspect>
#include <Qt3DRender/QCamera>

#include "qgs3dwindow.h"

Qgs3DWindow::Qgs3DWindow()
  : m_aspectEngine( new Qt3DCore::QAspectEngine )
  , m_renderAspect( new Qt3DRender::QRenderAspect )
  , m_inputAspect( new Qt3DInput::QInputAspect )
  , m_logicAspect( new Qt3DLogic::QLogicAspect )
  , m_renderSettings( new Qt3DRender::QRenderSettings )
  , m_forwardRenderer( new Qt3DExtras::QForwardRenderer )
  , m_defaultCamera( new Qt3DRender::QCamera )
  , m_inputSettings( new Qt3DInput::QInputSettings )
  , m_root( new Qt3DCore::QEntity )
  , m_userRoot( nullptr )
  , m_initialized( false )
{
  setSurfaceType( QSurface::OpenGLSurface );

  // register aspects
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  m_aspectEngine->registerAspect( new Qt3DCore::QCoreAspect );
#endif
  m_aspectEngine->registerAspect( m_renderAspect );
  m_aspectEngine->registerAspect( m_inputAspect );
  m_aspectEngine->registerAspect( m_logicAspect );

  m_defaultCamera->setParent( m_root );
  m_forwardRenderer->setCamera( m_defaultCamera );
  m_forwardRenderer->setSurface( this );
  m_renderSettings->setActiveFrameGraph( m_forwardRenderer );
  m_inputSettings->setEventSource( this );
}

Qgs3DWindow::~Qgs3DWindow()
{
  delete m_aspectEngine;
}

void Qgs3DWindow::setRootEntity( Qt3DCore::QEntity *root )
{
  if ( m_userRoot != root )
  {
    if ( m_userRoot != nullptr )
      m_userRoot->setParent( static_cast<Qt3DCore::QNode *>( nullptr ) );
    if ( root != nullptr )
      root->setParent( m_root );
    m_userRoot = root;
  }
}

void Qgs3DWindow::setActiveFrameGraph( Qt3DRender::QFrameGraphNode *activeFrameGraph )
{
  m_renderSettings->setActiveFrameGraph( activeFrameGraph );
}

Qt3DRender::QFrameGraphNode *Qgs3DWindow::activeFrameGraph() const
{
  return m_renderSettings->activeFrameGraph();
}

Qt3DExtras::QForwardRenderer *Qgs3DWindow::defaultFrameGraph() const
{
  return m_forwardRenderer;
}

Qt3DRender::QCamera *Qgs3DWindow::camera() const
{
  return m_defaultCamera;
}

Qt3DRender::QRenderSettings *Qgs3DWindow::renderSettings() const
{
  return m_renderSettings;
}

void Qgs3DWindow::showEvent( QShowEvent *e )
{
  if ( !m_initialized )
  {
    m_root->addComponent( m_renderSettings );
    m_root->addComponent( m_inputSettings );
    m_aspectEngine->setRootEntity( Qt3DCore::QEntityPtr( m_root ) );

    m_initialized = true;
  }
  QWindow::showEvent( e );
}

void Qgs3DWindow::resizeEvent( QResizeEvent * )
{
  m_defaultCamera->setAspectRatio( float( width() ) / std::max( 1.f, static_cast<float>( height() ) ) );
}
