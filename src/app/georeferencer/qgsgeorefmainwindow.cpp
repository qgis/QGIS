/***************************************************************************
     qgsgeorefmainwindow.cpp
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
#include <QUrl>

#include "qgssettings.h"
#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgsgui.h"
#include "qgisapp.h"

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
#include "qgsdatasourceselectdialog.h"

#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "../../gui/raster/qgsrasterlayerproperties.h"
#include "qgsproviderregistry.h"

#include "qgsgeorefdatapoint.h"
#include "qgsgeoreftooladdpoint.h"
#include "qgsgeoreftooldeletepoint.h"
#include "qgsgeoreftoolmovepoint.h"
#include "qgsgcpcanvasitem.h"

#include "qgsgcplistwidget.h"

#include "qgsgeorefconfigdialog.h"
#include "qgsresidualplotitem.h"
#include "qgstransformsettingsdialog.h"

#include "qgsgeorefmainwindow.h"
#include "qgsmessagebar.h"
#include "qgsvectorwarper.h"

QgsGeorefDockWidget::QgsGeorefDockWidget( const QString &title, QWidget *parent, Qt::WindowFlags flags )
  : QgsDockWidget( title, parent, flags )
{
  setObjectName( QStringLiteral( "GeorefDockWidget" ) ); // set object name so the position can be saved
}

QgsGeoreferencerMainWindow::QgsGeoreferencerMainWindow( QWidget *parent, Qt::WindowFlags fl )
  : QMainWindow( parent, fl )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );
  setAcceptDrops( true );

  QWidget *centralWidget = this->centralWidget();
  mCentralLayout = new QGridLayout( centralWidget );
  centralWidget->setLayout( mCentralLayout );
  mCentralLayout->setContentsMargins( 0, 0, 0, 0 );

  QgsSettings settings;
  // default to last used target CRS
  QString targetCRSString = settings.value( QStringLiteral( "/Plugin-GeoReferencer/targetsrs" ) ).toString();
  mTargetCrs = QgsCoordinateReferenceSystem( targetCRSString );

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

  mActionLinkGeorefToQgis->setEnabled( false );
  mActionLinkQGisToGeoref->setEnabled( false );

  mCanvas->clearExtentHistory(); // reset zoomnext/zoomlast

  if ( settings.value( QStringLiteral( "/Plugin-GeoReferencer/Config/ShowDocked" ) ).toBool() )
  {
    dockThisWindow( true );
  }
}

void QgsGeoreferencerMainWindow::dockThisWindow( bool dock )
{
  if ( mDock )
  {
    setParent( QgisApp::instance(), Qt::Window );
    show();

    QgisApp::instance()->removeDockWidget( mDock );
    mDock->setWidget( nullptr );
    delete mDock;
    mDock = nullptr;
  }

  if ( dock )
  {
    mDock = new QgsGeorefDockWidget( tr( "Georeferencer" ), QgisApp::instance() );
    mDock->setWidget( this );
    QgisApp::instance()->addDockWidget( Qt::BottomDockWidgetArea, mDock );
  }
}

QgsGeoreferencerMainWindow::~QgsGeoreferencerMainWindow()
{
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

void QgsGeoreferencerMainWindow::closeEvent( QCloseEvent *e )
{
  switch ( checkNeedGCPSave() )
  {
    case QgsGeoreferencerMainWindow::GCPSAVE:
      saveGCPsDialog();
      writeSettings();
      clearGCPData();
      removeOldLayer();
      mFileName.clear();
      e->accept();
      return;
    case QgsGeoreferencerMainWindow::GCPDISCARD:
      writeSettings();
      clearGCPData();
      removeOldLayer();
      mFileName.clear();
      e->accept();
      return;
    case QgsGeoreferencerMainWindow::GCPCANCEL:
      e->ignore();
      return;
  }
}

void QgsGeoreferencerMainWindow::reset()
{
  if ( QMessageBox::question( this,
                              tr( "Reset Georeferencer" ),
                              tr( "Reset georeferencer and clear all GCP points?" ),
                              QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel ) != QMessageBox::Cancel )
  {
    mFileName.clear();
    mModifiedFileName.clear();
    mCreateWorldFileOnly = false;
    setWindowTitle( tr( "Georeferencer" ) );

    //delete old points
    clearGCPData();

    //delete any old rasterlayers
    removeOldLayer();
  }
}

void QgsGeoreferencerMainWindow::openLayer( QgsMapLayerType layerType, const QString &fileName )
{
  switch ( checkNeedGCPSave() )
  {
    case QgsGeoreferencerMainWindow::GCPSAVE:
      saveGCPsDialog();
      break;
    case QgsGeoreferencerMainWindow::GCPDISCARD:
      break;
    case QgsGeoreferencerMainWindow::GCPCANCEL:
      return;
  }

  QString provider;
  QString uri;
  if ( fileName.isEmpty() )
  {
    QString dir = settingLastSourceFolder.value();
    if ( dir.isEmpty() )
      dir = QDir::homePath();

    QString otherFiles = tr( "All other files (*)" );
    QString lastUsedFilter;

    switch ( layerType )
    {

      case QgsMapLayerType::RasterLayer:
      {
        QString lastUsedFilter = settingLastRasterFileFilter.value();
        if ( lastUsedFilter.isEmpty() )
          lastUsedFilter = otherFiles;

        QString filters = QgsProviderRegistry::instance()->fileRasterFilters();
        filters.prepend( otherFiles + QStringLiteral( ";;" ) );
        filters.chop( otherFiles.size() + 2 );
        mFileName = QFileDialog::getOpenFileName( this, tr( "Open Raster" ), dir, filters, &lastUsedFilter, QFileDialog::HideNameFilterDetails );
        if ( mFileName.isEmpty() )
          return;

        provider = QStringLiteral( "gdal" );
        uri = mFileName;
        settingLastRasterFileFilter.setValue( lastUsedFilter );
        break;
      }

      case QgsMapLayerType::VectorLayer:
      {
        QgsDataSourceSelectDialog dlg( QgisApp::instance()->browserModel(), true, QgsMapLayerType::VectorLayer, this );
        dlg.setWindowTitle( tr( "Open Vector" ) );
        if ( !dlg.exec() )
          return;

        uri = dlg.uri().uri;
        provider = dlg.uri().providerKey;

        const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( provider, uri );
        mFileName = parts.value( QStringLiteral( "path" ) ).toString().isEmpty() ? uri : parts.value( QStringLiteral( "path" ) ).toString();

        break;
      }

      case QgsMapLayerType::PluginLayer:
      case QgsMapLayerType::MeshLayer:
      case QgsMapLayerType::VectorTileLayer:
      case QgsMapLayerType::AnnotationLayer:
      case QgsMapLayerType::PointCloudLayer:
      case QgsMapLayerType::GroupLayer:
        break;
    }
  }
  else
  {
    mFileName = fileName;
  }
  mModifiedFileName.clear();
  mCreateWorldFileOnly = false;

  QString errMsg;
  switch ( layerType )
  {
    case QgsMapLayerType::RasterLayer:
      if ( !QgsRasterLayer::isValidRasterFileName( mFileName, errMsg ) )
      {
        mMessageBar->pushMessage( tr( "Open Raster" ), tr( "%1 is not a supported raster data source.%2" ).arg( mFileName,
                                  !errMsg.isEmpty() ? QStringLiteral( " (%1)" ).arg( errMsg ) : QString() ), Qgis::MessageLevel::Critical );
        return;
      }
      break;

    case QgsMapLayerType::PluginLayer:
    case QgsMapLayerType::VectorLayer:
    case QgsMapLayerType::MeshLayer:
    case QgsMapLayerType::VectorTileLayer:
    case QgsMapLayerType::AnnotationLayer:
    case QgsMapLayerType::PointCloudLayer:
    case QgsMapLayerType::GroupLayer:
      break;
  }

  QFileInfo fileInfo( mFileName );
  settingLastSourceFolder.setValue( fileInfo.path() );

  mGeorefTransform.setMethod( mTransformMethod );


  switch ( layerType )
  {
    case QgsMapLayerType::RasterLayer:
      mGeorefTransform.loadRaster( mFileName );
      break;
    case QgsMapLayerType::VectorLayer:
    case QgsMapLayerType::PluginLayer:
    case QgsMapLayerType::MeshLayer:
    case QgsMapLayerType::VectorTileLayer:
    case QgsMapLayerType::AnnotationLayer:
    case QgsMapLayerType::PointCloudLayer:
    case QgsMapLayerType::GroupLayer:
      break;
  }

  statusBar()->showMessage( tr( "Source loaded: %1" ).arg( mFileName ) );
  setWindowTitle( tr( "Georeferencer - %1" ).arg( fileInfo.fileName() ) );

  //delete old points
  clearGCPData();

  //delete any old layers
  removeOldLayer();

  // Add source layer
  loadSource( layerType, uri, provider );

  // load previously added points
  mGCPpointsFileName = mFileName + ".points";
  QString error;
  ( void )loadGCPs( error );

  if ( mLayer )
    mCanvas->setExtent( mLayer->extent() );

  mCanvas->refresh();
  QgisApp::instance()->mapCanvas()->refresh();

  const bool hasExistingReference = mLayer->crs().isValid();
  mActionLinkGeorefToQgis->setEnabled( hasExistingReference );
  mActionLinkQGisToGeoref->setEnabled( hasExistingReference );
  if ( !hasExistingReference )
  {
    mActionLinkGeorefToQgis->setChecked( false );
    mActionLinkQGisToGeoref->setChecked( false );
  }

  mCanvas->clearExtentHistory();
  mWorldFileName = guessWorldFileName( mFileName );
}

void QgsGeoreferencerMainWindow::dropEvent( QDropEvent *event )
{
  // dragging app is locked for the duration of dropEvent. This causes explorer windows to hang
  // while large projects/layers are loaded. So instead we return from dropEvent as quickly as possible
  // and do the actual handling of the drop after a very short timeout
  QTimer *timer = new QTimer( this );
  timer->setSingleShot( true );
  timer->setInterval( 50 );

  // get the file list
  QList<QUrl>::iterator i;
  QList<QUrl>urls = event->mimeData()->urls();
  QString file;
  for ( i = urls.begin(); i != urls.end(); ++i )
  {
    QString fileName = i->toLocalFile();
    // seems that some drag and drop operations include an empty url
    // so we test for length to make sure we have something
    if ( !fileName.isEmpty() )
    {
      file = fileName;
      break;
    }
  }

  connect( timer, &QTimer::timeout, this, [this, timer, file]
  {
    // TODO -- consider using querySublayers to determine this instead
    if ( QgsRasterLayer::isValidRasterFileName( file ) )
      openLayer( QgsMapLayerType::RasterLayer, file );
    else
      openLayer( QgsMapLayerType::VectorLayer, file );

    timer->deleteLater();
  } );

  event->acceptProposedAction();
  timer->start();
}

void QgsGeoreferencerMainWindow::dragEnterEvent( QDragEnterEvent *event )
{
  if ( event->mimeData()->hasUrls() )
  {
    event->acceptProposedAction();
  }
}

bool QgsGeoreferencerMainWindow::showTransformSettingsDialog()
{
  QgsTransformSettingsDialog d( mLayer ? mLayer->type() : QgsMapLayerType::RasterLayer, mFileName, mModifiedFileName );
  d.setTargetCrs( mTargetCrs );
  d.setCreateWorldFileOnly( mCreateWorldFileOnly );
  d.setTransformMethod( mTransformMethod );
  d.setResamplingMethod( mResamplingMethod );
  d.setCompressionMethod( mCompressionMethod );
  d.setPdfMapFilename( mPdfOutputMapFile );
  d.setPdfReportFilename( mPdfOutputFile );
  d.setSaveGcpPoints( mSaveGcp );
  d.setUseZeroForTransparent( mUseZeroForTrans );
  d.setLoadInProject( mLoadInQgis );
  d.setOutputResolution( mUserResX, mUserResY );

  if ( !d.exec() )
  {
    return false;
  }

  d.outputResolution( mUserResX, mUserResY );
  mCreateWorldFileOnly = d.createWorldFileOnly();
  mTargetCrs = d.targetCrs();
  mTransformMethod = d.transformMethod();
  mResamplingMethod = d.resamplingMethod();
  mCompressionMethod = d.compressionMethod();
  mModifiedFileName = d.destinationFilename();
  mPdfOutputMapFile = d.pdfMapFilename();
  mPdfOutputFile = d.pdfReportFilename();
  mSaveGcp = d.saveGcpPoints();
  mUseZeroForTrans = d.useZeroForTransparent();
  mLoadInQgis = d.loadInProject();

  mTransformParamLabel->setText( tr( "Transform: " ) + QgsGcpTransformerInterface::methodToString( mTransformMethod ) );
  mGeorefTransform.setMethod( mTransformMethod );
  mGCPListWidget->setGeorefTransform( &mGeorefTransform );
  mWorldFileName = guessWorldFileName( mFileName );

  const bool hasReferencing = QgsGcpTransformerInterface::TransformMethod::InvalidTransform != mTransformMethod
                              || mLayer->crs().isValid();
  mActionLinkGeorefToQgis->setEnabled( hasReferencing );
  mActionLinkQGisToGeoref->setEnabled( hasReferencing );
  if ( !hasReferencing )
  {
    mActionLinkGeorefToQgis->setChecked( false );
    mActionLinkQGisToGeoref->setChecked( false );
  }

  //update gcp model
  if ( mGCPListWidget )
  {
    mGCPListWidget->setTargetCrs( mTargetCrs, QgsProject::instance()->transformContext() );
    mGCPListWidget->updateResiduals();
  }

  updateTransformParamLabel();
  return true;
}

void QgsGeoreferencerMainWindow::generateGDALScript()
{
  if ( !validate() )
    return;

  bool isIncompatibleTransformMethod = false;

  switch ( mTransformMethod )
  {
    case QgsGcpTransformerInterface::TransformMethod::PolynomialOrder1:
    case QgsGcpTransformerInterface::TransformMethod::PolynomialOrder2:
    case QgsGcpTransformerInterface::TransformMethod::PolynomialOrder3:
    case QgsGcpTransformerInterface::TransformMethod::ThinPlateSpline:
    {
      switch ( mLayer->type() )
      {
        case QgsMapLayerType::RasterLayer:
        {
          // CAVEAT: generateGDALwarpCommand() relies on some member variables being set
          // by generateGDALtranslateCommand(), so this method must be called before
          // gdalwarpCommand*()!
          QString translateCommand = generateGDALtranslateCommand( false );
          QString gdalwarpCommand;
          QString resamplingStr = convertResamplingEnumToString( mResamplingMethod );

          int order = polynomialOrder( mTransformMethod );
          if ( order != 0 )
          {
            gdalwarpCommand = generateGDALwarpCommand( resamplingStr, mCompressionMethod, mUseZeroForTrans, order,
                              mUserResX, mUserResY );
            showGDALScript( QStringList() << translateCommand << gdalwarpCommand );
          }
          else
          {
            isIncompatibleTransformMethod = true;
          }
          break;
        }

        case QgsMapLayerType::VectorLayer:
        {
          const QString command = generateGDALogr2ogrCommand();
          showGDALScript( QStringList() << command );
          break;
        }

        case QgsMapLayerType::PluginLayer:
        case QgsMapLayerType::MeshLayer:
        case QgsMapLayerType::VectorTileLayer:
        case QgsMapLayerType::AnnotationLayer:
        case QgsMapLayerType::PointCloudLayer:
        case QgsMapLayerType::GroupLayer:
          break;
      }
      break;
    }

    case QgsGcpTransformerInterface::TransformMethod::Linear:
    case QgsGcpTransformerInterface::TransformMethod::Helmert:
    case QgsGcpTransformerInterface::TransformMethod::Projective:
    case QgsGcpTransformerInterface::TransformMethod::InvalidTransform:
      isIncompatibleTransformMethod = true;
      break;
  }

  if ( isIncompatibleTransformMethod )
  {
    mMessageBar->pushMessage( tr( "Invalid Transform" ), tr( "GDAL scripting is not supported for %1 transformation." )
                              .arg( QgsGcpTransformerInterface::methodToString( mTransformMethod ) )
                              , Qgis::MessageLevel::Critical );
  }
}

void QgsGeoreferencerMainWindow::setAddPointTool()
{
  mCanvas->setMapTool( mToolAddPoint );
  QgsMapTool *activeQgisMapTool = QgisApp::instance()->mapCanvas()->mapTool();
  if ( activeQgisMapTool == mToolMovePointQgis )
    QgisApp::instance()->mapCanvas()->setMapTool( mPrevQgisMapTool );
}

void QgsGeoreferencerMainWindow::setDeletePointTool()
{
  mCanvas->setMapTool( mToolDeletePoint );
  QgsMapTool *activeQgisMapTool = QgisApp::instance()->mapCanvas()->mapTool();
  if ( activeQgisMapTool == mToolMovePointQgis )
    QgisApp::instance()->mapCanvas()->setMapTool( mPrevQgisMapTool );
}

void QgsGeoreferencerMainWindow::setMovePointTool()
{
  mCanvas->setMapTool( mToolMovePoint );
  QgsMapTool *activeQgisMapTool = QgisApp::instance()->mapCanvas()->mapTool();
  if ( activeQgisMapTool == mToolMovePointQgis )
    return;
  mPrevQgisMapTool = activeQgisMapTool;
  QgisApp::instance()->mapCanvas()->setMapTool( mToolMovePointQgis );
}

void QgsGeoreferencerMainWindow::setPanTool()
{
  mCanvas->setMapTool( mToolPan );
}

void QgsGeoreferencerMainWindow::setZoomInTool()
{
  mCanvas->setMapTool( mToolZoomIn );
}

void QgsGeoreferencerMainWindow::setZoomOutTool()
{
  mCanvas->setMapTool( mToolZoomOut );
}

void QgsGeoreferencerMainWindow::zoomToLayerTool()
{
  if ( mLayer )
  {
    mCanvas->setExtent( mLayer->extent() );
    mCanvas->refresh();
  }
}

void QgsGeoreferencerMainWindow::zoomToLast()
{
  mCanvas->zoomToPreviousExtent();
}

void QgsGeoreferencerMainWindow::zoomToNext()
{
  mCanvas->zoomToNextExtent();
}

void QgsGeoreferencerMainWindow::linkQGisToGeoref( bool link )
{
  if ( link )
  {
    // Indicate that georeferencer canvas extent has changed
    extentsChangedGeorefCanvas();
  }
}

void QgsGeoreferencerMainWindow::linkGeorefToQgis( bool link )
{
  if ( link )
  {
    // Indicate that qgis main canvas extent has changed
    extentsChangedQGisCanvas();
  }
}

void QgsGeoreferencerMainWindow::addPoint( const QgsPointXY &sourceCoords, const QgsPointXY &destinationMapCoords, const QgsCoordinateReferenceSystem &destinationCrs,
    bool enable, bool finalize )
{
  QgsGeorefDataPoint *pnt = new QgsGeorefDataPoint( mCanvas, QgisApp::instance()->mapCanvas(), sourceCoords, destinationMapCoords, destinationCrs, enable );
  mPoints.append( pnt );

  if ( !mLastGCPProjection.isValid() || mLastGCPProjection != destinationCrs )
    mLastGCPProjection = destinationCrs;
  mGCPsDirty = true;

  if ( finalize )
  {
    mGCPListWidget->setGCPList( &mPoints );
    mCanvas->refresh();
    QgisApp::instance()->mapCanvas()->refresh();
  }

  connect( mCanvas, &QgsMapCanvas::extentsChanged, pnt, &QgsGeorefDataPoint::updateCoords );
  if ( finalize )
  {
    updateGeorefTransform();
  }
}

void QgsGeoreferencerMainWindow::deleteDataPoint( QPoint coords )
{
  for ( QgsGCPList::iterator it = mPoints.begin(); it != mPoints.end(); ++it )
  {
    QgsGeorefDataPoint *pt = *it;
    if ( pt->contains( coords, QgsGcpPoint::PointType::Source ) ) // first operand for removing from GCP table
    {
      delete *it;
      mPoints.erase( it );
      mGCPListWidget->setGCPList( &mPoints );
      mCanvas->refresh();
      updateGeorefTransform();
      break;
    }
  }
}

void QgsGeoreferencerMainWindow::deleteDataPoint( int theGCPIndex )
{
  Q_ASSERT( theGCPIndex >= 0 );
  delete mPoints.takeAt( theGCPIndex );
  // TODO -- would be cleaner to move responsibility for this to the model!
  mGCPListWidget->setGCPList( &mPoints );
  updateGeorefTransform();
}

void QgsGeoreferencerMainWindow::selectPoint( QPoint p )
{
  const QgsGcpPoint::PointType pointType = sender() == mToolMovePoint ? QgsGcpPoint::PointType::Source : QgsGcpPoint::PointType::Destination;
  QgsGeorefDataPoint *&mvPoint = pointType == QgsGcpPoint::PointType::Source ? mMovingPoint : mMovingPointQgis;

  for ( QgsGCPList::const_iterator it = mPoints.constBegin(); it != mPoints.constEnd(); ++it )
  {
    if ( ( *it )->contains( p, pointType ) )
    {
      mvPoint = *it;
      break;
    }
  }
}

void QgsGeoreferencerMainWindow::movePoint( QPoint canvasPixels )
{
  const QgsGcpPoint::PointType pointType = sender() == mToolMovePoint ? QgsGcpPoint::PointType::Source : QgsGcpPoint::PointType::Destination;
  QgsGeorefDataPoint *&mvPoint = pointType == QgsGcpPoint::PointType::Source ? mMovingPoint : mMovingPointQgis;

  if ( mvPoint )
  {
    mvPoint->moveTo( canvasPixels, pointType );
  }
}

void QgsGeoreferencerMainWindow::releasePoint( QPoint p )
{
  Q_UNUSED( p )
  mGCPListWidget->updateResiduals();
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

void QgsGeoreferencerMainWindow::showCoordDialog( const QgsPointXY &sourceCoordinates )
{
  delete mNewlyAddedPointItem;
  mNewlyAddedPointItem = nullptr;

  // show a temporary marker at the clicked source point on the raster while we show the coordinate dialog.
  mNewlyAddedPointItem = new QgsGCPCanvasItem( mCanvas, nullptr, true );
  mNewlyAddedPointItem->setPointColor( QColor( 0, 200, 0 ) );
  mNewlyAddedPointItem->setPos( mNewlyAddedPointItem->toCanvasCoordinates( sourceCoordinates ) );

  QgsCoordinateReferenceSystem lastProjection = mLastGCPProjection.isValid() ? mLastGCPProjection : mTargetCrs;
  if ( mLayer && !mMapCoordsDialog )
  {
    mMapCoordsDialog = new QgsMapCoordsDialog( QgisApp::instance()->mapCanvas(), sourceCoordinates, lastProjection, this );
    connect( mMapCoordsDialog, &QgsMapCoordsDialog::pointAdded, this, [ = ]( const QgsPointXY & sourceLayerCoordinate, const QgsPointXY & destinationCoordinate, const QgsCoordinateReferenceSystem & destinationCrs )
    {
      addPoint( sourceLayerCoordinate, destinationCoordinate, destinationCrs );
    } );
    connect( mMapCoordsDialog, &QObject::destroyed, this, [ = ]
    {
      delete mNewlyAddedPointItem;
      mNewlyAddedPointItem = nullptr;
    } );
    mMapCoordsDialog->show();
  }
}

void QgsGeoreferencerMainWindow::loadGCPsDialog()
{
  QString selectedFile = mFileName.isEmpty() ? QString() : mFileName + ".points";
  mGCPpointsFileName = QFileDialog::getOpenFileName( this, tr( "Load GCP Points" ),
                       selectedFile, tr( "GCP file" ) + " (*.points)" );
  if ( mGCPpointsFileName.isEmpty() )
    return;

  QString error;
  if ( !loadGCPs( error ) )
  {
    mMessageBar->pushMessage( tr( "Load GCP Points" ), error, Qgis::MessageLevel::Critical );
  }
  else
  {
    mMessageBar->pushMessage( tr( "Load GCP Points" ), tr( "GCP file successfully loaded." ), Qgis::MessageLevel::Success );
  }
}

void QgsGeoreferencerMainWindow::saveGCPsDialog()
{
  if ( mPoints.isEmpty() )
  {
    mMessageBar->pushMessage( tr( "Save GCP Points" ), tr( "No GCP points are available to save." ), Qgis::MessageLevel::Warning );
    return;
  }

  QString selectedFile = mFileName.isEmpty() ? QString() : mFileName + ".points";
  mGCPpointsFileName = QFileDialog::getSaveFileName( this, tr( "Save GCP Points" ),
                       selectedFile,
                       tr( "GCP file" ) + " (*.points)" );

  if ( mGCPpointsFileName.isEmpty() )
    return;

  if ( mGCPpointsFileName.right( 7 ) != QLatin1String( ".points" ) )
    mGCPpointsFileName += QLatin1String( ".points" );

  saveGCPs();
}

void QgsGeoreferencerMainWindow::showLayerPropertiesDialog()
{
  if ( mLayer )
  {
    QgisApp::instance()->showLayerProperties( mLayer.get() );
  }
  else
  {
    mMessageBar->pushMessage( tr( "Please load file to be georeferenced." ), Qgis::MessageLevel::Warning );
  }
}

void QgsGeoreferencerMainWindow::showGeorefConfigDialog()
{
  QgsGeorefConfigDialog config;
  if ( config.exec() == QDialog::Accepted )
  {
    mCanvas->refresh();
    QgisApp::instance()->mapCanvas()->refresh();
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
      mGCPListWidget->updateResiduals();
    }
    //and status bar
    updateTransformParamLabel();
  }
}

void QgsGeoreferencerMainWindow::fullHistogramStretch()
{
  if ( mLayer && mLayer->type() == QgsMapLayerType::RasterLayer && mCanvas )
  {
    qobject_cast<QgsRasterLayer *>( mLayer.get() )->setContrastEnhancement( QgsContrastEnhancement::StretchToMinimumMaximum );
    mCanvas->refresh();
  }
}

void QgsGeoreferencerMainWindow::localHistogramStretch()
{
  QgsRectangle rectangle = QgisApp::instance()->mapCanvas()->mapSettings().outputExtentToLayerExtent( mLayer.get(), QgisApp::instance()->mapCanvas()->extent() );

  if ( mLayer && mLayer->type() == QgsMapLayerType::RasterLayer && mCanvas )
  {
    qobject_cast<QgsRasterLayer *>( mLayer.get() )->setContrastEnhancement( QgsContrastEnhancement::StretchToMinimumMaximum, QgsRasterMinMaxOrigin::MinMax, rectangle );
    mCanvas->refresh();
  }
}

void QgsGeoreferencerMainWindow::recenterOnPoint( const QgsPointXY &point )
{
  mCanvas->setCenter( point );
  mCanvas->refresh();
}

void QgsGeoreferencerMainWindow::extentsChangedGeorefCanvas()
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
    QgsRectangle rectMap = mGeorefTransform.transformSourceExtent( mCanvas->extent(), true );
    QgsRectangle boundingBox = transformViewportBoundingBox( rectMap, mGeorefTransform, true );

    mExtentsChangedRecursionGuard = true;
    // Just set the whole extent for now
    // TODO: better fitting function which accounts for differing aspect ratios etc.
    QgisApp::instance()->mapCanvas()->setExtent( boundingBox );
    QgisApp::instance()->mapCanvas()->refresh();
    mExtentsChangedRecursionGuard = false;
  }
}

void QgsGeoreferencerMainWindow::extentsChangedQGisCanvas()
{
  // Guard against endless recursion by ping-pong updates
  if ( mExtentsChangedRecursionGuard )
  {
    return;
  }

  if ( mActionLinkGeorefToQgis->isChecked() )
  {
    // Update transform if necessary
    if ( !updateGeorefTransform() )
    {
      return;
    }

    // Reproject the canvas into raster coordinates and fit axis aligned bounding box
    QgsRectangle boundingBox = transformViewportBoundingBox( QgisApp::instance()->mapCanvas()->extent(), mGeorefTransform, false );
    QgsRectangle rectMap = mGeorefTransform.transformSourceExtent( boundingBox, false );

    mExtentsChangedRecursionGuard = true;
    // Just set the whole extent for now
    // TODO: better fitting function which accounts for differing aspect ratios etc.
    mCanvas->setExtent( rectMap );
    mCanvas->refresh();
    mExtentsChangedRecursionGuard = false;
  }
}

void QgsGeoreferencerMainWindow::updateCanvasRotation()
{
  double degrees = mRotationEdit->value();
  mCanvas->setRotation( degrees );
  mCanvas->refresh();
}

void QgsGeoreferencerMainWindow::showMouseCoords( const QgsPointXY &p )
{
  mCoordsLabel->setText( p.toString( mMousePrecisionDecimalPlaces ) );
  // Set minimum necessary width
  if ( mCoordsLabel->width() > mCoordsLabel->minimumWidth() )
  {
    mCoordsLabel->setMinimumWidth( mCoordsLabel->width() );
  }
}

void QgsGeoreferencerMainWindow::updateMouseCoordinatePrecision()
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

void QgsGeoreferencerMainWindow::createActions()
{
  // File actions
  connect( mActionReset, &QAction::triggered, this, &QgsGeoreferencerMainWindow::reset );

  mActionOpenRaster->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddRasterLayer.svg" ) ) );
  connect( mActionOpenRaster, &QAction::triggered, this, [ = ] { openLayer( QgsMapLayerType::RasterLayer ); } );

  connect( mActionOpenVector, &QAction::triggered, this, [ = ] { openLayer( QgsMapLayerType::VectorLayer ); } );

  mActionStartGeoref->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionStart.svg" ) ) );
  connect( mActionStartGeoref, &QAction::triggered, this, &QgsGeoreferencerMainWindow::georeference );

  mActionGDALScript->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/georeferencer/mActionGDALScript.png" ) ) );
  connect( mActionGDALScript, &QAction::triggered, this, &QgsGeoreferencerMainWindow::generateGDALScript );

  mActionLoadGCPpoints->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/georeferencer/mActionLoadGCPpoints.png" ) ) );
  connect( mActionLoadGCPpoints, &QAction::triggered, this, &QgsGeoreferencerMainWindow::loadGCPsDialog );

  mActionSaveGCPpoints->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/georeferencer/mActionSaveGCPpointsAs.png" ) ) );
  connect( mActionSaveGCPpoints, &QAction::triggered, this, &QgsGeoreferencerMainWindow::saveGCPsDialog );

  mActionTransformSettings->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/settings.svg" ) ) );
  connect( mActionTransformSettings, &QAction::triggered, this, &QgsGeoreferencerMainWindow::showTransformSettingsDialog );

  // Edit actions
  mActionAddPoint->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/georeferencer/mActionAddGCPPoint.png" ) ) );
  connect( mActionAddPoint, &QAction::triggered, this, &QgsGeoreferencerMainWindow::setAddPointTool );

  mActionDeletePoint->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/georeferencer/mActionDeleteGCPPoint.png" ) ) );
  connect( mActionDeletePoint, &QAction::triggered, this, &QgsGeoreferencerMainWindow::setDeletePointTool );

  mActionMoveGCPPoint->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/georeferencer/mActionMoveGCPPoint.png" ) ) );
  connect( mActionMoveGCPPoint, &QAction::triggered, this, &QgsGeoreferencerMainWindow::setMovePointTool );

  // View actions
  mActionPan->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionPan.svg" ) ) );
  connect( mActionPan, &QAction::triggered, this, &QgsGeoreferencerMainWindow::setPanTool );

  mActionZoomIn->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomIn.svg" ) ) );
  connect( mActionZoomIn, &QAction::triggered, this, &QgsGeoreferencerMainWindow::setZoomInTool );

  mActionZoomOut->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomOut.svg" ) ) );
  connect( mActionZoomOut, &QAction::triggered, this, &QgsGeoreferencerMainWindow::setZoomOutTool );

  mActionZoomToLayer->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomToLayer.svg" ) ) );
  connect( mActionZoomToLayer, &QAction::triggered, this, &QgsGeoreferencerMainWindow::zoomToLayerTool );

  mActionZoomLast->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomLast.svg" ) ) );
  connect( mActionZoomLast, &QAction::triggered, this, &QgsGeoreferencerMainWindow::zoomToLast );

  mActionZoomNext->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomNext.svg" ) ) );
  connect( mActionZoomNext, &QAction::triggered, this, &QgsGeoreferencerMainWindow::zoomToNext );

  mActionLinkGeorefToQgis->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/georeferencer/mActionLinkGeorefToQgis.png" ) ) );
  connect( mActionLinkGeorefToQgis, &QAction::toggled, this, &QgsGeoreferencerMainWindow::linkGeorefToQgis );

  mActionLinkQGisToGeoref->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/georeferencer/mActionLinkQGisToGeoref.png" ) ) );
  connect( mActionLinkQGisToGeoref, &QAction::toggled, this, &QgsGeoreferencerMainWindow::linkQGisToGeoref );

  // Settings actions
  mActionSourceProperties->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRasterProperties.png" ) ) );
  connect( mActionSourceProperties, &QAction::triggered, this, &QgsGeoreferencerMainWindow::showLayerPropertiesDialog );

  mActionGeorefConfig->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionGeorefConfig.png" ) ) );
  connect( mActionGeorefConfig, &QAction::triggered, this, &QgsGeoreferencerMainWindow::showGeorefConfigDialog );

  // Histogram stretch
  mActionLocalHistogramStretch->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionLocalHistogramStretch.svg" ) ) );
  connect( mActionLocalHistogramStretch, &QAction::triggered, this, &QgsGeoreferencerMainWindow::localHistogramStretch );
  mActionLocalHistogramStretch->setEnabled( false );

  mActionFullHistogramStretch->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFullHistogramStretch.svg" ) ) );
  connect( mActionFullHistogramStretch, &QAction::triggered, this, &QgsGeoreferencerMainWindow::fullHistogramStretch );
  mActionFullHistogramStretch->setEnabled( false );

  mActionQuit->setShortcuts( QList<QKeySequence>() << QKeySequence( QStringLiteral( "CTRL+Q" ) )
                             << QKeySequence( Qt::Key_Escape ) );
  connect( mActionQuit, &QAction::triggered, this, &QWidget::close );
}

void QgsGeoreferencerMainWindow::createActionGroups()
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

void QgsGeoreferencerMainWindow::createMapCanvas()
{
  // set up the canvas
  mCanvas = new QgsMapCanvas( this->centralWidget() );
  mCanvas->setObjectName( QStringLiteral( "georefCanvas" ) );
  mCanvas->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
  mCanvas->setCanvasColor( Qt::white );
  mCanvas->setMinimumWidth( 400 );
  mCanvas->setCachingEnabled( true );
  mCanvas->setParallelRenderingEnabled( true );
  mCanvas->setPreviewJobsEnabled( true );

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
           this, &QgsGeoreferencerMainWindow::showCoordDialog );

  mToolDeletePoint = new QgsGeorefToolDeletePoint( mCanvas );
  mToolDeletePoint->setAction( mActionDeletePoint );
  connect( mToolDeletePoint, &QgsGeorefToolDeletePoint::deleteDataPoint,
           this, static_cast<void ( QgsGeoreferencerMainWindow::* )( QPoint )>( &QgsGeoreferencerMainWindow::deleteDataPoint ) );

  mToolMovePoint = new QgsGeorefToolMovePoint( mCanvas );
  mToolMovePoint->setAction( mActionMoveGCPPoint );
  connect( mToolMovePoint, &QgsGeorefToolMovePoint::pointPressed,
           this, &QgsGeoreferencerMainWindow::selectPoint );
  connect( mToolMovePoint, &QgsGeorefToolMovePoint::pointMoved,
           this, &QgsGeoreferencerMainWindow::movePoint );
  connect( mToolMovePoint, &QgsGeorefToolMovePoint::pointReleased,
           this, &QgsGeoreferencerMainWindow::releasePoint );

  // Point in QGIS Map
  mToolMovePointQgis = new QgsGeorefToolMovePoint( QgisApp::instance()->mapCanvas() );
  mToolMovePointQgis->setAction( mActionMoveGCPPoint );
  connect( mToolMovePointQgis, &QgsGeorefToolMovePoint::pointPressed,
           this, &QgsGeoreferencerMainWindow::selectPoint );
  connect( mToolMovePointQgis, &QgsGeorefToolMovePoint::pointMoved,
           this, &QgsGeoreferencerMainWindow::movePoint );
  connect( mToolMovePointQgis, &QgsGeorefToolMovePoint::pointReleased,
           this, &QgsGeoreferencerMainWindow::releasePoint );

  QgsSettings s;
  double zoomFactor = s.value( QStringLiteral( "/qgis/zoom_factor" ), 2 ).toDouble();
  mCanvas->setWheelFactor( zoomFactor );

  mExtentsChangedRecursionGuard = false;

  mGeorefTransform.setMethod( QgsGcpTransformerInterface::TransformMethod::Linear );
  mGCPsDirty = true;

  // Connect main canvas and georef canvas signals so we are aware if any of the viewports change
  // (used by the map follow mode)
  connect( mCanvas, &QgsMapCanvas::extentsChanged, this, &QgsGeoreferencerMainWindow::extentsChangedGeorefCanvas );
  connect( QgisApp::instance()->mapCanvas(), &QgsMapCanvas::extentsChanged, this, &QgsGeoreferencerMainWindow::extentsChangedQGisCanvas );
}

void QgsGeoreferencerMainWindow::createMenus()
{
  // Get platform for menu layout customization (Gnome, Kde, Mac, Win)
  QDialogButtonBox::ButtonLayout layout =
    QDialogButtonBox::ButtonLayout( style()->styleHint( QStyle::SH_DialogButtonLayout, nullptr, this ) );

  mPanelMenu = new QMenu( tr( "Panels" ) );
  mPanelMenu->setObjectName( QStringLiteral( "mPanelMenu" ) );
  mPanelMenu->addAction( dockWidgetGCPpoints->toggleViewAction() );

  mToolbarMenu = new QMenu( tr( "Toolbars" ) );
  mToolbarMenu->setObjectName( QStringLiteral( "mToolbarMenu" ) );
  mToolbarMenu->addAction( toolBarFile->toggleViewAction() );
  mToolbarMenu->addAction( toolBarEdit->toggleViewAction() );
  mToolbarMenu->addAction( toolBarView->toggleViewAction() );

  toolBarFile->setIconSize( QgisApp::instance()->iconSize() );
  toolBarEdit->setIconSize( QgisApp::instance()->iconSize() );
  toolBarView->setIconSize( QgisApp::instance()->iconSize() );
  toolBarHistogramStretch->setIconSize( QgisApp::instance()->iconSize() );

  // View menu
  if ( layout != QDialogButtonBox::KdeLayout )
  {
    menuView->addSeparator();
    menuView->addMenu( mPanelMenu );
    menuView->addMenu( mToolbarMenu );
  }
  else
  {
    menuSettings->addSeparator();
    menuSettings->addMenu( mPanelMenu );
    menuSettings->addMenu( mToolbarMenu );
  }
}

void QgsGeoreferencerMainWindow::createDockWidgets()
{
  mGCPListWidget = new QgsGCPListWidget( this );
  mGCPListWidget->setGeorefTransform( &mGeorefTransform );
  dockWidgetGCPpoints->setWidget( mGCPListWidget );

  connect( mGCPListWidget, &QgsGCPListWidget::jumpToGCP, this, &QgsGeoreferencerMainWindow::recenterOnPoint );
  connect( mGCPListWidget, static_cast<void ( QgsGCPListWidget::* )( int )>( &QgsGCPListWidget::deleteDataPoint ),
           this, static_cast<void ( QgsGeoreferencerMainWindow::* )( int )>( &QgsGeoreferencerMainWindow::deleteDataPoint ) );
  connect( mGCPListWidget, &QgsGCPListWidget::pointEnabled, this, &QgsGeoreferencerMainWindow::updateGeorefTransform );
}

QLabel *QgsGeoreferencerMainWindow::createBaseLabelStatus()
{
  QLabel *label = new QLabel( statusBar() );
  label->setFont( statusBarFont() );
  label->setMinimumWidth( 10 );
  label->setMaximumHeight( 20 );
  label->setMargin( 3 );
  label->setAlignment( Qt::AlignCenter );
  label->setFrameStyle( QFrame::NoFrame );
  return label;
}

QFont QgsGeoreferencerMainWindow::statusBarFont()
{
  // Drop the font size in the status bar by a couple of points (match main window)
  QFont barFont = font();
  int fontSize = barFont.pointSize();
#ifdef Q_OS_WIN
  fontSize = std::max( fontSize - 1, 8 ); // bit less on windows, due to poor rendering of small point sizes
#else
  fontSize = std::max( fontSize - 2, 6 );
#endif
  barFont.setPointSize( fontSize );
  return barFont;
}

void QgsGeoreferencerMainWindow::createStatusBar()
{
  statusBar()->setFont( statusBarFont() );

  // add a widget to show/set current rotation
  mRotationLabel = createBaseLabelStatus();
  mRotationLabel->setObjectName( QStringLiteral( "mRotationLabel" ) );
  mRotationLabel->setText( tr( "Rotation" ) );
  mRotationLabel->setToolTip( tr( "Current clockwise map rotation in degrees" ) );
  statusBar()->addPermanentWidget( mRotationLabel, 0 );

  mRotationEdit = new QgsDoubleSpinBox( statusBar() );
  mRotationEdit->setObjectName( QStringLiteral( "mRotationEdit" ) );
  mRotationEdit->setClearValue( 0.0 );
  mRotationEdit->setKeyboardTracking( false );
  mRotationEdit->setMaximumWidth( 120 );
  mRotationEdit->setDecimals( 1 );
  mRotationEdit->setRange( -360.0, 360.0 );
  mRotationEdit->setWrapping( true );
  mRotationEdit->setSingleStep( 5.0 );
  mRotationEdit->setSuffix( tr( " °" ) );
  mRotationEdit->setToolTip( tr( "Current clockwise map rotation in degrees" ) );
  statusBar()->addPermanentWidget( mRotationEdit, 0 );

  mTransformParamLabel = createBaseLabelStatus();
  mTransformParamLabel->setText( tr( "Transform: " ) + QgsGcpTransformerInterface::methodToString( mTransformMethod ) );
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

void QgsGeoreferencerMainWindow::setupConnections()
{
  connect( mCanvas, &QgsMapCanvas::xyCoordinates, this, &QgsGeoreferencerMainWindow::showMouseCoords );
  connect( mCanvas, &QgsMapCanvas::scaleChanged, this, &QgsGeoreferencerMainWindow::updateMouseCoordinatePrecision );

  // Connect status from ZoomLast/ZoomNext to corresponding action
  connect( mCanvas, &QgsMapCanvas::zoomLastStatusChanged, mActionZoomLast, &QAction::setEnabled );
  connect( mCanvas, &QgsMapCanvas::zoomNextStatusChanged, mActionZoomNext, &QAction::setEnabled );

  // Connect mapCanvas rotation widget
  connect( mRotationEdit, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, &QgsGeoreferencerMainWindow::updateCanvasRotation );
  connect( QgisApp::instance()->mapCanvas(), &QgsMapCanvas::destinationCrsChanged, this, &QgsGeoreferencerMainWindow::invalidateCanvasCoords );
}

void QgsGeoreferencerMainWindow::removeOldLayer()
{
  mLayer.reset();

  mCanvas->setLayers( QList<QgsMapLayer *>() );
  mCanvas->clearCache();
  mRotationEdit->clear();
  mCanvas->refresh();
}

void QgsGeoreferencerMainWindow::loadSource( QgsMapLayerType layerType, const QString &uri, const QString &provider )
{
  switch ( layerType )
  {
    case QgsMapLayerType::VectorLayer:
    {
      QgsVectorLayer::LayerOptions options( QgsProject::instance()->transformContext() );
      // never prompt for a crs selection for the input layer!
      options.skipCrsValidation = true;
      mLayer = std::make_unique< QgsVectorLayer >( uri, QStringLiteral( "Vector" ), provider, options );
      break;
    }

    case QgsMapLayerType::RasterLayer:
    {
      QgsRasterLayer::LayerOptions options( true, QgsProject::instance()->transformContext() );
      // never prompt for a crs selection for the input raster!
      options.skipCrsValidation = true;
      mLayer = std::make_unique< QgsRasterLayer >( uri, QStringLiteral( "Raster" ), provider, options );
      break;
    }

    case QgsMapLayerType::PluginLayer:
    case QgsMapLayerType::MeshLayer:
    case QgsMapLayerType::VectorTileLayer:
    case QgsMapLayerType::AnnotationLayer:
    case QgsMapLayerType::PointCloudLayer:
    case QgsMapLayerType::GroupLayer:
      Q_ASSERT_X( false, "QgsGeoreferencerMainWindow::loadSource", "unsupported layer type" );
      return;
  }

  // guess a reasonable target CRS to use by default
  if ( mLayer->crs().isValid() )
  {
    // if source raster already is already georeferenced, assume we'll be keeping the same CRS
    mTargetCrs = mLayer->crs();
  }
  // otherwise use the previous target crs, unless that's never been set
  else if ( !mTargetCrs.isValid() )
  {
    // in which case we'll use the current project CRS
    mTargetCrs = QgsProject::instance()->crs();
  }

  // add layer to map canvas
  mCanvas->setLayers( QList<QgsMapLayer *>() << mLayer.get() );

  mActionLocalHistogramStretch->setEnabled( layerType == QgsMapLayerType::RasterLayer );
  mActionFullHistogramStretch->setEnabled( layerType == QgsMapLayerType::RasterLayer );

  // Status Bar
  if ( mGeorefTransform.hasExistingGeoreference() )
  {
    QString authid = mLayer->crs().authid();
    mEPSG->setText( authid );
    mEPSG->setToolTip( mLayer->crs().toProj() );
  }
  else
  {
    mEPSG->setText( tr( "None" ) );

    switch ( layerType )
    {
      case QgsMapLayerType::VectorLayer:
        mEPSG->setToolTip( tr( "Source coordinate" ) );
        break;
      case QgsMapLayerType::RasterLayer:
        mEPSG->setToolTip( tr( "Coordinate of image (column/line)" ) );
        break;
      case QgsMapLayerType::PluginLayer:
      case QgsMapLayerType::MeshLayer:
      case QgsMapLayerType::VectorTileLayer:
      case QgsMapLayerType::AnnotationLayer:
      case QgsMapLayerType::PointCloudLayer:
      case QgsMapLayerType::GroupLayer:
        break;
    }
  }
}

void QgsGeoreferencerMainWindow::readSettings()
{
  QgsSettings s;
  QRect georefRect = QApplication::desktop()->screenGeometry( QgisApp::instance() );
  resize( s.value( QStringLiteral( "/Plugin-GeoReferencer/size" ), QSize( georefRect.width() / 2 + georefRect.width() / 5,
                   QgisApp::instance()->height() ) ).toSize() );
  move( s.value( QStringLiteral( "/Plugin-GeoReferencer/pos" ), QPoint( parentWidget()->width() / 2 - width() / 2, 0 ) ).toPoint() );
  restoreState( s.value( QStringLiteral( "/Plugin-GeoReferencer/uistate" ) ).toByteArray() );

  // warp options
  mResamplingMethod = settingResamplingMethod.value();
  mCompressionMethod = settingCompressionMethod.value();
  mUseZeroForTrans = settingUseZeroForTransparent.value();
  mTransformMethod = settingTransformMethod.value();
  mSaveGcp = settingSaveGcps.value();
  mLoadInQgis = settingLoadInProject.value();
}

void QgsGeoreferencerMainWindow::writeSettings()
{
  QgsSettings s;
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/pos" ), pos() );
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/size" ), size() );
  s.setValue( QStringLiteral( "/Plugin-GeoReferencer/uistate" ), saveState() );

  settingTransformMethod.setValue( mTransformMethod );
  settingResamplingMethod.setValue( mResamplingMethod );
  settingCompressionMethod.setValue( mCompressionMethod );
  settingUseZeroForTransparent.setValue( mUseZeroForTrans );
  settingSaveGcps.setValue( mSaveGcp );
  settingLoadInProject.setValue( mLoadInQgis );
}

bool QgsGeoreferencerMainWindow::loadGCPs( QString &error )
{
  QgsCoordinateReferenceSystem actualDestinationCrs;
  const QList< QgsGcpPoint > points = QgsGCPList::loadGcps( mGCPpointsFileName,
                                      mTargetCrs,
                                      actualDestinationCrs,
                                      error );
  if ( !error.isEmpty() )
    return false;

  mTargetCrs = actualDestinationCrs;
  clearGCPData();
  mGCPListWidget->setTargetCrs( actualDestinationCrs, QgsProject::instance()->transformContext() );

  for ( const QgsGcpPoint &point : points )
  {
    addPoint( point.sourcePoint(), point.destinationPoint(), point.destinationPointCrs(), point.isEnabled(), false );
  }

  mSavedPoints = mPoints.asPoints();
  if ( mGCPsDirty )
  {
    mGCPListWidget->setGCPList( &mPoints );
    updateGeorefTransform();
    mCanvas->refresh();
    QgisApp::instance()->mapCanvas()->refresh();
  }

  return true;
}

void QgsGeoreferencerMainWindow::saveGCPs()
{
  QString error;
  if ( mPoints.saveGcps( mGCPpointsFileName, mTargetCrs, QgsProject::instance()->transformContext(), error ) )
  {
    mSavedPoints = mPoints.asPoints();
  }
  else
  {
    mMessageBar->pushMessage( tr( "Write Error" ), error, Qgis::MessageLevel::Critical );
  }
}

QgsGeoreferencerMainWindow::SaveGCPs QgsGeoreferencerMainWindow::checkNeedGCPSave()
{
  if ( 0 == mPoints.count() )
    return QgsGeoreferencerMainWindow::GCPDISCARD;

  if ( !equalGCPlists( mSavedPoints, mPoints ) )
  {
    QMessageBox::StandardButton a = QMessageBox::question( this, tr( "Save GCPs" ),
                                    tr( "Save GCP points?" ),
                                    QMessageBox::Save | QMessageBox::Discard
                                    | QMessageBox::Cancel );
    if ( a == QMessageBox::Save )
    {
      return QgsGeoreferencerMainWindow::GCPSAVE;
    }
    else if ( a == QMessageBox::Cancel )
    {
      return QgsGeoreferencerMainWindow::GCPCANCEL;
    }
  }
  return QgsGeoreferencerMainWindow::GCPDISCARD;
}

bool QgsGeoreferencerMainWindow::georeference()
{
  if ( !validate() )
    return false;

  if ( mLayer->type() == QgsMapLayerType::RasterLayer && mCreateWorldFileOnly && ( QgsGcpTransformerInterface::TransformMethod::Linear == mGeorefTransform.transformParametrisation() ||
       QgsGcpTransformerInterface::TransformMethod::Helmert == mGeorefTransform.transformParametrisation() ) )
  {
    QgsPointXY origin;
    double pixelXSize, pixelYSize, rotation;
    if ( !mGeorefTransform.getOriginScaleRotation( origin, pixelXSize, pixelYSize, rotation ) )
    {
      mMessageBar->pushMessage( tr( "Transform Failed" ), tr( "Failed to calculate linear transform parameters." ), Qgis::MessageLevel::Critical );
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
  else
  {
    switch ( mLayer->type() )
    {
      case QgsMapLayerType::VectorLayer:
        return georeferenceVector();

      case QgsMapLayerType::RasterLayer:
        return georeferenceRaster();

      case QgsMapLayerType::PluginLayer:
      case QgsMapLayerType::MeshLayer:
      case QgsMapLayerType::VectorTileLayer:
      case QgsMapLayerType::AnnotationLayer:
      case QgsMapLayerType::PointCloudLayer:
      case QgsMapLayerType::GroupLayer:
        break;
    }
    return false;
  }
}

bool QgsGeoreferencerMainWindow::georeferenceRaster()
{

  QgsImageWarperTask *task = new QgsImageWarperTask(
    mFileName,
    mModifiedFileName,
    mGeorefTransform,
    mResamplingMethod,
    mUseZeroForTrans,
    mCompressionMethod,
    mTargetCrs,
    mUserResX,
    mUserResY );

  std::unique_ptr< QProgressDialog > progressDialog = std::make_unique< QProgressDialog >( tr( "Georeferencing layer…" ), tr( "Abort" ), 0, 100, this );
  progressDialog->setWindowTitle( tr( "Georeferencer" ) );
  connect( task, &QgsTask::progressChanged, progressDialog.get(), [ & ]( double progress )
  {
    progressDialog->setValue( static_cast< int >( progress ) );
  } );
  connect( progressDialog.get(), &QProgressDialog::canceled, task, [ & ]
  {
    task->cancel();
  } );
  progressDialog->show();

  // task is running in a thread, so we don't strictly need to show a blocking progress dialog here... but we don't have a good ui
  // for illustrating that the task is running in the main window otherwise, so stick with the dialog for now
  QEventLoop loop;
  bool result = true;
  connect( task, &QgsTask::taskCompleted, &loop, [&loop, this]
  {
    if ( !mPdfOutputFile.isEmpty() )
    {
      writePDFReportFile( mPdfOutputFile, mGeorefTransform );
    }
    if ( !mPdfOutputMapFile.isEmpty() )
    {
      writePDFMapFile( mPdfOutputMapFile, mGeorefTransform );
    }
    if ( mSaveGcp )
    {
      mGCPpointsFileName = mFileName + QLatin1String( ".points" );
      saveGCPs();
    }

    mMessageBar->pushMessage( tr( "Georeference Successful" ), tr( "Raster was successfully georeferenced." ), Qgis::MessageLevel::Success );
    if ( mLoadInQgis )
    {
      const QString layerSource = mCreateWorldFileOnly ? mFileName : mModifiedFileName;
      QgisApp::instance()->addRasterLayer( layerSource, QFileInfo( layerSource ).completeBaseName(), QStringLiteral( "gdal" ) );
    }

    loop.quit();
  } );

  connect( task, &QgsTask::taskTerminated, &loop, [&loop, this, task, &result]
  {
    switch ( task->result() )
    {
      case QgsImageWarper::Result::Success:
        break;
      case QgsImageWarper::Result::Canceled:
      {
        QFileInfo fi( mModifiedFileName );
        fi.dir().remove( mModifiedFileName );
        break;
      }
      case QgsImageWarper::Result::InvalidParameters:
        mMessageBar->pushMessage( tr( "Transform Failed" ), tr( "Failed to compute GCP transform: Transform is not solvable." ), Qgis::MessageLevel::Critical );
        break;
      case QgsImageWarper::Result::SourceError:
        mMessageBar->pushMessage( tr( "Transform Failed" ), tr( "Could not read source image." ), Qgis::MessageLevel::Critical );
        break;
      case QgsImageWarper::Result::TransformError:
        mMessageBar->pushMessage( tr( "Transform Failed" ), tr( "Error creating GDAL transformation." ), Qgis::MessageLevel::Critical );
        break;
      case QgsImageWarper::Result::DestinationCreationError:
        mMessageBar->pushMessage( tr( "Transform Failed" ), tr( "Could not create destination file." ), Qgis::MessageLevel::Critical );
        break;
      case QgsImageWarper::Result::WarpFailure:
        mMessageBar->pushMessage( tr( "Transform Failed" ), tr( "Error occurred while warping image." ), Qgis::MessageLevel::Critical );
        break;
    }
    loop.quit();
    result = false;
  } );

  QgsApplication::taskManager()->addTask( task );
  loop.exec();
  return result;
}

bool QgsGeoreferencerMainWindow::georeferenceVector()
{
  QgsVectorWarperTask *task = new QgsVectorWarperTask( mTransformMethod,
      mPoints.asPoints(), mTargetCrs, qobject_cast< QgsVectorLayer * >( mLayer.get() ), mModifiedFileName );

  std::unique_ptr< QProgressDialog > progressDialog = std::make_unique< QProgressDialog >( tr( "Georeferencing layer…" ), tr( "Abort" ), 0, 100, this );
  progressDialog->setWindowTitle( tr( "Georeferencer" ) );
  connect( task, &QgsTask::progressChanged, progressDialog.get(), [ & ]( double progress )
  {
    progressDialog->setValue( static_cast< int >( progress ) );
  } );
  connect( progressDialog.get(), &QProgressDialog::canceled, task, [ & ]
  {
    task->cancel();
  } );
  progressDialog->show();

  // task is running in a thread, so we don't strictly need to show a blocking progress dialog here... but we don't have a good ui
  // for illustrating that the task is running in the main window otherwise, so stick with the dialog for now
  QEventLoop loop;
  bool result = true;
  connect( task, &QgsTask::taskCompleted, &loop, [&loop, this]
  {
    if ( !mPdfOutputFile.isEmpty() )
    {
      writePDFReportFile( mPdfOutputFile, mGeorefTransform );
    }
    if ( !mPdfOutputMapFile.isEmpty() )
    {
      writePDFMapFile( mPdfOutputMapFile, mGeorefTransform );
    }
    if ( mSaveGcp )
    {
      mGCPpointsFileName = mFileName + QLatin1String( ".points" );
      saveGCPs();
    }
    mMessageBar->pushMessage( tr( "Georeference Successful" ), tr( "Vector layer was successfully georeferenced." ), Qgis::MessageLevel::Success );
    if ( mLoadInQgis )
    {
      QgisApp::instance()->addVectorLayer( mModifiedFileName, QFileInfo( mModifiedFileName ).completeBaseName(), QStringLiteral( "ogr" ) );
    }

    loop.quit();
  } );

  connect( task, &QgsTask::taskTerminated, &loop, [&loop, this, task, &result]
  {
    if ( task->result() != QgsVectorWarperTask::Result::Canceled )
    {
      mMessageBar->pushMessage( tr( "Transform Failed" ), task->errorMessage(), Qgis::MessageLevel::Critical );
    }
    loop.quit();
    result = false;
  } );

  QgsApplication::taskManager()->addTask( task );
  loop.exec();
  return result;
}

bool QgsGeoreferencerMainWindow::writeWorldFile( const QgsPointXY &origin, double pixelXSize, double pixelYSize, double rotation )
{
  // write the world file
  QFile file( mWorldFileName );
  if ( !file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    mMessageBar->pushMessage( tr( "Save World File" ), tr( "Could not write to %1." ).arg( mWorldFileName ), Qgis::MessageLevel::Critical );
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

bool QgsGeoreferencerMainWindow::calculateMeanError( double &error ) const
{
  if ( mGeorefTransform.transformParametrisation() == QgsGcpTransformerInterface::TransformMethod::InvalidTransform )
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

  if ( nPointsEnabled == mGeorefTransform.minimumGcpCount() )
  {
    error = 0;
    return true;
  }
  else if ( nPointsEnabled < mGeorefTransform.minimumGcpCount() )
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
  error = std::sqrt( ( sumVxSquare + sumVySquare ) / ( nPointsEnabled - mGeorefTransform.minimumGcpCount() ) );
  return true;
}

bool QgsGeoreferencerMainWindow::writePDFMapFile( const QString &fileName, const QgsGeorefTransform &transform )
{
  Q_UNUSED( transform )
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
  std::unique_ptr< QgsLayoutItemPage > page = std::make_unique< QgsLayoutItemPage >( &layout );

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

bool QgsGeoreferencerMainWindow::writePDFReportFile( const QString &fileName, const QgsGeorefTransform &transform )
{
  if ( !mCanvas )
  {
    return false;
  }

  //create layout A4 with 300 dpi
  QgsLayout layout( QgsProject::instance() );

  std::unique_ptr< QgsLayoutItemPage > page = std::make_unique< QgsLayoutItemPage >( &layout );
  page->setPageSize( QgsLayoutSize( 210, 297 ) ); //A4
  layout.pageCollection()->addPage( page.release() );
  std::unique_ptr< QgsLayoutItemPage > page2 = std::make_unique< QgsLayoutItemPage >( &layout );
  page2->setPageSize( QgsLayoutSize( 210, 297 ) ); //A4
  layout.pageCollection()->addPage( page2.release() );

  QFont titleFont;
  titleFont.setBold( true );
  QgsTextFormat titleFormat;
  titleFormat.setFont( titleFont );
  titleFormat.setSize( 9 );
  titleFormat.setSizeUnit( QgsUnitTypes::RenderPoints );

  QFont tableHeaderFont;
  tableHeaderFont.setPointSize( 9 );
  tableHeaderFont.setBold( true );
  QgsTextFormat tableHeaderFormat = QgsTextFormat::fromQFont( tableHeaderFont );
  QFont tableContentFont;
  tableContentFont.setPointSize( 9 );
  QgsTextFormat tableContentFormat = QgsTextFormat::fromQFont( tableContentFont );

  QgsSettings s;
  double leftMargin = s.value( QStringLiteral( "/Plugin-GeoReferencer/Config/LeftMarginPDF" ), "2.0" ).toDouble();
  double rightMargin = s.value( QStringLiteral( "/Plugin-GeoReferencer/Config/RightMarginPDF" ), "2.0" ).toDouble();
  double contentWidth = 210 - ( leftMargin + rightMargin );

  //title
  QFileInfo rasterFi( mFileName );
  QgsLayoutItemLabel *titleLabel = new QgsLayoutItemLabel( &layout );
  titleLabel->setTextFormat( titleFormat );
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
    QString parameterTitle = tr( "Transformation parameters" ) + QStringLiteral( " (" ) + QgsGcpTransformerInterface::methodToString( transform.transformParametrisation() ) + QStringLiteral( ")" );
    parameterLabel = new QgsLayoutItemLabel( &layout );
    parameterLabel->setTextFormat( titleFormat );
    parameterLabel->setText( parameterTitle );
    parameterLabel->adjustSizeToText();
    layout.addLayoutItem( parameterLabel );
    parameterLabel->attemptSetSceneRect( QRectF( leftMargin, layoutMap->rect().bottom() + layoutMap->pos().y() + 5, contentWidth, 8 ) );
    parameterLabel->setFrameEnabled( false );

    //calculate mean error
    double meanError = 0;
    calculateMeanError( meanError );

    parameterTable = new QgsLayoutItemTextTable( &layout );
    parameterTable->setHeaderTextFormat( tableHeaderFormat );
    parameterTable->setContentTextFormat( tableContentFormat );

    QgsLayoutTableColumns columns;
    columns << QgsLayoutTableColumn( tr( "Translation x" ) )
            << QgsLayoutTableColumn( tr( "Translation y" ) )
            << QgsLayoutTableColumn( tr( "Scale x" ) )
            << QgsLayoutTableColumn( tr( "Scale y" ) )
            << QgsLayoutTableColumn( tr( "Rotation [degrees]" ) )
            << QgsLayoutTableColumn( tr( "Mean error [%1]" ).arg( residualUnits ) );

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
  residualLabel->setTextFormat( titleFormat );
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
  gcpTable->setHeaderTextFormat( tableHeaderFormat );
  gcpTable->setContentTextFormat( tableContentFormat );
  gcpTable->setHeaderMode( QgsLayoutTable::AllFrames );
  QgsLayoutTableColumns columns;
  columns << QgsLayoutTableColumn( tr( "ID" ) )
          << QgsLayoutTableColumn( tr( "Enabled" ) )
          << QgsLayoutTableColumn( tr( "Pixel X" ) )
          << QgsLayoutTableColumn( tr( "Pixel Y" ) )
          << QgsLayoutTableColumn( tr( "Map X" ) )
          << QgsLayoutTableColumn( tr( "Map Y" ) )
          << QgsLayoutTableColumn( tr( "Res X (%1)" ).arg( residualUnits ) )
          << QgsLayoutTableColumn( tr( "Res Y (%1)" ).arg( residualUnits ) )
          << QgsLayoutTableColumn( tr( "Res Total (%1)" ).arg( residualUnits ) );

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

    const QgsPointXY transformedDestinationPoint = ( *gcpIt )->transformedDestinationPoint( mTargetCrs, QgsProject::instance()->transformContext() );

    currentGCPStrings << QString::number( ( *gcpIt )->sourcePoint().x(), 'f', 0 ) << QString::number( ( *gcpIt )->sourcePoint().y(), 'f', 0 ) << QString::number( transformedDestinationPoint.x(), 'f', 3 )
                      <<  QString::number( transformedDestinationPoint.y(), 'f', 3 ) <<  QString::number( residual.x() ) <<  QString::number( residual.y() ) << QString::number( residualTot );
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

void QgsGeoreferencerMainWindow::updateTransformParamLabel()
{
  if ( !mTransformParamLabel )
  {
    return;
  }

  QString transformName = QgsGcpTransformerInterface::methodToString( mGeorefTransform.transformParametrisation() );
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

void QgsGeoreferencerMainWindow::showGDALScript( const QStringList &commands )
{
  QString script = commands.join( QLatin1Char( '\n' ) ) + '\n';

  // create window to show gdal script
  QDialogButtonBox *bbxGdalScript = new QDialogButtonBox( QDialogButtonBox::Cancel, Qt::Horizontal, this );
  QPushButton *pbnCopyInClipBoard = new QPushButton( QgsApplication::getThemeIcon( QStringLiteral( "/mActionEditPaste.svg" ) ),
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

QString QgsGeoreferencerMainWindow::generateGDALtranslateCommand( bool generateTFW )
{
  QStringList gdalCommand;
  gdalCommand << QStringLiteral( "gdal_translate" ) << QStringLiteral( "-of GTiff" );
  if ( generateTFW )
  {
    // say gdal generate associated ESRI world file
    gdalCommand << QStringLiteral( "-co TFW=YES" );
  }

  for ( QgsGeorefDataPoint *pt : std::as_const( mPoints ) )
  {
    const QgsPointXY transformedDestinationPoint = pt->transformedDestinationPoint( mTargetCrs, QgsProject::instance()->transformContext() );
    gdalCommand << QStringLiteral( "-gcp %1 %2 %3 %4" ).arg( pt->sourcePoint().x() ).arg( -pt->sourcePoint().y() )
                .arg( transformedDestinationPoint.x() ).arg( transformedDestinationPoint.y() );
  }

  QFileInfo rasterFileInfo( mFileName );
  mTranslatedFileName = QDir::tempPath() + '/' + rasterFileInfo.fileName();
  gdalCommand << QStringLiteral( "\"%1\"" ).arg( mFileName ) << QStringLiteral( "\"%1\"" ).arg( mTranslatedFileName );

  return gdalCommand.join( QLatin1Char( ' ' ) );
}

QString QgsGeoreferencerMainWindow::generateGDALogr2ogrCommand() const
{
  QStringList gdalCommand;
  gdalCommand << QStringLiteral( "ogr2ogr" );

  for ( QgsGeorefDataPoint *pt : std::as_const( mPoints ) )
  {
    const QgsPointXY dest = pt->transformedDestinationPoint( mTargetCrs, QgsProject::instance()->transformContext() );
    gdalCommand << QStringLiteral( "-gcp %1 %2 %3 %4" ).arg( pt->sourcePoint().x() ).arg( pt->sourcePoint().y() )
                .arg( dest.x() ).arg( dest.y() );
  }

  switch ( mTransformMethod )
  {
    case QgsGcpTransformerInterface::TransformMethod::PolynomialOrder1:
      gdalCommand << QStringLiteral( "-order 1" );
      break;
    case QgsGcpTransformerInterface::TransformMethod::PolynomialOrder2:
      gdalCommand << QStringLiteral( "-order 2" );
      break;
    case QgsGcpTransformerInterface::TransformMethod::PolynomialOrder3:
      gdalCommand << QStringLiteral( "-order 3" );
      break;
    default:
      gdalCommand << QStringLiteral( "-tps" );
  }

  if ( mTargetCrs.authid().startsWith( QStringLiteral( "EPSG:" ), Qt::CaseInsensitive ) )
  {
    gdalCommand << QStringLiteral( "-t_srs %1" ).arg( mTargetCrs.authid() );
  }
  else
  {
    gdalCommand << QStringLiteral( "-t_srs \"%1\"" ).arg( mTargetCrs.toProj().simplified() );
  }

  const QVariantMap parts = QgsProviderRegistry::instance()->decodeUri( mLayer->providerType(), mLayer->source() );
  const QString sourcePath = parts.value( QStringLiteral( "path" ) ).toString();

  gdalCommand << QStringLiteral( "\"%1\"" ).arg( mModifiedFileName ) << QStringLiteral( "\"%1\"" ).arg( sourcePath );

  return gdalCommand.join( QLatin1Char( ' ' ) );
}

QString QgsGeoreferencerMainWindow::generateGDALwarpCommand( const QString &resampling, const QString &compress,
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

  if ( mTargetCrs.authid().startsWith( QStringLiteral( "EPSG:" ), Qt::CaseInsensitive ) )
  {
    gdalCommand << QStringLiteral( "-t_srs %1" ).arg( mTargetCrs.authid() );
  }
  else
  {
    gdalCommand << QStringLiteral( "-t_srs \"%1\"" ).arg( mTargetCrs.toProj().simplified() );
  }

  gdalCommand << QStringLiteral( "\"%1\"" ).arg( mTranslatedFileName ) << QStringLiteral( "\"%1\"" ).arg( mModifiedFileName );

  return gdalCommand.join( QLatin1Char( ' ' ) );
}

bool QgsGeoreferencerMainWindow::validate()
{
  if ( mFileName.isEmpty() )
  {
    mMessageBar->pushMessage( tr( "No Layer Loaded" ), tr( "Please load layer to be georeferenced." ), Qgis::MessageLevel::Warning );
    return false;
  }

  if ( QgsGcpTransformerInterface::TransformMethod::InvalidTransform == mTransformMethod )
  {
    QMessageBox::information( this, tr( "Georeferencer" ), tr( "Please set transformation type." ) );
    showTransformSettingsDialog();
    return false;
  }

  if ( mCreateWorldFileOnly
       && ( QgsGcpTransformerInterface::TransformMethod::Linear != mTransformMethod && QgsGcpTransformerInterface::TransformMethod::Helmert != mTransformMethod ) )
  {
    QMessageBox::information( this, tr( "Georeferencer" ), tr( "Please set output file name." ) );
    showTransformSettingsDialog();
    return false;
  }

  if ( mPoints.count() < static_cast<int>( mGeorefTransform.minimumGcpCount() ) )
  {
    mMessageBar->pushMessage( tr( "Not Enough GCPs" ), tr( "%1 transformation requires at least %n GCPs. Please define more.", nullptr, mGeorefTransform.minimumGcpCount() )
                              .arg( QgsGcpTransformerInterface::methodToString( mTransformMethod ) )
                              , Qgis::MessageLevel::Critical );
    return false;
  }

  // Update the transform if necessary
  if ( !updateGeorefTransform() )
  {
    mMessageBar->pushMessage( tr( "Transform Failed" ), tr( "Failed to compute GCP transform: Transform is not solvable." ), Qgis::MessageLevel::Critical );
    return false;
  }

  return true;
}

bool QgsGeoreferencerMainWindow::updateGeorefTransform()
{
  QVector<QgsPointXY> sourceCoordinates;
  QVector<QgsPointXY> destinationCoords;
  if ( mGCPListWidget->gcpList() )
    mGCPListWidget->gcpList()->createGCPVectors( sourceCoordinates, destinationCoords, mTargetCrs, QgsProject::instance()->transformContext() );
  else
    return false;

  // Parametrize the transform with GCPs
  if ( !mGeorefTransform.updateParametersFromGcps( sourceCoordinates, destinationCoords, true ) )
  {
    return false;
  }

  mGCPsDirty = false;
  updateTransformParamLabel();
  return true;
}

// Samples the given rectangle at numSamples per edge.
// Returns an axis aligned bounding box which contains the transformed samples.
QgsRectangle QgsGeoreferencerMainWindow::transformViewportBoundingBox( const QgsRectangle &canvasExtent,
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
          src = QgsPointXY( oX + static_cast<double>( s ) * stepX, oY );
          break;
        case 1:
          src = QgsPointXY( oX + static_cast<double>( s ) * stepX, dY );
          break;
        case 2:
          src = QgsPointXY( oX, oY + static_cast<double>( s ) * stepY );
          break;
        case 3:
          src = QgsPointXY( dX, oY + static_cast<double>( s ) * stepY );
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

QString QgsGeoreferencerMainWindow::convertResamplingEnumToString( QgsImageWarper::ResamplingMethod resampling )
{
  switch ( resampling )
  {
    case QgsImageWarper::ResamplingMethod::NearestNeighbour:
      return QStringLiteral( "near" );
    case QgsImageWarper::ResamplingMethod::Bilinear:
      return QStringLiteral( "bilinear" );
    case QgsImageWarper::ResamplingMethod::Cubic:
      return QStringLiteral( "cubic" );
    case QgsImageWarper::ResamplingMethod::CubicSpline:
      return QStringLiteral( "cubicspline" );
    case QgsImageWarper::ResamplingMethod::Lanczos:
      return QStringLiteral( "lanczos" );
  }
  BUILTIN_UNREACHABLE
}

int QgsGeoreferencerMainWindow::polynomialOrder( QgsGeorefTransform::TransformMethod transform )
{
  switch ( transform )
  {
    case QgsGcpTransformerInterface::TransformMethod::PolynomialOrder1:
      return 1;
    case QgsGcpTransformerInterface::TransformMethod::PolynomialOrder2:
      return 2;
    case QgsGcpTransformerInterface::TransformMethod::PolynomialOrder3:
      return 3;
    case QgsGcpTransformerInterface::TransformMethod::ThinPlateSpline:
      return -1;

    default:
      return 0;
  }
}

QString QgsGeoreferencerMainWindow::guessWorldFileName( const QString &sourceFileName )
{
  QString worldFileName;
  int point = sourceFileName.lastIndexOf( '.' );
  if ( point != -1 && point != sourceFileName.length() - 1 )
    worldFileName = sourceFileName.left( point + 1 ) + "wld";

  return worldFileName;
}

bool QgsGeoreferencerMainWindow::checkFileExisting( const QString &fileName, const QString &title, const QString &question )
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

bool QgsGeoreferencerMainWindow::equalGCPlists( const QList< QgsGcpPoint > &list1, const QgsGCPList &list2 )
{
  if ( list1.count() != list2.count() )
    return false;

  int count = list1.count();
  int j = 0;
  for ( int i = 0; i < count; ++i, ++j )
  {
    const QgsGcpPoint p1 = list1.at( i );
    const QgsGcpPoint p2 = list2.at( j )->point();
    if ( p1 != p2 )
      return false;
  }

  return true;
}

void QgsGeoreferencerMainWindow::clearGCPData()
{
  //force all list widget editors to close before removing data points
  //otherwise the editors try to update deleted data points when they close
  mGCPListWidget->closeEditors();

  qDeleteAll( mPoints );
  mPoints.clear();
  mGCPListWidget->setGCPList( &mPoints );

  delete mNewlyAddedPointItem;
  mNewlyAddedPointItem = nullptr;

  QgisApp::instance()->mapCanvas()->refresh();
}

void QgsGeoreferencerMainWindow::invalidateCanvasCoords()
{
  int count = mPoints.count();
  int j = 0;
  for ( int i = 0; i < count; ++i, ++j )
  {
    QgsGeorefDataPoint *p = mPoints.at( i );
    p->updateCoords();
  }
}
