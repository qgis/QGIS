/***************************************************************************
                          qgselevationprofilewidget.cpp
                          ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgselevationprofilewidget.h"
#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgselevationprofilecanvas.h"
#include "qgsdockablewidgethelper.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolprofilecurve.h"

#include <QToolBar>
#include <QProgressBar>
#include <QTimer>

QgsElevationProfileWidget::QgsElevationProfileWidget( const QString &name, bool isDocked )
  : QWidget( nullptr )
  , mCanvasName( name )
{
  setObjectName( QStringLiteral( "ElevationProfile" ) );

  setAttribute( Qt::WA_DeleteOnClose );
  const QgsSettings setting;

  QToolBar *toolBar = new QToolBar( this );
  toolBar->setIconSize( QgisApp::instance()->iconSize( true ) );

  mCaptureCurveAction = new QAction( tr( "Capture Curve" ), this );
  mCaptureCurveAction->setCheckable( true );
  connect( mCaptureCurveAction, &QAction::triggered, this, [ = ]
  {
    if ( mCaptureCurveMapTool && mMainCanvas )
    {
      mMainCanvas->setMapTool( mCaptureCurveMapTool.get() );
    }
  } );
  toolBar->addAction( mCaptureCurveAction );

  // Options Menu
  mOptionsMenu = new QMenu( this );

  mBtnOptions = new QToolButton();
  mBtnOptions->setAutoRaise( true );
  mBtnOptions->setToolTip( tr( "Options" ) );
  mBtnOptions->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionOptions.svg" ) ) );
  mBtnOptions->setPopupMode( QToolButton::InstantPopup );
  mBtnOptions->setMenu( mOptionsMenu );

  toolBar->addWidget( mBtnOptions );

#if 0
  mActionEnableShadows = new QAction( tr( "Show Shadows" ), this );
  mActionEnableShadows->setCheckable( true );
  connect( mActionEnableShadows, &QAction::toggled, this, [ = ]( bool enabled )
  {
    QgsShadowSettings settings = mCanvas->map()->shadowSettings();
    settings.setRenderShadows( enabled );
    mCanvas->map()->setShadowSettings( settings );
  } );
  mOptionsMenu->addAction( mActionEnableShadows );


  mOptionsMenu->addSeparator();

  QAction *configureAction = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "mActionOptions.svg" ) ),
                                          tr( "Configure…" ), this );
  connect( configureAction, &QAction::triggered, this, &QgsElevationProfileWidget::configure );
  mOptionsMenu->addAction( configureAction );
#endif

  mCanvas = new QgsElevationProfileCanvas( this );
  mCanvas->setProject( QgsProject::instance() );

#if 0
  connect( mCanvas, &Qgs3DMapCanvas::savedAsImage, this, [ = ]( const QString fileName )
  {
    QgisApp::instance()->messageBar()->pushSuccess( tr( "Save as Image" ), tr( "Successfully saved the 3D map to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( fileName ).toString(), QDir::toNativeSeparators( fileName ) ) );
  } );
#endif

  mProgressPendingJobs = new QProgressBar( this );
  mProgressPendingJobs->setRange( 0, 0 );

  QHBoxLayout *topLayout = new QHBoxLayout;
  topLayout->setContentsMargins( 0, 0, 0, 0 );
  topLayout->setSpacing( style()->pixelMetric( QStyle::PM_LayoutHorizontalSpacing ) );
  topLayout->addWidget( toolBar );
  topLayout->addStretch( 1 );
  topLayout->addWidget( mProgressPendingJobs );

  QVBoxLayout *layout = new QVBoxLayout;
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 0 );
  layout->addLayout( topLayout );
  layout->addWidget( mCanvas );

  setLayout( layout );

// onTotalPendingJobsCountChanged();

  mDockableWidgetHelper = new QgsDockableWidgetHelper( isDocked, mCanvasName, this, QgisApp::instance() );
  QToolButton *toggleButton = mDockableWidgetHelper->createDockUndockToolButton();
  toggleButton->setToolTip( tr( "Dock 3D Map View" ) );
  toolBar->addWidget( toggleButton );
  connect( mDockableWidgetHelper, &QgsDockableWidgetHelper::closed, [ = ]()
  {
    close();
  } );

  // updating the profile plot is defered on a timer, so that we don't trigger it too often
  mSetCurveTimer = new QTimer( this );
  mSetCurveTimer->setSingleShot( true );
  mSetCurveTimer->stop();
  connect( mSetCurveTimer, &QTimer::timeout, this, &QgsElevationProfileWidget::updatePlot );
}

QgsElevationProfileWidget::~QgsElevationProfileWidget()
{
  delete mDockableWidgetHelper;
}

void QgsElevationProfileWidget::setCanvasName( const QString &name )
{
  mCanvasName = name;
  mDockableWidgetHelper->setWindowTitle( name );
}

void QgsElevationProfileWidget::setMainCanvas( QgsMapCanvas *canvas )
{
  mMainCanvas = canvas;

  mCaptureCurveMapTool = std::make_unique< QgsMapToolProfileCurve >( canvas, QgisApp::instance()->cadDockWidget() );
  mCaptureCurveMapTool->setAction( mCaptureCurveAction );
  connect( mCaptureCurveMapTool.get(), &QgsMapToolProfileCurve::curveCaptured, this, &QgsElevationProfileWidget::setProfileCurve );

  // only do this from map tool!
  // mCanvas->setCrs( mMainCanvas->mapSettings().destinationCrs()
  connect( mMainCanvas, &QgsMapCanvas::layersChanged, this, &QgsElevationProfileWidget::onMainCanvasLayersChanged );
  onMainCanvasLayersChanged();
}

void QgsElevationProfileWidget::onMainCanvasLayersChanged()
{
  // not right -- should be done in response to project layers
  mCanvas->setLayers( mMainCanvas->layers( true ) );
  scheduleUpdate();
}

void QgsElevationProfileWidget::onTotalPendingJobsCountChanged()
{
//  const int count = mCanvas->scene() ? mCanvas->scene()->totalPendingJobsCount() : 0;
  // mProgressPendingJobs->setVisible( count );
}

void QgsElevationProfileWidget::setProfileCurve( const QgsGeometry &curve )
{
  mProfileCurve = curve;
  scheduleUpdate();
}

void QgsElevationProfileWidget::updatePlot()
{
  if ( const QgsCurve *curve = qgsgeometry_cast< const QgsCurve *>( mProfileCurve.constGet() ) )
  {
    mCanvas->setCrs( mMainCanvas->mapSettings().destinationCrs() );
    mCanvas->setProfileCurve( curve->clone() );
    mCanvas->update();
  }
  mUpdateScheduled = false;
}

void QgsElevationProfileWidget::scheduleUpdate()
{
  if ( !mUpdateScheduled )
  {
    mSetCurveTimer->start( 1 );
    mUpdateScheduled = true;
  }
}

