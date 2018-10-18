/***************************************************************************
    qgsarcgisresttokenmanagerdialog.cpp
     --------------------------------------
    Date                 : October 2018
    Copyright            : (C) 2018 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsarcgisresttokenmanagerdialog.h"
#include "qgsarcgisrestutils.h"
#include "qgsgui.h"
#include <QInputDialog>

QgsArcGisRestTokenManagerTableModel::QgsArcGisRestTokenManagerTableModel( QObject *parent )
  : QAbstractTableModel( parent )
{
  const QMap< QString, QString > t = QgsArcGisRestTokenManager::tokens();
  for ( auto it = t.constBegin(); it != t.constEnd(); ++it )
  {
    mServers << it.key();
    mTokens << it.value();
  }
}

void QgsArcGisRestTokenManagerTableModel::removeTokens( const QModelIndexList &indexes )
{
  if ( indexes.isEmpty() )
    return;

  const int row = indexes.at( 0 ).row();
  beginRemoveRows( QModelIndex(), row, row );
  mServers.removeAt( row );
  mTokens.removeAt( row );
  endRemoveRows();
}

void QgsArcGisRestTokenManagerTableModel::addToken( const QString &server )
{
  beginInsertRows( QModelIndex(), mServers.count(), mServers.count() );
  mServers << server;
  mTokens << QString();
  endInsertRows();
}

void QgsArcGisRestTokenManagerTableModel::saveTokens()
{
  QMap< QString, QString > tokens;
  for ( int i = 0; i < mServers.count(); ++ i )
  {
    tokens.insert( mServers.at( i ), mTokens.at( i ) );
  }
  QgsArcGisRestTokenManager::setTokens( tokens );
}


int QgsArcGisRestTokenManagerTableModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return mServers.count();
}

int QgsArcGisRestTokenManagerTableModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent );
  return 2;
}

Qt::ItemFlags QgsArcGisRestTokenManagerTableModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags f = QAbstractItemModel::flags( index );
  return f | Qt::ItemIsEditable;
}

QVariant QgsArcGisRestTokenManagerTableModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() >= mServers.size() )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::EditRole:
      switch ( index.column() )
      {
        case ServerColumn:
          return mServers.at( index.row() );

        case TokenColumn:
          return mTokens.at( index.row() );
      }
      break;

    default:
      break;
  }

  return QVariant();
}

bool QgsArcGisRestTokenManagerTableModel::setData( const QModelIndex &index, const QVariant &value, int )
{
  if ( index.row() >= mServers.size() )
    return false;

  switch ( index.column() )
  {
    case ServerColumn:
      mServers[ index.row() ] = value.toString();
      emit dataChanged( index, index );
      return true;

    case TokenColumn:
    {
      mTokens[ index.row() ] = value.toString();
      emit dataChanged( index, index );
      return true;
    }
  }
  return false;
}

QVariant QgsArcGisRestTokenManagerTableModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( orientation == Qt::Vertical )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
      switch ( section )
      {
        case ServerColumn :
          return tr( "Server URL" );
        case TokenColumn:
          return tr( "Token" );
      }
      break;

    default:
      break;
  }

  return QVariant();
}


QgsArcGisRestTokenManagerDialog::QgsArcGisRestTokenManagerDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  mTableView->setModel( mModel );
  mTableView->resizeColumnToContents( 0 );
  mTableView->resizeColumnToContents( 1 );
  mTableView->horizontalHeader()->setSectionResizeMode( QHeaderView::Interactive );
  mTableView->horizontalHeader()->show();
  mTableView->setSelectionMode( QAbstractItemView::SingleSelection );
  mTableView->setSelectionBehavior( QAbstractItemView::SelectRows );
  mTableView->setAlternatingRowColors( true );
  connect( mAddButton, &QToolButton::clicked, this, &QgsArcGisRestTokenManagerDialog::addToken );
  connect( mRemoveButton, &QToolButton::clicked, this, &QgsArcGisRestTokenManagerDialog::removeToken );

  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
}

void QgsArcGisRestTokenManagerDialog::accept()
{
  mModel->saveTokens();
  QDialog::accept();
}

void QgsArcGisRestTokenManagerDialog::addToken()
{
  bool ok;
  const QString server = QInputDialog::getText( this, tr( "Add Token" ), tr( "Server name (http(s):// prefix is not required)" ),
                         QLineEdit::Normal, QString(), &ok );
  if ( !ok )
    return;

  mModel->addToken( server );
}

void QgsArcGisRestTokenManagerDialog::removeToken()
{
  QModelIndexList selectedIndexes = mTableView->selectionModel()->selectedIndexes();
  if ( selectedIndexes.count() > 0 )
  {
    mModel->removeTokens( selectedIndexes );
  }
}

