/***************************************************************************
  qgsshadowrenderview.cpp
  --------------------------------------
  Date                 : June 2024
  Copyright            : (C) 2024 by Benoit De Mezzo and (C) 2020 by Belgacem Nedjima
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsshadowrenderview.h"
#include "qgsdirectionallightsettings.h"
#include "qgsshadowsettings.h"

#include <Qt3DRender/QCamera>
#include <Qt3DRender/QRenderSurfaceSelector>
#include <Qt3DRender/QViewport>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QRenderTargetSelector>
#include <Qt3DRender/QRenderTarget>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QFrustumCulling>
#include <Qt3DRender/QRenderStateSet>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QPolygonOffset>
#include <Qt3DRender/qsubtreeenabler.h>

QgsShadowRenderView::QgsShadowRenderView( QObject *parent, const QString &viewName )
  : QgsAbstractRenderView( parent, viewName )
{
  mLightCamera = new Qt3DRender::QCamera;
  mLightCamera->setObjectName( objectName() + "::LightCamera" );
  mLayer = new Qt3DRender::QLayer;
  mLayer->setRecursive( true );
  mLayer->setObjectName( objectName() + "::Layer" );

  // shadow rendering pass
  buildRenderPass();
}

void QgsShadowRenderView::setEnabled( bool enable )
{
  QgsAbstractRenderView::setEnabled( enable );
  emit shadowRenderingEnabled( enable );
  mLayerFilter->setEnabled( enable );
}

void QgsShadowRenderView::setupDirectionalLight( const QgsDirectionalLightSettings &light, double maximumShadowRenderingDistance,
    const Qt3DRender::QCamera *mainCamera )
{
  float minX, maxX, minY, maxY, minZ, maxZ;
  QVector3D lookingAt = mainCamera->viewCenter();
  const float d = 2.0f * ( mainCamera->position() - mainCamera->viewCenter() ).length();

  const QVector3D vertical = QVector3D( 0.0f, d, 0.0f );
  const QVector3D lightDirection = QVector3D( light.direction().x(), light.direction().y(), light.direction().z() ).normalized();
  QgsShadowRenderView::calculateViewExtent( mainCamera, static_cast<float>( maximumShadowRenderingDistance ), lookingAt.y(),
      minX, maxX, minY, maxY, minZ, maxZ );

  lookingAt = QVector3D( 0.5f * ( minX + maxX ), mainCamera->viewCenter().y(), 0.5f * ( minZ + maxZ ) );
  const QVector3D lightPosition = lookingAt + vertical;
  mLightCamera->setPosition( lightPosition );
  mLightCamera->setViewCenter( lookingAt );
  mLightCamera->setUpVector( QVector3D( 0.0f, 1.0f, 0.0f ) );
  mLightCamera->rotateAboutViewCenter( QQuaternion::rotationTo( vertical.normalized(), -lightDirection.normalized() ) );

  mLightCamera->setProjectionType( Qt3DRender::QCameraLens::ProjectionType::OrthographicProjection );
  mLightCamera->lens()->setOrthographicProjection(
    - 0.7f * ( maxX - minX ), 0.7f * ( maxX - minX ),
    - 0.7f * ( maxZ - minZ ), 0.7f * ( maxZ - minZ ),
    1.0f, 2.0f * ( lookingAt - lightPosition ).length() );

  emit shadowExtentChanged( minX, maxX, minY, maxY, minZ, maxZ );
  emit shadowDirectionLightUpdated( lightPosition, lightDirection );
}

Qt3DRender::QFrameGraphNode *QgsShadowRenderView::buildRenderPass()
{
  mLightCameraSelector = new Qt3DRender::QCameraSelector( mRendererEnabler );
  mLightCameraSelector->setObjectName( objectName() + "::CameraSelector" );
  mLightCameraSelector->setCamera( mLightCamera );

  mLayerFilter = new Qt3DRender::QLayerFilter( mLightCameraSelector );
  mLayerFilter->addLayer( mLayer );

  mRenderTargetSelector = new Qt3DRender::QRenderTargetSelector( mLayerFilter );
  // no target output for now, updateTargetOutput() will be called later

  mClearBuffers = new Qt3DRender::QClearBuffers( mRenderTargetSelector );
  mClearBuffers->setBuffers( Qt3DRender::QClearBuffers::BufferType::ColorDepthBuffer );
  mClearBuffers->setClearColor( QColor::fromRgbF( 0.0f, 1.0f, 0.0f ) );

  mRenderStateSet = new Qt3DRender::QRenderStateSet( mClearBuffers );

  Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest;
  depthTest->setDepthFunction( Qt3DRender::QDepthTest::Less );
  mRenderStateSet->addRenderState( depthTest );

  Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace;
  cullFace->setMode( Qt3DRender::QCullFace::CullingMode::Front );
  mRenderStateSet->addRenderState( cullFace );

  Qt3DRender::QPolygonOffset *polygonOffset = new Qt3DRender::QPolygonOffset;
  polygonOffset->setDepthSteps( 4.0 );
  polygonOffset->setScaleFactor( 1.1 );
  mRenderStateSet->addRenderState( polygonOffset );

  return mLightCameraSelector;
}

void QgsShadowRenderView::updateSettings(
  const QgsShadowSettings &shadowSettings,
  const QList<QgsLightSource *> &lightSources,
  Qt3DRender::QCamera *mainCamera )
{
  int selectedLight = shadowSettings.selectedDirectionalLight();
  QgsDirectionalLightSettings *light = nullptr;
  for ( int i = 0, dirLight = 0; !light && i < lightSources.size(); i++ )
  {
    if ( lightSources[i]->type() == Qgis::LightSourceType::Directional )
    {
      if ( dirLight == selectedLight )
        light = qgis::down_cast< QgsDirectionalLightSettings * >( lightSources[i] );
      dirLight++;
    }
  }

  if ( shadowSettings.renderShadows() && light )
  {
    setShadowBias( static_cast<float>( shadowSettings.shadowBias() ) );
    updateTargetOutputSize( shadowSettings.shadowMapResolution(), shadowSettings.shadowMapResolution() );
    setupDirectionalLight( *light, shadowSettings.maximumShadowRenderingDistance(), mainCamera );
    setEnabled( true );
  }
  else
    setEnabled( false );
}

void QgsShadowRenderView::setShadowBias( float bias )
{
  mBias = bias;
  emit shadowBiasChanged( mBias );
}

// computes the portion of the Y=y plane the camera is looking at
void QgsShadowRenderView::calculateViewExtent( const Qt3DRender::QCamera *camera, float shadowRenderingDistance, float y, float &minX, float &maxX, float &minY, float &maxY, float &minZ, float &maxZ )
{
  const QVector3D cameraPos = camera->position();
  const QMatrix4x4 projectionMatrix = camera->projectionMatrix();
  const QMatrix4x4 viewMatrix = camera->viewMatrix();
  float depth = 1.0f;
  QVector4D viewCenter =  viewMatrix * QVector4D( camera->viewCenter(), 1.0f );
  viewCenter /= viewCenter.w();
  viewCenter = projectionMatrix * viewCenter;
  viewCenter /= viewCenter.w();
  depth = viewCenter.z();
  QVector<QVector3D> viewFrustumPoints =
  {
    QVector3D( 0.0f,  0.0f, depth ),
    QVector3D( 0.0f,  1.0f, depth ),
    QVector3D( 1.0f,  0.0f, depth ),
    QVector3D( 1.0f,  1.0f, depth ),
    QVector3D( 0.0f,  0.0f, 0 ),
    QVector3D( 0.0f,  1.0f, 0 ),
    QVector3D( 1.0f,  0.0f, 0 ),
    QVector3D( 1.0f,  1.0f, 0 )
  };
  maxX = std::numeric_limits<float>::lowest();
  maxY = std::numeric_limits<float>::lowest();
  maxZ = std::numeric_limits<float>::lowest();
  minX = std::numeric_limits<float>::max();
  minY = std::numeric_limits<float>::max();
  minZ = std::numeric_limits<float>::max();
  for ( int i = 0; i < viewFrustumPoints.size(); ++i )
  {
    // convert from view port space to world space
    viewFrustumPoints[i] = viewFrustumPoints[i].unproject( viewMatrix, projectionMatrix, QRect( 0, 0, 1, 1 ) );
    minX = std::min( minX, viewFrustumPoints[i].x() );
    maxX = std::max( maxX, viewFrustumPoints[i].x() );
    minY = std::min( minY, viewFrustumPoints[i].y() );
    maxY = std::max( maxY, viewFrustumPoints[i].y() );
    minZ = std::min( minZ, viewFrustumPoints[i].z() );
    maxZ = std::max( maxZ, viewFrustumPoints[i].z() );
    // find the intersection between the line going from cameraPos to the frustum quad point
    // and the horizontal plane Y=y
    // if the intersection is on the back side of the viewing panel we get a point that is
    // shadowRenderingDistance units in front of the camera
    const QVector3D pt = cameraPos;
    const QVector3D vect = ( viewFrustumPoints[i] - pt ).normalized();
    float t = ( y - pt.y() ) / vect.y();
    if ( t < 0 )
      t = shadowRenderingDistance;
    else
      t = std::min( t, shadowRenderingDistance );
    viewFrustumPoints[i] = pt + t * vect;
    minX = std::min( minX, viewFrustumPoints[i].x() );
    maxX = std::max( maxX, viewFrustumPoints[i].x() );
    minY = std::min( minY, viewFrustumPoints[i].y() );
    maxY = std::max( maxY, viewFrustumPoints[i].y() );
    minZ = std::min( minZ, viewFrustumPoints[i].z() );
    maxZ = std::max( maxZ, viewFrustumPoints[i].z() );
  }
}
