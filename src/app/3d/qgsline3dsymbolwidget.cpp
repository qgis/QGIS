/***************************************************************************
  qgsline3dsymbolwidget.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsline3dsymbolwidget.h"

#include "qgsline3dsymbol.h"
#include "qgsphongmaterialsettings.h"

QgsLine3DSymbolWidget::QgsLine3DSymbolWidget( QWidget *parent )
  : Qgs3DSymbolWidget( parent )
{
  setupUi( this );

  spinHeight->setClearValue( 0.0 );
  spinWidth->setClearValue( 0.0, tr( "Hairline" ) );
  spinExtrusion->setClearValue( 0.0 );

  cboAltClamping->addItem( tr( "Absolute" ), static_cast< int >( Qgis::AltitudeClamping::Absolute ) );
  cboAltClamping->addItem( tr( "Relative" ), static_cast< int >( Qgis::AltitudeClamping::Relative ) );
  cboAltClamping->addItem( tr( "Terrain" ), static_cast< int >( Qgis::AltitudeClamping::Terrain ) );

  QgsLine3DSymbol defaultLine;
  setSymbol( &defaultLine, nullptr );

  connect( spinWidth, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLine3DSymbolWidget::changed );
  connect( spinHeight, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLine3DSymbolWidget::changed );
  connect( spinExtrusion, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLine3DSymbolWidget::changed );
  connect( cboAltClamping, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLine3DSymbolWidget::changed );
  connect( cboAltBinding, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLine3DSymbolWidget::changed );
  connect( chkSimpleLines, &QCheckBox::toggled, this, &QgsLine3DSymbolWidget::changed );
  connect( chkSimpleLines, &QCheckBox::toggled, this, &QgsLine3DSymbolWidget::updateGuiState );
  connect( chkSimpleLines, &QCheckBox::toggled, this, &QgsLine3DSymbolWidget::simple3DLinesToggled );
  connect( widgetMaterial, &QgsMaterialWidget::changed, this, &QgsLine3DSymbolWidget::changed );

  widgetMaterial->setTechnique( QgsMaterialSettingsRenderingTechnique::Triangles );
}

Qgs3DSymbolWidget *QgsLine3DSymbolWidget::create( QgsVectorLayer * )
{
  return new QgsLine3DSymbolWidget();
}

void QgsLine3DSymbolWidget::setSymbol( const QgsAbstract3DSymbol *symbol, QgsVectorLayer *layer )
{
  const QgsLine3DSymbol *lineSymbol = dynamic_cast< const QgsLine3DSymbol *>( symbol );
  if ( !lineSymbol )
    return;

  spinWidth->setValue( lineSymbol->width() );
  spinHeight->setValue( lineSymbol->height() );
  spinExtrusion->setValue( lineSymbol->extrusionHeight() );
  cboAltClamping->setCurrentIndex( cboAltClamping->findData( static_cast< int >( lineSymbol->altitudeClamping() ) ) );
  cboAltBinding->setCurrentIndex( static_cast<int>( lineSymbol->altitudeBinding() ) );
  chkSimpleLines->setChecked( lineSymbol->renderAsSimpleLines() );
  widgetMaterial->setSettings( lineSymbol->materialSettings(), layer );
  widgetMaterial->setTechnique( chkSimpleLines->isChecked() ? QgsMaterialSettingsRenderingTechnique::Lines
                                : QgsMaterialSettingsRenderingTechnique::Triangles );
  updateGuiState();
}

QgsAbstract3DSymbol *QgsLine3DSymbolWidget::symbol()
{
  std::unique_ptr< QgsLine3DSymbol > sym = std::make_unique< QgsLine3DSymbol >();
  sym->setWidth( spinWidth->value() );
  sym->setHeight( spinHeight->value() );
  sym->setExtrusionHeight( spinExtrusion->value() );
  sym->setAltitudeClamping( static_cast<Qgis::AltitudeClamping>( cboAltClamping->currentData().toInt() ) );
  sym->setAltitudeBinding( static_cast<Qgis::AltitudeBinding>( cboAltBinding->currentIndex() ) );
  sym->setRenderAsSimpleLines( chkSimpleLines->isChecked() );
  sym->setMaterialSettings( widgetMaterial->settings() );
  return sym.release();
}

QString QgsLine3DSymbolWidget::symbolType() const
{
  return QStringLiteral( "line" );
}

void QgsLine3DSymbolWidget::updateGuiState()
{
  const bool simple = chkSimpleLines->isChecked();
  spinExtrusion->setEnabled( !simple );
  widgetMaterial->setTechnique( chkSimpleLines->isChecked() ? QgsMaterialSettingsRenderingTechnique::Lines
                                : QgsMaterialSettingsRenderingTechnique::Triangles );
}

void QgsLine3DSymbolWidget::simple3DLinesToggled( bool active )
{
  if ( active )
  {
    //remove "terrain" option for altitude clamping
    int terrainIndex = cboAltClamping->findData( static_cast< int >( Qgis::AltitudeClamping::Terrain ) );
    if ( terrainIndex >= 0 )
    {
      cboAltClamping->removeItem( terrainIndex );
    }
    if ( cboAltClamping->currentIndex() == -1 )
    {
      cboAltClamping->setCurrentIndex( cboAltClamping->findData( static_cast< int >( Qgis::AltitudeClamping::Relative ) ) );
    }
  }
  else
  {
    // make sure "terrain" option is available
    int terrainIndex = cboAltClamping->findData( static_cast< int >( Qgis::AltitudeClamping::Terrain ) );
    if ( terrainIndex == -1 )
    {
      cboAltClamping->addItem( tr( "Terrain" ), static_cast< int >( Qgis::AltitudeClamping::Terrain ) );
    }
  }
}
