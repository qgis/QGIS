/***************************************************************************
     qgspointdialog.cpp
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

#include <cmath>

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QTextStream>
#include <QSettings>

#include <gdalwarper.h>
#include <ogr_spatialref.h>

#include "qgisinterface.h"
#include "qgsapplication.h"
#include "qgspointdialog.h"
#include "mapcoordsdialog.h"
#include "qgsleastsquares.h"
#include "qgsimagewarper.h"
#include "qgsgeorefwarpoptionsdialog.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaptoolzoom.h"
#include "qgsmaptoolpan.h"
#include "qgsgeorefdatapoint.h"
#include "qgsgcplistwidget.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include <cassert>

#include <QDebug>
class QgsGeorefTool : public QgsMapTool
{
  public:

    QgsGeorefTool( QgsMapCanvas* canvas, QgsPointDialog* dlg, bool addPoint )
        : QgsMapTool( canvas ), mDlg( dlg ), mAddPoint( addPoint )
    {
    }

    //! Mouse press event for overriding
    virtual void canvasPressEvent( QMouseEvent * e )
    {
      // Only add point on Qt:LeftButton
      if ( Qt::LeftButton == e->button() )
      {
        QgsPoint pnt = toMapCoordinates( e->pos() );

        if ( mAddPoint )
          mDlg->showCoordDialog( pnt );
        else
          mDlg->deleteDataPoint( pnt );
      }
    }

    virtual void canvasMoveEvent( QMouseEvent * e ) { }
    virtual void canvasReleaseEvent( QMouseEvent * e ) { }

  private:
    QgsPointDialog* mDlg;
    const bool mAddPoint;
};


QgsPointDialog::QgsPointDialog( QString layerPath, QgisInterface* theQgisInterface,
                                QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl ), mIface( theQgisInterface ), mAcetateCounter( 0 )
{
  initialize();
  openImageFile( layerPath );
}

QgsPointDialog::QgsPointDialog( QgisInterface* theQgisInterface, QWidget* parent, Qt::WFlags fl ): QDialog( parent, fl ), mLayer( 0 ), mIface( theQgisInterface ), mAcetateCounter( 0 )
{
  initialize();
}

QgsPointDialog::~QgsPointDialog()
{
  // delete layer (and don't signal it as it's our private layer)
  if ( mLayer )
  {
    QgsMapLayerRegistry::instance()->removeMapLayer( mLayer->getLayerID(), FALSE );
  }

  delete mToolZoomIn;
  delete mToolZoomOut;
  delete mToolPan;
  delete mToolAddPoint;
  delete mToolDeletePoint;
}

void QgsPointDialog::openImageFile( QString layerPath )
{
  //delete any old rasterlayers
  if ( mLayer )
  {
    QgsMapLayerRegistry::instance()->removeMapLayer( mLayer->getLayerID(), FALSE );
  }

  //add new raster layer
  QgsRasterLayer* layer = new QgsRasterLayer( layerPath, "Raster" );
  mLayer = layer;

  // add to map layer registry, do not signal addition
  // so layer is not added to legend
  QgsMapLayerRegistry* registry = QgsMapLayerRegistry::instance();
  registry->addMapLayer( layer, FALSE );

  // add layer to map canvas
  QList<QgsMapCanvasLayer> layers;
  layers.push_back( QgsMapCanvasLayer( layer ) );
  mCanvas->setLayerSet( layers );

  // load previously added points
  QString gcpsPath( layerPath + ".points" );
  loadGCPs( gcpsPath );

  mCanvas->setExtent( mLayer->extent() );
  mCanvas->freeze( false );

  leSelectWorldFile->setText( guessWorldFileName( mLayer->source() ) );
  pbnGenerateWorldFile->setEnabled( true );
  pbnGenerateAndLoad->setEnabled( true );
  mCanvas->refresh();

  enableControls(true);
}

void QgsPointDialog::addPoint( const QgsPoint& pixelCoords, const QgsPoint& mapCoords )
{
  addPointWithoutRefresh(pixelCoords, mapCoords);
  mCanvas->refresh();
  mIface->mapCanvas()->refresh();
}

void QgsPointDialog::addPointWithoutRefresh( const QgsPoint& pixelCoords, const QgsPoint& mapCoords )
{
  QgsGeorefDataPoint* pnt = new QgsGeorefDataPoint( mCanvas, mIface->mapCanvas(), mAcetateCounter++, pixelCoords, mapCoords );
  mPoints.push_back( pnt );
  mGCPsDirty = true;
}

void QgsPointDialog::on_leSelectModifiedRaster_textChanged(const QString &name)
{
  leSelectWorldFile->setText( guessWorldFileName(name) );
//  if (name.length() == 0) enableControls(false);
}

void QgsPointDialog::on_pbnSelectWorldFile_clicked()
{
  QString fileName = QFileDialog::getSaveFileName( this,
                     tr( "Choose a name for the world file" ), "." );
  leSelectWorldFile->setText( fileName );
}

void QgsPointDialog::on_pbnGenerateWorldFile_clicked()
{
  generateWorldFileAndWarp();
}

void QgsPointDialog::on_pbnLoadGCPs_clicked()
{
  QSettings settings;
  QString dir = settings.value( "/Plugin-GeoReferencer/rasterdirectory" ).toString();
  if ( dir.isEmpty() )
    dir = ".";

  QString fileName =
    QFileDialog::getOpenFileName( this,
                                  tr( "Select GCPs file" ),
                                  dir,
                                  tr( "GCPs points (*.points)" ) );

  if ( fileName.isNull() )
  {
    QMessageBox::information( this, tr( "Information" ), tr( "GCPs was not loaded." ) );
    return;
  }

  loadGCPs( fileName );
}

void QgsPointDialog::createGCPVectors(std::vector<QgsPoint> &mapCoords, std::vector<QgsPoint> &pixelCoords) const
{
  mapCoords   = std::vector<QgsPoint>(mPoints.size());
  pixelCoords = std::vector<QgsPoint>(mPoints.size());
  for (unsigned int i = 0; i < mPoints.size(); i++)
  {
    QgsGeorefDataPoint* pt = mPoints[i];
    mapCoords[i] = pt->mapCoords();
    pixelCoords[i] = pt->pixelCoords();
  }
}

QgsGeorefTransform::TransformParametrisation QgsPointDialog::convertTransformStringToEnum(const QString &transformName) const throw(std::invalid_argument)
{
  if (transformName == tr("Linear"))
  {
    return QgsGeorefTransform::Linear;
  }
  else if (transformName == tr("Helmert"))
  {
    return QgsGeorefTransform::Helmert;
  }
  else if (transformName == tr("Polynomial 1"))
  {
    return QgsGeorefTransform::PolynomialOrder1;
  }
  else if (transformName == tr("Polynomial 2"))
  {
    return QgsGeorefTransform::PolynomialOrder2;
  }
  else if (transformName == tr("Polynomial 3"))
  {
    return QgsGeorefTransform::PolynomialOrder3;
  }
  else if (transformName == tr("Thin plate spline (TPS)"))
  {
    return QgsGeorefTransform::ThinPlateSpline;
  }
  else 
    throw std::invalid_argument((transformName+" does not match a known transform type.").toStdString());
}

void QgsPointDialog::jumpToGCP(uint theGCPIndex)
{
  if (theGCPIndex >= mPoints.size())
  {
    return;
  }

  // qgsmapcanvas doesn't seem to have a method for recentering the map
  QgsRectangle ext = mCanvas->extent();

  QgsPoint center = ext.center();
  QgsPoint new_center = mPoints[theGCPIndex]->pixelCoords();

  QgsPoint diff(new_center.x() - center.x(), new_center.y() - center.y());
  QgsRectangle new_extent(ext.xMinimum() + diff.x(), ext.yMinimum() + diff.y(),
                          ext.xMaximum() + diff.x(), ext.yMaximum() + diff.y());
  mCanvas->setExtent(new_extent); 
  mCanvas->refresh();
}


void QgsPointDialog::on_pbnSaveGCPs_clicked()
{
	// create arrays with points from mPoints
  std::vector<QgsPoint> mapCoords, pixelCoords;
  createGCPVectors(mapCoords, pixelCoords);
  if (mapCoords.empty())
    return;
  saveGCPs( mapCoords, pixelCoords );
}

void QgsPointDialog::on_pbnGenerateAndLoad_clicked()
{
  if ( generateWorldFileAndWarp() )
  {
    QString source = mLayer->source();

    // delete layer before it's loaded again (otherwise it segfaults)
//     QgsMapLayerRegistry::instance()->removeMapLayer( mLayer->getLayerID(), FALSE );
 //    mLayer = 0;

    // load raster to the main map canvas of QGIS
    if ( cmbTransformType->currentText() == tr( "Linear" ) )
       mIface->mapCanvas()->setExtent(mIface->addRasterLayer( source )->extent());
    else
       mIface->mapCanvas()->setExtent(mIface->addRasterLayer( leSelectModifiedRaster->text() )->extent());
    mIface->mapCanvas()->refresh();
  }
}


void QgsPointDialog::on_pbnSelectModifiedRaster_clicked()
{
  QString fileName = QFileDialog::getSaveFileName( this,
                     tr( "Choose a name for the world file" ), "." );
  if ( fileName.right( 4 ) != ".tif" )
    fileName += ".tif";
  leSelectModifiedRaster->setText( fileName );
  leSelectWorldFile->setText( guessWorldFileName( fileName ) );
}

void QgsPointDialog::on_cmbTransformType_currentIndexChanged( const QString& value )
{
  if ( value == tr( "Linear" ) )
  {
    // Reset to the default file name
    leSelectModifiedRaster->setText( "" );
    enableModifiedRasterControls( false );
    if ( mLayer )
    {
      leSelectWorldFile->setText( guessWorldFileName( mLayer->source() ) );
    }
  }
  else
  {
    enableModifiedRasterControls( true );
    // Make up a modified raster field name based on the layer file name
    if ( mLayer )
    {
      QString fileName( mLayer->source() );
      QFileInfo file( mLayer->source() );
      int pos = fileName.size() - file.suffix().size() - 1;
      fileName.insert( pos, tr( "-modified", "Georeferencer:QgsPointDialog.cpp - used to modify a user given file name" ) );

      pos = fileName.size() - file.suffix().size();
      fileName.replace( pos, fileName.size(), "tif" );

      leSelectModifiedRaster->setText( fileName );
      leSelectWorldFile->setText( guessWorldFileName( fileName ) );
    }
  }

  QgsGeorefTransform::TransformParametrisation parametrisation;
  try 
  {
    parametrisation = convertTransformStringToEnum(cmbTransformType->currentText());
  }
  catch (std::exception &arg) 
  {
    QMessageBox::critical( this, tr( "Error" ), QString( arg.what() ) );
    return;
  }
  mGeorefTransform.selectTransformParametrisation(parametrisation);
}

void QgsPointDialog::onLinkGeorefToQgisChanged(int state)
{
  if (state)
  {
    extentsChangedMainCanvas();
  }
}

void QgsPointDialog::onLinkQgisToGeorefChanged(int state)
{
  if (state)
  {
      extentsChangedGeorefCanvas();
  }
}

bool QgsPointDialog::generateWorldFileAndWarp()
{
  QString outputFileName = leSelectModifiedRaster->text();
  QString worldFileName = leSelectWorldFile->text();

  // create arrays with points from mPoints
  std::vector<QgsPoint> pixelCoords, mapCoords;
  createGCPVectors(mapCoords, pixelCoords);
  if (mapCoords.size() < mGeorefTransform.getMinimumGCPCount())
  {
    QMessageBox::critical(this, tr("Error"), tr("%1 transform requires at least %2 ground control points").arg(cmbTransformType->currentText()).arg(mGeorefTransform.getMinimumGCPCount()));
    return false;
  }
  
  // Update the transform if necessary
  if (!updateGeorefTransform())
  {
    QMessageBox::critical( this, tr( "Error" ), tr( "Failed to compute GCP transform: Transform is not solvable." ) );
    return false;
  }
  
  if (mGeorefTransform.transformParametrisation() == QgsGeorefTransform::Linear)
  {
    QgsPoint origin;
    double pixelXSize, pixelYSize;
    if (!mGeorefTransform.getLinearOriginScale(origin, pixelXSize, pixelYSize))
    {
    
      QMessageBox::critical( this, tr( "Error" ),
                             tr( "Failed to get linear transform parameters." ));
      return false;
    }

    if ( !worldFileName.isEmpty() )
    {
      if ( QFile::exists( worldFileName ) )
      {
        int r = QMessageBox::question( this, tr( "World file exists" ),
                                      tr( "<p>The selected file already seems to have a " ) +
                                      tr( "world file! Do you want to replace it with the " ) +
                                      tr( "new world file?</p>" ),
                                      QMessageBox::Yes | QMessageBox::Default,
                                      QMessageBox::No | QMessageBox::Escape );
        if ( r == QMessageBox::No )
          return false;
        else
          QFile::remove( worldFileName );
      }
    }
    else {
      QMessageBox::critical( this, tr( "Error!" ),
                             tr( "<p>Please enter an output filename.</p>" )); 
      return false;
    }

    
    // write the world file
    QFile file( worldFileName );
    if ( !file.open( QIODevice::WriteOnly ) )
    {
      QMessageBox::critical( this, tr( "Error" ),
                             tr( "Could not write to " ) + worldFileName );
      return false;
    }
    QTextStream stream( &file );
    stream << QString::number( pixelXSize, 'f', 15 ) << endl
           << 0 << endl
           << 0 << endl
           << QString::number( -pixelYSize, 'f', 15 ) << endl
           << QString::number(  origin.x(), 'f', 15 ) << endl
           << QString::number(  origin.y(), 'f', 15 ) << endl;
  }
  else 
  {
    QMessageBox::StandardButton res = QMessageBox::warning( this, tr( "Warning" ),
                                        tr( "<p>A %1 transform requires modifications in "
                                            "the raster layer.</p><p>The modified raster will be "
                                            "saved in a new file and a world file will be "
                                            "generated for this new file instead.</p><p>Are you "
                                            "sure that this is what you want?</p>" ).arg("polynomial") +
                                        "<p><i>" + tr( "Currently all modified files will be written in TIFF format." ) +
                                        "</i><p>", QMessageBox::Ok | QMessageBox::Cancel );
    if ( res == QMessageBox::Cancel )
      return false;

    if ( !outputFileName.isEmpty() )
    {
      if ( QFile::exists( outputFileName ) )
      {
        int r = QMessageBox::question( this, tr( "Error!" ),
                                      tr( "<p>Modified raster file exists! Overwrite it?</p>" ),
                                      QMessageBox::Yes | QMessageBox::Default,
                                      QMessageBox::No | QMessageBox::Escape );
        if ( r == QMessageBox::No )
          return false;
        else
          QFile::remove( outputFileName );
      }
    }
    else {
      QMessageBox::critical( this, tr( "Error!" ),
                             tr( "<p>Please enter an output filename.</p>" )); 
      return false;
    }

    bool useZeroForTrans;
    QString compressionMethod;
    QgsImageWarper::ResamplingMethod resampling;

    QgsGeorefWarpOptionsDialog d( this );   
    if (!d.exec()) 
    {
      // The options dialog was canceled by the user
      return false;
    }
     
    d.getWarpOptions( resampling, useZeroForTrans, compressionMethod );

    QgsImageWarper warper(this);
    if (!warper.warpFile( mLayer->source(), outputFileName, mGeorefTransform, resampling, useZeroForTrans, compressionMethod))
    {
      //TODO: be more specific in the error message
      QMessageBox::critical( this, tr( "Error" ), tr( "Failed to compute GCP transform: Transform is not solvable." ) );
      return false;
    }
    // write the data points in case we need them later
//     saveGCPs( mapCoords, pixelCoords );
  }
  return true;
}

void QgsPointDialog::loadGCPs( QString &fileName )
{
  QFile pointFile( fileName );
  if ( pointFile.open( QIODevice::ReadOnly ) )
  {
    //delete old points
    for ( std::vector<QgsGeorefDataPoint*>::iterator it = mPoints.begin(); it != mPoints.end(); ++it )
    {
      delete *it;
    }
    mPoints.clear();
    mAcetateCounter = 0;
 
    QTextStream points( &pointFile );
    QString tmp;
    // read the header
    points >> tmp >> tmp >> tmp >> tmp;
    // read the first line
    double mapX, mapY, pixelX, pixelY;
    points >> mapX >> mapY >> pixelX >> pixelY;
    while ( !points.atEnd() )
    {
      QgsPoint mapCoords( mapX, mapY );
      QgsPoint pixelCoords( pixelX, pixelY );
      addPointWithoutRefresh( pixelCoords, mapCoords );
      // read the next line
      points >> mapX >> mapY >> pixelX >> pixelY;
    }
  }
  mGCPsDirty = true;
  mGCPListWidget->setGCPList(&mPoints); //TODO: use signal slot mechanism to update model
  mCanvas->refresh();
}

void QgsPointDialog::saveGCPs( std::vector<QgsPoint> mapCoords, std::vector<QgsPoint> pixelCoords )
{
  QFile pointFile( mLayer->source() + ".points" );
  if ( pointFile.open( QIODevice::WriteOnly ) )
  {
    QTextStream points( &pointFile );
    points << "mapX\tmapY\tpixelX\tpixelY" << endl;
    for ( unsigned int i = 0; i < mapCoords.size(); ++i )
    {
      points << ( QString( "%1\t%2\t%3\t%4" ).
                  arg( mapCoords[i].x(), 0, 'f', 15 ).arg( mapCoords[i].y(), 0, 'f', 15 ).
                  arg( pixelCoords[i].x(), 0, 'f', 15 ).arg( pixelCoords[i].y(), 0, 'f', 15 ) ) << endl;
    }
  }
}

void QgsPointDialog::zoomIn()
{
  mCanvas->setMapTool( mToolZoomIn );
}


void QgsPointDialog::zoomOut()
{
  mCanvas->setMapTool( mToolZoomOut );
}


void QgsPointDialog::zoomToLayer()
{
  if ( mLayer )
  {
    mCanvas->setExtent( mLayer->extent() );
    mCanvas->refresh();
  }
}


void QgsPointDialog::pan()
{
  mCanvas->setMapTool( mToolPan );
}


void QgsPointDialog::addPoint()
{
  mCanvas->setMapTool( mToolAddPoint );
}


void QgsPointDialog::deletePoint()
{
  mCanvas->setMapTool( mToolDeletePoint );
}

void QgsPointDialog::on_pbnClose_clicked()
{
  close();
}

void QgsPointDialog::on_pbnSelectRaster_clicked()
{
  QSettings settings;
  QString dir = settings.value( "/Plugin-GeoReferencer/rasterdirectory" ).toString();
  if ( dir.isEmpty() )
    dir = ".";
  QString fileName =
    QFileDialog::getOpenFileName( this,
                                  tr( "Choose a raster file" ),
                                  dir,
                                  tr( "Raster files (*.*)" ) );

  if ( fileName.isNull() )
  {
	leSelectRaster->setText("");
	enableControls(false);
    return;
  }
  leSelectRaster->setText( fileName );

  // do we think that this is a valid raster?
  if ( !QgsRasterLayer::isValidRasterFileName( leSelectRaster->text() ) )
  {
    QMessageBox::critical( this, tr( "Error" ),
                           tr( "The selected file is not a valid raster file." ) );
	leSelectRaster->setText("");
	enableControls(false);
    return;
  }

  // remember the directory
  {
    QSettings settings;
    QFileInfo fileInfo( leSelectRaster->text() );
    settings.setValue( "/Plugin-GeoReferencer/rasterdirectory",
                       fileInfo.path() );
  }

  // XXX This is horrible, but it works and I'm tired / ll
  {
    QSettings settings;
    QgsProject* prj = QgsProject::instance();
    mProjBehaviour = settings.value( "/Projections/defaultBehaviour" ).toString();
    mProjectCRS = prj->readEntry( "SpatialRefSys", "/ProjectCRSProj4String" );
    mProjectCrsId = prj->readNumEntry( "SpatialRefSys", "/ProjectCrsId" );

    settings.setValue( "/Projections/defaultBehaviour", "useProject" );
    prj->writeEntry( "SpatialRefSys", "/ProjectCRSProj4String", GEOPROJ4 );
    prj->writeEntry( "SpatialRefSys", "/ProjectCrsId", int( GEOCRS_ID ) );

    settings.setValue( "/Projections/defaultBehaviour", mProjBehaviour );
    prj->writeEntry( "SpatialRefSys", "/ProjectCRSProj4String", mProjectCRS );
    prj->writeEntry( "SpatialRefSys", "/ProjectCrsId", mProjectCrsId );
  }

  openImageFile( fileName );
//  mPointDialog->show();
}

QWidget* QgsPointDialog::findMainWindow()
{
  QWidget* result = 0;

  QWidgetList topLevelWidgets = qApp->topLevelWidgets();
  QWidgetList::iterator it = topLevelWidgets.begin();
  for ( ; it != topLevelWidgets.end(); ++it )
  {
    if (( *it )->objectName() == "QgisApp" )
    {
      result = *it;
      break;
    }
  }
  return result;
}

void QgsPointDialog::on_pbnShowGCPList_clicked()
{
  mGCPListWidget->setGCPList(&mPoints);
  mGCPListWidget->setGeorefTransform(&mGeorefTransform);
  mGCPListWidget->show();
  mGCPListWidget->raise();
}

void QgsPointDialog::showCoordDialog( QgsPoint& pixelCoords )
{
  MapCoordsDialog* mcd = new MapCoordsDialog( pixelCoords, mIface->mapCanvas(), this );
  connect( mcd, SIGNAL( pointAdded( const QgsPoint&, const QgsPoint& ) ),
           this, SLOT( addPoint( const QgsPoint&, const QgsPoint& ) ) );
  mcd->show();
}

void QgsPointDialog::deleteDataPoint( QgsPoint& coords )
{
  std::vector<QgsGeorefDataPoint*>::iterator it = mPoints.begin();

  double maxDistSqr = ( 5 * mCanvas->mapUnitsPerPixel() ) * ( 5 * mCanvas->mapUnitsPerPixel() );
  QgsDebugMsg( QString( "deleteDataPoint! maxDistSqr: %1" ).arg( maxDistSqr ) );
  for ( ; it != mPoints.end(); it++ )
  {
    QgsGeorefDataPoint* pt = *it;
    double x = pt->pixelCoords().x() - coords.x();
    double y = pt->pixelCoords().y() - coords.y();
    QgsDebugMsg( QString( "deleteDataPoint! test: %1" ).arg(( x*x + y*y ) ) );
    if (( x*x + y*y ) < maxDistSqr )
    {
      delete *it;
      mPoints.erase( it );
      --mAcetateCounter;
      mGCPsDirty = true;

      mCanvas->refresh();
      break;
    }
  }
}


QString QgsPointDialog::guessWorldFileName( const QString& raster )
{
  int point = raster.lastIndexOf( '.' );
  QString worldfile = "";
  if ( point != -1 && point != raster.length() - 1 )
      worldfile = raster.left( point + 2) + raster[raster.length() - 1] + "w";
  return worldfile;
}

void QgsPointDialog::enableModifiedRasterControls( bool state )
{
  lblSelectModifiedRaster->setEnabled( state );
  pbnSelectModifiedRaster->setEnabled( state );
  leSelectModifiedRaster->setEnabled( state );
}

void QgsPointDialog::enableControls(bool state)
{
  lblSelectWorldFile->setEnabled(state);
  leSelectWorldFile->setEnabled( state );
  pbnSelectWorldFile->setEnabled( state );

  pbnGenerateWorldFile->setEnabled(state);
  pbnGenerateAndLoad->setEnabled( state );
  pbnLoadGCPs->setEnabled(state);
  pbnSaveGCPs->setEnabled(state);
}

void QgsPointDialog::initialize()
{
  setupUi( this );

  setAttribute( Qt::WA_DeleteOnClose );
  // setup actions
  //
  mActionZoomIn = new QAction( getThemeIcon( "/mActionZoomIn.png" ), tr( "Zoom In" ), this );
  mActionZoomIn->setShortcut( tr( "z" ) );
  mActionZoomIn->setStatusTip( tr( "Zoom In" ) );
  connect( mActionZoomIn, SIGNAL( triggered() ), this, SLOT( zoomIn() ) );
  //
  mActionZoomOut = new QAction( getThemeIcon( "/mActionZoomOut.png" ), tr( "Zoom Out" ), this );
  mActionZoomOut->setShortcut( tr( "Z" ) );
  mActionZoomOut->setStatusTip( tr( "Zoom Out" ) );
  connect( mActionZoomOut, SIGNAL( triggered() ), this, SLOT( zoomOut() ) );
  //
  mActionZoomToLayer = new QAction( getThemeIcon( "/mActionZoomToLayer.png" ), tr( "Zoom To Layer" ), this );
  //mActionZoomToLayer->setShortcut(tr("Ctrl+O"));
  mActionZoomToLayer->setStatusTip( tr( "Zoom to Layer" ) );
  connect( mActionZoomToLayer, SIGNAL( triggered() ), this, SLOT( zoomToLayer() ) );
  //
  mActionPan = new QAction( getThemeIcon( "/mActionPan.png" ), tr( "Pan Map" ), this );
  mActionPan->setStatusTip( tr( "Pan the map" ) );
  connect( mActionPan, SIGNAL( triggered() ), this, SLOT( pan() ) );
  //
  mActionAddPoint = new QAction( getThemeIcon( "/mActionCapturePoint.png" ), tr( "Add Point" ), this );
  mActionAddPoint->setShortcut( tr( "." ) );
  mActionAddPoint->setStatusTip( tr( "Capture Points" ) );
  connect( mActionAddPoint, SIGNAL( triggered() ), this, SLOT( addPoint() ) );
  //
  mActionDeletePoint = new QAction( getThemeIcon( "/mActionDeleteSelected.png" ), tr( "Delete Point" ), this );
  mActionDeletePoint->setStatusTip( tr( "Delete Selected" ) );
  connect( mActionDeletePoint, SIGNAL( triggered() ), this, SLOT( deletePoint() ) );

  // Map Tool Group
  mMapToolGroup = new QActionGroup( this );
  mActionPan->setCheckable( true );
  mMapToolGroup->addAction( mActionPan );
  mActionZoomIn->setCheckable( true );
  mMapToolGroup->addAction( mActionZoomIn );
  mActionZoomOut->setCheckable( true );
//  mMapToolGroup->addAction( mActionZoomOut );
  mMapToolGroup->addAction( mActionZoomToLayer );
  mActionAddPoint->setCheckable( true );
  mMapToolGroup->addAction( mActionAddPoint );
  mActionDeletePoint->setCheckable( true );
  mMapToolGroup->addAction( mActionDeletePoint );

  // set appopriate actions for tool buttons
  tbnZoomIn->setDefaultAction( mActionZoomIn );
  tbnZoomOut->setDefaultAction( mActionZoomOut );
  tbnZoomToLayer->setDefaultAction( mActionZoomToLayer );
  tbnPan->setDefaultAction( mActionPan );
  tbnAddPoint->setDefaultAction( mActionAddPoint );
  tbnDeletePoint->setDefaultAction( mActionDeletePoint );

  // set up the canvas
  QHBoxLayout* layout = new QHBoxLayout( canvasFrame );
  mCanvas = new QgsMapCanvas( canvasFrame, "georefCanvas" );
  QPalette palette;
  palette.setColor( mCanvas->backgroundRole(), Qt::white );
  mCanvas->setPalette( palette );
  mCanvas->setMinimumWidth( 400 );
  //mCanvas->freeze(true);
  layout->addWidget( mCanvas );

  // set up map tools
  mToolZoomIn = new QgsMapToolZoom( mCanvas, FALSE /* zoomOut */ );
  mToolZoomIn->setAction( mActionZoomIn );
  mToolZoomOut = new QgsMapToolZoom( mCanvas, TRUE /* zoomOut */ );
  mToolZoomOut->setAction( mActionZoomOut );
  mToolPan = new QgsMapToolPan( mCanvas );
  mToolPan->setAction( mActionPan );
  mToolAddPoint = new QgsGeorefTool( mCanvas, this, TRUE /* addPoint */ );
  mToolAddPoint->setAction( mActionAddPoint );
  mToolDeletePoint = new QgsGeorefTool( mCanvas, this, FALSE /* addPoint */ );
  mToolDeletePoint->setAction( mActionDeletePoint );

  QSettings mySettings;
  int action = mySettings.value( "/qgis/wheel_action", 0 ).toInt();
  double zoomFactor = mySettings.value( "/qgis/zoom_factor", 2 ).toDouble();
  mCanvas->setWheelAction(( QgsMapCanvas::WheelAction ) action, zoomFactor );

  // set the currently supported transforms
  cmbTransformType->addItem( tr( "Linear" ) );
  cmbTransformType->addItem( tr( "Helmert" ) );
  cmbTransformType->addItem( tr( "Polynomial 1" ) );
  cmbTransformType->addItem( tr( "Polynomial 2" ) );
  cmbTransformType->addItem( tr( "Polynomial 3" ) );
  cmbTransformType->addItem( tr( "Thin plate spline (TPS)" ) );


  // Connect main canvas and georef canvas signals so we are aware if any of the viewports change
  // (used by the map follow mode)
  connect( mCanvas, SIGNAL( extentsChanged() ), this, SLOT( extentsChangedGeorefCanvas() ) );
  connect( mIface->mapCanvas(), SIGNAL( extentsChanged() ), this, SLOT( extentsChangedMainCanvas() ) );

  connect( cbLinkQgisToGeoref, SIGNAL( stateChanged( int )), this, SLOT( onLinkQgisToGeorefChanged( int ) ));
  connect( cbLinkGeorefToQgis, SIGNAL( stateChanged( int )), this, SLOT( onLinkGeorefToQgisChanged( int ) ));

  mExtentsChangedRecursionGuard = false;

  mGeorefTransform.selectTransformParametrisation(QgsGeorefTransform::Linear);
  mGCPsDirty = true;

  enableModifiedRasterControls( false );
  enableControls(false);
  addPoint();

  mGCPListWidget = new QgsGCPListWidget(0);
  mGCPListWidget->setGCPList(&mPoints);
  mGCPListWidget->setGeorefTransform(&mGeorefTransform);
  connect( mGCPListWidget, SIGNAL( jumpToGCP( uint )), this, SLOT( jumpToGCP( uint )));
}

bool QgsPointDialog::updateGeorefTransform()
{
  if (mGCPsDirty || !mGeorefTransform.parametersInitialized()) 
  {
    // Store gcp source and destination coordinates in vectors
    std::vector<QgsPoint> pixelCoords, mapCoords;
    createGCPVectors(mapCoords, pixelCoords);

    // Parametrize the transform with GCPs
    if (!mGeorefTransform.updateParametersFromGCPs(mapCoords, pixelCoords))
    {
      return false;
    }

    mGCPsDirty = false;
  }
  return true;
}

// Samples the given rectangle at numSamples per edge.
// Returns an axis aligned bounding box which contains the transformed samples.
QgsRectangle transformViewportBoundingBox(const QgsRectangle &canvasExtent, const QgsGeorefTransform &t, bool rasterToWorld = true, uint numSamples = 4)
{
  double minX, minY;
  double maxX, maxY;
  minX = minY =  std::numeric_limits<double>::max();
  maxX = maxY = -std::numeric_limits<double>::max();

  double oX = canvasExtent.xMinimum();
  double oY = canvasExtent.yMinimum();
  double dX = canvasExtent.xMaximum();
  double dY = canvasExtent.yMaximum();
  double stepX = numSamples ? (dX-oX)/(numSamples-1) : 0.0;  
  double stepY = numSamples ? (dY-oY)/(numSamples-1) : 0.0;  
  for (uint s = 0u;  s < numSamples; s++)
  {
    for (uint edge = 0; edge < 4; edge++) {
      QgsPoint src, raster;
      switch (edge) {
        case 0: src = QgsPoint(oX + (double)s*stepX, oY); break;
        case 1: src = QgsPoint(oX + (double)s*stepX, dY); break;
        case 2: src = QgsPoint(oX, oY + (double)s*stepY); break;
        case 3: src = QgsPoint(dX, oY + (double)s*stepY); break;
      }
      t.transform(src, raster, rasterToWorld);
      minX = std::min(raster.x(), minX);
      maxX = std::max(raster.x(), maxX);
      minY = std::min(raster.y(), minY);
      maxY = std::max(raster.y(), maxY);
    }
  }
  return QgsRectangle(minX, minY, maxX, maxY);
}

// This slot is called whenever the georeference canvas changes the displayed extent
void QgsPointDialog::extentsChangedGeorefCanvas()
{
  // Guard against endless recursion by ping-pong updates
  if (mExtentsChangedRecursionGuard) 
  {
    return;
  }

  if (cbLinkQgisToGeoref->isChecked()) 
  {
    if (!updateGeorefTransform()) 
    {
      return;
    }

    // Reproject the georeference plugin canvas into world coordinates and fit axis aligned bounding box
    QgsRectangle boundingBox = transformViewportBoundingBox(mCanvas->extent(), mGeorefTransform, true);

    mExtentsChangedRecursionGuard = true;
    // Just set the whole extent for now
    // TODO: better fitting function which acounts for differing aspect ratios etc.
    mIface->mapCanvas()->setExtent(boundingBox);
    mIface->mapCanvas()->refresh();
    mExtentsChangedRecursionGuard = false;   
  }
}

// This slot is called whenever the qgis main canvas changes the displayed extent
void QgsPointDialog::extentsChangedMainCanvas()
{
  // Guard against endless recursion by ping-pong updates
  if (mExtentsChangedRecursionGuard) 
  {
    return;
  }

  if (cbLinkGeorefToQgis->isChecked()) 
  {
    // Update transform if necessary
    if (!updateGeorefTransform())
    { 
      return;
    }

    // Reproject the canvas into raster coordinates and fit axis aligned bounding box
    QgsRectangle boundingBox = transformViewportBoundingBox(mIface->mapCanvas()->extent(), mGeorefTransform, false);
    
    mExtentsChangedRecursionGuard = true;
    // Just set the whole extent for now
    // TODO: better fitting function which acounts for differing aspect ratios etc.
    mCanvas->setExtent(boundingBox);
    mCanvas->refresh();
    mExtentsChangedRecursionGuard = false;
  }
}



// Note this code is duplicated from qgisapp.cpp because
// I didnt want to make plugins on qgsapplication [TS]
QIcon QgsPointDialog::getThemeIcon( const QString theName )
{
  if ( QFile::exists( QgsApplication::activeThemePath() + theName ) )
  {
    return QIcon( QgsApplication::activeThemePath() + theName );
  }
  else
  {
    //could still return an empty icon if it
    //doesnt exist in the default theme either!
    return QIcon( QgsApplication::defaultThemePath() + theName );
  }
}
