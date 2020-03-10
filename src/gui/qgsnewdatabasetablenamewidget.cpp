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
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"

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

  mValidationResults->setStyleSheet( QStringLiteral( "* { font-weight: bold; color: red; }" ) );

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
    if ( provider->dataProviderKey().isEmpty() )
    {
      hiddenProviders.push_back( provider->name() );
      continue;
    }
    QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( provider->dataProviderKey() ) };
    if ( ! md )
    {
      hiddenProviders.push_back( provider->name() );
      continue;
    }
    if ( provider->capabilities() & QgsDataProvider::DataCapability::Database )
    {
      if ( ! providersFilter.isEmpty() && ! providersFilter.contains( provider->dataProviderKey() ) )
      {
        hiddenProviders.push_back( provider->name() );
      }
      else
      {
        mShownProviders.insert( provider->dataProviderKey() );
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
          const QString providerKey { QgsApplication::dataItemProviderRegistry()->dataProviderKey( dataItem->providerKey() ) };
          if ( mShownProviders.contains( providerKey ) )
          {
            bool validationRequired { false };
            const QString oldSchema { mSchemaName };

            if ( mDataProviderKey != providerKey )
            {
              mSchemaName.clear();
              emit providerKeyChanged( providerKey );
              mDataProviderKey = providerKey;
              validate();
            }

            if ( collectionItem->layerCollection( ) )
            {
              mSchemaName = collectionItem->name(); // it may be cleared
              if ( oldSchema != collectionItem->name() )
              {
                emit schemaNameChanged( mSchemaName );
                validationRequired = true;
              }
            }

            if ( validationRequired )
            {
              validate();
            }
          }
        }
      }
    }
  } );

  validate();

}

QString QgsNewDatabaseTableNameWidget::schema()
{
  return mSchemaName;
}

QString QgsNewDatabaseTableNameWidget::table()
{
  return mTableName;
}

QString QgsNewDatabaseTableNameWidget::dataProviderKey()
{
  return mDataProviderKey;
}

void QgsNewDatabaseTableNameWidget::validate()
{
  const bool wasValid { mIsValid };
  // Check table uniqueness
  mIsValid = ! mDataProviderKey.isEmpty() &&
             mShownProviders.contains( mDataProviderKey ) &&
             ! mSchemaName.isEmpty() &&
             ! mTableName.isEmpty() &&
             ! tableNames( ).contains( mTableName );

  mValidationError.clear();

  if ( ! mIsValid )
  {
    if ( mTableName.isEmpty() && mSchemaName.isEmpty() )
    {
      mValidationError = tr( "Select a database schema and enter a unique name for the new table" );
    }
    else if ( ! mTableName.isEmpty() &&
              ! mSchemaName.isEmpty() &&
              tableNames( ).contains( mTableName ) )
    {
      mValidationError = tr( "A table named '%1' already exists" ).arg( mTableName );
    }
    else if ( mSchemaName.isEmpty() )
    {
      mValidationError = tr( "Select a database schema" );
    }
    else if ( mTableName.isEmpty() )
    {
      mValidationError = tr( "Enter a unique name for the new table" );
    }
    else if ( tableNames( ).contains( mTableName ) )
    {
      mValidationError = tr( "A table named '%1' already exists" ).arg( mTableName );
    }
    else
    {
      mValidationError = tr( "Select a database schema and enter a unique name for the new table" );
    }
  }
  mValidationResults->setText( mValidationError );
  mValidationResults->setVisible( ! mIsValid );
  if ( wasValid != mIsValid )
  {
    emit validationChanged( mIsValid );
  }
}

QStringList QgsNewDatabaseTableNameWidget::tableNames()
{
  QStringList tableNames;
  QModelIndex index { mBrowserTreeView->currentIndex() };
  if ( index.isValid() )
  {
    QgsDataItem *dataItem { mBrowserProxyModel.dataItem( index ) };
    if ( dataItem )
    {
      const QString dataProviderKey { QgsApplication::dataItemProviderRegistry()->dataProviderKey( dataItem->providerKey() ) };
      if ( ! dataProviderKey.isEmpty() )
      {
        QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( dataProviderKey ) };
        if ( md )
        {
          QgsDataItem *parentDataItem { dataItem->parent() };
          if ( parentDataItem )
          {
            QgsAbstractProviderConnection *conn { md->findConnection( parentDataItem->name() ) };
            const QString cacheKey { conn->uri() + dataItem->name() };
            if ( mTableNamesCache.contains( cacheKey ) )
            {
              tableNames = mTableNamesCache.value( cacheKey );
            }
            else if ( conn && static_cast<QgsAbstractDatabaseProviderConnection *>( conn ) )
            {
              const auto tables { static_cast<QgsAbstractDatabaseProviderConnection *>( conn )->tables( dataItem->name() ) };
              for ( const auto &tp : tables )
              {
                tableNames.push_back( tp.tableName() );
              }
              mTableNamesCache[ cacheKey ] = tableNames;
            }
          }
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
