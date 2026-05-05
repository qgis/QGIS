/***************************************************************************
  qgsmaterialpreviewwidget.cpp
  --------------------------------------
  Date                 : March 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaterialpreviewwidget.h"

#include "qgs3d.h"
#include "qgsabstractmaterialsettings.h"
#include "qgsapplication.h"
#include "qgsmaterialregistry.h"

#include <QActionGroup>
#include <QMenu>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <Qt3DCore/QAspectEngine>
#include <Qt3DCore/QCoreAspect>
#include <Qt3DExtras/QForwardRenderer>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DInput/QInputAspect>
#include <Qt3DInput/QInputSettings>
#include <Qt3DLogic/QFrameAction>
#include <Qt3DLogic/QLogicAspect>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QRenderAspect>
#include <Qt3DRender/QRenderSettings>

#include "moc_qgsmaterialpreviewwidget.cpp"

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
  m_aspectEngine->registerAspect( new Qt3DCore::QCoreAspect );
  m_aspectEngine->registerAspect( m_renderAspect );
  m_aspectEngine->registerAspect( m_inputAspect );
  m_aspectEngine->registerAspect( m_logicAspect );

  m_defaultCamera->setParent( m_root );
  m_forwardRenderer->setCamera( m_defaultCamera );
  m_forwardRenderer->setSurface( this );
  m_renderSettings->setActiveFrameGraph( m_forwardRenderer );
  m_inputSettings->setEventSource( this );

  setSurfaceType( QSurface::OpenGLSurface );
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

Qt3DRender::QCamera *Qgs3DWindow::camera() const
{
  return m_defaultCamera;
}

Qt3DExtras::QForwardRenderer *Qgs3DWindow::defaultFrameGraph() const
{
  return m_forwardRenderer;
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


QgsMaterialPreviewWidget::QgsMaterialPreviewWidget( QWidget *parent )
  : QWidget( parent )
{
  // defer initialization until showEvent!
}

void QgsMaterialPreviewWidget::setMaterialType( const QString &type )
{
  if ( const QgsMaterialSettingsMetadata *metadata = dynamic_cast< const QgsMaterialSettingsMetadata * >( QgsApplication::materialRegistry()->materialSettingsMetadata( type ) ) )
  {
    if ( const QgsAbstractMaterial3DHandler *handler = metadata->handler() )
    {
      mMeshTypes = handler->previewMeshTypes();
      mPreviewSceneType = mMeshTypes.at( 0 ).type;
    }
  }
}

void QgsMaterialPreviewWidget::setupCamera( Qt3DRender::QCamera *camera )
{
  camera->lens()->setPerspectiveProjection( 45.0f, 1.0f, 0.1f, 100.0f );
  camera->setPosition( { 0, 0, 4 } );
  camera->setViewCenter( { 0, 0, 0 } );
}

void QgsMaterialPreviewWidget::updatePreview( const QgsAbstractMaterialSettings *settings )
{
  mLastPreviewSettings.reset( settings->clone() );
  if ( !mView )
    return;

  const QgsAbstractMaterial3DHandler *handler = Qgs3D::handlerForMaterialSettings( mLastPreviewSettings.get() );
  if ( !handler )
    return;

  QgsMaterialContext context;
  if ( !mPreviewScene )
  {
    delete mPreviewScene;
    mPreviewScene = handler->createPreviewScene( mLastPreviewSettings.get(), mPreviewSceneType, context, mView, mSceneRoot );
  }
  else
  {
    if ( !handler->updatePreviewScene( mPreviewScene, mLastPreviewSettings.get(), context ) )
    {
      delete mPreviewScene;
      mPreviewScene = handler->createPreviewScene( mLastPreviewSettings.get(), mPreviewSceneType, context, mView, mSceneRoot );
    }
  }
}

bool QgsMaterialPreviewWidget::eventFilter( QObject *watched, QEvent *event )
{
  if ( watched == mView && event->type() == QEvent::MouseButtonPress )
  {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
    if ( mouseEvent->button() == Qt::RightButton )
    {
      QMenu menu( this );
      auto actionGroup = new QActionGroup( &menu );
      actionGroup->setExclusive( true );
      for ( const QgsAbstractMaterial3DHandler::PreviewMeshType &type : mMeshTypes )
      {
        QAction *action = new QAction( type.displayName, &menu );
        action->setCheckable( true );
        action->setChecked( type.type == mPreviewSceneType );
        connect( action, &QAction::toggled, this, [this, type]( bool checked ) {
          if ( checked )
          {
            mPreviewSceneType = type.type;
            mPreviewScene->deleteLater();
            mPreviewScene = nullptr;
            updatePreview( mLastPreviewSettings.get() );
          }
        } );
        menu.addAction( action );
        actionGroup->addAction( action );
      }
      menu.exec( mouseEvent->globalPosition().toPoint() );
      return true;
    }
  }
  return QWidget::eventFilter( watched, event );
}

void QgsMaterialPreviewWidget::showEvent( QShowEvent *e )
{
  if ( mView )
    return;

  mView = new Qgs3DWindow();
  mView->defaultFrameGraph()->setClearColor( palette().color( QPalette::ColorGroup::Active, QPalette::ColorRole::Window ) );

  mView->installEventFilter( this );

  QWidget *container = QWidget::createWindowContainer( mView, this );
  container->setMinimumSize( 200, 200 );
  container->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

  auto *layout = new QVBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( container );
  setLayout( layout );

  mSceneRoot = new Qt3DCore::QEntity;
  setupCamera( mView->camera() );
  mView->setRootEntity( mSceneRoot );
  QWidget::showEvent( e );

  if ( mLastPreviewSettings )
  {
    updatePreview( mLastPreviewSettings.get() );
  }
}
