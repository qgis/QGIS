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
#include <QTextStream>
#include <QPen>
#include <QStringList>
#include <QList>

#include "qgssettings.h"
#include "qgisinterface.h"
#include "qgsapplication.h"

#include "qgslayout.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitemtexttable.h"
#include "qgslayouttablecolumn.h"
#include "qgslayoutframe.h"
#include "qgslayoutpagecollection.h"
#include "qgsmapcanvas.h"
#include "qgsmapcoordsdialog.h"
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

QgsGeorefDockWidget::QgsGeorefDockWidget( const QString &title, QWidget *parent, Qt::WindowFlags flags )
  : QgsDockWidget( title, parent, flags )
{
  setObjectName( QStringLiteral( "GeorefDockWidget" ) ); // set object name so the position can be saved
}

QgsGeorefPluginGui::QgsGeorefPluginGui( QgisInterface *qgisInterface, QWidget *parent, Qt::WindowFlags fl )
  : QMainWindow( parent, fl )
  , mMousePrecisionDecimalPlaces( 0 )
  , mTransformParam( QgsGeorefTransform::InvalidTransform )
  , mIface( qgisInterface )
  , mAgainAddRaster( false )
  , mMapCoordsDialog( nullptr )
  , mUseZeroForTrans( false )
  , mLoadInQgis( false )
{
  setupUi( this );

  QgsSettings s;
  restoreGeometry( s.value( QStringLiteral( "/Plugin-GeoReferencer/Window/geometry" ) ).toByteArray() );

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

  connect( mIface, &QgisInterface::currentThemeChanged, this, &QgsGeorefPluginGui::updateIconTheme );

  if ( s.value( QStringLiteral( "/Plugin-GeoReferencer/Config/ShowDocked" ) ).toBool() )
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
    mDock->setWidget( nullptr );
    delete mDock;
    mDock = nullptr;
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
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Plugin-GeoReferencer/Window/geometry" ), saveGeometry() );

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
      mRasterFileName.clear();
      e->accept();
      return;
    case QgsGeorefPluginGui::GCPSILENTSAVE:
      if ( !mGCPpointsFileName.isEmpty() )
        saveGCPs();
      clearGCPData();
      removeOldLayer();
      mRasterFileName.clear();
      return;
    case QgsGeorefPluginGui::GCPDISCARD:
      writeSettings();
      clearGCPData();
      removeOldLayer();
      mRasterFileName.clear();
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

  QgsSettings s;
  QString dir = s.value( QStringLiteral( "/Plugin-GeoReferencer/rasterdirectory" ) ).toString();
  if ( dir.isEmpty() )
    dir = '.';

  QString otherFiles = tr( "All other files (*)" );
  QString lastUsedFilter = s.value( QStringLiteral( "/Plugin-GeoReferencer/lastusedfilter" ), otherFiles ).toString();

  QString filters = QgsProviderRegistry::instance()->fileRasterFilters();
  filters.prepend( otherFiles + ";;" );
  filters.chop( otherFiles.size() + 2 );
  mRasterFileName = QFileDialog::getOpenFileName( this, tr( "Open Raster" ), dir, filters, &lastUsedFilter );
  mModifiedRasterFileName.clear();

  if ( mRasterFileName.isEmpty() )
    return;

  QString errMsg;
  if ( !QgsRasterLayer::isValidRasterFileName( mRasterFileName, errMsg ) )
  {
    QString msg = tr( "%1 is not a supported raster data source." ).arg( mRasterFileName );

    if ( !errMsg.isEmpty() )
      msg += '\n' + errMsg;

    QMessageBox::information( this, tr( "Open Raster" ), msg );
    return;
  }

  QFileInfo fileInfo( mRasterFileName );
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/rasterdirectory" ), fileInfo.path() );
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/lastusedfilter" ), lastUsedFilter );

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
    mMessageBar->pushMessage( tr( "Georeference Successful" ), tr( "Raster was successfully georeferenced." ), Qgis::Info, messageTimeout() );
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
    FALLTHROUGH
    default:
      mMessageBar->pushMessage( tr( "Invalid Transform" ), tr( "GDAL scripting is not supported for %1 transformation." )
                                .arg( convertTransformEnumToString( mTransformParam ) )
                                , Qgis::Warning, messageTimeout() );
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
void QgsGeorefPluginGui::addPoint( const QgsPointXY &pixelCoords, const QgsPointXY &mapCoords,
                                   bool enable, bool finalize )
{
  QgsGeorefDataPoint *pnt = new QgsGeorefDataPoint( mCanvas, mIface->mapCanvas(),
      pixelCoords, mapCoords, enable );
  mPoints.append( pnt );
  mGCPsDirty = true;
  if ( finalize )
  {
    mGCPListWidget->setGCPList( &mPoints );
    mCanvas->refresh();
    mIface->mapCanvas()->refresh();
  }

  connect( mCanvas, &QgsMapCanvas::extentsChanged, pnt, &QgsGeorefDataPoint::updateCoords );
  if ( finalize )
  {
    updateGeorefTransform();
  }
}

void QgsGeorefPluginGui::deleteDataPoint( QPoint coords )
{
  for ( QgsGCPList::iterator it = mPoints.begin(); it != mPoints.end(); ++it )
  {
    QgsGeorefDataPoint *pt = *it;
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

void QgsGeorefPluginGui::selectPoint( QPoint p )
{
  // Get Map Sender
  bool isMapPlugin = sender() == mToolMovePoint;
  QgsGeorefDataPoint *&mvPoint = isMapPlugin ? mMovingPoint : mMovingPointQgis;

  for ( QgsGCPList::const_iterator it = mPoints.constBegin(); it != mPoints.constEnd(); ++it )
  {
    if ( ( *it )->contains( p, isMapPlugin ) )
    {
      mvPoint = *it;
      break;
    }
  }
}

void QgsGeorefPluginGui::movePoint( QPoint p )
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

void QgsGeorefPluginGui::releasePoint( QPoint p )
{
  Q_UNUSED( p );
  // Get Map Sender
  if ( sender() == mToolMovePoint )
  {
    mMovingPoint = nullptr;
  }
  else
  {
    mMovingPointQgis = nullptr;
  }
}

void QgsGeorefPluginGui::showCoordDialog( const QgsPointXY &pixelCoords )
{
  if ( mLayer && !mMapCoordsDialog )
  {
    mMapCoordsDialog = new QgsMapCoordsDialog( mIface->mapCanvas(), pixelCoords, this );
    connect( mMapCoordsDialog, &QgsMapCoordsDialog::pointAdded,
    [this]( const QgsPointXY & a, const QgsPointXY & b ) { this->addPoint( a, b ); }
           );
    mMapCoordsDialog->show();
  }
}

void QgsGeorefPluginGui::loadGCPsDialog()
{
  QString selectedFile = mRasterFileName.isEmpty() ? QString() : mRasterFileName + ".points";
  mGCPpointsFileName = QFileDialog::getOpenFileName( this, tr( "Load GCP Points" ),
                       selectedFile, tr( "GCP file" ) + " (*.points)" );
  if ( mGCPpointsFileName.isEmpty() )
    return;

  if ( !loadGCPs() )
  {
    mMessageBar->pushMessage( tr( "Load GCP Points" ), tr( "Invalid GCP file. File could not be read." ), Qgis::Warning, messageTimeout() );
  }
  else
  {
    mMessageBar->pushMessage( tr( "Load GCP Points" ), tr( "GCP file successfully loaded." ), Qgis::Info, messageTimeout() );
  }
}

void QgsGeorefPluginGui::saveGCPsDialog()
{
  if ( mPoints.isEmpty() )
  {
    mMessageBar->pushMessage( tr( "Save GCP Points" ), tr( "No GCP points are available to save." ), Qgis::Warning, messageTimeout() );
    return;
  }

  QString selectedFile = mRasterFileName.isEmpty() ? QString() : mRasterFileName + ".points";
  mGCPpointsFileName = QFileDialog::getSaveFileName( this, tr( "Save GCP Points" ),
                       selectedFile,
                       tr( "GCP file" ) + " (*.points)" );

  if ( mGCPpointsFileName.isEmpty() )
    return;

  if ( mGCPpointsFileName.right( 7 ) != QLatin1String( ".points" ) )
    mGCPpointsFileName += QLatin1String( ".points" );

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
    mMessageBar->pushMessage( tr( "Raster Properties" ), tr( "Please load raster to be georeferenced." ), Qgis::Info, messageTimeout() );
  }
}

void QgsGeorefPluginGui::showGeorefConfigDialog()
{
  QgsGeorefConfigDialog config;
  if ( config.exec() == QDialog::Accepted )
  {
    mCanvas->refresh();
    mIface->mapCanvas()->refresh();
    QgsSettings s;
    //update dock state
    bool dock = s.value( QStringLiteral( "/Plugin-GeoReferencer/Config/ShowDocked" ) ).toBool();
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

  mLayer->setContrastEnhancement( QgsContrastEnhancement::StretchToMinimumMaximum, QgsRasterMinMaxOrigin::MinMax, rectangle );
  mCanvas->refresh();
}

// Info slots
void QgsGeorefPluginGui::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "plugins/plugins_georeferencer.html#defining-the-transformation-settings" ) );
}

// Comfort slots
void QgsGeorefPluginGui::jumpToGCP( uint theGCPIndex )
{
  if ( ( int )theGCPIndex >= mPoints.size() )
  {
    return;
  }

  // qgsmapcanvas doesn't seem to have a method for recentering the map
  QgsRectangle ext = mCanvas->extent();

  QgsPointXY center = ext.center();
  QgsPointXY new_center = mPoints[theGCPIndex]->pixelCoords();

  QgsPointXY diff( new_center.x() - center.x(), new_center.y() - center.y() );
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
void QgsGeorefPluginGui::showMouseCoords( const QgsPointXY &p )
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
  bool automatic = QgsProject::instance()->readBoolEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/Automatic" ) );
  int dp = 0;

  if ( automatic )
  {
    // Work out a suitable number of decimal places for the mouse
    // coordinates with the aim of always having enough decimal places
    // to show the difference in position between adjacent pixels.
    // Also avoid taking the log of 0.
    if ( mCanvas->mapUnitsPerPixel() != 0.0 )
      dp = static_cast<int>( std::ceil( -1.0 * std::log10( mCanvas->mapUnitsPerPixel() ) ) );
  }
  else
    dp = QgsProject::instance()->readNumEntry( QStringLiteral( "PositionPrecision" ), QStringLiteral( "/DecimalPlaces" ) );

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
      mLayer = nullptr;
      mAgainAddRaster = false;
    }
  }
}

// Registry layer QGis
void QgsGeorefPluginGui::layerWillBeRemoved( const QString &layerId )
{
  mAgainAddRaster = mLayer && mLayer->id().compare( layerId ) == 0;
}

// ------------------------------ private ---------------------------------- //
// Gui
void QgsGeorefPluginGui::createActions()
{
  // File actions
  connect( mActionReset, &QAction::triggered, this, &QgsGeorefPluginGui::reset );

  mActionOpenRaster->setIcon( getThemeIcon( QStringLiteral( "/mActionAddRasterLayer.svg" ) ) );
  connect( mActionOpenRaster, &QAction::triggered, this, &QgsGeorefPluginGui::openRaster );

  mActionStartGeoref->setIcon( getThemeIcon( QStringLiteral( "/mActionStartGeoref.png" ) ) );
  connect( mActionStartGeoref, &QAction::triggered, this, &QgsGeorefPluginGui::doGeoreference );

  mActionGDALScript->setIcon( getThemeIcon( QStringLiteral( "/mActionGDALScript.png" ) ) );
  connect( mActionGDALScript, &QAction::triggered, this, &QgsGeorefPluginGui::generateGDALScript );

  mActionLoadGCPpoints->setIcon( getThemeIcon( QStringLiteral( "/mActionLoadGCPpoints.png" ) ) );
  connect( mActionLoadGCPpoints, &QAction::triggered, this, &QgsGeorefPluginGui::loadGCPsDialog );

  mActionSaveGCPpoints->setIcon( getThemeIcon( QStringLiteral( "/mActionSaveGCPpointsAs.png" ) ) );
  connect( mActionSaveGCPpoints, &QAction::triggered, this, &QgsGeorefPluginGui::saveGCPsDialog );

  mActionTransformSettings->setIcon( getThemeIcon( QStringLiteral( "/mActionTransformSettings.png" ) ) );
  connect( mActionTransformSettings, &QAction::triggered, this, &QgsGeorefPluginGui::getTransformSettings );

  // Edit actions
  mActionAddPoint->setIcon( getThemeIcon( QStringLiteral( "/mActionAddGCPPoint.png" ) ) );
  connect( mActionAddPoint, &QAction::triggered, this, &QgsGeorefPluginGui::setAddPointTool );

  mActionDeletePoint->setIcon( getThemeIcon( QStringLiteral( "/mActionDeleteGCPPoint.png" ) ) );
  connect( mActionDeletePoint, &QAction::triggered, this, &QgsGeorefPluginGui::setDeletePointTool );

  mActionMoveGCPPoint->setIcon( getThemeIcon( QStringLiteral( "/mActionMoveGCPPoint.png" ) ) );
  connect( mActionMoveGCPPoint, &QAction::triggered, this, &QgsGeorefPluginGui::setMovePointTool );

  // View actions
  mActionPan->setIcon( getThemeIcon( QStringLiteral( "/mActionPan.svg" ) ) );
  connect( mActionPan, &QAction::triggered, this, &QgsGeorefPluginGui::setPanTool );

  mActionZoomIn->setIcon( getThemeIcon( QStringLiteral( "/mActionZoomIn.svg" ) ) );
  connect( mActionZoomIn, &QAction::triggered, this, &QgsGeorefPluginGui::setZoomInTool );

  mActionZoomOut->setIcon( getThemeIcon( QStringLiteral( "/mActionZoomOut.svg" ) ) );
  connect( mActionZoomOut, &QAction::triggered, this, &QgsGeorefPluginGui::setZoomOutTool );

  mActionZoomToLayer->setIcon( getThemeIcon( QStringLiteral( "/mActionZoomToLayer.svg" ) ) );
  connect( mActionZoomToLayer, &QAction::triggered, this, &QgsGeorefPluginGui::zoomToLayerTool );

  mActionZoomLast->setIcon( getThemeIcon( QStringLiteral( "/mActionZoomLast.svg" ) ) );
  connect( mActionZoomLast, &QAction::triggered, this, &QgsGeorefPluginGui::zoomToLast );

  mActionZoomNext->setIcon( getThemeIcon( QStringLiteral( "/mActionZoomNext.svg" ) ) );
  connect( mActionZoomNext, &QAction::triggered, this, &QgsGeorefPluginGui::zoomToNext );

  mActionLinkGeorefToQGis->setIcon( getThemeIcon( QStringLiteral( "/mActionLinkGeorefToQGis.png" ) ) );
  connect( mActionLinkGeorefToQGis, &QAction::triggered, this, &QgsGeorefPluginGui::linkGeorefToQGis );

  mActionLinkQGisToGeoref->setIcon( getThemeIcon( QStringLiteral( "/mActionLinkQGisToGeoref.png" ) ) );
  connect( mActionLinkQGisToGeoref, &QAction::triggered, this, &QgsGeorefPluginGui::linkQGisToGeoref );

  // Settings actions
  mActionRasterProperties->setIcon( getThemeIcon( QStringLiteral( "/mActionRasterProperties.png" ) ) );
  connect( mActionRasterProperties, &QAction::triggered, this, &QgsGeorefPluginGui::showRasterPropertiesDialog );

  mActionGeorefConfig->setIcon( getThemeIcon( QStringLiteral( "/mActionGeorefConfig.png" ) ) );
  connect( mActionGeorefConfig, &QAction::triggered, this, &QgsGeorefPluginGui::showGeorefConfigDialog );

  // Histogram stretch
  mActionLocalHistogramStretch->setIcon( getThemeIcon( QStringLiteral( "/mActionLocalHistogramStretch.svg" ) ) );
  connect( mActionLocalHistogramStretch, &QAction::triggered, this, &QgsGeorefPluginGui::localHistogramStretch );
  mActionLocalHistogramStretch->setEnabled( false );

  mActionFullHistogramStretch->setIcon( getThemeIcon( QStringLiteral( "/mActionFullHistogramStretch.svg" ) ) );
  connect( mActionFullHistogramStretch, &QAction::triggered, this, &QgsGeorefPluginGui::fullHistogramStretch );
  mActionFullHistogramStretch->setEnabled( false );

  // Help actions
  mActionHelp = new QAction( tr( "Help" ), this );
  connect( mActionHelp, &QAction::triggered, this, &QgsGeorefPluginGui::showHelp );

  mActionQuit->setIcon( getThemeIcon( QStringLiteral( "/mActionQuit.png" ) ) );
  mActionQuit->setShortcuts( QList<QKeySequence>() << QKeySequence( Qt::CTRL + Qt::Key_Q )
                             << QKeySequence( Qt::Key_Escape ) );
  connect( mActionQuit, &QAction::triggered, this, &QWidget::close );
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
  mCanvas = new QgsMapCanvas( this->centralWidget() );
  mCanvas->setObjectName( QStringLiteral( "georefCanvas" ) );
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
  connect( mToolAddPoint, &QgsGeorefToolAddPoint::showCoordDialog,
           this, &QgsGeorefPluginGui::showCoordDialog );

  mToolDeletePoint = new QgsGeorefToolDeletePoint( mCanvas );
  mToolDeletePoint->setAction( mActionDeletePoint );
  connect( mToolDeletePoint, &QgsGeorefToolDeletePoint::deleteDataPoint,
           this, static_cast<void ( QgsGeorefPluginGui::* )( QPoint )>( &QgsGeorefPluginGui::deleteDataPoint ) );

  mToolMovePoint = new QgsGeorefToolMovePoint( mCanvas );
  mToolMovePoint->setAction( mActionMoveGCPPoint );
  connect( mToolMovePoint, &QgsGeorefToolMovePoint::pointPressed,
           this, &QgsGeorefPluginGui::selectPoint );
  connect( mToolMovePoint, &QgsGeorefToolMovePoint::pointMoved,
           this, &QgsGeorefPluginGui::movePoint );
  connect( mToolMovePoint, &QgsGeorefToolMovePoint::pointReleased,
           this, &QgsGeorefPluginGui::releasePoint );

  // Point in Qgis Map
  mToolMovePointQgis = new QgsGeorefToolMovePoint( mIface->mapCanvas() );
  mToolMovePointQgis->setAction( mActionMoveGCPPoint );
  connect( mToolMovePointQgis, &QgsGeorefToolMovePoint::pointPressed,
           this, &QgsGeorefPluginGui::selectPoint );
  connect( mToolMovePointQgis, &QgsGeorefToolMovePoint::pointMoved,
           this, &QgsGeorefPluginGui::movePoint );
  connect( mToolMovePointQgis, &QgsGeorefToolMovePoint::pointReleased,
           this, &QgsGeorefPluginGui::releasePoint );

  QgsSettings s;
  double zoomFactor = s.value( QStringLiteral( "/qgis/zoom_factor" ), 2 ).toDouble();
  mCanvas->setWheelFactor( zoomFactor );

  mExtentsChangedRecursionGuard = false;

  mGeorefTransform.selectTransformParametrisation( QgsGeorefTransform::Linear );
  mGCPsDirty = true;

  // Connect main canvas and georef canvas signals so we are aware if any of the viewports change
  // (used by the map follow mode)
  connect( mCanvas, &QgsMapCanvas::extentsChanged, this, &QgsGeorefPluginGui::extentsChangedGeorefCanvas );
  connect( mIface->mapCanvas(), &QgsMapCanvas::extentsChanged, this, &QgsGeorefPluginGui::extentsChangedQGisCanvas );
}

void QgsGeorefPluginGui::createMenus()
{
  // Get platform for menu layout customization (Gnome, Kde, Mac, Win)
  QDialogButtonBox::ButtonLayout layout =
    QDialogButtonBox::ButtonLayout( style()->styleHint( QStyle::SH_DialogButtonLayout, nullptr, this ) );

  mPanelMenu = new QMenu( tr( "Panels" ) );
  mPanelMenu->setObjectName( QStringLiteral( "mPanelMenu" ) );
  mPanelMenu->addAction( dockWidgetGCPpoints->toggleViewAction() );
  //  mPanelMenu->addAction(dockWidgetLogView->toggleViewAction());

  mToolbarMenu = new QMenu( tr( "Toolbars" ) );
  mToolbarMenu->setObjectName( QStringLiteral( "mToolbarMenu" ) );
  mToolbarMenu->addAction( toolBarFile->toggleViewAction() );
  mToolbarMenu->addAction( toolBarEdit->toggleViewAction() );
  mToolbarMenu->addAction( toolBarView->toggleViewAction() );

  toolBarFile->setIconSize( mIface->iconSize() );
  toolBarEdit->setIconSize( mIface->iconSize() );
  toolBarView->setIconSize( mIface->iconSize() );
  toolBarHistogramStretch->setIconSize( mIface->iconSize() );

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

  connect( mGCPListWidget, &QgsGCPListWidget::jumpToGCP, this, &QgsGeorefPluginGui::jumpToGCP );
#if 0
  connect( mGCPListWidget, SIGNAL( replaceDataPoint( QgsGeorefDataPoint *, int ) ),
           this, SLOT( replaceDataPoint( QgsGeorefDataPoint *, int ) ) );
#endif
  connect( mGCPListWidget, static_cast<void ( QgsGCPListWidget::* )( int )>( &QgsGCPListWidget::deleteDataPoint ),
           this, static_cast<void ( QgsGeorefPluginGui::* )( int )>( &QgsGeorefPluginGui::deleteDataPoint ) );
  connect( mGCPListWidget, &QgsGCPListWidget::pointEnabled, this, &QgsGeorefPluginGui::updateGeorefTransform );
}

QLabel *QgsGeorefPluginGui::createBaseLabelStatus()
{
  QFont myFont( QStringLiteral( "Arial" ), 9 );
  QLabel *label = new QLabel( statusBar() );
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
  mEPSG->setText( QStringLiteral( "EPSG:" ) );
  statusBar()->addPermanentWidget( mEPSG, 0 );
}

void QgsGeorefPluginGui::setupConnections()
{
  connect( mCanvas, &QgsMapCanvas::xyCoordinates, this, &QgsGeorefPluginGui::showMouseCoords );
  connect( mCanvas, &QgsMapCanvas::scaleChanged, this, &QgsGeorefPluginGui::updateMouseCoordinatePrecision );

  // Connect status from ZoomLast/ZoomNext to corresponding action
  connect( mCanvas, &QgsMapCanvas::zoomLastStatusChanged, mActionZoomLast, &QAction::setEnabled );
  connect( mCanvas, &QgsMapCanvas::zoomNextStatusChanged, mActionZoomNext, &QAction::setEnabled );
  // Connect when one Layer is removed - Case where change the Projetct in QGIS
  connect( QgsProject::instance(), static_cast<void ( QgsProject::* )( const QString & )>( &QgsProject::layerWillBeRemoved ), this, &QgsGeorefPluginGui::layerWillBeRemoved );

  // Connect extents changed - Use for need add again Raster
  connect( mCanvas, &QgsMapCanvas::extentsChanged, this, &QgsGeorefPluginGui::extentsChanged );
}

void QgsGeorefPluginGui::removeOldLayer()
{
  // delete layer (and don't signal it as it's our private layer)
  if ( mLayer )
  {
    QgsProject::instance()->removeMapLayers(
      ( QStringList() << mLayer->id() ) );
    mLayer = nullptr;
  }
  mCanvas->refresh();
}

void QgsGeorefPluginGui::updateIconTheme( const QString &theme )
{
  Q_UNUSED( theme );
  // File actions
  mActionOpenRaster->setIcon( getThemeIcon( QStringLiteral( "/mActionAddRasterLayer.svg" ) ) );
  mActionStartGeoref->setIcon( getThemeIcon( QStringLiteral( "/mActionStartGeoref.png" ) ) );
  mActionGDALScript->setIcon( getThemeIcon( QStringLiteral( "/mActionGDALScript.png" ) ) );
  mActionLoadGCPpoints->setIcon( getThemeIcon( QStringLiteral( "/mActionLoadGCPpoints.png" ) ) );
  mActionSaveGCPpoints->setIcon( getThemeIcon( QStringLiteral( "/mActionSaveGCPpointsAs.png" ) ) );
  mActionTransformSettings->setIcon( getThemeIcon( QStringLiteral( "/mActionTransformSettings.png" ) ) );

  // Edit actions
  mActionAddPoint->setIcon( getThemeIcon( QStringLiteral( "/mActionAddGCPPoint.png" ) ) );
  mActionDeletePoint->setIcon( getThemeIcon( QStringLiteral( "/mActionDeleteGCPPoint.png" ) ) );
  mActionMoveGCPPoint->setIcon( getThemeIcon( QStringLiteral( "/mActionMoveGCPPoint.png" ) ) );

  // View actions
  mActionPan->setIcon( getThemeIcon( QStringLiteral( "/mActionPan.svg" ) ) );
  mActionZoomIn->setIcon( getThemeIcon( QStringLiteral( "/mActionZoomIn.svg" ) ) );
  mActionZoomOut->setIcon( getThemeIcon( QStringLiteral( "/mActionZoomOut.svg" ) ) );
  mActionZoomToLayer->setIcon( getThemeIcon( QStringLiteral( "/mActionZoomToLayer.svg" ) ) );
  mActionZoomLast->setIcon( getThemeIcon( QStringLiteral( "/mActionZoomLast.svg" ) ) );
  mActionZoomNext->setIcon( getThemeIcon( QStringLiteral( "/mActionZoomNext.svg" ) ) );
  mActionLinkGeorefToQGis->setIcon( getThemeIcon( QStringLiteral( "/mActionLinkGeorefToQGis.png" ) ) );
  mActionLinkQGisToGeoref->setIcon( getThemeIcon( QStringLiteral( "/mActionLinkQGisToGeoref.png" ) ) );

  // Settings actions
  mActionRasterProperties->setIcon( getThemeIcon( QStringLiteral( "/mActionRasterProperties.png" ) ) );
  mActionGeorefConfig->setIcon( getThemeIcon( QStringLiteral( "/mActionGeorefConfig.png" ) ) );

  mActionQuit->setIcon( getThemeIcon( QStringLiteral( "/mActionQuit.png" ) ) );
}

// Mapcanvas Plugin
void QgsGeorefPluginGui::addRaster( const QString &file )
{
  mLayer = new QgsRasterLayer( file, QStringLiteral( "Raster" ) );

  // so layer is not added to legend
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mLayer, false, false );

  // add layer to map canvas
  mCanvas->setLayers( QList<QgsMapLayer *>() << mLayer );

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
  QgsSettings s;
  QRect georefRect = QApplication::desktop()->screenGeometry( mIface->mainWindow() );
  resize( s.value( QStringLiteral( "/Plugin-GeoReferencer/size" ), QSize( georefRect.width() / 2 + georefRect.width() / 5,
                   mIface->mainWindow()->height() ) ).toSize() );
  move( s.value( QStringLiteral( "/Plugin-GeoReferencer/pos" ), QPoint( parentWidget()->width() / 2 - width() / 2, 0 ) ).toPoint() );
  restoreState( s.value( QStringLiteral( "/Plugin-GeoReferencer/uistate" ) ).toByteArray() );

  // warp options
  mResamplingMethod = ( QgsImageWarper::ResamplingMethod )s.value( QStringLiteral( "/Plugin-GeoReferencer/resamplingmethod" ),
                      QgsImageWarper::NearestNeighbour ).toInt();
  mCompressionMethod = s.value( QStringLiteral( "/Plugin-GeoReferencer/compressionmethod" ), "NONE" ).toString();
  mUseZeroForTrans = s.value( QStringLiteral( "/Plugin-GeoReferencer/usezerofortrans" ), false ).toBool();
}

void QgsGeorefPluginGui::writeSettings()
{
  QgsSettings s;
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/pos" ), pos() );
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/size" ), size() );
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/uistate" ), saveState() );

  // warp options
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/transformparam" ), mTransformParam );
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/resamplingmethod" ), mResamplingMethod );
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/compressionmethod" ), mCompressionMethod );
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/usezerofortrans" ), mUseZeroForTrans );
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
    if ( line.contains( ',' ) ) // in previous format "\t" is delimiter of points in new - ","
    {
      // points from new georeferencer
      ls = line.split( ',' );
    }
    else
    {
      // points from prev georeferencer
      ls = line.split( '\t' );
    }

    if ( ls.count() < 4 )
    {
      return false;
    }

    QgsPointXY mapCoords( ls.at( 0 ).toDouble(), ls.at( 1 ).toDouble() ); // map x,y
    QgsPointXY pixelCoords( ls.at( 2 ).toDouble(), ls.at( 3 ).toDouble() ); // pixel x,y
    if ( ls.count() == 5 )
    {
      bool enable = ls.at( 4 ).toInt();
      addPoint( pixelCoords, mapCoords, enable, false );
    }
    else
      addPoint( pixelCoords, mapCoords, true, false );

    ++i;
  }

  mInitialPoints = mPoints;
  //    showMessageInLog(tr("GCP points loaded from"), mGCPpointsFileName);
  if ( mGCPsDirty )
  {
    mGCPListWidget->setGCPList( &mPoints );
    updateGeorefTransform();
    mCanvas->refresh();
    mIface->mapCanvas()->refresh();
  }

  return true;
}

void QgsGeorefPluginGui::saveGCPs()
{
  QFile pointFile( mGCPpointsFileName );
  if ( pointFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    QTextStream points( &pointFile );
    points << "mapX,mapY,pixelX,pixelY,enable" << endl;
    Q_FOREACH ( QgsGeorefDataPoint *pt, mPoints )
    {
      points << QStringLiteral( "%1,%2,%3,%4,%5" )
             .arg( qgsDoubleToString( pt->mapCoords().x() ),
                   qgsDoubleToString( pt->mapCoords().y() ),
                   qgsDoubleToString( pt->pixelCoords().x() ),
                   qgsDoubleToString( pt->pixelCoords().y() ) )
             .arg( pt->isEnabled() ) << endl;
    }

    mInitialPoints = mPoints;
  }
  else
  {
    mMessageBar->pushMessage( tr( "Write Error" ), tr( "Could not write to GCP points file %1." ).arg( mGCPpointsFileName ), Qgis::Warning, messageTimeout() );
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
    QMessageBox::StandardButton a = QMessageBox::question( this, tr( "Save GCPs" ),
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
    QgsPointXY origin;
    double pixelXSize, pixelYSize, rotation;
    if ( !mGeorefTransform.getOriginScaleRotation( origin, pixelXSize, pixelYSize, rotation ) )
    {
      mMessageBar->pushMessage( tr( "Transform Failed" ), tr( "Failed to calculate linear transform parameters." ), Qgis::Warning, messageTimeout() );
      return false;
    }

    if ( !mWorldFileName.isEmpty() )
    {
      if ( QFile::exists( mWorldFileName ) )
      {
        int r = QMessageBox::question( this, tr( "Georeference" ),
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
      mMessageBar->pushMessage( tr( "Transform Failed" ), tr( "Failed to compute GCP transform: Transform is not solvable." ), Qgis::Warning, messageTimeout() );
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
}

bool QgsGeorefPluginGui::writeWorldFile( const QgsPointXY &origin, double pixelXSize, double pixelYSize, double rotation )
{
  // write the world file
  QFile file( mWorldFileName );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    mMessageBar->pushMessage( tr( "Save World File" ), tr( "Could not write to %1." ).arg( mWorldFileName ), Qgis::Critical, messageTimeout() );
    return false;
  }

  double rotationX = 0;
  double rotationY = 0;

  if ( !qgsDoubleNear( rotation, 0.0 ) )
  {
    rotationX = pixelXSize * std::sin( rotation );
    rotationY = pixelYSize * std::sin( rotation );
    pixelXSize *= std::cos( rotation );
    pixelYSize *= std::cos( rotation );
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

bool QgsGeorefPluginGui::calculateMeanError( double &error ) const
{
  if ( mGeorefTransform.transformParametrisation() == QgsGeorefTransform::InvalidTransform )
  {
    return false;
  }

  int nPointsEnabled = 0;
  QgsGCPList::const_iterator gcpIt = mPoints.constBegin();
  for ( ; gcpIt != mPoints.constEnd(); ++gcpIt )
  {
    if ( ( *gcpIt )->isEnabled() )
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
    if ( ( *gcpIt )->isEnabled() )
    {
      sumVxSquare += ( ( *gcpIt )->residual().x() * ( *gcpIt )->residual().x() );
      sumVySquare += ( ( *gcpIt )->residual().y() * ( *gcpIt )->residual().y() );
    }
  }

  // Calculate the root mean square error, adjusted for degrees of freedom of the transform
  // Caveat: The number of DoFs is assumed to be even (as each control point fixes two degrees of freedom).
  error = std::sqrt( ( sumVxSquare + sumVySquare ) / ( nPointsEnabled - mGeorefTransform.getMinimumGCPCount() ) );
  return true;
}

bool QgsGeorefPluginGui::writePDFMapFile( const QString &fileName, const QgsGeorefTransform &transform )
{
  Q_UNUSED( transform );
  if ( !mCanvas )
  {
    return false;
  }

  QgsRasterLayer *rlayer = ( QgsRasterLayer * ) mCanvas->layer( 0 );
  if ( !rlayer )
  {
    return false;
  }
  double mapRatio = rlayer->extent().width() / rlayer->extent().height();

  QgsSettings s;
  double paperWidth = s.value( QStringLiteral( "/Plugin-GeoReferencer/Config/WidthPDFMap" ), "297" ).toDouble();
  double paperHeight = s.value( QStringLiteral( "/Plugin-GeoReferencer/Config/HeightPDFMap" ), "420" ).toDouble();

  //create layout
  QgsLayout layout( QgsProject::instance() );
  std::unique_ptr< QgsLayoutItemPage > page = qgis::make_unique< QgsLayoutItemPage >( &layout );

  double leftMargin = 8;
  double topMargin = 8;
  double contentWidth = 0.0;
  double contentHeight = 0.0;

  if ( mapRatio >= 1 )
  {
    page->setPageSize( QgsLayoutSize( paperHeight, paperWidth ) );
    contentWidth = paperHeight - 2 * leftMargin;
    contentHeight = paperWidth - 2 * topMargin;
  }
  else
  {
    page->setPageSize( QgsLayoutSize( paperWidth, paperHeight ) );
    contentWidth = paperWidth - 2 * leftMargin;
    contentHeight = paperHeight - 2 * topMargin;
  }
  layout.pageCollection()->addPage( page.release() );

  //layout map
  QgsLayoutItemMap *layoutMap = new QgsLayoutItemMap( &layout );
  layoutMap->attemptSetSceneRect( QRectF( leftMargin, topMargin, contentWidth, contentHeight ) );
  layoutMap->setKeepLayerSet( true );
  QgsMapLayer *firstLayer = mCanvas->mapSettings().layers()[0];
  layoutMap->setLayers( QList<QgsMapLayer *>() << firstLayer );
  layoutMap->setCrs( rlayer->crs() );
  layoutMap->zoomToExtent( rlayer->extent() );
  layout.addLayoutItem( layoutMap );

  QString residualUnits;
  if ( s.value( QStringLiteral( "/Plugin-GeoReferencer/Config/ResidualUnits" ) ) == "mapUnits" && mGeorefTransform.providesAccurateInverseTransformation() )
  {
    residualUnits = tr( "map units" );
  }
  else
  {
    residualUnits = tr( "pixels" );
  }

  //residual plot
  QgsResidualPlotItem *resPlotItem = new QgsResidualPlotItem( &layout );
  layout.addLayoutItem( resPlotItem );
  resPlotItem->attemptSetSceneRect( QRectF( leftMargin, topMargin, contentWidth, contentHeight ) );
  resPlotItem->setExtent( layoutMap->extent() );
  resPlotItem->setGCPList( mPoints );
  resPlotItem->setConvertScaleToMapUnits( residualUnits == tr( "map units" ) );

  QgsLayoutExporter exporter( &layout );
  QgsLayoutExporter::PdfExportSettings settings;
  settings.dpi = 300;
  exporter.exportToPdf( fileName, settings );

  return true;
}

bool QgsGeorefPluginGui::writePDFReportFile( const QString &fileName, const QgsGeorefTransform &transform )
{
  if ( !mCanvas )
  {
    return false;
  }

  //create layout A4 with 300 dpi
  QgsLayout layout( QgsProject::instance() );

  std::unique_ptr< QgsLayoutItemPage > page = qgis::make_unique< QgsLayoutItemPage >( &layout );
  page->setPageSize( QgsLayoutSize( 210, 297 ) ); //A4
  layout.pageCollection()->addPage( page.release() );
  std::unique_ptr< QgsLayoutItemPage > page2 = qgis::make_unique< QgsLayoutItemPage >( &layout );
  page2->setPageSize( QgsLayoutSize( 210, 297 ) ); //A4
  layout.pageCollection()->addPage( page2.release() );

  QFont titleFont;
  titleFont.setPointSize( 9 );
  titleFont.setBold( true );
  QFont tableHeaderFont;
  tableHeaderFont.setPointSize( 9 );
  tableHeaderFont.setBold( true );
  QFont tableContentFont;
  tableContentFont.setPointSize( 9 );

  QgsSettings s;
  double leftMargin = s.value( QStringLiteral( "/Plugin-GeoReferencer/Config/LeftMarginPDF" ), "2.0" ).toDouble();
  double rightMargin = s.value( QStringLiteral( "/Plugin-GeoReferencer/Config/RightMarginPDF" ), "2.0" ).toDouble();
  double contentWidth = 210 - ( leftMargin + rightMargin );

  //title
  QFileInfo rasterFi( mRasterFileName );
  QgsLayoutItemLabel *titleLabel = new QgsLayoutItemLabel( &layout );
  titleLabel->setFont( titleFont );
  titleLabel->setText( rasterFi.fileName() );
  layout.addLayoutItem( titleLabel );
  titleLabel->attemptSetSceneRect( QRectF( leftMargin, 5, contentWidth, 8 ) );
  titleLabel->setFrameEnabled( false );

  //layout map
  QgsRasterLayer *rLayer = ( QgsRasterLayer * ) mCanvas->layer( 0 );
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

  QgsLayoutItemMap *layoutMap = new QgsLayoutItemMap( &layout );
  layoutMap->attemptSetSceneRect( QRectF( leftMargin, titleLabel->rect().bottom() + titleLabel->pos().y(), mapWidthMM, mapHeightMM ) );
  layoutMap->setLayers( mCanvas->mapSettings().layers() );
  layoutMap->setCrs( rLayer->crs() );
  layoutMap->zoomToExtent( layerExtent );
  layout.addLayoutItem( layoutMap );

  QgsLayoutItemTextTable *parameterTable = nullptr;
  double scaleX, scaleY, rotation;
  QgsPointXY origin;

  QgsLayoutItemLabel *parameterLabel = nullptr;
  //transformation that involves only scaling and rotation (linear or helmert) ?
  bool wldTransform = transform.getOriginScaleRotation( origin, scaleX, scaleY, rotation );

  QString residualUnits;
  if ( s.value( QStringLiteral( "/Plugin-GeoReferencer/Config/ResidualUnits" ) ) == "mapUnits" && mGeorefTransform.providesAccurateInverseTransformation() )
  {
    residualUnits = tr( "map units" );
  }
  else
  {
    residualUnits = tr( "pixels" );
  }

  QGraphicsRectItem *previousItem = layoutMap;
  if ( wldTransform )
  {
    QString parameterTitle = tr( "Transformation parameters" ) + QStringLiteral( " (" ) + convertTransformEnumToString( transform.transformParametrisation() ) + QStringLiteral( ")" );
    parameterLabel = new QgsLayoutItemLabel( &layout );
    parameterLabel->setFont( titleFont );
    parameterLabel->setText( parameterTitle );
    parameterLabel->adjustSizeToText();
    layout.addLayoutItem( parameterLabel );
    parameterLabel->attemptSetSceneRect( QRectF( leftMargin, layoutMap->rect().bottom() + layoutMap->pos().y() + 5, contentWidth, 8 ) );
    parameterLabel->setFrameEnabled( false );

    //calculate mean error
    double meanError = 0;
    calculateMeanError( meanError );

    parameterTable = new QgsLayoutItemTextTable( &layout );
    parameterTable->setHeaderFont( tableHeaderFont );
    parameterTable->setContentFont( tableContentFont );

    QgsLayoutTableColumns columns;
    columns << new QgsLayoutTableColumn( tr( "Translation x" ) )
            << new QgsLayoutTableColumn( tr( "Translation y" ) )
            << new QgsLayoutTableColumn( tr( "Scale x" ) )
            << new QgsLayoutTableColumn( tr( "Scale y" ) )
            << new QgsLayoutTableColumn( tr( "Rotation [degrees]" ) )
            << new QgsLayoutTableColumn( tr( "Mean error [%1]" ).arg( residualUnits ) );

    parameterTable->setColumns( columns );
    QStringList row;
    row << QString::number( origin.x(), 'f', 3 ) << QString::number( origin.y(), 'f', 3 ) << QString::number( scaleX ) << QString::number( scaleY ) << QString::number( rotation * 180 / M_PI ) << QString::number( meanError );
    parameterTable->addRow( row );

    QgsLayoutFrame *tableFrame = new QgsLayoutFrame( &layout, parameterTable );
    tableFrame->attemptSetSceneRect( QRectF( leftMargin, parameterLabel->rect().bottom() + parameterLabel->pos().y() + 5, contentWidth, 12 ) );
    parameterTable->addFrame( tableFrame );

    parameterTable->setGridStrokeWidth( 0.1 );

    previousItem = tableFrame;
  }

  QgsLayoutItemLabel *residualLabel = new QgsLayoutItemLabel( &layout );
  residualLabel->setFont( titleFont );
  residualLabel->setText( tr( "Residuals" ) );
  layout.addLayoutItem( residualLabel );
  residualLabel->attemptSetSceneRect( QRectF( leftMargin, previousItem->rect().bottom() + previousItem->pos().y() + 5, contentWidth, 6 ) );
  residualLabel->setFrameEnabled( false );

  //residual plot
  QgsResidualPlotItem *resPlotItem = new QgsResidualPlotItem( &layout );
  layout.addLayoutItem( resPlotItem );
  resPlotItem->attemptSetSceneRect( QRectF( leftMargin, residualLabel->rect().bottom() + residualLabel->pos().y() + 5, contentWidth, layoutMap->rect().height() ) );
  resPlotItem->setExtent( layoutMap->extent() );
  resPlotItem->setGCPList( mPoints );

  //necessary for the correct scale bar unit label
  resPlotItem->setConvertScaleToMapUnits( residualUnits == tr( "map units" ) );

  QgsLayoutItemTextTable *gcpTable = new QgsLayoutItemTextTable( &layout );
  gcpTable->setHeaderFont( tableHeaderFont );
  gcpTable->setContentFont( tableContentFont );
  gcpTable->setHeaderMode( QgsLayoutTable::AllFrames );
  QgsLayoutTableColumns columns;
  columns << new QgsLayoutTableColumn( tr( "ID" ) )
          << new QgsLayoutTableColumn( tr( "Enabled" ) )
          << new QgsLayoutTableColumn( tr( "Pixel X" ) )
          << new QgsLayoutTableColumn( tr( "Pixel Y" ) )
          << new QgsLayoutTableColumn( tr( "Map X" ) )
          << new QgsLayoutTableColumn( tr( "Map Y" ) )
          << new QgsLayoutTableColumn( tr( "Res X (%1)" ).arg( residualUnits ) )
          << new QgsLayoutTableColumn( tr( "Res Y (%1)" ).arg( residualUnits ) )
          << new QgsLayoutTableColumn( tr( "Res Total (%1)" ).arg( residualUnits ) );

  gcpTable->setColumns( columns );

  QgsGCPList::const_iterator gcpIt = mPoints.constBegin();
  QVector< QStringList > gcpTableContents;
  for ( ; gcpIt != mPoints.constEnd(); ++gcpIt )
  {
    QStringList currentGCPStrings;
    QPointF residual = ( *gcpIt )->residual();
    double residualTot = std::sqrt( residual.x() * residual.x() +  residual.y() * residual.y() );

    currentGCPStrings << QString::number( ( *gcpIt )->id() );
    if ( ( *gcpIt )->isEnabled() )
    {
      currentGCPStrings << tr( "yes" );
    }
    else
    {
      currentGCPStrings << tr( "no" );
    }
    currentGCPStrings << QString::number( ( *gcpIt )->pixelCoords().x(), 'f', 0 ) << QString::number( ( *gcpIt )->pixelCoords().y(), 'f', 0 ) << QString::number( ( *gcpIt )->mapCoords().x(), 'f', 3 )
                      <<  QString::number( ( *gcpIt )->mapCoords().y(), 'f', 3 ) <<  QString::number( residual.x() ) <<  QString::number( residual.y() ) << QString::number( residualTot );
    gcpTableContents << currentGCPStrings;
  }

  gcpTable->setContents( gcpTableContents );

  double firstFrameY = resPlotItem->rect().bottom() + resPlotItem->pos().y() + 5;
  double firstFrameHeight = 287 - firstFrameY;
  QgsLayoutFrame *gcpFirstFrame = new QgsLayoutFrame( &layout, gcpTable );
  gcpFirstFrame->attemptSetSceneRect( QRectF( leftMargin, firstFrameY, contentWidth, firstFrameHeight ) );
  gcpTable->addFrame( gcpFirstFrame );

  QgsLayoutFrame *gcpSecondFrame = new QgsLayoutFrame( &layout, gcpTable );
  gcpSecondFrame->attemptSetSceneRect( QRectF( leftMargin, 10, contentWidth, 277.0 ) );
  gcpSecondFrame->attemptMove( QgsLayoutPoint( leftMargin, 10 ), true, false, 1 );
  gcpSecondFrame->setHidePageIfEmpty( true );
  gcpTable->addFrame( gcpSecondFrame );

  gcpTable->setGridStrokeWidth( 0.1 );
  gcpTable->setResizeMode( QgsLayoutMultiFrame::RepeatUntilFinished );

  QgsLayoutExporter exporter( &layout );
  QgsLayoutExporter::PdfExportSettings settings;
  settings.dpi = 300;
  exporter.exportToPdf( fileName, settings );

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

  QgsPointXY origin;
  double scaleX, scaleY, rotation;
  if ( mGeorefTransform.getOriginScaleRotation( origin, scaleX, scaleY, rotation ) )
  {
    labelString += ' ';
    labelString += tr( "Translation (%1, %2)" ).arg( origin.x() ).arg( origin.y() );
    labelString += ' ';
    labelString += tr( "Scale (%1, %2)" ).arg( scaleX ).arg( scaleY );
    labelString += ' ';
    labelString += tr( "Rotation: %1" ).arg( rotation * 180 / M_PI );
  }

  double meanError = 0;
  if ( calculateMeanError( meanError ) )
  {
    labelString += ' ';
    labelString += tr( "Mean error: %1" ).arg( meanError );
  }
  mTransformParamLabel->setText( labelString );
}

// Gdal script
void QgsGeorefPluginGui::showGDALScript( const QStringList &commands )
{
  QString script = commands.join( QStringLiteral( "\n" ) ) + '\n';

  // create window to show gdal script
  QDialogButtonBox *bbxGdalScript = new QDialogButtonBox( QDialogButtonBox::Cancel, Qt::Horizontal, this );
  QPushButton *pbnCopyInClipBoard = new QPushButton( getThemeIcon( QStringLiteral( "/mActionEditPaste.svg" ) ),
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
  dlgShowGdalScrip->setWindowTitle( tr( "GDAL Script" ) );
  dlgShowGdalScrip->setLayout( layout );

  connect( bbxGdalScript, &QDialogButtonBox::accepted, dlgShowGdalScrip, &QDialog::accept );
  connect( bbxGdalScript, &QDialogButtonBox::rejected, dlgShowGdalScrip, &QDialog::reject );

  if ( dlgShowGdalScrip->exec() == QDialog::Accepted )
  {
    QApplication::clipboard()->setText( pteScript->toPlainText() );
  }
}

QString QgsGeorefPluginGui::generateGDALtranslateCommand( bool generateTFW )
{
  QStringList gdalCommand;
  gdalCommand << QStringLiteral( "gdal_translate" ) << QStringLiteral( "-of GTiff" );
  if ( generateTFW )
  {
    // say gdal generate associated ESRI world file
    gdalCommand << QStringLiteral( "-co TFW=YES" );
  }

  Q_FOREACH ( QgsGeorefDataPoint *pt, mPoints )
  {
    gdalCommand << QStringLiteral( "-gcp %1 %2 %3 %4" ).arg( pt->pixelCoords().x() ).arg( -pt->pixelCoords().y() )
                .arg( pt->mapCoords().x() ).arg( pt->mapCoords().y() );
  }

  QFileInfo rasterFileInfo( mRasterFileName );
  mTranslatedRasterFileName = QDir::tempPath() + '/' + rasterFileInfo.fileName();
  gdalCommand << QStringLiteral( "\"%1\"" ).arg( mRasterFileName ) << QStringLiteral( "\"%1\"" ).arg( mTranslatedRasterFileName );

  return gdalCommand.join( QStringLiteral( " " ) );
}

QString QgsGeorefPluginGui::generateGDALwarpCommand( const QString &resampling, const QString &compress,
    bool useZeroForTrans, int order, double targetResX, double targetResY )
{
  QStringList gdalCommand;
  gdalCommand << QStringLiteral( "gdalwarp" ) << QStringLiteral( "-r" ) << resampling;

  if ( order > 0 && order <= 3 )
  {
    // Let gdalwarp use polynomial warp with the given degree
    gdalCommand << QStringLiteral( "-order" ) << QString::number( order );
  }
  else
  {
    // Otherwise, use thin plate spline interpolation
    gdalCommand << QStringLiteral( "-tps" );
  }
  gdalCommand << "-co COMPRESS=" + compress << ( useZeroForTrans ? "-dstalpha" : "" );

  if ( targetResX != 0.0 && targetResY != 0.0 )
  {
    gdalCommand << QStringLiteral( "-tr" ) << QString::number( targetResX, 'f' ) << QString::number( targetResY, 'f' );
  }

  gdalCommand << QStringLiteral( "\"%1\"" ).arg( mTranslatedRasterFileName ) << QStringLiteral( "\"%1\"" ).arg( mModifiedRasterFileName );

  return gdalCommand.join( QStringLiteral( " " ) );
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
    mMessageBar->pushMessage( tr( "No Raster Loaded" ), tr( "Please load raster to be georeferenced." ), Qgis::Warning, messageTimeout() );
    return false;
  }

  if ( QgsGeorefTransform::InvalidTransform == mTransformParam )
  {
    QMessageBox::information( this, tr( "Georeferencer" ), tr( "Please set transformation type." ) );
    getTransformSettings();
    return false;
  }

  //MH: helmert transformation without warping disabled until qgis is able to read rotated rasters efficiently
  if ( mModifiedRasterFileName.isEmpty() && QgsGeorefTransform::Linear != mTransformParam /*&& QgsGeorefTransform::Helmert != mTransformParam*/ )
  {
    QMessageBox::information( this, tr( "Georeferencer" ), tr( "Please set output raster name." ) );
    getTransformSettings();
    return false;
  }

  if ( mPoints.count() < ( int )mGeorefTransform.getMinimumGCPCount() )
  {
    mMessageBar->pushMessage( tr( "Not Enough GCPs" ), tr( "%1 transformation requires at least %2 GCPs. Please define more." )
                              .arg( convertTransformEnumToString( mTransformParam ) ).arg( mGeorefTransform.getMinimumGCPCount() )
                              , Qgis::Warning, messageTimeout() );
    return false;
  }

  // Update the transform if necessary
  if ( !updateGeorefTransform() )
  {
    mMessageBar->pushMessage( tr( "Transform Failed" ), tr( "Failed to compute GCP transform: Transform is not solvable." ), Qgis::Warning, messageTimeout() );
    //    logRequaredGCPs();
    return false;
  }

  return true;
}

bool QgsGeorefPluginGui::updateGeorefTransform()
{
  QVector<QgsPointXY> mapCoords, pixelCoords;
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
  minX = minY = std::numeric_limits<double>::max();
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
      QgsPointXY src, raster;
      switch ( edge )
      {
        case 0:
          src = QgsPointXY( oX + ( double )s * stepX, oY );
          break;
        case 1:
          src = QgsPointXY( oX + ( double )s * stepX, dY );
          break;
        case 2:
          src = QgsPointXY( oX, oY + ( double )s * stepY );
          break;
        case 3:
          src = QgsPointXY( dX, oY + ( double )s * stepY );
          break;
      }
      t.transform( src, raster, rasterToWorld );
      minX = std::min( raster.x(), minX );
      maxX = std::max( raster.x(), maxX );
      minY = std::min( raster.y(), minY );
      maxY = std::max( raster.y(), maxY );
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
      return QStringLiteral( "near" );
    case QgsImageWarper::Bilinear:
      return QStringLiteral( "bilinear" );
    case QgsImageWarper::Cubic:
      return QStringLiteral( "cubic" );
    case QgsImageWarper::CubicSpline:
      return QStringLiteral( "cubicspline" );
    case QgsImageWarper::Lanczos:
      return QStringLiteral( "lanczos" );
  }
  return QString();
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
  QString worldFileName;
  int point = rasterFileName.lastIndexOf( '.' );
  if ( point != -1 && point != rasterFileName.length() - 1 )
    worldFileName = rasterFileName.left( point + 1 ) + "wld";

  return worldFileName;
}

// Note this code is duplicated from qgisapp.cpp because
// I didn't want to make plugins on qgsapplication [TS]
QIcon QgsGeorefPluginGui::getThemeIcon( const QString &name )
{
  if ( QFile::exists( QgsApplication::activeThemePath() + name ) )
  {
    return QIcon( QgsApplication::activeThemePath() + name );
  }
  else if ( QFile::exists( QgsApplication::defaultThemePath() + name ) )
  {
    return QIcon( QgsApplication::defaultThemePath() + name );
  }
  else
  {
    QgsSettings settings;
    QString themePath = ":/icons/" + settings.value( QStringLiteral( "Themes" ) ).toString() + name;
    if ( QFile::exists( themePath ) )
    {
      return QIcon( themePath );
    }
    else
    {
      return QIcon( ":/icons/default" + name );
    }
  }
}

bool QgsGeorefPluginGui::checkFileExisting( const QString &fileName, const QString &title, const QString &question )
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
  QgsSettings settings;
  return settings.value( QStringLiteral( "qgis/messageTimeout" ), 5 ).toInt();
}
