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
#include "qgsrubberband.h"
#include "qgssettingsregistrycore.h"
#include "qgsplotpantool.h"
#include "qgsplottoolzoom.h"
#include "qgselevationprofilepdfexportdialog.h"
#include "qgsfileutils.h"
#include "qgsmessagebar.h"
#include "qgsplot.h"

#include <QToolBar>
#include <QProgressBar>
#include <QTimer>
#include <QPrinter>

QgsElevationProfileWidget::QgsElevationProfileWidget( const QString &name )
  : QWidget( nullptr )
  , mCanvasName( name )
{
  setObjectName( QStringLiteral( "ElevationProfile" ) );

  setAttribute( Qt::WA_DeleteOnClose );
  const QgsSettings setting;

  QToolBar *toolBar = new QToolBar( this );
  toolBar->setIconSize( QgisApp::instance()->iconSize( true ) );

  mCanvas = new QgsElevationProfileCanvas( this );
  mCanvas->setProject( QgsProject::instance() );
  connect( mCanvas, &QgsElevationProfileCanvas::activeJobCountChanged, this, &QgsElevationProfileWidget::onTotalPendingJobsCountChanged );

  mPanTool = new QgsPlotToolPan( mCanvas );
  mCanvas->setTool( mPanTool );

  mZoomTool = new QgsPlotToolZoom( mCanvas );

  mCaptureCurveAction = new QAction( tr( "Capture Curve" ), this );
  mCaptureCurveAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionCaptureLine.svg" ) ) );
  mCaptureCurveAction->setCheckable( true );
  connect( mCaptureCurveAction, &QAction::triggered, this, [ = ]
  {
    if ( mCaptureCurveMapTool && mMainCanvas )
    {
      mMainCanvas->setMapTool( mCaptureCurveMapTool.get() );
    }
  } );
  toolBar->addAction( mCaptureCurveAction );

  QAction *clearAction = new QAction( tr( "Clear" ), this );
  clearAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "console/iconClearConsole.svg" ) ) );
  connect( clearAction, &QAction::triggered, this, &QgsElevationProfileWidget::clear );
  toolBar->addAction( clearAction );

  toolBar->addSeparator();

  QAction *panToolAction = new QAction( tr( "Pan" ), this );
  panToolAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionPan.svg" ) ) );
  panToolAction->setCheckable( true );
  panToolAction->setChecked( true );
  mPanTool->setAction( panToolAction );
  connect( panToolAction, &QAction::triggered, mPanTool, [ = ] { mCanvas->setTool( mPanTool ); } );
  toolBar->addAction( panToolAction );

  QAction *zoomToolAction = new QAction( tr( "Zoom" ), this );
  zoomToolAction->setCheckable( true );
  zoomToolAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomIn.svg" ) ) );
  mZoomTool->setAction( zoomToolAction );
  connect( zoomToolAction, &QAction::triggered, mZoomTool, [ = ] { mCanvas->setTool( mZoomTool ); } );
  toolBar->addAction( zoomToolAction );

  QAction *resetViewAction = new QAction( tr( "Zoom Full" ), this );
  resetViewAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomFullExtent.svg" ) ) );
  connect( resetViewAction, &QAction::triggered, mCanvas, &QgsElevationProfileCanvas::zoomFull );
  toolBar->addAction( resetViewAction );

  toolBar->addSeparator();

  QAction *exportAsPdfAction = new QAction( tr( "Export as PDF" ), this );
  exportAsPdfAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSaveAsPDF.svg" ) ) );
  connect( exportAsPdfAction, &QAction::triggered, this, &QgsElevationProfileWidget::exportAsPdf );
  toolBar->addAction( exportAsPdfAction );



#if 0
  // Options Menu
  mOptionsMenu = new QMenu( this );

  mBtnOptions = new QToolButton();
  mBtnOptions->setAutoRaise( true );
  mBtnOptions->setToolTip( tr( "Options" ) );
  mBtnOptions->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionOptions.svg" ) ) );
  mBtnOptions->setPopupMode( QToolButton::InstantPopup );
  mBtnOptions->setMenu( mOptionsMenu );

  toolBar->addWidget( mBtnOptions );


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

  mProgressPendingJobs = new QProgressBar( this );
  mProgressPendingJobs->setRange( 0, 0 );
  mProgressPendingJobs->hide();

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

  mDockableWidgetHelper = new QgsDockableWidgetHelper( true, mCanvasName, this, QgisApp::instance(), Qt::BottomDockWidgetArea,  QStringList(), true );
  QToolButton *toggleButton = mDockableWidgetHelper->createDockUndockToolButton();
  toggleButton->setToolTip( tr( "Dock 3D Map View" ) );
  toolBar->addWidget( toggleButton );
  connect( mDockableWidgetHelper, &QgsDockableWidgetHelper::closed, [ = ]()
  {
    close();
  } );

  // updating the profile plot is deferred on a timer, so that we don't trigger it too often
  mSetCurveTimer = new QTimer( this );
  mSetCurveTimer->setSingleShot( true );
  mSetCurveTimer->stop();
  connect( mSetCurveTimer, &QTimer::timeout, this, &QgsElevationProfileWidget::updatePlot );
}

QgsElevationProfileWidget::~QgsElevationProfileWidget()
{
  if ( mRubberBand )
    mRubberBand.reset();

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
  connect( mCaptureCurveMapTool.get(), &QgsMapToolProfileCurve::captureStarted, this, [ = ]
  {
    // if capturing a new curve, we just hide the existing rubber band -- if the user cancels the new curve digitizing then we'll
    // re-show the old curve rubber band
    if ( mRubberBand )
      mRubberBand->hide();
  } );
  connect( mCaptureCurveMapTool.get(), &QgsMapToolProfileCurve::captureCanceled, this, [ = ]
  {
    if ( mRubberBand )
      mRubberBand->show();
  } );
  connect( mCaptureCurveMapTool.get(), &QgsMapToolProfileCurve::curveCaptured, this, &QgsElevationProfileWidget::setProfileCurve );

  // only do this from map tool!
  connect( mMainCanvas, &QgsMapCanvas::layersChanged, this, &QgsElevationProfileWidget::onMainCanvasLayersChanged );
  onMainCanvasLayersChanged();
}

void QgsElevationProfileWidget::cancelJobs()
{
  mCanvas->cancelJobs();
}

void QgsElevationProfileWidget::onMainCanvasLayersChanged()
{
  // possibly not right -- do we always want to sync the profile layers to canvas layers?
  mCanvas->setLayers( mMainCanvas->layers( true ) );
  scheduleUpdate();
}

void QgsElevationProfileWidget::onTotalPendingJobsCountChanged( int count )
{
  mProgressPendingJobs->setVisible( count );
}

void QgsElevationProfileWidget::setProfileCurve( const QgsGeometry &curve )
{
  mProfileCurve = curve;
  mRubberBand.reset( createRubberBand() );
  mRubberBand->setToGeometry( mProfileCurve );
  scheduleUpdate();
}

void QgsElevationProfileWidget::updatePlot()
{
  if ( const QgsCurve *curve = qgsgeometry_cast< const QgsCurve *>( mProfileCurve.constGet() ) )
  {
    mCanvas->setCrs( mMainCanvas->mapSettings().destinationCrs() );
    mCanvas->setProfileCurve( curve->clone() );
    mCanvas->refresh();
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

void QgsElevationProfileWidget::clear()
{
  mRubberBand.reset();
  mCanvas->clear();
}

void QgsElevationProfileWidget::exportAsPdf()
{
  QgsSettings s;
  QString outputFileName = QgsFileUtils::findClosestExistingPath( s.value( QStringLiteral( "lastProfileExportDir" ), QDir::homePath(), QgsSettings::App ).toString() );

#ifdef Q_OS_MAC
  QgisApp::instance()->activateWindow();
  this->raise();
#endif
  outputFileName = QFileDialog::getSaveFileName(
                     this,
                     tr( "Export to PDF" ),
                     outputFileName,
                     tr( "PDF Format" ) + " (*.pdf *.PDF)" );
  this->activateWindow();
  if ( outputFileName.isEmpty() )
  {
    return;
  }
  outputFileName = QgsFileUtils::ensureFileNameHasExtension( outputFileName, { QStringLiteral( "pdf" ) } );
  QgsSettings().setValue( QStringLiteral( "lastProfileExportDir" ), outputFileName, QgsSettings::App );

  QgsElevationProfilePdfExportDialog dialog( this );
  dialog.setPlotSettings( mCanvas->plot() );

  if ( !dialog.exec() )
    return;

  QPrinter printer;
  printer.setOutputFileName( outputFileName );
  printer.setOutputFormat( QPrinter::PdfFormat );

  const QgsLayoutSize pageSizeMM = dialog.pageSizeMM();
  QPageLayout pageLayout( QPageSize( pageSizeMM.toQSizeF(), QPageSize::Millimeter ),
                          QPageLayout::Portrait,
                          QMarginsF( 0, 0, 0, 0 ) );
  pageLayout.setMode( QPageLayout::FullPageMode );
  printer.setPageLayout( pageLayout );
  printer.setFullPage( true );
  printer.setPageMargins( QMarginsF( 0, 0, 0, 0 ) );
  printer.setFullPage( true );
  printer.setColorMode( QPrinter::Color );
  printer.setResolution( 300 );

  QPainter p;
  if ( !p.begin( &printer ) )
  {
    //error beginning print
    QgisApp::instance()->messageBar()->pushWarning( tr( "Save as PDF" ), tr( "Could not create %1" ).arg( QDir::toNativeSeparators( outputFileName ) ) );
    return;
  }

  QgsRenderContext rc = QgsRenderContext::fromQPainter( &p );
  rc.setFlag( Qgis::RenderContextFlag::Antialiasing, true );
  rc.setFlag( Qgis::RenderContextFlag::ForceVectorOutput, true );
  rc.setFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering, true );
  rc.setFlag( Qgis::RenderContextFlag::HighQualityImageTransforms, true );
  rc.setTextRenderFormat( Qgis::TextRenderFormat::AlwaysText );
  rc.setPainterFlagsUsingContext( &p );

  Qgs2DPlot plotSettings;
  dialog.updatePlotSettings( plotSettings );

  mCanvas->render( rc, rc.convertToPainterUnits( pageSizeMM.width(), QgsUnitTypes::RenderMillimeters ),
                   rc.convertToPainterUnits( pageSizeMM.height(), QgsUnitTypes::RenderMillimeters ), plotSettings );
  p.end();

  QgisApp::instance()->messageBar()->pushSuccess( tr( "Save as PDF" ), tr( "Successfully saved the profile to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( outputFileName ).toString(), QDir::toNativeSeparators( outputFileName ) ) );
}

QgsRubberBand *QgsElevationProfileWidget::createRubberBand( )
{
  QgsRubberBand *rb = new QgsRubberBand( mMainCanvas, QgsWkbTypes::LineGeometry );
  rb->setWidth( QgsSettingsRegistryCore::settingsDigitizingLineWidth.value() );
  QColor color = QColor( QgsSettingsRegistryCore::settingsDigitizingLineColorRed.value(),
                         QgsSettingsRegistryCore::settingsDigitizingLineColorGreen.value(),
                         QgsSettingsRegistryCore::settingsDigitizingLineColorBlue.value(),
                         QgsSettingsRegistryCore::settingsDigitizingLineColorAlpha.value() );
  rb->setStrokeColor( color );
  rb->show();
  return rb;
}

