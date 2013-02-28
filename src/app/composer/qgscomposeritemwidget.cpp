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
#include "qgspoint.h"
#include <QColorDialog>

QgsComposerItemWidget::QgsComposerItemWidget( QWidget* parent, QgsComposerItem* item ): QWidget( parent ), mItem( item )
{

  setupUi( this );

  //make button exclusive
  QButtonGroup* buttonGroup = new QButtonGroup( this );
  buttonGroup->addButton( mUpperLeftCheckBox );
  buttonGroup->addButton( mUpperMiddleCheckBox );
  buttonGroup->addButton( mUpperRightCheckBox );
  buttonGroup->addButton( mMiddleLeftCheckBox );
  buttonGroup->addButton( mMiddleCheckBox );
  buttonGroup->addButton( mMiddleRightCheckBox );
  buttonGroup->addButton( mLowerLeftCheckBox );
  buttonGroup->addButton( mLowerMiddleCheckBox );
  buttonGroup->addButton( mLowerRightCheckBox );
  buttonGroup->setExclusive( true );

  mXLineEdit->setValidator( new QDoubleValidator( 0 ) );
  mYLineEdit->setValidator( new QDoubleValidator( 0 ) );
  mWidthLineEdit->setValidator( new QDoubleValidator( 0 ) );
  mHeightLineEdit->setValidator( new QDoubleValidator( 0 ) );

  setValuesForGuiElements();
  connect( mItem, SIGNAL( sizeChanged() ), this, SLOT( setValuesForGuiPositionElements() ) );

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
  newBackgroundColor.setAlpha( mTransparencySlider->value() );
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

void QgsComposerItemWidget::on_mTransparencySpinBox_valueChanged( int value )
{
  if ( !mItem )
  {
    return;
  }

  mTransparencySlider->blockSignals( true );
  mTransparencySlider->setValue( value );
  mTransparencySlider->blockSignals( false );
  changeItemTransparency( value );
}

void QgsComposerItemWidget::on_mTransparencySlider_sliderReleased()
{
  if ( !mItem )
  {
    return;
  }
  int value = mTransparencySlider->value();
  mTransparencySpinBox->blockSignals( true );
  mTransparencySpinBox->setValue( value );
  mTransparencySpinBox->blockSignals( false );
  changeItemTransparency( value );
}

void QgsComposerItemWidget::changeItemTransparency( int value )
{
  mItem->beginCommand( tr( "Item Transparency changed" ) );
  QBrush itemBrush = mItem->brush();
  QColor brushColor = itemBrush.color();
  brushColor.setAlpha( 255 - value );
  mItem->setBrush( QBrush( brushColor ) );
  mItem->update();
  mItem->endCommand();
}

void QgsComposerItemWidget::changeItemPosition()
{
  mItem->beginCommand( tr( "Item position changed" ) );

  bool convXSuccess, convYSuccess;
  double x = mXLineEdit->text().toDouble( &convXSuccess );
  double y = mYLineEdit->text().toDouble( &convYSuccess );

  bool convSuccessWidth, convSuccessHeight;
  double width = mWidthLineEdit->text().toDouble( &convSuccessWidth );
  double height = mHeightLineEdit->text().toDouble( &convSuccessHeight );

  if ( !convXSuccess || !convYSuccess || !convSuccessWidth || !convSuccessHeight )
  {
    return;
  }

  mItem->setItemPosition( x, y, width, height, positionMode() );

  mItem->update();
  mItem->endCommand();
}

QgsComposerItem::ItemPositionMode QgsComposerItemWidget::positionMode() const
{
  if ( mUpperLeftCheckBox->checkState() == Qt::Checked )
  {
    return QgsComposerItem::UpperLeft;
  }
  else if ( mUpperMiddleCheckBox->checkState() == Qt::Checked )
  {
    return QgsComposerItem::UpperMiddle;
  }
  else if ( mUpperRightCheckBox->checkState() == Qt::Checked )
  {
    return QgsComposerItem::UpperRight;
  }
  else if ( mMiddleLeftCheckBox->checkState() == Qt::Checked )
  {
    return QgsComposerItem::MiddleLeft;
  }
  else if ( mMiddleCheckBox->checkState() == Qt::Checked )
  {
    return QgsComposerItem::Middle;
  }
  else if ( mMiddleRightCheckBox->checkState() == Qt::Checked )
  {
    return QgsComposerItem::MiddleRight;
  }
  else if ( mLowerLeftCheckBox->checkState() == Qt::Checked )
  {
    return QgsComposerItem::LowerLeft;
  }
  else if ( mLowerMiddleCheckBox->checkState() == Qt::Checked )
  {
    return QgsComposerItem::LowerMiddle;
  }
  else if ( mLowerRightCheckBox->checkState() == Qt::Checked )
  {
    return QgsComposerItem::LowerRight;
  }
  return QgsComposerItem::UpperLeft;
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

void QgsComposerItemWidget::on_mFrameGroupBox_toggled( bool state )
{
  if ( !mItem )
  {
    return;
  }

  mItem->beginCommand( tr( "Item frame toggled" ) );
  mItem->setFrameEnabled( state );
  mItem->update();
  mItem->endCommand();
}

void QgsComposerItemWidget::on_mBackgroundGroupBox_toggled( bool state )
{
  if ( !mItem )
  {
    return;
  }

  mItem->beginCommand( tr( "Item background toggled" ) );
  mItem->setBackgroundEnabled( state );
  mItem->update();
  mItem->endCommand();
}


void QgsComposerItemWidget::setValuesForGuiPositionElements()
{
  if ( !mItem )
  {
    return;
  }

  mXLineEdit->blockSignals( true );
  mYLineEdit->blockSignals( true );
  mWidthLineEdit->blockSignals( true );
  mHeightLineEdit->blockSignals( true );


  if ( mItem->lastUsedPositionMode() == QgsComposerItem::UpperLeft )
  {
    mUpperLeftCheckBox->setChecked( true );
    mXLineEdit->setText( QString::number( mItem->transform().dx() ) );
    mYLineEdit->setText( QString::number( mItem->transform().dy() ) );
  }

  if ( mItem->lastUsedPositionMode() == QgsComposerItem::UpperMiddle )
  {
    mUpperMiddleCheckBox->setChecked( true );
    mXLineEdit->setText( QString::number( mItem->transform().dx() + mItem->rect().width() / 2.0 ) );
    mYLineEdit->setText( QString::number( mItem->transform().dy() ) );
  }

  if ( mItem->lastUsedPositionMode() == QgsComposerItem::UpperRight )
  {
    mUpperRightCheckBox->setChecked( true );
    mXLineEdit->setText( QString::number( mItem->transform().dx() + mItem->rect().width() ) );
    mYLineEdit->setText( QString::number( mItem->transform().dy() ) );
  }

  if ( mItem->lastUsedPositionMode() == QgsComposerItem::MiddleLeft )
  {
    mMiddleLeftCheckBox->setChecked( true );
    mXLineEdit->setText( QString::number( mItem->transform().dx() ) );
    mYLineEdit->setText( QString::number( mItem->transform().dy() + mItem->rect().height() / 2.0 ) );
  }

  if ( mItem->lastUsedPositionMode() == QgsComposerItem::Middle )
  {
    mMiddleCheckBox->setChecked( true );
    mXLineEdit->setText( QString::number( mItem->transform().dx() + mItem->rect().width() / 2.0 ) );
    mYLineEdit->setText( QString::number( mItem->transform().dy() + mItem->rect().height() / 2.0 ) );
  }

  if ( mItem->lastUsedPositionMode() == QgsComposerItem::MiddleRight )
  {
    mMiddleRightCheckBox->setChecked( true );
    mXLineEdit->setText( QString::number( mItem->transform().dx() + mItem->rect().width() ) );
    mYLineEdit->setText( QString::number( mItem->transform().dy() + mItem->rect().height() / 2.0 ) );
  }

  if ( mItem->lastUsedPositionMode() == QgsComposerItem::LowerLeft )
  {
    mLowerLeftCheckBox->setChecked( true );
    mXLineEdit->setText( QString::number( mItem->transform().dx() ) );
    mYLineEdit->setText( QString::number( mItem->transform().dy() + mItem->rect().height() ) );
  }

  if ( mItem->lastUsedPositionMode() == QgsComposerItem::LowerMiddle )
  {
    mLowerMiddleCheckBox->setChecked( true );
    mXLineEdit->setText( QString::number( mItem->transform().dx() + mItem->rect().width() / 2.0 ) );
    mYLineEdit->setText( QString::number( mItem->transform().dy() + mItem->rect().height() ) );
  }

  if ( mItem->lastUsedPositionMode() == QgsComposerItem::LowerRight )
  {
    mLowerRightCheckBox->setChecked( true );
    mXLineEdit->setText( QString::number( mItem->transform().dx() + mItem->rect().width() ) );
    mYLineEdit->setText( QString::number( mItem->transform().dy() + mItem->rect().height() ) );
  }

  mWidthLineEdit->setText( QString::number( mItem->rect().width() ) );
  mHeightLineEdit->setText( QString::number( mItem->rect().height() ) );


  mXLineEdit->blockSignals( false );
  mYLineEdit->blockSignals( false );
  mWidthLineEdit->blockSignals( false );
  mHeightLineEdit->blockSignals( false );
}

void QgsComposerItemWidget::setValuesForGuiElements()
{
  if ( !mItem )
  {
    return;
  }

  setValuesForGuiPositionElements();

  mTransparencySlider->blockSignals( true );
  mOutlineWidthSpinBox->blockSignals( true );
  mFrameGroupBox->blockSignals( true );
  mBackgroundGroupBox->blockSignals( true );
  mItemIdLineEdit->blockSignals( true );
  mTransparencySpinBox->blockSignals( true );

  mTransparencySpinBox->setValue( 255 - mItem->brush().color().alpha() );
  mTransparencySlider->setValue( 255 - mItem->brush().color().alpha() );
  mOutlineWidthSpinBox->setValue( mItem->pen().widthF() );
  mItemIdLineEdit->setText( mItem->id() );
  mFrameGroupBox->setChecked( mItem->hasFrame() );
  mBackgroundGroupBox->setChecked( mItem->hasBackground() );

  mTransparencySlider->blockSignals( false );
  mOutlineWidthSpinBox->blockSignals( false );
  mFrameGroupBox->blockSignals( false );
  mBackgroundGroupBox->blockSignals( false );
  mItemIdLineEdit->blockSignals( false );
  mTransparencySpinBox->blockSignals( false );
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
