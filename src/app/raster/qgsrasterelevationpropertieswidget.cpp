/***************************************************************************
    qgsrasterelevationpropertieswidget.cpp
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

#include "qgsrasterelevationpropertieswidget.h"
#include "qgsstyle.h"
#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerelevationproperties.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"

QgsRasterElevationPropertiesWidget::QgsRasterElevationPropertiesWidget( QgsRasterLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  setupUi( this );
  setObjectName( QStringLiteral( "mOptsPage_Elevation" ) );

  mOffsetZSpinBox->setClearValue( 0 );
  mScaleZSpinBox->setClearValue( 1 );
  mElevationGroupBox->setChecked( false );
  mLineStyleButton->setSymbolType( Qgis::SymbolType::Line );
  mFillStyleButton->setSymbolType( Qgis::SymbolType::Fill );
  mStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconSurfaceElevationLine.svg" ) ), tr( "Line" ), static_cast< int >( Qgis::ProfileSurfaceSymbology::Line ) );
  mStyleComboBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "mIconSurfaceElevationFillBelow.svg" ) ), tr( "Fill Below" ), static_cast< int >( Qgis::ProfileSurfaceSymbology::FillBelow ) );

  syncToLayer( layer );

  connect( mOffsetZSpinBox, qOverload<double >( &QDoubleSpinBox::valueChanged ), this, &QgsRasterElevationPropertiesWidget::onChanged );
  connect( mScaleZSpinBox, qOverload<double >( &QDoubleSpinBox::valueChanged ), this, &QgsRasterElevationPropertiesWidget::onChanged );
  connect( mElevationGroupBox, &QGroupBox::toggled, this, &QgsRasterElevationPropertiesWidget::onChanged );
  connect( mLineStyleButton, &QgsSymbolButton::changed, this, &QgsRasterElevationPropertiesWidget::onChanged );
  connect( mFillStyleButton, &QgsSymbolButton::changed, this, &QgsRasterElevationPropertiesWidget::onChanged );
  connect( mBandComboBox, &QgsRasterBandComboBox::bandChanged, this, &QgsRasterElevationPropertiesWidget::onChanged );
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
}

void QgsRasterElevationPropertiesWidget::syncToLayer( QgsMapLayer *layer )
{
  mLayer = qobject_cast< QgsRasterLayer * >( layer );
  if ( !mLayer )
    return;

  mBlockUpdates = true;
  const QgsRasterLayerElevationProperties *props = qgis::down_cast< const QgsRasterLayerElevationProperties * >( mLayer->elevationProperties() );
  mElevationGroupBox->setChecked( props->isEnabled() );
  mOffsetZSpinBox->setValue( props->zOffset() );
  mScaleZSpinBox->setValue( props->zScale() );
  mLineStyleButton->setSymbol( props->profileLineSymbol()->clone() );
  mFillStyleButton->setSymbol( props->profileFillSymbol()->clone() );
  mBandComboBox->setLayer( mLayer );
  mBandComboBox->setBand( props->bandNumber() );
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

void QgsRasterElevationPropertiesWidget::apply()
{
  if ( !mLayer )
    return;

  QgsRasterLayerElevationProperties *props = qgis::down_cast< QgsRasterLayerElevationProperties * >( mLayer->elevationProperties() );
  props->setEnabled( mElevationGroupBox->isChecked() );
  props->setZOffset( mOffsetZSpinBox->value() );
  props->setZScale( mScaleZSpinBox->value() );
  props->setProfileLineSymbol( mLineStyleButton->clonedSymbol< QgsLineSymbol >() );
  props->setProfileFillSymbol( mFillStyleButton->clonedSymbol< QgsFillSymbol >() );
  props->setProfileSymbology( static_cast< Qgis::ProfileSurfaceSymbology >( mStyleComboBox->currentData().toInt() ) );
  props->setBandNumber( mBandComboBox->currentBand() );
  mLayer->trigger3DUpdate();
}

void QgsRasterElevationPropertiesWidget::onChanged()
{
  if ( !mBlockUpdates )
    emit widgetChanged();
}


//
// QgsRasterElevationPropertiesWidgetFactory
//

QgsRasterElevationPropertiesWidgetFactory::QgsRasterElevationPropertiesWidgetFactory( QObject *parent )
  : QObject( parent )
{
  setIcon( QgsApplication::getThemeIcon( QStringLiteral( "propertyicons/elevationscale.svg" ) ) );
  setTitle( tr( "Elevation" ) );
}

QgsMapLayerConfigWidget *QgsRasterElevationPropertiesWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool, QWidget *parent ) const
{
  return new QgsRasterElevationPropertiesWidget( qobject_cast< QgsRasterLayer * >( layer ), canvas, parent );
}

bool QgsRasterElevationPropertiesWidgetFactory::supportLayerPropertiesDialog() const
{
  return true;
}

bool QgsRasterElevationPropertiesWidgetFactory::supportsStyleDock() const
{
  return false;
}

bool QgsRasterElevationPropertiesWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer->type() == QgsMapLayerType::RasterLayer;
}

QString QgsRasterElevationPropertiesWidgetFactory::layerPropertiesPagePositionHint() const
{
  return QStringLiteral( "mOptsPage_Metadata" );
}

