/***************************************************************************
      Virtual layer embedded layer selection widget

begin                : Feb 2016
copyright            : (C) 2016 Hugo Mercier, Oslandia
email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsembeddedlayerselectdialog.h"

#include <QMainWindow>
#include <QSettings>

#include "qgsvectorlayer.h"
#include "layertree/qgslayertreeview.h"
#include "layertree/qgslayertreemodel.h"
#include "layertree/qgslayertreegroup.h"
#include "layertree/qgslayertreelayer.h"
#include "layertree/qgslayertree.h"
#include "qgsproviderregistry.h"
#include "qgsvectordataprovider.h"

QgsEmbeddedLayerSelectDialog::QgsEmbeddedLayerSelectDialog( QWidget *parent, QgsLayerTreeView *tv )
  : QDialog( parent ),
    mTreeView( tv )
{
  setupUi( this );
  updateLayersList();
}

QStringList QgsEmbeddedLayerSelectDialog::layers() const
{
  QStringList ids;
  QModelIndexList selected = mLayers->selectionModel()->selectedRows();
  for ( int i = 0; i < selected.size(); i++ )
  {
    QgsVectorLayer *l = static_cast<QgsVectorLayer *>( mLayers->item( selected[i].row() )->data( Qt::UserRole ).value<void *>() );
    ids << l->id();
  }
  return ids;
}

void QgsEmbeddedLayerSelectDialog::updateLayersList()
{
  // populate list
  mLayers->clear();
  QList<QgsLayerTreeLayer *> layers = mTreeView->layerTreeModel()->rootGroup()->findLayers();
  Q_FOREACH ( const QgsLayerTreeLayer *l, layers )
  {
    if ( l->layer() && l->layer()->type() == QgsMapLayer::VectorLayer )
    {
      // display layer name and store its pointer
      QListWidgetItem *item = new QListWidgetItem();
      item->setText( l->layer()->name() );
      item->setData( Qt::UserRole, QVariant::fromValue( static_cast<void *>( l->layer() ) ) );
      mLayers->insertItem( mLayers->count(), item );
    }
  }
}
