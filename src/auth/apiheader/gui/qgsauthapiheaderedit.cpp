/***************************************************************************
    qgsauthapiheaderedit.cpp
    ------------------------
    begin                : October 2021
    copyright            : (C) 2021 by Tom Cummins
    author               : Tom Cummins
    email                : tom cumminsc9 at googlemail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthapiheaderedit.h"
#include "ui_qgsauthapiheaderedit.h"

#include "qgsapplication.h"

QgsAuthApiHeaderEdit::QgsAuthApiHeaderEdit( QWidget *parent )
  : QgsAuthMethodEdit( parent )
{
  setupUi( this );

  connect( tblwdgHeaderPairs, &QTableWidget::itemSelectionChanged, this, &QgsAuthApiHeaderEdit::headerTableSelectionChanged );
  connect( tblwdgHeaderPairs, &QTableWidget::cellChanged, this, &QgsAuthApiHeaderEdit::headerTableCellChanged );

  connect( btnAddHeaderPair, &QToolButton::clicked, this, &QgsAuthApiHeaderEdit::addHeaderPair );
  connect( btnRemoveHeaderPair, &QToolButton::clicked, this, &QgsAuthApiHeaderEdit::removeHeaderPair );
}


bool QgsAuthApiHeaderEdit::validateConfig()
{
  const bool curvalid = !emptyHeadersKeysPresent() && tblwdgHeaderPairs->rowCount() != 0;
  if ( mValid != curvalid )
  {
    mValid = curvalid;
    emit validityChanged( curvalid );
  }
  return curvalid;
}


QgsStringMap QgsAuthApiHeaderEdit::configMap() const
{
  return headerPairs();
}


void QgsAuthApiHeaderEdit::loadConfig( const QgsStringMap &configmap )
{
  clearConfig();

  mConfigMap = configmap;

  populateHeaderPairs( configmap );

  validateConfig();
}


void QgsAuthApiHeaderEdit::resetConfig()
{
  loadConfig( mConfigMap );
}


void QgsAuthApiHeaderEdit::clearConfig()
{
  clearHeaderPairs();
}


void QgsAuthApiHeaderEdit::addHeaderPair()
{
  addHeaderPairRow( QString(), QString() );
  tblwdgHeaderPairs->setFocus();
  tblwdgHeaderPairs->setCurrentCell( tblwdgHeaderPairs->rowCount() - 1, 0 );
  tblwdgHeaderPairs->edit( tblwdgHeaderPairs->currentIndex() );
}


void QgsAuthApiHeaderEdit::removeHeaderPair()
{
  tblwdgHeaderPairs->removeRow( tblwdgHeaderPairs->currentRow() );
  validateConfig();
}


void QgsAuthApiHeaderEdit::clearHeaderPairs()
{
  for ( int i = tblwdgHeaderPairs->rowCount(); i > 0 ; --i )
  {
    tblwdgHeaderPairs->removeRow( i - 1 );
  }
  validateConfig();
}


void QgsAuthApiHeaderEdit::populateHeaderPairs( const QgsStringMap &headerpairs, bool append )
{
  if ( !append )
  {
    clearHeaderPairs();
  }

  QgsStringMap::const_iterator i = headerpairs.constBegin();
  while ( i != headerpairs.constEnd() )
  {
    addHeaderPairRow( i.key(), i.value() );
    ++i;
  }
}


void QgsAuthApiHeaderEdit::headerTableSelectionChanged()
{
  const bool hasPair = tblwdgHeaderPairs->selectedItems().count() > 0;
  btnRemoveHeaderPair->setEnabled( hasPair );
  validateConfig();
}


void QgsAuthApiHeaderEdit::headerTableCellChanged( const int row, const int column )
{
  Q_UNUSED( row );
  Q_UNUSED( column );

  validateConfig();
}


bool QgsAuthApiHeaderEdit::emptyHeadersKeysPresent()
{
  const int rowCount = tblwdgHeaderPairs->rowCount();
  QgsDebugMsgLevel( QStringLiteral( "Validate header table contains valid header keys for %1 rows" ).arg( rowCount ), 2 );

  for ( int i = 0; i < rowCount; ++i )
  {
    if ( tblwdgHeaderPairs->item( i, 0 )->text().isEmpty() )
    {
      return true;
    }
  }

  return false;
}


void QgsAuthApiHeaderEdit::addHeaderPairRow( const QString &key, const QString &val )
{
  const int rowCount = tblwdgHeaderPairs->rowCount();
  tblwdgHeaderPairs->insertRow( rowCount );

  const Qt::ItemFlags itemFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable
                                  | Qt::ItemIsEditable | Qt::ItemIsDropEnabled;

  QTableWidgetItem *keyItem = new QTableWidgetItem( key );
  keyItem->setFlags( itemFlags );
  tblwdgHeaderPairs->setItem( rowCount, 0, keyItem );

  QTableWidgetItem *valueItem = new QTableWidgetItem( val );
  keyItem->setFlags( itemFlags );
  tblwdgHeaderPairs->setItem( rowCount, 1, valueItem );

  validateConfig();
}


QgsStringMap QgsAuthApiHeaderEdit::headerPairs() const
{
  QgsStringMap headerPairs;
  for ( int i = 0; i < tblwdgHeaderPairs->rowCount(); ++i )
  {
    if ( tblwdgHeaderPairs->item( i, 0 )->text().isEmpty() )
    {
      continue;
    }

    const QString headerKey = tblwdgHeaderPairs->item( i, 0 )->text();
    const QString headerValue = tblwdgHeaderPairs->item( i, 1 )->text();
    headerPairs.insert( headerKey, headerValue );
  }
  return headerPairs;
}
