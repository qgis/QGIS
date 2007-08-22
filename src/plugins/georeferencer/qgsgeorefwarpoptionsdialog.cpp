
#include "qgsgeorefwarpoptionsdialog.h"


QgsGeorefWarpOptionsDialog::QgsGeorefWarpOptionsDialog(QWidget* parent)
  : QgsGeorefWarpOptionsDialogBase() 
{
  setupUi(this);
  QStringList compressionMethods;
  compressionMethods << "NONE";
  compressionMethods << tr("LZW (unstable)");
  compressionMethods << tr("PACKBITS (unstable)");
  compressionMethods << tr("DEFLATE (unstable)");
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
