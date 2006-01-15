#include <qlineedit.h>
#include <qvalidator.h>

#include <mapcoordsdialog.h>


MapCoordsDialog::MapCoordsDialog() {

}


MapCoordsDialog::MapCoordsDialog(const QgsPoint& pixelCoords,
				 QWidget* parent, Qt::WFlags fl) 
  : QDialog(parent, fl) {
  setupUi(this);
  mPixelCoords = pixelCoords;
  leXCoord->setValidator(new QDoubleValidator(this));
  leYCoord->setValidator(new QDoubleValidator(this));
}


MapCoordsDialog::~MapCoordsDialog() {

}


void MapCoordsDialog::pbnOK_clicked() {
  QgsPoint mapCoords(leXCoord->text().toDouble(), leYCoord->text().toDouble());
  emit pointAdded(mPixelCoords, mapCoords);
  accept();
}
