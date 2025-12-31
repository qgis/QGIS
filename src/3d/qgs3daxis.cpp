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

#include <ctime>

#include "qgs3dmapscene.h"
#include "qgs3dmapsettings.h"
#include "qgs3dwiredmesh_p.h"
#include "qgsabstractterrainsettings.h"
#include "qgscoordinatereferencesystemutils.h"
#include "qgsframegraph.h"
#include "qgsterrainentity.h"
#include "qgswindow3dengine.h"

#include <QActionGroup>
#include <QApplication>
#include <QFontDatabase>
#include <QScreen>
#include <QWidget>
#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QConeMesh>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QPointLight>
#include <Qt3DRender/QSortPolicy>
#include <Qt3DRender/qcameralens.h>

#include "moc_qgs3daxis.cpp"

Qgs3DAxis::Qgs3DAxis( Qgs3DMapCanvas *canvas, Qt3DCore::QEntity *parent3DScene, Qgs3DMapScene *mapScene, //
                      QgsCameraController *cameraCtrl, Qgs3DMapSettings *map )
  : QObject( canvas )
  , mMapSettings( map )
  , mCanvas( canvas )
  , mMapScene( mapScene )
  , mCameraController( cameraCtrl )
  , mCrs( map->crs() )
{
  mMapScene->engine()->frameGraph()->registerRenderView( std::make_unique<Qgs3DAxisRenderView>(      //
                                                           QgsFrameGraph::AXIS3D_RENDERVIEW,         //
                                                           mCanvas, mCameraController, mMapSettings, //
                                                           this
                                                         ),
                                                         QgsFrameGraph::AXIS3D_RENDERVIEW );

  mRenderView = dynamic_cast<Qgs3DAxisRenderView *>( mMapScene->engine()->frameGraph()->renderView( QgsFrameGraph::AXIS3D_RENDERVIEW ) );
  Q_ASSERT( mRenderView );
  constructAxisScene( parent3DScene );
  constructLabelsScene( parent3DScene );

  mTwoDLabelSceneEntity->addComponent( mRenderView->labelLayer() );

  connect( cameraCtrl, &QgsCameraController::cameraChanged, this, &Qgs3DAxis::onCameraUpdate );

  createAxisScene();
  onAxisViewportSizeUpdate();

  init3DObjectPicking();
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

  // render view unregistration will be done by framegraph destructor!
}

void Qgs3DAxis::init3DObjectPicking()
{
  mDefaultPickingMethod = mMapScene->engine()->renderSettings()->pickingSettings()->pickMethod();

  // Create screencaster to be used by EventFilter:
  //   1- Perform ray casting tests by specifying "touch" coordinates in screen space
  //   2- connect screencaster results to onTouchedByRay
  //   3- screencaster will be triggered by EventFilter
  mScreenRayCaster = new Qt3DRender::QScreenRayCaster( mAxisSceneEntity );
  mScreenRayCaster->addLayer( mRenderView->objectLayer() ); // to only filter on axis objects
  mScreenRayCaster->setFilterMode( Qt3DRender::QScreenRayCaster::AcceptAllMatchingLayers );
  mScreenRayCaster->setRunMode( Qt3DRender::QAbstractRayCaster::SingleShot );

  mAxisSceneEntity->addComponent( mScreenRayCaster );

  QObject::connect( mScreenRayCaster, &Qt3DRender::QScreenRayCaster::hitsChanged, this, &Qgs3DAxis::onTouchedByRay );
}

// will be called by Qgs3DMapCanvas::eventFilter
bool Qgs3DAxis::handleEvent( QEvent *event )
{
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
        os << "QGS3DAxis: normalized pos: " << normalizedPos << " / viewport: " << mRenderView->viewport()->normalizedRect();
        QgsDebugMsgLevel( os.str().c_str(), 2 );
      }

      if ( mRenderView->viewport()->normalizedRect().contains( normalizedPos ) )
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
  int hitFoundIdx = -1;
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

    for ( int i = 0; i < hits.length() && hitFoundIdx == -1; ++i )
    {
      Qt3DCore::QEntity *hitEntity = hits.at( i ).entity();
      // In Qt6, a Qt3DExtras::Text2DEntity contains a private entity: Qt3DExtras::DistanceFieldTextRenderer
      // The Text2DEntity needs to be retrieved to handle proper picking
      if ( hitEntity && qobject_cast<Qt3DExtras::QText2DEntity *>( hitEntity->parentEntity() ) )
      {
        hitEntity = hitEntity->parentEntity();
      }
      if ( hits.at( i ).distance() < 500.0f && hitEntity && ( hitEntity == mCubeRoot || hitEntity == mAxisRoot || hitEntity->parent() == mCubeRoot || hitEntity->parent() == mAxisRoot ) )
      {
        hitFoundIdx = i;
      }
    }
  }

  if ( mLastClickedButton == Qt::NoButton ) // hover
  {
    if ( hitFoundIdx != -1 )
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
  else if ( mLastClickedButton == Qt::MouseButton::RightButton && hitFoundIdx != -1 ) // show menu
  {
    displayMenuAt( mLastClickedPos );
  }
  else if ( mLastClickedButton == Qt::MouseButton::LeftButton ) // handle cube face clicks
  {
    hideMenu();

    if ( hitFoundIdx != -1 )
    {
      Qt3DCore::QEntity *hitEntity = hits.at( hitFoundIdx ).entity();
      if ( hitEntity && qobject_cast<Qt3DExtras::QText2DEntity *>( hitEntity->parentEntity() ) )
      {
        hitEntity = hitEntity->parentEntity();
      }
      if ( hitEntity && ( hitEntity == mCubeRoot || hitEntity->parent() == mCubeRoot ) )
      {
        switch ( hits.at( hitFoundIdx ).primitiveIndex() / 2 )
        {
          case 0: // "East face";
            QgsDebugMsgLevel( "Qgs3DAxis: East face clicked", 2 );
            mCameraController->rotateCameraToEast();
            break;

          case 1: // "West face ";
            QgsDebugMsgLevel( "Qgs3DAxis: West face clicked", 2 );
            mCameraController->rotateCameraToWest();
            break;

          case 2: // "North face ";
            QgsDebugMsgLevel( "Qgs3DAxis: North face clicked", 2 );
            mCameraController->rotateCameraToNorth();
            break;

          case 3: // "South face";
            QgsDebugMsgLevel( "Qgs3DAxis: South face clicked", 2 );
            mCameraController->rotateCameraToSouth();
            break;

          case 4: // "Top face ";
            QgsDebugMsgLevel( "Qgs3DAxis: Top face clicked", 2 );
            mCameraController->rotateCameraToTop();
            break;

          case 5: // "Bottom face ";
            QgsDebugMsgLevel( "Qgs3DAxis: Bottom face clicked", 2 );
            mCameraController->rotateCameraToBottom();
            break;

          default:
            break;
        }
      }
    }
  }
}

void Qgs3DAxis::constructAxisScene( Qt3DCore::QEntity *parent3DScene )
{
  mAxisSceneEntity = new Qt3DCore::QEntity;
  mAxisSceneEntity->setParent( parent3DScene );
  mAxisSceneEntity->setObjectName( "3DAxis_SceneEntity" );

  mAxisCamera = mRenderView->objectCamera();
  mAxisCamera->setUpVector( QVector3D( 0.0f, 1.0f, 0.0f ) );
  mAxisCamera->setViewCenter( QVector3D( 0.0f, 0.0f, 0.0f ) );
  // position will be set later
}

void Qgs3DAxis::constructLabelsScene( Qt3DCore::QEntity *parent3DScene )
{
  mTwoDLabelSceneEntity = new Qt3DCore::QEntity;
  mTwoDLabelSceneEntity->setParent( parent3DScene );
  mTwoDLabelSceneEntity->setEnabled( true );

  mTwoDLabelCamera = mRenderView->labelCamera();
  mTwoDLabelCamera->setUpVector( QVector3D( 0.0f, 1.0f, 0.0f ) );
  mTwoDLabelCamera->setViewCenter( QVector3D( 0.0f, 0.0f, 0.0f ) );
  mTwoDLabelCamera->setPosition( QVector3D( 0.0f, 0.0f, 100.0f ) );
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
    mAxisRoot->addComponent( mRenderView->objectLayer() ); // raycaster will filter object containing this layer

    createAxis( Qt::Axis::XAxis );
    createAxis( Qt::Axis::YAxis );
    createAxis( Qt::Axis::ZAxis );

    mCubeRoot = new Qt3DCore::QEntity;
    mCubeRoot->setParent( mAxisSceneEntity );
    mCubeRoot->setObjectName( "3DAxis_CubeRoot" );
    mCubeRoot->addComponent( mRenderView->objectLayer() ); // raycaster will filter object containing this layer

    createCube();
  }

  Qgs3DAxisSettings::Mode mode = mMapSettings->get3DAxisSettings().mode();

  if ( mode == Qgs3DAxisSettings::Mode::Off )
  {
    mAxisSceneEntity->setEnabled( false );
    setEnableAxis( false );
    setEnableCube( false );
    mRenderView->setEnabled( false );
  }
  else
  {
    mRenderView->setEnabled( true );
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
        mTextZ->setText( u"up"_s );
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

  QMenu *typeMenu = new QMenu( u"Axis Type"_s, mMenu );
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

  connect( hPosLeftAct, &QAction::triggered, this, [this]( bool ) { mRenderView->onHorizontalPositionChanged( Qt::AnchorPoint::AnchorLeft ); } );
  connect( hPosMiddleAct, &QAction::triggered, this, [this]( bool ) { mRenderView->onHorizontalPositionChanged( Qt::AnchorPoint::AnchorHorizontalCenter ); } );
  connect( hPosRightAct, &QAction::triggered, this, [this]( bool ) { mRenderView->onHorizontalPositionChanged( Qt::AnchorPoint::AnchorRight ); } );

  QMenu *horizPosMenu = new QMenu( u"Horizontal Position"_s, mMenu );
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

  connect( vPosTopAct, &QAction::triggered, this, [this]( bool ) { mRenderView->onVerticalPositionChanged( Qt::AnchorPoint::AnchorTop ); } );
  connect( vPosMiddleAct, &QAction::triggered, this, [this]( bool ) { mRenderView->onVerticalPositionChanged( Qt::AnchorPoint::AnchorVerticalCenter ); } );
  connect( vPosBottomAct, &QAction::triggered, this, [this]( bool ) { mRenderView->onVerticalPositionChanged( Qt::AnchorPoint::AnchorBottom ); } );

  QMenu *vertPosMenu = new QMenu( u"Vertical Position"_s, mMenu );
  vertPosMenu->addAction( vPosTopAct );
  vertPosMenu->addAction( vPosMiddleAct );
  vertPosMenu->addAction( vPosBottomAct );
  mMenu->addMenu( vertPosMenu );

  // axis view menu
  // Make sure to sync the key combinations with QgsCameraController::keyboardEventFilter()!
  QAction *viewHomeAct = new QAction( tr( "&Home" ) + "\t Ctrl+5", mMenu );
  QAction *viewTopAct = new QAction( tr( "&Top" ) + "\t Ctrl+9", mMenu );
  QAction *viewNorthAct = new QAction( tr( "&North" ) + "\t Ctrl+8", mMenu );
  QAction *viewEastAct = new QAction( tr( "&East" ) + "\t Ctrl+6", mMenu );
  QAction *viewSouthAct = new QAction( tr( "&South" ) + "\t Ctrl+2", mMenu );
  QAction *viewWestAct = new QAction( tr( "&West" ) + "\t Ctrl+4", mMenu );
  QAction *viewBottomAct = new QAction( tr( "&Bottom" ) + "\t Ctrl+3", mMenu );

  connect( viewHomeAct, &QAction::triggered, mCameraController, &QgsCameraController::rotateCameraToHome );
  connect( viewTopAct, &QAction::triggered, mCameraController, &QgsCameraController::rotateCameraToTop );
  connect( viewNorthAct, &QAction::triggered, mCameraController, &QgsCameraController::rotateCameraToNorth );
  connect( viewEastAct, &QAction::triggered, mCameraController, &QgsCameraController::rotateCameraToEast );
  connect( viewSouthAct, &QAction::triggered, mCameraController, &QgsCameraController::rotateCameraToSouth );
  connect( viewWestAct, &QAction::triggered, mCameraController, &QgsCameraController::rotateCameraToWest );
  connect( viewBottomAct, &QAction::triggered, mCameraController, &QgsCameraController::rotateCameraToBottom );

  QMenu *viewMenu = new QMenu( u"Camera View"_s, mMenu );
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
    text = u"top"_s;
    textWidth = static_cast<float>( text.length() * fontSize ) * 0.75f;
    QVector3D translation = minPos + QVector3D( mCylinderLength * 0.5f - textWidth / 2.0f, mCylinderLength * 0.5f - textHeight / 2.0f, mCylinderLength * 1.01f );
    QMatrix4x4 rotation;
    mCubeLabels << addCubeText( text, textHeight, textWidth, font, rotation, translation );
  }

  {
    text = u"btm"_s;
    textWidth = static_cast<float>( text.length() * fontSize ) * 0.75f;
    QVector3D translation = minPos + QVector3D( mCylinderLength * 0.5f - textWidth / 2.0f, mCylinderLength * 0.5f + textHeight / 2.0f, -mCylinderLength * 0.01f );
    QMatrix4x4 rotation;
    rotation.rotate( 180.0f, QVector3D( 1.0f, 0.0f, 0.0f ).normalized() );
    mCubeLabels << addCubeText( text, textHeight, textWidth, font, rotation, translation );
  }

  {
    text = u"west"_s;
    textWidth = static_cast<float>( text.length() * fontSize ) * 0.75f;
    QVector3D translation = minPos + QVector3D( -mCylinderLength * 0.01f, mCylinderLength * 0.5f + textWidth / 2.0f, mCylinderLength * 0.5f - textHeight / 2.0f );
    QMatrix4x4 rotation;
    rotation.rotate( 90.0f, QVector3D( 0.0f, -1.0f, 0.0f ).normalized() );
    rotation.rotate( 90.0f, QVector3D( 0.0f, 0.0f, -1.0f ).normalized() );
    mCubeLabels << addCubeText( text, textHeight, textWidth, font, rotation, translation );
  }

  {
    text = u"east"_s;
    textWidth = static_cast<float>( text.length() * fontSize ) * 0.75f;
    QVector3D translation = minPos + QVector3D( mCylinderLength * 1.01f, mCylinderLength * 0.5f - textWidth / 2.0f, mCylinderLength * 0.5f - textHeight / 2.0f );
    QMatrix4x4 rotation;
    rotation.rotate( 90.0f, QVector3D( 0.0f, 1.0f, 0.0f ).normalized() );
    rotation.rotate( 90.0f, QVector3D( 0.0f, 0.0f, 1.0f ).normalized() );
    mCubeLabels << addCubeText( text, textHeight, textWidth, font, rotation, translation );
  }

  {
    text = u"south"_s;
    textWidth = static_cast<float>( text.length() * fontSize ) * 0.75f;
    QVector3D translation = minPos + QVector3D( mCylinderLength * 0.5f - textWidth / 2.0f, -mCylinderLength * 0.01f, mCylinderLength * 0.5f - textHeight / 2.0f );
    QMatrix4x4 rotation;
    rotation.rotate( 90.0f, QVector3D( 1.0f, 0.0f, 0.0f ).normalized() );
    mCubeLabels << addCubeText( text, textHeight, textWidth, font, rotation, translation );
  }

  {
    text = u"north"_s;
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

void Qgs3DAxis::onAxisViewportSizeUpdate()
{
  mRenderView->onViewportSizeUpdate(); // will call onViewportScaleFactorChanged as callback

  // mRenderView->onViewportSizeUpdate() has updated `mTwoDLabelCamera` lens parameters.
  // The position of the labels needs to be updated.
  const Qgs3DAxisSettings axisSettings = mMapSettings->get3DAxisSettings();
  if ( axisSettings.mode() == Qgs3DAxisSettings::Mode::Crs && mAxisRoot->isEnabled() )
  {
    updateAxisLabelPosition();
  }
}

void Qgs3DAxis::onViewportScaleFactorChanged( double scaleFactor )
{
  // if the axis scene has not been created, don't do anything
  if ( !mAxisRoot || !mCubeRoot )
  {
    return;
  }

  if ( scaleFactor > 0.0 )
  {
    Qgs3DAxisSettings settings = mMapSettings->get3DAxisSettings();
    if ( settings.mode() == Qgs3DAxisSettings::Mode::Crs )
      setEnableAxis( true );
    else if ( settings.mode() == Qgs3DAxisSettings::Mode::Cube )
      setEnableCube( true );

    mAxisScaleFactor = scaleFactor;
    QgsDebugMsgLevel( QString( "3DAxis viewport mAxisScaleFactor %1" ).arg( mAxisScaleFactor ), 2 );
  }
  else
  {
    setEnableCube( false );
    setEnableAxis( false );
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
