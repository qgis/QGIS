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
#include "qgsmaplayerproxymodel.h"
#include "qgsmaplayermodel.h"
#include "qgsgui.h"

QgsEmbeddedLayerSelectDialog::QgsEmbeddedLayerSelectDialog( QWidget *parent )
  : QDialog( parent )
  , mLayerProxyModel( new QgsMapLayerProxyModel( this ) )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  mLayerProxyModel->setFilters( QgsMapLayerProxyModel::Filter::VectorLayer );
  mLayers->setModel( mLayerProxyModel );

  mSearchLineEdit->setShowSearchIcon( true );
  mSearchLineEdit->setShowClearButton( true );
  connect( mSearchLineEdit, &QLineEdit::textChanged, mLayerProxyModel, &QgsMapLayerProxyModel::setFilterString );
  mSearchLineEdit->setFocus();
}

QStringList QgsEmbeddedLayerSelectDialog::layers() const
{
  QStringList ids;
  const QModelIndexList selected = mLayers->selectionModel()->selectedRows();
  ids.reserve( selected.size() );
  for ( const QModelIndex &index : selected )
  {
    ids << index.data( QgsMapLayerModel::LayerIdRole ).toString();
  }
  return ids;
}
