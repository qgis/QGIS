/***************************************************************************
    qgslayoutlegendlayersdialog.cpp
    -------------------------------
    begin                : March 2020
    copyright            : (C) 2020 by roya0045
    email                : roya0045 at users dot noreply dot github dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmatchinglayersdialog.h"

#include <QStandardItem>
#include "qgsmaplayer.h"
#include "qgsmaplayermodel.h"
#include "qgsmaplayerproxymodel.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgswkbtypes.h"
#include "qgsmapthemecollection.h"

QgsMatchingLayersDialog::QgsMatchingLayersDialog( QgsMapLayer *layer, QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  setWindowTitle( tr( "Select the desired layer" ) );
  QgsGui::enableAutoGeometryRestore( this );
  QgsMapLayerProxyModel::Filter filter; 
  mModel = new QgsMapLayerProxyModel( listMapLayers );
  if ( layer )
  {
    if ( layer->type() == QgsMapLayerType::VectorLayer)
    {
        switch ( qobject_cast<QgsVectorLayer *>( layer )->wkbType() )
        {
            case QgsWkbTypes::Point:
                filter = QgsMapLayerProxyModel::PointLayer;
                break;
            case QgsWkbTypes::MultiPoint:
                filter = QgsMapLayerProxyModel::PointLayer;
                break;
            case QgsWkbTypes::LineString:
                filter = QgsMapLayerProxyModel::LineLayer;
                break;
            case QgsWkbTypes::MultiLineString:
                filter = QgsMapLayerProxyModel::LineLayer;
                break;
            case QgsWkbTypes::Polygon:
                filter = QgsMapLayerProxyModel::PolygonLayer;
                break;
            case QgsWkbTypes::MultiPolygon:
                filter = QgsMapLayerProxyModel::PolygonLayer;
                break;
            default:
                filter = QgsMapLayerProxyModel::VectorLayer;
        }
    }
    else
    {
        switch ( layer->type() )
        {
            case QgsMapLayerType::RasterLayer:
                filter = QgsMapLayerProxyModel::RasterLayer;
                break;
            case QgsMapLayerType::MeshLayer:
                filter = QgsMapLayerProxyModel::MeshLayer;
                break;
            default:
                filter = QgsMapLayerProxyModel::All;
        }
    }
    mModel->setFilters( filter );
  }
  
  listMapLayers->setModel( mModel );
  QModelIndex firstLayer = mModel->index( 0, 0 );
  listMapLayers->selectionModel()->select( firstLayer, QItemSelectionModel::Select );

  connect( listMapLayers, &QListView::doubleClicked, this, &QgsMatchingLayersDialog::accept );
  connect( mFilterLineEdit, &QLineEdit::textChanged, mModel, &QgsMapLayerProxyModel::setFilterString );
  connect( mCheckBoxVisibleLayers, &QCheckBox::toggled, this, &QgsMatchingLayersDialog::filterVisible );
}

QList< QgsMapLayer *> QgsMatchingLayersDialog::selectedLayers() const
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


void QgsMatchingLayersDialog::filterVisible( bool enabled )
{
  if ( enabled )
    mModel->setLayerWhitelist( QgsProject::instance()->mapThemeCollection()->masterVisibleLayers() );
  else
    mModel->setLayerWhitelist( QList< QgsMapLayer * >() );
}
