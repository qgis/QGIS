/***************************************************************************
                             qgslayoutguidewidget.cpp
                             ------------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutguidewidget.h"
#include "qgslayout.h"
#include "qgslayoutview.h"
#include "qgsdoublespinbox.h"
#include "qgslayoutunitscombobox.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutundostack.h"

QgsLayoutGuideWidget::QgsLayoutGuideWidget( QWidget *parent, QgsLayout *layout, QgsLayoutView *layoutView )
  : QgsPanelWidget( parent )
  , mLayout( layout )
{
  setupUi( this );
  setPanelTitle( tr( "Guides" ) );

  mHozProxyModel = new QgsLayoutGuideProxyModel( mHozGuidesTableView, Qt::Horizontal, 0 );
  mHozProxyModel->setSourceModel( &mLayout->guides() );
  mVertProxyModel = new QgsLayoutGuideProxyModel( mVertGuidesTableView, Qt::Vertical, 0 );
  mVertProxyModel->setSourceModel( &mLayout->guides() );

  mHozGuidesTableView->setModel( mHozProxyModel );
  mVertGuidesTableView->setModel( mVertProxyModel );

  mHozGuidesTableView->setEditTriggers( QAbstractItemView::AllEditTriggers );
  mVertGuidesTableView->setEditTriggers( QAbstractItemView::AllEditTriggers );

  mHozGuidesTableView->setItemDelegateForColumn( 0, new QgsLayoutGuidePositionDelegate( mHozGuidesTableView ) );
  mHozGuidesTableView->setItemDelegateForColumn( 1, new QgsLayoutGuideUnitDelegate( mHozGuidesTableView ) );

  mVertGuidesTableView->setItemDelegateForColumn( 0, new QgsLayoutGuidePositionDelegate( mVertGuidesTableView ) );
  mVertGuidesTableView->setItemDelegateForColumn( 1, new QgsLayoutGuideUnitDelegate( mVertGuidesTableView ) );

  connect( mAddHozGuideButton, &QPushButton::clicked, this, &QgsLayoutGuideWidget::addHorizontalGuide );
  connect( mAddVertGuideButton, &QPushButton::clicked, this, &QgsLayoutGuideWidget::addVerticalGuide );

  connect( mDeleteHozGuideButton, &QPushButton::clicked, this, &QgsLayoutGuideWidget::deleteHorizontalGuide );
  connect( mDeleteVertGuideButton, &QPushButton::clicked, this, &QgsLayoutGuideWidget::deleteVerticalGuide );

  connect( mClearAllButton, &QPushButton::clicked, this, &QgsLayoutGuideWidget::clearAll );
  connect( mApplyToAllButton, &QPushButton::clicked, this, &QgsLayoutGuideWidget::applyToAll );

  connect( mLayout->pageCollection(), &QgsLayoutPageCollection::changed, this, &QgsLayoutGuideWidget::updatePageCount );
  updatePageCount();
  connect( mPageNumberComboBox, qgis::overload< int >::of( &QComboBox::currentIndexChanged ), this, [ = ]( int )
  {
    setCurrentPage( mPageNumberComboBox->currentData().toInt() );
  } );

  connect( layoutView, &QgsLayoutView::pageChanged, this, &QgsLayoutGuideWidget::setCurrentPage );
  setCurrentPage( 0 );
}

void QgsLayoutGuideWidget::addHorizontalGuide()
{
  std::unique_ptr< QgsLayoutGuide > newGuide( new QgsLayoutGuide( Qt::Horizontal, QgsLayoutMeasurement( 0 ), mLayout->pageCollection()->page( mPage ) ) );
  mLayout->guides().addGuide( newGuide.release() );
}

void QgsLayoutGuideWidget::addVerticalGuide()
{
  std::unique_ptr< QgsLayoutGuide > newGuide( new QgsLayoutGuide( Qt::Vertical, QgsLayoutMeasurement( 0 ), mLayout->pageCollection()->page( mPage ) ) );
  mLayout->guides().addGuide( newGuide.release() );
}

void QgsLayoutGuideWidget::deleteHorizontalGuide()
{
  mLayout->undoStack()->beginMacro( tr( "Remove Horizontal Guides" ) );
  const auto constSelectedIndexes = mHozGuidesTableView->selectionModel()->selectedIndexes();
  for ( const QModelIndex &index : constSelectedIndexes )
  {
    mHozGuidesTableView->closePersistentEditor( index );
    if ( index.column() == 0 )
      mHozProxyModel->removeRow( index.row() );
  }
  mLayout->undoStack()->endMacro();
}

void QgsLayoutGuideWidget::deleteVerticalGuide()
{
  mLayout->undoStack()->beginMacro( tr( "Remove Vertical Guides" ) );
  const auto constSelectedIndexes = mVertGuidesTableView->selectionModel()->selectedIndexes();
  for ( const QModelIndex &index : constSelectedIndexes )
  {
    mVertGuidesTableView->closePersistentEditor( index );
    if ( index.column() == 0 )
      mVertProxyModel->removeRow( index.row() );
  }
  mLayout->undoStack()->endMacro();
}

void QgsLayoutGuideWidget::setCurrentPage( int page )
{
  mPage = page;

  // have to close any open editors - or we'll get a crash

  // qt - y u no do this for me?
  const auto horizontalSelectedIndexes = mHozGuidesTableView->selectionModel()->selectedIndexes();
  for ( const QModelIndex &index : horizontalSelectedIndexes )
  {
    mHozGuidesTableView->closePersistentEditor( index );
  }
  const auto verticalSelectedIndexes = mVertGuidesTableView->selectionModel()->selectedIndexes();
  for ( const QModelIndex &index : verticalSelectedIndexes )
  {
    mVertGuidesTableView->closePersistentEditor( index );
  }

  mHozProxyModel->setPage( page );
  mVertProxyModel->setPage( page );

  whileBlocking( mPageNumberComboBox )->setCurrentIndex( page );
}

void QgsLayoutGuideWidget::clearAll()
{
  // qt - y u no do this for me?
  const auto horizontalSelectedIndexes = mHozGuidesTableView->selectionModel()->selectedIndexes();
  for ( const QModelIndex &index : horizontalSelectedIndexes )
  {
    mHozGuidesTableView->closePersistentEditor( index );
  }
  const auto verticalSelectedIndexes = mVertGuidesTableView->selectionModel()->selectedIndexes();
  for ( const QModelIndex &index : verticalSelectedIndexes )
  {
    mVertGuidesTableView->closePersistentEditor( index );
  }

  mLayout->undoStack()->beginMacro( tr( "Remove All Guides" ) );
  mVertProxyModel->removeRows( 0, mVertProxyModel->rowCount() );
  mHozProxyModel->removeRows( 0, mHozProxyModel->rowCount() );
  mLayout->undoStack()->endMacro();
}

void QgsLayoutGuideWidget::applyToAll()
{
  mLayout->guides().applyGuidesToAllOtherPages( mPage );
}

void QgsLayoutGuideWidget::updatePageCount()
{
  const int prevPage = mPageNumberComboBox->currentIndex();
  mPageNumberComboBox->clear();
  for ( int i = 0; i < mLayout->pageCollection()->pageCount(); ++ i )
    mPageNumberComboBox->addItem( QString::number( i + 1 ), i );

  if ( mPageNumberComboBox->count() > prevPage )
    mPageNumberComboBox->setCurrentIndex( prevPage );
}


QgsLayoutGuidePositionDelegate::QgsLayoutGuidePositionDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{

}

QWidget *QgsLayoutGuidePositionDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & ) const
{
  QgsDoubleSpinBox *spin = new QgsDoubleSpinBox( parent );
  spin->setMinimum( 0 );
  spin->setMaximum( 1000000 );
  spin->setDecimals( 2 );
  spin->setShowClearButton( false );
  connect( spin, static_cast < void ( QgsDoubleSpinBox::* )( double ) > ( &QgsDoubleSpinBox::valueChanged ), this, [ = ]( double )
  {
    // we want to update on every spin change, not just the final
    const_cast< QgsLayoutGuidePositionDelegate * >( this )->emit commitData( spin );
  } );
  return spin;
}

void QgsLayoutGuidePositionDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QgsDoubleSpinBox *spin = qobject_cast< QgsDoubleSpinBox * >( editor );
  model->setData( index, spin->value(), QgsLayoutGuideCollection::PositionRole );
}

QgsLayoutGuideUnitDelegate::QgsLayoutGuideUnitDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{
}

QWidget *QgsLayoutGuideUnitDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &, const QModelIndex & ) const
{
  QgsLayoutUnitsComboBox *unitsCb = new QgsLayoutUnitsComboBox( parent );
  connect( unitsCb, &QgsLayoutUnitsComboBox::changed, this, [ = ]( int )
  {
    // we want to update on every unit change, not just the final
    const_cast< QgsLayoutGuideUnitDelegate * >( this )->emit commitData( unitsCb );
  } );
  return unitsCb;
}

void QgsLayoutGuideUnitDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QgsLayoutUnitsComboBox *cb = qobject_cast< QgsLayoutUnitsComboBox *>( editor );
  model->setData( index, cb->unit(), QgsLayoutGuideCollection::UnitsRole );
}

