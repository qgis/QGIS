/***************************************************************************
    qgs3dadvancedlinesymbolsettingsdialog.cpp
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

#include "qgs3dadvancedlinesymbolsettingswidget.h"

#include "qgs3dutils.h"
#include "qgsvectorlayer.h"

#include <QDialogButtonBox>
#include <QQuaternion>
#include <QString>

#include "moc_qgs3dadvancedlinesymbolsettingswidget.cpp"

using namespace Qt::StringLiterals;

Qgs3DAdvancedLineSymbolSettingsWidget::Qgs3DAdvancedLineSymbolSettingsWidget( const QgsAbstract3DSymbol *symbol, QgsVectorLayer *vLayer, Qgis::MaterialRenderingTechnique renderingTechnique, QWidget *parent )
  : Qgs3DAdvancedSymbolSettingsWidget( symbol, parent )
{
  setupUi( this );

  // material widget
  mWidgetMaterial->setSettings( symbol->materialSettings(), vLayer );
  mWidgetMaterial->setDockMode( dockMode() );
  mWidgetMaterial->setTechnique( renderingTechnique );
  mWidgetMaterial->setStyle( QgsMaterialSettingsWidget::WidgetStyle::Full );
  mWidgetMaterial->setFilterByTechnique( true );

  connect( mWidgetMaterial, &QgsMaterialWidget::changed, this, &Qgs3DAdvancedLineSymbolSettingsWidget::widgetChanged );
}

std::unique_ptr<QgsAbstract3DSymbol> Qgs3DAdvancedLineSymbolSettingsWidget::symbol() const
{
  std::unique_ptr<QgsAbstract3DSymbol> symbol( mBaseSymbol->clone() );
  symbol->setMaterialSettings( mWidgetMaterial->settings().release() );
  return symbol;
}
