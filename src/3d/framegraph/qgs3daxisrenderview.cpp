/***************************************************************************
  qgs3daxisrenderview.cpp
  --------------------------------------
  Date                 : June 2024
  Copyright            : (C) 2024 by Benoit De Mezzo
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3daxisrenderview.h"

#include "qgsframegraph.h"
#include <Qt3DCore/QEntity>
#include <Qt3DExtras/QText2DEntity>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QViewport>
#include <Qt3DRender/QPickEvent>
#include <Qt3DRender/QScreenRayCaster>
#include <Qt3DRender/qsubtreeenabler.h>
#include <Qt3DRender/QSortPolicy>

#include <QVector3D>
#include <QVector2D>
#include <QScreen>

#include <Qt3DRender/QLayer>
#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
#include <Qt3DRender/QBuffer>
typedef Qt3DRender::QBuffer Qt3DQBuffer;
#else
#include <Qt3DCore/QBuffer>
typedef Qt3DCore::QBuffer Qt3DQBuffer;
#endif
#include <Qt3DRender/QGeometryRenderer>

#include "qgsmapsettings.h"
#include "qgs3dmapsettings.h"
#include "qgs3dmapcanvas.h"
#include "qgscameracontroller.h"
#include "qgs3daxis.h"


Qgs3DAxisRenderView::Qgs3DAxisRenderView( const QString &viewName, Qgs3DMapCanvas *canvas,             //
                                          QgsCameraController *cameraCtrl, Qgs3DMapSettings *settings, //
                                          Qgs3DAxis *axis3D )
  : QgsAbstractRenderView( viewName )
  , mCanvas( canvas )
  , mMapSettings( settings )
  , m3DAxis( axis3D )
{
  mViewport = new Qt3DRender::QViewport( mRendererEnabler );
  mViewport->setObjectName( mViewName + "::Viewport" );

  mObjectLayer = new Qt3DRender::QLayer;
  mObjectLayer->setObjectName( mViewName + "::ObjectLayer" );
  mObjectLayer->setRecursive( true );

  mLabelLayer = new Qt3DRender::QLayer;
  mLabelLayer->setObjectName( mViewName + "::LabelLayer" );
  mLabelLayer->setRecursive( true );

  // render pass for the object (axis or cube)
  Qt3DRender::QLayerFilter *objectFilter = new Qt3DRender::QLayerFilter( mViewport );
  objectFilter->addLayer( mObjectLayer );

  mObjectCamera = new Qt3DRender::QCamera;
  mObjectCamera->setObjectName( mViewName + "::ObjectCamera" );
  mObjectCamera->setProjectionType( cameraCtrl->camera()->projectionType() );
  mObjectCamera->lens()->setFieldOfView( cameraCtrl->camera()->lens()->fieldOfView() * 0.5f );

  Qt3DRender::QCameraSelector *cameraSelector = new Qt3DRender::QCameraSelector( objectFilter );
  cameraSelector->setCamera( mObjectCamera );

  // This ensures to have the label (Text2DEntity) rendered after the other objects and therefore
  // avoid any transparency issue on the label.
  Qt3DRender::QSortPolicy *objectSortPolicy = new Qt3DRender::QSortPolicy( cameraSelector );
  QVector<Qt3DRender::QSortPolicy::SortType> objectSortTypes = QVector<Qt3DRender::QSortPolicy::SortType>();
  objectSortTypes << Qt3DRender::QSortPolicy::BackToFront;
  objectSortPolicy->setSortTypes( objectSortTypes );

  Qt3DRender::QClearBuffers *objectClearBuffers = new Qt3DRender::QClearBuffers( objectSortPolicy );
  objectClearBuffers->setBuffers( Qt3DRender::QClearBuffers::DepthBuffer );

  Qt3DRender::QLayerFilter *labelFilter = new Qt3DRender::QLayerFilter( mViewport );
  labelFilter->addLayer( mLabelLayer );

  mLabelCamera = new Qt3DRender::QCamera;
  mLabelCamera->setObjectName( mViewName + "::LabelCamera" );
  mLabelCamera->setProjectionType( Qt3DRender::QCameraLens::ProjectionType::OrthographicProjection );

  Qt3DRender::QCameraSelector *labelCameraSelector = new Qt3DRender::QCameraSelector( labelFilter );
  labelCameraSelector->setCamera( mLabelCamera );

  // this ensures to have the label (Text2DEntity) rendered after the other objects and therefore
  // avoid any transparency issue on the label.
  Qt3DRender::QSortPolicy *labelSortPolicy = new Qt3DRender::QSortPolicy( labelCameraSelector );
  QVector<Qt3DRender::QSortPolicy::SortType> labelSortTypes = QVector<Qt3DRender::QSortPolicy::SortType>();
  labelSortTypes << Qt3DRender::QSortPolicy::BackToFront;
  labelSortPolicy->setSortTypes( labelSortTypes );

  Qt3DRender::QClearBuffers *clearBuffers = new Qt3DRender::QClearBuffers( labelSortPolicy );
  clearBuffers->setBuffers( Qt3DRender::QClearBuffers::DepthBuffer );

  // update viewport size
  onViewportSizeUpdate();
}

Qt3DRender::QViewport *Qgs3DAxisRenderView::viewport() const
{
  return mViewport;
}

Qt3DRender::QLayer *Qgs3DAxisRenderView::objectLayer() const
{
  return mObjectLayer;
}

Qt3DRender::QLayer *Qgs3DAxisRenderView::labelLayer() const
{
  return mLabelLayer;
}

Qt3DRender::QCamera *Qgs3DAxisRenderView::objectCamera() const
{
  return mObjectCamera;
}

Qt3DRender::QCamera *Qgs3DAxisRenderView::labelCamera() const
{
  return mLabelCamera;
}

void Qgs3DAxisRenderView::updateWindowResize( int width, int height )
{
  onViewportSizeUpdate( width, height );
}


void Qgs3DAxisRenderView::onViewportSizeUpdate( int width, int height )
{
  Qgs3DAxisSettings settings = mMapSettings->get3DAxisSettings();
  double windowWidth = static_cast<double>( width < 0 ? mCanvas->width() : width );
  double windowHeight = static_cast<double>( height < 0 ? mCanvas->height() : height );

  if ( 2 <= QgsLogger::debugLevel() )
  {
    QgsMapSettings set;
    QgsDebugMsgLevel( QString( "onViewportSizeUpdate window w/h: %1px / %2px" ).arg( windowWidth ).arg( windowHeight ), 2 );
    QgsDebugMsgLevel( QString( "onViewportSizeUpdate window physicalDpi %1 (%2, %3)" ).arg( mCanvas->screen()->physicalDotsPerInch() ).arg( mCanvas->screen()->physicalDotsPerInchX() ).arg( mCanvas->screen()->physicalDotsPerInchY() ), 2 );
    QgsDebugMsgLevel( QString( "onViewportSizeUpdate window logicalDotsPerInch %1 (%2, %3)" ).arg( mCanvas->screen()->logicalDotsPerInch() ).arg( mCanvas->screen()->logicalDotsPerInchX() ).arg( mCanvas->screen()->logicalDotsPerInchY() ), 2 );

    QgsDebugMsgLevel( QString( "onViewportSizeUpdate window pixel ratio %1" ).arg( mCanvas->screen()->devicePixelRatio() ), 2 );

    QgsDebugMsgLevel( QString( "onViewportSizeUpdate set pixel ratio %1" ).arg( set.devicePixelRatio() ), 2 );
    QgsDebugMsgLevel( QString( "onViewportSizeUpdate set outputDpi %1" ).arg( set.outputDpi() ), 2 );
    QgsDebugMsgLevel( QString( "onViewportSizeUpdate set dpiTarget %1" ).arg( set.dpiTarget() ), 2 );
  }

  // default viewport size in pixel according to 92 dpi
  double defaultViewportPixelSize = ( ( double ) settings.defaultViewportSize() / 25.4 ) * 92.0;

  // computes the viewport size according to screen dpi but as the viewport size growths too fast
  // then we limit the growth by using a factor on the dpi difference.
  double viewportPixelSize = defaultViewportPixelSize + ( ( double ) settings.defaultViewportSize() / 25.4 ) * ( mCanvas->screen()->physicalDotsPerInch() - 92.0 ) * 0.7;
  QgsDebugMsgLevel( QString( "onViewportSizeUpdate viewportPixelSize %1" ).arg( viewportPixelSize ), 2 );
  double widthRatio = viewportPixelSize / windowWidth;
  double heightRatio = widthRatio * windowWidth / windowHeight;

  QgsDebugMsgLevel( QString( "3DAxis viewport ratios width: %1% / height: %2%" ).arg( widthRatio * 100.0 ).arg( heightRatio * 100.0 ), 2 );

  if ( heightRatio * windowHeight < viewportPixelSize )
  {
    heightRatio = viewportPixelSize / windowHeight;
    widthRatio = heightRatio * windowHeight / windowWidth;
    QgsDebugMsgLevel( QString( "3DAxis viewport, height too small, ratios adjusted to width: %1% / height: %2%" ).arg( widthRatio * 100.0 ).arg( heightRatio * 100.0 ), 2 );
  }

  if ( heightRatio > settings.maxViewportRatio() || widthRatio > settings.maxViewportRatio() )
  {
    QgsDebugMsgLevel( QString( "3DAxis viewport takes too much place into the 3d view, disabling it (maxViewportRatio: %1)." ).arg( settings.maxViewportRatio() ), 2 );
    // take too much place into the 3d view
    mViewport->setEnabled( false );
    m3DAxis->onViewportScaleFactorChanged( 0.0 );
  }
  else
  {
    if ( !mViewport->isEnabled() )
    {
      // will be used to adjust the axis label translations/sizes
      m3DAxis->onViewportScaleFactorChanged( viewportPixelSize / defaultViewportPixelSize );
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
      mLabelCamera->lens()->setOrthographicProjection(
        -halfWidthSize, halfWidthSize,
        -halfHeightSize, halfHeightSize,
        mLabelCamera->lens()->nearPlane(), mLabelCamera->lens()->farPlane()
      );
    }
  }
}

void Qgs3DAxisRenderView::onHorizontalPositionChanged( Qt::AnchorPoint pos )
{
  Qgs3DAxisSettings axisSettings = mMapSettings->get3DAxisSettings();
  axisSettings.setHorizontalPosition( pos );
  mMapSettings->set3DAxisSettings( axisSettings );
  onViewportSizeUpdate();
}

void Qgs3DAxisRenderView::onVerticalPositionChanged( Qt::AnchorPoint pos )
{
  Qgs3DAxisSettings axisSettings = mMapSettings->get3DAxisSettings();
  axisSettings.setVerticalPosition( pos );
  mMapSettings->set3DAxisSettings( axisSettings );
  onViewportSizeUpdate();
}
