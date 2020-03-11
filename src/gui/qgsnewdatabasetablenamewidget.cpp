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


// List of data item provider keys that are filesystem based
QStringList QgsNewDatabaseTableNameWidget::FILESYSTEM_BASED_DATAITEM_PROVIDERS { QStringLiteral( "GPKG" ), QStringLiteral( "SPATIALITE" ) };

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
    if ( provider->dataProviderKey().isEmpty() )
    {
      hiddenProviders.push_back( provider->name() );
      continue;
    }
    QgsProviderMetadata *metadata { QgsProviderRegistry::instance()->providerMetadata( provider->dataProviderKey() ) };
    if ( ! metadata )
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
  mBrowserTreeView->setModel( &mBrowserProxyModel );
  mBrowserTreeView->setBrowserModel( mBrowserModel );

  // Connections
  connect( mNewTableName, &QLineEdit::textChanged, this, [ = ]
  {
    mTableName = mNewTableName->text();
    emit tableNameChanged( mTableName );
    updateUri();
    validate();
  } );

  connect( mBrowserTreeView, &QgsBrowserTreeView::clicked, this, [ = ]( const QModelIndex & index )
  {
    if ( index.isValid() )
    {
      if ( const QgsDataItem *dataItem = mBrowserProxyModel.dataItem( index ) )
      {
        if ( const QgsDataCollectionItem *collectionItem = qobject_cast<const QgsDataCollectionItem *>( dataItem ) )
        {
          const QString providerKey { QgsApplication::dataItemProviderRegistry()->dataProviderKey( dataItem->providerKey() ) };
          bool validationRequired { false };
          const QString oldSchema { mSchemaName };

          if ( mDataProviderKey != providerKey )
          {
            mSchemaName.clear();
            mDataProviderKey = providerKey;
            emit providerKeyChanged( providerKey );
            validationRequired = true;
          }

          if ( collectionItem->layerCollection( ) )
          {
            mIsFilePath = FILESYSTEM_BASED_DATAITEM_PROVIDERS.contains( collectionItem->providerKey() );
            // Data items for filesystem based items are in the form gpkg://path/to/file.gpkg
            mSchemaName = mIsFilePath ? collectionItem->path().remove( QRegularExpression( QStringLiteral( "^[A-z]+:/" ) ) ) : collectionItem->name(); // it may be cleared
            mConnectionName = mIsFilePath ? collectionItem->name() : collectionItem->parent()->name();
            if ( oldSchema != mSchemaName )
            {
              emit schemaNameChanged( mSchemaName );
              validationRequired = true;
            }
          }

          if ( validationRequired )
          {
            updateUri();
            validate();
          }
        }
      }
    }
  } );

  validate();
}

void QgsNewDatabaseTableNameWidget::updateUri()
{
  const QString oldUri { mUri };
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( mDataProviderKey ) };
  if ( md )
  {
    QgsAbstractProviderConnection *conn { md->findConnection( mConnectionName ) };
    if ( conn )
    {
      QVariantMap uriParts { md->decodeUri( conn->uri() ) };
      uriParts[ QStringLiteral( "layerName" ) ] = mTableName;
      uriParts[ QStringLiteral( "schema" ) ] = mSchemaName;
      uriParts[ QStringLiteral( "table" ) ] = mTableName;
      if ( mIsFilePath )
      {
        uriParts[ QStringLiteral( "dbname" ) ] = mSchemaName;
      }
      mUri = md->encodeUri( uriParts );
    }
    else
    {
      mUri = QString();
    }
  }
  else
  {
    mUri = QString();
  }

  if ( mUri != oldUri )
  {
    emit uriChanged( mUri );
  }
}

QString QgsNewDatabaseTableNameWidget::schema() const
{
  return mSchemaName;
}

QString QgsNewDatabaseTableNameWidget::uri() const
{
  return mUri;
}

QString QgsNewDatabaseTableNameWidget::table() const
{
  return mTableName;
}

QString QgsNewDatabaseTableNameWidget::dataProviderKey() const
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

  // Whether to show it red
  bool isError { false };

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
      isError = true;
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

  mValidationResults->setStyleSheet( isError ?
                                     QStringLiteral( "* { color: red; }" ) :
                                     QString() );

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
        QgsProviderMetadata *metadata { QgsProviderRegistry::instance()->providerMetadata( dataProviderKey ) };
        if ( metadata )
        {
          QgsDataItem *parentDataItem { mIsFilePath ? dataItem : dataItem->parent() };
          if ( parentDataItem )
          {
            QgsAbstractProviderConnection *conn { metadata->findConnection( parentDataItem->name() ) };
            if ( conn )
            {
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
  }
  return tableNames;
}

bool QgsNewDatabaseTableNameWidget::isValid() const
{
  return mIsValid;
}

QString QgsNewDatabaseTableNameWidget::validationError() const
{
  return mValidationError;
}
