#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qframe.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>

#include "datapointacetate.h"
#include "qgspointdialog.h"
#include "mapcoordsdialog.h"
#include "qgsleastsquares.h"

#include "zoom_in.xpm"
#include "zoom_out.xpm"
#include "pan.xpm"
#include "add_point.xpm"

QgsPointDialog::QgsPointDialog() {
  
}


QgsPointDialog::QgsPointDialog(QgsRasterLayer* layer, const QString& worldfile,
			       QWidget* parent, const char* name, 
			       bool modal, WFlags fl) 
  : QgsPointDialogBase(parent, name, modal, fl), 
    mLayer(layer), mWorldfile(worldfile), mCursor(NULL) {
  
  QHBoxLayout* layout = new QHBoxLayout(canvasFrame);
  layout->setAutoAdd(true);
  mCanvas = new QgsMapCanvas(canvasFrame, "georefCanvas");
  mCanvas->setBackgroundColor(Qt::white);
  mCanvas->setMinimumWidth(400);
  mCanvas->addLayer(mLayer);
  mCanvas->setExtent(mLayer->extent());
  tbnAddPoint->setOn(true);
  
  connect(mCanvas, SIGNAL(xyClickCoordinates(QgsPoint&)),
	  this, SLOT(showCoordDialog(QgsPoint&)));
  leSelectWorldFile->setText(worldfile);
}


QgsPointDialog::~QgsPointDialog() {
  
}


void QgsPointDialog::showCoordDialog(QgsPoint& pixelCoords) {
  MapCoordsDialog* mcd = new MapCoordsDialog(pixelCoords, this, NULL, true);
  connect(mcd, SIGNAL(pointAdded(const QgsPoint&, const QgsPoint&)),
	  this, SLOT(addPoint(const QgsPoint&, const QgsPoint&)));
  mcd->show();
}


void QgsPointDialog::addPoint(const QgsPoint& pixelCoords, 
			      const QgsPoint& mapCoords) {
  mPixelCoords.push_back(pixelCoords);
  mMapCoords.push_back(mapCoords);
  static int acetateCounter = 0;
  mCanvas->addAcetateObject(QString("%1").arg(++acetateCounter),
			    new DataPointAcetate(pixelCoords, mapCoords));
  mCanvas->refresh();
  for (int i = 0; i < mPixelCoords.size(); ++i) {
    std::cerr<<"GEOREF: pixel ("
	     <<mPixelCoords[i].x()<<","<<mPixelCoords[i].y()<<") map ("
	     <<mMapCoords[i].x()<<","<<mMapCoords[i].y()<<")"<<std::endl;
  }
}


void QgsPointDialog::pbnCancel_clicked() {
  delete mCanvas;
  delete mLayer;
  reject();
}


void QgsPointDialog::pbnGenerateWorldFile_clicked() {
  if (generateWorldFile()) {
    delete mCanvas;
    delete mLayer;
    accept();
  }
}


void QgsPointDialog::pbnGenerateAndLoad_clicked() {
  if (generateWorldFile()) {
    delete mCanvas;
    emit loadLayer(mLayer->source());
    delete mLayer;
    accept();
  }
}


void QgsPointDialog::pbnSelectWorldFile_clicked() {
  QString filename = 
    QFileDialog::getSaveFileName(".",
				 NULL,
				 this,
				 "Save world file"
				 "Choose a name for the world file");
  leSelectWorldFile->setText(filename);
}


bool QgsPointDialog::generateWorldFile() {
  QgsPoint origin(0, 0);
  double pixelSize = 1;
  double rotation = 0;
  try {
    if (cmbTransformType->currentItem() == 0)
      QgsLeastSquares::linear(mMapCoords, mPixelCoords, origin, pixelSize);
    else if (cmbTransformType->currentItem() == 1) {
      QMessageBox::critical(this, "Not implemented!",
			    "A Helmert transform requires a rotation of the "
			    "original raster file. This is not yet "
			    "supported.");
      return false;
      /*
	QgsLeastSquares::helmert(mMapCoords, mPixelCoords, 
	origin, pixelSize, rotation);
      */
    }
    else if (cmbTransformType->currentItem() == 2) {
      QMessageBox::critical(this, "Not implemented!",
			    "An affine transform requires changing the "
			    "original raster file. This is not yet "
			    "supported.");
      return false;
    }
  }
  catch (std::domain_error& e) {
    QMessageBox::critical(this, "Error", QString(e.what()));
    return false;
  }

  std::cerr<<"================="<<std::endl
	   <<pixelSize<<std::endl
	   <<0<<std::endl
	   <<0<<std::endl
	   <<-pixelSize<<std::endl
	   <<origin.x()<<std::endl
	   <<origin.y()<<std::endl
	   <<"================="<<std::endl
	   <<"ROTATION: "<<(rotation*180/3.14159265)<<std::endl;
  
  QFile file(leSelectWorldFile->text());
  if (!file.open(IO_WriteOnly)) {
    QMessageBox::critical(this, "Error", 
			  "Could not write to " + leSelectWorldFile->text());
    return false;
  }

  QTextStream stream(&file);
  stream<<pixelSize<<endl
	<<0<<endl
	<<0<<endl
	<<-pixelSize<<endl
	<<origin.x()<<endl
	<<origin.y()<<endl;  
  
  return true;
}


void QgsPointDialog::tbnZoomIn_changed(int state) {
  if (state == QButton::On) {
    tbnZoomOut->setOn(false);
    tbnPan->setOn(false);
    tbnAddPoint->setOn(false);
    tbnDeletePoint->setOn(false);
    mCanvas->setMapTool(QGis::ZoomIn);
    delete mCursor;
    QPixmap pix((const char **)zoom_in2);
    mCursor = new QCursor(pix, 7, 7);
    mCanvas->setCursor(*mCursor);
  }
}


void QgsPointDialog::tbnZoomOut_changed(int state) {
  if (state == QButton::On) {
    tbnZoomIn->setOn(false);
    tbnPan->setOn(false);
    tbnAddPoint->setOn(false);
    tbnDeletePoint->setOn(false);
    mCanvas->setMapTool(QGis::ZoomOut);
    delete mCursor;
    QPixmap pix((const char **)zoom_out);
    mCursor = new QCursor(pix, 7, 7);
    mCanvas->setCursor(*mCursor);
  }
}


void QgsPointDialog::tbnZoomToLayer_clicked() {
  mCanvas->setExtent(mLayer->extent());
  mCanvas->refresh();
}


void QgsPointDialog::tbnPan_changed(int state) {
  if (state == QButton::On) {
    tbnZoomIn->setOn(false);
    tbnZoomOut->setOn(false);
    tbnAddPoint->setOn(false);
    tbnDeletePoint->setOn(false);
    mCanvas->setMapTool(QGis::Pan);
    delete mCursor;
    QPixmap pix((const char **)pan);
    mCursor = new QCursor(pix, 7, 7);
    mCanvas->setCursor(*mCursor);
  }
}


void QgsPointDialog::tbnAddPoint_changed(int state) {
  if (state == QButton::On) {
    tbnZoomIn->setOn(false);
    tbnZoomOut->setOn(false);
    tbnPan->setOn(false);
    tbnDeletePoint->setOn(false);
    mCanvas->setMapTool(QGis::EmitPoint);
    delete mCursor;
    QPixmap pix((const char **)add_point);
    mCursor = new QCursor(pix, 7, 7);
    mCanvas->setCursor(*mCursor);
  }
}


void QgsPointDialog::tbnDeletePoint_changed(int state) {
  if (state == QButton::On) {
    tbnZoomIn->setOn(false);
    tbnZoomOut->setOn(false);
    tbnPan->setOn(false);
    tbnAddPoint->setOn(false);
    mCanvas->setMapTool(QGis::EmitPoint);
  }
}


