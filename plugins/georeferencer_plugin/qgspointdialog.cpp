#include <qfiledialog.h>
#include <qframe.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmessagebox.h>

#include "datapointacetate.h"
#include "qgspointdialog.h"
#include "mapcoordsdialog.h"
#include "qgsleastsquares.h"


QgsPointDialog::QgsPointDialog() {
  
}


QgsPointDialog::QgsPointDialog(QgsRasterLayer* layer, const QString& worldfile,
			       QWidget* parent, const char* name, 
			       bool modal, WFlags fl) 
  : QgsPointDialogBase(parent, name, modal, fl) {
  
  mLayer = layer;
  mWorldfile = worldfile;
  QHBoxLayout* layout = new QHBoxLayout(canvasFrame);
  layout->setAutoAdd(true);
  mCanvas = new QgsMapCanvas(canvasFrame, "georefCanvas");
  mCanvas->setBackgroundColor(Qt::white);
  mCanvas->setMinimumWidth(400);
  mCanvas->addLayer(layer);
  mCanvas->setExtent(layer->extent());
  mCanvas->setMapTool(QGis::EmitPoint);
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
  QgsPoint origin;
  double pixelSize;
  try {
    QgsLeastSquares::linear(mMapCoords, mPixelCoords, origin, pixelSize);
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
	   <<"================="<<std::endl;
  
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
