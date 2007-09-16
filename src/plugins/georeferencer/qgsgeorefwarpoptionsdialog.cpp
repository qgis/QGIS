/***************************************************************************
     qgsgeorefwarpoptionsdialog.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:03:02 AKDT 2007
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
