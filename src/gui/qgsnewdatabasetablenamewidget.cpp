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
#include "qgssettings.h"
#include "qgsguiutils.h"
#include "qgsdatacollectionitem.h"
#include "qgsabstractdatabaseproviderconnection.h"

#include <QRegularExpression>
#include <QDialogButtonBox>
#include <QPushButton>

// List of data item provider keys that are filesystem based
QStringList QgsNewDatabaseTableNameWidget::FILESYSTEM_BASED_DATAITEM_PROVIDERS { QStringLiteral( "GPKG" ), QStringLiteral( "spatialite" ) };

QgsNewDatabaseTableNameWidget::QgsNewDatabaseTableNameWidget(
  QgsBrowserGuiModel *browserModel,
  const QStringList &providersFilter,
  QWidget *parent )
  : QgsPanelWidget( parent )
{

  // Initialize the browser
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

  mOkButton->hide();
  mOkButton->setEnabled( false );

  QStringList shownDataItemProvidersFilter;

  const auto providerList { QgsApplication::dataItemProviderRegistry()->providers() };
  for ( const auto &provider : providerList )
  {
    if ( provider->dataProviderKey().isEmpty() )
    {
      continue;
    }
    if ( ! QgsProviderRegistry::instance()->providerMetadata( provider->dataProviderKey() ) )
    {
      continue;
    }
    if ( provider->capabilities() & QgsDataProvider::DataCapability::Database )
    {
      if ( providersFilter.isEmpty() || providersFilter.contains( provider->dataProviderKey() ) )
      {
        mShownProviders.insert( provider->dataProviderKey() );
        shownDataItemProvidersFilter.push_back( provider->name() );
      }
    }
  }

  mBrowserToolbar->setIconSize( QgsGuiUtils::iconSize( true ) );

  mBrowserProxyModel.setBrowserModel( mBrowserModel );
  // If a filter was specified but the data provider could not be found
  // this makes sure no providers are shown instead of ALL of them
  if ( ! providersFilter.isEmpty() && shownDataItemProvidersFilter.isEmpty() )
  {
    shownDataItemProvidersFilter = providersFilter;
  }
  mBrowserProxyModel.setShownDataItemProviderKeyFilter( shownDataItemProvidersFilter );
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

  connect( mActionRefresh, &QAction::triggered, this, [ = ]
  {
    refreshModel( QModelIndex() );
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
              // Store last viewed item
              QgsSettings().setValue( QStringLiteral( "newDatabaseTableNameWidgetLastSelectedItem" ),
                                      mBrowserProxyModel.data( index, QgsBrowserGuiModel::PathRole ).toString(), QgsSettings::Section::Gui );
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

  connect( this, &QgsNewDatabaseTableNameWidget::validationChanged, mOkButton, &QWidget::setEnabled );
  connect( mOkButton, &QPushButton::clicked, this, &QgsNewDatabaseTableNameWidget::accepted );

  validate();
}

void QgsNewDatabaseTableNameWidget::setAcceptButtonVisible( bool visible )
{
  mOkButton->setVisible( visible );
}

void QgsNewDatabaseTableNameWidget::refreshModel( const QModelIndex &index )
{

  QgsDataItem *item = mBrowserModel->dataItem( index );

  if ( item && ( item->capabilities2() & Qgis::BrowserItemCapability::Fertile ) )
  {
    mBrowserModel->refresh( index );
  }

  for ( int i = 0; i < mBrowserModel->rowCount( index ); i++ )
  {
    const QModelIndex idx = mBrowserModel->index( i, 0, index );
    const QModelIndex proxyIdx = mBrowserProxyModel.mapFromSource( idx );
    QgsDataItem *child = mBrowserModel->dataItem( idx );

    // Check also expanded descendants so that the whole expanded path does not get collapsed if one item is collapsed.
    // Fast items (usually root items) are refreshed so that when collapsed, it is obvious they are if empty (no expand symbol).
    if ( mBrowserTreeView->isExpanded( proxyIdx ) || mBrowserTreeView->hasExpandedDescendant( proxyIdx ) || ( child && child->capabilities2() & Qgis::BrowserItemCapability::Fast ) )
    {
      refreshModel( idx );
    }
    else
    {
      if ( child && ( child->capabilities2() & Qgis::BrowserItemCapability::Fertile ) )
      {
        child->depopulate();
      }
    }
  }
}

void QgsNewDatabaseTableNameWidget::updateUri()
{
  const QString oldUri { mUri };
  QgsProviderMetadata *dataProviderMetadata { QgsProviderRegistry::instance()->providerMetadata( mDataProviderKey ) };
  if ( dataProviderMetadata )
  {
    QgsAbstractProviderConnection *conn { dataProviderMetadata->findConnection( mConnectionName ) };
    if ( conn )
    {
      QVariantMap uriParts = dataProviderMetadata->decodeUri( conn->uri() );
      uriParts[ QStringLiteral( "layerName" ) ] = mTableName;
      uriParts[ QStringLiteral( "schema" ) ] = mSchemaName;
      uriParts[ QStringLiteral( "table" ) ] = mTableName;
      if ( mIsFilePath )
      {
        uriParts[ QStringLiteral( "dbname" ) ] = mSchemaName;
      }
      mUri = dataProviderMetadata->encodeUri( uriParts );
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
  const QModelIndex index { mBrowserTreeView->currentIndex() };
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

void QgsNewDatabaseTableNameWidget::showEvent( QShowEvent *e )
{
  QWidget::showEvent( e );
  const QString lastSelectedPath( QgsSettings().value( QStringLiteral( "newDatabaseTableNameWidgetLastSelectedItem" ),
                                  QString(), QgsSettings::Section::Gui ).toString() );
  if ( ! lastSelectedPath.isEmpty() )
  {
    const QModelIndexList items = mBrowserProxyModel.match(
                                    mBrowserProxyModel.index( 0, 0 ),
                                    QgsBrowserGuiModel::PathRole,
                                    QVariant::fromValue( lastSelectedPath ),
                                    1,
                                    Qt::MatchRecursive );
    if ( items.count( ) > 0 )
    {
      const QModelIndex expandIndex = items.at( 0 );
      if ( expandIndex.isValid() )
      {
        mBrowserTreeView->scrollTo( expandIndex, QgsBrowserTreeView::ScrollHint::PositionAtTop );
        mBrowserTreeView->expand( expandIndex );
      }
    }
  }
}

//
// QgsNewDatabaseTableNameDialog
//
QgsNewDatabaseTableNameDialog::QgsNewDatabaseTableNameDialog( QgsBrowserGuiModel *browserModel, const QStringList &providersFilter, QWidget *parent )
  : QDialog( parent )
{
  mWidget = new QgsNewDatabaseTableNameWidget( browserModel, providersFilter );
  QVBoxLayout *vl = new QVBoxLayout();
  vl->addWidget( mWidget, 1 );
  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  connect( mWidget, &QgsNewDatabaseTableNameWidget::validationChanged, buttonBox->button( QDialogButtonBox::Ok ), &QWidget::setEnabled );
  vl->addWidget( buttonBox );
  setLayout( vl );
}

QString QgsNewDatabaseTableNameDialog::schema() const
{
  return mWidget->schema();
}

QString QgsNewDatabaseTableNameDialog::uri() const
{
  return mWidget->uri();
}

QString QgsNewDatabaseTableNameDialog::table() const
{
  return mWidget->table();
}

QString QgsNewDatabaseTableNameDialog::dataProviderKey() const
{
  return mWidget->dataProviderKey();
}

bool QgsNewDatabaseTableNameDialog::isValid() const
{
  return mWidget->isValid();
}

QString QgsNewDatabaseTableNameDialog::validationError() const
{
  return mWidget->validationError();
}
