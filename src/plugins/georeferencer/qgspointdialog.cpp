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

class QgsGeorefTool : public QgsMapTool
{
  public:
    
    QgsGeorefTool(QgsMapCanvas* canvas, QgsPointDialog* dlg, bool addPoint)
      : QgsMapTool(canvas), mDlg(dlg), mAddPoint(addPoint)
    {
    }    

    //! Mouse press event for overriding
    virtual void canvasPressEvent(QMouseEvent * e)
    {
      QgsPoint pnt = toMapCoords(e->pos());
      
      if (mAddPoint)
        mDlg->showCoordDialog(pnt);
      else
        mDlg->deleteDataPoint(pnt);
    }

    virtual void canvasMoveEvent(QMouseEvent * e) { }
    virtual void canvasReleaseEvent(QMouseEvent * e) { }
    
  private:
    QgsPointDialog* mDlg;
    bool mAddPoint;
};


QgsPointDialog::QgsPointDialog(QString layerPath, QgisInterface* theQgisInterface,
                                QWidget* parent, Qt::WFlags fl) 
  : QDialog(parent, fl), mIface(theQgisInterface), mAcetateCounter(0)
{ 
  initialize();
  openImageFile(layerPath);
}

QgsPointDialog::QgsPointDialog(QgisInterface* theQgisInterface, QWidget* parent, Qt::WFlags fl): QDialog(parent, fl), mLayer(0), mIface(theQgisInterface), mAcetateCounter(0)
{
  initialize();
}

QgsPointDialog::~QgsPointDialog()
{
  // delete layer (and don't signal it as it's our private layer)
  if (mLayer)
  {
    QgsMapLayerRegistry::instance()->removeMapLayer(mLayer->getLayerID(), FALSE);
  }
  
  delete mToolZoomIn;
  delete mToolZoomOut;
  delete mToolPan;
  delete mToolAddPoint;
  delete mToolDeletePoint;
  
}

void QgsPointDialog::openImageFile(QString layerPath)
{
  //delete old points
  for(std::vector<QgsGeorefDataPoint*>::iterator it = mPoints.begin(); it != mPoints.end(); ++it)
    {
      delete *it;
    }
  mPoints.clear();
  mAcetateCounter = 0;
  
  //delete any old rasterlayers
  if(mLayer)
    {
      QgsMapLayerRegistry::instance()->removeMapLayer(mLayer->getLayerID(), FALSE);
    }

  //add new raster layer
  QgsRasterLayer* layer = new QgsRasterLayer(layerPath, "Raster");
  mLayer = layer;
  
  // add to map layer registry, do not signal addition
  // so layer is not added to legend
  QgsMapLayerRegistry* registry = QgsMapLayerRegistry::instance();
  registry->addMapLayer(layer, FALSE);  
  
  // add layer to map canvas
  QList<QgsMapCanvasLayer> layers;
  layers.push_back(QgsMapCanvasLayer(layer));
  mCanvas->setLayerSet(layers);
  
  // load previously added points
  QFile pointFile(mLayer->source() + ".points");
  if (pointFile.open(QIODevice::ReadOnly)) {
    QTextStream points(&pointFile);
    QString tmp;
    // read the header
    points>>tmp>>tmp>>tmp>>tmp;
    // read the first line
    double mapX, mapY, pixelX, pixelY;
    points>>mapX>>mapY>>pixelX>>pixelY;
    while (!points.atEnd()) {
      QgsPoint mapCoords(mapX, mapY);
      QgsPoint pixelCoords(pixelX, pixelY);
      addPoint(pixelCoords, mapCoords);
      // read the next line
      points>>mapX>>mapY>>pixelX>>pixelY;
    }
  }
  
  mCanvas->setExtent(mLayer->extent());
  mCanvas->freeze(false);
  
  leSelectWorldFile->setText(guessWorldFileName(mLayer->source()));
  pbnGenerateWorldFile->setEnabled(true);
  pbnGenerateAndLoad->setEnabled(true);
  mCanvas->refresh();
}

void QgsPointDialog::addPoint(const QgsPoint& pixelCoords, const QgsPoint& mapCoords)
{           
  QgsGeorefDataPoint* pnt = new QgsGeorefDataPoint(mCanvas,
                        mAcetateCounter++, pixelCoords, mapCoords);
  pnt->show();
  mPoints.push_back(pnt);
  
  mCanvas->refresh();
}

void QgsPointDialog::on_pbnGenerateWorldFile_clicked()
{
  generateWorldFile();
}

void QgsPointDialog::on_pbnGenerateAndLoad_clicked()
{
  if (generateWorldFile())
  {
    QString source = mLayer->source();
    
    // delete layer before it's loaded again (otherwise it segfaults)
    QgsMapLayerRegistry::instance()->removeMapLayer(mLayer->getLayerID(), FALSE);
    mLayer = 0;
    
    // load raster to the main map canvas of QGIS
    if (cmbTransformType->currentText() == tr("Linear"))
      mIface->addRasterLayer(source);
    else 
      mIface->addRasterLayer(leSelectModifiedRaster->text());
  }
}


void QgsPointDialog::on_pbnSelectWorldFile_clicked()
{
  QString fileName = QFileDialog::getSaveFileName(this,
              tr("Choose a name for the world file"), ".");
  leSelectWorldFile->setText(fileName);
}


void QgsPointDialog::on_pbnSelectModifiedRaster_clicked()
{
  QString fileName = QFileDialog::getSaveFileName(this,
              tr("Choose a name for the world file"), ".");
  if (fileName.right(4) != ".tif")
    fileName += ".tif";
  leSelectModifiedRaster->setText(fileName);
  leSelectWorldFile->setText(guessWorldFileName(fileName));
}

void QgsPointDialog::on_cmbTransformType_currentIndexChanged(const QString& value)
{
  if (value == tr("Helmert"))
  {
    enableModifiedRasterControls(true);
    // Make up a modified raster field name based on the layer file name
    if(mLayer)
      {
	QString fileName(mLayer->source());
	QFileInfo file(mLayer->source());
	int pos = fileName.size()-file.suffix().size()-1;
	fileName.insert(pos, tr("-modified", "Georeferencer:QgsPointDialog.cpp - used to modify a user given fileName"));
	pos = fileName.size()-file.suffix().size();
	fileName.replace(pos, fileName.size(), "tif");
	
	leSelectModifiedRaster->setText(fileName);
	leSelectWorldFile->setText(guessWorldFileName(fileName));
      }
  }
  else
  {
    // Reset to the default fileNames
    leSelectModifiedRaster->setText("");
    enableModifiedRasterControls(false);
    if(mLayer)
      {
	leSelectWorldFile->setText(guessWorldFileName(mLayer->source()));
      }
  }
}

bool QgsPointDialog::generateWorldFile()
{
  QgsPoint origin(0, 0);
  double pixelXSize = 1;
  double pixelYSize = 1;
  double rotation = 0;

  QString outputFileName = leSelectModifiedRaster->text();
  QString worldFileName = leSelectWorldFile->text();
  
  // create arrays with points from mPoints
  std::vector<QgsPoint> pixelCoords, mapCoords;
  for (unsigned int i = 0; i < mPoints.size(); i++)
  {
    QgsGeorefDataPoint* pt = mPoints[i];
    pixelCoords.push_back(pt->pixelCoords());
    mapCoords.push_back(pt->mapCoords());
  }
  
  // compute the parameters using the least squares method 
  // (might throw std::domain_error)
  try
  {
    if (cmbTransformType->currentText() == tr("Linear"))
    {
      QgsLeastSquares::linear(mapCoords, pixelCoords, origin, pixelXSize, pixelYSize);
    }
    else if (cmbTransformType->currentText() == tr("Helmert"))
    {
      QMessageBox::StandardButton res = QMessageBox::warning(this, tr("Warning"),
		     tr("<p>A Helmert transform requires modifications in "
		     "the raster layer.</p><p>The modified raster will be "
		     "saved in a new file and a world file will be "
		     "generated for this new file instead.</p><p>Are you "
		     "sure that this is what you want?</p>") + 
		     "<p><i>" + tr("Currently all modified files will be written in TIFF format.") + 
		     "</i><p>", QMessageBox::Ok | QMessageBox::Cancel);
      if (res == QMessageBox::Cancel)
	       return false;

      QgsLeastSquares::helmert(mapCoords, pixelCoords, origin, pixelXSize, rotation);
      pixelYSize = pixelXSize;
      
    }
    else if (cmbTransformType->currentText() == tr("Affine"))
    {
      QMessageBox::critical(this, tr("Not implemented!"),
			    tr("<p>An affine transform requires changing the "
			    "original raster file. This is not yet "
			    "supported.</p>"));
      return false;
    }
    else
    {
      QMessageBox::critical(this, tr("Not implemented!"),
			    tr("<p>The ") +
                            cmbTransformType->currentText() +
                            tr(" transform is not yet supported.</p>"));
      return false;      
    }
  }
  catch (std::domain_error& e)
  {
    QMessageBox::critical(this, tr("Error"), QString(e.what()));
    return false;
  }

  // warp the raster if needed
  double xOffset = 0;
  double yOffset = 0;
  if (rotation != 0)
  {

    QgsGeorefWarpOptionsDialog d(this);
    d.exec();
    bool useZeroForTrans;
    QString compressionMethod;
    QgsImageWarper::ResamplingMethod resampling;
    QgsImageWarper warper(-rotation);
    d.getWarpOptions(resampling, useZeroForTrans, compressionMethod);
    //Closing the dialog by pressing the X button rather than clicking the OK button causes GDAL to barf and QGIS
    //to crash because reasampling is not a valid option
    //**not sure exactly what is going on in the case as the other two options are still correct but that could be coincidence
    if(resampling != QgsImageWarper::NearestNeighbour && resampling != QgsImageWarper::Bilinear && resampling != QgsImageWarper::Cubic)
    {
      return false;
    }
    warper.warp(mLayer->source(), outputFileName,
		xOffset, yOffset, resampling, useZeroForTrans, compressionMethod);
  }

  // write the world file
  QFile file(worldFileName);
  if (!file.open(QIODevice::WriteOnly))
  {
    QMessageBox::critical(this, tr("Error"), 
         tr("Could not write to ") + worldFileName);
    return false;
  }
  QTextStream stream(&file);
  stream<<QString::number(pixelXSize, 'f', 15)<<endl
	<<0<<endl
	<<0<<endl
	<<QString::number(-pixelYSize, 'f', 15)<<endl
	<<QString::number((origin.x() - xOffset * pixelXSize), 'f', 15)<<endl
	<<QString::number((origin.y() + yOffset * pixelYSize), 'f', 15)<<endl;  
  // write the data points in case we need them later
  QFile pointFile(mLayer->source() + ".points");
  if (pointFile.open(QIODevice::WriteOnly))
  {
    QTextStream points(&pointFile);
    points<<"mapX\tmapY\tpixelX\tpixelY"<<endl;
    for (unsigned int i = 0; i < mapCoords.size(); ++i)
    {
      points<<(QString("%1\t%2\t%3\t%4").
	       arg(mapCoords[i].x()).arg(mapCoords[i].y(), 0, 'f', 15).
	       arg(pixelCoords[i].x()).arg(pixelCoords[i].y(), 0, 'f', 15))<<endl;
    }
  }
  return true;
}


void QgsPointDialog::zoomIn()
{
  mCanvas->setMapTool(mToolZoomIn);
}


void QgsPointDialog::zoomOut()
{
  mCanvas->setMapTool(mToolZoomOut);
}


void QgsPointDialog::zoomToLayer()
{
  if(mLayer)
    {
      mCanvas->setExtent(mLayer->extent());
      mCanvas->refresh();
    }
}


void QgsPointDialog::pan()
{
  mCanvas->setMapTool(mToolPan);
}


void QgsPointDialog::addPoint()
{
  mCanvas->setMapTool(mToolAddPoint);
}


void QgsPointDialog::deletePoint()
{
  mCanvas->setMapTool(mToolDeletePoint);
}


void QgsPointDialog::showCoordDialog(QgsPoint& pixelCoords)
{
  MapCoordsDialog* mcd = new MapCoordsDialog(pixelCoords, mIface->getMapCanvas(), this);
  connect(mcd, SIGNAL(pointAdded(const QgsPoint&, const QgsPoint&)),
          this, SLOT(addPoint(const QgsPoint&, const QgsPoint&)));
  mcd->show();
}


void QgsPointDialog::deleteDataPoint(QgsPoint& coords)
{
  std::vector<QgsGeorefDataPoint*>::iterator it = mPoints.begin();
  
  double maxDistSqr = (5 * mCanvas->mapUnitsPerPixel())*(5 * mCanvas->mapUnitsPerPixel());
#ifdef QGISDEBUG
  std::cout << "deleteDataPoint! maxDistSqr: " << maxDistSqr << std::endl;
#endif
  for ( ; it != mPoints.end(); it++)
  {
    QgsGeorefDataPoint* pt = *it;
    double x = pt->pixelCoords().x() - coords.x();
    double y = pt->pixelCoords().y() - coords.y();
#ifdef QGISDEBUG
  std::cout << "deleteDataPoint! test: " << (x*x+y*y) << std::endl;
#endif
    if ((x*x + y*y) < maxDistSqr)
    {
      mPoints.erase(it);
      delete *it;
      --mAcetateCounter;
      mCanvas->refresh();
      break;
    }
  }
}


void QgsPointDialog::enableRelevantControls()
{
  if (cmbTransformType->currentText() == tr("Linear"))
  {
    leSelectModifiedRaster->setEnabled(false);
    pbnSelectModifiedRaster->setEnabled(false);
  }
  else
  {
    leSelectModifiedRaster->setEnabled(true);
    pbnSelectModifiedRaster->setEnabled(true);
  }
  
  if ((cmbTransformType->currentText() == tr("Linear") &&
       !leSelectWorldFile->text().isEmpty()) ||
      (!leSelectWorldFile->text().isEmpty() &&
       !leSelectModifiedRaster->text().isEmpty()))
  {
    pbnGenerateWorldFile->setEnabled(true);
    pbnGenerateAndLoad->setEnabled(true);
  }
  else
  {
    pbnGenerateWorldFile->setEnabled(false);
    pbnGenerateAndLoad->setEnabled(false);
  }
}


QString QgsPointDialog::guessWorldFileName(const QString& raster)
{
  int point = raster.lastIndexOf('.');
  QString worldfile = "";
  if (point != -1 && point != raster.length() - 1) 
    {
      worldfile = raster.left(point + 1);
      //MH: suffix .wld seems to be fine for most GDAL drivers
      worldfile += "wld";
    }
  return worldfile;
}

void QgsPointDialog::enableModifiedRasterControls(bool state)
{
  lblSelectModifiedRaster->setEnabled(state);
  pbnSelectModifiedRaster->setEnabled(state);
  leSelectModifiedRaster->setEnabled(state);
}

void QgsPointDialog::initialize()
{
  setupUi(this);
  
  // setup actions
  //
  mActionZoomIn= new QAction(getThemeIcon("/mActionZoomIn.png"), tr("Zoom In"), this);
  mActionZoomIn->setShortcut(tr("z"));
  mActionZoomIn->setStatusTip(tr("Zoom In"));
  connect(mActionZoomIn, SIGNAL(triggered()), this, SLOT(zoomIn()));
  //
  mActionZoomOut= new QAction(getThemeIcon("/mActionZoomOut.png"), tr("Zoom Out"), this);
  mActionZoomOut->setShortcut(tr("Z"));
  mActionZoomOut->setStatusTip(tr("Zoom Out"));
  connect(mActionZoomOut, SIGNAL(triggered()), this, SLOT(zoomOut()));
  //
  mActionZoomToLayer= new QAction(getThemeIcon("/mActionZoomToLayer.png"), tr("Zoom To Layer"), this);
  //mActionZoomToLayer->setShortcut(tr("Ctrl+O"));
  mActionZoomToLayer->setStatusTip(tr("Zoom to Layer"));
  connect(mActionZoomToLayer, SIGNAL(triggered()), this, SLOT(zoomToLayer()));
  //
  mActionPan= new QAction(getThemeIcon("/mActionPan.png"), tr("Pan Map"), this);
  mActionPan->setStatusTip(tr("Pan the map"));
  connect(mActionPan, SIGNAL(triggered()), this, SLOT(pan()));
  //
  mActionAddPoint= new QAction(getThemeIcon("/mActionCapturePoint.png"), tr("Add Point"), this);
  mActionAddPoint->setShortcut(tr("."));
  mActionAddPoint->setStatusTip(tr("Capture Points"));
  connect(mActionAddPoint, SIGNAL(triggered()), this, SLOT(addPoint()));
  //
  mActionDeletePoint = new QAction(getThemeIcon("/mActionDeleteSelected.png"), tr("Delete Point"), this);
  mActionDeletePoint->setStatusTip(tr("Delete Selected"));
  connect(mActionDeletePoint, SIGNAL(triggered()), this, SLOT(deletePoint()));
  
  // Map Tool Group
  mMapToolGroup = new QActionGroup(this);
  mActionPan->setCheckable(true);
  mMapToolGroup->addAction(mActionPan);
  mActionZoomIn->setCheckable(true);
  mMapToolGroup->addAction(mActionZoomIn);
  mActionZoomOut->setCheckable(true);
  mMapToolGroup->addAction(mActionZoomOut);
  mMapToolGroup->addAction(mActionZoomToLayer);
  mActionAddPoint->setCheckable(true);
  mMapToolGroup->addAction(mActionAddPoint);
  mActionDeletePoint->setCheckable(true);
  mMapToolGroup->addAction(mActionDeletePoint);
  
  // set appopriate actions for tool buttons
  tbnZoomIn->setDefaultAction(mActionZoomIn);
  tbnZoomOut->setDefaultAction(mActionZoomOut);
  tbnZoomToLayer->setDefaultAction(mActionZoomToLayer);
  tbnPan->setDefaultAction(mActionPan);
  tbnAddPoint->setDefaultAction(mActionAddPoint);
  tbnDeletePoint->setDefaultAction(mActionDeletePoint);
  
  // set up the canvas
  QHBoxLayout* layout = new QHBoxLayout(canvasFrame);
  mCanvas = new QgsMapCanvas(canvasFrame, "georefCanvas");
  QPalette palette;
  palette.setColor(mCanvas->backgroundRole(), Qt::white);
  mCanvas->setPalette(palette);
  mCanvas->setMinimumWidth(400);
  //mCanvas->freeze(true);
  layout->addWidget(mCanvas);
  
  // set up map tools
  mToolZoomIn = new QgsMapToolZoom(mCanvas, FALSE /* zoomOut */); 
  mToolZoomIn->setAction(mActionZoomIn);
  mToolZoomOut = new QgsMapToolZoom(mCanvas, TRUE /* zoomOut */); 
  mToolZoomOut->setAction(mActionZoomOut);
  mToolPan = new QgsMapToolPan(mCanvas); 
  mToolPan->setAction(mActionPan);
  mToolAddPoint = new QgsGeorefTool(mCanvas, this, TRUE /* addPoint */); 
  mToolAddPoint->setAction(mActionAddPoint);
  mToolDeletePoint = new QgsGeorefTool(mCanvas, this, FALSE /* addPoint */); 
  mToolDeletePoint->setAction(mActionDeletePoint);

  // set the currently supported transforms
  cmbTransformType->addItem(tr("Linear"));
  cmbTransformType->addItem(tr("Helmert"));

  enableModifiedRasterControls(false);
  addPoint();

  pbnGenerateWorldFile->setEnabled(false);
  pbnGenerateAndLoad->setEnabled(false);
}

// Note this code is duplicated from qgisapp.cpp because
// I didnt want to make plugins on qgsapplication [TS]
QIcon QgsPointDialog::getThemeIcon(const QString theName)
{
  if (QFile::exists(QgsApplication::activeThemePath() + theName))
  {
    return QIcon(QgsApplication::activeThemePath() + theName);
  }
  else
  {
    //could still return an empty icon if it
    //doesnt exist in the default theme either!
    return QIcon(QgsApplication::defaultThemePath() + theName);
  }
}
