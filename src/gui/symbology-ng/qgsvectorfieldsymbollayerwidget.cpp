/***************************************************************************
    qgsvectorfieldsymbollayerwidget.cpp
    ---------------------
    begin                : October 2011
    copyright            : (C) 2011 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsvectorfieldsymbollayerwidget.h"
#include "qgssymbolv2propertiesdialog.h"
#include "qgsvectorfieldsymbollayer.h"
#include "qgsvectorlayer.h"

QgsVectorFieldSymbolLayerWidget::QgsVectorFieldSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent ): QgsSymbolLayerV2Widget( parent, vl ), mLayer( 0 )
{
  setupUi( this );
  if ( mVectorLayer )
  {
    const QgsFieldMap& fm = mVectorLayer->pendingFields();
    QgsFieldMap::const_iterator fieldIt = fm.constBegin();
    mXAttributeComboBox->addItem( "" );
    mYAttributeComboBox->addItem( "" );
    for ( ; fieldIt != fm.constEnd(); ++fieldIt )
    {
      QString fieldName = fieldIt.value().name();
      mXAttributeComboBox->addItem( fieldName );
      mYAttributeComboBox->addItem( fieldName );
    }
  }
}

QgsVectorFieldSymbolLayerWidget::~QgsVectorFieldSymbolLayerWidget()
{
}

void QgsVectorFieldSymbolLayerWidget::setSymbolLayer( QgsSymbolLayerV2* layer )
{
  if ( layer->layerType() != "VectorField" )
  {
    return;
  }
  mLayer = static_cast<QgsVectorFieldSymbolLayer*>( layer );
  if ( !mLayer )
  {
    return;
  }

  mXAttributeComboBox->setCurrentIndex( mXAttributeComboBox->findText( mLayer->xAttribute() ) );
  mYAttributeComboBox->setCurrentIndex( mYAttributeComboBox->findText( mLayer->yAttribute() ) );
  mScaleSpinBox->setValue( mLayer->scale() );

  QgsVectorFieldSymbolLayer::VectorFieldType type = mLayer->vectorFieldType();
  if ( type == QgsVectorFieldSymbolLayer::Cartesian )
  {
    mCartesianRadioButton->setChecked( true );
  }
  else if ( type == QgsVectorFieldSymbolLayer::Polar )
  {
    mPolarRadioButton->setChecked( true );
  }
  else if ( type == QgsVectorFieldSymbolLayer::Height )
  {
    mHeightRadioButton->setChecked( true );
  }

  QgsVectorFieldSymbolLayer::AngleOrientation orientation = mLayer->angleOrientation();
  if ( orientation == QgsVectorFieldSymbolLayer::ClockwiseFromNorth )
  {
    mClockwiseFromNorthRadioButton->setChecked( true );
  }
  else if ( orientation == QgsVectorFieldSymbolLayer::CounterclockwiseFromEast )
  {
    mCounterclockwiseFromEastRadioButton->setChecked( true );
  }

  QgsVectorFieldSymbolLayer::AngleUnits  angleUnits = mLayer->angleUnits();
  if ( angleUnits == QgsVectorFieldSymbolLayer::Degrees )
  {
    mDegreesRadioButton->setChecked( true );
  }
  else if ( angleUnits == QgsVectorFieldSymbolLayer::Radians )
  {
    mRadiansRadioButton->setChecked( true );
  }
  updateMarkerIcon();
}

QgsSymbolLayerV2* QgsVectorFieldSymbolLayerWidget::symbolLayer()
{
  return mLayer;
}

void QgsVectorFieldSymbolLayerWidget::on_mScaleSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setScale( d );
    emit changed();
  }
}

void QgsVectorFieldSymbolLayerWidget::on_mXAttributeComboBox_currentIndexChanged( int index )
{
  if ( mLayer )
  {
    mLayer->setXAttribute( mXAttributeComboBox->itemText( index ) );
    emit changed();
  }
}

void QgsVectorFieldSymbolLayerWidget::on_mYAttributeComboBox_currentIndexChanged( int index )
{
  if ( mLayer )
  {
    mLayer->setYAttribute( mYAttributeComboBox->itemText( index ) );
    emit changed();
  }
}

void QgsVectorFieldSymbolLayerWidget::on_mLineStylePushButton_clicked()
{
  if ( !mLayer )
  {
    return;
  }

  QgsSymbolV2PropertiesDialog dlg( mLayer->subSymbol(), mVectorLayer, this );
  if ( dlg.exec() == QDialog::Rejected )
  {
    return;
  }

  updateMarkerIcon();
  emit changed();
}

void QgsVectorFieldSymbolLayerWidget::updateMarkerIcon()
{
  if ( mLayer )
  {
    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mLayer->subSymbol(), mLineStylePushButton->iconSize() );
    mLineStylePushButton->setIcon( icon );
  }
}

void QgsVectorFieldSymbolLayerWidget::on_mCartesianRadioButton_toggled( bool checked )
{
  if ( mLayer && checked )
  {
    mLayer->setVectorFieldType( QgsVectorFieldSymbolLayer::Cartesian );
    mXAttributeComboBox->setEnabled( true );
    mYAttributeComboBox->setEnabled( true );
    mXAttributeLabel->setText( tr( "X attribute" ) );
    mYAttributeLabel->setText( tr( "Y attribute" ) );
    emit changed();
  }
}

void QgsVectorFieldSymbolLayerWidget::on_mPolarRadioButton_toggled( bool checked )
{
  if ( mLayer && checked )
  {
    mLayer->setVectorFieldType( QgsVectorFieldSymbolLayer::Polar );
    mXAttributeComboBox->setEnabled( true );
    mYAttributeComboBox->setEnabled( true );
    mXAttributeLabel->setText( tr( "Length attribute" ) );
    mYAttributeLabel->setText( tr( "Angle attribute" ) );
    emit changed();
  }
}

void QgsVectorFieldSymbolLayerWidget::on_mHeightRadioButton_toggled( bool checked )
{
  if ( mLayer && checked )
  {
    mLayer->setVectorFieldType( QgsVectorFieldSymbolLayer::Height );
    mXAttributeLabel->setText( "" );
    mXAttributeComboBox->setEnabled( false );
    mYAttributeLabel->setText( tr( "Height attribute" ) );
    emit changed();
  }
}

void QgsVectorFieldSymbolLayerWidget::on_mDegreesRadioButton_toggled( bool checked )
{
  if ( mLayer && checked )
  {
    mLayer->setAngleUnits( QgsVectorFieldSymbolLayer::Degrees );
    emit changed();
  }
}

void QgsVectorFieldSymbolLayerWidget::on_mRadiansRadioButton_toggled( bool checked )
{
  if ( mLayer && checked )
  {
    mLayer->setAngleUnits( QgsVectorFieldSymbolLayer::Radians );
    emit changed();
  }
}

void QgsVectorFieldSymbolLayerWidget::on_mClockwiseFromNorthRadioButton_toggled( bool checked )
{
  if ( mLayer && checked )
  {
    mLayer->setAngleOrientation( QgsVectorFieldSymbolLayer::ClockwiseFromNorth );
    emit changed();
  }
}

void QgsVectorFieldSymbolLayerWidget::on_mCounterclockwiseFromEastRadioButton_toggled( bool checked )
{
  if ( mLayer && checked )
  {
    mLayer->setAngleOrientation( QgsVectorFieldSymbolLayer::CounterclockwiseFromEast );
    emit changed();
  }
}
