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


QgsGeorefWarpOptionsDialog::QgsGeorefWarpOptionsDialog( QWidget* parent )
    : QDialog( parent ) //QgsGeorefWarpOptionsDialogBase()
{
  setupUi( this );
  QStringList compressionMethods;
  compressionMethods << "NONE";
  compressionMethods << "LZW";
  compressionMethods << "PACKBITS";
  compressionMethods << "DEFLATE";
  mCompressionComboBox->addItems( compressionMethods );
}


void QgsGeorefWarpOptionsDialog::
getWarpOptions( QgsImageWarper::ResamplingMethod& resampling,
                bool& useZeroForTransparency, QString& compression )
{
  resampling = this->resampling;
  useZeroForTransparency = this->useZeroAsTransparency;

  QString compressionString = mCompressionComboBox->currentText();
  if ( compressionString.startsWith( "NONE" ) )
  {
    compression = "NONE";
  }
  else if ( compressionString.startsWith( "LZW" ) )
  {
    compression = "LZW";
  }
  else if ( compressionString.startsWith( "PACKBITS" ) )
  {
    compression = "PACKBITS";
  }
  else if ( compressionString.startsWith( "DEFLATE" ) )
  {
    compression = "DEFLATE";
  }
}


void QgsGeorefWarpOptionsDialog::on_buttonBox_accepted()
{
  QgsImageWarper::ResamplingMethod methods[] =
  {
    QgsImageWarper::NearestNeighbour,
    QgsImageWarper::Bilinear,
    QgsImageWarper::Cubic,
    QgsImageWarper::CubicSpline,
    QgsImageWarper::Lanczos
  };
  resampling = methods[cmbResampling->currentIndex()];
  useZeroAsTransparency = cbxZeroAsTrans->isChecked();
  accept();
}

void QgsGeorefWarpOptionsDialog::on_buttonBox_rejected()
{
  close();
}
