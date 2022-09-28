/***************************************************************************
  qgs3daxis.cpp
  --------------------------------------
  Date                 : March 2022
  Copyright            : (C) 2022 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3daxis.h"

#include <Qt3DCore/QTransform>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QGeometry>
typedef Qt3DRender::QAttribute Qt3DQAttribute;
typedef Qt3DRender::QGeometry Qt3DQGeometry;
typedef Qt3DRender::QBuffer Qt3DQBuffer;
#else
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QGeometry>
typedef Qt3DCore::QAttribute Qt3DQAttribute;
typedef Qt3DCore::QGeometry Qt3DQGeometry;
typedef Qt3DCore::QBuffer Qt3DQBuffer;
#endif
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QConeMesh>
#include <Qt3DRender/qcameralens.h>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QPointLight>
#include <QWidget>
#include <QScreen>
#include <QShortcut>
#include <QFontDatabase>
#include <ctime>
#include <QApplication>
#include <QActionGroup>

#include "qgsmapsettings.h"
#include "qgs3dmapscene.h"
#include "qgsterrainentity_p.h"
#include "qgscoordinatereferencesystemutils.h"
#include "qgscoordinatereferencesystem.h"
#include "qgswindow3dengine.h"
#include "qgsraycastingutils_p.h"

Qgs3DAxis::Qgs3DAxis( Qt3DExtras::Qt3DWindow *parentWindow,
                      Qt3DCore::QEntity *parent3DScene,
                      Qgs3DMapScene *mapScene,
                      QgsCameraController *cameraCtrl,
                      Qgs3DMapSettings *map )
  : QObject( parentWindow )
  , mMapSettings( map )
  , mParentWindow( parentWindow )
  , mMapScene( mapScene )
  , mCameraController( cameraCtrl )
  , mCrs( map->crs() )
{
  mAxisViewport = constructAxisViewport( parent3DScene );
  mAxisViewport->setParent( mParentWindow->activeFrameGraph() );

  mTwoDLabelViewport = constructLabelViewport( parent3DScene, QRectF( 0.0f, 0.0f, 1.0f, 1.0f ) );
  mTwoDLabelViewport->setParent( mParentWindow->activeFrameGraph() );

  connect( cameraCtrl, &QgsCameraController::cameraChanged, this, &Qgs3DAxis::onCameraUpdate );
  connect( mParentWindow, &Qt3DExtras::Qt3DWindow::widthChanged, this, &Qgs3DAxis::onAxisViewportSizeUpdate );
  connect( mParentWindow, &Qt3DExtras::Qt3DWindow::heightChanged, this, &Qgs3DAxis::onAxisViewportSizeUpdate );

  createAxisScene();
  onAxisViewportSizeUpdate();

  init3DObjectPicking();

  createKeyboardShortCut();
}

Qgs3DAxis::~Qgs3DAxis()
{
  delete mMenu;
  mMenu = nullptr;
}

void Qgs3DAxis::init3DObjectPicking( )
{
  // Create screencaster to be used by EventFilter:
  //   1- Perform ray casting tests by specifying "touch" coordinates in screen space
  //   2- connect screencaster results to onTouchedByRay
  //   3- screencaster will be triggered by EventFilter
  mScreenRayCaster = new Qt3DRender::QScreenRayCaster( mAxisSceneEntity );
  mScreenRayCaster->addLayer( mAxisSceneLayer ); // to only filter on axis objects
  mScreenRayCaster->setFilterMode( Qt3DRender::QScreenRayCaster::AcceptAllMatchingLayers );
  mScreenRayCaster->setRunMode( Qt3DRender::QAbstractRayCaster::SingleShot );

  mAxisSceneEntity->addComponent( mScreenRayCaster );

  QObject::connect( mScreenRayCaster, &Qt3DRender::QScreenRayCaster::hitsChanged, this, &Qgs3DAxis::onTouchedByRay );

  // we need event filter (see Qgs3DAxis::eventFilter) to handle the mouse click event as this event is not catchable via the Qt3DRender::QObjectPicker
  mParentWindow->installEventFilter( this );
}

bool Qgs3DAxis::eventFilter( QObject *watched, QEvent *event )
{
  if ( watched != mParentWindow )
    return false;

  if ( event->type() == QEvent::MouseButtonPress )
  {
    // register mouse click to detect dragging
    mHasClicked = true;
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );
    mLastClickedPos = mouseEvent->pos();
  }

  // handle QEvent::MouseButtonRelease as it represents the end of click and QEvent::MouseMove.
  else if ( event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseMove )
  {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>( event );

    // user has clicked and move ==> dragging start
    if ( event->type() == QEvent::MouseMove &&
         ( ( mHasClicked  && ( mouseEvent->pos() - mLastClickedPos ).manhattanLength() < QApplication::startDragDistance() ) || mIsDragging ) )
    {
      mIsDragging = true;
    }

    // user has released ==> dragging ends
    else if ( mIsDragging && event->type() == QEvent::MouseButtonRelease )
    {
      mIsDragging = false;
      mHasClicked = false;
    }

    // user is moving or has released but not dragging
    else if ( ! mIsDragging )
    {
      // limit ray caster usage to the axis viewport
      QPointF normalizedPos( static_cast<float>( mouseEvent->pos().x() ) / mParentWindow->width(),
                             ( float )mouseEvent->pos().y() / mParentWindow->height() );

      if ( 2 <= QgsLogger::debugLevel() && event->type() == QEvent::MouseButtonRelease )
      {
        std::ostringstream os;
        os << "QGS3DAxis: normalized pos: " << normalizedPos << " / viewport: " << mAxisViewport->normalizedRect();
        QgsDebugMsgLevel( os.str().c_str(), 2 );
      }

      if ( mAxisViewport->normalizedRect().contains( normalizedPos ) )
      {
        mLastClickedButton = mouseEvent->button();
        mLastClickedPos = mouseEvent->pos();

        // if casted ray from pos matches an entity, call onTouchedByRay
        mScreenRayCaster->trigger( mLastClickedPos );
      }

      // when we exit the viewport, reset the mouse cursor if needed
      else if ( mPreviousCursor != Qt::ArrowCursor && mParentWindow->cursor() == Qt::ArrowCursor )
      {
        mParentWindow->setCursor( mPreviousCursor );
        mPreviousCursor = Qt::ArrowCursor;
      }

      mIsDragging = false; // drag ends
      mHasClicked = false;
    }
  }

  return false;
}

void Qgs3DAxis::onTouchedByRay( const Qt3DRender::QAbstractRayCaster::Hits &hits )
{
  int mHitsFound = -1;
  if ( !hits.empty() )
  {
    if ( 2 <= QgsLogger::debugLevel() )
    {
      std::ostringstream os;
      os << "Qgs3DAxis::onTouchedByRay " << hits.length() << " hits at pos " << mLastClickedPos << " with QButton: " << mLastClickedButton;
      for ( int i = 0; i < hits.length(); ++i )
      {
        os << "\n";
        os << "\tHit Type: " << hits.at( i ).type() << "\n";
        os << "\tHit triangle id: " << hits.at( i ).primitiveIndex() << "\n";
        os << "\tHit distance: " << hits.at( i ).distance() << "\n";
        os << "\tHit entity name: " << hits.at( i ).entity()->objectName().toStdString();
      }
      QgsDebugMsgLevel( os.str().c_str(), 2 );
    }

    for ( int i = 0; i < hits.length() && mHitsFound == -1; ++i )
    {
      if ( hits.at( i ).distance() < 500.0f && ( hits.at( i ).entity() == mCubeRoot || hits.at( i ).entity() == mAxisRoot || hits.at( i ).entity()->parent() == mCubeRoot || hits.at( i ).entity()->parent() == mAxisRoot ) )
      {
        mHitsFound = i;
      }
    }
  }

  if ( mLastClickedButton == Qt::NoButton )  // hover
  {
    if ( mHitsFound != -1 )
    {
      if ( mParentWindow->cursor() != Qt::ArrowCursor )
      {
        mPreviousCursor = mParentWindow->cursor();
        mParentWindow->setCursor( Qt::ArrowCursor );
        QgsDebugMsgLevel( "Enabling arrow cursor", 2 );
      }
    }
  }
  else if ( mLastClickedButton == Qt::MouseButton::RightButton && mHitsFound != -1 ) // show menu
  {
    displayMenuAt( mLastClickedPos );
  }
  else if ( mLastClickedButton == Qt::MouseButton::LeftButton ) // handle cube face clicks
  {
    hideMenu();

    if ( mHitsFound != -1 )
    {
      if ( hits.at( mHitsFound ).entity() == mCubeRoot || hits.at( mHitsFound ).entity()->parent() == mCubeRoot )
      {
        switch ( hits.at( mHitsFound ).primitiveIndex() / 2 )
        {
          case 0: // "East face";
            QgsDebugMsgLevel( "Qgs3DAxis: East face clicked", 2 );
            onCameraViewChangeEast();
            break;

          case 1: // "West face ";
            QgsDebugMsgLevel( "Qgs3DAxis: West face clicked", 2 );
            onCameraViewChangeWest();
            break;

          case 2: // "North face ";
            QgsDebugMsgLevel( "Qgs3DAxis: North face clicked", 2 );
            onCameraViewChangeNorth();
            break;

          case 3: // "South face";
            QgsDebugMsgLevel( "Qgs3DAxis: South face clicked", 2 );
            onCameraViewChangeSouth();
            break;

          case 4: // "Top face ";
            QgsDebugMsgLevel( "Qgs3DAxis: Top face clicked", 2 );
            onCameraViewChangeTop();
            break;

          case 5: // "Bottom face ";
            QgsDebugMsgLevel( "Qgs3DAxis: Bottom face clicked", 2 );
            onCameraViewChangeBottom();
            break;

          default:
            break;
        }
      }
    }
  }
}

Qt3DRender::QViewport *Qgs3DAxis::constructAxisViewport( Qt3DCore::QEntity *parent3DScene )
{
  Qt3DRender::QViewport *axisViewport = new Qt3DRender::QViewport;
  // parent will be set later
  // size will be set later

  mAxisSceneEntity = new Qt3DCore::QEntity;
  mAxisSceneEntity->setParent( parent3DScene );
  mAxisSceneEntity->setObjectName( "3DAxis_SceneEntity" );

  mAxisSceneLayer = new Qt3DRender::QLayer;
  mAxisSceneLayer->setObjectName( "3DAxis_SceneLayer" );
  mAxisSceneLayer->setParent( mAxisSceneEntity );
  mAxisSceneLayer->setRecursive( true );

  mAxisCamera = new Qt3DRender::QCamera;
  mAxisCamera->setParent( mAxisSceneEntity );
  mAxisCamera->setProjectionType( mCameraController->camera()->projectionType() );
  mAxisCamera->lens()->setFieldOfView( mCameraController->camera()->lens()->fieldOfView() * 0.5f );

  mAxisCamera->setUpVector( QVector3D( 0.0f, 0.0f, 1.0f ) );
  mAxisCamera->setViewCenter( QVector3D( 0.0f, 0.0f, 0.0f ) );
  // position will be set later

  Qt3DRender::QLayer *axisLayer = new Qt3DRender::QLayer;
  axisLayer->setRecursive( true );
  mAxisSceneEntity->addComponent( axisLayer );

  Qt3DRender::QLayerFilter *axisLayerFilter = new Qt3DRender::QLayerFilter( axisViewport );
  axisLayerFilter->addLayer( axisLayer );

  Qt3DRender::QCameraSelector *axisCameraSelector = new Qt3DRender::QCameraSelector;
  axisCameraSelector->setParent( axisLayerFilter );
  axisCameraSelector->setCamera( mAxisCamera );

  Qt3DRender::QClearBuffers *clearBuffers = new Qt3DRender::QClearBuffers( axisCameraSelector );
  clearBuffers->setBuffers( Qt3DRender::QClearBuffers::DepthBuffer );

  // cppcheck-suppress memleak
  return axisViewport;
}

Qt3DRender::QViewport *Qgs3DAxis::constructLabelViewport( Qt3DCore::QEntity *parent3DScene, const QRectF &parentViewportSize )
{
  Qt3DRender::QViewport *twoDViewport = new Qt3DRender::QViewport;
  // parent will be set later
  twoDViewport->setNormalizedRect( parentViewportSize );

  mTwoDLabelSceneEntity = new Qt3DCore::QEntity;
  mTwoDLabelSceneEntity->setParent( parent3DScene );
  mTwoDLabelSceneEntity->setEnabled( true );

  mTwoDLabelCamera = new Qt3DRender::QCamera;
  mTwoDLabelCamera->setParent( mTwoDLabelSceneEntity );
  mTwoDLabelCamera->setProjectionType( Qt3DRender::QCameraLens::ProjectionType::OrthographicProjection );
  mTwoDLabelCamera->lens()->setOrthographicProjection(
    -mParentWindow->width() / 2.0f, mParentWindow->width() / 2.0f,
    -mParentWindow->height() / 2.0f, mParentWindow->height() / 2.0f,
    -10.0f, 100.0f );

  mTwoDLabelCamera->setUpVector( QVector3D( 0.0f, 0.0f, 1.0f ) );
  mTwoDLabelCamera->setViewCenter( QVector3D( 0.0f, 0.0f, 0.0f ) );

  mTwoDLabelCamera->setPosition( QVector3D( 0.0f, 0.0f, 100.0f ) );

  Qt3DRender::QLayer *twoDLayer = new Qt3DRender::QLayer;
  twoDLayer->setRecursive( true );
  mTwoDLabelSceneEntity->addComponent( twoDLayer );

  Qt3DRender::QLayerFilter *twoDLayerFilter = new Qt3DRender::QLayerFilter( twoDViewport );
  twoDLayerFilter->addLayer( twoDLayer );

  Qt3DRender::QCameraSelector *twoDCameraSelector = new Qt3DRender::QCameraSelector;
  twoDCameraSelector->setParent( twoDLayerFilter );
  twoDCameraSelector->setCamera( mTwoDLabelCamera );

  Qt3DRender::QClearBuffers *clearBuffers = new Qt3DRender::QClearBuffers( twoDCameraSelector );
  clearBuffers->setBuffers( Qt3DRender::QClearBuffers::DepthBuffer );

  // cppcheck-suppress memleak
  return twoDViewport;
}

QVector3D Qgs3DAxis::from3DTo2DLabelPosition( const QVector3D &sourcePos,
    Qt3DRender::QCamera *sourceCamera, Qt3DRender::QViewport *sourceViewport,
    Qt3DRender::QCamera *destCamera, Qt3DRender::QViewport *destViewport,
    const QSize &destSize )
{
  QVector3D destPos = sourcePos.project( sourceCamera->viewMatrix(),
                                         destCamera->projectionMatrix(),
                                         QRect( 0.0f, 0.0f,
                                             destViewport->normalizedRect().width() * destSize.width(),
                                             destViewport->normalizedRect().height() * destSize.height() ) );
  QPointF axisCenter = sourceViewport->normalizedRect().center();
  QPointF labelCenter = destViewport->normalizedRect().center();
  QVector3D viewTranslation = QVector3D( ( axisCenter - labelCenter ).x() * destSize.width(),
                                         ( axisCenter - labelCenter ).y() * destSize.height(),
                                         0.0f );
  destPos -= QVector3D( labelCenter.x() * destSize.width(),
                        labelCenter.y() * destSize.height(),
                        0.0f );
  destPos.setX( destPos.x() + viewTranslation.x() );
  destPos.setY( destPos.y() - viewTranslation.y() );
  destPos.setZ( 0.0f );

  if ( 2 <= QgsLogger::debugLevel() )
  {
    std::ostringstream os;
    os << "Qgs3DAxis::from3DTo2DLabelPosition: sourcePos: " << sourcePos.toPoint()
       << " with translation: " << viewTranslation.toPoint()
       << " corrected to pos: " << destPos.toPoint();
    QgsDebugMsgLevel( os.str().c_str(), 2 );
  }
  return destPos;
}

void Qgs3DAxis::setEnableCube( bool show )
{
  mCubeRoot->setEnabled( show );
  if ( show )
  {
    mCubeRoot->setParent( mAxisSceneEntity );
  }
  else
  {
    mCubeRoot->setParent( static_cast<Qt3DCore::QEntity *>( nullptr ) );
  }
}

void Qgs3DAxis::setEnableAxis( bool show )
{
  mAxisRoot->setEnabled( show );
  if ( show )
  {
    mAxisRoot->setParent( mAxisSceneEntity );
  }
  else
  {
    mAxisRoot->setParent( static_cast<Qt3DCore::QEntity *>( nullptr ) );
  }

  mTextX->setEnabled( show );
  mTextY->setEnabled( show );
  mTextZ->setEnabled( show );
}

void Qgs3DAxis::createAxisScene()
{
  if ( mAxisRoot == nullptr || mCubeRoot == nullptr )
  {
    mAxisRoot = new Qt3DCore::QEntity;
    mAxisRoot->setParent( mAxisSceneEntity );
    mAxisRoot->setObjectName( "3DAxis_AxisRoot" );
    mAxisRoot->addComponent( mAxisSceneLayer ); // raycaster will filter object containing this layer

    createAxis( Qt::Axis::XAxis );
    createAxis( Qt::Axis::YAxis );
    createAxis( Qt::Axis::ZAxis );

    mCubeRoot = new Qt3DCore::QEntity;
    mCubeRoot->setParent( mAxisSceneEntity );
    mCubeRoot->setObjectName( "3DAxis_CubeRoot" );
    mCubeRoot->addComponent( mAxisSceneLayer ); // raycaster will filter object containing this layer

    createCube( );
  }

  Qgs3DAxisSettings::Mode mode = mMapSettings->get3DAxisSettings().mode();

  if ( mode == Qgs3DAxisSettings::Mode::Off )
  {
    mAxisSceneEntity->setEnabled( false );
    setEnableAxis( false );
    setEnableCube( false );
  }
  else
  {
    mAxisSceneEntity->setEnabled( true );
    if ( mode == Qgs3DAxisSettings::Mode::Crs )
    {
      setEnableCube( false );
      setEnableAxis( true );

      const QList< Qgis::CrsAxisDirection > axisDirections = mCrs.axisOrdering();

      if ( axisDirections.length() > 0 )
        mTextX->setText( QgsCoordinateReferenceSystemUtils::axisDirectionToAbbreviatedString( axisDirections.at( 0 ) ) );
      else
        mTextY->setText( "X?" );

      if ( axisDirections.length() > 1 )
        mTextY->setText( QgsCoordinateReferenceSystemUtils::axisDirectionToAbbreviatedString( axisDirections.at( 1 ) ) );
      else
        mTextY->setText( "Y?" );

      if ( axisDirections.length() > 2 )
        mTextZ->setText( QgsCoordinateReferenceSystemUtils::axisDirectionToAbbreviatedString( axisDirections.at( 2 ) ) );
      else
        mTextZ->setText( QStringLiteral( "up" ) );
    }
    else if ( mode == Qgs3DAxisSettings::Mode::Cube )
    {
      setEnableCube( true );
      setEnableAxis( false );
    }
    else
    {
      setEnableCube( false );
      setEnableAxis( true );
      mTextX->setText( "X?" );
      mTextY->setText( "Y?" );
      mTextZ->setText( "Z?" );
    }

    updateAxisLabelPosition();
  }
}

void Qgs3DAxis::createKeyboardShortCut()
{
  QgsWindow3DEngine *eng = dynamic_cast<QgsWindow3DEngine *>( mMapScene->engine() );
  if ( eng )
  {
    QWidget *mapCanvas = dynamic_cast<QWidget *>( eng->parent() );
    if ( mapCanvas == nullptr )
    {
      QgsLogger::warning( "Qgs3DAxis: no canvas defined!" );
    }
    else
    {
      QShortcut *shortcutHome = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_1 ), mapCanvas );
      connect( shortcutHome, &QShortcut::activated, this, [this]( ) {onCameraViewChangeHome();} );

      QShortcut *shortcutTop = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_5 ), mapCanvas );
      connect( shortcutTop, &QShortcut::activated, this, [this]( ) {onCameraViewChangeTop();} );

      QShortcut *shortcutNorth = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_8 ), mapCanvas );
      connect( shortcutNorth, &QShortcut::activated, this, [this]( ) {onCameraViewChangeNorth();} );

      QShortcut *shortcutEast = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_6 ), mapCanvas );
      connect( shortcutEast, &QShortcut::activated, this, [this]( ) {onCameraViewChangeEast();} );

      QShortcut *shortcutSouth = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_2 ), mapCanvas );
      connect( shortcutSouth, &QShortcut::activated, this, [this]( ) {onCameraViewChangeSouth();} );

      QShortcut *shortcutWest = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_4 ), mapCanvas );
      connect( shortcutWest, &QShortcut::activated, this, [this]( ) {onCameraViewChangeWest();} );
    }
  }
}

void Qgs3DAxis::createMenu()
{
  mMenu = new QMenu();

  // axis type menu
  QAction *typeOffAct = new QAction( tr( "&Off" ), mMenu );
  typeOffAct->setCheckable( true );
  typeOffAct->setStatusTip( tr( "Disable 3D axis" ) );
  connect( mMapSettings, &Qgs3DMapSettings::axisSettingsChanged, this, [typeOffAct, this]()
  {
    if ( mMapSettings->get3DAxisSettings().mode() == Qgs3DAxisSettings::Mode::Off )
      typeOffAct->setChecked( true );
  } );

  QAction *typeCrsAct = new QAction( tr( "Coordinate Reference &System" ), mMenu );
  typeCrsAct->setCheckable( true );
  typeCrsAct->setStatusTip( tr( "Coordinate Reference System 3D axis" ) );
  connect( mMapSettings, &Qgs3DMapSettings::axisSettingsChanged, this, [typeCrsAct, this]()
  {
    if ( mMapSettings->get3DAxisSettings().mode() == Qgs3DAxisSettings::Mode::Crs )
      typeCrsAct->setChecked( true );
  } );

  QAction *typeCubeAct = new QAction( tr( "&Cube" ), mMenu );
  typeCubeAct->setCheckable( true );
  typeCubeAct->setStatusTip( tr( "Cube 3D axis" ) );
  connect( mMapSettings, &Qgs3DMapSettings::axisSettingsChanged, this, [typeCubeAct, this]()
  {
    if ( mMapSettings->get3DAxisSettings().mode() == Qgs3DAxisSettings::Mode::Cube )
      typeCubeAct->setChecked( true );
  } );

  QActionGroup *typeGroup = new QActionGroup( mMenu );
  typeGroup->addAction( typeOffAct );
  typeGroup->addAction( typeCrsAct );
  typeGroup->addAction( typeCubeAct );

  connect( typeOffAct, &QAction::triggered, this, [this]( bool ) {onAxisModeChanged( Qgs3DAxisSettings::Mode::Off );} );
  connect( typeCrsAct, &QAction::triggered, this, [this]( bool ) {onAxisModeChanged( Qgs3DAxisSettings::Mode::Crs );} );
  connect( typeCubeAct, &QAction::triggered, this, [this]( bool ) {onAxisModeChanged( Qgs3DAxisSettings::Mode::Cube );} );

  QMenu *typeMenu = new QMenu( QStringLiteral( "Axis Type" ), mMenu );
  Q_ASSERT( typeMenu );
  typeMenu->addAction( typeOffAct );
  typeMenu->addAction( typeCrsAct );
  typeMenu->addAction( typeCubeAct );
  mMenu->addMenu( typeMenu );

  // horizontal position menu
  QAction *hPosLeftAct = new QAction( tr( "&Left" ), mMenu );
  hPosLeftAct->setCheckable( true );
  connect( mMapSettings, &Qgs3DMapSettings::axisSettingsChanged, this, [hPosLeftAct, this]()
  {
    if ( mMapSettings->get3DAxisSettings().horizontalPosition() == Qt::AnchorPoint::AnchorLeft )
      hPosLeftAct->setChecked( true );
  } );

  QAction *hPosMiddleAct = new QAction( tr( "&Center" ), mMenu );
  hPosMiddleAct->setCheckable( true );
  connect( mMapSettings, &Qgs3DMapSettings::axisSettingsChanged, this, [hPosMiddleAct, this]()
  {
    if ( mMapSettings->get3DAxisSettings().horizontalPosition() == Qt::AnchorPoint::AnchorHorizontalCenter )
      hPosMiddleAct->setChecked( true );
  } );

  QAction *hPosRightAct = new QAction( tr( "&Right" ), mMenu );
  hPosRightAct->setCheckable( true );
  connect( mMapSettings, &Qgs3DMapSettings::axisSettingsChanged, this, [hPosRightAct, this]()
  {
    if ( mMapSettings->get3DAxisSettings().horizontalPosition() == Qt::AnchorPoint::AnchorRight )
      hPosRightAct->setChecked( true );
  } );

  QActionGroup *hPosGroup = new QActionGroup( mMenu );
  hPosGroup->addAction( hPosLeftAct );
  hPosGroup->addAction( hPosMiddleAct );
  hPosGroup->addAction( hPosRightAct );

  connect( hPosLeftAct, &QAction::triggered, this, [this]( bool ) {onAxisHorizPositionChanged( Qt::AnchorPoint::AnchorLeft );} );
  connect( hPosMiddleAct, &QAction::triggered, this, [this]( bool ) {onAxisHorizPositionChanged( Qt::AnchorPoint::AnchorHorizontalCenter );} );
  connect( hPosRightAct, &QAction::triggered, this, [this]( bool ) {onAxisHorizPositionChanged( Qt::AnchorPoint::AnchorRight );} );

  QMenu *horizPosMenu = new QMenu( QStringLiteral( "Horizontal Position" ), mMenu );
  horizPosMenu->addAction( hPosLeftAct );
  horizPosMenu->addAction( hPosMiddleAct );
  horizPosMenu->addAction( hPosRightAct );
  mMenu->addMenu( horizPosMenu );

  // vertical position menu
  QAction *vPosTopAct = new QAction( tr( "&Top" ), mMenu );
  vPosTopAct->setCheckable( true );
  connect( mMapSettings, &Qgs3DMapSettings::axisSettingsChanged, this, [vPosTopAct, this]()
  {
    if ( mMapSettings->get3DAxisSettings().verticalPosition() == Qt::AnchorPoint::AnchorTop )
      vPosTopAct->setChecked( true );
  } );

  QAction *vPosMiddleAct = new QAction( tr( "&Middle" ), mMenu );
  vPosMiddleAct->setCheckable( true );
  connect( mMapSettings, &Qgs3DMapSettings::axisSettingsChanged, this, [vPosMiddleAct, this]()
  {
    if ( mMapSettings->get3DAxisSettings().verticalPosition() == Qt::AnchorPoint::AnchorVerticalCenter )
      vPosMiddleAct->setChecked( true );
  } );

  QAction *vPosBottomAct = new QAction( tr( "&Bottom" ), mMenu );
  vPosBottomAct->setCheckable( true );
  connect( mMapSettings, &Qgs3DMapSettings::axisSettingsChanged, this, [vPosBottomAct, this]()
  {
    if ( mMapSettings->get3DAxisSettings().verticalPosition() == Qt::AnchorPoint::AnchorBottom )
      vPosBottomAct->setChecked( true );
  } );

  QActionGroup *vPosGroup = new QActionGroup( mMenu );
  vPosGroup->addAction( vPosTopAct );
  vPosGroup->addAction( vPosMiddleAct );
  vPosGroup->addAction( vPosBottomAct );

  connect( vPosTopAct, &QAction::triggered, this, [this]( bool ) {onAxisVertPositionChanged( Qt::AnchorPoint::AnchorTop );} );
  connect( vPosMiddleAct, &QAction::triggered, this, [this]( bool ) {onAxisVertPositionChanged( Qt::AnchorPoint::AnchorVerticalCenter );} );
  connect( vPosBottomAct, &QAction::triggered, this, [this]( bool ) {onAxisVertPositionChanged( Qt::AnchorPoint::AnchorBottom );} );

  QMenu *vertPosMenu = new QMenu( QStringLiteral( "Vertical Position" ), mMenu );
  vertPosMenu->addAction( vPosTopAct );
  vertPosMenu->addAction( vPosMiddleAct );
  vertPosMenu->addAction( vPosBottomAct );
  mMenu->addMenu( vertPosMenu );

  // axis view menu
  QAction *viewHomeAct = new QAction( tr( "&Home" ) + "\t Ctrl+1", mMenu );
  QAction *viewTopAct = new QAction( tr( "&Top" ) + "\t Ctrl+5", mMenu );
  QAction *viewNorthAct = new QAction( tr( "&North" ) + "\t Ctrl+8", mMenu );
  QAction *viewEastAct = new QAction( tr( "&East" ) + "\t Ctrl+6", mMenu );
  QAction *viewSouthAct = new QAction( tr( "&South" ) + "\t Ctrl+2", mMenu );
  QAction *viewWestAct = new QAction( tr( "&West" ) + "\t Ctrl+4", mMenu );
  QAction *viewBottomAct = new QAction( tr( "&Bottom" ), mMenu );

  connect( viewHomeAct, &QAction::triggered, this, &Qgs3DAxis::onCameraViewChangeHome );
  connect( viewTopAct, &QAction::triggered, this, &Qgs3DAxis::onCameraViewChangeTop );
  connect( viewNorthAct, &QAction::triggered, this, &Qgs3DAxis::onCameraViewChangeNorth );
  connect( viewEastAct, &QAction::triggered, this, &Qgs3DAxis::onCameraViewChangeEast );
  connect( viewSouthAct, &QAction::triggered, this, &Qgs3DAxis::onCameraViewChangeSouth );
  connect( viewWestAct, &QAction::triggered, this, &Qgs3DAxis::onCameraViewChangeWest );
  connect( viewBottomAct, &QAction::triggered, this, &Qgs3DAxis::onCameraViewChangeBottom );

  QMenu *viewMenu = new QMenu( QStringLiteral( "Camera View" ), mMenu );
  viewMenu->addAction( viewHomeAct );
  viewMenu->addAction( viewTopAct );
  viewMenu->addAction( viewNorthAct );
  viewMenu->addAction( viewEastAct );
  viewMenu->addAction( viewSouthAct );
  viewMenu->addAction( viewWestAct );
  viewMenu->addAction( viewBottomAct );
  mMenu->addMenu( viewMenu );

  // update checkable items
  mMapSettings->set3DAxisSettings( mMapSettings->get3DAxisSettings(), true );
}

void Qgs3DAxis::hideMenu()
{
  if ( mMenu && mMenu->isVisible() )
    mMenu->hide();
}

void Qgs3DAxis::displayMenuAt( const QPoint &sourcePos )
{
  if ( mMenu == nullptr )
  {
    createMenu();
  }
  QObject *threeDMapCanvasWidget = mMapScene->engine()  // ie. 3DEngine
                                   ->parent() // ie. Qgs3DMapCanvas
                                   ->parent(); // ie. Qgs3DMapCanvasWidget

  QWidget *container = dynamic_cast<QWidget * >( threeDMapCanvasWidget->parent() );
  if ( container )
    mMenu->popup( container->mapToGlobal( sourcePos ) );
  else
    mMenu->popup( mParentWindow->parent()->mapToGlobal( sourcePos ) );
}

void Qgs3DAxis::onAxisModeChanged( Qgs3DAxisSettings::Mode mode )
{
  Qgs3DAxisSettings s = mMapSettings->get3DAxisSettings();
  s.setMode( mode );
  mMapSettings->set3DAxisSettings( s );
}

void Qgs3DAxis::onAxisHorizPositionChanged( Qt::AnchorPoint pos )
{
  Qgs3DAxisSettings s = mMapSettings->get3DAxisSettings();
  s.setHorizontalPosition( pos );
  mMapSettings->set3DAxisSettings( s );
}

void Qgs3DAxis::onAxisVertPositionChanged( Qt::AnchorPoint pos )
{
  Qgs3DAxisSettings s = mMapSettings->get3DAxisSettings();
  s.setVerticalPosition( pos );
  mMapSettings->set3DAxisSettings( s );
}

void Qgs3DAxis::onCameraViewChange( float pitch, float yaw )
{
  QgsVector3D pos = mCameraController->lookingAtPoint();
  double elevation = 0.0;
  if ( mMapSettings->terrainRenderingEnabled() )
  {
    QgsDebugMsgLevel( "Checking elevation from terrain...", 2 );
    QVector3D intersectionPoint;
    QVector3D camPos = mCameraController->camera()->position();
    QgsRayCastingUtils::Ray3D r( camPos, pos.toVector3D() - camPos );
    if ( mMapScene->terrainEntity()->rayIntersection( r, intersectionPoint ) )
    {
      elevation = intersectionPoint.y();
      QgsDebugMsgLevel( QString( "Computed elevation from terrain: %1" ).arg( elevation ), 2 );
    }
    else
      QgsDebugMsgLevel( "Unable to obtain elevation from terrain", 2 );

  }
  pos.set( pos.x(), elevation, pos.z() );

  mCameraController->setLookingAtPoint( pos, ( mCameraController->camera()->position() - pos.toVector3D() ).length(),
                                        pitch, yaw );
}


void Qgs3DAxis::createCube( )
{
  QVector3D minPos = QVector3D( -mCylinderLength * 0.5f, -mCylinderLength * 0.5f, -mCylinderLength * 0.5f );

  // cube outlines
  Qt3DCore::QEntity *cubeLineEntity = new Qt3DCore::QEntity( mCubeRoot );
  cubeLineEntity->setObjectName( "3DAxis_cubeline" );
  Qgs3DWiredMesh *cubeLine = new Qgs3DWiredMesh;
  QgsAABB box = QgsAABB( -mCylinderLength * 0.5f, -mCylinderLength * 0.5f, -mCylinderLength * 0.5f,
                         mCylinderLength * 0.5f, mCylinderLength * 0.5f, mCylinderLength * 0.5f );
  cubeLine->setVertices( box.verticesForLines() );
  cubeLineEntity->addComponent( cubeLine );

  Qt3DExtras::QPhongMaterial *cubeLineMaterial = new Qt3DExtras::QPhongMaterial;
  cubeLineMaterial->setAmbient( Qt::white );
  cubeLineEntity->addComponent( cubeLineMaterial );

  // cube mesh
  Qt3DExtras::QCuboidMesh *cubeMesh = new Qt3DExtras::QCuboidMesh;
  cubeMesh->setObjectName( "3DAxis_cubemesh" );
  cubeMesh->setXExtent( mCylinderLength );
  cubeMesh->setYExtent( mCylinderLength );
  cubeMesh->setZExtent( mCylinderLength );
  mCubeRoot->addComponent( cubeMesh );

  Qt3DExtras::QPhongMaterial *cubeMaterial = new Qt3DExtras::QPhongMaterial( mCubeRoot );
  cubeMaterial->setAmbient( QColor( 100, 100, 100, 50 ) );
  cubeMaterial->setShininess( 100 );
  mCubeRoot->addComponent( cubeMaterial );

  Qt3DCore::QTransform *cubeTransform = new Qt3DCore::QTransform;
  QMatrix4x4 transformMatrixcube;
  //transformMatrixcube.rotate( rotation );
  transformMatrixcube.translate( minPos + QVector3D( mCylinderLength * 0.5f, mCylinderLength * 0.5f, mCylinderLength * 0.5f ) );
  cubeTransform->setMatrix( transformMatrixcube );
  mCubeRoot->addComponent( cubeTransform );

  // text
  QString text;
  int fontSize = 0.75 * mFontSize;
  float textHeight = fontSize * 1.5f;
  float textWidth;
  QFont f = QFontDatabase::systemFont( QFontDatabase::FixedFont );
  f.setPointSize( fontSize );
  f.setWeight( QFont::Weight::Black );

  {
    text = QStringLiteral( "top" );
    textWidth = text.length() * fontSize * 0.75f;
    QVector3D translation = minPos + QVector3D(
                              mCylinderLength * 0.5f - textWidth / 2.0f,
                              mCylinderLength * 0.5f - textHeight / 2.0f,
                              mCylinderLength * 1.01f );
    QMatrix4x4 rotation;
    mCubeLabels << addCubeText( text, textHeight, textWidth, f, rotation, translation );
  }

  {
    text = QStringLiteral( "btm" );
    textWidth = text.length() * fontSize * 0.75f;
    QVector3D translation = minPos + QVector3D(
                              mCylinderLength * 0.5f - textWidth / 2.0f,
                              mCylinderLength * 0.5f + textHeight / 2.0f,
                              -mCylinderLength * 0.01f );
    QMatrix4x4 rotation;
    rotation.rotate( 180.0f, QVector3D( 1.0f, 0.0f, 0.0f ).normalized() );
    mCubeLabels << addCubeText( text, textHeight, textWidth, f, rotation, translation );
  }

  {
    text = QStringLiteral( "west" );
    textWidth = text.length() * fontSize * 0.75f;
    QVector3D translation = minPos + QVector3D(
                              - mCylinderLength * 0.01f,
                              mCylinderLength * 0.5f + textWidth / 2.0f,
                              mCylinderLength * 0.5f - textHeight / 2.0f );
    QMatrix4x4 rotation;
    rotation.rotate( 90.0f, QVector3D( 0.0f, -1.0f, 0.0f ).normalized() );
    rotation.rotate( 90.0f, QVector3D( 0.0f, 0.0f, -1.0f ).normalized() );
    mCubeLabels << addCubeText( text, textHeight, textWidth, f, rotation, translation );
  }

  {
    text = QStringLiteral( "east" );
    textWidth = text.length() * fontSize * 0.75f;
    QVector3D translation = minPos + QVector3D(
                              mCylinderLength * 1.01f,
                              mCylinderLength * 0.5f - textWidth / 2.0f,
                              mCylinderLength * 0.5f - textHeight / 2.0f );
    QMatrix4x4 rotation;
    rotation.rotate( 90.0f, QVector3D( 0.0f, 1.0f, 0.0f ).normalized() );
    rotation.rotate( 90.0f, QVector3D( 0.0f, 0.0f, 1.0f ).normalized() );
    mCubeLabels << addCubeText( text, textHeight, textWidth, f, rotation, translation );
  }

  {
    text = QStringLiteral( "south" );
    textWidth = text.length() * fontSize * 0.75f;
    QVector3D translation = minPos + QVector3D(
                              mCylinderLength * 0.5f - textWidth / 2.0f,
                              - mCylinderLength * 0.01f,
                              mCylinderLength * 0.5f - textHeight / 2.0f );
    QMatrix4x4 rotation;
    rotation.rotate( 90.0f, QVector3D( 1.0f, 0.0f, 0.0f ).normalized() );
    mCubeLabels << addCubeText( text, textHeight, textWidth, f, rotation, translation );
  }

  {
    text = QStringLiteral( "north" );
    textWidth = text.length() * fontSize * 0.75f;
    QVector3D translation = minPos + QVector3D(
                              mCylinderLength * 0.5f + textWidth / 2.0f,
                              mCylinderLength * 1.01f,
                              mCylinderLength * 0.5f - textHeight / 2.0f );
    QMatrix4x4 rotation;
    rotation.rotate( 90.0f, QVector3D( -1.0f, 0.0f, 0.0f ).normalized() );
    rotation.rotate( 180.0f, QVector3D( 0.0f, 0.0f, 1.0f ).normalized() );
    mCubeLabels << addCubeText( text, textHeight, textWidth, f, rotation, translation );
  }

  for ( Qt3DExtras::QText2DEntity *l : std::as_const( mCubeLabels ) )
  {
    l->setParent( mCubeRoot );
  }
}

Qt3DExtras::QText2DEntity *Qgs3DAxis::addCubeText( const QString &text, float textHeight, float textWidth, const QFont &f, const QMatrix4x4 &rotation, const QVector3D &translation )
{
  Qt3DExtras::QText2DEntity *textEntity = new Qt3DExtras::QText2DEntity;
  textEntity->setObjectName( "3DAxis_cube_label_" + text );
  textEntity->setFont( f );
  textEntity->setHeight( textHeight );
  textEntity->setWidth( textWidth );
  textEntity->setColor( QColor( 192, 192, 192 ) );
  textEntity->setText( text );

  Qt3DCore::QTransform *textFrontTransform = new Qt3DCore::QTransform();
  textFrontTransform->setMatrix( rotation );
  textFrontTransform->setTranslation( translation );
  textEntity->addComponent( textFrontTransform );

  return textEntity;
}

void Qgs3DAxis::createAxis( Qt::Axis axisType )
{
  float cylinderRadius = 0.05f * mCylinderLength;
  float coneLength = 0.3f * mCylinderLength;
  float coneBottomRadius = 0.1f * mCylinderLength;

  QQuaternion rotation;
  QColor color;

  Qt3DExtras::QText2DEntity *text = nullptr;
  Qt3DCore::QTransform *textTransform = nullptr;
  QString name;

  switch ( axisType )
  {
    case Qt::Axis::XAxis:
      mTextX = new Qt3DExtras::QText2DEntity( );  // object initialization in two step:
      mTextX->setParent( mTwoDLabelSceneEntity ); // see https://bugreports.qt.io/browse/QTBUG-77139
      connect( mTextX, &Qt3DExtras::QText2DEntity::textChanged, this, &Qgs3DAxis::onTextXChanged );
      mTextTransformX = new Qt3DCore::QTransform();
      mTextCoordX = QVector3D( mCylinderLength + coneLength / 2.0f, 0.0f, 0.0f );

      rotation = QQuaternion::fromAxisAndAngle( QVector3D( 0.0f, 0.0f, 1.0f ), -90.0f );
      color = Qt::red;
      text = mTextX;
      textTransform = mTextTransformX;
      name = "3DAxis_axisX";
      break;

    case Qt::Axis::YAxis:
      mTextY = new Qt3DExtras::QText2DEntity( );  // object initialization in two step:
      mTextY->setParent( mTwoDLabelSceneEntity ); // see https://bugreports.qt.io/browse/QTBUG-77139
      connect( mTextY, &Qt3DExtras::QText2DEntity::textChanged, this, &Qgs3DAxis::onTextYChanged );
      mTextTransformY = new Qt3DCore::QTransform();
      mTextCoordY = QVector3D( 0.0f, mCylinderLength + coneLength / 2.0f, 0.0f );

      rotation = QQuaternion::fromAxisAndAngle( QVector3D( 0.0f, 0.0f, 0.0f ), 0.0f );
      color = Qt::green;
      text = mTextY;
      textTransform = mTextTransformY;
      name = "3DAxis_axisY";
      break;

    case Qt::Axis::ZAxis:
      mTextZ = new Qt3DExtras::QText2DEntity( );  // object initialization in two step:
      mTextZ->setParent( mTwoDLabelSceneEntity ); // see https://bugreports.qt.io/browse/QTBUG-77139
      connect( mTextZ, &Qt3DExtras::QText2DEntity::textChanged, this, &Qgs3DAxis::onTextZChanged );
      mTextTransformZ = new Qt3DCore::QTransform();
      mTextCoordZ = QVector3D( 0.0f, 0.0f, mCylinderLength + coneLength / 2.0f );

      rotation = QQuaternion::fromAxisAndAngle( QVector3D( 1.0f, 0.0f, 0.0f ), 90.0f );
      color = Qt::blue;
      text = mTextZ;
      textTransform = mTextTransformZ;
      name = "3DAxis_axisZ";
      break;

    default:
      return;
  }

  // cylinder
  Qt3DCore::QEntity *cylinder = new Qt3DCore::QEntity( mAxisRoot );
  cylinder->setObjectName( name );

  Qt3DExtras::QCylinderMesh *cylinderMesh = new Qt3DExtras::QCylinderMesh;
  cylinderMesh->setRadius( cylinderRadius );
  cylinderMesh->setLength( mCylinderLength );
  cylinderMesh->setRings( 10 );
  cylinderMesh->setSlices( 4 );
  cylinder->addComponent( cylinderMesh );

  Qt3DExtras::QPhongMaterial *cylinderMaterial = new Qt3DExtras::QPhongMaterial( cylinder );
  cylinderMaterial->setAmbient( color );
  cylinderMaterial->setShininess( 0 );
  cylinder->addComponent( cylinderMaterial );

  Qt3DCore::QTransform *cylinderTransform = new Qt3DCore::QTransform;
  QMatrix4x4 transformMatrixCylinder;
  transformMatrixCylinder.rotate( rotation );
  transformMatrixCylinder.translate( QVector3D( 0.0f, mCylinderLength / 2.0f, 0.0f ) );
  cylinderTransform->setMatrix( transformMatrixCylinder );
  cylinder->addComponent( cylinderTransform );

  // cone
  Qt3DCore::QEntity *coneEntity = new Qt3DCore::QEntity( mAxisRoot );
  coneEntity->setObjectName( name );
  Qt3DExtras::QConeMesh *coneMesh = new Qt3DExtras::QConeMesh;
  coneMesh->setLength( coneLength );
  coneMesh->setBottomRadius( coneBottomRadius );
  coneMesh->setTopRadius( 0.0f );
  coneMesh->setRings( 10 );
  coneMesh->setSlices( 4 );
  coneEntity->addComponent( coneMesh );

  Qt3DExtras::QPhongMaterial *coneMaterial = new Qt3DExtras::QPhongMaterial( coneEntity );
  coneMaterial->setAmbient( color );
  coneMaterial->setShininess( 0 );
  coneEntity->addComponent( coneMaterial );

  Qt3DCore::QTransform *coneTransform = new Qt3DCore::QTransform;
  QMatrix4x4 transformMatrixCone;
  transformMatrixCone.rotate( rotation );
  transformMatrixCone.translate( QVector3D( 0.0f, mCylinderLength, 0.0f ) );
  coneTransform->setMatrix( transformMatrixCone );
  coneEntity->addComponent( coneTransform );

  // text font, height and width will be set later in onText?Changed
  text->setColor( QColor( 192, 192, 192, 192 ) );
  text->addComponent( textTransform );
}

void Qgs3DAxis::onAxisSettingsChanged()
{
  createAxisScene();
  onAxisViewportSizeUpdate();
}

void Qgs3DAxis::onAxisViewportSizeUpdate( int )
{
  Qgs3DAxisSettings settings = mMapSettings->get3DAxisSettings();

  double windowWidth = ( double )mParentWindow->width();
  double windowHeight = ( double )mParentWindow->height();

  QgsMapSettings set;
  if ( 2 <= QgsLogger::debugLevel() )
  {
    QgsDebugMsgLevel( QString( "onAxisViewportSizeUpdate window w/h: %1px / %2px" )
                      .arg( windowWidth ).arg( windowHeight ), 2 );
    QgsDebugMsgLevel( QString( "onAxisViewportSizeUpdate window physicalDpi %1 (%2, %3)" )
                      .arg( mParentWindow->screen()->physicalDotsPerInch() )
                      .arg( mParentWindow->screen()->physicalDotsPerInchX() )
                      .arg( mParentWindow->screen()->physicalDotsPerInchY() ), 2 );
    QgsDebugMsgLevel( QString( "onAxisViewportSizeUpdate window logicalDotsPerInch %1 (%2, %3)" )
                      .arg( mParentWindow->screen()->logicalDotsPerInch() )
                      .arg( mParentWindow->screen()->logicalDotsPerInchX() )
                      .arg( mParentWindow->screen()->logicalDotsPerInchY() ), 2 );

    QgsDebugMsgLevel( QString( "onAxisViewportSizeUpdate window pixel ratio %1" )
                      .arg( mParentWindow->screen()->devicePixelRatio() ), 2 );

    QgsDebugMsgLevel( QString( "onAxisViewportSizeUpdate set pixel ratio %1" )
                      .arg( set.devicePixelRatio() ), 2 );
    QgsDebugMsgLevel( QString( "onAxisViewportSizeUpdate set outputDpi %1" )
                      .arg( set.outputDpi() ), 2 );
    QgsDebugMsgLevel( QString( "onAxisViewportSizeUpdate set dpiTarget %1" )
                      .arg( set.dpiTarget() ), 2 );
  }

  // default viewport size in pixel according to 92 dpi
  double defaultViewportPixelSize = ( ( double )settings.defaultViewportSize() / 25.4 ) * 92.0;

  // computes the viewport size according to screen dpi but as the viewport size growths too fast
  // then we limit the growth by using a factor on the dpi difference.
  double viewportPixelSize = defaultViewportPixelSize + ( ( double )settings.defaultViewportSize() / 25.4 )
                             * ( mParentWindow->screen()->physicalDotsPerInch() - 92.0 ) * 0.7;
  QgsDebugMsgLevel( QString( "onAxisViewportSizeUpdate viewportPixelSize %1" ).arg( viewportPixelSize ), 2 );
  double widthRatio = viewportPixelSize / windowWidth;
  double heightRatio = widthRatio * windowWidth / windowHeight;

  QgsDebugMsgLevel( QString( "3DAxis viewport ratios width: %1% / height: %2%" ).arg( widthRatio ).arg( heightRatio ), 2 );

  if ( heightRatio * windowHeight < viewportPixelSize )
  {
    heightRatio = viewportPixelSize / windowHeight;
    widthRatio = heightRatio * windowHeight / windowWidth;
    QgsDebugMsgLevel( QString( "3DAxis viewport, height too small, ratios adjusted to width: %1% / height: %2%" ).arg( widthRatio ).arg( heightRatio ), 2 );
  }

  if ( heightRatio > settings.maxViewportRatio() || widthRatio > settings.maxViewportRatio() )
  {
    QgsDebugMsgLevel( "viewport takes too much place into the 3d view, disabling it", 2 );
    // take too much place into the 3d view
    mAxisViewport->setEnabled( false );
    setEnableCube( false );
    setEnableAxis( false );
  }
  else
  {
    // will be used to adjust the axis label translations/sizes
    mAxisScaleFactor = viewportPixelSize / defaultViewportPixelSize;
    QgsDebugMsgLevel( QString( "3DAxis viewport mAxisScaleFactor %1" ).arg( mAxisScaleFactor ), 2 );

    if ( ! mAxisViewport->isEnabled() )
    {
      if ( settings.mode() == Qgs3DAxisSettings::Mode::Crs )
        setEnableAxis( true );
      else if ( settings.mode() == Qgs3DAxisSettings::Mode::Cube )
        setEnableCube( true );
    }
    mAxisViewport->setEnabled( true );

    float xRatio = 1.0f;
    float yRatio = 1.0f;
    if ( settings.horizontalPosition() == Qt::AnchorPoint::AnchorLeft )
      xRatio = 0.0f;
    else if ( settings.horizontalPosition() == Qt::AnchorPoint::AnchorHorizontalCenter )
      xRatio = 0.5f - widthRatio / 2.0f;
    else
      xRatio = 1.0f - widthRatio;

    if ( settings.verticalPosition() == Qt::AnchorPoint::AnchorTop )
      yRatio = 0.0f;
    else if ( settings.verticalPosition() == Qt::AnchorPoint::AnchorVerticalCenter )
      yRatio = 0.5f - heightRatio / 2.0f;
    else
      yRatio = 1.0f - heightRatio;

    QgsDebugMsgLevel( QString( "Qgs3DAxis: update viewport: %1 x %2 x %3 x %4" ).arg( xRatio ).arg( yRatio ).arg( widthRatio ).arg( heightRatio ), 2 );
    mAxisViewport->setNormalizedRect( QRectF( xRatio, yRatio, widthRatio, heightRatio ) );

    if ( settings.mode() == Qgs3DAxisSettings::Mode::Crs )
    {
      mTwoDLabelCamera->lens()->setOrthographicProjection(
        -windowWidth / 2.0f, windowWidth / 2.0f,
        -windowHeight / 2.0f, windowHeight / 2.0f,
        mTwoDLabelCamera->lens()->nearPlane(), mTwoDLabelCamera->lens()->farPlane() );

      updateAxisLabelPosition();
    }
  }
}

void Qgs3DAxis::onCameraUpdate( )
{
  Qt3DRender::QCamera *parentCamera = mCameraController->camera();

  if ( parentCamera->viewVector() != mPreviousVector
       && !std::isnan( parentCamera->viewVector().x() )
       && !std::isnan( parentCamera->viewVector().y() )
       && !std::isnan( parentCamera->viewVector().z() ) )
  {
    mPreviousVector = parentCamera->viewVector();
    QVector3D mainCameraShift = parentCamera->viewVector().normalized();
    float zy_swap = mainCameraShift.y();
    mainCameraShift.setY( mainCameraShift.z() );
    mainCameraShift.setZ( -zy_swap );
    mainCameraShift.setX( -mainCameraShift.x() );

    if ( mAxisCamera->projectionType() == Qt3DRender::QCameraLens::ProjectionType::OrthographicProjection )
    {
      mAxisCamera->setPosition( mainCameraShift );
    }
    else
    {
      mAxisCamera->setPosition( mainCameraShift * mCylinderLength * 10.0 );
    }

    if ( mAxisRoot->isEnabled() )
    {
      updateAxisLabelPosition();
    }
  }
}

void Qgs3DAxis::updateAxisLabelPosition()
{
  if ( mTextTransformX && mTextTransformY && mTextTransformZ )
  {
    mTextTransformX->setTranslation( from3DTo2DLabelPosition( mTextCoordX * mAxisScaleFactor, mAxisCamera,
                                     mAxisViewport, mTwoDLabelCamera, mTwoDLabelViewport,
                                     mParentWindow->size() ) );
    onTextXChanged( mTextX->text() );

    mTextTransformY->setTranslation( from3DTo2DLabelPosition( mTextCoordY * mAxisScaleFactor, mAxisCamera,
                                     mAxisViewport, mTwoDLabelCamera, mTwoDLabelViewport,
                                     mParentWindow->size() ) );
    onTextYChanged( mTextY->text() );

    mTextTransformZ->setTranslation( from3DTo2DLabelPosition( mTextCoordZ * mAxisScaleFactor, mAxisCamera,
                                     mAxisViewport, mTwoDLabelCamera, mTwoDLabelViewport,
                                     mParentWindow->size() ) );
    onTextZChanged( mTextZ->text() );
  }
}

void Qgs3DAxis::onTextXChanged( const QString &text )
{
  QFont f = QFont( "monospace", mAxisScaleFactor *  mFontSize ); // TODO: should use outlined font
  f.setWeight( QFont::Weight::Black );
  f.setStyleStrategy( QFont::StyleStrategy::ForceOutline );
  mTextX->setFont( f );
  mTextX->setWidth( mAxisScaleFactor * mFontSize * text.length() );
  mTextX->setHeight( mAxisScaleFactor * mFontSize * 1.5f );
}

void Qgs3DAxis::onTextYChanged( const QString &text )
{
  QFont f = QFont( "monospace", mAxisScaleFactor *  mFontSize ); // TODO: should use outlined font
  f.setWeight( QFont::Weight::Black );
  f.setStyleStrategy( QFont::StyleStrategy::ForceOutline );
  mTextY->setFont( f );
  mTextY->setWidth( mAxisScaleFactor * mFontSize * text.length() );
  mTextY->setHeight( mAxisScaleFactor * mFontSize * 1.5f );
}

void Qgs3DAxis::onTextZChanged( const QString &text )
{
  QFont f = QFont( "monospace", mAxisScaleFactor *  mFontSize ); // TODO: should use outlined font
  f.setWeight( QFont::Weight::Black );
  f.setStyleStrategy( QFont::StyleStrategy::ForceOutline );
  mTextZ->setFont( f );
  mTextZ->setWidth( mAxisScaleFactor * mFontSize * text.length() );
  mTextZ->setHeight( mAxisScaleFactor * mFontSize * 1.5f );
}

//
// Qgs3DWiredMesh
//

Qgs3DWiredMesh::Qgs3DWiredMesh( Qt3DCore::QNode *parent )
  : Qt3DRender::QGeometryRenderer( parent )
  , mPositionAttribute( new Qt3DQAttribute( this ) )
  , mVertexBuffer( new Qt3DQBuffer( this ) )
{
  mPositionAttribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setVertexBaseType( Qt3DQAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setName( Qt3DQAttribute::defaultPositionAttributeName() );

  mGeom = new Qt3DQGeometry( this );
  mGeom->addAttribute( mPositionAttribute );

  setInstanceCount( 1 );
  setIndexOffset( 0 );
  setFirstInstance( 0 );
  setPrimitiveType( Qt3DRender::QGeometryRenderer::Lines );
  setGeometry( mGeom );
}

Qgs3DWiredMesh::~Qgs3DWiredMesh() = default;

void Qgs3DWiredMesh::setVertices( const QList<QVector3D> &vertices )
{
  QByteArray vertexBufferData;
  vertexBufferData.resize( vertices.size() * 3 * sizeof( float ) );
  float *rawVertexArray = reinterpret_cast<float *>( vertexBufferData.data() );
  int idx = 0;
  for ( const QVector3D &v : std::as_const( vertices ) )
  {
    rawVertexArray[idx++] = v.x();
    rawVertexArray[idx++] = v.y();
    rawVertexArray[idx++] = v.z();
  }

  mVertexBuffer->setData( vertexBufferData );
  setVertexCount( vertices.count() );
}
