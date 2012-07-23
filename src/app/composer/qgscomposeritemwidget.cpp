/***************************************************************************
                         qgscomposeritemwidget.cpp
                         -------------------------
    begin                : August 2008
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

#include "qgscomposeritemwidget.h"
#include "qgscomposeritem.h"
#include "qgscomposermap.h"
#include "qgsitempositiondialog.h"
#include "qgspoint.h"
#include <QColorDialog>

QgsComposerItemWidget::QgsComposerItemWidget( QWidget* parent, QgsComposerItem* item ): QWidget( parent ), mItem( item )
{
  setupUi( this );
  setValuesForGuiElements();
}

QgsComposerItemWidget::QgsComposerItemWidget(): QWidget( 0 ), mItem( 0 )
{

}

QgsComposerItemWidget::~QgsComposerItemWidget()
{

}

//slots
void QgsComposerItemWidget::on_mFrameColorButton_clicked()
{
  if ( !mItem )
  {
    return;
  }

  QColor newFrameColor = QColorDialog::getColor( mItem->pen().color(), 0 );
  if ( !newFrameColor.isValid() )
  {
    return; //dialog canceled
  }

  mItem->beginCommand( tr( "Frame color changed" ) );
  QPen thePen;
  thePen.setColor( newFrameColor );
  thePen.setWidthF( mOutlineWidthSpinBox->value() );

  mItem->setPen( thePen );
  mItem->update();
  mItem->endCommand();
}

void QgsComposerItemWidget::on_mBackgroundColorButton_clicked()
{
  if ( !mItem )
  {
    return;
  }

  QColor newBackgroundColor = QColorDialog::getColor( mItem->brush().color(), 0 );
  if ( !newBackgroundColor.isValid() )
  {
    return; //dialog canceled
  }

  mItem->beginCommand( tr( "Background color changed" ) );
  newBackgroundColor.setAlpha( mOpacitySlider->value() );
  mItem->setBrush( QBrush( QColor( newBackgroundColor ), Qt::SolidPattern ) );
  //if the item is a composer map, we need to regenerate the map image
  //because it usually is cached
  QgsComposerMap* cm = dynamic_cast<QgsComposerMap *>( mItem );
  if ( cm )
  {
    cm->cache();
  }
  mItem->update();
  mItem->endCommand();
}

void QgsComposerItemWidget::on_mOpacitySpinBox_valueChanged( int value )
{
  if ( !mItem )
  {
    return;
  }

  mOpacitySlider->blockSignals( true );
  mOpacitySlider->setValue( value );
  mOpacitySlider->blockSignals( false );
  changeItemOpacity( value );
}

void QgsComposerItemWidget::on_mOpacitySlider_sliderReleased()
{
  if ( !mItem )
  {
    return;
  }
  int value = mOpacitySlider->value();
  mOpacitySpinBox->blockSignals( true );
  mOpacitySpinBox->setValue( value );
  mOpacitySpinBox->blockSignals( false );
  changeItemOpacity( value );
}

void QgsComposerItemWidget::changeItemOpacity( int value )
{
  mItem->beginCommand( tr( "Item opacity changed" ) );
  QBrush itemBrush = mItem->brush();
  QColor brushColor = itemBrush.color();
  brushColor.setAlpha( value );
  mItem->setBrush( QBrush( brushColor ) );
  mItem->update();
  mItem->endCommand();
}

void QgsComposerItemWidget::on_mOutlineWidthSpinBox_valueChanged( double d )
{
  if ( !mItem )
  {
    return;
  }

  mItem->beginCommand( tr( "Item outline width" ), QgsComposerMergeCommand::ItemOutlineWidth );
  QPen itemPen = mItem->pen();
  itemPen.setWidthF( d );
  mItem->setPen( itemPen );
  mItem->endCommand();
}

void QgsComposerItemWidget::on_mFrameCheckBox_stateChanged( int state )
{
  if ( !mItem )
  {
    return;
  }

  mItem->beginCommand( tr( "Item frame toggled" ) );
  if ( state == Qt::Checked )
  {
    mItem->setFrameEnabled( true );
  }
  else
  {
    mItem->setFrameEnabled( false );
  }
  mItem->update();
  mItem->endCommand();
}

void QgsComposerItemWidget::setValuesForGuiElements()
{
  if ( !mItem )
  {
    return;
  }

  mOpacitySlider->blockSignals( true );
  mOutlineWidthSpinBox->blockSignals( true );
  mFrameCheckBox->blockSignals( true );
  mItemIdLineEdit->blockSignals( true );
  mOpacitySpinBox->blockSignals( true );

  mOpacitySpinBox->setValue( mItem->brush().color().alpha() );
  mOpacitySlider->setValue( mItem->brush().color().alpha() );
  mOutlineWidthSpinBox->setValue( mItem->pen().widthF() );
  mItemIdLineEdit->setText( mItem->id() );
  if ( mItem->hasFrame() )
  {
    mFrameCheckBox->setCheckState( Qt::Checked );
  }
  else
  {
    mFrameCheckBox->setCheckState( Qt::Unchecked );
  }

  mOpacitySlider->blockSignals( false );
  mOutlineWidthSpinBox->blockSignals( false );
  mFrameCheckBox->blockSignals( false );
  mItemIdLineEdit->blockSignals( false );
  mOpacitySpinBox->blockSignals( false );
}

void QgsComposerItemWidget::on_mPositionButton_clicked()
{
  if ( !mItem )
  {
    return;
  }

  mItem->beginCommand( tr( "Item position changed" ) );
  QgsItemPositionDialog d( mItem, 0 );
  if ( d.exec() == QDialog::Accepted )
  {
    mItem->endCommand();
  }
  else
  {
    mItem->cancelCommand();
  }
}

void QgsComposerItemWidget::on_mItemIdLineEdit_textChanged( const QString &text )
{
  if ( mItem )
  {
    mItem->beginCommand( tr( "Item id changed" ), QgsComposerMergeCommand::ComposerLabelSetId );
    mItem->setId( text );
    mItem->endCommand();
  }
}
