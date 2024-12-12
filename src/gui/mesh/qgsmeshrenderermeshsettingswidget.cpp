/***************************************************************************
    qgsmeshrenderermeshsettingswidget.cpp
    ---------------------------------------
    begin                : June 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshrenderermeshsettingswidget.h"
#include "moc_qgsmeshrenderermeshsettingswidget.cpp"
#include <QtGlobal>

#include "qgis.h"
#include "qgsmeshlayer.h"
#include "qgscolorbutton.h"
#include "qgsdoublespinbox.h"

QgsMeshRendererMeshSettingsWidget::QgsMeshRendererMeshSettingsWidget( QWidget *parent )
  : QWidget( parent )

{
  setupUi( this );

  mLineUnitsComboBox->setUnits(
    { Qgis::RenderUnit::Millimeters,
      Qgis::RenderUnit::MetersInMapUnits,
      Qgis::RenderUnit::Pixels,
      Qgis::RenderUnit::Points
    }
  );


  connect( mColorWidget, &QgsColorButton::colorChanged, this, &QgsMeshRendererMeshSettingsWidget::widgetChanged );
  connect( mLineWidthSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsMeshRendererMeshSettingsWidget::widgetChanged );
  connect( mLineUnitsComboBox, &QgsUnitSelectionWidget::changed, this, &QgsMeshRendererMeshSettingsWidget::widgetChanged );
}

void QgsMeshRendererMeshSettingsWidget::setLayer( QgsMeshLayer *layer, MeshType meshType )
{
  mMeshType = meshType;
  mMeshLayer = layer;
}

QgsMeshRendererMeshSettings QgsMeshRendererMeshSettingsWidget::settings() const
{
  QgsMeshRendererMeshSettings settings;
  settings.setColor( mColorWidget->color() );
  settings.setLineWidth( mLineWidthSpinBox->value() );
  settings.setLineWidthUnit( mLineUnitsComboBox->unit() );
  return settings;
}

void QgsMeshRendererMeshSettingsWidget::syncToLayer()
{
  if ( !mMeshLayer )
    return;

  const QgsMeshRendererSettings rendererSettings = mMeshLayer->rendererSettings();

  QgsMeshRendererMeshSettings settings;
  switch ( mMeshType )
  {
    case Native:
      settings = rendererSettings.nativeMeshSettings();
      break;
    case Triangular:
      settings = rendererSettings.triangularMeshSettings();
      break;
    case Edge:
      settings = rendererSettings.edgeMeshSettings();
      break;
  }
  mColorWidget->setColor( settings.color() );
  mLineWidthSpinBox->setValue( settings.lineWidth() );
  mLineUnitsComboBox->setUnit( settings.lineWidthUnit() );
}
