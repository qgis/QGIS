/***************************************************************************
  qgsdatasourceselectdialog.cpp - QgsDataSourceSelectDialog

 ---------------------
 begin                : 1.11.2018
 copyright            : (C) 2018 by ale
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatasourceselectdialog.h"
#include "ui_qgsdatasourceselectdialog.h"
#include "qgssettings.h"
#include "qgis.h"

#include <QPushButton>

QgsDataSourceSelectDialog::QgsDataSourceSelectDialog( bool setFilterByLayerType,
    const QgsMapLayer::LayerType &layerType,
    QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );
  setWindowTitle( tr( "Select a data source" ) );
  QByteArray dlgGeom( QgsSettings().value( QStringLiteral( "/Windows/selectDataSourceDialog/geometry" ), QVariant(), QgsSettings::Section::Gui ).toByteArray() );
  restoreGeometry( dlgGeom );

  mBrowserModel.initialize();
  mBrowserProxyModel.setBrowserModel( &mBrowserModel );
  mBrowserTreeView->setHeaderHidden( true );

  if ( setFilterByLayerType )
  {
    setLayerTypeFilter( layerType );
  }
  else
  {
    mBrowserTreeView->setModel( &mBrowserProxyModel );
    buttonBox->button( QDialogButtonBox::StandardButton::Ok )->setEnabled( false );
  }
  connect( mBrowserTreeView, &QgsBrowserTreeView::clicked, this, &QgsDataSourceSelectDialog::onLayerSelected );
}


QgsDataSourceSelectDialog::~QgsDataSourceSelectDialog()
{
  QgsSettings().setValue( QStringLiteral( "/Windows/selectDataSourceDialog/geometry" ), saveGeometry(), QgsSettings::Section::Gui );
}

void QgsDataSourceSelectDialog::setLayerTypeFilter( const QgsMapLayer::LayerType &layerType )
{
  mBrowserProxyModel.setFilterByLayerType( true );
  mBrowserProxyModel.setLayerType( layerType );
  // reset model and button
  mBrowserTreeView->setModel( &mBrowserProxyModel );
  buttonBox->button( QDialogButtonBox::StandardButton::Ok )->setEnabled( false );
}

QgsMimeDataUtils::Uri QgsDataSourceSelectDialog::uri() const
{
  return mUri;
}

void QgsDataSourceSelectDialog::onLayerSelected( const QModelIndex &index )
{
  bool isLayerCompatible = false;
  mUri = QgsMimeDataUtils::Uri();
  if ( index.isValid() )
  {
    const QgsDataItem *dataItem( mBrowserProxyModel.dataItem( index ) );
    if ( dataItem )
    {
      const QgsLayerItem *layerItem = qobject_cast<const QgsLayerItem *>( dataItem );
      if ( layerItem && ( ! mBrowserProxyModel.filterByLayerType() ||
                          ( layerItem->mapLayerType() == mBrowserProxyModel.layerType() ) ) )
      {
        isLayerCompatible = true;
        mUri = layerItem->mimeUri();
      }
    }
  }
  buttonBox->button( QDialogButtonBox::StandardButton::Ok )->setEnabled( isLayerCompatible );
}

