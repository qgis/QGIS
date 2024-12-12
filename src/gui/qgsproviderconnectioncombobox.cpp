/***************************************************************************
   qgsproviderconnectioncombobox.cpp
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

#include "qgsproviderconnectioncombobox.h"
#include "moc_qgsproviderconnectioncombobox.cpp"
#include "qgsproviderconnectionmodel.h"

QgsProviderConnectionComboBox::QgsProviderConnectionComboBox( const QString &provider, QWidget *parent )
  : QComboBox( parent )
{
  setProvider( provider );
}

QgsProviderConnectionComboBox::QgsProviderConnectionComboBox( QWidget *parent )
  : QComboBox( parent )
{
}

void QgsProviderConnectionComboBox::setProvider( const QString &provider )
{
  if ( mSortModel )
  {
    disconnect( this, static_cast<void ( QComboBox::* )( int )>( &QComboBox::activated ), this, &QgsProviderConnectionComboBox::indexChanged );
    disconnect( mSortModel, &QAbstractItemModel::rowsInserted, this, &QgsProviderConnectionComboBox::rowsChanged );
    disconnect( mSortModel, &QAbstractItemModel::rowsAboutToBeRemoved, this, &QgsProviderConnectionComboBox::rowsAboutToBeRemoved );
    disconnect( mSortModel, &QAbstractItemModel::rowsRemoved, this, &QgsProviderConnectionComboBox::rowsRemoved );
    delete mSortModel;
    delete mModel;
  }

  mModel = new QgsProviderConnectionModel( provider, this );

  mSortModel = new QgsProviderConnectionComboBoxSortModel( this );
  mSortModel->setSourceModel( mModel );
  mSortModel->setSortRole( Qt::DisplayRole );
  mSortModel->setSortLocaleAware( true );
  mSortModel->setSortCaseSensitivity( Qt::CaseInsensitive );
  mSortModel->setDynamicSortFilter( true );
  mSortModel->sort( 0 );

  setModel( mSortModel );

  connect( this, static_cast<void ( QComboBox::* )( int )>( &QComboBox::activated ), this, &QgsProviderConnectionComboBox::indexChanged );
  connect( mSortModel, &QAbstractItemModel::rowsInserted, this, &QgsProviderConnectionComboBox::rowsChanged );
  connect( mSortModel, &QAbstractItemModel::rowsAboutToBeRemoved, this, &QgsProviderConnectionComboBox::rowsAboutToBeRemoved );
  connect( mSortModel, &QAbstractItemModel::rowsRemoved, this, &QgsProviderConnectionComboBox::rowsRemoved );
}

void QgsProviderConnectionComboBox::setAllowEmptyConnection( bool allowEmpty )
{
  mModel->setAllowEmptyConnection( allowEmpty );
}

bool QgsProviderConnectionComboBox::allowEmptyConnection() const
{
  return mModel->allowEmptyConnection();
}

void QgsProviderConnectionComboBox::setConnection( const QString &connection )
{
  if ( connection == currentConnection() )
    return;

  if ( connection.isEmpty() )
  {
    if ( mModel->allowEmptyConnection() )
      setCurrentIndex( 0 );
    else
      setCurrentIndex( -1 );
    emit connectionChanged( QString() );
    return;
  }

  const QModelIndexList idx = mSortModel->match( mSortModel->index( 0, 0 ), static_cast<int>( QgsProviderConnectionModel::CustomRole::ConnectionName ), connection, Qt::MatchFixedString | Qt::MatchCaseSensitive );
  if ( !idx.empty() )
  {
    const QModelIndex proxyIdx = idx.at( 0 );
    if ( proxyIdx.isValid() )
    {
      setCurrentIndex( proxyIdx.row() );
      emit connectionChanged( currentConnection() );
      return;
    }
  }
  setCurrentIndex( -1 );
  emit connectionChanged( QString() );
}

QString QgsProviderConnectionComboBox::currentConnection() const
{
  const QModelIndex proxyIndex = mSortModel->index( currentIndex(), 0 );
  if ( !proxyIndex.isValid() )
  {
    return QString();
  }

  return mSortModel->data( proxyIndex, static_cast<int>( QgsProviderConnectionModel::CustomRole::ConnectionName ) ).toString();
}

QString QgsProviderConnectionComboBox::currentConnectionUri() const
{
  const QModelIndex proxyIndex = mSortModel->index( currentIndex(), 0 );
  if ( !proxyIndex.isValid() )
  {
    return QString();
  }

  return mSortModel->data( proxyIndex, static_cast<int>( QgsProviderConnectionModel::CustomRole::Uri ) ).toString();
}

void QgsProviderConnectionComboBox::indexChanged( int i )
{
  Q_UNUSED( i )
  emit connectionChanged( currentConnection() );
}

void QgsProviderConnectionComboBox::rowsChanged()
{
  if ( count() == 1 || ( mModel->allowEmptyConnection() && count() == 2 && currentIndex() == 1 ) )
  {
    //currently selected connection item has changed
    emit connectionChanged( currentConnection() );
  }
  else if ( count() == 0 )
  {
    emit connectionChanged( QString() );
  }
}

void QgsProviderConnectionComboBox::rowsAboutToBeRemoved()
{
  mPreviousConnection = currentConnection();
}

void QgsProviderConnectionComboBox::rowsRemoved()
{
  const QString newConnection = currentConnection();
  if ( mPreviousConnection != newConnection )
  {
    if ( mModel->allowEmptyConnection() )
    {
      // if current connection was removed, reset to empty connection item
      setCurrentIndex( 0 );
    }
    if ( currentIndex() == -1 )
    {
      // make sure we have a valid selection
      setCurrentIndex( 0 );
    }
    emit connectionChanged( currentConnection() );
  }
}


///@cond PRIVATE
QgsProviderConnectionComboBoxSortModel::QgsProviderConnectionComboBoxSortModel( QObject *parent )
  : QSortFilterProxyModel( parent )
{
}

bool QgsProviderConnectionComboBoxSortModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  // empty row is always first
  if ( sourceModel()->data( left, static_cast<int>( QgsProviderConnectionModel::CustomRole::Empty ) ).toBool() )
    return true;
  else if ( sourceModel()->data( right, static_cast<int>( QgsProviderConnectionModel::CustomRole::Empty ) ).toBool() )
    return false;

  // default mode is alphabetical order
  const QString leftStr = sourceModel()->data( left ).toString();
  const QString rightStr = sourceModel()->data( right ).toString();
  return QString::localeAwareCompare( leftStr, rightStr ) < 0;
}


///@endcond
