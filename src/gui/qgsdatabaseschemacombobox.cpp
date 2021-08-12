/***************************************************************************
   qgsdatabaseschemacombobox.cpp
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

#include "qgsdatabaseschemacombobox.h"
#include "qgsdatabaseschemamodel.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsapplication.h"
#include <QHBoxLayout>
#include <QToolButton>

QgsDatabaseSchemaComboBox::QgsDatabaseSchemaComboBox( const QString &provider, const QString &connection, QWidget *parent )
  : QWidget( parent )
  , mProvider( provider )
{
  if ( !provider.isEmpty() && !connection.isEmpty() )
    mModel = new QgsDatabaseSchemaModel( provider, connection, this );
  init();
}

QgsDatabaseSchemaComboBox::QgsDatabaseSchemaComboBox( QgsAbstractDatabaseProviderConnection *connection, QWidget *parent )
  : QWidget( parent )
{
  mModel = new QgsDatabaseSchemaModel( connection, this );
  init();
}

void QgsDatabaseSchemaComboBox::setAllowEmptySchema( bool allowEmpty )
{
  mAllowEmpty = allowEmpty;
  if ( mModel )
    mModel->setAllowEmptySchema( mAllowEmpty );
}

bool QgsDatabaseSchemaComboBox::allowEmptySchema() const
{
  return mAllowEmpty;
}

void QgsDatabaseSchemaComboBox::init()
{
  mComboBox = new QComboBox();

  mSortModel = new QgsDatabaseSchemaComboBoxSortModel( this );

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
  refreshButton->setToolTip( tr( "Refresh schemas" ) );
  refreshButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionRefresh.svg" ) ) );
  l->addWidget( refreshButton );
  setLayout( l );

  connect( refreshButton, &QToolButton::clicked, this, &QgsDatabaseSchemaComboBox::refreshSchemas );

  connect( mComboBox, static_cast < void ( QComboBox::* )( int ) > ( &QComboBox::activated ), this, &QgsDatabaseSchemaComboBox::indexChanged );
  connect( mSortModel, &QAbstractItemModel::rowsInserted, this, &QgsDatabaseSchemaComboBox::rowsChanged );
  connect( mSortModel, &QAbstractItemModel::rowsRemoved, this, &QgsDatabaseSchemaComboBox::rowsChanged );
}

void QgsDatabaseSchemaComboBox::setSchema( const QString &schema )
{
  if ( schema == currentSchema() )
    return;

  if ( schema.isEmpty() )
  {
    if ( mAllowEmpty )
      mComboBox->setCurrentIndex( 0 );
    else
      mComboBox->setCurrentIndex( -1 );

    emit schemaChanged( QString() );
    return;
  }

  const QModelIndexList idx = mSortModel->match( mSortModel->index( 0, 0 ), Qt::DisplayRole, schema, 1, Qt::MatchFixedString | Qt::MatchCaseSensitive );
  if ( !idx.empty() )
  {
    const QModelIndex proxyIdx = idx.at( 0 );
    if ( proxyIdx.isValid() )
    {
      mComboBox->setCurrentIndex( proxyIdx.row() );
      emit schemaChanged( currentSchema() );
      return;
    }
  }
  mComboBox->setCurrentIndex( -1 );
  emit schemaChanged( QString() );
}

void QgsDatabaseSchemaComboBox::setConnectionName( const QString &connection, const QString &provider )
{
  if ( !provider.isEmpty() )
    mProvider = provider;

  const QString oldSchema = currentSchema();
  QgsDatabaseSchemaModel *oldModel = mModel;
  if ( !connection.isEmpty() && !mProvider.isEmpty() )
  {
    mModel = new QgsDatabaseSchemaModel( mProvider, connection, this );
    mModel->setAllowEmptySchema( mAllowEmpty );
    mSortModel->setSourceModel( mModel );
  }
  else
  {
    mModel = nullptr;
    mSortModel->setSourceModel( nullptr );
  }
  if ( oldModel )
    oldModel->deleteLater();

  if ( currentSchema() != oldSchema )
    setSchema( oldSchema );
}

void QgsDatabaseSchemaComboBox::refreshSchemas()
{
  const QString oldSchema = currentSchema();
  if ( mModel )
    mModel->refresh();
  setSchema( oldSchema );
}

QString QgsDatabaseSchemaComboBox::currentSchema() const
{
  const QModelIndex proxyIndex = mSortModel->index( mComboBox->currentIndex(), 0 );
  if ( !proxyIndex.isValid() )
  {
    return QString();
  }

  return mSortModel->data( proxyIndex, Qt::DisplayRole ).toString();
}

void QgsDatabaseSchemaComboBox::indexChanged( int i )
{
  Q_UNUSED( i )
  emit schemaChanged( currentSchema() );
}

void QgsDatabaseSchemaComboBox::rowsChanged()
{
  if ( mComboBox->count() == 1 || ( mAllowEmpty && mComboBox->count() == 2 && mComboBox->currentIndex() == 1 ) )
  {
    //currently selected connection item has changed
    emit schemaChanged( currentSchema() );
  }
  else if ( mComboBox->count() == 0 )
  {
    emit schemaChanged( QString() );
  }
}


///@cond PRIVATE
QgsDatabaseSchemaComboBoxSortModel::QgsDatabaseSchemaComboBoxSortModel( QObject *parent )
  : QSortFilterProxyModel( parent )
{

}

bool QgsDatabaseSchemaComboBoxSortModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  // empty row is always first
  if ( sourceModel()->data( left, QgsDatabaseSchemaModel::RoleEmpty ).toBool() )
    return true;
  else if ( sourceModel()->data( right, QgsDatabaseSchemaModel::RoleEmpty ).toBool() )
    return false;

  // default mode is alphabetical order
  const QString leftStr = sourceModel()->data( left ).toString();
  const QString rightStr = sourceModel()->data( right ).toString();
  return QString::localeAwareCompare( leftStr, rightStr ) < 0;
}

///@endcond
