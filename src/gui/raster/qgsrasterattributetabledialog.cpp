/***************************************************************************
  qgsrasterattributetabledialog.cpp - QgsRasterAttributeTableDialog

 ---------------------
 begin                : 13.10.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsrasterattributetabledialog.h"
#include "qgsrasterlayer.h"


QgsRasterAttributeTableDialog::QgsRasterAttributeTableDialog( QgsRasterLayer *rasterLayer, int bandNumber, QWidget *parent )
  : QDialog( parent )
{
  Q_ASSERT( rasterLayer );
  setupUi( this );
  mRatWidget->setRasterLayer( rasterLayer, bandNumber );
  setWindowTitle( tr( "Raster Attribute Table for %1" ).arg( rasterLayer->name() ) );

  connect( rasterLayer, &QgsRasterLayer::dataSourceChanged, this, &QgsRasterAttributeTableDialog::reject );
  connect( rasterLayer, &QgsRasterLayer::willBeDeleted, this, &QgsRasterAttributeTableDialog::reject );

}

void QgsRasterAttributeTableDialog::reject()
{
  if ( mRatWidget->setEditable( false ) )
  {
    QDialog::reject();
  }
}
