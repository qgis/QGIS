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
/* $Id$ */

#include "qgsgeorefwarpoptionsdialog.h"


QgsGeorefWarpOptionsDialog::QgsGeorefWarpOptionsDialog( QWidget* parent )
  : QDialog( parent ) //QgsGeorefWarpOptionsDialogBase()
{
  setupUi( this );

  textBrowser->setHtml(tr("<p>A %1 transform requires modifications in "
                          "the raster layer.</p><p>The modified raster will be "
                          "saved in a new file and a world file will be "
                          "generated for this new file instead.</p><p>Are you "
                          "sure that this is what you want?</p>" ).arg("polynomial") +
                       "<p><i>" + tr( "Currently all modified files will be written in TIFF format." ) +
                       "</i><p>");
  adjustSize();
}

void QgsGeorefWarpOptionsDialog::
    getWarpOptions( QgsImageWarper::ResamplingMethod& resampling,
                    bool& useZeroForTransparency, QString& compression )
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
  useZeroForTransparency = cbxZeroAsTrans->isChecked();

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

