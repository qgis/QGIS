/***************************************************************************
                         qgsitempositiondialog.cpp
                         -------------------------
    begin                : October 2008
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

#include "qgsitempositiondialog.h"
#include "qgspoint.h"
#include <QButtonGroup>
#include <QDoubleValidator>

QgsItemPositionDialog::QgsItemPositionDialog( QgsComposerItem* item, QWidget* parent ): QDialog( parent ), mItem( item )
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

  //set lower left position of item
  mUpperLeftCheckBox->setCheckState( Qt::Checked );

  //set initial width and height
  if ( mItem )
  {
    mWidthLineEdit->setText( QString::number( mItem->rect().width() ) );
    mHeightLineEdit->setText( QString::number( mItem->rect().height() ) );
  }
}

QgsItemPositionDialog::QgsItemPositionDialog(): mItem( 0 )
{
}

QgsItemPositionDialog::~QgsItemPositionDialog()
{

}

int QgsItemPositionDialog::position( QgsPoint& point ) const
{
  bool convXSuccess, convYSuccess;
  double x = mXLineEdit->text().toDouble( &convXSuccess );
  double y = mYLineEdit->text().toDouble( &convYSuccess );

  if ( !convXSuccess || !convYSuccess )
  {
    return 1;
  }

  point.setX( x );
  point.setY( y );
  return 0;
}

int QgsItemPositionDialog::size( QSizeF& s ) const
{
  bool convSuccessWidth, convSuccessHeight;
  double width = mWidthLineEdit->text().toDouble( &convSuccessWidth );
  double height = mHeightLineEdit->text().toDouble( &convSuccessHeight );

  if ( !convSuccessWidth || !convSuccessHeight )
  {
    return 1;
  }

  s.setWidth( width );
  s.setHeight( height );
  return 0;
}

QgsComposerItem::ItemPositionMode QgsItemPositionDialog::positionMode() const
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

void QgsItemPositionDialog::on_mCloseButton_clicked()
{
  accept();
}

void QgsItemPositionDialog::on_mSetPositionButton_clicked()
{
  if ( !mItem )
  {
    return;
  }

  QgsPoint itemPosition;
  QSizeF itemSize;

  if ( position( itemPosition ) == 0 )
  {
    if ( size( itemSize ) == 0 )
    {
      mItem->setItemPosition( itemPosition.x(), itemPosition.y(), itemSize.width(), itemSize.height(), positionMode() );
    }
    else
    {
      mItem->setItemPosition( itemPosition.x(), itemPosition.y(), positionMode() );
    }
    mItem->update();
  }
}

void QgsItemPositionDialog::on_mUpperLeftCheckBox_stateChanged( int state )
{
  if ( state == Qt::Checked && mItem )
  {
    mXLineEdit->setText( QString::number( mItem->transform().dx() ) );
    mYLineEdit->setText( QString::number( mItem->transform().dy() ) );
  }
}

void QgsItemPositionDialog::on_mUpperMiddleCheckBox_stateChanged( int state )
{

  if ( state == Qt::Checked && mItem )
  {
    mXLineEdit->setText( QString::number( mItem->transform().dx() + mItem->rect().width() / 2.0 ) );
    mYLineEdit->setText( QString::number( mItem->transform().dy() ) );
  }
}

void QgsItemPositionDialog::on_mUpperRightCheckBox_stateChanged( int state )
{
  if ( state == Qt::Checked && mItem )
  {
    mXLineEdit->setText( QString::number( mItem->transform().dx() + mItem->rect().width() ) );
    mYLineEdit->setText( QString::number( mItem->transform().dy() ) );
  }
}

void QgsItemPositionDialog::on_mMiddleLeftCheckBox_stateChanged( int state )
{
  if ( state == Qt::Checked && mItem )
  {
    mXLineEdit->setText( QString::number( mItem->transform().dx() ) );
    mYLineEdit->setText( QString::number( mItem->transform().dy() + mItem->rect().height() / 2.0 ) );
  }
}

void QgsItemPositionDialog::on_mMiddleCheckBox_stateChanged( int state )
{
  if ( state == Qt::Checked && mItem )
  {
    mXLineEdit->setText( QString::number( mItem->transform().dx() + mItem->rect().width() / 2.0 ) );
    mYLineEdit->setText( QString::number( mItem->transform().dy() + mItem->rect().height() / 2.0 ) );
  }
}

void QgsItemPositionDialog::on_mMiddleRightCheckBox_stateChanged( int state )
{
  if ( state == Qt::Checked && mItem )
  {
    mXLineEdit->setText( QString::number( mItem->transform().dx() + mItem->rect().width() ) );
    mYLineEdit->setText( QString::number( mItem->transform().dy() + mItem->rect().height() / 2.0 ) );
  }
}

void QgsItemPositionDialog::on_mLowerLeftCheckBox_stateChanged( int state )
{
  if ( state == Qt::Checked && mItem )
  {
    mXLineEdit->setText( QString::number( mItem->transform().dx() ) );
    mYLineEdit->setText( QString::number( mItem->transform().dy() + mItem->rect().height() ) );
  }
}

void QgsItemPositionDialog::on_mLowerMiddleCheckBox_stateChanged( int state )
{
  if ( state == Qt::Checked && mItem )
  {
    mXLineEdit->setText( QString::number( mItem->transform().dx() + mItem->rect().width() / 2.0 ) );
    mYLineEdit->setText( QString::number( mItem->transform().dy() + mItem->rect().height() ) );
  }
}

void QgsItemPositionDialog::on_mLowerRightCheckBox_stateChanged( int state )
{
  if ( state == Qt::Checked && mItem )
  {
    mXLineEdit->setText( QString::number( mItem->transform().dx() + mItem->rect().width() ) );
    mYLineEdit->setText( QString::number( mItem->transform().dy() + mItem->rect().height() ) );
  }
}
