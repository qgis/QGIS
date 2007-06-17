
#include <QValidator>
#include <mapcoordsdialog.h>

#include <qgsmapcanvas.h>
#include <qgsmaptoolemitpoint.h>

MapCoordsDialog::MapCoordsDialog() {

}


MapCoordsDialog::MapCoordsDialog(const QgsPoint& pixelCoords, QgsMapCanvas* qgisCanvas,
				 QWidget* parent, Qt::WFlags fl) 
  : QDialog(parent, fl) {
  setupUi(this);
  mPixelCoords = pixelCoords;
  mQgisCanvas = qgisCanvas;
  leXCoord->setValidator(new QDoubleValidator(this));
  leYCoord->setValidator(new QDoubleValidator(this));

  mToolEmitPoint = new QgsMapToolEmitPoint(qgisCanvas);
  mToolEmitPoint->setButton(btnPointFromCanvas);
  connect(mToolEmitPoint, SIGNAL(gotPoint(QgsPoint&,Qt::MouseButton)), this, SLOT(setXY(QgsPoint&)));
}


MapCoordsDialog::~MapCoordsDialog() {

  delete mToolEmitPoint;
}


void MapCoordsDialog::on_buttonOk_clicked() {
  QgsPoint mapCoords(leXCoord->text().toDouble(), leYCoord->text().toDouble());
  emit pointAdded(mPixelCoords, mapCoords);
  accept();
}

void MapCoordsDialog::on_buttonCancel_clicked()
{
  reject();
}

void MapCoordsDialog::setXY(QgsPoint & xy)
{
  leXCoord->clear();
  leYCoord->clear();
  leXCoord->insert(QString::number(xy.x(),'f',7));
  leYCoord->insert(QString::number(xy.y(),'f',7));
  
  mQgisCanvas->setMapTool(mPrevMapTool);
}

void MapCoordsDialog::on_btnPointFromCanvas_clicked()
{
  mPrevMapTool = mQgisCanvas->mapTool();
  mQgisCanvas->setMapTool(mToolEmitPoint);
}
