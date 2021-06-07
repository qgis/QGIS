/***************************************************************************
    qgsmeshrenderermeshsettingswidget.cpp
    ---------------------------------------
    begin                : June 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshrenderermeshsettingswidget.h"
#include <QtGlobal>

#include "qgis.h"
#include "qgsmapcanvas.h"
#include "qgsmeshlayer.h"
#include "qgsrasterlayer.h"
#include "raster/qgsrasterminmaxwidget.h"
#include "qgsrasterminmaxorigin.h"
#include "qgsmessagelog.h"
#include "qgscolorbutton.h"
#include "qgsdoublespinbox.h"

QgsMeshRendererMeshSettingsWidget::QgsMeshRendererMeshSettingsWidget( QWidget *parent )
  : QWidget( parent )

{
  setupUi( this );

  mLineUnitsComboBox->setUnits( QgsUnitTypes::RenderUnitList()
                                << QgsUnitTypes::RenderMillimeters
                                << QgsUnitTypes::RenderMetersInMapUnits
                                << QgsUnitTypes::RenderPixels
                                << QgsUnitTypes::RenderPoints );


  connect( mColorWidget, &QgsColorButton::colorChanged, this, &QgsMeshRendererMeshSettingsWidget::widgetChanged );
  connect( mLineWidthSpinBox, qgis::overload<double>::of( &QgsDoubleSpinBox::valueChanged ),
           this, &QgsMeshRendererMeshSettingsWidget::widgetChanged );
  connect( mLineUnitsComboBox, &QgsUnitSelectionWidget::changed,
           this, &QgsMeshRendererMeshSettingsWidget::widgetChanged );
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

void QgsMeshRendererMeshSettingsWidget::syncToLayer( )
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
