/***************************************************************************
                         qgsgeorefdescriptiondialog.cpp  -  description
                         ------------------------------
    begin                : Oct 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeorefdescriptiondialog.h"

QgsGeorefDescriptionDialog::QgsGeorefDescriptionDialog( QWidget* parent ): QDialog( parent )
{
  setupUi( this );

  textEdit->setText( "<h2>Description</h2>"
                     "<p>This plugin can generate world files "
                     "for rasters. You select points on the "
                     "raster and give their world "
                     "coordinates, and the plugin will "
                     "compute the world file parameters. "
                     "The more coordinates you can "
                     "provide the better the result will be.</p>" );
}
