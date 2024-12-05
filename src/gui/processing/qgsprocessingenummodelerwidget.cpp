/***************************************************************************
                             qgsprocessingenummodelerwidget.cpp
                             ------------------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingenummodelerwidget.h"
#include "moc_qgsprocessingenummodelerwidget.cpp"
#include "qgsgui.h"
#include <QMessageBox>
#include <QToolButton>

///@cond NOT_STABLE

QgsProcessingEnumModelerWidget::QgsProcessingEnumModelerWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mModel = new QStandardItemModel( this );
  mItemList->setModel( mModel );
  connect( mModel, &QStandardItemModel::itemChanged, this, &QgsProcessingEnumModelerWidget::onItemChanged );

  connect( mButtonAdd, &QToolButton::clicked, this, &QgsProcessingEnumModelerWidget::addItem );
  connect( mButtonRemove, &QToolButton::clicked, this, [=] { removeItems( false ); } );
  connect( mButtonClear, &QToolButton::clicked, this, [=] { removeItems( true ); } );
}

void QgsProcessingEnumModelerWidget::addItem()
{
  QStandardItem *item = new QStandardItem( tr( "new item" ) );
  item->setCheckable( true );
  item->setDropEnabled( false );
  item->setData( Qt::Unchecked );
  mModel->appendRow( item );
}

void QgsProcessingEnumModelerWidget::removeItems( const bool removeAll )
{
  if ( removeAll )
  {
    if ( QMessageBox::question( nullptr, tr( "Delete items" ), tr( "Are you sure you want to delete all items" ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::Yes )
      mModel->clear();
  }
  else
  {
    QModelIndexList selected = mItemList->selectionModel()->selectedIndexes();
    QSet<int> rows;
    rows.reserve( selected.count() );
    for ( const QModelIndex &i : selected )
      rows << i.row();

    QList<int> rowsToDelete = qgis::setToList( rows );
    std::sort( rowsToDelete.begin(), rowsToDelete.end(), std::greater<int>() );

    mItemList->setUpdatesEnabled( false );
    for ( int i : std::as_const( rowsToDelete ) )
      mModel->removeRows( i, 1 );
    mItemList->setUpdatesEnabled( true );
  }
}

void QgsProcessingEnumModelerWidget::onItemChanged( QStandardItem *item )
{
  int checkedItemIndex = -1;
  for ( int i = 0; i < mModel->rowCount(); i++ )
  {
    QStandardItem *itm = mModel->item( i );
    if ( itm->checkState() == Qt::Checked && itm->data() == Qt::Checked )
    {
      checkedItemIndex = i;
      break;
    }
  }

  mModel->blockSignals( true );
  if ( checkedItemIndex < 0 )
  {
    item->setData( item->checkState() );
  }
  else
  {
    if ( mAllowMultiple->isChecked() )
    {
      item->setData( item->checkState() );
    }
    else
    {
      mModel->item( checkedItemIndex )->setCheckState( Qt::Unchecked );
      mModel->item( checkedItemIndex )->setData( Qt::Unchecked );
      item->setData( item->checkState() );
    }
  }
  mModel->blockSignals( false );
}

QStringList QgsProcessingEnumModelerWidget::options() const
{
  QStringList options;
  options.reserve( mModel->rowCount() );
  for ( int i = 0; i < mModel->rowCount(); ++i )
  {
    options << mModel->item( i )->text();
  }
  return options;
}

void QgsProcessingEnumModelerWidget::setOptions( const QStringList &options )
{
  for ( const QString &option : options )
  {
    QStandardItem *item = new QStandardItem( option );
    item->setCheckable( true );
    item->setDropEnabled( false );
    item->setData( Qt::Unchecked );
    mModel->appendRow( item );
  }
}

QVariant QgsProcessingEnumModelerWidget::defaultOptions() const
{
  QVariantList defaults;
  for ( int i = 0; i < mModel->rowCount(); ++i )
  {
    if ( mModel->item( i )->checkState() == Qt::Checked )
      defaults << i;
  }
  QVariant val( defaults );
  return val;
}

void QgsProcessingEnumModelerWidget::setDefaultOptions( const QVariant &defaultValue )
{
  if ( !defaultValue.isValid() )
    return;

  QVariant val = defaultValue;
  QList<int> values;
  if ( val.userType() == QMetaType::Type::QVariantList || val.userType() == QMetaType::Type::QStringList )
  {
    for ( const QVariant &var : val.toList() )
      values << var.toInt();
  }
  else if ( val.userType() == QMetaType::Type::QString )
  {
    QStringList split = val.toString().split( ',' );
    for ( const QString &var : split )
      values << var.toInt();
  }
  else if ( val.userType() == QMetaType::Type::Int )
  {
    values << val.toInt();
  }

  QStandardItem *item;
  for ( const int &i : values )
  {
    item = mModel->item( i );
    if ( item )
    {
      item->setCheckState( Qt::Checked );
      item->setData( Qt::Checked );
    }
  }
}

bool QgsProcessingEnumModelerWidget::allowMultiple() const
{
  return mAllowMultiple->isChecked();
}

void QgsProcessingEnumModelerWidget::setAllowMultiple( bool allowMultiple )
{
  mAllowMultiple->setChecked( allowMultiple );
}

///@endcond
