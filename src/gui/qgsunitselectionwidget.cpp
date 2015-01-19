/***************************************************************************
    qgsunitselectionwidget.h
    -------------------
    begin                : Mar 24, 2014
    copyright            : (C) 2014 Sandro Mani
    email                : smani@sourcepole.ch

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsunitselectionwidget.h"

QgsMapUnitScaleDialog::QgsMapUnitScaleDialog( QWidget* parent )
    : QDialog( parent )
{
  setupUi( this );
  mComboBoxMinScale->setScale( 0.0000001 );
  mComboBoxMaxScale->setScale( 1 );
  connect( mCheckBoxMinScale, SIGNAL( toggled( bool ) ), this, SLOT( configureMinComboBox() ) );
  connect( mCheckBoxMaxScale, SIGNAL( toggled( bool ) ), this, SLOT( configureMaxComboBox() ) );
  connect( mComboBoxMinScale, SIGNAL( scaleChanged() ), this, SLOT( configureMaxComboBox() ) );
  connect( mComboBoxMaxScale, SIGNAL( scaleChanged() ), this, SLOT( configureMinComboBox() ) );
}

void QgsMapUnitScaleDialog::setMapUnitScale( const QgsMapUnitScale &scale )
{
  mComboBoxMinScale->setScale( scale.minScale > 0.0 ? scale.minScale : 0.0000001 );
  mCheckBoxMinScale->setChecked( scale.minScale > 0.0 );
  mComboBoxMinScale->setEnabled( scale.minScale > 0.0 );
  mComboBoxMaxScale->setScale( scale.maxScale > 0.0 ? scale.maxScale : 1.0 );
  mCheckBoxMaxScale->setChecked( scale.maxScale > 0.0 );
  mComboBoxMaxScale->setEnabled( scale.maxScale > 0.0 );
}

void QgsMapUnitScaleDialog::configureMinComboBox()
{
  mComboBoxMinScale->setEnabled( mCheckBoxMinScale->isChecked() );
  if ( mCheckBoxMinScale->isChecked() && mComboBoxMinScale->scale() > mComboBoxMaxScale->scale() )
  {
    mComboBoxMinScale->setScale( mComboBoxMaxScale->scale() );
  }
}

void QgsMapUnitScaleDialog::configureMaxComboBox()
{
  mComboBoxMaxScale->setEnabled( mCheckBoxMaxScale->isChecked() );
  if ( mCheckBoxMaxScale->isChecked() && mComboBoxMaxScale->scale() < mComboBoxMinScale->scale() )
  {
    mComboBoxMaxScale->setScale( mComboBoxMinScale->scale() );
  }
}

QgsMapUnitScale QgsMapUnitScaleDialog::getMapUnitScale() const
{
  QgsMapUnitScale scale;
  scale.minScale = mCheckBoxMinScale->isChecked() ? mComboBoxMinScale->scale() : 0;
  scale.maxScale = mCheckBoxMaxScale->isChecked() ? mComboBoxMaxScale->scale() : 0;
  return scale;
}


QgsUnitSelectionWidget::QgsUnitSelectionWidget( QWidget *parent )
    : QWidget( parent )
{
  mMapUnitIdx = -1;
  mUnitScaleDialog = new QgsMapUnitScaleDialog( this );

  setupUi( this );
  mMapScaleButton->setVisible( false );
  mMapScaleButton->setToolTip( tr( "Adjust scaling range" ) );

  setFocusPolicy( Qt::StrongFocus );
  setFocusProxy( mUnitCombo );

  connect( mUnitCombo, SIGNAL( currentIndexChanged( int ) ), this, SLOT( toggleUnitRangeButton() ) );
  connect( mMapScaleButton, SIGNAL( clicked() ), this, SLOT( showDialog() ) );
  connect( mUnitCombo, SIGNAL( currentIndexChanged( int ) ), this, SIGNAL( changed() ) );
}

void QgsUnitSelectionWidget::setUnits( const QStringList &units, int mapUnitIdx )
{
  blockSignals( true );
  mUnitCombo->addItems( units );
  mMapUnitIdx = mapUnitIdx;
  blockSignals( false );
}

void QgsUnitSelectionWidget::setUnit( int unitIndex )
{
  blockSignals( true );
  mUnitCombo->setCurrentIndex( unitIndex );
  blockSignals( false );
}

void QgsUnitSelectionWidget::showDialog()
{
  QgsMapUnitScale scale = mUnitScaleDialog->getMapUnitScale();
  if ( mUnitScaleDialog->exec() != QDialog::Accepted )
  {
    mUnitScaleDialog->setMapUnitScale( scale );
  }
  else
  {
    QgsMapUnitScale newScale = mUnitScaleDialog->getMapUnitScale();
    if ( scale.minScale != newScale.minScale || scale.maxScale != newScale.maxScale )
    {
      emit changed();
    }
  }
}

void QgsUnitSelectionWidget::toggleUnitRangeButton()
{
  mMapScaleButton->setVisible( mMapUnitIdx != -1 && mUnitCombo->currentIndex() == mMapUnitIdx );
}



