/***************************************************************************
  qgsnewdatabasetablenamewidget.cpp - QgsNewDatabaseTableNameWidget

 ---------------------
 begin                : 9.3.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QTreeWidgetItemIterator>

#include "qgsnewdatabasetablenamewidget.h"
#include "qgsapplication.h"
#include "qgsdataitemproviderregistry.h"
#include "qgsdataitemprovider.h"

QgsNewDatabaseTableNameWidget::QgsNewDatabaseTableNameWidget(
  QgsBrowserGuiModel *browserModel,
  const QStringList &providersFilter,
  QWidget *parent )
  : QWidget( parent )
{


  // Initalize the browser
  if ( ! browserModel )
  {
    mBrowserModel = new QgsBrowserGuiModel( this );
    mBrowserModel->initialize();
  }
  else
  {
    mBrowserModel = browserModel;
    mBrowserModel->initialize();
  }

  setupUi( this );

  QStringList hiddenProviders
  {
    QStringLiteral( "special:Favorites" ),
    QStringLiteral( "special:Drives" ),
    QStringLiteral( "special:Volumes" ),
    QStringLiteral( "special:Home" ),
    QStringLiteral( "special:ProjectHome" )
  };

  const auto providerList { QgsApplication::dataItemProviderRegistry()->providers() };
  for ( const auto &provider : providerList )
  {
    if ( provider->capabilities() & QgsDataProvider::DataCapability::Database )
    {
      if ( ! providersFilter.isEmpty() && ! providersFilter.contains( provider->name() ) )
      {
        hiddenProviders.push_back( provider->name() );
      }
      else
      {
        mShownProviders.insert( provider->name() );
      }
    }
    else
    {
      hiddenProviders.push_back( provider->name() );
    }
  }

  mBrowserProxyModel.setBrowserModel( mBrowserModel );
  mBrowserProxyModel.setDataItemProviderKeyFilter( hiddenProviders );
  mBrowserProxyModel.setShowLayers( false );
  mBrowserTreeView->setHeaderHidden( true );
  mBrowserTreeView->setExpandsOnDoubleClick( false );
  mBrowserTreeView->setModel( &mBrowserProxyModel );
  mBrowserTreeView->setBrowserModel( mBrowserModel );

  // Connections
  connect( mNewTableName, &QLineEdit::textChanged, this, [ = ]
  {
    mTableName = mNewTableName->text();
    emit tableNameChanged( mTableName );
    validate();
  } );

  connect( mBrowserTreeView, &QgsBrowserTreeView::clicked, this, [ = ]( const QModelIndex & index )
  {
    if ( index.isValid() )
    {
      const QgsDataItem *dataItem( mBrowserProxyModel.dataItem( index ) );
      if ( dataItem )
      {
        const QgsDataCollectionItem *collectionItem = qobject_cast<const QgsDataCollectionItem *>( dataItem );
        if ( collectionItem )
        {
          if ( mShownProviders.contains( collectionItem->name() ) )
          {
            if ( mDataProviderName != collectionItem->name() )
            {
              mSchemaName.clear();
              mDataProviderName = collectionItem->name();
            }
          }
          else
          {
            mSchemaName = collectionItem->name();
            emit schemaNameChanged( mSchemaName );
          }
          validate();
        }
      }
    }
  } );

  mValidationResults->hide();

}

QString QgsNewDatabaseTableNameWidget::schema()
{
  return mSchemaName;
}

QString QgsNewDatabaseTableNameWidget::table()
{
  return mTableName;
}

QString QgsNewDatabaseTableNameWidget::dataItemProviderName()
{
  return mDataProviderName;
}

void QgsNewDatabaseTableNameWidget::validate()
{
  // Check table uniqueness
  mIsValid = ! mDataProviderName.isEmpty() &&
             mShownProviders.contains( mDataProviderName ) &&
             ! mSchemaName.isEmpty() &&
             ! mTableName.isEmpty() &&
             ! tableNames( ).contains( mTableName );

  mValidationError.clear();

  if ( ! mIsValid )
  {

    if ( mTableName.isEmpty() )
    {
      mValidationError = tr( "Enter a unique name for the new table" );
    }
    else if ( mSchemaName.isEmpty() )
    {
      mValidationError = tr( "Select a database schema" );
    }
    else if ( tableNames( ).contains( mTableName ) )
    {
      mValidationError = tr( "A table named '%1' already exists" ).arg( mTableName );
    }
    else
    {
      mValidationError = tr( "Select a schema and enter a unique name for the new table" );
    }
  }
  mValidationResults->setText( mValidationError );
  mValidationResults->setVisible( ! mIsValid );
  emit validationChanged( mIsValid );
}

QStringList QgsNewDatabaseTableNameWidget::tableNames()
{
  QStringList tableNames;
  QModelIndex index { mBrowserTreeView->currentIndex() };
  if ( index.isValid() )
  {
    for ( int row = 0; row < mBrowserProxyModel.rowCount( ); ++row )
    {
      // Column 1 contains the
      index = mBrowserProxyModel.index( row, 1, index );
      if ( index.isValid() )
      {
        const QgsDataItem *dataItem { mBrowserProxyModel.dataItem( index ) };
        if ( dataItem )
        {
          tableNames.push_back( dataItem->name() );
        }
      }
    }
  }
  return tableNames;
}

bool QgsNewDatabaseTableNameWidget::isValid() const
{
  return mIsValid;
}

QString QgsNewDatabaseTableNameWidget::validationError()
{
  return mValidationError;
}
