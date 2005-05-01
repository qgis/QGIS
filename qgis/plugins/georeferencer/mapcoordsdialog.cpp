#include <qlineedit.h>
#include <qvalidator.h>

#include <mapcoordsdialog.h>


MapCoordsDialog::MapCoordsDialog() {

}


MapCoordsDialog::MapCoordsDialog(const QgsPoint& pixelCoords,
				 QWidget* parent, const char* name, 
				 bool modal, WFlags fl) 
  : MapCoordsDialogBase(parent, name, modal, fl) {
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
