/***************************************************************************
                             qgsprocessingmultipleselectiondialog.cpp
                             ------------------------------------
    Date                 : February 2019
    Copyright            : (C) 2019 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingmultipleselectiondialog.h"
#include "qgsgui.h"
#include <QStandardItemModel>
#include <QStandardItem>
#include <QPushButton>
#include <QLineEdit>
#include <QToolButton>

///@cond NOT_STABLE

QgsProcessingMultipleSelectionPanelWidget::QgsProcessingMultipleSelectionPanelWidget( const QVariantList &availableOptions,
    const QVariantList &selectedOptions,
    QWidget *parent )
  : QgsPanelWidget( parent )
  , mValueFormatter( []( const QVariant & v )->QString { return v.toString(); } )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  mSelectionList->setSelectionBehavior( QAbstractItemView::SelectRows );
  mSelectionList->setSelectionMode( QAbstractItemView::ExtendedSelection );
  mSelectionList->setDragDropMode( QAbstractItemView::InternalMove );

  mButtonSelectAll = new QPushButton( tr( "Select All" ) );
  mButtonBox->addButton( mButtonSelectAll, QDialogButtonBox::ActionRole );

  mButtonClearSelection = new QPushButton( tr( "Clear Selection" ) );
  mButtonBox->addButton( mButtonClearSelection, QDialogButtonBox::ActionRole );

  mButtonToggleSelection = new QPushButton( tr( "Toggle Selection" ) );
  mButtonBox->addButton( mButtonToggleSelection, QDialogButtonBox::ActionRole );

  connect( mButtonSelectAll, &QPushButton::clicked, this, [ = ] { selectAll( true ); } );
  connect( mButtonClearSelection, &QPushButton::clicked, this, [ = ] { selectAll( false ); } );
  connect( mButtonToggleSelection, &QPushButton::clicked, this, &QgsProcessingMultipleSelectionPanelWidget::toggleSelection );

  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsProcessingMultipleSelectionPanelWidget::acceptClicked );
  populateList( availableOptions, selectedOptions );

  connect( mModel, &QStandardItemModel::itemChanged, this, &QgsProcessingMultipleSelectionPanelWidget::selectionChanged );
}

void QgsProcessingMultipleSelectionPanelWidget::setValueFormatter( const std::function<QString( const QVariant & )> &formatter )
{
  mValueFormatter = formatter;
  // update item text using new formatter
  for ( int i = 0; i < mModel->rowCount(); ++i )
  {
    mModel->item( i )->setText( mValueFormatter( mModel->item( i )->data( Qt::UserRole ) ) );
  }
}

QVariantList QgsProcessingMultipleSelectionPanelWidget::selectedOptions() const
{
  QVariantList options;
  options.reserve( mModel->rowCount() );
  for ( int i = 0; i < mModel->rowCount(); ++i )
  {
    if ( mModel->item( i )->checkState() == Qt::Checked )
      options << mModel->item( i )->data( Qt::UserRole );
  }
  return options;
}

void QgsProcessingMultipleSelectionPanelWidget::selectAll( const bool checked )
{
  const QList<QStandardItem *> items = currentItems();
  for ( QStandardItem *item : items )
  {
    item->setCheckState( checked ? Qt::Checked : Qt::Unchecked );
  }
}

void QgsProcessingMultipleSelectionPanelWidget::toggleSelection()
{
  const QList<QStandardItem *> items = currentItems();
  for ( QStandardItem *item : items )
  {
    item->setCheckState( item->checkState() == Qt::Unchecked ? Qt::Checked : Qt::Unchecked );
  }
}

QList<QStandardItem *> QgsProcessingMultipleSelectionPanelWidget::currentItems()
{
  QList<QStandardItem *> items;
  const QModelIndexList selection = mSelectionList->selectionModel()->selectedIndexes();
  if ( selection.size() > 1 )
  {
    items.reserve( selection.size() );
    for ( const QModelIndex &index : selection )
    {
      items << mModel->itemFromIndex( index );
    }
  }
  else
  {
    items.reserve( mModel->rowCount() );
    for ( int i = 0; i < mModel->rowCount(); ++i )
    {
      items << mModel->item( i );
    }
  }
  return items;
}

void QgsProcessingMultipleSelectionPanelWidget::populateList( const QVariantList &availableOptions, const QVariantList &selectedOptions )
{
  mModel = new QStandardItemModel( this );

  QVariantList remainingOptions = availableOptions;

  // we add selected options first, keeping the existing order of options
  for ( const QVariant &option : selectedOptions )
  {
//    if isinstance(t, QgsProcessingModelChildParameterSource):
//       item = QStandardItem(t.staticValue())
    // else:
    std::unique_ptr< QStandardItem > item = qgis::make_unique< QStandardItem >( mValueFormatter( option ) );
    item->setData( option, Qt::UserRole );
    item->setCheckState( Qt::Checked );
    item->setCheckable( true );
    item->setDropEnabled( false );
    mModel->appendRow( item.release() );
    remainingOptions.removeAll( option );
  }

  for ( const QVariant &option : qgis::as_const( remainingOptions ) )
  {
    std::unique_ptr< QStandardItem > item = qgis::make_unique< QStandardItem >( mValueFormatter( option ) );
    item->setData( option, Qt::UserRole );
    item->setCheckState( Qt::Unchecked );
    item->setCheckable( true );
    item->setDropEnabled( false );
    mModel->appendRow( item.release() );
  }

  mSelectionList->setModel( mModel );
}



//
// QgsProcessingMultipleSelectionDialog
//



///@endcond

QgsProcessingMultipleSelectionDialog::QgsProcessingMultipleSelectionDialog( const QVariantList &availableOptions, const QVariantList &selectedOptions, QWidget *parent, Qt::WindowFlags flags )
  : QDialog( parent, flags )
{
  setWindowTitle( tr( "Multiple Selection" ) );
  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsProcessingMultipleSelectionPanelWidget( availableOptions, selectedOptions );
  vLayout->addWidget( mWidget );
  mWidget->buttonBox()->addButton( QDialogButtonBox::Cancel );
  connect( mWidget->buttonBox(), &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mWidget->buttonBox(), &QDialogButtonBox::rejected, this, &QDialog::reject );
  setLayout( vLayout );
}

void QgsProcessingMultipleSelectionDialog::setValueFormatter( const std::function<QString( const QVariant & )> &formatter )
{
  mWidget->setValueFormatter( formatter );
}

QVariantList QgsProcessingMultipleSelectionDialog::selectedOptions() const
{
  return mWidget->selectedOptions();
}
