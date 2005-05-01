#include <iostream>

#include <qcheckbox.h>
#include <qcombobox.h>

#include "qgsgeorefwarpoptionsdialog.h"


QgsGeorefWarpOptionsDialog::QgsGeorefWarpOptionsDialog(QWidget* parent)
  : QgsGeorefWarpOptionsDialogBase(parent, NULL, TRUE, 0) {

}


void QgsGeorefWarpOptionsDialog::
getWarpOptions(QgsImageWarper::ResamplingMethod& resampling, 
	       bool& useZeroForTransparency) {
  resampling = this->resampling;
  useZeroForTransparency = this->useZeroAsTransparency;
}


void QgsGeorefWarpOptionsDialog::pbnOK_clicked() {
  QgsImageWarper::ResamplingMethod methods[] = {
    QgsImageWarper::NearestNeighbour,
    QgsImageWarper::Bilinear,
    QgsImageWarper::Cubic
  };
  resampling = methods[cmbResampling->currentItem()];
  useZeroAsTransparency = cbxZeroAsTrans->isChecked();
  close();
}
