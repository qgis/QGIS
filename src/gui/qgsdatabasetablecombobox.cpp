/***************************************************************************
   qgsdatabasetablecombobox.cpp
    --------------------------------
   Date                 : March 2020
   Copyright            : (C) 2020 Nyall Dawson
   Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgsdatabasetablecombobox.h"
#include "qgsdatabasetablemodel.h"
#include "qgsapplication.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include <QHBoxLayout>
#include <QToolButton>

QgsDatabaseTableComboBox::QgsDatabaseTableComboBox( const QString &provider, const QString &connection, const QString &schema, QWidget *parent )
  : QWidget( parent )
  , mProvider( provider )
  , mConnection( connection )
  , mSchema( schema )
{
  if ( !provider.isEmpty() && !connection.isEmpty() )
    mModel = new QgsDatabaseTableModel( provider, connection, schema, this );
  init();
}

QgsDatabaseTableComboBox::QgsDatabaseTableComboBox( QgsAbstractDatabaseProviderConnection *connection, const QString &schema, QWidget *parent )
  : QWidget( parent )
  , mSchema( schema )
{
  mModel = new QgsDatabaseTableModel( connection, schema, this );
  init();
}

void QgsDatabaseTableComboBox::setAllowEmptyTable( bool allowEmpty )
{
  mAllowEmpty = allowEmpty;
  if ( mModel )
    mModel->setAllowEmptyTable( allowEmpty );
}

bool QgsDatabaseTableComboBox::allowEmptyTable() const
{
  return mAllowEmpty;
}

void QgsDatabaseTableComboBox::init()
{
  mComboBox = new QComboBox();

  mSortModel = new QgsDatabaseTableComboBoxSortModel( this );
  if ( mModel )
    mSortModel->setSourceModel( mModel );
  mSortModel->setSortRole( Qt::DisplayRole );
  mSortModel->setSortLocaleAware( true );
  mSortModel->setSortCaseSensitivity( Qt::CaseInsensitive );
  mSortModel->setDynamicSortFilter( true );
  mSortModel->sort( 0 );

  mComboBox->setModel( mSortModel );

  QHBoxLayout *l = new QHBoxLayout();
  l->setContentsMargins( 0, 0, 0, 0 );
  l->addWidget( mComboBox );

  QToolButton *refreshButton = new QToolButton();
  refreshButton->setAutoRaise( true );
  refreshButton->setToolTip( tr( "Refresh tables" ) );
  refreshButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionRefresh.svg" ) ) );
  l->addWidget( refreshButton );
  setLayout( l );

  connect( refreshButton, &QToolButton::clicked, this, &QgsDatabaseTableComboBox::refreshTables );

  connect( mComboBox, static_cast < void ( QComboBox::* )( int ) > ( &QComboBox::activated ), this, &QgsDatabaseTableComboBox::indexChanged );
  connect( mSortModel, &QAbstractItemModel::rowsInserted, this, &QgsDatabaseTableComboBox::rowsChanged );
  connect( mSortModel, &QAbstractItemModel::rowsRemoved, this, &QgsDatabaseTableComboBox::rowsChanged );
}

void QgsDatabaseTableComboBox::setTable( const QString &table, const QString &schema )
{
  if ( schema == currentSchema() && table == currentTable() )
    return;

  if ( table.isEmpty() )
  {
    if ( mAllowEmpty )
      mComboBox->setCurrentIndex( 0 );
    else
      mComboBox->setCurrentIndex( -1 );

    emit tableChanged( QString() );
    return;
  }

  const QModelIndexList idxs = mSortModel->match( mSortModel->index( 0, 0 ), QgsDatabaseTableModel::RoleTableName, table, -1, Qt::MatchFixedString | Qt::MatchCaseSensitive );
  for ( const QModelIndex &proxyIdx : idxs )
  {
    if ( proxyIdx.isValid()  && proxyIdx.data( QgsDatabaseTableModel::RoleTableName ).toString() == table
         && ( schema.isEmpty() || proxyIdx.data( QgsDatabaseTableModel::RoleSchema ).toString() == schema ) )
    {
      mComboBox->setCurrentIndex( proxyIdx.row() );
      emit tableChanged( currentTable(), currentSchema() );
      return;
    }
  }
  mComboBox->setCurrentIndex( -1 );
  emit tableChanged( QString() );
}

void QgsDatabaseTableComboBox::setConnectionName( const QString &connection, const QString &provider )
{
  if ( provider.isEmpty() && mConnection == connection )
    return;

  if ( !provider.isEmpty() )
    mProvider = provider;

  mConnection = connection;

  const QString oldTable = currentTable();
  const QString oldSchema = currentSchema();
  QgsDatabaseTableModel *oldModel = mModel;
  if ( !mConnection.isEmpty() )
  {
    mModel = new QgsDatabaseTableModel( mProvider, mConnection, mSchema, this );
    mModel->setAllowEmptyTable( mAllowEmpty );
  }
  else
    mModel = nullptr;

  mSortModel->setSourceModel( mModel );
  if ( oldModel )
    oldModel->deleteLater();
  if ( currentTable() != oldTable || currentSchema() != oldSchema )
    setTable( oldTable, oldSchema );
}

void QgsDatabaseTableComboBox::setSchema( const QString &schema )
{
  if ( schema == mSchema )
    return;
  mSchema = schema;

  if ( !mProvider.isEmpty() && !mConnection.isEmpty() )
  {
    const QString oldTable = currentTable();
    QgsDatabaseTableModel *oldModel = mModel;
    mModel = new QgsDatabaseTableModel( mProvider, mConnection, mSchema, this );
    mSortModel->setSourceModel( mModel );
    oldModel->deleteLater();
    setTable( oldTable );
  }
}

void QgsDatabaseTableComboBox::refreshTables()
{
  const QString oldSchema = currentSchema();
  const QString oldTable = currentTable();
  if ( mModel )
    mModel->refresh();
  setTable( oldTable, oldSchema );
}

QString QgsDatabaseTableComboBox::currentSchema() const
{
  const QModelIndex proxyIndex = mSortModel->index( mComboBox->currentIndex(), 0 );
  if ( !proxyIndex.isValid() )
  {
    return QString();
  }

  return mSortModel->data( proxyIndex, QgsDatabaseTableModel::RoleSchema ).toString();
}

QString QgsDatabaseTableComboBox::currentTable() const
{
  const QModelIndex proxyIndex = mSortModel->index( mComboBox->currentIndex(), 0 );
  if ( !proxyIndex.isValid() )
  {
    return QString();
  }

  return mSortModel->data( proxyIndex, QgsDatabaseTableModel::RoleTableName ).toString();
}

void QgsDatabaseTableComboBox::indexChanged( int i )
{
  Q_UNUSED( i )
  emit tableChanged( currentTable() );
}

void QgsDatabaseTableComboBox::rowsChanged()
{
  if ( mComboBox->count() == 1 || ( mAllowEmpty && mComboBox->count() == 2 && mComboBox->currentIndex() == 1 ) )
  {
    //currently selected connection item has changed
    emit tableChanged( currentTable(), currentSchema() );
  }
  else if ( mComboBox->count() == 0 )
  {
    emit tableChanged( QString() );
  }
}

///@cond PRIVATE
QgsDatabaseTableComboBoxSortModel::QgsDatabaseTableComboBoxSortModel( QObject *parent )
  : QSortFilterProxyModel( parent )
{

}

bool QgsDatabaseTableComboBoxSortModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  // empty row is always first
  if ( sourceModel()->data( left, QgsDatabaseTableModel::RoleEmpty ).toBool() )
    return true;
  else if ( sourceModel()->data( right, QgsDatabaseTableModel::RoleEmpty ).toBool() )
    return false;

  // default mode is alphabetical order
  const QString leftStr = sourceModel()->data( left ).toString();
  const QString rightStr = sourceModel()->data( right ).toString();
  return QString::localeAwareCompare( leftStr, rightStr ) < 0;
}

///@endcond
