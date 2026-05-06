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
#include "qgs3dutils.h"
#include "qgsabstractmaterialsettings.h"
#include "qgsapplication.h"
#include "qgsmaterialregistry.h"
#include "qgssettingsentryenumflag.h"

#include <QActionGroup>
#include <QMenu>
#include <QMouseEvent>
#include <QString>
#include <QVBoxLayout>
#include <Qt3DCore/QAspectEngine>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QCoreAspect>
#include <Qt3DCore/QGeometry>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QRenderAspect>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QRenderSettings>
#include <Qt3DRender/QRenderSurfaceSelector>
#include <Qt3DRender/QRenderTarget>
#include <Qt3DRender/QRenderTargetOutput>
#include <Qt3DRender/QRenderTargetSelector>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QViewport>

#include "moc_qgsmaterialpreviewwidget.cpp"

using namespace Qt::StringLiterals;

QgsMaterialPreview3DWindow::QgsMaterialPreview3DWindow()
  : m_aspectEngine( new Qt3DCore::QAspectEngine )
  , m_renderAspect( new Qt3DRender::QRenderAspect )
  , m_renderSettings( new Qt3DRender::QRenderSettings )
  , m_defaultCamera( new Qt3DRender::QCamera )
  , m_root( new Qt3DCore::QEntity )
  , m_userRoot( nullptr )
  , m_initialized( false )
{
  m_aspectEngine->registerAspect( new Qt3DCore::QCoreAspect );
  m_aspectEngine->registerAspect( m_renderAspect );

  m_defaultCamera->setParent( m_root );

  setSurfaceType( QSurface::OpenGLSurface );

  setupFrameGraph();
  setupPostProcessQuad();

  setSurfaceType( QSurface::OpenGLSurface );
}

QgsMaterialPreview3DWindow::~QgsMaterialPreview3DWindow()
{
  delete m_aspectEngine;
}

void QgsMaterialPreview3DWindow::setRootEntity( Qt3DCore::QEntity *root )
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

Qt3DRender::QCamera *QgsMaterialPreview3DWindow::camera() const
{
  return m_defaultCamera;
}

Qt3DRender::QLayer *QgsMaterialPreview3DWindow::sceneLayer()
{
  return m_sceneLayer;
}

void QgsMaterialPreview3DWindow::showEvent( QShowEvent *e )
{
  if ( !m_initialized )
  {
    m_root->addComponent( m_renderSettings );
    m_aspectEngine->setRootEntity( Qt3DCore::QEntityPtr( m_root ) );

    m_initialized = true;
  }
  QWindow::showEvent( e );
}

void QgsMaterialPreview3DWindow::resizeEvent( QResizeEvent *e )
{
  m_defaultCamera->setAspectRatio( float( width() ) / std::max( 1.f, static_cast<float>( height() ) ) );

  if ( m_colorTexture )
  {
    const int w = static_cast< int >( width() * devicePixelRatio() );
    const int h = static_cast< int >( height() * devicePixelRatio() );
    m_colorTexture->setWidth( w );
    m_colorTexture->setHeight( h );
    m_depthTexture->setWidth( w );
    m_depthTexture->setHeight( h );
  }
  QWindow::resizeEvent( e );
}

void QgsMaterialPreview3DWindow::setupFrameGraph()
{
  m_surfaceSelector = new Qt3DRender::QRenderSurfaceSelector( m_root );
  m_surfaceSelector->setSurface( this );

  Qt3DRender::QViewport *viewport = new Qt3DRender::QViewport( m_surfaceSelector );
  viewport->setNormalizedRect( QRectF( 0.0f, 0.0f, 1.0f, 1.0f ) );

  m_sceneLayer = new Qt3DRender::QLayer( m_root );
  m_sceneLayer->setRecursive( true );
  m_quadLayer = new Qt3DRender::QLayer( m_root );

  Qt3DRender::QLayerFilter *sceneFilter = new Qt3DRender::QLayerFilter( viewport );
  sceneFilter->addLayer( m_sceneLayer );

  Qt3DRender::QRenderTargetSelector *rtSelector = new Qt3DRender::QRenderTargetSelector( sceneFilter );
  Qt3DRender::QRenderTarget *renderTarget = new Qt3DRender::QRenderTarget( rtSelector );

  m_colorTexture = new Qt3DRender::QTexture2D( renderTarget );
  m_colorTexture->setFormat( Qt3DRender::QAbstractTexture::RGBA8_UNorm );
  m_colorTexture->setGenerateMipMaps( false );
  m_colorTexture->setWidth( 1 );
  m_colorTexture->setHeight( 1 );

  Qt3DRender::QRenderTargetOutput *colorOutput = new Qt3DRender::QRenderTargetOutput( renderTarget );
  colorOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Color0 );
  colorOutput->setTexture( m_colorTexture );
  renderTarget->addOutput( colorOutput );

  m_depthTexture = new Qt3DRender::QTexture2D( renderTarget );
  m_depthTexture->setFormat( Qt3DRender::QAbstractTexture::D24 );
  m_depthTexture->setGenerateMipMaps( false );
  m_depthTexture->setWidth( 1 );
  m_depthTexture->setHeight( 1 );

  Qt3DRender::QRenderTargetOutput *depthOutput = new Qt3DRender::QRenderTargetOutput( renderTarget );
  depthOutput->setAttachmentPoint( Qt3DRender::QRenderTargetOutput::Depth );
  depthOutput->setTexture( m_depthTexture );
  renderTarget->addOutput( depthOutput );

  rtSelector->setTarget( renderTarget );

  Qt3DRender::QClearBuffers *sceneClear = new Qt3DRender::QClearBuffers( rtSelector );
  sceneClear->setBuffers( Qt3DRender::QClearBuffers::ColorDepthBuffer );

  // background color should match default window background color
  sceneClear->setClearColor( Qgs3DUtils::srgbToLinear( QApplication::palette().color( QPalette::ColorGroup::Active, QPalette::ColorRole::Window ) ) );

  Qt3DRender::QCameraSelector *cameraSelector = new Qt3DRender::QCameraSelector( sceneClear );
  cameraSelector->setCamera( m_defaultCamera );

  Qt3DRender::QLayerFilter *quadFilter = new Qt3DRender::QLayerFilter( viewport );
  quadFilter->addLayer( m_quadLayer );

  Qt3DRender::QClearBuffers *quadClear = new Qt3DRender::QClearBuffers( quadFilter );
  quadClear->setBuffers( Qt3DRender::QClearBuffers::ColorDepthBuffer );
  quadClear->setClearColor( QColor( Qt::black ) );

  m_renderSettings->setActiveFrameGraph( m_surfaceSelector );
}

void QgsMaterialPreview3DWindow::setupPostProcessQuad()
{
  m_quadEntity = new Qt3DCore::QEntity( m_root );
  m_quadEntity->addComponent( m_quadLayer );

  Qt3DRender::QGeometryRenderer *customQuad = new Qt3DRender::QGeometryRenderer( m_quadEntity );
  Qt3DCore::QGeometry *geometry = new Qt3DCore::QGeometry( customQuad );
  Qt3DCore::QBuffer *vertexBuffer = new Qt3DCore::QBuffer( geometry );

  QByteArray vertexData;
  vertexData.resize( static_cast< qsizetype >( 4 * 3 * static_cast< int >( sizeof( float ) ) ) );
  float *v = reinterpret_cast<float *>( vertexData.data() );
  *v++ = -1.0f;
  *v++ = -1.0f;
  *v++ = 0.0f;
  *v++ = 1.0f;
  *v++ = -1.0f;
  *v++ = 0.0f;
  *v++ = -1.0f;
  *v++ = 1.0f;
  *v++ = 0.0f;
  *v++ = 1.0f;
  *v++ = 1.0f;
  *v++ = 0.0f;
  vertexBuffer->setData( vertexData );

  Qt3DCore::QAttribute *posAttr = new Qt3DCore::QAttribute( geometry );
  posAttr->setName( Qt3DCore::QAttribute::defaultPositionAttributeName() );
  posAttr->setVertexBaseType( Qt3DCore::QAttribute::Float );
  posAttr->setVertexSize( 3 );
  posAttr->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  posAttr->setBuffer( vertexBuffer );
  posAttr->setByteStride( 3 * sizeof( float ) );
  posAttr->setCount( 4 );

  geometry->addAttribute( posAttr );
  customQuad->setGeometry( geometry );
  customQuad->setPrimitiveType( Qt3DRender::QGeometryRenderer::TriangleStrip );

  m_quadEntity->addComponent( customQuad );

  Qt3DRender::QMaterial *material = new Qt3DRender::QMaterial( m_quadEntity );
  Qt3DRender::QEffect *effect = new Qt3DRender::QEffect( material );
  Qt3DRender::QTechnique *technique = new Qt3DRender::QTechnique( effect );

  technique->graphicsApiFilter()->setApi( Qt3DRender::QGraphicsApiFilter::OpenGL );
  technique->graphicsApiFilter()->setMajorVersion( 3 );
  technique->graphicsApiFilter()->setMinorVersion( 3 );
  technique->graphicsApiFilter()->setProfile( Qt3DRender::QGraphicsApiFilter::CoreProfile );

  Qt3DRender::QRenderPass *pass = new Qt3DRender::QRenderPass( technique );

  Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace( pass );
  cullFace->setMode( Qt3DRender::QCullFace::NoCulling );
  pass->addRenderState( cullFace );

  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest( pass );
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::Always );
  pass->addRenderState( depthTest );
  Qt3DRender::QShaderProgram *shader = new Qt3DRender::QShaderProgram( pass );

  const QString vertexShaderPath = u"qrc:/shaders/postprocess.vert"_s;
  shader->setVertexShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( vertexShaderPath ) ) );

  const QString fragmentShaderPath = u"qrc:/shaders/postprocess.frag"_s;
  shader->setFragmentShaderCode( Qt3DRender::QShaderProgram::loadSource( QUrl( fragmentShaderPath ) ) );

  pass->setShaderProgram( shader );
  technique->addRenderPass( pass );
  effect->addTechnique( technique );
  material->setEffect( effect );

  Qt3DRender::QParameter *texParam = new Qt3DRender::QParameter( u"colorTexture"_s, m_colorTexture, material );
  material->addParameter( texParam );

  m_quadEntity->addComponent( material );
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
  context.setTextureFilterQuality( Qgs3D::settingTextureFilterQuality->value() );
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

  mView = new QgsMaterialPreview3DWindow();
  mView->installEventFilter( this );

  QWidget *container = QWidget::createWindowContainer( mView, this );
  container->setMinimumSize( 200, 200 );
  container->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

  auto *layout = new QVBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( container );
  setLayout( layout );

  mSceneRoot = new Qt3DCore::QEntity;
  mSceneRoot->addComponent( mView->sceneLayer() );

  setupCamera( mView->camera() );
  mView->setRootEntity( mSceneRoot );
  QWidget::showEvent( e );

  if ( mLastPreviewSettings )
  {
    updatePreview( mLastPreviewSettings.get() );
  }
}
