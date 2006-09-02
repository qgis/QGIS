
#include "qgsgeorefwarpoptionsdialog.h"


QgsGeorefWarpOptionsDialog::QgsGeorefWarpOptionsDialog(QWidget* parent)
  : QgsGeorefWarpOptionsDialogBase() 
{
  setupUi(this);
}


void QgsGeorefWarpOptionsDialog::
getWarpOptions(QgsImageWarper::ResamplingMethod& resampling, 
	       bool& useZeroForTransparency) {
  resampling = this->resampling;
  useZeroForTransparency = this->useZeroAsTransparency;
}


void QgsGeorefWarpOptionsDialog::on_pbnOK_clicked() {
  QgsImageWarper::ResamplingMethod methods[] = {
    QgsImageWarper::NearestNeighbour,
    QgsImageWarper::Bilinear,
    QgsImageWarper::Cubic
  };
  resampling = methods[cmbResampling->currentItem()];
  useZeroAsTransparency = cbxZeroAsTrans->isChecked();
  close();
}
