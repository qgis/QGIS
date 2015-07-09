/***************************************************************************
     QgsGeorefPluginGui.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:03:52 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QClipboard>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPrinter>
#include <QProgressDialog>
#include <QPushButton>
#include <QSettings>
#include <QTextStream>
#include <QPen>
#include <QStringList>
#include <QList>

#include "qgisinterface.h"
#include "qgslegendinterface.h"
#include "qgsapplication.h"

#include "qgscomposerlabel.h"
#include "qgscomposermap.h"
#include "qgscomposertexttable.h"
#include "qgscomposertablecolumn.h"
#include "qgscomposerframe.h"
#include "qgsmapcanvas.h"
#include "qgsmapcoordsdialog.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsmaptoolzoom.h"
#include "qgsmaptoolpan.h"

#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "../../app/qgsrasterlayerproperties.h"
#include "qgsproviderregistry.h"

#include "qgsgeorefdatapoint.h"
#include "qgsgeoreftooladdpoint.h"
#include "qgsgeoreftooldeletepoint.h"
#include "qgsgeoreftoolmovepoint.h"

#include "qgsleastsquares.h"
#include "qgsgcplistwidget.h"

#include "qgsgeorefconfigdialog.h"
#include "qgsgeorefdescriptiondialog.h"
#include "qgsresidualplotitem.h"
#include "qgstransformsettingsdialog.h"

#include "qgsgeorefplugingui.h"
#include "qgsmessagebar.h"

QgsGeorefDockWidget::QgsGeorefDockWidget( const QString & title, QWidget * parent, Qt::WindowFlags flags )
    : QDockWidget( title, parent, flags )
{
  setObjectName( "GeorefDockWidget" ); // set object name so the position can be saved
}

QgsGeorefPluginGui::QgsGeorefPluginGui( QgisInterface* theQgisInterface, QWidget* parent, Qt::WindowFlags fl )
    : QMainWindow( parent, fl )
    , mMousePrecisionDecimalPlaces( 0 )
    , mTransformParam( QgsGeorefTransform::InvalidTransform )
    , mIface( theQgisInterface )
    , mLayer( 0 )
    , mAgainAddRaster( false )
    , mMovingPoint( 0 )
    , mMovingPointQgis( 0 )
    , mMapCoordsDialog( 0 )
    , mUseZeroForTrans( false )
    , mLoadInQgis( false )
    , mDock( 0 )
{
  setupUi( this );

  QSettings s;
  restoreGeometry( s.value( "/Plugin-GeoReferencer/Window/geometry" ).toByteArray() );

  QWidget *centralWidget = this->centralWidget();
  mCentralLayout = new QGridLayout( centralWidget );
  centralWidget->setLayout( mCentralLayout );
  mCentralLayout->setContentsMargins( 0, 0, 0, 0 );

  createActions();
  createActionGroups();
  createMenus();
  createMapCanvas();
  createDockWidgets();
  createStatusBar();

  // a bar to warn the user with non-blocking messages
  mMessageBar = new QgsMessageBar( centralWidget );
  mMessageBar->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Fixed );
  mCentralLayout->addWidget( mMessageBar, 0, 0, 1, 1 );

  setAddPointTool();
  setupConnections();
  readSettings();

  mActionLinkGeorefToQGis->setEnabled( false );
  mActionLinkQGisToGeoref->setEnabled( false );

  mCanvas->clearExtentHistory(); // reset zoomnext/zoomlast

  connect( mIface, SIGNAL( currentThemeChanged( QString ) ), this, SLOT( updateIconTheme( QString ) ) );

  if ( s.value( "/Plugin-GeoReferencer/Config/ShowDocked" ).toBool() )
  {
    dockThisWindow( true );
  }
}

void QgsGeorefPluginGui::dockThisWindow( bool dock )
{
  if ( mDock )
  {
    setParent( mIface->mainWindow(), Qt::Window );
    show();

    mIface->removeDockWidget( mDock );
    mDock->setWidget( 0 );
    delete mDock;
    mDock = 0;
  }

  if ( dock )
  {
    mDock = new QgsGeorefDockWidget( tr( "Georeferencer" ), mIface->mainWindow() );
    mDock->setWidget( this );
    mIface->addDockWidget( Qt::BottomDockWidgetArea, mDock );
  }
}

QgsGeorefPluginGui::~QgsGeorefPluginGui()
{
  QSettings settings;
  settings.setValue( "/Plugin-GeoReferencer/Window/geometry", saveGeometry() );

  clearGCPData();

  removeOldLayer();

  delete mToolZoomIn;
  delete mToolZoomOut;
  delete mToolPan;
  delete mToolAddPoint;
  delete mToolDeletePoint;
  delete mToolMovePoint;
  delete mToolMovePointQgis;
}

// ----------------------------- protected --------------------------------- //
void QgsGeorefPluginGui::closeEvent( QCloseEvent *e )
{
  switch ( checkNeedGCPSave() )
  {
    case QgsGeorefPluginGui::GCPSAVE:
      if ( mGCPpointsFileName.isEmpty() )
        saveGCPsDialog();
      else
        saveGCPs();
      writeSettings();
      clearGCPData();
      removeOldLayer();
      mRasterFileName = "";
      e->accept();
      return;
    case QgsGeorefPluginGui::GCPSILENTSAVE:
      if ( !mGCPpointsFileName.isEmpty() )
        saveGCPs();
      clearGCPData();
      removeOldLayer();
      mRasterFileName = "";
      return;
    case QgsGeorefPluginGui::GCPDISCARD:
      writeSettings();
      clearGCPData();
      removeOldLayer();
      mRasterFileName = "";
      e->accept();
      return;
    case QgsGeorefPluginGui::GCPCANCEL:
      e->ignore();
      return;
  }
}

void QgsGeorefPluginGui::reset()
{
  if ( QMessageBox::question( this,
                              tr( "Reset Georeferencer" ),
                              tr( "Reset georeferencer and clear all GCP points?" ),
                              QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel ) != QMessageBox::Cancel )
  {
    mRasterFileName.clear();
    mModifiedRasterFileName.clear();
    setWindowTitle( tr( "Georeferencer" ) );

    //delete old points
    clearGCPData();

    //delete any old rasterlayers
    removeOldLayer();
  }
}

// -------------------------- private slots -------------------------------- //
// File slots
void QgsGeorefPluginGui::openRaster()
{
  //  clearLog();
  switch ( checkNeedGCPSave() )
  {
    case QgsGeorefPluginGui::GCPSAVE:
      saveGCPsDialog();
      break;
    case QgsGeorefPluginGui::GCPSILENTSAVE:
      if ( !mGCPpointsFileName.isEmpty() )
        saveGCPs();
      break;
    case QgsGeorefPluginGui::GCPDISCARD:
      break;
    case QgsGeorefPluginGui::GCPCANCEL:
      return;
  }

  QSettings s;
  QString dir = s.value( "/Plugin-GeoReferencer/rasterdirectory" ).toString();
  if ( dir.isEmpty() )
    dir = ".";

  QString otherFiles = tr( "All other files (*)" );
  QString lastUsedFilter = s.value( "/Plugin-GeoReferencer/lastusedfilter", otherFiles ).toString();

  QString filters = QgsProviderRegistry::instance()->fileRasterFilters();
  filters.prepend( otherFiles + ";;" );
  filters.chop( otherFiles.size() + 2 );
  mRasterFileName = QFileDialog::getOpenFileName( this, tr( "Open raster" ), dir, filters, &lastUsedFilter );
  mModifiedRasterFileName = "";

  if ( mRasterFileName.isEmpty() )
    return;

  QString errMsg;
  if ( !QgsRasterLayer::isValidRasterFileName( mRasterFileName, errMsg ) )
  {
    QString msg = tr( "%1 is not a supported raster data source" ).arg( mRasterFileName );

    if ( errMsg.size() > 0 )
      msg += "\n" + errMsg;

    QMessageBox::information( this, tr( "Unsupported Data Source" ), msg );
    return;
  }

  QFileInfo fileInfo( mRasterFileName );
  s.setValue( "/Plugin-GeoReferencer/rasterdirectory", fileInfo.path() );
  s.setValue( "/Plugin-GeoReferencer/lastusedfilter", lastUsedFilter );

  mGeorefTransform.selectTransformParametrisation( mTransformParam );
  mGeorefTransform.setRasterChangeCoords( mRasterFileName );
  statusBar()->showMessage( tr( "Raster loaded: %1" ).arg( mRasterFileName ) );
  setWindowTitle( tr( "Georeferencer - %1" ).arg( fileInfo.fileName() ) );

  //  showMessageInLog(tr("Input raster"), mRasterFileName);

  //delete old points
  clearGCPData();

  //delete any old rasterlayers
  removeOldLayer();

  // Add raster
  addRaster( mRasterFileName );

  // load previously added points
  mGCPpointsFileName = mRasterFileName + ".points";
  ( void )loadGCPs();

  mCanvas->setExtent( mLayer->extent() );
  mCanvas->refresh();
  mIface->mapCanvas()->refresh();

  mActionLinkGeorefToQGis->setChecked( false );
  mActionLinkQGisToGeoref->setChecked( false );
  mActionLinkGeorefToQGis->setEnabled( false );
  mActionLinkQGisToGeoref->setEnabled( false );

  mCanvas->clearExtentHistory(); // reset zoomnext/zoomlast
  mWorldFileName = guessWorldFileName( mRasterFileName );
}

void QgsGeorefPluginGui::doGeoreference()
{
  if ( georeference() )
  {
    mMessageBar->pushMessage( tr( "Georeference Successful" ), tr( "Raster was successfully georeferenced." ), QgsMessageBar::INFO, messageTimeout() );
    if ( mLoadInQgis )
    {
      if ( mModifiedRasterFileName.isEmpty() )
      {
        mIface->addRasterLayer( mRasterFileName );
      }
      else
      {
        mIface->addRasterLayer( mModifiedRasterFileName );
      }

      //      showMessageInLog(tr("Modified raster saved in"), mModifiedRasterFileName);
      //      saveGCPs();

      //      mTransformParam = QgsGeorefTransform::InvalidTransform;
      //      mGeorefTransform.selectTransformParametrisation(mTransformParam);
      //      mGCPListWidget->setGeorefTransform(&mGeorefTransform);
      //      mTransformParamLabel->setText(tr("Transform: ") + convertTransformEnumToString(mTransformParam));

      mActionLinkGeorefToQGis->setEnabled( false );
      mActionLinkQGisToGeoref->setEnabled( false );
    }
  }
}

bool QgsGeorefPluginGui::getTransformSettings()
{
  QgsTransformSettingsDialog d( mRasterFileName, mModifiedRasterFileName, mPoints.size() );
  if ( !d.exec() )
  {
    return false;
  }

  d.getTransformSettings( mTransformParam, mResamplingMethod, mCompressionMethod,
                          mModifiedRasterFileName, mProjection, mPdfOutputMapFile, mPdfOutputFile, mUseZeroForTrans, mLoadInQgis, mUserResX, mUserResY );
  mTransformParamLabel->setText( tr( "Transform: " ) + convertTransformEnumToString( mTransformParam ) );
  mGeorefTransform.selectTransformParametrisation( mTransformParam );
  mGCPListWidget->setGeorefTransform( &mGeorefTransform );
  mWorldFileName = guessWorldFileName( mRasterFileName );

  //  showMessageInLog(tr("Output raster"), mModifiedRasterFileName.isEmpty() ? tr("Non set") : mModifiedRasterFileName);
  //  showMessageInLog(tr("Target projection"), mProjection.isEmpty() ? tr("Non set") : mProjection);
  //  logTransformOptions();
  //  logRequaredGCPs();

  if ( QgsGeorefTransform::InvalidTransform != mTransformParam )
  {
    mActionLinkGeorefToQGis->setEnabled( true );
    mActionLinkQGisToGeoref->setEnabled( true );
  }
  else
  {
    mActionLinkGeorefToQGis->setEnabled( false );
    mActionLinkQGisToGeoref->setEnabled( false );
  }

  updateTransformParamLabel();
  return true;
}

void QgsGeorefPluginGui::generateGDALScript()
{
  if ( !checkReadyGeoref() )
    return;

  switch ( mTransformParam )
  {
    case QgsGeorefTransform::PolynomialOrder1:
    case QgsGeorefTransform::PolynomialOrder2:
    case QgsGeorefTransform::PolynomialOrder3:
    case QgsGeorefTransform::ThinPlateSpline:
    {
      // CAVEAT: generateGDALwarpCommand() relies on some member variables being set
      // by generateGDALtranslateCommand(), so this method must be called before
      // gdalwarpCommand*()!
      QString translateCommand = generateGDALtranslateCommand( false );
      QString gdalwarpCommand;
      QString resamplingStr = convertResamplingEnumToString( mResamplingMethod );

      int order = polynomialOrder( mTransformParam );
      if ( order != 0 )
      {
        gdalwarpCommand = generateGDALwarpCommand( resamplingStr, mCompressionMethod, mUseZeroForTrans, order,
                          mUserResX, mUserResY );
        showGDALScript( QStringList() << translateCommand << gdalwarpCommand );
        break;
      }
    }
    default:
      mMessageBar->pushMessage( tr( "Invalid Transform" ), tr( "GDAL scripting is not supported for %1 transformation." )
                                .arg( convertTransformEnumToString( mTransformParam ) )
                                , QgsMessageBar::WARNING, messageTimeout() );
  }
}

// Edit slots
void QgsGeorefPluginGui::setAddPointTool()
{
  mCanvas->setMapTool( mToolAddPoint );
}

void QgsGeorefPluginGui::setDeletePointTool()
{
  mCanvas->setMapTool( mToolDeletePoint );
}

void QgsGeorefPluginGui::setMovePointTool()
{
  mCanvas->setMapTool( mToolMovePoint );
  mIface->mapCanvas()->setMapTool( mToolMovePointQgis );
}

// View slots
void QgsGeorefPluginGui::setPanTool()
{
  mCanvas->setMapTool( mToolPan );
}

void QgsGeorefPluginGui::setZoomInTool()
{
  mCanvas->setMapTool( mToolZoomIn );
}

void QgsGeorefPluginGui::setZoomOutTool()
{
  mCanvas->setMapTool( mToolZoomOut );
}

void QgsGeorefPluginGui::zoomToLayerTool()
{
  if ( mLayer )
  {
    mCanvas->setExtent( mLayer->extent() );
    mCanvas->refresh();
  }
}

void QgsGeorefPluginGui::zoomToLast()
{
  mCanvas->zoomToPreviousExtent();
}

void QgsGeorefPluginGui::zoomToNext()
{
  mCanvas->zoomToNextExtent();
}

void QgsGeorefPluginGui::linkQGisToGeoref( bool link )
{
  if ( link )
  {
    if ( QgsGeorefTransform::InvalidTransform != mTransformParam )
    {
      // Indicate that georeferencer canvas extent has changed
      extentsChangedGeorefCanvas();
    }
    else
    {
      mActionLinkGeorefToQGis->setEnabled( false );
    }
  }
}

void QgsGeorefPluginGui::linkGeorefToQGis( bool link )
{
  if ( link )
  {
    if ( QgsGeorefTransform::InvalidTransform != mTransformParam )
    {
      // Indicate that qgis main canvas extent has changed
      extentsChangedQGisCanvas();
    }
    else
    {
      mActionLinkQGisToGeoref->setEnabled( false );
    }
  }
}

// GCPs slots
void QgsGeorefPluginGui::addPoint( const QgsPoint& pixelCoords, const QgsPoint& mapCoords,
                                   bool enable, bool refreshCanvas/*, bool verbose*/ )
{
  QgsGeorefDataPoint* pnt = new QgsGeorefDataPoint( mCanvas, mIface->mapCanvas(),
      pixelCoords, mapCoords, enable );
  mPoints.append( pnt );
  mGCPsDirty = true;
  mGCPListWidget->setGCPList( &mPoints );
  if ( refreshCanvas )
  {
    mCanvas->refresh();
    mIface->mapCanvas()->refresh();
  }

  connect( mCanvas, SIGNAL( extentsChanged() ), pnt, SLOT( updateCoords() ) );
  updateGeorefTransform();

  //  if (verbose)
  //    logRequaredGCPs();
}

void QgsGeorefPluginGui::deleteDataPoint( const QPoint &coords )
{
  for ( QgsGCPList::iterator it = mPoints.begin(); it != mPoints.end(); ++it )
  {
    QgsGeorefDataPoint* pt = *it;
    if ( /*pt->pixelCoords() == coords ||*/ pt->contains( coords, true ) ) // first operand for removing from GCP table
    {
      delete *it;
      mPoints.erase( it );
      mGCPListWidget->updateGCPList();

      mCanvas->refresh();
      break;
    }
  }
  updateGeorefTransform();
}

void QgsGeorefPluginGui::deleteDataPoint( int theGCPIndex )
{
  Q_ASSERT( theGCPIndex >= 0 );
  delete mPoints.takeAt( theGCPIndex );
  mGCPListWidget->updateGCPList();
  updateGeorefTransform();
}

void QgsGeorefPluginGui::selectPoint( const QPoint &p )
{
  // Get Map Sender
  bool isMapPlugin = sender() == mToolMovePoint;
  QgsGeorefDataPoint *&mvPoint = isMapPlugin ? mMovingPoint : mMovingPointQgis;

  for ( QgsGCPList::iterator it = mPoints.begin(); it != mPoints.end(); ++it )
  {
    if (( *it )->contains( p, isMapPlugin ) )
    {
      mvPoint = *it;
      break;
    }
  }
}

void QgsGeorefPluginGui::movePoint( const QPoint &p )
{
  // Get Map Sender
  bool isMapPlugin = sender() == mToolMovePoint;
  QgsGeorefDataPoint *mvPoint = isMapPlugin ? mMovingPoint : mMovingPointQgis;

  if ( mvPoint )
  {
    mvPoint->moveTo( p, isMapPlugin );
    mGCPListWidget->updateGCPList();
  }

}

void QgsGeorefPluginGui::releasePoint( const QPoint &p )
{
  Q_UNUSED( p );
  // Get Map Sender
  if ( sender() == mToolMovePoint )
  {
    mMovingPoint = 0;
  }
  else
  {
    mMovingPointQgis = 0;
  }
}

void QgsGeorefPluginGui::showCoordDialog( const QgsPoint &pixelCoords )
{
  if ( mLayer && !mMapCoordsDialog )
  {
    mMapCoordsDialog = new QgsMapCoordsDialog( mIface->mapCanvas(), pixelCoords, this );
    connect( mMapCoordsDialog, SIGNAL( pointAdded( const QgsPoint &, const QgsPoint & ) ),
             this, SLOT( addPoint( const QgsPoint &, const QgsPoint & ) ) );
    mMapCoordsDialog->show();
  }
}

void QgsGeorefPluginGui::loadGCPsDialog()
{
  QString selectedFile = mRasterFileName.isEmpty() ? "" : mRasterFileName + ".points";
  mGCPpointsFileName = QFileDialog::getOpenFileName( this, tr( "Load GCP points" ),
                       selectedFile, tr( "GCP file" ) + " (*.points)" );
  if ( mGCPpointsFileName.isEmpty() )
    return;

  if ( !loadGCPs() )
  {
    mMessageBar->pushMessage( tr( "Invalid GCP file" ), tr( "GCP file could not be read." ), QgsMessageBar::WARNING, messageTimeout() );
  }
  else
  {
    mMessageBar->pushMessage( tr( "GCPs loaded" ), tr( "GCP file successfully loaded." ), QgsMessageBar::INFO, messageTimeout() );
  }
}

void QgsGeorefPluginGui::saveGCPsDialog()
{
  if ( mPoints.isEmpty() )
  {
    mMessageBar->pushMessage( tr( "No GCP Points" ), tr( "No GCP points are available to save." ), QgsMessageBar::WARNING, messageTimeout() );
    return;
  }

  QString selectedFile = mRasterFileName.isEmpty() ? "" : mRasterFileName + ".points";
  mGCPpointsFileName = QFileDialog::getSaveFileName( this, tr( "Save GCP points" ),
                       selectedFile,
                       tr( "GCP file" ) + " (*.points)" );

  if ( mGCPpointsFileName.isEmpty() )
    return;

  if ( mGCPpointsFileName.right( 7 ) != ".points" )
    mGCPpointsFileName += ".points";

  saveGCPs();
}

// Settings slots
void QgsGeorefPluginGui::showRasterPropertiesDialog()
{
  if ( mLayer )
  {
    mIface->showLayerProperties( mLayer );
  }
  else
  {
    mMessageBar->pushMessage( tr( "Raster Properties" ), tr( "Please load raster to be georeferenced." ), QgsMessageBar::INFO, messageTimeout() );
  }
}

void QgsGeorefPluginGui::showGeorefConfigDialog()
{
  QgsGeorefConfigDialog config;
  if ( config.exec() == QDialog::Accepted )
  {
    mCanvas->refresh();
    mIface->mapCanvas()->refresh();
    QSettings s;
    //update dock state
    bool dock = s.value( "/Plugin-GeoReferencer/Config/ShowDocked" ).toBool();
    if ( dock && !mDock )
    {
      dockThisWindow( true );
    }
    else if ( !dock && mDock )
    {
      dockThisWindow( false );
    }
    //update gcp model
    if ( mGCPListWidget )
    {
      mGCPListWidget->updateGCPList();
    }
    //and status bar
    updateTransformParamLabel();
  }
}

// Histogram stretch slots
void QgsGeorefPluginGui::fullHistogramStretch()
{
  mLayer->setContrastEnhancement( QgsContrastEnhancement::StretchToMinimumMaximum );
  mCanvas->refresh();
}

void QgsGeorefPluginGui::localHistogramStretch()
{
  QgsRectangle rectangle = mIface->mapCanvas()->mapSettings().outputExtentToLayerExtent( mLayer, mIface->mapCanvas()->extent() );

  mLayer->setContrastEnhancement( QgsContrastEnhancement::StretchToMinimumMaximum, QgsRaster::ContrastEnhancementMinMax, rectangle );
  mCanvas->refresh();
}

// Info slots
void QgsGeorefPluginGui::contextHelp()
{
  QgsGeorefDescriptionDialog dlg( this );
  dlg.exec();
}

// Comfort slots
void QgsGeorefPluginGui::jumpToGCP( uint theGCPIndex )
{
  if (( int )theGCPIndex >= mPoints.size() )
  {
    return;
  }

  // qgsmapcanvas doesn't seem to have a method for recentering the map
  QgsRectangle ext = mCanvas->extent();

  QgsPoint center = ext.center();
  QgsPoint new_center = mPoints[theGCPIndex]->pixelCoords();

  QgsPoint diff( new_center.x() - center.x(), new_center.y() - center.y() );
  QgsRectangle new_extent( ext.xMinimum() + diff.x(), ext.yMinimum() + diff.y(),
                           ext.xMaximum() + diff.x(), ext.yMaximum() + diff.y() );
  mCanvas->setExtent( new_extent );
  mCanvas->refresh();
}

// This slot is called whenever the georeference canvas changes the displayed extent
void QgsGeorefPluginGui::extentsChangedGeorefCanvas()
{
  // Guard against endless recursion by ping-pong updates
  if ( mExtentsChangedRecursionGuard )
  {
    return;
  }

  if ( mActionLinkQGisToGeoref->isChecked() )
  {
    if ( !updateGeorefTransform() )
    {
      return;
    }

    // Reproject the georeference plugin canvas into world coordinates and fit axis aligned bounding box
    QgsRectangle rectMap = mGeorefTransform.hasCrs() ? mGeorefTransform.getBoundingBox( mCanvas->extent(), true ) : mCanvas->extent();
    QgsRectangle boundingBox = transformViewportBoundingBox( rectMap, mGeorefTransform, true );

    mExtentsChangedRecursionGuard = true;
    // Just set the whole extent for now
    // TODO: better fitting function which acounts for differing aspect ratios etc.
    mIface->mapCanvas()->setExtent( boundingBox );
    mIface->mapCanvas()->refresh();
    mExtentsChangedRecursionGuard = false;
  }
}

// This slot is called whenever the qgis main canvas changes the displayed extent
void QgsGeorefPluginGui::extentsChangedQGisCanvas()
{
  // Guard against endless recursion by ping-pong updates
  if ( mExtentsChangedRecursionGuard )
  {
    return;
  }

  if ( mActionLinkGeorefToQGis->isChecked() )
  {
    // Update transform if necessary
    if ( !updateGeorefTransform() )
    {
      return;
    }

    // Reproject the canvas into raster coordinates and fit axis aligned bounding box
    QgsRectangle boundingBox = transformViewportBoundingBox( mIface->mapCanvas()->extent(), mGeorefTransform, false );
    QgsRectangle rectMap = mGeorefTransform.hasCrs() ? mGeorefTransform.getBoundingBox( boundingBox, false ) : boundingBox;

    mExtentsChangedRecursionGuard = true;
    // Just set the whole extent for now
    // TODO: better fitting function which acounts for differing aspect ratios etc.
    mCanvas->setExtent( rectMap );
    mCanvas->refresh();
    mExtentsChangedRecursionGuard = false;
  }
}

// Canvas info slots (copy/pasted from QGIS :) )
void QgsGeorefPluginGui::showMouseCoords( const QgsPoint &p )
{
  mCoordsLabel->setText( p.toString( mMousePrecisionDecimalPlaces ) );
  // Set minimum necessary width
  if ( mCoordsLabel->width() > mCoordsLabel->minimumWidth() )
  {
    mCoordsLabel->setMinimumWidth( mCoordsLabel->width() );
  }
}

void QgsGeorefPluginGui::updateMouseCoordinatePrecision()
{
  // Work out what mouse display precision to use. This only needs to
  // be when the s change or the zoom level changes. This
  // function needs to be called every time one of the above happens.

  // Get the display precision from the project s
  bool automatic = QgsProject::instance()->readBoolEntry( "PositionPrecision", "/Automatic" );
  int dp = 0;

  if ( automatic )
  {
    // Work out a suitable number of decimal places for the mouse
    // coordinates with the aim of always having enough decimal places
    // to show the difference in position between adjacent pixels.
    // Also avoid taking the log of 0.
    if ( mCanvas->mapUnitsPerPixel() != 0.0 )
      dp = static_cast<int>( ceil( -1.0 * log10( mCanvas->mapUnitsPerPixel() ) ) );
  }
  else
    dp = QgsProject::instance()->readNumEntry( "PositionPrecision", "/DecimalPlaces" );

  // Keep dp sensible
  if ( dp < 0 )
    dp = 0;

  mMousePrecisionDecimalPlaces = dp;
}

void QgsGeorefPluginGui::extentsChanged()
{
  if ( mAgainAddRaster )
  {
    if ( QFile::exists( mRasterFileName ) )
    {
      addRaster( mRasterFileName );
    }
    else
    {
      mLayer = 0;
      mAgainAddRaster = false;
    }
  }
}

// Registry layer QGis
void QgsGeorefPluginGui::layerWillBeRemoved( QString theLayerId )
{
  mAgainAddRaster = mLayer && mLayer->id().compare( theLayerId ) == 0;
}

// ------------------------------ private ---------------------------------- //
// Gui
void QgsGeorefPluginGui::createActions()
{
  // File actions
  connect( mActionReset, SIGNAL( triggered() ), this, SLOT( reset() ) );

  mActionOpenRaster->setIcon( getThemeIcon( "/mActionAddRasterLayer.svg" ) );
  connect( mActionOpenRaster, SIGNAL( triggered() ), this, SLOT( openRaster() ) );

  mActionStartGeoref->setIcon( getThemeIcon( "/mActionStartGeoref.png" ) );
  connect( mActionStartGeoref, SIGNAL( triggered() ), this, SLOT( doGeoreference() ) );

  mActionGDALScript->setIcon( getThemeIcon( "/mActionGDALScript.png" ) );
  connect( mActionGDALScript, SIGNAL( triggered() ), this, SLOT( generateGDALScript() ) );

  mActionLoadGCPpoints->setIcon( getThemeIcon( "/mActionLoadGCPpoints.png" ) );
  connect( mActionLoadGCPpoints, SIGNAL( triggered() ), this, SLOT( loadGCPsDialog() ) );

  mActionSaveGCPpoints->setIcon( getThemeIcon( "/mActionSaveGCPpointsAs.png" ) );
  connect( mActionSaveGCPpoints, SIGNAL( triggered() ), this, SLOT( saveGCPsDialog() ) );

  mActionTransformSettings->setIcon( getThemeIcon( "/mActionTransformSettings.png" ) );
  connect( mActionTransformSettings, SIGNAL( triggered() ), this, SLOT( getTransformSettings() ) );

  // Edit actions
  mActionAddPoint->setIcon( getThemeIcon( "/mActionAddGCPPoint.png" ) );
  connect( mActionAddPoint, SIGNAL( triggered() ), this, SLOT( setAddPointTool() ) );

  mActionDeletePoint->setIcon( getThemeIcon( "/mActionDeleteGCPPoint.png" ) );
  connect( mActionDeletePoint, SIGNAL( triggered() ), this, SLOT( setDeletePointTool() ) );

  mActionMoveGCPPoint->setIcon( getThemeIcon( "/mActionMoveGCPPoint.png" ) );
  connect( mActionMoveGCPPoint, SIGNAL( triggered() ), this, SLOT( setMovePointTool() ) );

  // View actions
  mActionPan->setIcon( getThemeIcon( "/mActionPan.svg" ) );
  connect( mActionPan, SIGNAL( triggered() ), this, SLOT( setPanTool() ) );

  mActionZoomIn->setIcon( getThemeIcon( "/mActionZoomIn.svg" ) );
  connect( mActionZoomIn, SIGNAL( triggered() ), this, SLOT( setZoomInTool() ) );

  mActionZoomOut->setIcon( getThemeIcon( "/mActionZoomOut.svg" ) );
  connect( mActionZoomOut, SIGNAL( triggered() ), this, SLOT( setZoomOutTool() ) );

  mActionZoomToLayer->setIcon( getThemeIcon( "/mActionZoomToLayer.svg" ) );
  connect( mActionZoomToLayer, SIGNAL( triggered() ), this, SLOT( zoomToLayerTool() ) );

  mActionZoomLast->setIcon( getThemeIcon( "/mActionZoomLast.svg" ) );
  connect( mActionZoomLast, SIGNAL( triggered() ), this, SLOT( zoomToLast() ) );

  mActionZoomNext->setIcon( getThemeIcon( "/mActionZoomNext.svg" ) );
  connect( mActionZoomNext, SIGNAL( triggered() ), this, SLOT( zoomToNext() ) );

  mActionLinkGeorefToQGis->setIcon( getThemeIcon( "/mActionLinkGeorefToQGis.png" ) );
  connect( mActionLinkGeorefToQGis, SIGNAL( triggered( bool ) ), this, SLOT( linkGeorefToQGis( bool ) ) );

  mActionLinkQGisToGeoref->setIcon( getThemeIcon( "/mActionLinkQGisToGeoref.png" ) );
  connect( mActionLinkQGisToGeoref, SIGNAL( triggered( bool ) ), this, SLOT( linkQGisToGeoref( bool ) ) );

  // Settings actions
  mActionRasterProperties->setIcon( getThemeIcon( "/mActionRasterProperties.png" ) );
  connect( mActionRasterProperties, SIGNAL( triggered() ), this, SLOT( showRasterPropertiesDialog() ) );

  mActionGeorefConfig->setIcon( getThemeIcon( "/mActionGeorefConfig.png" ) );
  connect( mActionGeorefConfig, SIGNAL( triggered() ), this, SLOT( showGeorefConfigDialog() ) );

  // Histogram stretch
  mActionLocalHistogramStretch->setIcon( getThemeIcon( "/mActionLocalHistogramStretch.png" ) );
  connect( mActionLocalHistogramStretch, SIGNAL( triggered() ), this, SLOT( localHistogramStretch() ) );
  mActionLocalHistogramStretch->setEnabled( false );

  mActionFullHistogramStretch->setIcon( getThemeIcon( "/mActionFullHistogramStretch.png" ) );
  connect( mActionFullHistogramStretch, SIGNAL( triggered() ), this, SLOT( fullHistogramStretch() ) );
  mActionFullHistogramStretch->setEnabled( false );

  // Help actions
  mActionHelp = new QAction( tr( "Help" ), this );
  connect( mActionHelp, SIGNAL( triggered() ), this, SLOT( contextHelp() ) );

  mActionQuit->setIcon( getThemeIcon( "/mActionQuit.png" ) );
  mActionQuit->setShortcuts( QList<QKeySequence>() << QKeySequence( Qt::CTRL + Qt::Key_Q )
                             << QKeySequence( Qt::Key_Escape ) );
  connect( mActionQuit, SIGNAL( triggered() ), this, SLOT( close() ) );
}

void QgsGeorefPluginGui::createActionGroups()
{
  QActionGroup *mapToolGroup = new QActionGroup( this );
  mActionPan->setCheckable( true );
  mapToolGroup->addAction( mActionPan );
  mActionZoomIn->setCheckable( true );
  mapToolGroup->addAction( mActionZoomIn );
  mActionZoomOut->setCheckable( true );
  mapToolGroup->addAction( mActionZoomOut );

  mActionAddPoint->setCheckable( true );
  mapToolGroup->addAction( mActionAddPoint );
  mActionDeletePoint->setCheckable( true );
  mapToolGroup->addAction( mActionDeletePoint );
  mActionMoveGCPPoint->setCheckable( true );
  mapToolGroup->addAction( mActionMoveGCPPoint );
}

void QgsGeorefPluginGui::createMapCanvas()
{
  // set up the canvas
  mCanvas = new QgsMapCanvas( this->centralWidget(), "georefCanvas" );
  mCanvas->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
  mCanvas->setCanvasColor( Qt::white );
  mCanvas->setMinimumWidth( 400 );
  mCentralLayout->addWidget( mCanvas, 0, 0, 2, 1 );

  // set up map tools
  mToolZoomIn = new QgsMapToolZoom( mCanvas, false /* zoomOut */ );
  mToolZoomIn->setAction( mActionZoomIn );

  mToolZoomOut = new QgsMapToolZoom( mCanvas, true /* zoomOut */ );
  mToolZoomOut->setAction( mActionZoomOut );

  mToolPan = new QgsMapToolPan( mCanvas );
  mToolPan->setAction( mActionPan );

  mToolAddPoint = new QgsGeorefToolAddPoint( mCanvas );
  mToolAddPoint->setAction( mActionAddPoint );
  connect( mToolAddPoint, SIGNAL( showCoordDialog( const QgsPoint & ) ),
           this, SLOT( showCoordDialog( const QgsPoint & ) ) );

  mToolDeletePoint = new QgsGeorefToolDeletePoint( mCanvas );
  mToolDeletePoint->setAction( mActionDeletePoint );
  connect( mToolDeletePoint, SIGNAL( deleteDataPoint( const QPoint & ) ),
           this, SLOT( deleteDataPoint( const QPoint& ) ) );

  mToolMovePoint = new QgsGeorefToolMovePoint( mCanvas );
  mToolMovePoint->setAction( mActionMoveGCPPoint );
  connect( mToolMovePoint, SIGNAL( pointPressed( const QPoint & ) ),
           this, SLOT( selectPoint( const QPoint & ) ) );
  connect( mToolMovePoint, SIGNAL( pointMoved( const QPoint & ) ),
           this, SLOT( movePoint( const QPoint & ) ) );
  connect( mToolMovePoint, SIGNAL( pointReleased( const QPoint & ) ),
           this, SLOT( releasePoint( const QPoint & ) ) );

  // Point in Qgis Map
  mToolMovePointQgis = new QgsGeorefToolMovePoint( mIface->mapCanvas() );
  mToolMovePointQgis->setAction( mActionMoveGCPPoint );
  connect( mToolMovePointQgis, SIGNAL( pointPressed( const QPoint & ) ),
           this, SLOT( selectPoint( const QPoint & ) ) );
  connect( mToolMovePointQgis, SIGNAL( pointMoved( const QPoint & ) ),
           this, SLOT( movePoint( const QPoint & ) ) );
  connect( mToolMovePointQgis, SIGNAL( pointReleased( const QPoint & ) ),
           this, SLOT( releasePoint( const QPoint & ) ) );

  QSettings s;
  int action = s.value( "/qgis/wheel_action", 2 ).toInt();
  double zoomFactor = s.value( "/qgis/zoom_factor", 2 ).toDouble();
  mCanvas->setWheelAction(( QgsMapCanvas::WheelAction ) action, zoomFactor );

  mExtentsChangedRecursionGuard = false;

  mGeorefTransform.selectTransformParametrisation( QgsGeorefTransform::Linear );
  mGCPsDirty = true;

  // Connect main canvas and georef canvas signals so we are aware if any of the viewports change
  // (used by the map follow mode)
  connect( mCanvas, SIGNAL( extentsChanged() ), this, SLOT( extentsChangedGeorefCanvas() ) );
  connect( mIface->mapCanvas(), SIGNAL( extentsChanged() ), this, SLOT( extentsChangedQGisCanvas() ) );
}

void QgsGeorefPluginGui::createMenus()
{
  // Get platform for menu layout customization (Gnome, Kde, Mac, Win)
  QDialogButtonBox::ButtonLayout layout =
    QDialogButtonBox::ButtonLayout( style()->styleHint( QStyle::SH_DialogButtonLayout, 0, this ) );

  mPanelMenu = new QMenu( tr( "Panels" ) );
  mPanelMenu->setObjectName( "mPanelMenu" );
  mPanelMenu->addAction( dockWidgetGCPpoints->toggleViewAction() );
  //  mPanelMenu->addAction(dockWidgetLogView->toggleViewAction());

  mToolbarMenu = new QMenu( tr( "Toolbars" ) );
  mToolbarMenu->setObjectName( "mToolbarMenu" );
  mToolbarMenu->addAction( toolBarFile->toggleViewAction() );
  mToolbarMenu->addAction( toolBarEdit->toggleViewAction() );
  mToolbarMenu->addAction( toolBarView->toggleViewAction() );

  QSettings s;
  int size = s.value( "/IconSize", 32 ).toInt();
  toolBarFile->setIconSize( QSize( size, size ) );
  toolBarEdit->setIconSize( QSize( size, size ) );
  toolBarView->setIconSize( QSize( size, size ) );
  toolBarHistogramStretch->setIconSize( QSize( size, size ) );

  // View menu
  if ( layout != QDialogButtonBox::KdeLayout )
  {
    menuView->addSeparator();
    menuView->addMenu( mPanelMenu );
    menuView->addMenu( mToolbarMenu );
  }
  else // if ( layout == QDialogButtonBox::KdeLayout )
  {
    menuSettings->addSeparator();
    menuSettings->addMenu( mPanelMenu );
    menuSettings->addMenu( mToolbarMenu );
  }
}

void QgsGeorefPluginGui::createDockWidgets()
{
  //  mLogViewer = new QPlainTextEdit;
  //  mLogViewer->setReadOnly(true);
  //  mLogViewer->setWordWrapMode(QTextOption::NoWrap);
  //  dockWidgetLogView->setWidget(mLogViewer);

  mGCPListWidget = new QgsGCPListWidget( this );
  mGCPListWidget->setGeorefTransform( &mGeorefTransform );
  dockWidgetGCPpoints->setWidget( mGCPListWidget );

  connect( mGCPListWidget, SIGNAL( jumpToGCP( uint ) ), this, SLOT( jumpToGCP( uint ) ) );
#if 0
  connect( mGCPListWidget, SIGNAL( replaceDataPoint( QgsGeorefDataPoint*, int ) ),
           this, SLOT( replaceDataPoint( QgsGeorefDataPoint*, int ) ) );
#endif
  connect( mGCPListWidget, SIGNAL( deleteDataPoint( int ) ),
           this, SLOT( deleteDataPoint( int ) ) );
  connect( mGCPListWidget, SIGNAL( pointEnabled( QgsGeorefDataPoint*, int ) ), this, SLOT( updateGeorefTransform() ) );
}

QLabel* QgsGeorefPluginGui::createBaseLabelStatus()
{
  QFont myFont( "Arial", 9 );
  QLabel* label = new QLabel( statusBar() );
  label->setFont( myFont );
  label->setMinimumWidth( 10 );
  label->setMaximumHeight( 20 );
  label->setMargin( 3 );
  label->setAlignment( Qt::AlignCenter );
  label->setFrameStyle( QFrame::NoFrame );
  return label;
}

void QgsGeorefPluginGui::createStatusBar()
{
  mTransformParamLabel = createBaseLabelStatus();
  mTransformParamLabel->setText( tr( "Transform: " ) + convertTransformEnumToString( mTransformParam ) );
  mTransformParamLabel->setToolTip( tr( "Current transform parametrisation" ) );
  statusBar()->addPermanentWidget( mTransformParamLabel, 0 );

  mCoordsLabel = createBaseLabelStatus();
  mCoordsLabel->setMaximumWidth( 100 );
  mCoordsLabel->setText( tr( "Coordinate: " ) );
  mCoordsLabel->setToolTip( tr( "Current map coordinate" ) );
  statusBar()->addPermanentWidget( mCoordsLabel, 0 );

  mEPSG = createBaseLabelStatus();
  mEPSG->setText( "EPSG:" );
  statusBar()->addPermanentWidget( mEPSG, 0 );
}

void QgsGeorefPluginGui::setupConnections()
{
  connect( mCanvas, SIGNAL( xyCoordinates( QgsPoint ) ), this, SLOT( showMouseCoords( QgsPoint ) ) );
  connect( mCanvas, SIGNAL( scaleChanged( double ) ), this, SLOT( updateMouseCoordinatePrecision() ) );

  // Connect status from ZoomLast/ZoomNext to corresponding action
  connect( mCanvas, SIGNAL( zoomLastStatusChanged( bool ) ), mActionZoomLast, SLOT( setEnabled( bool ) ) );
  connect( mCanvas, SIGNAL( zoomNextStatusChanged( bool ) ), mActionZoomNext, SLOT( setEnabled( bool ) ) );
  // Connect when one Layer is removed - Case where change the Projetct in QGIS
  connect( QgsMapLayerRegistry::instance(), SIGNAL( layerWillBeRemoved( QString ) ), this, SLOT( layerWillBeRemoved( QString ) ) );

  // Connect extents changed - Use for need add again Raster
  connect( mCanvas, SIGNAL( extentsChanged() ), this, SLOT( extentsChanged() ) );
}

void QgsGeorefPluginGui::removeOldLayer()
{
  // delete layer (and don't signal it as it's our private layer)
  if ( mLayer )
  {
    QgsMapLayerRegistry::instance()->removeMapLayers(
      ( QStringList() << mLayer->id() ) );
    mLayer = NULL;
  }
  mCanvas->refresh();
}

void QgsGeorefPluginGui::updateIconTheme( QString theme )
{
  Q_UNUSED( theme );
  // File actions
  mActionOpenRaster->setIcon( getThemeIcon( "/mActionAddRasterLayer.svg" ) );
  mActionStartGeoref->setIcon( getThemeIcon( "/mActionStartGeoref.png" ) );
  mActionGDALScript->setIcon( getThemeIcon( "/mActionGDALScript.png" ) );
  mActionLoadGCPpoints->setIcon( getThemeIcon( "/mActionLoadGCPpoints.png" ) );
  mActionSaveGCPpoints->setIcon( getThemeIcon( "/mActionSaveGCPpointsAs.png" ) );
  mActionTransformSettings->setIcon( getThemeIcon( "/mActionTransformSettings.png" ) );

  // Edit actions
  mActionAddPoint->setIcon( getThemeIcon( "/mActionAddGCPPoint.png" ) );
  mActionDeletePoint->setIcon( getThemeIcon( "/mActionDeleteGCPPoint.png" ) );
  mActionMoveGCPPoint->setIcon( getThemeIcon( "/mActionMoveGCPPoint.png" ) );

  // View actions
  mActionPan->setIcon( getThemeIcon( "/mActionPan.svg" ) );
  mActionZoomIn->setIcon( getThemeIcon( "/mActionZoomIn.svg" ) );
  mActionZoomOut->setIcon( getThemeIcon( "/mActionZoomOut.svg" ) );
  mActionZoomToLayer->setIcon( getThemeIcon( "/mActionZoomToLayer.svg" ) );
  mActionZoomLast->setIcon( getThemeIcon( "/mActionZoomLast.svg" ) );
  mActionZoomNext->setIcon( getThemeIcon( "/mActionZoomNext.svg" ) );
  mActionLinkGeorefToQGis->setIcon( getThemeIcon( "/mActionLinkGeorefToQGis.png" ) );
  mActionLinkQGisToGeoref->setIcon( getThemeIcon( "/mActionLinkQGisToGeoref.png" ) );

  // Settings actions
  mActionRasterProperties->setIcon( getThemeIcon( "/mActionRasterProperties.png" ) );
  mActionGeorefConfig->setIcon( getThemeIcon( "/mActionGeorefConfig.png" ) );

  mActionQuit->setIcon( getThemeIcon( "/mActionQuit.png" ) );
}

// Mapcanvas Plugin
void QgsGeorefPluginGui::addRaster( QString file )
{
  mLayer = new QgsRasterLayer( file, "Raster" );

  // so layer is not added to legend
  QgsMapLayerRegistry::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mLayer, false, false );

  // add layer to map canvas
  QList<QgsMapCanvasLayer> layers;
  layers.append( QgsMapCanvasLayer( mLayer ) );
  mCanvas->setLayerSet( layers );

  mAgainAddRaster = false;

  mActionLocalHistogramStretch->setEnabled( true );
  mActionFullHistogramStretch->setEnabled( true );

  // Status Bar
  if ( mGeorefTransform.hasCrs() )
  {
    QString authid = mLayer->crs().authid();
    mEPSG->setText( authid );
    mEPSG->setToolTip( mLayer->crs().toProj4() );
  }
  else
  {
    mEPSG->setText( tr( "None" ) );
    mEPSG->setToolTip( tr( "Coordinate of image(column/line)" ) );
  }
}

// Settings
void QgsGeorefPluginGui::readSettings()
{
  QSettings s;
  QRect georefRect = QApplication::desktop()->screenGeometry( mIface->mainWindow() );
  resize( s.value( "/Plugin-GeoReferencer/size", QSize( georefRect.width() / 2 + georefRect.width() / 5,
                   mIface->mainWindow()->height() ) ).toSize() );
  move( s.value( "/Plugin-GeoReferencer/pos", QPoint( parentWidget()->width() / 2 - width() / 2, 0 ) ).toPoint() );
  restoreState( s.value( "/Plugin-GeoReferencer/uistate" ).toByteArray() );

  // warp options
  mResamplingMethod = ( QgsImageWarper::ResamplingMethod )s.value( "/Plugin-GeoReferencer/resamplingmethod",
                      QgsImageWarper::NearestNeighbour ).toInt();
  mCompressionMethod = s.value( "/Plugin-GeoReferencer/compressionmethod", "NONE" ).toString();
  mUseZeroForTrans = s.value( "/Plugin-GeoReferencer/usezerofortrans", false ).toBool();
}

void QgsGeorefPluginGui::writeSettings()
{
  QSettings s;
  s.setValue( "/Plugin-GeoReferencer/pos", pos() );
  s.setValue( "/Plugin-GeoReferencer/size", size() );
  s.setValue( "/Plugin-GeoReferencer/uistate", saveState() );

  // warp options
  s.setValue( "/Plugin-GeoReferencer/transformparam", mTransformParam );
  s.setValue( "/Plugin-GeoReferencer/resamplingmethod", mResamplingMethod );
  s.setValue( "/Plugin-GeoReferencer/compressionmethod", mCompressionMethod );
  s.setValue( "/Plugin-GeoReferencer/usezerofortrans", mUseZeroForTrans );
}

// GCP points
bool QgsGeorefPluginGui::loadGCPs( /*bool verbose*/ )
{
  QFile pointFile( mGCPpointsFileName );
  if ( !pointFile.open( QIODevice::ReadOnly ) )
  {
    return false;
  }

  clearGCPData();

  QTextStream points( &pointFile );
  QString line = points.readLine();
  int i = 0;
  while ( !points.atEnd() )
  {
    line = points.readLine();
    QStringList ls;
    if ( line.contains( QRegExp( "," ) ) ) // in previous format "\t" is delimiter of points in new - ","
    {
      // points from new georeferencer
      ls = line.split( "," );
    }
    else
    {
      // points from prev georeferencer
      ls = line.split( "\t" );
    }

    if ( ls.count() < 4 )
    {
      return false;
    }

    QgsPoint mapCoords( ls.at( 0 ).toDouble(), ls.at( 1 ).toDouble() ); // map x,y
    QgsPoint pixelCoords( ls.at( 2 ).toDouble(), ls.at( 3 ).toDouble() ); // pixel x,y
    if ( ls.count() == 5 )
    {
      bool enable = ls.at( 4 ).toInt();
      addPoint( pixelCoords, mapCoords, enable, false/*, verbose*/ ); // enabled
    }
    else
      addPoint( pixelCoords, mapCoords, true, false );

    ++i;
  }

  mInitialPoints = mPoints;
  //    showMessageInLog(tr("GCP points loaded from"), mGCPpointsFileName);
  mCanvas->refresh();

  return true;
}

void QgsGeorefPluginGui::saveGCPs()
{
  QFile pointFile( mGCPpointsFileName );
  if ( pointFile.open( QIODevice::WriteOnly ) )
  {
    QTextStream points( &pointFile );
    points << "mapX,mapY,pixelX,pixelY,enable" << endl;
    foreach ( QgsGeorefDataPoint *pt, mPoints )
    {
      points << QString( "%1,%2,%3,%4,%5" )
      .arg( qgsDoubleToString( pt->mapCoords().x() ) )
      .arg( qgsDoubleToString( pt->mapCoords().y() ) )
      .arg( qgsDoubleToString( pt->pixelCoords().x() ) )
      .arg( qgsDoubleToString( pt->pixelCoords().y() ) )
      .arg( pt->isEnabled() ) << endl;
    }

    mInitialPoints = mPoints;
  }
  else
  {
    mMessageBar->pushMessage( tr( "Write Error" ), tr( "Could not write to GCP points file %1." ).arg( mGCPpointsFileName ), QgsMessageBar::WARNING, messageTimeout() );
    return;
  }

  //  showMessageInLog(tr("GCP points saved in"), mGCPpointsFileName);
}

QgsGeorefPluginGui::SaveGCPs QgsGeorefPluginGui::checkNeedGCPSave()
{
  if ( 0 == mPoints.count() )
    return QgsGeorefPluginGui::GCPDISCARD;

  if ( !equalGCPlists( mInitialPoints, mPoints ) )
  {
    QMessageBox::StandardButton a = QMessageBox::information( this, tr( "Save GCPs" ),
                                    tr( "Save GCP points?" ),
                                    QMessageBox::Save | QMessageBox::Discard
                                    | QMessageBox::Cancel );
    if ( a == QMessageBox::Save )
    {
      return QgsGeorefPluginGui::GCPSAVE;
    }
    else if ( a == QMessageBox::Cancel )
    {
      return QgsGeorefPluginGui::GCPCANCEL;
    }
    else if ( a == QMessageBox::Discard )
    {
      return QgsGeorefPluginGui::GCPDISCARD;
    }
  }

  return QgsGeorefPluginGui::GCPSILENTSAVE;
}

// Georeference
bool QgsGeorefPluginGui::georeference()
{
  if ( !checkReadyGeoref() )
    return false;

  if ( mModifiedRasterFileName.isEmpty() && ( QgsGeorefTransform::Linear == mGeorefTransform.transformParametrisation() ||
       QgsGeorefTransform::Helmert == mGeorefTransform.transformParametrisation() ) )
  {
    QgsPoint origin;
    double pixelXSize, pixelYSize, rotation;
    if ( !mGeorefTransform.getOriginScaleRotation( origin, pixelXSize, pixelYSize, rotation ) )
    {
      mMessageBar->pushMessage( tr( "Transform Failed" ), tr( "Failed to calculate linear transform parameters." ), QgsMessageBar::WARNING, messageTimeout() );
      return false;
    }

    if ( !mWorldFileName.isEmpty() )
    {
      if ( QFile::exists( mWorldFileName ) )
      {
        int r = QMessageBox::question( this, tr( "World file exists" ),
                                       tr( "<p>The selected file already seems to have a "
                                           "world file! Do you want to replace it with the "
                                           "new world file?</p>" ),
                                       QMessageBox::Yes | QMessageBox::Default,
                                       QMessageBox::No | QMessageBox::Escape );
        if ( r == QMessageBox::No )
          return false;
        else
          QFile::remove( mWorldFileName );
      }
    }
    else
    {
      return false;
    }

    if ( !writeWorldFile( origin, pixelXSize, pixelYSize, rotation ) )
    {
      return false;
    }

    if ( !mPdfOutputFile.isEmpty() )
    {
      writePDFReportFile( mPdfOutputFile, mGeorefTransform );
    }
    if ( !mPdfOutputMapFile.isEmpty() )
    {
      writePDFMapFile( mPdfOutputMapFile, mGeorefTransform );
    }
    return true;
  }
  else // Helmert, Polinom 1, Polinom 2, Polinom 3
  {
    QgsImageWarper warper( this );
    int res = warper.warpFile( mRasterFileName, mModifiedRasterFileName, mGeorefTransform,
                               mResamplingMethod, mUseZeroForTrans, mCompressionMethod, mProjection, mUserResX, mUserResY );
    if ( res == 0 ) // fault to compute GCP transform
    {
      //TODO: be more specific in the error message
      mMessageBar->pushMessage( tr( "Transform Failed" ), tr( "Failed to compute GCP transform: Transform is not solvable." ), QgsMessageBar::WARNING, messageTimeout() );
      return false;
    }
    else if ( res == -1 ) // operation canceled
    {
      QFileInfo fi( mModifiedRasterFileName );
      fi.dir().remove( mModifiedRasterFileName );
      return false;
    }
    else // 1 all right
    {
      if ( !mPdfOutputFile.isEmpty() )
      {
        writePDFReportFile( mPdfOutputFile, mGeorefTransform );
      }
      if ( !mPdfOutputMapFile.isEmpty() )
      {
        writePDFMapFile( mPdfOutputMapFile, mGeorefTransform );
      }
      return true;
    }
  }

  return false;
}

bool QgsGeorefPluginGui::writeWorldFile( QgsPoint origin, double pixelXSize, double pixelYSize, double rotation )
{
  // write the world file
  QFile file( mWorldFileName );
  if ( !file.open( QIODevice::WriteOnly ) )
  {
    mMessageBar->pushMessage( tr( "Error" ), tr( "Could not write to %1." ).arg( mWorldFileName ), QgsMessageBar::CRITICAL, messageTimeout() );
    return false;
  }

  double rotationX = 0;
  double rotationY = 0;

  if ( !qgsDoubleNear( rotation, 0.0 ) )
  {
    rotationX = pixelXSize * sin( rotation );
    rotationY = pixelYSize * sin( rotation );
    pixelXSize *= cos( rotation );
    pixelYSize *= cos( rotation );
  }

  QTextStream stream( &file );
  stream << qgsDoubleToString( pixelXSize ) << endl
  << rotationX << endl
  << rotationY << endl
  << qgsDoubleToString( -pixelYSize ) << endl
  << qgsDoubleToString( origin.x() ) << endl
  << qgsDoubleToString( origin.y() ) << endl;
  return true;
}

bool QgsGeorefPluginGui::calculateMeanError( double& error ) const
{
  if ( mGeorefTransform.transformParametrisation() == QgsGeorefTransform::InvalidTransform )
  {
    return false;
  }

  unsigned int nPointsEnabled = 0;
  QgsGCPList::const_iterator gcpIt = mPoints.constBegin();
  for ( ; gcpIt != mPoints.constEnd(); ++gcpIt )
  {
    if (( *gcpIt )->isEnabled() )
    {
      ++nPointsEnabled;
    }
  }

  if ( nPointsEnabled == mGeorefTransform.getMinimumGCPCount() )
  {
    error = 0;
    return true;
  }
  else if ( nPointsEnabled < mGeorefTransform.getMinimumGCPCount() )
  {
    return false;
  }

  double sumVxSquare = 0;
  double sumVySquare = 0;

  gcpIt = mPoints.constBegin();
  for ( ; gcpIt != mPoints.constEnd(); ++gcpIt )
  {
    if (( *gcpIt )->isEnabled() )
    {
      sumVxSquare += (( *gcpIt )->residual().x() * ( *gcpIt )->residual().x() );
      sumVySquare += (( *gcpIt )->residual().y() * ( *gcpIt )->residual().y() );
    }
  }

  // Calculate the root mean square error, adjusted for degrees of freedom of the transform
  // Caveat: The number of DoFs is assumed to be even (as each control point fixes two degrees of freedom).
  error = sqrt(( sumVxSquare + sumVySquare ) / ( nPointsEnabled - mGeorefTransform.getMinimumGCPCount() ) );
  return true;
}

bool QgsGeorefPluginGui::writePDFMapFile( const QString& fileName, const QgsGeorefTransform& transform )
{
  Q_UNUSED( transform );
  if ( !mCanvas )
  {
    return false;
  }

  QgsRasterLayer *rlayer = ( QgsRasterLayer* ) mCanvas->layer( 0 );
  if ( !rlayer )
  {
    return false;
  }
  double mapRatio =  rlayer->extent().width() / rlayer->extent().height();

  QPrinter printer;
  printer.setOutputFormat( QPrinter::PdfFormat );
  printer.setOutputFileName( fileName );

  QSettings s;
  double paperWidth = s.value( "/Plugin-GeoReferencer/Config/WidthPDFMap", "297" ).toDouble();
  double paperHeight = s.value( "/Plugin-GeoReferencer/Config/HeightPDFMap", "420" ).toDouble();

  //create composition
  QgsComposition* composition = new QgsComposition( mCanvas->mapSettings() );
  if ( mapRatio >= 1 )
  {
    composition->setPaperSize( paperHeight, paperWidth );
  }
  else
  {
    composition->setPaperSize( paperWidth, paperHeight );
  }
  composition->setPrintResolution( 300 );
  printer.setPaperSize( QSizeF( composition->paperWidth(), composition->paperHeight() ), QPrinter::Millimeter );

  double leftMargin = 8;
  double topMargin = 8;
  double contentWidth = composition->paperWidth() - 2 * leftMargin;
  double contentHeight = composition->paperHeight() - 2 * topMargin;

  //composer map
  QgsComposerMap* composerMap = new QgsComposerMap( composition, leftMargin, topMargin, contentWidth, contentHeight );
  composerMap->setKeepLayerSet( true );
  QStringList list;
  list.append( mCanvas->mapSettings().layers()[0] );
  composerMap->setLayerSet( list );
  composerMap->zoomToExtent( rlayer->extent() );
  composition->addItem( composerMap );
  printer.setFullPage( true );
  printer.setColorMode( QPrinter::Color );

  QString residualUnits;
  if ( s.value( "/Plugin-GeoReferencer/Config/ResidualUnits" ) == "mapUnits" && mGeorefTransform.providesAccurateInverseTransformation() )
  {
    residualUnits = tr( "map units" );
  }
  else
  {
    residualUnits = tr( "pixels" );
  }

  //residual plot
  QgsResidualPlotItem* resPlotItem = new QgsResidualPlotItem( composition );
  composition->addItem( resPlotItem );
  resPlotItem->setSceneRect( QRectF( leftMargin, topMargin, contentWidth, contentHeight ) );
  resPlotItem->setExtent( composerMap->extent() );
  resPlotItem->setGCPList( mPoints );
  resPlotItem->setConvertScaleToMapUnits( residualUnits == tr( "map units" ) );

  printer.setResolution( composition->printResolution() );
  QPainter p( &printer );
  composition->setPlotStyle( QgsComposition::Print );
  QRectF paperRectMM = printer.pageRect( QPrinter::Millimeter );
  QRectF paperRectPixel = printer.pageRect( QPrinter::DevicePixel );
  composition->render( &p, paperRectPixel, paperRectMM );

  delete resPlotItem;
  delete composerMap;
  delete composition;

  return true;
}

bool QgsGeorefPluginGui::writePDFReportFile( const QString& fileName, const QgsGeorefTransform& transform )
{
  if ( !mCanvas )
  {
    return false;
  }

  //create composition A4 with 300 dpi
  QgsComposition* composition = new QgsComposition( mCanvas->mapSettings() );
  composition->setPaperSize( 210, 297 ); //A4
  composition->setPrintResolution( 300 );
  composition->setNumPages( 2 );

  QFont titleFont;
  titleFont.setPointSize( 9 );
  titleFont.setBold( true );
  QFont tableHeaderFont;
  tableHeaderFont.setPointSize( 9 );
  tableHeaderFont.setBold( true );
  QFont tableContentFont;
  tableContentFont.setPointSize( 9 );

  QSettings s;
  double leftMargin = s.value( "/Plugin-GeoReferencer/Config/LeftMarginPDF", "2.0" ).toDouble();
  double rightMargin = s.value( "/Plugin-GeoReferencer/Config/RightMarginPDF", "2.0" ).toDouble();
  double contentWidth = 210 - ( leftMargin + rightMargin );

  //title
  QFileInfo rasterFi( mRasterFileName );
  QgsComposerLabel* titleLabel = new QgsComposerLabel( composition );
  titleLabel->setFont( titleFont );
  titleLabel->setText( rasterFi.fileName() );
  composition->addItem( titleLabel );
  titleLabel->setSceneRect( QRectF( leftMargin, 5, contentWidth, 8 ) );
  titleLabel->setFrameEnabled( false );

  //composer map
  QgsRasterLayer *rLayer = ( QgsRasterLayer* ) mCanvas->layer( 0 );
  if ( !rLayer )
  {
    return false;
  }
  QgsRectangle layerExtent = rLayer->extent();
  //calculate width and height considering extent aspect ratio and max Width 206, maxHeight 70
  double widthExtentRatio = contentWidth / layerExtent.width();
  double heightExtentRatio = 70 / layerExtent.height();
  double mapWidthMM = 0;
  double mapHeightMM = 0;
  if ( widthExtentRatio < heightExtentRatio )
  {
    mapWidthMM = contentWidth;
    mapHeightMM = contentWidth / layerExtent.width() * layerExtent.height();
  }
  else
  {
    mapHeightMM = 70;
    mapWidthMM = 70 / layerExtent.height() * layerExtent.width();
  }

  QgsComposerMap* composerMap = new QgsComposerMap( composition, leftMargin, titleLabel->rect().bottom() + titleLabel->pos().y(), mapWidthMM, mapHeightMM );
  composerMap->setLayerSet( mCanvas->mapSettings().layers() );
  composerMap->zoomToExtent( layerExtent );
  composerMap->setMapCanvas( mCanvas );
  composition->addItem( composerMap );

  QgsComposerTextTableV2* parameterTable = 0;
  double scaleX, scaleY, rotation;
  QgsPoint origin;

  QgsComposerLabel* parameterLabel = 0;
  //transformation that involves only scaling and rotation (linear or helmert) ?
  bool wldTransform = transform.getOriginScaleRotation( origin, scaleX, scaleY, rotation );

  QString residualUnits;
  if ( s.value( "/Plugin-GeoReferencer/Config/ResidualUnits" ) == "mapUnits" && mGeorefTransform.providesAccurateInverseTransformation() )
  {
    residualUnits = tr( "map units" );
  }
  else
  {
    residualUnits = tr( "pixels" );
  }

  QGraphicsRectItem* previousItem = composerMap;
  if ( wldTransform )
  {
    QString parameterTitle = tr( "Transformation parameters" ) + QString( " (" ) + convertTransformEnumToString( transform.transformParametrisation() ) + QString( ")" );
    parameterLabel = new QgsComposerLabel( composition );
    parameterLabel->setFont( titleFont );
    parameterLabel->setText( parameterTitle );
    parameterLabel->adjustSizeToText();
    composition->addItem( parameterLabel );
    parameterLabel->setSceneRect( QRectF( leftMargin, composerMap->rect().bottom() + composerMap->pos().y() + 5, contentWidth, 8 ) );
    parameterLabel->setFrameEnabled( false );

    //calculate mean error
    double meanError = 0;
    calculateMeanError( meanError );

    parameterTable = new QgsComposerTextTableV2( composition, false );
    parameterTable->setHeaderFont( tableHeaderFont );
    parameterTable->setContentFont( tableContentFont );

    QgsComposerTableColumns columns;
    columns << new QgsComposerTableColumn( tr( "Translation x" ) )
    << new QgsComposerTableColumn( tr( "Translation y" ) )
    << new QgsComposerTableColumn( tr( "Scale x" ) )
    << new QgsComposerTableColumn( tr( "Scale y" ) )
    << new QgsComposerTableColumn( tr( "Rotation [degrees]" ) )
    << new QgsComposerTableColumn( tr( "Mean error [%1]" ).arg( residualUnits ) );

    parameterTable->setColumns( columns );
    QStringList row;
    row << QString::number( origin.x(), 'f', 3 ) << QString::number( origin.y(), 'f', 3 ) << QString::number( scaleX ) << QString::number( scaleY ) << QString::number( rotation * 180 / M_PI ) << QString::number( meanError );
    parameterTable->addRow( row );

    QgsComposerFrame* tableFrame = new QgsComposerFrame( composition, parameterTable, leftMargin, parameterLabel->rect().bottom() + parameterLabel->pos().y() + 5, contentWidth, 12 );
    parameterTable->addFrame( tableFrame );

    composition->addItem( tableFrame );
    parameterTable->setGridStrokeWidth( 0.1 );

    previousItem = tableFrame;
  }

  QgsComposerLabel* residualLabel = new QgsComposerLabel( composition );
  residualLabel->setFont( titleFont );
  residualLabel->setText( tr( "Residuals" ) );
  composition->addItem( residualLabel );
  residualLabel->setSceneRect( QRectF( leftMargin, previousItem->rect().bottom() + previousItem->pos().y() + 5, contentWidth, 6 ) );
  residualLabel->setFrameEnabled( false );

  //residual plot
  QgsResidualPlotItem* resPlotItem = new QgsResidualPlotItem( composition );
  composition->addItem( resPlotItem );
  resPlotItem->setSceneRect( QRectF( leftMargin, residualLabel->rect().bottom() + residualLabel->pos().y() + 5, contentWidth, composerMap->rect().height() ) );
  resPlotItem->setExtent( composerMap->extent() );
  resPlotItem->setGCPList( mPoints );

  //necessary for the correct scale bar unit label
  resPlotItem->setConvertScaleToMapUnits( residualUnits == tr( "map units" ) );

  QgsComposerTextTableV2* gcpTable = new QgsComposerTextTableV2( composition, false );
  gcpTable->setHeaderFont( tableHeaderFont );
  gcpTable->setContentFont( tableContentFont );
  gcpTable->setHeaderMode( QgsComposerTableV2::AllFrames );
  QgsComposerTableColumns columns;
  columns << new QgsComposerTableColumn( tr( "ID" ) )
  << new QgsComposerTableColumn( tr( "Enabled" ) )
  << new QgsComposerTableColumn( tr( "Pixel X" ) )
  << new QgsComposerTableColumn( tr( "Pixel Y" ) )
  << new QgsComposerTableColumn( tr( "Map X" ) )
  << new QgsComposerTableColumn( tr( "Map Y" ) )
  << new QgsComposerTableColumn( tr( "Res X (%1)" ).arg( residualUnits ) )
  << new QgsComposerTableColumn( tr( "Res Y (%1)" ).arg( residualUnits ) )
  << new QgsComposerTableColumn( tr( "Res Total (%1)" ).arg( residualUnits ) );

  gcpTable->setColumns( columns );

  QgsGCPList::const_iterator gcpIt = mPoints.constBegin();
  QList< QStringList > gcpTableContents;
  for ( ; gcpIt != mPoints.constEnd(); ++gcpIt )
  {
    QStringList currentGCPStrings;
    QPointF residual = ( *gcpIt )->residual();
    double residualTot = sqrt( residual.x() * residual.x() +  residual.y() * residual.y() );

    currentGCPStrings << QString::number(( *gcpIt )->id() );
    if (( *gcpIt )->isEnabled() )
    {
      currentGCPStrings << tr( "yes" );
    }
    else
    {
      currentGCPStrings << tr( "no" );
    }
    currentGCPStrings << QString::number(( *gcpIt )->pixelCoords().x(), 'f', 0 ) << QString::number(( *gcpIt )->pixelCoords().y(), 'f', 0 ) << QString::number(( *gcpIt )->mapCoords().x(), 'f', 3 )
    <<  QString::number(( *gcpIt )->mapCoords().y(), 'f', 3 ) <<  QString::number( residual.x() ) <<  QString::number( residual.y() ) << QString::number( residualTot );
    gcpTableContents << currentGCPStrings ;
  }

  gcpTable->setContents( gcpTableContents );

  double firstFrameY = resPlotItem->rect().bottom() + resPlotItem->pos().y() + 5;
  double firstFrameHeight = 287 - firstFrameY;
  QgsComposerFrame* gcpFirstFrame = new QgsComposerFrame( composition, gcpTable, leftMargin, firstFrameY, contentWidth, firstFrameHeight );
  gcpTable->addFrame( gcpFirstFrame );
  composition->addItem( gcpFirstFrame );

  QgsComposerFrame* gcpSecondFrame = new QgsComposerFrame( composition, gcpTable, leftMargin, 10, contentWidth, 277.0 );
  gcpSecondFrame->setItemPosition( leftMargin, 10, QgsComposerItem::UpperLeft, 2 );
  gcpSecondFrame->setHidePageIfEmpty( true );
  gcpTable->addFrame( gcpSecondFrame );
  composition->addItem( gcpSecondFrame );

  gcpTable->setGridStrokeWidth( 0.1 );
  gcpTable->setResizeMode( QgsComposerMultiFrame::RepeatUntilFinished );

  composition->exportAsPDF( fileName );

  delete titleLabel;
  delete parameterLabel;
  delete residualLabel;
  delete resPlotItem;
  delete composerMap;
  delete composition;
  return true;
}

void QgsGeorefPluginGui::updateTransformParamLabel()
{
  if ( !mTransformParamLabel )
  {
    return;
  }

  QString transformName = convertTransformEnumToString( mGeorefTransform.transformParametrisation() );
  QString labelString = tr( "Transform: " ) + transformName;

  QgsPoint origin;
  double scaleX, scaleY, rotation;
  if ( mGeorefTransform.getOriginScaleRotation( origin, scaleX, scaleY, rotation ) )
  {
    labelString += " ";
    labelString += tr( "Translation (%1, %2)" ).arg( origin.x() ).arg( origin.y() ); labelString += " ";
    labelString += tr( "Scale (%1, %2)" ).arg( scaleX ).arg( scaleY ); labelString += " ";
    labelString += tr( "Rotation: %1" ).arg( rotation * 180 / M_PI );
  }

  double meanError = 0;
  if ( calculateMeanError( meanError ) )
  {
    labelString += " ";
    labelString += tr( "Mean error: %1" ).arg( meanError );
  }
  mTransformParamLabel->setText( labelString );
}

// Gdal script
void QgsGeorefPluginGui::showGDALScript( const QStringList& commands )
{
  QString script = commands.join( "\n" ) + "\n";

  // create window to show gdal script
  QDialogButtonBox *bbxGdalScript = new QDialogButtonBox( QDialogButtonBox::Cancel, Qt::Horizontal, this );
  QPushButton *pbnCopyInClipBoard = new QPushButton( getThemeIcon( "/mActionEditPaste.png" ),
      tr( "Copy to Clipboard" ), bbxGdalScript );
  bbxGdalScript->addButton( pbnCopyInClipBoard, QDialogButtonBox::AcceptRole );

  QPlainTextEdit *pteScript = new QPlainTextEdit();
  pteScript->setReadOnly( true );
  pteScript->setWordWrapMode( QTextOption::NoWrap );
  pteScript->setPlainText( tr( "%1" ).arg( script ) );

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget( pteScript );
  layout->addWidget( bbxGdalScript );

  QDialog *dlgShowGdalScrip = new QDialog( this );
  dlgShowGdalScrip->setWindowTitle( tr( "GDAL script" ) );
  dlgShowGdalScrip->setLayout( layout );

  connect( bbxGdalScript, SIGNAL( accepted() ), dlgShowGdalScrip, SLOT( accept() ) );
  connect( bbxGdalScript, SIGNAL( rejected() ), dlgShowGdalScrip, SLOT( reject() ) );

  if ( dlgShowGdalScrip->exec() == QDialog::Accepted )
  {
    QApplication::clipboard()->setText( pteScript->toPlainText() );
  }
}

QString QgsGeorefPluginGui::generateGDALtranslateCommand( bool generateTFW )
{
  QStringList gdalCommand;
  gdalCommand << "gdal_translate" << "-of GTiff";
  if ( generateTFW )
  {
    // say gdal generate associated ESRI world file
    gdalCommand << "-co TFW=YES";
  }

  foreach ( QgsGeorefDataPoint *pt, mPoints )
  {
    gdalCommand << QString( "-gcp %1 %2 %3 %4" ).arg( pt->pixelCoords().x() ).arg( -pt->pixelCoords().y() )
    .arg( pt->mapCoords().x() ).arg( pt->mapCoords().y() );
  }

  QFileInfo rasterFileInfo( mRasterFileName );
  mTranslatedRasterFileName = QDir::tempPath() + "/" + rasterFileInfo.fileName();
  gdalCommand << QString( "\"%1\"" ).arg( mRasterFileName ) << QString( "\"%1\"" ).arg( mTranslatedRasterFileName );

  return gdalCommand.join( " " );
}

QString QgsGeorefPluginGui::generateGDALwarpCommand( QString resampling, QString compress,
    bool useZeroForTrans, int order, double targetResX, double targetResY )
{
  QStringList gdalCommand;
  gdalCommand << "gdalwarp" << "-r" << resampling;

  if ( order > 0 && order <= 3 )
  {
    // Let gdalwarp use polynomial warp with the given degree
    gdalCommand << "-order" << QString::number( order );
  }
  else
  {
    // Otherwise, use thin plate spline interpolation
    gdalCommand << "-tps";
  }
  gdalCommand << "-co COMPRESS=" + compress << ( useZeroForTrans ? "-dstalpha" : "" );

  if ( targetResX != 0.0 && targetResY != 0.0 )
  {
    gdalCommand << "-tr" << QString::number( targetResX, 'f' ) << QString::number( targetResY, 'f' );
  }

  gdalCommand << QString( "\"%1\"" ).arg( mTranslatedRasterFileName ) << QString( "\"%1\"" ).arg( mModifiedRasterFileName );

  return gdalCommand.join( " " );
}

// Log
//void QgsGeorefPluginGui::showMessageInLog(const QString &description, const QString &msg)
//{
//  QString logItem = QString("<code>%1: %2</code>").arg(description).arg(msg);
//
//  mLogViewer->appendHtml(logItem);
//}
//
//void QgsGeorefPluginGui::clearLog()
//{
//  mLogViewer->clear();
//}

// Helpers
bool QgsGeorefPluginGui::checkReadyGeoref()
{
  if ( mRasterFileName.isEmpty() )
  {
    mMessageBar->pushMessage( tr( "No Raster Loaded" ), tr( "Please load raster to be georeferenced" ), QgsMessageBar::WARNING, messageTimeout() );
    return false;
  }

  if ( QgsGeorefTransform::InvalidTransform == mTransformParam )
  {
    QMessageBox::information( this, tr( "Info" ), tr( "Please set transformation type" ) );
    getTransformSettings();
    return false;
  }

  //MH: helmert transformation without warping disabled until qgis is able to read rotated rasters efficiently
  if ( mModifiedRasterFileName.isEmpty() && QgsGeorefTransform::Linear != mTransformParam /*&& QgsGeorefTransform::Helmert != mTransformParam*/ )
  {
    QMessageBox::information( this, tr( "Info" ), tr( "Please set output raster name" ) );
    getTransformSettings();
    return false;
  }

  if ( mPoints.count() < ( int )mGeorefTransform.getMinimumGCPCount() )
  {
    mMessageBar->pushMessage( tr( "Not Enough GCPs" ), tr( "%1 transformation requires at least %2 GCPs. Please define more." )
                              .arg( convertTransformEnumToString( mTransformParam ) ).arg( mGeorefTransform.getMinimumGCPCount() )
                              , QgsMessageBar::WARNING, messageTimeout() );
    return false;
  }

  // Update the transform if necessary
  if ( !updateGeorefTransform() )
  {
    mMessageBar->pushMessage( tr( "Transform Failed" ), tr( "Failed to compute GCP transform: Transform is not solvable." ), QgsMessageBar::WARNING, messageTimeout() );
    //    logRequaredGCPs();
    return false;
  }

  return true;
}

bool QgsGeorefPluginGui::updateGeorefTransform()
{
  std::vector<QgsPoint> mapCoords, pixelCoords;
  if ( mGCPListWidget->gcpList() )
    mGCPListWidget->gcpList()->createGCPVectors( mapCoords, pixelCoords );
  else
    return false;

  // Parametrize the transform with GCPs
  if ( !mGeorefTransform.updateParametersFromGCPs( mapCoords, pixelCoords ) )
  {
    return false;
  }

  mGCPsDirty = false;
  updateTransformParamLabel();
  return true;
}

// Samples the given rectangle at numSamples per edge.
// Returns an axis aligned bounding box which contains the transformed samples.
QgsRectangle QgsGeorefPluginGui::transformViewportBoundingBox( const QgsRectangle &canvasExtent,
    QgsGeorefTransform &t,
    bool rasterToWorld, uint numSamples )
{
  double minX, minY;
  double maxX, maxY;
  minX = minY =  std::numeric_limits<double>::max();
  maxX = maxY = -std::numeric_limits<double>::max();

  double oX = canvasExtent.xMinimum();
  double oY = canvasExtent.yMinimum();
  double dX = canvasExtent.xMaximum();
  double dY = canvasExtent.yMaximum();
  double stepX = numSamples ? ( dX - oX ) / ( numSamples - 1 ) : 0.0;
  double stepY = numSamples ? ( dY - oY ) / ( numSamples - 1 ) : 0.0;
  for ( uint s = 0u;  s < numSamples; s++ )
  {
    for ( uint edge = 0; edge < 4; edge++ )
    {
      QgsPoint src, raster;
      switch ( edge )
      {
        case 0:
          src = QgsPoint( oX + ( double )s * stepX, oY );
          break;
        case 1:
          src = QgsPoint( oX + ( double )s * stepX, dY );
          break;
        case 2:
          src = QgsPoint( oX, oY + ( double )s * stepY );
          break;
        case 3:
          src = QgsPoint( dX, oY + ( double )s * stepY );
          break;
      }
      t.transform( src, raster, rasterToWorld );
      minX = qMin( raster.x(), minX );
      maxX = qMax( raster.x(), maxX );
      minY = qMin( raster.y(), minY );
      maxY = qMax( raster.y(), maxY );
    }
  }
  return QgsRectangle( minX, minY, maxX, maxY );
}

QString QgsGeorefPluginGui::convertTransformEnumToString( QgsGeorefTransform::TransformParametrisation transform )
{
  switch ( transform )
  {
    case QgsGeorefTransform::Linear:
      return tr( "Linear" );
    case QgsGeorefTransform::Helmert:
      return tr( "Helmert" );
    case QgsGeorefTransform::PolynomialOrder1:
      return tr( "Polynomial 1" );
    case QgsGeorefTransform::PolynomialOrder2:
      return tr( "Polynomial 2" );
    case QgsGeorefTransform::PolynomialOrder3:
      return tr( "Polynomial 3" );
    case QgsGeorefTransform::ThinPlateSpline:
      return tr( "Thin plate spline (TPS)" );
    case QgsGeorefTransform::Projective:
      return tr( "Projective" );
    default:
      return tr( "Not set" );
  }
}

QString QgsGeorefPluginGui::convertResamplingEnumToString( QgsImageWarper::ResamplingMethod resampling )
{
  switch ( resampling )
  {
    case QgsImageWarper::NearestNeighbour:
      return "near";
    case QgsImageWarper::Bilinear:
      return "bilinear";
    case QgsImageWarper::Cubic:
      return "cubic";
    case QgsImageWarper::CubicSpline:
      return "cubicspline";
    case QgsImageWarper::Lanczos:
      return "lanczos";
  }
  return "";
}

int QgsGeorefPluginGui::polynomialOrder( QgsGeorefTransform::TransformParametrisation transform )
{
  switch ( transform )
  {
    case QgsGeorefTransform::PolynomialOrder1:
      return 1;
    case QgsGeorefTransform::PolynomialOrder2:
      return 2;
    case QgsGeorefTransform::PolynomialOrder3:
      return 3;
    case QgsGeorefTransform::ThinPlateSpline:
      return -1;

    default:
      return 0;
  }
}

QString QgsGeorefPluginGui::guessWorldFileName( const QString &rasterFileName )
{
  QString worldFileName = "";
  int point = rasterFileName.lastIndexOf( '.' );
  if ( point != -1 && point != rasterFileName.length() - 1 )
    worldFileName = rasterFileName.left( point + 1 ) + "wld";

  return worldFileName;
}

// Note this code is duplicated from qgisapp.cpp because
// I didnt want to make plugins on qgsapplication [TS]
QIcon QgsGeorefPluginGui::getThemeIcon( const QString &theName )
{
  if ( QFile::exists( QgsApplication::activeThemePath() + theName ) )
  {
    return QIcon( QgsApplication::activeThemePath() + theName );
  }
  else if ( QFile::exists( QgsApplication::defaultThemePath() + theName ) )
  {
    return QIcon( QgsApplication::defaultThemePath() + theName );
  }
  else
  {
    QSettings settings;
    QString themePath = ":/icons/" + settings.value( "/Themes" ).toString() + theName;
    if ( QFile::exists( themePath ) )
    {
      return QIcon( themePath );
    }
    else
    {
      return QIcon( ":/icons/default" + theName );
    }
  }
}

bool QgsGeorefPluginGui::checkFileExisting( QString fileName, QString title, QString question )
{
  if ( !fileName.isEmpty() )
  {
    if ( QFile::exists( fileName ) )
    {
      int r = QMessageBox::question( this, title, question,
                                     QMessageBox::Yes | QMessageBox::Default,
                                     QMessageBox::No | QMessageBox::Escape );
      if ( r == QMessageBox::No )
        return false;
      else
        QFile::remove( fileName );
    }
  }

  return true;
}

bool QgsGeorefPluginGui::equalGCPlists( const QgsGCPList &list1, const QgsGCPList &list2 )
{
  if ( list1.count() != list2.count() )
    return false;

  int count = list1.count();
  int j = 0;
  for ( int i = 0; i < count; ++i, ++j )
  {
    QgsGeorefDataPoint *p1 = list1.at( i );
    QgsGeorefDataPoint *p2 = list2.at( j );
    if ( p1->pixelCoords() != p2->pixelCoords() )
      return false;

    if ( p1->mapCoords() != p2->mapCoords() )
      return false;
  }

  return true;
}

//void QgsGeorefPluginGui::logTransformOptions()
//{
//  showMessageInLog(tr("Interpolation"), convertResamplingEnumToString(mResamplingMethod));
//  showMessageInLog(tr("Compression method"), mCompressionMethod);
//  showMessageInLog(tr("Zero for transparency"), mUseZeroForTrans ? "true" : "false");
//}
//
//void QgsGeorefPluginGui::logRequaredGCPs()
//{
//  if (mGeorefTransform.getMinimumGCPCount() != 0)
//  {
//    if ((uint)mPoints.size() >= mGeorefTransform.getMinimumGCPCount())
//      showMessageInLog(tr("Info"), tr("For georeferencing requared at least %1 GCP points")
//                       .arg(mGeorefTransform.getMinimumGCPCount()));
//    else
//      showMessageInLog(tr("Critical"), tr("For georeferencing requared at least %1 GCP points")
//                       .arg(mGeorefTransform.getMinimumGCPCount()));
//  }
//}

void QgsGeorefPluginGui::clearGCPData()
{
  //force all list widget editors to close before removing data points
  //otherwise the editors try to update deleted data points when they close
  mGCPListWidget->closeEditors();

  qDeleteAll( mPoints );
  mPoints.clear();
  mGCPListWidget->updateGCPList();

  mIface->mapCanvas()->refresh();
}

int QgsGeorefPluginGui::messageTimeout()
{
  QSettings settings;
  return settings.value( "/qgis/messageTimeout", 5 ).toInt();
}
