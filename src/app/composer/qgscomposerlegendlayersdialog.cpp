/***************************************************************************
                         qgscomposerlegendlayersdialog.cpp
                         -------------------------------
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerlegendlayersdialog.h"

#include <QStandardItem>

QgsComposerLegendLayersDialog::QgsComposerLegendLayersDialog( QList<QgsMapLayer*> layers, QWidget* parent ): QDialog( parent )
{
  setupUi( this );

  QList<QgsMapLayer*>::iterator layerIt = layers.begin();
  for (; layerIt != layers.end(); ++layerIt )
  {
    QListWidgetItem* item = new QListWidgetItem(( *layerIt )->name(), listMapLayers );
    mItemLayerMap.insert( item, *layerIt );
  } 
}

QgsComposerLegendLayersDialog::QgsComposerLegendLayersDialog(): QDialog( 0 )
{

}

QgsComposerLegendLayersDialog::~QgsComposerLegendLayersDialog()
{

}

QgsMapLayer* QgsComposerLegendLayersDialog::selectedLayer()
{
  QListWidgetItem* item = listMapLayers->currentItem();
  if ( !item )
  {
    return 0;
  }

  QMap<QListWidgetItem*, QgsMapLayer*>::iterator it = mItemLayerMap.find( item );
  QgsMapLayer* c = 0;
  c = it.value();
  return c;
}
