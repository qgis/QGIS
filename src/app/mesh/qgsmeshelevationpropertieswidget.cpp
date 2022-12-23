/***************************************************************************
    qgsmeshelevationpropertieswidget.cpp
    ---------------------
    begin                : February 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshelevationpropertieswidget.h"
#include "qgsstyle.h"
#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgsmeshlayer.h"
#include "qgsmeshlayerelevationproperties.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"

QgsMeshElevationPropertiesWidget::QgsMeshElevationPropertiesWidget( QgsMeshLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setupUi( this );
  setObjectName( QStringLiteral( "mOptsPage_Elevation" ) );

  mOffsetZSpinBox->setClearValue( 0 );
  mScaleZSpinBox->setClearValue( 1 );
  mLineStyleButton->setSymbolType( Qgis::SymbolType::Line );
  mFillStyleButton->setSymbolType( Qgis::SymbolType::Fill );
  mStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconSurfaceElevationLine.svg" ) ), tr( "Line" ), static_cast< int >( Qgis::ProfileSurfaceSymbology::Line ) );
  mStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconSurfaceElevationFillBelow.svg" ) ), tr( "Fill Below" ), static_cast< int >( Qgis::ProfileSurfaceSymbology::FillBelow ) );

  syncToLayer( layer );

  connect( mOffsetZSpinBox, qOverload<double >( &QDoubleSpinBox::valueChanged ), this, &QgsMeshElevationPropertiesWidget::onChanged );
  connect( mScaleZSpinBox, qOverload<double >( &QDoubleSpinBox::valueChanged ), this, &QgsMeshElevationPropertiesWidget::onChanged );
  connect( mLineStyleButton, &QgsSymbolButton::changed, this, &QgsMeshElevationPropertiesWidget::onChanged );
  connect( mFillStyleButton, &QgsSymbolButton::changed, this, &QgsMeshElevationPropertiesWidget::onChanged );
  connect( mStyleComboBox, qOverload< int >( &QComboBox::currentIndexChanged ), this, [ = ]
  {
    switch ( static_cast< Qgis::ProfileSurfaceSymbology >( mStyleComboBox->currentData().toInt() ) )
    {
      case Qgis::ProfileSurfaceSymbology::Line:
        mSymbologyStackedWidget->setCurrentWidget( mPageLine );
        break;
      case Qgis::ProfileSurfaceSymbology::FillBelow:
        mSymbologyStackedWidget->setCurrentWidget( mPageFill );
        break;
    }

    onChanged();
  } );

  setProperty( "helpPage", QStringLiteral( "working_with_mesh/mesh_properties.html#elevation-properties" ) );
}

void QgsMeshElevationPropertiesWidget::syncToLayer( QgsMapLayer *layer )
{
  mLayer = qobject_cast< QgsMeshLayer * >( layer );
  if ( !mLayer )
    return;

  mBlockUpdates = true;
  const QgsMeshLayerElevationProperties *props = qgis::down_cast< const QgsMeshLayerElevationProperties * >( mLayer->elevationProperties() );
  mOffsetZSpinBox->setValue( props->zOffset() );
  mScaleZSpinBox->setValue( props->zScale() );
  mLineStyleButton->setSymbol( props->profileLineSymbol()->clone() );
  mFillStyleButton->setSymbol( props->profileFillSymbol()->clone() );

  mStyleComboBox->setCurrentIndex( mStyleComboBox->findData( static_cast <int >( props->profileSymbology() ) ) );
  switch ( props->profileSymbology() )
  {
    case Qgis::ProfileSurfaceSymbology::Line:
      mSymbologyStackedWidget->setCurrentWidget( mPageLine );
      break;
    case Qgis::ProfileSurfaceSymbology::FillBelow:
      mSymbologyStackedWidget->setCurrentWidget( mPageFill );
      break;
  }

  mBlockUpdates = false;
}

void QgsMeshElevationPropertiesWidget::apply()
{
  if ( !mLayer )
    return;

  QgsMeshLayerElevationProperties *props = qgis::down_cast< QgsMeshLayerElevationProperties * >( mLayer->elevationProperties() );
  props->setZOffset( mOffsetZSpinBox->value() );
  props->setZScale( mScaleZSpinBox->value() );
  props->setProfileLineSymbol( mLineStyleButton->clonedSymbol< QgsLineSymbol >() );
  props->setProfileFillSymbol( mFillStyleButton->clonedSymbol< QgsFillSymbol >() );
  props->setProfileSymbology( static_cast< Qgis::ProfileSurfaceSymbology >( mStyleComboBox->currentData().toInt() ) );
  mLayer->trigger3DUpdate();
}

void QgsMeshElevationPropertiesWidget::onChanged()
{
  if ( !mBlockUpdates )
    emit widgetChanged();
}


//
// QgsMeshElevationPropertiesWidgetFactory
//

QgsMeshElevationPropertiesWidgetFactory::QgsMeshElevationPropertiesWidgetFactory( QObject *parent )
  : QObject( parent )
{
  setIcon( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/elevationscale.svg" ) ) );
  setTitle( tr( "Elevation" ) );
}

QgsMapLayerConfigWidget *QgsMeshElevationPropertiesWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool, QWidget *parent ) const
{
  return new QgsMeshElevationPropertiesWidget( qobject_cast< QgsMeshLayer * >( layer ), canvas, parent );
}

bool QgsMeshElevationPropertiesWidgetFactory::supportLayerPropertiesDialog() const
{
  return true;
}

bool QgsMeshElevationPropertiesWidgetFactory::supportsStyleDock() const
{
  return false;
}

bool QgsMeshElevationPropertiesWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer->type() == QgsMapLayerType::MeshLayer;
}

QString QgsMeshElevationPropertiesWidgetFactory::layerPropertiesPagePositionHint() const
{
  return QStringLiteral( "mOptsPage_Metadata" );
}

