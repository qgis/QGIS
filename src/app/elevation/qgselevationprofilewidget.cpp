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
#include "qgsmaptoolprofilecurvefromfeature.h"
#include "qgsrubberband.h"
#include "qgssettingsregistrycore.h"
#include "qgsplottoolpan.h"
#include "qgsplottoolxaxiszoom.h"
#include "qgsplottoolzoom.h"
#include "qgselevationprofilepdfexportdialog.h"
#include "qgselevationprofileimageexportdialog.h"
#include "qgsfileutils.h"
#include "qgsmessagebar.h"
#include "qgsplot.h"
#include "qgsmulticurve.h"
#include "qgsmaplayerutils.h"
#include "qgslinesymbol.h"
#include "qgslinesymbollayer.h"
#include "qgsfillsymbol.h"
#include "qgsfillsymbollayer.h"

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
  connect( mCanvas, &QgsElevationProfileCanvas::canvasPointHovered, this, &QgsElevationProfileWidget::onCanvasPointHovered );

  mPanTool = new QgsPlotToolPan( mCanvas );
  mCanvas->setTool( mPanTool );

  mZoomTool = new QgsPlotToolZoom( mCanvas );
  mXAxisZoomTool = new QgsPlotToolXAxisZoom( mCanvas );

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

  mCaptureCurveFromFeatureAction = new QAction( tr( "Capture Curve From Feature" ), this );
  mCaptureCurveFromFeatureAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionCaptureCurveFromFeature.svg" ) ) );
  mCaptureCurveFromFeatureAction->setCheckable( true );
  connect( mCaptureCurveFromFeatureAction, &QAction::triggered, this, [ = ]
  {
    if ( mCaptureCurveFromFeatureMapTool && mMainCanvas )
    {
      mMainCanvas->setMapTool( mCaptureCurveFromFeatureMapTool.get() );
    }
  } );
  toolBar->addAction( mCaptureCurveFromFeatureAction );

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

  QAction *zoomXAxisToolAction = new QAction( tr( "Zoom X Axis" ), this );
  zoomXAxisToolAction->setCheckable( true );
  zoomXAxisToolAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomInXAxis.svg" ) ) );
  mXAxisZoomTool->setAction( zoomXAxisToolAction );
  connect( zoomXAxisToolAction, &QAction::triggered, mXAxisZoomTool, [ = ] { mCanvas->setTool( mXAxisZoomTool ); } );
  toolBar->addAction( zoomXAxisToolAction );

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

  QAction *enabledSnappingAction = new QAction( tr( "Enable Snapping" ), this );
  enabledSnappingAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconSnapping.svg" ) ) );
  enabledSnappingAction->setCheckable( true );
  enabledSnappingAction->setChecked( true );
  connect( enabledSnappingAction, &QAction::toggled, mCanvas, &QgsElevationProfileCanvas::setSnappingEnabled );
  toolBar->addAction( enabledSnappingAction );

  toolBar->addSeparator();

  QAction *exportAsPdfAction = new QAction( tr( "Export as PDF" ), this );
  exportAsPdfAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSaveAsPDF.svg" ) ) );
  connect( exportAsPdfAction, &QAction::triggered, this, &QgsElevationProfileWidget::exportAsPdf );
  toolBar->addAction( exportAsPdfAction );

  QAction *exportAsImageAction = new QAction( tr( "Export as Image" ), this );
  exportAsImageAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionSaveMapAsImage.svg" ) ) );
  connect( exportAsImageAction, &QAction::triggered, this, &QgsElevationProfileWidget::exportAsImage );
  toolBar->addAction( exportAsImageAction );

  toolBar->addSeparator();

  // Options Menu
  mOptionsMenu = new QMenu( this );

  mSettingsAction = new QgsElevationProfileWidgetSettingsAction( mOptionsMenu );

  mSettingsAction->toleranceSpinBox()->setValue( settingTolerance.value() );
  connect( mSettingsAction->toleranceSpinBox(), qOverload< double >( &QDoubleSpinBox::valueChanged ), this, [ = ]( double value )
  {
    settingTolerance.setValue( value );
    createOrUpdateRubberBands();
    scheduleUpdate();
  } );

  mOptionsMenu->addAction( mSettingsAction );

  mBtnOptions = new QToolButton();
  mBtnOptions->setAutoRaise( true );
  mBtnOptions->setToolTip( tr( "Options" ) );
  mBtnOptions->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionOptions.svg" ) ) );
  mBtnOptions->setPopupMode( QToolButton::InstantPopup );
  mBtnOptions->setMenu( mOptionsMenu );

  toolBar->addWidget( mBtnOptions );

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
  toggleButton->setToolTip( tr( "Dock Elevation Profile View" ) );
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
  if ( mToleranceRubberBand )
    mToleranceRubberBand.reset();

  if ( mMapPointRubberBand )
    mMapPointRubberBand.reset();

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
    if ( mToleranceRubberBand )
      mToleranceRubberBand->hide();
  } );
  connect( mCaptureCurveMapTool.get(), &QgsMapToolProfileCurve::captureCanceled, this, [ = ]
  {
    if ( mRubberBand )
      mRubberBand->show();
    if ( mToleranceRubberBand )
      mToleranceRubberBand->show();
  } );

  mCaptureCurveFromFeatureMapTool = std::make_unique< QgsMapToolProfileCurveFromFeature >( canvas );
  mCaptureCurveFromFeatureMapTool->setAction( mCaptureCurveFromFeatureAction );
  connect( mCaptureCurveFromFeatureMapTool.get(), &QgsMapToolProfileCurveFromFeature::curveCaptured, this, &QgsElevationProfileWidget::setProfileCurve );

  mMapPointRubberBand.reset( new QgsRubberBand( canvas, QgsWkbTypes::PointGeometry ) );
  mMapPointRubberBand->setZValue( 1000 );
  mMapPointRubberBand->setIcon( QgsRubberBand::ICON_FULL_DIAMOND );
  mMapPointRubberBand->setWidth( QgsGuiUtils::scaleIconSize( 8 ) );
  mMapPointRubberBand->setIconSize( QgsGuiUtils::scaleIconSize( 4 ) );
  mMapPointRubberBand->setSecondaryStrokeColor( QColor( 255, 255, 255, 100 ) );
  mMapPointRubberBand->setColor( QColor( 0, 0, 0 ) );
  mMapPointRubberBand->hide();

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

  QList< QgsMapLayer * > layers = mMainCanvas->layers( true );

  // sort layers so that types which are more likely to obscure others are rendered below
  // e.g. vector features should be drawn above raster DEMS, or the DEM line may completely obscure
  // the vector feature
  layers = QgsMapLayerUtils::sortLayersByType( layers,
  {
    QgsMapLayerType::RasterLayer,
    QgsMapLayerType::MeshLayer,
    QgsMapLayerType::VectorLayer,
    QgsMapLayerType::PointCloudLayer
  } );

  mCanvas->setLayers( layers );
  scheduleUpdate();
}

void QgsElevationProfileWidget::onTotalPendingJobsCountChanged( int count )
{
  mProgressPendingJobs->setVisible( count );
}

void QgsElevationProfileWidget::setProfileCurve( const QgsGeometry &curve )
{
  mProfileCurve = curve;
  createOrUpdateRubberBands();
  scheduleUpdate();
}

void QgsElevationProfileWidget::onCanvasPointHovered( const QgsPointXY &point )
{
  if ( !mMapPointRubberBand )
    return;

  const QgsPointXY mapPoint = mCanvas->toMapCoordinates( point );
  if ( mapPoint.isEmpty() )
  {
    mMapPointRubberBand->hide();
  }
  else
  {
    mMapPointRubberBand->setToGeometry( QgsGeometry::fromPointXY( mapPoint ) );
    mMapPointRubberBand->show();
  }
}

void QgsElevationProfileWidget::updatePlot()
{
  mCanvas->setTolerance( mSettingsAction->toleranceSpinBox()->value() );
  mCanvas->setCrs( mMainCanvas->mapSettings().destinationCrs() );

  if ( !mProfileCurve.isEmpty() )
  {
    if ( const QgsCurve *curve = qgsgeometry_cast< const QgsCurve *>( mProfileCurve.constGet()->simplifiedTypeRef() ) )
    {
      mCanvas->setProfileCurve( curve->clone() );
      mCanvas->refresh();
    }
    else if ( const QgsMultiCurve *multiCurve = qgsgeometry_cast< const QgsMultiCurve *>( mProfileCurve.constGet()->simplifiedTypeRef() ) )
    {
      // hm, just grab the first part!
      mCanvas->setProfileCurve( multiCurve->curveN( 0 )->clone() );
      mCanvas->refresh();
    }
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
  mToleranceRubberBand.reset();
  if ( mMapPointRubberBand )
    mMapPointRubberBand->hide();
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

void QgsElevationProfileWidget::exportAsImage()
{
  QgsSettings s;
  QString outputFileName = QgsFileUtils::findClosestExistingPath( s.value( QStringLiteral( "lastProfileExportDir" ), QDir::homePath(), QgsSettings::App ).toString() );

#ifdef Q_OS_MAC
  QgisApp::instance()->activateWindow();
  this->raise();
#endif
  const QPair<QString, QString> fileWithExtension = QgsGuiUtils::getSaveAsImageName( this, tr( "Save Plot As" ), outputFileName );

  this->activateWindow();
  if ( fileWithExtension.first.isEmpty() )
  {
    return;
  }
  QgsSettings().setValue( QStringLiteral( "lastProfileExportDir" ), fileWithExtension.first, QgsSettings::App );

  QgsElevationProfileImageExportDialog dialog( this );
  dialog.setPlotSettings( mCanvas->plot() );
  dialog.setImageSize( mCanvas->plot().size().toSize() );

  if ( !dialog.exec() )
    return;

  QImage image( dialog.imageSize(), QImage::Format_ARGB32 );
  if ( image.isNull() )
  {
    QgisApp::instance()->messageBar()->pushWarning( tr( "Save as Image" ), tr( "Could not create image" ) );
    return;
  }
  image.fill( Qt::transparent );

  QPainter p( &image );

  QgsRenderContext rc = QgsRenderContext::fromQPainter( &p );
  rc.setFlag( Qgis::RenderContextFlag::Antialiasing, true );
  rc.setFlag( Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering, true );
  rc.setFlag( Qgis::RenderContextFlag::HighQualityImageTransforms, true );
  rc.setPainterFlagsUsingContext( &p );

  Qgs2DPlot plotSettings;
  dialog.updatePlotSettings( plotSettings );

  mCanvas->render( rc, image.width(), image.height(), plotSettings );
  p.end();

  image.save( fileWithExtension.first );

  QgisApp::instance()->messageBar()->pushSuccess( tr( "Save as Image" ), tr( "Successfully saved the profile to <a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( fileWithExtension.first ).toString(), QDir::toNativeSeparators( fileWithExtension.first ) ) );

}

void QgsElevationProfileWidget::createOrUpdateRubberBands( )
{
  if ( !mRubberBand )
  {
    mRubberBand.reset( new QgsRubberBand( mMainCanvas, QgsWkbTypes::LineGeometry ) );
    mRubberBand->setZValue( 1000 );
    mRubberBand->setWidth( QgsGuiUtils::scaleIconSize( 2 ) );

    QgsSymbolLayerList layers;

    std::unique_ptr< QgsSimpleLineSymbolLayer > bottomLayer = std::make_unique< QgsSimpleLineSymbolLayer >();
    bottomLayer->setWidth( 0.8 );
    bottomLayer->setWidthUnit( QgsUnitTypes::RenderMillimeters );
    bottomLayer->setColor( QColor( 40, 40, 40, 100 ) );
    bottomLayer->setPenCapStyle( Qt::PenCapStyle::FlatCap );
    layers.append( bottomLayer.release() );

    std::unique_ptr< QgsSimpleLineSymbolLayer > topLayer = std::make_unique< QgsSimpleLineSymbolLayer >();
    topLayer->setWidth( 0.4 );
    topLayer->setWidthUnit( QgsUnitTypes::RenderMillimeters );
    topLayer->setColor( QColor( 255, 255, 255, 255 ) );
    topLayer->setPenStyle( Qt::DashLine );
    topLayer->setPenCapStyle( Qt::PenCapStyle::FlatCap );
    layers.append( topLayer.release() );

    std::unique_ptr< QgsLineSymbol > symbol = std::make_unique< QgsLineSymbol >( layers );

    mRubberBand->setSymbol( symbol.release() );
    mRubberBand->updatePosition();
    mRubberBand->show();
  }

  mRubberBand->setToGeometry( mProfileCurve );

  const double tolerance = mSettingsAction->toleranceSpinBox()->value();
  if ( !qgsDoubleNear( tolerance, 0, 0.000001 ) )
  {
    if ( !mToleranceRubberBand )
    {
      mToleranceRubberBand.reset( new QgsRubberBand( mMainCanvas, QgsWkbTypes::PolygonGeometry ) );
      mToleranceRubberBand->setZValue( 999 );

      QgsSymbolLayerList layers;

      std::unique_ptr< QgsSimpleFillSymbolLayer > bottomLayer = std::make_unique< QgsSimpleFillSymbolLayer >();
      bottomLayer->setColor( QColor( 40, 40, 40, 50 ) );
      bottomLayer->setStrokeColor( QColor( 255, 255, 255, 150 ) );
      layers.append( bottomLayer.release() );

      std::unique_ptr< QgsFillSymbol > symbol = std::make_unique< QgsFillSymbol >( layers );
      mToleranceRubberBand->setSymbol( symbol.release() );
    }

    const QgsGeometry buffered = mProfileCurve.buffer( tolerance, 8, Qgis::EndCapStyle::Flat, Qgis::JoinStyle::Round, 2 );
    mToleranceRubberBand->setToGeometry( buffered );
    mToleranceRubberBand->show();
  }
  else
  {
    if ( mToleranceRubberBand )
      mToleranceRubberBand->hide();
  }
}

QgsElevationProfileWidgetSettingsAction::QgsElevationProfileWidgetSettingsAction( QWidget *parent )
  : QWidgetAction( parent )
{
  QGridLayout *gLayout = new QGridLayout();
  gLayout->setContentsMargins( 3, 2, 3, 2 );

  mToleranceWidget = new QgsDoubleSpinBox();
  mToleranceWidget->setClearValue( QgsElevationProfileWidget::settingTolerance.defaultValue() );
  mToleranceWidget->setValue( QgsElevationProfileWidget::settingTolerance.defaultValue() );
  mToleranceWidget->setKeyboardTracking( false );
  mToleranceWidget->setMaximumWidth( QFontMetrics( mToleranceWidget->font() ).horizontalAdvance( '0' ) * 50 );
  mToleranceWidget->setDecimals( 2 );
  mToleranceWidget->setRange( 0, 9999999999 );
  mToleranceWidget->setSingleStep( 1.0 );

  QLabel *label = new QLabel( tr( "Tolerance" ) );
  gLayout->addWidget( label, 0, 0 );
  gLayout->addWidget( mToleranceWidget, 0, 1 );

  QWidget *w = new QWidget();
  w->setLayout( gLayout );
  setDefaultWidget( w );
}
