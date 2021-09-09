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
#include "qgsvectorfieldsymbollayer.h"
#include "qgsvectorlayer.h"

QgsVectorFieldSymbolLayerWidget::QgsVectorFieldSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent ): QgsSymbolLayerWidget( parent, vl )
{
  setupUi( this );
  connect( mScaleSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsVectorFieldSymbolLayerWidget::mScaleSpinBox_valueChanged );
  connect( mXAttributeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsVectorFieldSymbolLayerWidget::mXAttributeComboBox_currentIndexChanged );
  connect( mYAttributeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsVectorFieldSymbolLayerWidget::mYAttributeComboBox_currentIndexChanged );
  connect( mCartesianRadioButton, &QRadioButton::toggled, this, &QgsVectorFieldSymbolLayerWidget::mCartesianRadioButton_toggled );
  connect( mPolarRadioButton, &QRadioButton::toggled, this, &QgsVectorFieldSymbolLayerWidget::mPolarRadioButton_toggled );
  connect( mHeightRadioButton, &QRadioButton::toggled, this, &QgsVectorFieldSymbolLayerWidget::mHeightRadioButton_toggled );
  connect( mDegreesRadioButton, &QRadioButton::toggled, this, &QgsVectorFieldSymbolLayerWidget::mDegreesRadioButton_toggled );
  connect( mRadiansRadioButton, &QRadioButton::toggled, this, &QgsVectorFieldSymbolLayerWidget::mRadiansRadioButton_toggled );
  connect( mClockwiseFromNorthRadioButton, &QRadioButton::toggled, this, &QgsVectorFieldSymbolLayerWidget::mClockwiseFromNorthRadioButton_toggled );
  connect( mCounterclockwiseFromEastRadioButton, &QRadioButton::toggled, this, &QgsVectorFieldSymbolLayerWidget::mCounterclockwiseFromEastRadioButton_toggled );
  connect( mDistanceUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsVectorFieldSymbolLayerWidget::mDistanceUnitWidget_changed );

  mDistanceUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                 << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );

  if ( auto *lVectorLayer = vectorLayer() )
  {
    mXAttributeComboBox->addItem( QString() );
    mYAttributeComboBox->addItem( QString() );
    int i = 0;
    const QgsFields fields = lVectorLayer->fields();
    for ( const QgsField &f : fields )
    {
      const QString fieldName = f.name();
      mXAttributeComboBox->addItem( lVectorLayer->fields().iconForField( i ), fieldName );
      mYAttributeComboBox->addItem( lVectorLayer->fields().iconForField( i ), fieldName );
      i++;
    }
  }
}

void QgsVectorFieldSymbolLayerWidget::setSymbolLayer( QgsSymbolLayer *layer )
{
  if ( layer->layerType() != QLatin1String( "VectorField" ) )
  {
    return;
  }
  mLayer = static_cast<QgsVectorFieldSymbolLayer *>( layer );
  if ( !mLayer )
  {
    return;
  }

  mXAttributeComboBox->setCurrentIndex( mXAttributeComboBox->findText( mLayer->xAttribute() ) );
  mYAttributeComboBox->setCurrentIndex( mYAttributeComboBox->findText( mLayer->yAttribute() ) );
  mScaleSpinBox->setValue( mLayer->scale() );

  const QgsVectorFieldSymbolLayer::VectorFieldType type = mLayer->vectorFieldType();
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

  const QgsVectorFieldSymbolLayer::AngleOrientation orientation = mLayer->angleOrientation();
  if ( orientation == QgsVectorFieldSymbolLayer::ClockwiseFromNorth )
  {
    mClockwiseFromNorthRadioButton->setChecked( true );
  }
  else if ( orientation == QgsVectorFieldSymbolLayer::CounterclockwiseFromEast )
  {
    mCounterclockwiseFromEastRadioButton->setChecked( true );
  }

  const QgsVectorFieldSymbolLayer::AngleUnits  angleUnits = mLayer->angleUnits();
  if ( angleUnits == QgsVectorFieldSymbolLayer::Degrees )
  {
    mDegreesRadioButton->setChecked( true );
  }
  else if ( angleUnits == QgsVectorFieldSymbolLayer::Radians )
  {
    mRadiansRadioButton->setChecked( true );
  }

  mDistanceUnitWidget->blockSignals( true );
  mDistanceUnitWidget->setUnit( mLayer->distanceUnit() );
  mDistanceUnitWidget->setMapUnitScale( mLayer->distanceMapUnitScale() );
  mDistanceUnitWidget->blockSignals( false );

  emit changed();
}

QgsSymbolLayer *QgsVectorFieldSymbolLayerWidget::symbolLayer()
{
  return mLayer;
}

void QgsVectorFieldSymbolLayerWidget::mScaleSpinBox_valueChanged( double d )
{
  if ( mLayer )
  {
    mLayer->setScale( d );
    emit changed();
  }
}

void QgsVectorFieldSymbolLayerWidget::mXAttributeComboBox_currentIndexChanged( int index )
{
  if ( mLayer )
  {
    mLayer->setXAttribute( mXAttributeComboBox->itemText( index ) );
    emit changed();
  }
}

void QgsVectorFieldSymbolLayerWidget::mYAttributeComboBox_currentIndexChanged( int index )
{
  if ( mLayer )
  {
    mLayer->setYAttribute( mYAttributeComboBox->itemText( index ) );
    emit changed();
  }
}

void QgsVectorFieldSymbolLayerWidget::mCartesianRadioButton_toggled( bool checked )
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

void QgsVectorFieldSymbolLayerWidget::mPolarRadioButton_toggled( bool checked )
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

void QgsVectorFieldSymbolLayerWidget::mHeightRadioButton_toggled( bool checked )
{
  if ( mLayer && checked )
  {
    mLayer->setVectorFieldType( QgsVectorFieldSymbolLayer::Height );
    mXAttributeLabel->clear();
    mXAttributeComboBox->setEnabled( false );
    mYAttributeLabel->setText( tr( "Height attribute" ) );
    emit changed();
  }
}

void QgsVectorFieldSymbolLayerWidget::mDegreesRadioButton_toggled( bool checked )
{
  if ( mLayer && checked )
  {
    mLayer->setAngleUnits( QgsVectorFieldSymbolLayer::Degrees );
    emit changed();
  }
}

void QgsVectorFieldSymbolLayerWidget::mRadiansRadioButton_toggled( bool checked )
{
  if ( mLayer && checked )
  {
    mLayer->setAngleUnits( QgsVectorFieldSymbolLayer::Radians );
    emit changed();
  }
}

void QgsVectorFieldSymbolLayerWidget::mClockwiseFromNorthRadioButton_toggled( bool checked )
{
  if ( mLayer && checked )
  {
    mLayer->setAngleOrientation( QgsVectorFieldSymbolLayer::ClockwiseFromNorth );
    emit changed();
  }
}

void QgsVectorFieldSymbolLayerWidget::mCounterclockwiseFromEastRadioButton_toggled( bool checked )
{
  if ( mLayer && checked )
  {
    mLayer->setAngleOrientation( QgsVectorFieldSymbolLayer::CounterclockwiseFromEast );
    emit changed();
  }
}

void QgsVectorFieldSymbolLayerWidget::mDistanceUnitWidget_changed()
{
  if ( !mLayer )
  {
    return;
  }

  mLayer->setDistanceUnit( mDistanceUnitWidget->unit() );
  mLayer->setDistanceMapUnitScale( mDistanceUnitWidget->getMapUnitScale() );
  emit changed();
}



