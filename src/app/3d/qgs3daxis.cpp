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

#include <Qt3DCore/QTransform>
#include <Qt3DExtras/QConeMesh>
#include <Qt3DExtras/QCylinderMesh>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/qcameralens.h>
#include<ctime>

#include "qgs3daxis.h"

Qgs3DAxis::Qgs3DAxis( Qt3DExtras::Qt3DWindow *parentWindow, Qt3DCore::QEntity *parent3DScene, QgsCameraController *cameraCtrl, const Qgs3DMapSettings *map )
  : QObject( parentWindow ), mParentWindow( parentWindow ), mParentCamera( cameraCtrl->camera() ),
    mCylinderLength( 40.0f ), mAxisViewportSize( 3.0 * mCylinderLength ),
    mAxisViewportVertPos( AxisViewportPosition::END ), mAxisViewportHorizPos( AxisViewportPosition::END ),
    mFontSize( 10 ), mMode( Mode::SRS ), mAxisRoot( nullptr ), mTextTransform_X( nullptr ), mTextTransform_Y( nullptr ),
    mTextTransform_Z( nullptr ), mCrs( map->crs() )
{
  mAxisViewport = constructAxisViewport( parent3DScene );
  mAxisViewport->setParent( mParentWindow->activeFrameGraph() );

  mTwoDLabelViewport = constructLabelViewport( parent3DScene, QRectF( 0.0f, 0.0f, 1.0f, 1.0f ) );
  mTwoDLabelViewport->setParent( mParentWindow->activeFrameGraph() );

  connect( cameraCtrl, &QgsCameraController::cameraChanged, this, &Qgs3DAxis::updateCamera );
  connect( mParentWindow, &Qt3DExtras::Qt3DWindow::widthChanged, this, &Qgs3DAxis::updateAxisViewportSize );
  connect( mParentWindow, &Qt3DExtras::Qt3DWindow::heightChanged, this, &Qgs3DAxis::updateAxisViewportSize );

  updateAxisViewportSize();

  createAxisScene();
}

Qt3DRender::QViewport *Qgs3DAxis::constructAxisViewport( Qt3DCore::QEntity *parent3DScene )
{
  auto axisViewport = new Qt3DRender::QViewport;
  // size will be set later

  mAxisSceneEntity = new Qt3DCore::QEntity;
  mAxisSceneEntity->setParent( parent3DScene );

  mAxisCamera = new Qt3DRender::QCamera;
  mAxisCamera->setParent( mAxisSceneEntity );
  mAxisCamera->setProjectionType( mParentCamera->projectionType() );
  // always ortho for now as it seem more "accurate"
  if ( true || mParentCamera->projectionType() == Qt3DRender::QCameraLens::ProjectionType::OrthographicProjection )
    mAxisCamera->lens()->setOrthographicProjection(
      -mAxisViewportSize / 2.0f, mAxisViewportSize / 2.0f,
      -mAxisViewportSize / 2.0f, mAxisViewportSize / 2.0f,
      -100.0f, 500.0f );
  else
    mAxisCamera->lens()->setPerspectiveProjection(
      mParentCamera->lens()->fieldOfView(), mParentCamera->lens()->aspectRatio(),
      1.0f, -250.0f );

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
  //QVector3D parentWin (mParentWindow->width(), mParentWindow->height(), 0.0);
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
  qDebug() << "sourcePos" << sourcePos << " with" << viewTranslation << "corrected destPos" << destPos;
#endif
  return destPos;
}

void Qgs3DAxis::createAxisScene()
{
  if ( mAxisRoot == nullptr )
  {
#ifdef DEBUG
    qDebug() << "Should recreate mAxisRoot" << mMode;
#endif
    mAxisRoot = new Qt3DCore::QEntity;
    mAxisRoot->setParent( mAxisSceneEntity );
    mAxisRoot->setObjectName( "3DAxisRoot" );

    createAxis( Axis::X );
    createAxis( Axis::Y );
    createAxis( Axis::Z );
  }

  if ( mMode == OFF )
  {
    mAxisRoot->setEnabled( false );
    mText_X->setEnabled( false );
    mText_Y->setEnabled( false );
    mText_Z->setEnabled( false );
  }
  else
  {
    mAxisRoot->setEnabled( true );
    mText_X->setEnabled( true );
    mText_Y->setEnabled( true );
    mText_Z->setEnabled( true );
    if ( mMode == SRS )
    {
      if ( mCrs.isGeographic() )
      {
        mText_X->setText( "Long" );
        mText_Y->setText( "Lat" );
      }
      else
      {
        mText_X->setText( "X" );
        mText_Y->setText( "Y" );
      }
      mText_Z->setText( "Z" );
    }
    else if ( mMode == NEU )
    {
      mText_X->setText( "East" );
      mText_Y->setText( "North" );
      mText_Z->setText( "Up" );
    }
    else
    {
      mText_X->setText( "X?" );
      mText_Y->setText( "Y?" );
      mText_Z->setText( "Z?" );
    }

    updateLabelPosition();
  }
}

void Qgs3DAxis::createAxis( const Qgs3DAxis::Axis &axis )
{
  float cylinderRadius = 0.05 * mCylinderLength;
  float coneLength = 0.3 * mCylinderLength;
  float coneBottomRadius = 0.1 * mCylinderLength;

  QQuaternion rotation;
  QColor color;

  Qt3DExtras::QText2DEntity *text;
  Qt3DCore::QTransform *textTransform;

  switch ( axis )
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
      break;

    default:
      return;
  }

  // cylinder
  Qt3DCore::QEntity *cylinder = new Qt3DCore::QEntity( mAxisRoot );
  Qt3DExtras::QCylinderMesh *cylinderMesh = new Qt3DExtras::QCylinderMesh;
  cylinderMesh->setRadius( cylinderRadius );
  cylinderMesh->setLength( mCylinderLength );
  cylinderMesh->setRings( 100 );
  cylinderMesh->setSlices( 20 );
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
  Qt3DExtras::QConeMesh *coneMesh = new Qt3DExtras::QConeMesh;
  coneMesh->setLength( coneLength );
  coneMesh->setBottomRadius( coneBottomRadius );
  coneMesh->setTopRadius( 0.0f );
  coneMesh->setRings( 100 );
  coneMesh->setSlices( 20 );
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
  updateAxisViewportSize();
}

void Qgs3DAxis::updateAxisViewportSize( int )
{
  float widthRatio = ( float )mAxisViewportSize / mParentWindow->width();
  float heightRatio = ( float )mAxisViewportSize / mParentWindow->height();

  float xRatio;
  float yRatio;
  if ( mAxisViewportHorizPos == AxisViewportPosition::BEGIN )
    xRatio = 0.0f;
  else if ( mAxisViewportHorizPos == AxisViewportPosition::MIDDLE )
    xRatio = 0.5 - widthRatio / 2.0;
  else
    xRatio = 1.0 - widthRatio;

  if ( mAxisViewportVertPos == AxisViewportPosition::BEGIN )
    yRatio = 0.0f;
  else if ( mAxisViewportVertPos == AxisViewportPosition::MIDDLE )
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
    -10.0f, 100.0f );

  updateLabelPosition();
}

void Qgs3DAxis::updateCamera( /* const QVector3D & viewVector*/ )
{
  if ( mParentCamera->viewVector() != mPreviousVector
       && !std::isnan( mParentCamera->viewVector().x() )
       && !std::isnan( mParentCamera->viewVector().y() )
       && !std::isnan( mParentCamera->viewVector().z() ) )
  {
    mPreviousVector = mParentCamera->viewVector();
    QVector3D mainCameraShift = mParentCamera->viewVector().normalized();
    float zy_swap = mainCameraShift.y();
    mainCameraShift.setY( mainCameraShift.z() );
    mainCameraShift.setZ( -zy_swap );
    mainCameraShift.setX( -mainCameraShift.x() );

    if ( mAxisCamera->projectionType() == Qt3DRender::QCameraLens::ProjectionType::OrthographicProjection )
    {
      mAxisCamera->setPosition( mainCameraShift );
    }
    else
      mAxisCamera->setPosition( mainCameraShift * mCylinderLength * 5.0 );

    updateLabelPosition();
#ifdef DEBUG
    qDebug() << std::time( nullptr ) << this << "update camera from" << mPreviousVector << "to" << mainCameraShift << " / r:" << r;
    mText_X->dumpObjectTree();
    mText_X->setFont( QFont( "monospace", 10 ) );
    mText_X->setText( "TEST" );
#endif
  }
}

void Qgs3DAxis::updateLabelPosition()
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
  }
}
