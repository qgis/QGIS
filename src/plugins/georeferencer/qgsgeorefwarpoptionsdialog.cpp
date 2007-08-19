
#include "qgsgeorefwarpoptionsdialog.h"


QgsGeorefWarpOptionsDialog::QgsGeorefWarpOptionsDialog(QWidget* parent)
  : QgsGeorefWarpOptionsDialogBase() 
{
  setupUi(this);
  QStringList compressionMethods;
  compressionMethods << "LZW";
  compressionMethods << "PACKBITS";
  compressionMethods << "DEFLATE";
  compressionMethods << "NONE";
  mCompressionComboBox->addItems(compressionMethods);
}


void QgsGeorefWarpOptionsDialog::
getWarpOptions(QgsImageWarper::ResamplingMethod& resampling, 
	       bool& useZeroForTransparency, QString& compression) 
{
  resampling = this->resampling;
  useZeroForTransparency = this->useZeroAsTransparency;
  compression = mCompressionComboBox->currentText();
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
