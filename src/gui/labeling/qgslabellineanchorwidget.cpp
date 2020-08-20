/***************************************************************************
    qgslabellineanchorwidget.cpp
    ----------------------
    begin                : August 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgslabellineanchorwidget.h"
#include "qgsexpressioncontextutils.h"
#include "qgsapplication.h"

QgsLabelLineAnchorWidget::QgsLabelLineAnchorWidget( QWidget *parent, QgsVectorLayer *vl )
  : QgsLabelSettingsWidgetBase( parent, vl )
{
  setupUi( this );

  setPanelTitle( tr( "Line Anchor Settings" ) );

  mPercentPlacementComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mActionLabelAnchorCenter.svg" ) ), tr( "Center of Line" ), 0.5 );
  mPercentPlacementComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mActionLabelAnchorStart.svg" ) ), tr( "Start of Line" ), 0.0 );
  mPercentPlacementComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mActionLabelAnchorEnd.svg" ) ), tr( "End of Line" ), 1.0 );
  mPercentPlacementComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mActionLabelAnchorCustom.svg" ) ), tr( "Custom…" ), -1.0 );

  connect( mPercentPlacementComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, [ = ]( int )
  {
    if ( !mBlockSignals )
      emit changed();

    mCustomPlacementSpinBox->setEnabled( mPercentPlacementComboBox->currentData().toDouble() < 0 );
  } );
  connect( mCustomPlacementSpinBox, qgis::overload<double>::of( &QDoubleSpinBox::valueChanged ), this, [ = ]( double )
  {
    if ( !mBlockSignals )
      emit changed();
  } );

  registerDataDefinedButton( mLinePlacementDDBtn, QgsPalLayerSettings::LineAnchorPercent );

}

void QgsLabelLineAnchorWidget::setSettings( const QgsLabelLineSettings &settings )
{
  mBlockSignals = true;
  const int comboIndex = mPercentPlacementComboBox->findData( settings.lineAnchorPercent() );
  if ( comboIndex >= 0 )
  {
    mPercentPlacementComboBox->setCurrentIndex( comboIndex );
  }
  else
  {
    // set custom control
    mPercentPlacementComboBox->setCurrentIndex( mPercentPlacementComboBox->findData( -1.0 ) );
    mCustomPlacementSpinBox->setValue( settings.lineAnchorPercent() * 100.0 );
  }
  mCustomPlacementSpinBox->setEnabled( mPercentPlacementComboBox->currentData().toDouble() < 0 );
  mBlockSignals = false;
}

QgsLabelLineSettings QgsLabelLineAnchorWidget::settings() const
{
  QgsLabelLineSettings settings;

  if ( mPercentPlacementComboBox->currentData().toDouble() >= 0 )
  {
    settings.setLineAnchorPercent( mPercentPlacementComboBox->currentData().toDouble() );
  }
  else
  {
    settings.setLineAnchorPercent( mCustomPlacementSpinBox->value() / 100.0 );
  }
  return settings;
}

void QgsLabelLineAnchorWidget::updateDataDefinedProperties( QgsPropertyCollection &properties )
{
  properties.setProperty( QgsPalLayerSettings::LineAnchorPercent, mDataDefinedProperties.property( QgsPalLayerSettings::LineAnchorPercent ) );
}
