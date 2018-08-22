/***************************************************************************
    qgslayoutlegendlayersdialog.cpp
    -------------------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslayoutlegendlayersdialog.h"

#include <QStandardItem>
#include "qgsmaplayer.h"
#include "qgsmaplayermodel.h"
#include "qgsmaplayerproxymodel.h"
#include "qgssettings.h"

QgsLayoutLegendLayersDialog::QgsLayoutLegendLayersDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/LayoutLegendLayers/geometry" ) ).toByteArray() );

  mModel = new QgsMapLayerProxyModel( listMapLayers );
  listMapLayers->setModel( mModel );
  QModelIndex firstLayer = mModel->index( 0, 0 );
  listMapLayers->selectionModel()->select( firstLayer, QItemSelectionModel::Select );

  connect( listMapLayers, &QListView::doubleClicked, this, &QgsLayoutLegendLayersDialog::accept );
}

QgsLayoutLegendLayersDialog::~QgsLayoutLegendLayersDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/LayoutLegendLayers/geometry" ), saveGeometry() );
}

QList< QgsMapLayer *> QgsLayoutLegendLayersDialog::selectedLayers() const
{
  QList< QgsMapLayer * > layers;

  const QModelIndexList selection = listMapLayers->selectionModel()->selectedIndexes();
  for ( const QModelIndex &index : selection )
  {
    const QModelIndex sourceIndex = mModel->mapToSource( index );
    if ( !sourceIndex.isValid() )
    {
      continue;
    }

    QgsMapLayer *layer = mModel->sourceLayerModel()->layerFromIndex( sourceIndex );
    if ( layer )
      layers << layer;
  }
  return layers;
}
