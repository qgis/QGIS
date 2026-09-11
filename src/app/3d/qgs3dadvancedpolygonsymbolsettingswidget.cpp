/***************************************************************************
    qgs3dadvancedpolygonsymbolsettingswidget.cpp
    ---------------------
    begin                : July 2026
    copyright            : (C) 2026 by Jean Felder
    email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dadvancedpolygonsymbolsettingswidget.h"

#include "qgs3dutils.h"
#include "qgspolygon3dsymbol.h"
#include "qgsvectorlayer.h"

#include <QDialogButtonBox>
#include <QQuaternion>
#include <QString>

#include "moc_qgs3dadvancedpolygonsymbolsettingswidget.cpp"

using namespace Qt::StringLiterals;

Qgs3DAdvancedPolygonSymbolSettingsWidget::Qgs3DAdvancedPolygonSymbolSettingsWidget(
  const QgsAbstract3DSymbol *symbol, QgsVectorLayer *vLayer, Qgis::MaterialRenderingTechnique renderingTechnique, QWidget *parent
)
  : Qgs3DAdvancedSymbolSettingsWidget( symbol, parent )
{
  setupUi( this );

  const QgsPolygon3DSymbol *polygonSymbol = qgis::down_cast<const QgsPolygon3DSymbol *>( symbol );
  mGroupEdges->setChecked( polygonSymbol->edgesEnabled() );
  mSpinEdgeWidth->setValue( polygonSymbol->edgeWidth() );
  mSpinEdgeWidth->setClearValue( 1.0 );
  mBtnEdgeColor->setColor( polygonSymbol->edgeColor() );

  connect( mGroupEdges, &QGroupBox::toggled, this, &Qgs3DAdvancedPolygonSymbolSettingsWidget::widgetChanged );
  connect( mSpinEdgeWidth, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &Qgs3DAdvancedPolygonSymbolSettingsWidget::widgetChanged );
  connect( mBtnEdgeColor, &QgsColorButton::colorChanged, this, &Qgs3DAdvancedPolygonSymbolSettingsWidget::widgetChanged );

  // material widget
  mWidgetMaterial->setSettings( symbol->materialSettings(), vLayer );
  mWidgetMaterial->setDockMode( dockMode() );
  mWidgetMaterial->setTechnique( renderingTechnique );
  mWidgetMaterial->setStyle( QgsMaterialSettingsWidget::WidgetStyle::Full );
  mWidgetMaterial->setFilterByTechnique( true );

  connect( mWidgetMaterial, &QgsMaterialWidget::changed, this, &Qgs3DAdvancedPolygonSymbolSettingsWidget::widgetChanged );
}

std::unique_ptr<QgsAbstract3DSymbol> Qgs3DAdvancedPolygonSymbolSettingsWidget::symbol() const
{
  std::unique_ptr<QgsAbstract3DSymbol> symbol( mBaseSymbol->clone() );
  symbol->setMaterialSettings( mWidgetMaterial->settings().release() );

  QgsPolygon3DSymbol *polygonSymbol = qgis::down_cast< QgsPolygon3DSymbol *>( symbol.get() );
  polygonSymbol->setEdgesEnabled( mGroupEdges->isChecked() );
  polygonSymbol->setEdgeColor( mBtnEdgeColor->color() );
  polygonSymbol->setEdgeWidth( mSpinEdgeWidth->value() );

  return symbol;
}
