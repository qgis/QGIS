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
#include "moc_qgs3daxis.cpp"

#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QConeMesh>
#include <Qt3DRender/qcameralens.h>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QSortPolicy>
#include <QWidget>
#include <QScreen>
#include <QShortcut>
#include <QFontDatabase>
#include <ctime>
#include <QApplication>
#include <QActionGroup>

#include "qgsmapsettings.h"
#include "qgs3dmapscene.h"
#include "qgsterrainentity.h"
#include "qgscoordinatereferencesystemutils.h"
#include "qgscoordinatereferencesystem.h"
#include "qgswindow3dengine.h"
#include "qgsraycastingutils_p.h"
#include "qgs3dwiredmesh_p.h"
#include "qgsabstractterrainsettings.h"

Qgs3DAxis::Qgs3DAxis( Qgs3DMapCanvas *canvas, Qt3DCore::QEntity *parent3DScene, Qgs3DMapScene *mapScene, QgsCameraController *cameraCtrl, Qgs3DMapSettings *map )
  : QObject( canvas )
  , mMapSettings( map )
  , mCanvas( canvas )
  , mMapScene( mapScene )
  , mCameraController( cameraCtrl )
  , mCrs( map->crs() )
{
  mViewport = constructAxisScene( parent3DScene );
  mViewport->setParent( mCanvas->activeFrameGraph() );

  constructLabelsScene( parent3DScene );

  connect( cameraCtrl, &QgsCameraController::cameraChanged, this, &Qgs3DAxis::onCameraUpdate );
  connect( mCanvas, &Qgs3DMapCanvas::widthChanged, this, &Qgs3DAxis::onAxisViewportSizeUpdate );
  connect( mCanvas, &Qgs3DMapCanvas::heightChanged, this, &Qgs3DAxis::onAxisViewportSizeUpdate );

  createAxisScene();
  onAxisViewportSizeUpdate();

  init3DObjectPicking();

  createKeyboardShortCut();
}

Qgs3DAxis::~Qgs3DAxis()
{
  delete mMenu;
  mMenu = nullptr;

  // When an object (axis or cube) is not enabled. It is still present but it does not have a parent.
  // In that case, it will never be automatically deleted. Therefore, it needs to be manually deleted.
  // See setEnableCube() and setEnableAxis().
  switch ( mMapSettings->get3DAxisSettings().mode() )
  {
    case Qgs3DAxisSettings::Mode::Crs:
      delete mCubeRoot;
      mCubeRoot = nullptr;
      break;
    case Qgs3DAxisSettings::Mode::Cube:
      delete mAxisRoot;
      mAxisRoot = nullptr;
      break;
    case Qgs3DAxisSettings::Mode::Off:
      delete mAxisRoot;
      mAxisRoot = nullptr;
      delete mCubeRoot;
      mCubeRoot = nullptr;
      break;
  }
}

void Qgs3DAxis::init3DObjectPicking()
{
  mDefaultPickingMethod = mMapScene->engine()->renderSettings()->pickingSettings()->pickMethod();

  // Create screencaster to be used by EventFilter:
  //   1- Perform ray casting tests by specifying "touch" coordinates in screen space
  //   2- connect screencaster results to onTouchedByRay
  //   3- screencaster will be triggered by EventFilter
  mScreenRayCaster = new Qt3DRender::QScreenRayCaster( mAxisSceneEntity );
  mScreenRayCaster->addLayer( mAxisObjectLayer ); // to only filter on axis objects
  mScreenRayCaster->setFilterMode( Qt3DRender::QScreenRayCaster::AcceptAllMatchingLayers );
  mScreenRayCaster->setRunMode( Qt3DRender::QAbstractRayCaster::SingleShot );

  mAxisSceneEntity->addComponent( mScreenRayCaster );

  QObject::connect( mScreenRayCaster, &Qt3DRender::QScreenRayCaster::hitsChanged, this, &Qgs3DAxis::onTouchedByRay );

  // we need event filter (see Qgs3DAxis::eventFilter) to handle the mouse click event as this event is not catchable via the Qt3DRender::QObjectPicker
  mCanvas->installEventFilter( this );
}

bool Qgs3DAxis::eventFilter( QObject *watched, QEvent *event )
{
  if ( watched != mCanvas )
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
    if ( event->type() == QEvent::MouseMove && ( ( mHasClicked && ( mouseEvent->pos() - mLastClickedPos ).manhattanLength() < QApplication::startDragDistance() ) || mIsDragging ) )
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
    else if ( !mIsDragging )
    {
      // limit ray caster usage to the axis viewport
      QPointF normalizedPos( static_cast<float>( mouseEvent->pos().x() ) / static_cast<float>( mCanvas->width() ), static_cast<float>( mouseEvent->pos().y() ) / static_cast<float>( mCanvas->height() ) );

      if ( 2 <= QgsLogger::debugLevel() && event->type() == QEvent::MouseButtonRelease )
      {
        std::ostringstream os;
        os << "QGS3DAxis: normalized pos: " << normalizedPos << " / viewport: " << mViewport->normalizedRect();
        QgsDebugMsgLevel( os.str().c_str(), 2 );
      }

      if ( mViewport->normalizedRect().contains( normalizedPos ) )
      {
        mLastClickedButton = mouseEvent->button();
        mLastClickedPos = mouseEvent->pos();

        // if casted ray from pos matches an entity, call onTouchedByRay
        mScreenRayCaster->trigger( mLastClickedPos );
      }
      // exit the viewport
      else
      {
        // reset the mouse cursor if needed
        if ( mPreviousCursor != Qt::ArrowCursor && mCanvas->cursor() == Qt::ArrowCursor )
        {
          mCanvas->setCursor( mPreviousCursor );
          mPreviousCursor = Qt::ArrowCursor;
        }

        // reset the picking settings if needed
        if ( mMapScene->engine()->renderSettings()->pickingSettings()->pickMethod() == Qt3DRender::QPickingSettings::TrianglePicking
             && mDefaultPickingMethod != Qt3DRender::QPickingSettings::TrianglePicking )
        {
          mMapScene->engine()->renderSettings()->pickingSettings()->setPickMethod( mDefaultPickingMethod );
          QgsDebugMsgLevel( "Disabling triangle picking", 2 );
        }
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
      if ( hits.at( i ).distance() < 500.0f && hits.at( i ).entity() && ( hits.at( i ).entity() == mCubeRoot || hits.at( i ).entity() == mAxisRoot || hits.at( i ).entity()->parent() == mCubeRoot || hits.at( i ).entity()->parent() == mAxisRoot ) )
      {
        mHitsFound = i;
      }
    }
  }

  if ( mLastClickedButton == Qt::NoButton ) // hover
  {
    if ( mHitsFound != -1 )
    {
      if ( mCanvas->cursor() != Qt::ArrowCursor )
      {
        mPreviousCursor = mCanvas->cursor();
        mCanvas->setCursor( Qt::ArrowCursor );
        QgsDebugMsgLevel( "Enabling arrow cursor", 2 );

        // The cube needs triangle picking to handle click on faces.
        if ( mMapScene->engine()->renderSettings()->pickingSettings()->pickMethod() != Qt3DRender::QPickingSettings::TrianglePicking && mCubeRoot->isEnabled() )
        {
          mMapScene->engine()->renderSettings()->pickingSettings()->setPickMethod( Qt3DRender::QPickingSettings::TrianglePicking );
          QgsDebugMsgLevel( "Enabling triangle picking", 2 );
        }
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

Qt3DRender::QViewport *Qgs3DAxis::constructAxisScene( Qt3DCore::QEntity *parent3DScene )
{
  Qt3DRender::QViewport *axisViewport = new Qt3DRender::QViewport;
  // parent will be set later
  // size will be set later

  mAxisSceneEntity = new Qt3DCore::QEntity;
  mAxisSceneEntity->setParent( parent3DScene );
  mAxisSceneEntity->setObjectName( "3DAxis_SceneEntity" );

  mAxisObjectLayer = new Qt3DRender::QLayer;
  mAxisObjectLayer->setObjectName( "3DAxis_ObjectLayer" );
  mAxisObjectLayer->setParent( mAxisSceneEntity );
  mAxisObjectLayer->setRecursive( true );

  mAxisCamera = new Qt3DRender::QCamera;
  mAxisCamera->setParent( mAxisSceneEntity );
  mAxisCamera->setProjectionType( mCameraController->camera()->projectionType() );
  mAxisCamera->lens()->setFieldOfView( mCameraController->camera()->lens()->fieldOfView() * 0.5f );

  mAxisCamera->setUpVector( QVector3D( 0.0f, 1.0f, 0.0f ) );
  mAxisCamera->setViewCenter( QVector3D( 0.0f, 0.0f, 0.0f ) );
  // position will be set later

  Qt3DRender::QLayerFilter *axisLayerFilter = new Qt3DRender::QLayerFilter( axisViewport );
  axisLayerFilter->addLayer( mAxisObjectLayer );

  Qt3DRender::QCameraSelector *axisCameraSelector = new Qt3DRender::QCameraSelector;
  axisCameraSelector->setParent( axisLayerFilter );
  axisCameraSelector->setCamera( mAxisCamera );

  // This ensures to have the labels (Text2DEntity) rendered after the other objects and therefore
  // avoid any transparency issue on the labels.
  Qt3DRender::QSortPolicy *sortPolicy = new Qt3DRender::QSortPolicy( axisCameraSelector );
  QVector<Qt3DRender::QSortPolicy::SortType> sortTypes = QVector<Qt3DRender::QSortPolicy::SortType>();
  sortTypes << Qt3DRender::QSortPolicy::BackToFront;
  sortPolicy->setSortTypes( sortTypes );

  Qt3DRender::QClearBuffers *clearBuffers = new Qt3DRender::QClearBuffers( sortPolicy );
  clearBuffers->setBuffers( Qt3DRender::QClearBuffers::DepthBuffer );

  // cppcheck-suppress memleak
  return axisViewport;
}

void Qgs3DAxis::constructLabelsScene( Qt3DCore::QEntity *parent3DScene )
{
  mTwoDLabelSceneEntity = new Qt3DCore::QEntity;
  mTwoDLabelSceneEntity->setParent( parent3DScene );
  mTwoDLabelSceneEntity->setEnabled( true );

  mTwoDLabelCamera = new Qt3DRender::QCamera;
  mTwoDLabelCamera->setParent( mTwoDLabelSceneEntity );
  mTwoDLabelCamera->setProjectionType( Qt3DRender::QCameraLens::ProjectionType::OrthographicProjection );
  // the camera lens parameters are defined by onAxisViewportSizeUpdate()

  mTwoDLabelCamera->setUpVector( QVector3D( 0.0f, 0.0f, 1.0f ) );
  mTwoDLabelCamera->setViewCenter( QVector3D( 0.0f, 0.0f, 0.0f ) );

  mTwoDLabelCamera->setPosition( QVector3D( 0.0f, 0.0f, 100.0f ) );

  Qt3DRender::QLayer *twoDLayer = new Qt3DRender::QLayer;
  twoDLayer->setObjectName( "3DAxis_LabelsLayer" );
  twoDLayer->setRecursive( true );
  mTwoDLabelSceneEntity->addComponent( twoDLayer );

  Qt3DRender::QLayerFilter *twoDLayerFilter = new Qt3DRender::QLayerFilter;
  twoDLayerFilter->addLayer( twoDLayer );

  Qt3DRender::QCameraSelector *twoDCameraSelector = new Qt3DRender::QCameraSelector;
  twoDCameraSelector->setParent( twoDLayerFilter );
  twoDCameraSelector->setCamera( mTwoDLabelCamera );

  // this ensures to have the labels (Text2DEntity) rendered after the other objects and therefore
  // avoid any transparency issue on the labels.
  Qt3DRender::QSortPolicy *sortPolicy = new Qt3DRender::QSortPolicy( twoDCameraSelector );
  QVector<Qt3DRender::QSortPolicy::SortType> sortTypes = QVector<Qt3DRender::QSortPolicy::SortType>();
  sortTypes << Qt3DRender::QSortPolicy::BackToFront;
  sortPolicy->setSortTypes( sortTypes );

  Qt3DRender::QClearBuffers *clearBuffers = new Qt3DRender::QClearBuffers( sortPolicy );
  clearBuffers->setBuffers( Qt3DRender::QClearBuffers::DepthBuffer );

  twoDLayerFilter->setParent( mViewport );
}

QVector3D Qgs3DAxis::from3DTo2DLabelPosition( const QVector3D &sourcePos, Qt3DRender::QCamera *sourceCamera, Qt3DRender::QCamera *destCamera )
{
  const int viewportWidth = static_cast<int>( std::round( mTwoDLabelCamera->lens()->right() - mTwoDLabelCamera->lens()->left() ) );
  const int viewportHeight = static_cast<int>( std::round( mTwoDLabelCamera->lens()->top() - mTwoDLabelCamera->lens()->bottom() ) );
  QRect viewportRect( static_cast<int>( std::round( mTwoDLabelCamera->lens()->left() ) ), static_cast<int>( std::round( mTwoDLabelCamera->lens()->bottom() ) ), viewportWidth, viewportHeight );

  QVector3D destPos = sourcePos.project( sourceCamera->viewMatrix(), destCamera->projectionMatrix(), viewportRect );
  destPos.setZ( 0.0f );
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
  if ( !mAxisRoot || !mCubeRoot )
  {
    mAxisRoot = new Qt3DCore::QEntity;
    mAxisRoot->setParent( mAxisSceneEntity );
    mAxisRoot->setObjectName( "3DAxis_AxisRoot" );
    mAxisRoot->addComponent( mAxisObjectLayer ); // raycaster will filter object containing this layer

    createAxis( Qt::Axis::XAxis );
    createAxis( Qt::Axis::YAxis );
    createAxis( Qt::Axis::ZAxis );

    mCubeRoot = new Qt3DCore::QEntity;
    mCubeRoot->setParent( mAxisSceneEntity );
    mCubeRoot->setObjectName( "3DAxis_CubeRoot" );
    mCubeRoot->addComponent( mAxisObjectLayer ); // raycaster will filter object containing this layer

    createCube();
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

      const QList<Qgis::CrsAxisDirection> axisDirections = mCrs.axisOrdering();

      if ( axisDirections.length() > 0 )
        mTextX->setText( QgsCoordinateReferenceSystemUtils::axisDirectionToAbbreviatedString( axisDirections.at( 0 ) ) );
      else
        mTextX->setText( "X?" );

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
    if ( !mapCanvas )
    {
      QgsLogger::warning( "Qgs3DAxis: no canvas defined!" );
    }
    else
    {
      QShortcut *shortcutHome = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_1 ), mapCanvas );
      connect( shortcutHome, &QShortcut::activated, this, [this]() { onCameraViewChangeHome(); } );

      QShortcut *shortcutTop = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_5 ), mapCanvas );
      connect( shortcutTop, &QShortcut::activated, this, [this]() { onCameraViewChangeTop(); } );

      QShortcut *shortcutNorth = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_8 ), mapCanvas );
      connect( shortcutNorth, &QShortcut::activated, this, [this]() { onCameraViewChangeNorth(); } );

      QShortcut *shortcutEast = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_6 ), mapCanvas );
      connect( shortcutEast, &QShortcut::activated, this, [this]() { onCameraViewChangeEast(); } );

      QShortcut *shortcutSouth = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_2 ), mapCanvas );
      connect( shortcutSouth, &QShortcut::activated, this, [this]() { onCameraViewChangeSouth(); } );

      QShortcut *shortcutWest = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_4 ), mapCanvas );
      connect( shortcutWest, &QShortcut::activated, this, [this]() { onCameraViewChangeWest(); } );
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
  connect( mMapSettings, &Qgs3DMapSettings::axisSettingsChanged, this, [typeOffAct, this]() {
    if ( mMapSettings->get3DAxisSettings().mode() == Qgs3DAxisSettings::Mode::Off )
      typeOffAct->setChecked( true );
  } );

  QAction *typeCrsAct = new QAction( tr( "Coordinate Reference &System" ), mMenu );
  typeCrsAct->setCheckable( true );
  typeCrsAct->setStatusTip( tr( "Coordinate Reference System 3D axis" ) );
  connect( mMapSettings, &Qgs3DMapSettings::axisSettingsChanged, this, [typeCrsAct, this]() {
    if ( mMapSettings->get3DAxisSettings().mode() == Qgs3DAxisSettings::Mode::Crs )
      typeCrsAct->setChecked( true );
  } );

  QAction *typeCubeAct = new QAction( tr( "&Cube" ), mMenu );
  typeCubeAct->setCheckable( true );
  typeCubeAct->setStatusTip( tr( "Cube 3D axis" ) );
  connect( mMapSettings, &Qgs3DMapSettings::axisSettingsChanged, this, [typeCubeAct, this]() {
    if ( mMapSettings->get3DAxisSettings().mode() == Qgs3DAxisSettings::Mode::Cube )
      typeCubeAct->setChecked( true );
  } );

  QActionGroup *typeGroup = new QActionGroup( mMenu );
  typeGroup->addAction( typeOffAct );
  typeGroup->addAction( typeCrsAct );
  typeGroup->addAction( typeCubeAct );

  connect( typeOffAct, &QAction::triggered, this, [this]( bool ) { onAxisModeChanged( Qgs3DAxisSettings::Mode::Off ); } );
  connect( typeCrsAct, &QAction::triggered, this, [this]( bool ) { onAxisModeChanged( Qgs3DAxisSettings::Mode::Crs ); } );
  connect( typeCubeAct, &QAction::triggered, this, [this]( bool ) { onAxisModeChanged( Qgs3DAxisSettings::Mode::Cube ); } );

  QMenu *typeMenu = new QMenu( QStringLiteral( "Axis Type" ), mMenu );
  Q_ASSERT( typeMenu );
  typeMenu->addAction( typeOffAct );
  typeMenu->addAction( typeCrsAct );
  typeMenu->addAction( typeCubeAct );
  mMenu->addMenu( typeMenu );

  // horizontal position menu
  QAction *hPosLeftAct = new QAction( tr( "&Left" ), mMenu );
  hPosLeftAct->setCheckable( true );
  connect( mMapSettings, &Qgs3DMapSettings::axisSettingsChanged, this, [hPosLeftAct, this]() {
    if ( mMapSettings->get3DAxisSettings().horizontalPosition() == Qt::AnchorPoint::AnchorLeft )
      hPosLeftAct->setChecked( true );
  } );

  QAction *hPosMiddleAct = new QAction( tr( "&Center" ), mMenu );
  hPosMiddleAct->setCheckable( true );
  connect( mMapSettings, &Qgs3DMapSettings::axisSettingsChanged, this, [hPosMiddleAct, this]() {
    if ( mMapSettings->get3DAxisSettings().horizontalPosition() == Qt::AnchorPoint::AnchorHorizontalCenter )
      hPosMiddleAct->setChecked( true );
  } );

  QAction *hPosRightAct = new QAction( tr( "&Right" ), mMenu );
  hPosRightAct->setCheckable( true );
  connect( mMapSettings, &Qgs3DMapSettings::axisSettingsChanged, this, [hPosRightAct, this]() {
    if ( mMapSettings->get3DAxisSettings().horizontalPosition() == Qt::AnchorPoint::AnchorRight )
      hPosRightAct->setChecked( true );
  } );

  QActionGroup *hPosGroup = new QActionGroup( mMenu );
  hPosGroup->addAction( hPosLeftAct );
  hPosGroup->addAction( hPosMiddleAct );
  hPosGroup->addAction( hPosRightAct );

  connect( hPosLeftAct, &QAction::triggered, this, [this]( bool ) { onAxisHorizPositionChanged( Qt::AnchorPoint::AnchorLeft ); } );
  connect( hPosMiddleAct, &QAction::triggered, this, [this]( bool ) { onAxisHorizPositionChanged( Qt::AnchorPoint::AnchorHorizontalCenter ); } );
  connect( hPosRightAct, &QAction::triggered, this, [this]( bool ) { onAxisHorizPositionChanged( Qt::AnchorPoint::AnchorRight ); } );

  QMenu *horizPosMenu = new QMenu( QStringLiteral( "Horizontal Position" ), mMenu );
  horizPosMenu->addAction( hPosLeftAct );
  horizPosMenu->addAction( hPosMiddleAct );
  horizPosMenu->addAction( hPosRightAct );
  mMenu->addMenu( horizPosMenu );

  // vertical position menu
  QAction *vPosTopAct = new QAction( tr( "&Top" ), mMenu );
  vPosTopAct->setCheckable( true );
  connect( mMapSettings, &Qgs3DMapSettings::axisSettingsChanged, this, [vPosTopAct, this]() {
    if ( mMapSettings->get3DAxisSettings().verticalPosition() == Qt::AnchorPoint::AnchorTop )
      vPosTopAct->setChecked( true );
  } );

  QAction *vPosMiddleAct = new QAction( tr( "&Middle" ), mMenu );
  vPosMiddleAct->setCheckable( true );
  connect( mMapSettings, &Qgs3DMapSettings::axisSettingsChanged, this, [vPosMiddleAct, this]() {
    if ( mMapSettings->get3DAxisSettings().verticalPosition() == Qt::AnchorPoint::AnchorVerticalCenter )
      vPosMiddleAct->setChecked( true );
  } );

  QAction *vPosBottomAct = new QAction( tr( "&Bottom" ), mMenu );
  vPosBottomAct->setCheckable( true );
  connect( mMapSettings, &Qgs3DMapSettings::axisSettingsChanged, this, [vPosBottomAct, this]() {
    if ( mMapSettings->get3DAxisSettings().verticalPosition() == Qt::AnchorPoint::AnchorBottom )
      vPosBottomAct->setChecked( true );
  } );

  QActionGroup *vPosGroup = new QActionGroup( mMenu );
  vPosGroup->addAction( vPosTopAct );
  vPosGroup->addAction( vPosMiddleAct );
  vPosGroup->addAction( vPosBottomAct );

  connect( vPosTopAct, &QAction::triggered, this, [this]( bool ) { onAxisVertPositionChanged( Qt::AnchorPoint::AnchorTop ); } );
  connect( vPosMiddleAct, &QAction::triggered, this, [this]( bool ) { onAxisVertPositionChanged( Qt::AnchorPoint::AnchorVerticalCenter ); } );
  connect( vPosBottomAct, &QAction::triggered, this, [this]( bool ) { onAxisVertPositionChanged( Qt::AnchorPoint::AnchorBottom ); } );

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
  if ( !mMenu )
  {
    createMenu();
  }

  mMenu->popup( mCanvas->mapToGlobal( sourcePos ) );
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
    QVector3D camPos = mCameraController->camera()->position();
    QgsRayCastingUtils::Ray3D ray( camPos, pos.toVector3D() - camPos, mCameraController->camera()->farPlane() );
    const QVector<QgsRayCastingUtils::RayHit> hits = mMapScene->terrainEntity()->rayIntersection( ray, QgsRayCastingUtils::RayCastContext() );
    if ( !hits.isEmpty() )
    {
      elevation = hits.at( 0 ).pos.z();
      QgsDebugMsgLevel( QString( "Computed elevation from terrain: %1" ).arg( elevation ), 2 );
    }
    else
    {
      QgsDebugMsgLevel( "Unable to obtain elevation from terrain", 2 );
    }
  }
  pos.set( pos.x(), pos.y(), elevation + mMapSettings->terrainSettings()->elevationOffset() );

  mCameraController->setLookingAtPoint( pos, ( mCameraController->camera()->position() - pos.toVector3D() ).length(), pitch, yaw );
}


void Qgs3DAxis::createCube()
{
  QVector3D minPos = QVector3D( -mCylinderLength * 0.5f, -mCylinderLength * 0.5f, -mCylinderLength * 0.5f );

  // cube outlines
  Qt3DCore::QEntity *cubeLineEntity = new Qt3DCore::QEntity( mCubeRoot );
  cubeLineEntity->setObjectName( "3DAxis_cubeline" );
  Qgs3DWiredMesh *cubeLine = new Qgs3DWiredMesh;
  QgsAABB box = QgsAABB( -mCylinderLength * 0.5f, -mCylinderLength * 0.5f, -mCylinderLength * 0.5f, mCylinderLength * 0.5f, mCylinderLength * 0.5f, mCylinderLength * 0.5f );
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
  const int fontSize = static_cast<int>( std::round( 0.75f * static_cast<float>( mFontSize ) ) );
  const float textHeight = static_cast<float>( fontSize ) * 1.5f;
  float textWidth;
  const QFont font = createFont( fontSize );

  {
    text = QStringLiteral( "top" );
    textWidth = static_cast<float>( text.length() * fontSize ) * 0.75f;
    QVector3D translation = minPos + QVector3D( mCylinderLength * 0.5f - textWidth / 2.0f, mCylinderLength * 0.5f - textHeight / 2.0f, mCylinderLength * 1.01f );
    QMatrix4x4 rotation;
    mCubeLabels << addCubeText( text, textHeight, textWidth, font, rotation, translation );
  }

  {
    text = QStringLiteral( "btm" );
    textWidth = static_cast<float>( text.length() * fontSize ) * 0.75f;
    QVector3D translation = minPos + QVector3D( mCylinderLength * 0.5f - textWidth / 2.0f, mCylinderLength * 0.5f + textHeight / 2.0f, -mCylinderLength * 0.01f );
    QMatrix4x4 rotation;
    rotation.rotate( 180.0f, QVector3D( 1.0f, 0.0f, 0.0f ).normalized() );
    mCubeLabels << addCubeText( text, textHeight, textWidth, font, rotation, translation );
  }

  {
    text = QStringLiteral( "west" );
    textWidth = static_cast<float>( text.length() * fontSize ) * 0.75f;
    QVector3D translation = minPos + QVector3D( -mCylinderLength * 0.01f, mCylinderLength * 0.5f + textWidth / 2.0f, mCylinderLength * 0.5f - textHeight / 2.0f );
    QMatrix4x4 rotation;
    rotation.rotate( 90.0f, QVector3D( 0.0f, -1.0f, 0.0f ).normalized() );
    rotation.rotate( 90.0f, QVector3D( 0.0f, 0.0f, -1.0f ).normalized() );
    mCubeLabels << addCubeText( text, textHeight, textWidth, font, rotation, translation );
  }

  {
    text = QStringLiteral( "east" );
    textWidth = static_cast<float>( text.length() * fontSize ) * 0.75f;
    QVector3D translation = minPos + QVector3D( mCylinderLength * 1.01f, mCylinderLength * 0.5f - textWidth / 2.0f, mCylinderLength * 0.5f - textHeight / 2.0f );
    QMatrix4x4 rotation;
    rotation.rotate( 90.0f, QVector3D( 0.0f, 1.0f, 0.0f ).normalized() );
    rotation.rotate( 90.0f, QVector3D( 0.0f, 0.0f, 1.0f ).normalized() );
    mCubeLabels << addCubeText( text, textHeight, textWidth, font, rotation, translation );
  }

  {
    text = QStringLiteral( "south" );
    textWidth = static_cast<float>( text.length() * fontSize ) * 0.75f;
    QVector3D translation = minPos + QVector3D( mCylinderLength * 0.5f - textWidth / 2.0f, -mCylinderLength * 0.01f, mCylinderLength * 0.5f - textHeight / 2.0f );
    QMatrix4x4 rotation;
    rotation.rotate( 90.0f, QVector3D( 1.0f, 0.0f, 0.0f ).normalized() );
    mCubeLabels << addCubeText( text, textHeight, textWidth, font, rotation, translation );
  }

  {
    text = QStringLiteral( "north" );
    textWidth = static_cast<float>( text.length() * fontSize ) * 0.75f;
    QVector3D translation = minPos + QVector3D( mCylinderLength * 0.5f + textWidth / 2.0f, mCylinderLength * 1.01f, mCylinderLength * 0.5f - textHeight / 2.0f );
    QMatrix4x4 rotation;
    rotation.rotate( 90.0f, QVector3D( -1.0f, 0.0f, 0.0f ).normalized() );
    rotation.rotate( 180.0f, QVector3D( 0.0f, 0.0f, 1.0f ).normalized() );
    mCubeLabels << addCubeText( text, textHeight, textWidth, font, rotation, translation );
  }

  for ( Qt3DExtras::QText2DEntity *l : std::as_const( mCubeLabels ) )
  {
    l->setParent( mCubeRoot );
  }
}

Qt3DExtras::QText2DEntity *Qgs3DAxis::addCubeText( const QString &text, float textHeight, float textWidth, const QFont &font, const QMatrix4x4 &rotation, const QVector3D &translation )
{
  Qt3DExtras::QText2DEntity *textEntity = new Qt3DExtras::QText2DEntity;
  textEntity->setObjectName( "3DAxis_cube_label_" + text );
  textEntity->setFont( font );
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
      mTextX = new Qt3DExtras::QText2DEntity();   // object initialization in two step:
      mTextX->setParent( mTwoDLabelSceneEntity ); // see https://bugreports.qt.io/browse/QTBUG-77139
      connect( mTextX, &Qt3DExtras::QText2DEntity::textChanged, this, [this]( const QString &text ) {
        updateAxisLabelText( mTextX, text );
      } );
      mTextTransformX = new Qt3DCore::QTransform();
      mTextCoordX = QVector3D( mCylinderLength + coneLength / 2.0f, 0.0f, 0.0f );

      rotation = QQuaternion::fromAxisAndAngle( QVector3D( 0.0f, 0.0f, 1.0f ), -90.0f );
      color = Qt::red;
      text = mTextX;
      textTransform = mTextTransformX;
      name = "3DAxis_axisX";
      break;

    case Qt::Axis::YAxis:
      mTextY = new Qt3DExtras::QText2DEntity();   // object initialization in two step:
      mTextY->setParent( mTwoDLabelSceneEntity ); // see https://bugreports.qt.io/browse/QTBUG-77139
      connect( mTextY, &Qt3DExtras::QText2DEntity::textChanged, this, [this]( const QString &text ) {
        updateAxisLabelText( mTextY, text );
      } );
      mTextTransformY = new Qt3DCore::QTransform();
      mTextCoordY = QVector3D( 0.0f, mCylinderLength + coneLength / 2.0f, 0.0f );

      // no rotation

      color = Qt::green;
      text = mTextY;
      textTransform = mTextTransformY;
      name = "3DAxis_axisY";
      break;

    case Qt::Axis::ZAxis:
      mTextZ = new Qt3DExtras::QText2DEntity();   // object initialization in two step:
      mTextZ->setParent( mTwoDLabelSceneEntity ); // see https://bugreports.qt.io/browse/QTBUG-77139
      connect( mTextZ, &Qt3DExtras::QText2DEntity::textChanged, this, [this]( const QString &text ) {
        updateAxisLabelText( mTextZ, text );
      } );
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

  double windowWidth = ( double ) mCanvas->width();
  double windowHeight = ( double ) mCanvas->height();

  QgsMapSettings set;
  if ( 2 <= QgsLogger::debugLevel() )
  {
    QgsDebugMsgLevel( QString( "onAxisViewportSizeUpdate window w/h: %1px / %2px" ).arg( windowWidth ).arg( windowHeight ), 2 );
    QgsDebugMsgLevel( QString( "onAxisViewportSizeUpdate window physicalDpi %1 (%2, %3)" ).arg( mCanvas->screen()->physicalDotsPerInch() ).arg( mCanvas->screen()->physicalDotsPerInchX() ).arg( mCanvas->screen()->physicalDotsPerInchY() ), 2 );
    QgsDebugMsgLevel( QString( "onAxisViewportSizeUpdate window logicalDotsPerInch %1 (%2, %3)" ).arg( mCanvas->screen()->logicalDotsPerInch() ).arg( mCanvas->screen()->logicalDotsPerInchX() ).arg( mCanvas->screen()->logicalDotsPerInchY() ), 2 );

    QgsDebugMsgLevel( QString( "onAxisViewportSizeUpdate window pixel ratio %1" ).arg( mCanvas->screen()->devicePixelRatio() ), 2 );

    QgsDebugMsgLevel( QString( "onAxisViewportSizeUpdate set pixel ratio %1" ).arg( set.devicePixelRatio() ), 2 );
    QgsDebugMsgLevel( QString( "onAxisViewportSizeUpdate set outputDpi %1" ).arg( set.outputDpi() ), 2 );
    QgsDebugMsgLevel( QString( "onAxisViewportSizeUpdate set dpiTarget %1" ).arg( set.dpiTarget() ), 2 );
  }

  // default viewport size in pixel according to 92 dpi
  double defaultViewportPixelSize = ( ( double ) settings.defaultViewportSize() / 25.4 ) * 92.0;

  // computes the viewport size according to screen dpi but as the viewport size growths too fast
  // then we limit the growth by using a factor on the dpi difference.
  double viewportPixelSize = defaultViewportPixelSize + ( ( double ) settings.defaultViewportSize() / 25.4 ) * ( mCanvas->screen()->physicalDotsPerInch() - 92.0 ) * 0.7;
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
    mViewport->setEnabled( false );
    setEnableCube( false );
    setEnableAxis( false );
  }
  else
  {
    // will be used to adjust the axis label translations/sizes
    mAxisScaleFactor = viewportPixelSize / defaultViewportPixelSize;
    QgsDebugMsgLevel( QString( "3DAxis viewport mAxisScaleFactor %1" ).arg( mAxisScaleFactor ), 2 );

    if ( !mViewport->isEnabled() )
    {
      if ( settings.mode() == Qgs3DAxisSettings::Mode::Crs )
        setEnableAxis( true );
      else if ( settings.mode() == Qgs3DAxisSettings::Mode::Cube )
        setEnableCube( true );
    }
    mViewport->setEnabled( true );

    float xRatio = 1.0f;
    float yRatio = 1.0f;
    if ( settings.horizontalPosition() == Qt::AnchorPoint::AnchorLeft )
      xRatio = 0.0f;
    else if ( settings.horizontalPosition() == Qt::AnchorPoint::AnchorHorizontalCenter )
      xRatio = 0.5f - static_cast<float>( widthRatio ) / 2.0f;
    else
      xRatio = 1.0f - static_cast<float>( widthRatio );

    if ( settings.verticalPosition() == Qt::AnchorPoint::AnchorTop )
      yRatio = 0.0f;
    else if ( settings.verticalPosition() == Qt::AnchorPoint::AnchorVerticalCenter )
      yRatio = 0.5f - static_cast<float>( heightRatio ) / 2.0f;
    else
      yRatio = 1.0f - static_cast<float>( heightRatio );

    QgsDebugMsgLevel( QString( "Qgs3DAxis: update viewport: %1 x %2 x %3 x %4" ).arg( xRatio ).arg( yRatio ).arg( widthRatio ).arg( heightRatio ), 2 );
    mViewport->setNormalizedRect( QRectF( xRatio, yRatio, widthRatio, heightRatio ) );

    if ( settings.mode() == Qgs3DAxisSettings::Mode::Crs )
    {
      const float halfWidthSize = static_cast<float>( windowWidth * widthRatio / 2.0 );
      const float halfHeightSize = static_cast<float>( windowWidth * widthRatio / 2.0 );
      mTwoDLabelCamera->lens()->setOrthographicProjection(
        -halfWidthSize, halfWidthSize,
        -halfHeightSize, halfHeightSize,
        mTwoDLabelCamera->lens()->nearPlane(), mTwoDLabelCamera->lens()->farPlane()
      );

      updateAxisLabelPosition();
    }
  }
}

void Qgs3DAxis::onCameraUpdate()
{
  Qt3DRender::QCamera *parentCamera = mCameraController->camera();

  if ( parentCamera->viewVector() != mPreviousVector
       && !std::isnan( parentCamera->viewVector().x() )
       && !std::isnan( parentCamera->viewVector().y() )
       && !std::isnan( parentCamera->viewVector().z() ) )
  {
    mPreviousVector = parentCamera->viewVector();

    QQuaternion q = QQuaternion::fromDirection( -parentCamera->viewVector(), parentCamera->upVector() );
    mAxisCamera->setPosition( q.rotatedVector( QVector3D( 0, 0, mCylinderLength * 9.0f ) ) );
    mAxisCamera->setUpVector( q.rotatedVector( QVector3D( 0, 1, 0 ) ) );

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
    mTextTransformX->setTranslation( from3DTo2DLabelPosition( mTextCoordX * static_cast<float>( mAxisScaleFactor ), mAxisCamera, mTwoDLabelCamera ) );
    updateAxisLabelText( mTextX, mTextX->text() );

    mTextTransformY->setTranslation( from3DTo2DLabelPosition( mTextCoordY * static_cast<float>( mAxisScaleFactor ), mAxisCamera, mTwoDLabelCamera ) );
    updateAxisLabelText( mTextY, mTextY->text() );

    mTextTransformZ->setTranslation( from3DTo2DLabelPosition( mTextCoordZ * static_cast<float>( mAxisScaleFactor ), mAxisCamera, mTwoDLabelCamera ) );
    updateAxisLabelText( mTextZ, mTextZ->text() );
  }
}

void Qgs3DAxis::updateAxisLabelText( Qt3DExtras::QText2DEntity *textEntity, const QString &text )
{
  const float scaledFontSize = static_cast<float>( mAxisScaleFactor ) * static_cast<float>( mFontSize );
  const QFont font = createFont( static_cast<int>( std::round( scaledFontSize ) ) );
  textEntity->setFont( font );
  textEntity->setWidth( scaledFontSize * static_cast<float>( text.length() ) );
  textEntity->setHeight( 1.5f * scaledFontSize );
}

QFont Qgs3DAxis::createFont( int pointSize )
{
  QFont font = QFontDatabase::systemFont( QFontDatabase::FixedFont );
  font.setPointSize( pointSize );
  font.setWeight( QFont::Weight::Black );
  font.setStyleStrategy( QFont::StyleStrategy::ForceOutline );
  return font;
}
