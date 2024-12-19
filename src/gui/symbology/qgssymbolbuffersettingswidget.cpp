/***************************************************************************
    qgssymbolbuffersettingswidget.h
    ---------------------
    begin                : July 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssymbolbuffersettingswidget.h"
#include "moc_qgssymbolbuffersettingswidget.cpp"
#include "qgssymbol.h"
#include "qgsfillsymbol.h"
#include "qgsfillsymbollayer.h"

#include <QDialogButtonBox>

QgsSymbolBufferSettingsWidget::QgsSymbolBufferSettingsWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );

  setPanelTitle( tr( "Buffer Settings" ) );

  mSpinWidth->setClearValue( 1 );
  mSpinWidth->setShowClearButton( true );
  mSpinWidth->setValue( 1 );

  mEnabledGroup->setChecked( false );

  mSizeUnitWidget->setUnits( { Qgis::RenderUnit::Millimeters, Qgis::RenderUnit::MetersInMapUnits, Qgis::RenderUnit::MapUnits, Qgis::RenderUnit::Pixels, Qgis::RenderUnit::Points, Qgis::RenderUnit::Inches } );
  mSizeUnitWidget->setUnit( Qgis::RenderUnit::Millimeters );

  mFillSymbolButton->setSymbolType( Qgis::SymbolType::Fill );
  mFillSymbolButton->setDialogTitle( tr( "Fill Symbol" ) );
  mFillSymbolButton->setDefaultSymbol( new QgsFillSymbol( QgsSymbolLayerList() << new QgsSimpleFillSymbolLayer( QColor( 255, 255, 255 ), Qt::SolidPattern, QColor( 200, 200, 200 ), Qt::NoPen ) ) );
  mFillSymbolButton->setToDefaultSymbol();

  mComboJoinStyle->setPenJoinStyle( Qt::RoundJoin );

  connect( mEnabledGroup, &QGroupBox::toggled, this, &QgsSymbolBufferSettingsWidget::onWidgetChanged );
  connect( mSpinWidth, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsSymbolBufferSettingsWidget::onWidgetChanged );
  connect( mComboJoinStyle, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsSymbolBufferSettingsWidget::onWidgetChanged );
  connect( mSizeUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsSymbolBufferSettingsWidget::onWidgetChanged );
  connect( mFillSymbolButton, &QgsSymbolButton::changed, this, &QgsSymbolBufferSettingsWidget::onWidgetChanged );
}

void QgsSymbolBufferSettingsWidget::setBufferSettings( const QgsSymbolBufferSettings &settings )
{
  mBlockUpdates = true;
  mEnabledGroup->setChecked( settings.enabled() );
  mSpinWidth->setValue( settings.size() );
  mSizeUnitWidget->setUnit( settings.sizeUnit() );
  mSizeUnitWidget->setMapUnitScale( settings.sizeMapUnitScale() );
  mComboJoinStyle->setPenJoinStyle( settings.joinStyle() );

  if ( const QgsFillSymbol *fill = settings.fillSymbol() )
    mFillSymbolButton->setSymbol( fill->clone() );
  mBlockUpdates = false;
}

QgsSymbolBufferSettings QgsSymbolBufferSettingsWidget::bufferSettings() const
{
  QgsSymbolBufferSettings settings;
  settings.setEnabled( mEnabledGroup->isChecked() );
  settings.setSize( mSpinWidth->value() );
  settings.setSizeUnit( mSizeUnitWidget->unit() );
  settings.setSizeMapUnitScale( mSizeUnitWidget->getMapUnitScale() );
  settings.setJoinStyle( mComboJoinStyle->penJoinStyle() );
  settings.setFillSymbol( mFillSymbolButton->clonedSymbol<QgsFillSymbol>() );
  return settings;
}

void QgsSymbolBufferSettingsWidget::onWidgetChanged()
{
  if ( !mBlockUpdates )
    emit widgetChanged();
}


//
// QgsSymbolBufferSettingsDialog
//

QgsSymbolBufferSettingsDialog::QgsSymbolBufferSettingsDialog( QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
{
  QVBoxLayout *vLayout = new QVBoxLayout();
  mWidget = new QgsSymbolBufferSettingsWidget();
  vLayout->addWidget( mWidget );
  QDialogButtonBox *bbox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal );
  connect( bbox, &QDialogButtonBox::accepted, this, &QgsSymbolBufferSettingsDialog::accept );
  connect( bbox, &QDialogButtonBox::rejected, this, &QgsSymbolBufferSettingsDialog::reject );
  vLayout->addWidget( bbox );
  setLayout( vLayout );
  setWindowTitle( tr( "Buffer Settings" ) );
}

void QgsSymbolBufferSettingsDialog::setBufferSettings( const QgsSymbolBufferSettings &settings )
{
  mWidget->setBufferSettings( settings );
}

QgsSymbolBufferSettings QgsSymbolBufferSettingsDialog::bufferSettings() const
{
  return mWidget->bufferSettings();
}
