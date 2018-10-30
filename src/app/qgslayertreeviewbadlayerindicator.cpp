/***************************************************************************
  qgslayertreeviewbadlayerindicatorprovider.cpp - QgsLayerTreeViewBadLayerIndicatorProvider

 ---------------------
 begin                : 17.10.2018
 copyright            : (C) 2018 by Alessandro Pasotti
 email                : elpaso@itopen.it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreeviewbadlayerindicator.h"
#include "qgslayertree.h"
#include "qgslayertreeview.h"
#include "qgslayertreeutils.h"
#include "qgslayertreemodel.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgisapp.h"
#include "qgsbrowsermodel.h"
#include "qgsbrowsertreeview.h"

#include <functional>
#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>

QgsLayerTreeViewBadLayerIndicatorProvider::QgsLayerTreeViewBadLayerIndicatorProvider( QgsLayerTreeView *view )
  : QgsLayerTreeViewIndicatorProvider( view )
{
}

void QgsLayerTreeViewBadLayerIndicatorProvider::onIndicatorClicked( const QModelIndex &index )
{
  QgsLayerTreeNode *node = mLayerTreeView->layerTreeModel()->index2node( index );
  if ( !QgsLayerTree::isLayer( node ) )
    return;

  // Only raster/vector for now are supported
  QgsMapLayer *layer = nullptr;
  if ( qobject_cast<QgsVectorLayer *>( QgsLayerTree::toLayer( node )->layer() ) ||
       qobject_cast<QgsRasterLayer *>( QgsLayerTree::toLayer( node )->layer() ) )
  {
    layer = qobject_cast<QgsMapLayer *>( QgsLayerTree::toLayer( node )->layer() );
  }
  if ( !layer )
    return;

  // Get provider type
  QString providerType( layer->providerType() );
  QgsMapLayer::LayerType layerType( layer->type() );

  // Builds the dialog to select a new data source
  QgsBrowserModel browserModel;
  browserModel.initialize();
  QDialog dlg;
  dlg.setWindowTitle( tr( "Select the new data source" ) );
  QByteArray dlgGeom( QgsSettings().value( QStringLiteral( "/Windows/selectDataSourceDialog/geometry" ), QVariant(), QgsSettings::Section::App ).toByteArray() );
  dlg.restoreGeometry( dlgGeom );
  QVBoxLayout lay( &dlg );
  QgsBrowserTreeView *browserWidget( new QgsBrowserTreeView( ) );
  browserWidget->setModel( &browserModel );
  browserWidget->setHeaderHidden( true );
  lay.addWidget( browserWidget );
  QDialogButtonBox *buttonBox( new QDialogButtonBox( QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel ) );
  lay.addWidget( buttonBox );
  connect( buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept );
  connect( buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject );
  buttonBox->button( QDialogButtonBox::StandardButton::Ok )->setEnabled( false );

  std::function<bool( const QModelIndex & )> isItemCompatible = [ & ]( const QModelIndex & index )
  {
    if ( index.isValid() )
    {
      const QgsLayerItem *item = qobject_cast<QgsLayerItem *>( browserModel.dataItem( index ) );
      if ( item && item->mapLayerType() == layerType && item->providerKey() == providerType )
      {
        return true;
      }
    }
    return false;
  };

  connect( browserWidget, &QgsBrowserTreeView::clicked, [ & ]( const QModelIndex & index )
  {
    buttonBox->button( QDialogButtonBox::StandardButton::Ok )->setEnabled( isItemCompatible( index ) );
  } );

  dlg.setLayout( &lay );
  if ( dlg.exec() == QDialog::Accepted )
  {
    QModelIndex index = browserWidget->currentIndex();
    if ( isItemCompatible( index ) )
    {
      const QgsDataItem *item( browserModel.dataItem( index ) );
      layer->setDataSource( item->mimeUri().uri, layer->name(), item->mimeUri().providerKey, QgsDataProvider::ProviderOptions() );
    }
  }
  QgsSettings().setValue( QStringLiteral( "/Windows/selectDataSourceDialog/geometry" ), dlg.saveGeometry(), QgsSettings::Section::App );
}

QString QgsLayerTreeViewBadLayerIndicatorProvider::iconName( QgsMapLayer *layer )
{
  Q_UNUSED( layer );
  return QStringLiteral( "/mIndicatorBadLayer.svg" );
}

QString QgsLayerTreeViewBadLayerIndicatorProvider::tooltipText( QgsMapLayer *layer )
{
  Q_UNUSED( layer );
  return tr( "<b>Bad layer!</b><br>Layer data source could not be found. Click to set a new data source" );
}

bool QgsLayerTreeViewBadLayerIndicatorProvider::acceptLayer( QgsMapLayer *layer )
{
  return ! layer->isValid();
}
