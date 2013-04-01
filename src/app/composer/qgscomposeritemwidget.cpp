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
#include <QPen>

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
}

void QgsComposerItemWidget::on_mFrameColorButton_colorChanged( const QColor& newFrameColor )
{
  if ( !mItem )
  {
    return;
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
}

void QgsComposerItemWidget::on_mBackgroundColorButton_colorChanged( const QColor& newBackgroundColor )
{
  if ( !mItem )
  {
    return;
  }
//  QColor newColor( newBackgroundColor );
  mItem->beginCommand( tr( "Background color changed" ) );
//  newColor.setAlpha( 255 - ( mTransparencySpinBox->value() * 2.55 ) );
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

//void QgsComposerItemWidget::on_mTransparencySpinBox_valueChanged( int value )
//{
//  if ( !mItem )
//  {
//    return;
//  }

//  mTransparencySlider->blockSignals( true );
//  mTransparencySlider->setValue( value );
//  mTransparencySlider->blockSignals( false );
//  changeItemTransparency( value );
//}

//void QgsComposerItemWidget::on_mTransparencySlider_valueChanged( int value )
//{
//  if ( !mItem )
//  {
//    return;
//  }
//  // do item updates only off of mTransparencySpinBox valueChanged
//  mTransparencySpinBox->setValue( value );
//}

//void QgsComposerItemWidget::changeItemTransparency( int value )
//{
//  mItem->beginCommand( tr( "Item transparency changed" ) );
//  QBrush itemBrush = mItem->brush();
//  QColor brushColor = itemBrush.color();
//  brushColor.setAlpha( 255 - ( value * 2.55 ) );
//  mItem->setBrush( QBrush( brushColor ) );
//  mItem->update();
//  mItem->endCommand();
//}

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

//  mTransparencySlider->blockSignals( true );
  mOutlineWidthSpinBox->blockSignals( true );
  mFrameGroupBox->blockSignals( true );
  mBackgroundGroupBox->blockSignals( true );
  mItemIdLineEdit->blockSignals( true );
  mItemUuidLineEdit->blockSignals( true );
  mBlendModeCombo->blockSignals( true );
  mTransparencySlider->blockSignals( true );
//  mTransparencySpinBox->blockSignals( true );

  mBackgroundColorButton->setColor( mItem->brush().color() );
  mBackgroundColorButton->setColorDialogTitle( tr( "Select background color" ) );
  mBackgroundColorButton->setColorDialogOptions( QColorDialog::ShowAlphaChannel );
//  int alphaPercent = ( 255 - mItem->brush().color().alpha() ) / 2.55;
//  mTransparencySpinBox->setValue( alphaPercent );
//  mTransparencySlider->setValue( alphaPercent );

  mFrameColorButton->setColor( mItem->pen().color() );
  mFrameColorButton->setColorDialogTitle( tr( "Select frame color" ) );
  mFrameColorButton->setColorDialogOptions( QColorDialog::ShowAlphaChannel );
  mOutlineWidthSpinBox->setValue( mItem->pen().widthF() );
  mItemIdLineEdit->setText( mItem->id() );
  mItemUuidLineEdit->setText( mItem->uuid() );
  mFrameGroupBox->setChecked( mItem->hasFrame() );
  mBackgroundGroupBox->setChecked( mItem->hasBackground() );
  mBlendModeCombo->setBlendMode( mItem->blendMode() );
  mTransparencySlider->setValue( mItem->transparency() );

//  mTransparencySlider->blockSignals( false );
  mOutlineWidthSpinBox->blockSignals( false );
  mFrameGroupBox->blockSignals( false );
  mBackgroundGroupBox->blockSignals( false );
  mItemIdLineEdit->blockSignals( false );
  mItemUuidLineEdit->blockSignals( false );
  mBlendModeCombo->blockSignals( false );
  mTransparencySlider->blockSignals( false );
//  mTransparencySpinBox->blockSignals( false );
}

void QgsComposerItemWidget::on_mBlendModeCombo_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  if ( mItem )
  {
    mItem->setBlendMode(( QgsMapRenderer::BlendMode ) mBlendModeCombo->blendMode() );
  }
}

void QgsComposerItemWidget::on_mTransparencySlider_valueChanged( int value )
{
  if ( mItem )
  {
    mItem->setTransparency( value );
  }
}

void QgsComposerItemWidget::on_mItemIdLineEdit_editingFinished()
{
  if ( mItem )
  {
    mItem->beginCommand( tr( "Item id changed" ), QgsComposerMergeCommand::ComposerLabelSetId );
    mItem->setId( mItemIdLineEdit->text() );
    mItemIdLineEdit->setText( mItem->id() );
    mItem->endCommand();
  }
}
