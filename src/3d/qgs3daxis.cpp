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
#include <Qt3DRender/QAttribute>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QConeMesh>
#include <Qt3DRender/qcameralens.h>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QPointLight>
#include <QShortcut>
#include<ctime>

#include "qgs3dmapscene.h"
#include "qgsterrainentity_p.h"
#include "qgs3dmapsettings.h"
#include "qgscoordinatereferencesystemutils.h"
#include "qgscoordinatereferencesystem.h"
#include "qgswindow3dengine.h"

Qgs3DAxis::Qgs3DAxis( Qt3DExtras::Qt3DWindow *parentWindow
                      , Qt3DCore::QEntity *parent3DScene
                      , Qgs3DMapScene *mapScene
                      , QgsCameraController *cameraCtrl
                      , Qgs3DMapSettings &map )
  : QObject( parentWindow )
  , mMapSettings( map )
  , mParentWindow( parentWindow )
  , mMapScene( mapScene )
  , mCameraController( cameraCtrl )
  , mCrs( map.crs() )
  , mPreviousCursor( Qt::ArrowCursor )
{
  mAxisViewport = constructAxisViewport( parent3DScene );
  mAxisViewport->setParent( mParentWindow->activeFrameGraph() );

  mTwoDLabelViewport = constructLabelViewport( parent3DScene, QRectF( 0.0f, 0.0f, 1.0f, 1.0f ) );
  mTwoDLabelViewport->setParent( mParentWindow->activeFrameGraph() );

  Qgs3DAxisSettings s = mMapSettings.get3dAxisSettings();
  setAxisViewportPosition( mAxisViewportSize, s.verticalPosition(), s.horizontalPosition() );
  mMode = s.mode();

  connect( cameraCtrl, &QgsCameraController::cameraChanged, this, &Qgs3DAxis::onCameraUpdate );
  connect( mParentWindow, &Qt3DExtras::Qt3DWindow::widthChanged, this, &Qgs3DAxis::onAxisViewportSizeUpdate );
  connect( mParentWindow, &Qt3DExtras::Qt3DWindow::heightChanged, this, &Qgs3DAxis::onAxisViewportSizeUpdate );

  onAxisViewportSizeUpdate();

  createAxisScene();

  createMenu();

  init3DObjectPicking();
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

  // handle QEvent::MouseButtonRelease as it represents the end of click and QEvent::MouseMove.
  if ( event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseMove )
  {
    QMouseEvent *lastClickEvent = static_cast<QMouseEvent *>( event );
    // limit ray caster usage to the axis viewport
    QPointF normalizedPos( ( float )lastClickEvent->pos().x() / mParentWindow->width(),
                           ( float )lastClickEvent->pos().y() / mParentWindow->height() );
#ifdef DEBUG
    if ( event->type() == QEvent::MouseButtonRelease )
      qDebug() << "normalized pos:" << normalizedPos << "/ viewport:" << mAxisViewport->normalizedRect();
#endif

    if ( mAxisViewport->normalizedRect().contains( normalizedPos ) )
    {
      mLastClickedButton = lastClickEvent->button();
      mLastClickedPos = lastClickEvent->pos();

      // if casted ray from pos matches an entity, call onTouchedByRay
      mScreenRayCaster->trigger( mLastClickedPos );
    }

    // when we exit the viewport, reset the mouse cursor if needed
    else if ( mPreviousCursor != Qt::ArrowCursor && mParentWindow->cursor() == Qt::ArrowCursor )
    {
      mParentWindow->setCursor( mPreviousCursor );
      mPreviousCursor = Qt::ArrowCursor;
    }
  }

  return false;
}

void Qgs3DAxis::onTouchedByRay( const Qt3DRender::QAbstractRayCaster::Hits &hits )
{
  int mHitsFound = -1;
  if ( hits.length() )
  {
#ifdef DEBUG
    qDebug() << hits.length() << "hit(s) at " << mLastClickedPos << "with" << mLastClickedButton;
    for ( int i = 0; i < hits.length(); ++i )
    {
      qDebug() << "\tHit Type: " << hits.at( i ).type();
      qDebug() << "\tHit triangle id: " << hits.at( i ).primitiveIndex();
      qDebug() << "\tHit distance: " << hits.at( i ).distance();
      qDebug() << "\tHit entity name: " << hits.at( i ).entity()->objectName();
    }
#endif

    for ( int i = 0; i < hits.length() && mHitsFound == -1; ++i )
    {
      if ( hits.at( i ).distance() < 500.0 && ( hits.at( i ).entity() == mCubeRoot || hits.at( i ).entity() == mAxisRoot || hits.at( i ).entity()->parent() == mCubeRoot || hits.at( i ).entity()->parent() == mAxisRoot ) )
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
      }
    }
  }

  else if ( mLastClickedButton == Qt::MouseButton::RightButton && mHitsFound != -1 ) // show menu
  {
    displayMenuAt( mLastClickedPos );
  }

  else if ( mLastClickedButton == Qt::MouseButton::LeftButton ) // handle cube face clicks
  {
    if ( mMenu->isVisible() )
      hideMenu();

    if ( mHitsFound != -1 )
      if ( hits.at( mHitsFound ).entity() == mCubeRoot || hits.at( mHitsFound ).entity()->parent() == mCubeRoot )
      {
#ifdef DEBUG
        switch ( hits.at( mHitsFound ).primitiveIndex() / 2 )
        {
          case 0:
            qDebug() << "East face";
            break;

          case 1:
            qDebug() << "West face ";
            break;

          case 2:
            qDebug() << "North face ";
            break;

          case 3:
            qDebug() << "South face";
            break;

          case 4:
            qDebug() << "Top face ";
            break;

          case 5:
            qDebug() << "Bottom face ";
            break;
        }
#endif
        switch ( hits.at( mHitsFound ).primitiveIndex() / 2 )
        {
          case 0: // "East face";
            onCameraViewChangeEast();
            break;

          case 1: // "West face ";
            onCameraViewChangeWest();
            break;

          case 2: // "North face ";
            onCameraViewChangeNorth();
            break;

          case 3: // "South face";
            onCameraViewChangeSouth();
            break;

          case 4: // "Top face ";
            onCameraViewChangeTop();
            break;

          case 5: // "Bottom face ";
            onCameraViewChangeBottom();
            break;

          default:
            break;
        }

        mParentWindow->requestUpdate();
      }
  }

}

Qt3DRender::QViewport *Qgs3DAxis::constructAxisViewport( Qt3DCore::QEntity *parent3DScene )
{
  auto axisViewport = new Qt3DRender::QViewport;
  // parent will be set later
  // size will be set later

  mAxisSceneLayer = new Qt3DRender::QLayer;

  mAxisSceneEntity = new Qt3DCore::QEntity;
  mAxisSceneEntity->setParent( parent3DScene );
  mAxisSceneEntity->setObjectName( "3DAxis_SceneEntity" );

  mAxisCamera = new Qt3DRender::QCamera;
  mAxisCamera->setParent( mAxisSceneEntity );
  mAxisCamera->setProjectionType( mCameraController->camera()->projectionType() );
  mAxisCamera->lens()->setFieldOfView( mCameraController->camera()->lens()->fieldOfView() * 0.5 );

  mAxisCamera->setUpVector( QVector3D( 0.0f, 0.0f, 1.0f ) );
  mAxisCamera->setViewCenter( QVector3D( 0.0f, 0.0f, 0.0f ) );
  // position will be set later

  auto axisLayer = new Qt3DRender::QLayer;
  axisLayer->setRecursive( true );
  mAxisSceneEntity->addComponent( axisLayer );

  auto axisLayerFilter = new Qt3DRender::QLayerFilter( axisViewport );
  axisLayerFilter->addLayer( axisLayer );

  auto axisCameraSelector = new Qt3DRender::QCameraSelector;
  axisCameraSelector->setParent( axisLayerFilter );
  axisCameraSelector->setCamera( mAxisCamera );

  auto clearBuffers = new Qt3DRender::QClearBuffers( axisCameraSelector );
  clearBuffers->setBuffers( Qt3DRender::QClearBuffers::DepthBuffer );

  return axisViewport;
}

Qt3DRender::QViewport *Qgs3DAxis::constructLabelViewport( Qt3DCore::QEntity *parent3DScene, const QRectF &parentViewportSize )
{
  auto twoDViewport = new Qt3DRender::QViewport;
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

  auto twoDLayer = new Qt3DRender::QLayer;
  twoDLayer->setRecursive( true );
  mTwoDLabelSceneEntity->addComponent( twoDLayer );

  auto twoDLayerFilter = new Qt3DRender::QLayerFilter( twoDViewport );
  twoDLayerFilter->addLayer( twoDLayer );

  auto twoDCameraSelector = new Qt3DRender::QCameraSelector;
  twoDCameraSelector->setParent( twoDLayerFilter );
  twoDCameraSelector->setCamera( mTwoDLabelCamera );

  auto clearBuffers = new Qt3DRender::QClearBuffers( twoDCameraSelector );
  clearBuffers->setBuffers( Qt3DRender::QClearBuffers::DepthBuffer );

  return twoDViewport;
}

QVector3D Qgs3DAxis::from3dTo2dLabelPosition( const QVector3D &sourcePos,
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
                                         0.0 );
  destPos -= QVector3D( labelCenter.x() * destSize.width(),
                        labelCenter.y() * destSize.height(),
                        0.0f );
  destPos.setX( destPos.x() + viewTranslation.x() );
  destPos.setY( destPos.y() - viewTranslation.y() );
  destPos.setZ( 0.0f );

#ifdef DEBUG
  qDebug() << "from3dTo2dLabelPosition: sourcePos" << sourcePos << " with" << viewTranslation << "corrected destPos" << destPos;
#endif
  return destPos;
}

void Qgs3DAxis::setEnableCube( bool show )
{
  mCubeRoot->setEnabled( show );
}

void Qgs3DAxis::setEnableAxis( bool show )
{
  mAxisRoot->setEnabled( show );
  mText_X->setEnabled( show );
  mText_Y->setEnabled( show );
  mText_Z->setEnabled( show );
}

void Qgs3DAxis::createAxisScene()
{
  if ( mAxisRoot == nullptr || mCubeRoot == nullptr )
  {
#ifdef DEBUG
    qDebug() << "Should recreate mAxisRoot" << mMode;
#endif
    mAxisRoot = new Qt3DCore::QEntity;
    mAxisRoot->setParent( mAxisSceneEntity );
    mAxisRoot->setObjectName( "3DAxis_AxisRoot" );
    mAxisRoot->addComponent( mAxisSceneLayer );

    createAxis( Axis::X );
    createAxis( Axis::Y );
    createAxis( Axis::Z );

    mCubeRoot = new Qt3DCore::QEntity;
    mCubeRoot->setParent( mAxisSceneEntity );
    mCubeRoot->setObjectName( "3DAxis_CubeRoot" );
    mCubeRoot->addComponent( mAxisSceneLayer );

    createCube( );
  }

  if ( mMode == Mode::Off )
  {
    mAxisSceneEntity->setEnabled( false );
    setEnableAxis( false );
    setEnableCube( false );
  }
  else
  {
    mAxisSceneEntity->setEnabled( true );
    if ( mMode == Mode::Crs )
    {
      setEnableCube( false );
      setEnableAxis( true );

      auto axisDirections = mCrs.axisOrdering();

      mText_X->setText( QgsCoordinateReferenceSystemUtils::axisDirectionToAbbreviatedString( axisDirections.at( 0 ) ) );

      if ( axisDirections.length() > 1 )
        mText_Y->setText( QgsCoordinateReferenceSystemUtils::axisDirectionToAbbreviatedString( axisDirections.at( 1 ) ) );
      else
        mText_Y->setText( "Y?" );

      if ( axisDirections.length() > 2 )
        mText_Z->setText( QgsCoordinateReferenceSystemUtils::axisDirectionToAbbreviatedString( axisDirections.at( 2 ) ) );
      else
        mText_Z->setText( QStringLiteral( "up" ) );
    }
    else if ( mMode == Mode::Cube )
    {
      setEnableCube( true );
      setEnableAxis( false );
    }
    else
    {
      setEnableCube( false );
      setEnableAxis( true );
      mText_X->setText( "X?" );
      mText_Y->setText( "Y?" );
      mText_Z->setText( "Z?" );
    }

    updateAxisLabelPosition();
  }
}

void Qgs3DAxis::createMenu()
{
  mMenu = new QMenu;
  // ============== axis type menu
  auto typeOffAct = new QAction( tr( "&Off" ), this );
  typeOffAct->setCheckable( true );
  typeOffAct->setStatusTip( tr( "Disable 3D axis" ) );
  if ( mMode == Mode::Off ) typeOffAct->setChecked( true );

  auto typeCrsAct = new QAction( tr( "Cr&s" ), this );
  typeCrsAct->setCheckable( true );
  typeCrsAct->setStatusTip( tr( "Crs 3D axis" ) );
  if ( mMode == Mode::Crs ) typeCrsAct->setChecked( true );

  auto typeCubeAct = new QAction( tr( "&Cube" ), this );
  typeCubeAct->setCheckable( true );
  typeCubeAct->setStatusTip( tr( "Cube 3D axis" ) );
  if ( mMode == Mode::Cube ) typeCubeAct->setChecked( true );

  auto typeGroup = new QActionGroup( this );
  typeGroup->addAction( typeOffAct );
  typeGroup->addAction( typeCrsAct );
  typeGroup->addAction( typeCubeAct );

  connect( typeOffAct, &QAction::triggered, this, [this]( bool ) {onAxisModeChanged( Mode::Off );} );
  connect( typeCrsAct, &QAction::triggered, this, [this]( bool ) {onAxisModeChanged( Mode::Crs );} );
  connect( typeCubeAct, &QAction::triggered, this, [this]( bool ) {onAxisModeChanged( Mode::Cube );} );

  auto typeMenu = new QMenu( QStringLiteral( "Axis type" ) );
  typeMenu->addAction( typeOffAct );
  typeMenu->addAction( typeCrsAct );
  typeMenu->addAction( typeCubeAct );
  mMenu->addMenu( typeMenu );

  // ============== horizontal position menu
  auto hPosLeftAct = new QAction( tr( "&Left" ), this );
  hPosLeftAct->setCheckable( true );
  if ( mAxisViewportVertPos == AxisViewportPosition::Begin ) hPosLeftAct->setChecked( true );

  auto hPosMiddleAct = new QAction( tr( "&Middle" ), this );
  hPosMiddleAct->setCheckable( true );
  if ( mAxisViewportVertPos == AxisViewportPosition::Middle ) hPosMiddleAct->setChecked( true );

  auto hPosRightAct = new QAction( tr( "&Right" ), this );
  hPosRightAct->setCheckable( true );
  if ( mAxisViewportVertPos == AxisViewportPosition::End ) hPosRightAct->setChecked( true );

  auto hPosGroup = new QActionGroup( this );
  hPosGroup->addAction( hPosLeftAct );
  hPosGroup->addAction( hPosMiddleAct );
  hPosGroup->addAction( hPosRightAct );

  connect( hPosLeftAct, &QAction::triggered, this, [this]( bool ) {onAxisHorizPositionChanged( AxisViewportPosition::Begin );} );
  connect( hPosMiddleAct, &QAction::triggered, this, [this]( bool ) {onAxisHorizPositionChanged( AxisViewportPosition::Middle );} );
  connect( hPosRightAct, &QAction::triggered, this, [this]( bool ) {onAxisHorizPositionChanged( AxisViewportPosition::End );} );

  auto horizPosMenu = new QMenu( QStringLiteral( "Horizontal position" ) );
  horizPosMenu->addAction( hPosLeftAct );
  horizPosMenu->addAction( hPosMiddleAct );
  horizPosMenu->addAction( hPosRightAct );
  mMenu->addMenu( horizPosMenu );

  // ============== vertical position menu
  auto vPosTopAct = new QAction( tr( "&Top" ), this );
  vPosTopAct->setCheckable( true );
  if ( mAxisViewportVertPos == AxisViewportPosition::Begin ) vPosTopAct->setChecked( true );

  auto vPosMiddleAct = new QAction( tr( "&Middle" ), this );
  vPosMiddleAct->setCheckable( true );
  if ( mAxisViewportVertPos == AxisViewportPosition::Middle ) vPosMiddleAct->setChecked( true );

  auto vPosBottomAct = new QAction( tr( "&Bottom" ), this );
  vPosBottomAct->setCheckable( true );
  if ( mAxisViewportVertPos == AxisViewportPosition::End ) vPosBottomAct->setChecked( true );

  auto vPosGroup = new QActionGroup( this );
  vPosGroup->addAction( vPosTopAct );
  vPosGroup->addAction( vPosMiddleAct );
  vPosGroup->addAction( vPosBottomAct );

  connect( vPosTopAct, &QAction::triggered, this, [this]( bool ) {onAxisVertPositionChanged( AxisViewportPosition::Begin );} );
  connect( vPosMiddleAct, &QAction::triggered, this, [this]( bool ) {onAxisVertPositionChanged( AxisViewportPosition::Middle );} );
  connect( vPosBottomAct, &QAction::triggered, this, [this]( bool ) {onAxisVertPositionChanged( AxisViewportPosition::End );} );

  auto vertPosMenu = new QMenu( QStringLiteral( "Vertical position" ) );
  vertPosMenu->addAction( vPosTopAct );
  vertPosMenu->addAction( vPosMiddleAct );
  vertPosMenu->addAction( vPosBottomAct );
  mMenu->addMenu( vertPosMenu );

  // ============== axis view menu
  auto viewHomeAct = new QAction( tr( "&Home" ) + "\t Ctrl+1", this );
  auto viewTopAct = new QAction( tr( "&Top" ) + "\t Ctrl+5", this );
  auto viewNorthAct = new QAction( tr( "&North" ) + "\t Ctrl+8", this );
  auto viewEastAct = new QAction( tr( "&East" ) + "\t Ctrl+6", this );
  auto viewSouthAct = new QAction( tr( "&South" ) + "\t Ctrl+2", this );
  auto viewWestAct = new QAction( tr( "&West" ) + "\t Ctrl+4", this );
  auto viewBottomAct = new QAction( tr( "&Bottom" ), this );

  connect( viewHomeAct, &QAction::triggered, this, &Qgs3DAxis::onCameraViewChangeHome );
  connect( viewTopAct, &QAction::triggered, this, &Qgs3DAxis::onCameraViewChangeTop );
  connect( viewNorthAct, &QAction::triggered, this, &Qgs3DAxis::onCameraViewChangeNorth );
  connect( viewEastAct, &QAction::triggered, this, &Qgs3DAxis::onCameraViewChangeEast );
  connect( viewSouthAct, &QAction::triggered, this, &Qgs3DAxis::onCameraViewChangeSouth );
  connect( viewWestAct, &QAction::triggered, this, &Qgs3DAxis::onCameraViewChangeWest );
  connect( viewBottomAct, &QAction::triggered, this, &Qgs3DAxis::onCameraViewChangeBottom );

  QgsWindow3DEngine *eng = dynamic_cast<QgsWindow3DEngine *>( mMapScene->engine() );
  if ( eng )
  {
    QWidget *mapCanvas = dynamic_cast<QWidget *>( eng->parent() );
    if ( mapCanvas == nullptr )
      qDebug() << "NO CANVAS!";
    else
    {
      auto shortcutHome = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_1 ), mapCanvas );
      connect( shortcutHome, &QShortcut::activated, this, [this]( ) {onCameraViewChangeHome();} );

      auto shortcutTop = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_5 ), mapCanvas );
      connect( shortcutTop, &QShortcut::activated, this, [this]( ) {onCameraViewChangeTop();} );

      auto shortcutNorth = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_8 ), mapCanvas );
      connect( shortcutNorth, &QShortcut::activated, this, [this]( ) {onCameraViewChangeNorth();} );

      auto shortcutEast = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_6 ), mapCanvas );
      connect( shortcutEast, &QShortcut::activated, this, [this]( ) {onCameraViewChangeEast();} );

      auto shortcutSouth = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_2 ), mapCanvas );
      connect( shortcutSouth, &QShortcut::activated, this, [this]( ) {onCameraViewChangeSouth();} );

      auto shortcutWest = new QShortcut( QKeySequence( Qt::CTRL + Qt::Key_4 ), mapCanvas );
      connect( shortcutWest, &QShortcut::activated, this, [this]( ) {onCameraViewChangeWest();} );
    }
  }

  auto viewMenu = new QMenu( QStringLiteral( "Camera view" ) );
  viewMenu->addAction( viewHomeAct );
  viewMenu->addAction( viewTopAct );
  viewMenu->addAction( viewNorthAct );
  viewMenu->addAction( viewEastAct );
  viewMenu->addAction( viewSouthAct );
  viewMenu->addAction( viewWestAct );
  viewMenu->addAction( viewBottomAct );
  mMenu->addMenu( viewMenu );
}

void Qgs3DAxis::hideMenu()
{
  mMenu->hide();
}

void Qgs3DAxis::displayMenuAt( const QPoint &sourcePos )
{
  mMenu->popup( mParentWindow->parent()->mapToGlobal( sourcePos ) );
}

void Qgs3DAxis::onAxisModeChanged( Qgs3DAxis::Mode mode )
{
  setMode( mode );
  Qgs3DAxisSettings s = mMapSettings.get3dAxisSettings();
  s.setMode( mode );
  mMapSettings.set3dAxisSettings( s );
}

void Qgs3DAxis::onAxisHorizPositionChanged( AxisViewportPosition pos )
{
  setAxisViewportPosition( mAxisViewportSize, mAxisViewportVertPos, pos );
  Qgs3DAxisSettings s = mMapSettings.get3dAxisSettings();
  s.setHorizontalPosition( pos );
  mMapSettings.set3dAxisSettings( s );
}

void Qgs3DAxis::onAxisVertPositionChanged( AxisViewportPosition pos )
{
  setAxisViewportPosition( mAxisViewportSize, pos, mAxisViewportHorizPos );
  Qgs3DAxisSettings s = mMapSettings.get3dAxisSettings();
  s.setVerticalPosition( pos );
  mMapSettings.set3dAxisSettings( s );
}

void Qgs3DAxis::onCameraViewChange( float pitch, float yaw )
{
  QgsVector3D pos = mCameraController->lookingAtPoint();
  if ( mMapSettings.terrainRenderingEnabled() )
    pos.set( pos.x(), mMapScene->terrainEntity()->terrainElevationOffset(), pos.z() );
  else
    pos.set( pos.x(), 0.0, pos.z() );

  mCameraController->setLookingAtPoint( pos, ( mCameraController->camera()->position() - pos.toVector3D() ).length(),
                                        pitch, yaw );
}


void Qgs3DAxis::createCube( )
{
  QVector3D minPos = QVector3D( -mCylinderLength * 0.5f, -mCylinderLength * 0.5f, -mCylinderLength * 0.5f );

  // cube outlines
  auto cubeLineEntity = new Qt3DCore::QEntity( mCubeRoot );
  cubeLineEntity->setObjectName( "3DAxis_cubeline" );
  Qgs3DWiredMesh *cubeLine = new Qgs3DWiredMesh;
  auto box = QgsAABB( -mCylinderLength * 0.5f, -mCylinderLength * 0.5f, -mCylinderLength * 0.5f,
                      mCylinderLength * 0.5f, mCylinderLength * 0.5f, mCylinderLength * 0.5f );
  cubeLine->setVertices( box.verticesForLines() );
  cubeLineEntity->addComponent( cubeLine );

  auto cubeLineMaterial = new Qt3DExtras::QPhongMaterial;
  cubeLineMaterial->setAmbient( Qt::white );
  cubeLineEntity->addComponent( cubeLineMaterial );

  // cube mesh
  auto cubeMesh = new Qt3DExtras::QCuboidMesh;
  cubeMesh->setObjectName( "3DAxis_cubemesh" );
  cubeMesh->setXExtent( mCylinderLength );
  cubeMesh->setYExtent( mCylinderLength );
  cubeMesh->setZExtent( mCylinderLength );
  mCubeRoot->addComponent( cubeMesh );

  auto cubeMaterial = new Qt3DExtras::QPhongMaterial( mCubeRoot );
  cubeMaterial->setAmbient( QColor( 100, 100, 100, 50 ) );
  cubeMaterial->setShininess( 100 );
  mCubeRoot->addComponent( cubeMaterial );

  auto cubeTransform = new Qt3DCore::QTransform;
  QMatrix4x4 transformMatrixcube;
  //transformMatrixcube.rotate( rotation );
  transformMatrixcube.translate( minPos + QVector3D( mCylinderLength * 0.5f, mCylinderLength * 0.5f, mCylinderLength * 0.5f ) );
  cubeTransform->setMatrix( transformMatrixcube );
  mCubeRoot->addComponent( cubeTransform );

  // text
  QString text;
  int fontSize = 0.75 * mFontSize;
  float textHeight = fontSize * 1.5;
  float textWidth;
  auto f = QFont( "monospace", fontSize );
  f.setWeight( QFont::Weight::Black );

  {
    text = QStringLiteral( "top" );
    textWidth = text.length() * fontSize * 0.75;
    QVector3D translation = minPos + QVector3D(
                              mCylinderLength * 0.5f - textWidth / 2.0f,
                              mCylinderLength * 0.5f - textHeight / 2.0f,
                              mCylinderLength * 1.01f );
    QMatrix4x4 rotation;
    mCubeLabels << addCubeText( text, textHeight, textWidth, f, rotation, translation );
  }

  {
    text = QStringLiteral( "btm" );
    textWidth = text.length() * fontSize * 0.75;
    QVector3D translation = minPos + QVector3D(
                              mCylinderLength * 0.5f - textWidth / 2.0f,
                              mCylinderLength * 0.5f + textHeight / 2.0f,
                              -mCylinderLength * 0.01f );
    QMatrix4x4 rotation;
    rotation.rotate( 180.0f, QVector3D( 1.0, 0.0, 0.0 ).normalized() );
    mCubeLabels << addCubeText( text, textHeight, textWidth, f, rotation, translation );
  }

  {
    text = QStringLiteral( "west" );
    textWidth = text.length() * fontSize * 0.75;
    QVector3D translation = minPos + QVector3D(
                              - mCylinderLength * 0.01f,
                              mCylinderLength * 0.5f + textWidth / 2.0f,
                              mCylinderLength * 0.5f - textHeight / 2.0f );
    QMatrix4x4 rotation;
    rotation.rotate( 90.0f, QVector3D( 0.0, -1.0, 0.0 ).normalized() );
    rotation.rotate( 90.0f, QVector3D( 0.0, 0.0, -1.0 ).normalized() );
    mCubeLabels << addCubeText( text, textHeight, textWidth, f, rotation, translation );
  }

  {
    text = QStringLiteral( "east" );
    textWidth = text.length() * fontSize * 0.75;
    QVector3D translation = minPos + QVector3D(
                              mCylinderLength * 1.01f,
                              mCylinderLength * 0.5f - textWidth / 2.0f,
                              mCylinderLength * 0.5f - textHeight / 2.0f );
    QMatrix4x4 rotation;
    rotation.rotate( 90.0f, QVector3D( 0.0, 1.0, 0.0 ).normalized() );
    rotation.rotate( 90.0f, QVector3D( 0.0, 0.0, 1.0 ).normalized() );
    mCubeLabels << addCubeText( text, textHeight, textWidth, f, rotation, translation );
  }

  {
    text = QStringLiteral( "south" );
    textWidth = text.length() * fontSize * 0.75;
    QVector3D translation = minPos + QVector3D(
                              mCylinderLength * 0.5f - textWidth / 2.0f,
                              - mCylinderLength * 0.01f,
                              mCylinderLength * 0.5f - textHeight / 2.0f );
    QMatrix4x4 rotation;
    rotation.rotate( 90.0f, QVector3D( 1.0, 0.0, 0.0 ).normalized() );
    mCubeLabels << addCubeText( text, textHeight, textWidth, f, rotation, translation );
  }

  {
    text = QStringLiteral( "north" );
    textWidth = text.length() * fontSize * 0.75;
    QVector3D translation = minPos + QVector3D(
                              mCylinderLength * 0.5f + textWidth / 2.0f,
                              mCylinderLength * 1.01f,
                              mCylinderLength * 0.5f - textHeight / 2.0f );
    QMatrix4x4 rotation;
    rotation.rotate( 90.0f, QVector3D( -1.0, 0.0, 0.0 ).normalized() );
    rotation.rotate( 180.0f, QVector3D( 0.0, 0.0, 1.0 ).normalized() );
    mCubeLabels << addCubeText( text, textHeight, textWidth, f, rotation, translation );
  }

  for ( auto l : std::as_const( mCubeLabels ) )
  {
    l->setParent( mCubeRoot );
  }
}

Qt3DExtras::QText2DEntity *Qgs3DAxis::addCubeText( const QString &text, float textHeight, float textWidth, const QFont &f, const QMatrix4x4 &rotation, const QVector3D &translation )
{
  auto  textEntity = new Qt3DExtras::QText2DEntity;
  textEntity->setObjectName( "3DAxis_cube_label_" + text );
  textEntity->setFont( f );
  textEntity->setHeight( textHeight );
  textEntity->setWidth( textWidth );
  textEntity->setColor( QColor( 192, 192, 192 ) );
  textEntity->setText( text );

  auto textFrontTransform = new Qt3DCore::QTransform();
  textFrontTransform->setMatrix( rotation );
  textFrontTransform->setTranslation( translation );
  textEntity->addComponent( textFrontTransform );

  return textEntity;
}

void Qgs3DAxis::createAxis( const Qgs3DAxis::Axis &axisType )
{
  float cylinderRadius = 0.05 * mCylinderLength;
  float coneLength = 0.3 * mCylinderLength;
  float coneBottomRadius = 0.1 * mCylinderLength;

  QQuaternion rotation;
  QColor color;

  Qt3DExtras::QText2DEntity *text;
  Qt3DCore::QTransform *textTransform;
  QString name;

  switch ( axisType )
  {
    case Axis::X:
      mText_X = new Qt3DExtras::QText2DEntity( );  // object initialization in two step:
      mText_X->setParent( mTwoDLabelSceneEntity ); // see https://bugreports.qt.io/browse/QTBUG-77139
      connect( mText_X, &Qt3DExtras::QText2DEntity::textChanged, this, [this]( const QString & text )
      {
        mText_X->setWidth( mFontSize * text.length() );
      } );
      mTextTransform_X = new Qt3DCore::QTransform();
      mTextCoord_X = QVector3D( mCylinderLength + coneLength / 2.0, 0.0f, 0.0f );

      rotation = QQuaternion::fromAxisAndAngle( QVector3D( 0.0f, 0.0f, 1.0f ), -90.0f );
      color = Qt::red;
      text = mText_X;
      textTransform = mTextTransform_X;
      name = "3DAxis_axisX";
      break;

    case Axis::Y:
      mText_Y = new Qt3DExtras::QText2DEntity( );  // object initialization in two step:
      mText_Y->setParent( mTwoDLabelSceneEntity ); // see https://bugreports.qt.io/browse/QTBUG-77139
      connect( mText_Y, &Qt3DExtras::QText2DEntity::textChanged, this, [this]( const QString & text )
      {
        mText_Y->setWidth( mFontSize * text.length() );
      } );
      mTextTransform_Y = new Qt3DCore::QTransform();
      mTextCoord_Y = QVector3D( 0.0f, mCylinderLength + coneLength / 2.0, 0.0f );

      rotation = QQuaternion::fromAxisAndAngle( QVector3D( 0.0f, 0.0f, 0.0f ), 0.0f );
      color = Qt::green;
      text = mText_Y;
      textTransform = mTextTransform_Y;
      name = "3DAxis_axisY";
      break;

    case Axis::Z:
      mText_Z = new Qt3DExtras::QText2DEntity( );  // object initialization in two step:
      mText_Z->setParent( mTwoDLabelSceneEntity ); // see https://bugreports.qt.io/browse/QTBUG-77139
      connect( mText_Z, &Qt3DExtras::QText2DEntity::textChanged, this, [this]( const QString & text )
      {
        mText_Z->setWidth( mFontSize * text.length() );
      } );
      mTextTransform_Z = new Qt3DCore::QTransform();
      mTextCoord_Z = QVector3D( 0.0f, 0.0f, mCylinderLength + coneLength / 2.0 );

      rotation = QQuaternion::fromAxisAndAngle( QVector3D( 1.0f, 0.0f, 0.0f ), 90.0f );
      color = Qt::blue;
      text = mText_Z;
      textTransform = mTextTransform_Z;
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

  // text
  QFont f = QFont( "monospace", mFontSize ); // TODO: should use outlined font
  f.setWeight( QFont::Weight::Black );
  f.setStyleStrategy( QFont::StyleStrategy::ForceOutline );
  text->setFont( f );
  text->setHeight( mFontSize * 1.5 );
  text->setWidth( mFontSize );
  text->setColor( QColor( 192, 192, 192, 192 ) );
  text->addComponent( textTransform );
}


void Qgs3DAxis::setAxisViewportPosition( int axisViewportSize, AxisViewportPosition axisViewportVertPos, AxisViewportPosition axisViewportHorizPos )
{
  mAxisViewportSize = axisViewportSize;
  mAxisViewportVertPos = axisViewportVertPos;
  mAxisViewportHorizPos = axisViewportHorizPos;
  onAxisViewportSizeUpdate();
  mParentWindow->requestUpdate();
}

void Qgs3DAxis::onAxisViewportSizeUpdate( int )
{
  float widthRatio = ( float )mAxisViewportSize / mParentWindow->width();
  float heightRatio = ( float )mAxisViewportSize / mParentWindow->height();

  float xRatio;
  float yRatio;
  if ( mAxisViewportHorizPos == AxisViewportPosition::Begin )
    xRatio = 0.0f;
  else if ( mAxisViewportHorizPos == AxisViewportPosition::Middle )
    xRatio = 0.5 - widthRatio / 2.0;
  else
    xRatio = 1.0 - widthRatio;

  if ( mAxisViewportVertPos == AxisViewportPosition::Begin )
    yRatio = 0.0f;
  else if ( mAxisViewportVertPos == AxisViewportPosition::Middle )
    yRatio = 0.5 - heightRatio / 2.0;
  else
    yRatio = 1.0 - heightRatio;

#ifdef DEBUG
  qDebug() << "Axis, update viewport" << xRatio << yRatio << widthRatio << heightRatio;
#endif
  mAxisViewport->setNormalizedRect( QRectF( xRatio, yRatio, widthRatio, heightRatio ) );

  mTwoDLabelCamera->lens()->setOrthographicProjection(
    -mParentWindow->width() / 2.0f, mParentWindow->width() / 2.0f,
    -mParentWindow->height() / 2.0f, mParentWindow->height() / 2.0f,
    mTwoDLabelCamera->lens()->nearPlane(), mTwoDLabelCamera->lens()->farPlane() );

  updateAxisLabelPosition();
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
  if ( mTextTransform_X && mTextTransform_Y && mTextTransform_Z )
  {
    mTextTransform_X->setTranslation( from3dTo2dLabelPosition( mTextCoord_X, mAxisCamera, mAxisViewport,
                                      mTwoDLabelCamera, mTwoDLabelViewport,
                                      mParentWindow->size() ) );
    mTextTransform_Y->setTranslation( from3dTo2dLabelPosition( mTextCoord_Y, mAxisCamera, mAxisViewport,
                                      mTwoDLabelCamera, mTwoDLabelViewport,
                                      mParentWindow->size() ) );
    mTextTransform_Z->setTranslation( from3dTo2dLabelPosition( mTextCoord_Z, mAxisCamera, mAxisViewport,
                                      mTwoDLabelCamera, mTwoDLabelViewport,
                                      mParentWindow->size() ) );
  }
}

void Qgs3DAxis::setMode( Mode axisMode )
{
  if ( mMode != axisMode )
  {
    mMode = axisMode;
    createAxisScene();
    mParentWindow->requestUpdate();
  }
}


// ----------------

Qgs3DWiredMesh::Qgs3DWiredMesh( Qt3DCore::QNode *parent )
  : Qt3DRender::QGeometryRenderer( parent ),
    mPositionAttribute( new Qt3DRender::QAttribute( this ) ),
    mVertexBuffer( new Qt3DRender::QBuffer( this ) )
{
  mPositionAttribute->setAttributeType( Qt3DRender::QAttribute::VertexAttribute );
  mPositionAttribute->setBuffer( mVertexBuffer );
  mPositionAttribute->setVertexBaseType( Qt3DRender::QAttribute::Float );
  mPositionAttribute->setVertexSize( 3 );
  mPositionAttribute->setName( Qt3DRender::QAttribute::defaultPositionAttributeName() );

  mGeom = new Qt3DRender::QGeometry( this );
  mGeom->addAttribute( mPositionAttribute );

  setInstanceCount( 1 );
  setIndexOffset( 0 );
  setFirstInstance( 0 );
  setPrimitiveType( Qt3DRender::QGeometryRenderer::Lines );
  setGeometry( mGeom );
}

Qgs3DWiredMesh::~Qgs3DWiredMesh()
{
  // nope
}

void Qgs3DWiredMesh::setVertices( const QList<QVector3D> &vertices )
{
  QByteArray vertexBufferData;
  vertexBufferData.resize( vertices.size() * 3 * sizeof( float ) );
  float *rawVertexArray = reinterpret_cast<float *>( vertexBufferData.data() );
  int idx = 0;
  for ( const auto &v : std::as_const( vertices ) )
  {
    rawVertexArray[idx++] = v.x();
    rawVertexArray[idx++] = v.y();
    rawVertexArray[idx++] = v.z();
  }

  mVertexBuffer->setData( vertexBufferData );
  setVertexCount( vertices.count() );
}

// ----------------
