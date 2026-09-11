/***************************************************************************
    qgs3dadvancedsymbolsettingswidget.cpp
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

#include "qgs3dadvancedsymbolsettingswidget.h"

#include "qgs3dadvancedlinesymbolsettingswidget.h"
#include "qgs3dadvancedpointsymbolsettingswidget.h"
#include "qgs3dadvancedpolygonsymbolsettingswidget.h"
#include "qgs3dutils.h"
#include "qgsvectorlayer.h"

#include <QDialogButtonBox>
#include <QQuaternion>
#include <QString>
#include <QVBoxLayout>

#include "moc_qgs3dadvancedsymbolsettingswidget.cpp"

using namespace Qt::StringLiterals;

Qgs3DAdvancedSymbolSettingsWidget::Qgs3DAdvancedSymbolSettingsWidget( const QgsAbstract3DSymbol *symbol, QWidget *parent )
  : QgsPanelWidget( parent )
  , mBaseSymbol( symbol->clone() )
{}

std::unique_ptr<QgsAbstract3DSymbol> Qgs3DAdvancedSymbolSettingsWidget::symbol() const
{
  return nullptr;
}

//
// Qgs3DAdvancedSymbolSettingsDialog
//

Qgs3DAdvancedSymbolSettingsDialog::Qgs3DAdvancedSymbolSettingsDialog(
  const QgsAbstract3DSymbol *symbol, QgsVectorLayer *vLayer, Qgis::MaterialRenderingTechnique renderingTechnique, QWidget *parent, Qt::WindowFlags flags
)
  : QDialog( parent, flags )
{
  const Qgis::GeometryType geomType = vLayer->geometryType();
  if ( geomType == Qgis::GeometryType::Point )
  {
    mWidget = new Qgs3DAdvancedPointSymbolSettingsWidget( symbol, vLayer, renderingTechnique, this );
  }
  else if ( geomType == Qgis::GeometryType::Line )
  {
    mWidget = new Qgs3DAdvancedLineSymbolSettingsWidget( symbol, vLayer, renderingTechnique, this );
  }
  else if ( geomType == Qgis::GeometryType::Polygon )
  {
    mWidget = new Qgs3DAdvancedPolygonSymbolSettingsWidget( symbol, vLayer, renderingTechnique, this );
  }
  else
  {
    QgsDebugError( u"Unexpected geometry type for 3D symbol settings dialog, should not happen"_s );
    return;
  }

  QVBoxLayout *vLayout = new QVBoxLayout();
  vLayout->addWidget( mWidget );
  QDialogButtonBox *bbox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal );
  connect( bbox, &QDialogButtonBox::accepted, this, &Qgs3DAdvancedSymbolSettingsDialog::accept );
  connect( bbox, &QDialogButtonBox::rejected, this, &Qgs3DAdvancedSymbolSettingsDialog::reject );
  vLayout->addWidget( bbox );
  setLayout( vLayout );
  setWindowTitle( tr( "Advanced Symbol Settings" ) );
}

std::unique_ptr<QgsAbstract3DSymbol> Qgs3DAdvancedSymbolSettingsDialog::symbol() const
{
  return mWidget->symbol();
}
