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

#include <QMessageBox>

QgsRasterAttributeTableDialog::QgsRasterAttributeTableDialog( QgsRasterLayer *rasterLayer, int bandNumber, QWidget *parent )
  : QDialog( parent )
  , mRasterLayer( rasterLayer )
{
  Q_ASSERT( rasterLayer );
  setupUi( this );
  mRatWidget->setRasterLayer( rasterLayer, bandNumber );
  setWindowTitle( tr( "Raster Attribute Table for %1" ).arg( rasterLayer->name() ) );

  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsRasterAttributeTableDialog::reject );

}

void QgsRasterAttributeTableDialog::reject()
{
  QList<int> dirtyBands;
  for ( int bandNo = 1;  bandNo <= mRasterLayer->bandCount(); ++bandNo )
  {
    if ( QgsRasterAttributeTable *rat = mRasterLayer->attributeTable( bandNo ) )
    {
      if ( rat->isDirty() )
      {
        dirtyBands.push_back( bandNo );
      }
    }
  }

  if ( ! dirtyBands.isEmpty() )
  {
    QString bandsStr;
    for ( int i = 0; i < dirtyBands.size(); i++ )
    {
      bandsStr.append( QString::number( dirtyBands[i] ) );
      if ( i < dirtyBands.size() - 1 )
        bandsStr.append( QStringLiteral( ", " ) );
    }

    QString msg { dirtyBands.count( ) > 1 ? tr( "Attribute table bands (%1) contain unsaved changes, close without saving?" ).arg( bandsStr ) : tr( "Attribute table band %1 contains unsaved changes, close without saving?" ).arg( bandsStr ) };

    if ( QMessageBox::question( nullptr, tr( "Save Attribute Table" ), msg ) != QMessageBox::Yes )
    {
      return;
    }
  }

  QDialog::reject();
}
