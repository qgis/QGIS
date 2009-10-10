/***************************************************************************
                         qgscomposerlegendwidget.cpp
                         ---------------------------
    begin                : July 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerlegendwidget.h"
#include "qgscomposerlegend.h"
#include "qgscomposerlegenditemdialog.h"
#include "qgscomposeritemwidget.h"
#include <QFontDialog>

QgsComposerLegendWidget::QgsComposerLegendWidget( QgsComposerLegend* legend ): mLegend( legend )
{
  setupUi( this );

  //add widget for item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, legend );
  gridLayout->addWidget( itemPropertiesWidget, 2, 0, 1, 1 );

  if ( legend )
  {
    mItemTreeView->setModel( legend->model() );
  }

  setGuiElements();
}

QgsComposerLegendWidget::QgsComposerLegendWidget(): mLegend( 0 )
{
  setupUi( this );
}

QgsComposerLegendWidget::~QgsComposerLegendWidget()
{

}

void QgsComposerLegendWidget::setGuiElements()
{
  if ( !mLegend )
  {
    return;
  }

  blockSignals( true );
  mTitleLineEdit->setText( mLegend->title() );
  mSymbolWidthSpinBox->setValue( mLegend->symbolWidth() );
  mSymbolHeightSpinBox->setValue( mLegend->symbolHeight() );
  mLayerSpaceSpinBox->setValue( mLegend->layerSpace() );
  mSymbolSpaceSpinBox->setValue( mLegend->symbolSpace() );
  mIconLabelSpaceSpinBox->setValue( mLegend->iconLabelSpace() );
  mBoxSpaceSpinBox->setValue( mLegend->boxSpace() );

  blockSignals( false );
}


void QgsComposerLegendWidget::on_mTitleLineEdit_textChanged( const QString& text )
{
  if ( mLegend )
  {
    mLegend->setTitle( text );
    mLegend->adjustBoxSize();
    mLegend->update();
  }
}

void QgsComposerLegendWidget::on_mSymbolWidthSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->setSymbolWidth( d );
    mLegend->adjustBoxSize();
    mLegend->update();
  }
}

void QgsComposerLegendWidget::on_mSymbolHeightSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->setSymbolHeight( d );
    mLegend->adjustBoxSize();
    mLegend->update();
  }
}

void QgsComposerLegendWidget::on_mLayerSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->setLayerSpace( d );
    mLegend->adjustBoxSize();
    mLegend->update();
  }
}

void QgsComposerLegendWidget::on_mSymbolSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->setSymbolSpace( d );
    mLegend->adjustBoxSize();
    mLegend->update();
  }
}

void QgsComposerLegendWidget::on_mIconLabelSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->setIconLabelSpace( d );
    mLegend->adjustBoxSize();
    mLegend->update();
  }
}

void QgsComposerLegendWidget::on_mTitleFontButton_clicked()
{
  if ( mLegend )
  {
    bool ok;
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && !defined(__LP64__)
    // Native Mac dialog works only for 64 bit Cocoa (observed in Qt 4.5.2, probably a Qt bug)
    QFont newFont = QFontDialog::getFont( &ok, mLegend->titleFont(), this, QString(), QFontDialog::DontUseNativeDialog );
#else
    QFont newFont = QFontDialog::getFont( &ok, mLegend->titleFont() );
#endif
    if ( ok )
    {
      mLegend->setTitleFont( newFont );
      mLegend->adjustBoxSize();
      mLegend->update();
    }
  }
}

void QgsComposerLegendWidget::on_mLayerFontButton_clicked()
{
  if ( mLegend )
  {
    bool ok;
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && !defined(__LP64__)
    // Native Mac dialog works only for 64 bit Cocoa (observed in Qt 4.5.2, probably a Qt bug)
    QFont newFont = QFontDialog::getFont( &ok, mLegend->layerFont(), this, QString(), QFontDialog::DontUseNativeDialog );
#else
    QFont newFont = QFontDialog::getFont( &ok, mLegend->layerFont() );
#endif
    if ( ok )
    {
      mLegend->setLayerFont( newFont );
      mLegend->adjustBoxSize();
      mLegend->update();
    }
  }
}

void QgsComposerLegendWidget::on_mItemFontButton_clicked()
{
  if ( mLegend )
  {
    bool ok;
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && !defined(__LP64__)
    // Native Mac dialog works only for 64 bit Cocoa (observed in Qt 4.5.2, probably a Qt bug)
    QFont newFont = QFontDialog::getFont( &ok, mLegend->itemFont(), this, QString(), QFontDialog::DontUseNativeDialog );
#else
    QFont newFont = QFontDialog::getFont( &ok, mLegend->itemFont() );
#endif
    if ( ok )
    {
      mLegend->setItemFont( newFont );
      mLegend->adjustBoxSize();
      mLegend->update();
    }
  }
}


void QgsComposerLegendWidget::on_mBoxSpaceSpinBox_valueChanged( double d )
{
  if ( mLegend )
  {
    mLegend->setBoxSpace( d );
    mLegend->adjustBoxSize();
    mLegend->update();
  }
}

void QgsComposerLegendWidget::on_mMoveDownPushButton_clicked()
{
  QStandardItemModel* itemModel = qobject_cast<QStandardItemModel *>( mItemTreeView->model() );
  if ( !itemModel )
  {
    return;
  }

  QModelIndex currentIndex = mItemTreeView->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  //is there an older sibling?
  int row = currentIndex.row();
  QModelIndex youngerSibling = currentIndex.sibling( row + 1, 0 );

  if ( !youngerSibling.isValid() )
  {
    return;
  }

  QModelIndex parentIndex = currentIndex.parent();
  QList<QStandardItem*> itemToMove;
  QList<QStandardItem*> youngerSiblingItem;

  if ( !parentIndex.isValid() ) //move toplevel (layer) item
  {
    youngerSiblingItem = itemModel->takeRow( row + 1 );
    itemToMove = itemModel->takeRow( row );
    itemModel->insertRow( row, youngerSiblingItem );
    itemModel->insertRow( row + 1, itemToMove );
  }
  else //move child (classification) item
  {
    QStandardItem* parentItem = itemModel->itemFromIndex( parentIndex );
    youngerSiblingItem = parentItem->takeRow( row + 1 );
    itemToMove = parentItem->takeRow( row );
    parentItem->insertRow( row, youngerSiblingItem );
    parentItem->insertRow( row + 1, itemToMove );
  }

  mItemTreeView->setCurrentIndex( itemModel->indexFromItem( itemToMove.at( 0 ) ) );
  if ( mLegend )
  {
    mLegend->update();
  }
}

void QgsComposerLegendWidget::on_mMoveUpPushButton_clicked()
{
  QStandardItemModel* itemModel = qobject_cast<QStandardItemModel *>( mItemTreeView->model() );
  if ( !itemModel )
  {
    return;
  }

  QModelIndex currentIndex = mItemTreeView->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  //is there an older sibling?
  int row = currentIndex.row();
  QModelIndex olderSibling = currentIndex.sibling( row - 1, 0 );

  if ( !olderSibling.isValid() )
  {
    return;
  }

  QModelIndex parentIndex = currentIndex.parent();
  QList<QStandardItem*> itemToMove;
  QList<QStandardItem*> olderSiblingItem;

  if ( !parentIndex.isValid() ) //move toplevel item
  {
    itemToMove = itemModel->takeRow( row );
    olderSiblingItem = itemModel->takeRow( row - 1 );
    itemModel->insertRow( row - 1, itemToMove );
    itemModel->insertRow( row, olderSiblingItem );

  }
  else //move classification items
  {
    QStandardItem* parentItem = itemModel->itemFromIndex( parentIndex );
    itemToMove = parentItem->takeRow( row );
    olderSiblingItem = parentItem->takeRow( row - 1 );
    parentItem->insertRow( row - 1, itemToMove );
    parentItem->insertRow( row, olderSiblingItem );
  }

  mItemTreeView->setCurrentIndex( itemModel->indexFromItem( itemToMove.at( 0 ) ) );
  if ( mLegend )
  {
    mLegend->update();
  }
}

void QgsComposerLegendWidget::on_mRemovePushButton_clicked()
{
  QStandardItemModel* itemModel = qobject_cast<QStandardItemModel *>( mItemTreeView->model() );
  if ( !itemModel )
  {
    return;
  }

  QModelIndex currentIndex = mItemTreeView->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  QModelIndex parentIndex = currentIndex.parent();

  itemModel->removeRow( currentIndex.row(), parentIndex );
  if ( mLegend )
  {
    mLegend->adjustBoxSize();
    mLegend->update();
  }
}

void QgsComposerLegendWidget::on_mEditPushButton_clicked()
{
  QStandardItemModel* itemModel = qobject_cast<QStandardItemModel *>( mItemTreeView->model() );
  if ( !itemModel )
  {
    return;
  }

  //get current item
  QModelIndex currentIndex = mItemTreeView->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  QStandardItem* currentItem = itemModel->itemFromIndex( currentIndex );
  if ( !currentItem )
  {
    return;
  }

  QgsComposerLegendItemDialog itemDialog( currentItem );
  if ( itemDialog.exec() == QDialog::Accepted )
  {
    currentItem->setText( itemDialog.itemText() );
  }
  if ( mLegend )
  {
    mLegend->adjustBoxSize();
    mLegend->update();
  }
}

void QgsComposerLegendWidget::on_mUpdatePushButton_clicked()
{
  //get current item
  QStandardItemModel* itemModel = qobject_cast<QStandardItemModel *>( mItemTreeView->model() );
  if ( !itemModel )
  {
    return;
  }

  //get current item
  QModelIndex currentIndex = mItemTreeView->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  QStandardItem* currentItem = itemModel->itemFromIndex( currentIndex );
  if ( !currentItem )
  {
    return;
  }

  if ( mLegend->model() )
  {
    mLegend->model()->updateItem( currentItem );
  }
  mLegend->update();
  mLegend->adjustBoxSize();
}

void QgsComposerLegendWidget::on_mUpdateAllPushButton_clicked()
{
  if ( mLegend )
  {
    mLegend->updateLegend();
  }
}
