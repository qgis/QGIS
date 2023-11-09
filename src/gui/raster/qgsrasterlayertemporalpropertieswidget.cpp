/***************************************************************************
                         qgsrasterlayertemporalpropertieswidget.cpp
                         ------------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterlayertemporalpropertieswidget.h"
#include "qgsgui.h"
#include "qgsproject.h"
#include "qgsprojecttimesettings.h"
#include "qgsrasterdataprovidertemporalcapabilities.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayertemporalproperties.h"
#include "qgsmaplayerconfigwidget.h"

QgsRasterLayerTemporalPropertiesWidget::QgsRasterLayerTemporalPropertiesWidget( QWidget *parent, QgsRasterLayer *layer )
  : QWidget( parent )
  , mLayer( layer )
{
  Q_ASSERT( mLayer );
  setupUi( this );

  mExtraWidgetLayout = new QVBoxLayout();
  mExtraWidgetLayout->setContentsMargins( 0, 0, 0, 0 );
  mExtraWidgetLayout->addStretch();
  mExtraWidgetContainer->setLayout( mExtraWidgetLayout );

  connect( mModeFixedRangeRadio, &QRadioButton::toggled, mFixedTimeRangeFrame, &QWidget::setEnabled );

  connect( mTemporalGroupBox, &QGroupBox::toggled, this, &QgsRasterLayerTemporalPropertiesWidget::temporalGroupBoxChecked );
  connect( mModeRedrawLayer, &QRadioButton::toggled, mLabelRedrawLayer, &QWidget::setEnabled );

  mStartTemporalDateTimeEdit->setDisplayFormat( QStringLiteral( "yyyy-MM-dd HH:mm:ss" ) );
  mEndTemporalDateTimeEdit->setDisplayFormat( QStringLiteral( "yyyy-MM-dd HH:mm:ss" ) );

  if ( !mLayer->dataProvider() || !mLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() )
  {
    mModeAutomaticRadio->setEnabled( false );
    mModeAutomaticRadio->setChecked( false );
    mModeFixedRangeRadio->setChecked( true );
  }

  syncToLayer();
}

void QgsRasterLayerTemporalPropertiesWidget::saveTemporalProperties()
{
  mLayer->temporalProperties()->setIsActive( mTemporalGroupBox->isChecked() );

  QgsRasterLayerTemporalProperties *temporalProperties = qobject_cast< QgsRasterLayerTemporalProperties * >( mLayer->temporalProperties() );

  QgsDateTimeRange normalRange = QgsDateTimeRange( mStartTemporalDateTimeEdit->dateTime(),
                                 mEndTemporalDateTimeEdit->dateTime() );

  if ( mModeAutomaticRadio->isChecked() )
    temporalProperties->setMode( Qgis::RasterTemporalMode::TemporalRangeFromDataProvider );
  else if ( mModeFixedRangeRadio->isChecked() )
    temporalProperties->setMode( Qgis::RasterTemporalMode::FixedTemporalRange );
  else if ( mModeRedrawLayer->isChecked() )
    temporalProperties->setMode( Qgis::RasterTemporalMode::RedrawLayerOnly );
  temporalProperties->setFixedTemporalRange( normalRange );

  for ( QgsMapLayerConfigWidget *widget : std::as_const( mExtraWidgets ) )
  {
    widget->apply();
  }
}

void QgsRasterLayerTemporalPropertiesWidget::syncToLayer()
{
  const QgsRasterLayerTemporalProperties *temporalProperties = qobject_cast< const QgsRasterLayerTemporalProperties * >( mLayer->temporalProperties() );
  switch ( temporalProperties->mode() )
  {
    case Qgis::RasterTemporalMode::TemporalRangeFromDataProvider:
      mModeAutomaticRadio->setChecked( true );
      break;
    case Qgis::RasterTemporalMode::FixedTemporalRange:
      mModeFixedRangeRadio->setChecked( true );
      break;
    case Qgis::RasterTemporalMode::RedrawLayerOnly:
      mModeRedrawLayer->setChecked( true );
      break;
  }

  mStartTemporalDateTimeEdit->setDateTime( temporalProperties->fixedTemporalRange().begin() );
  mEndTemporalDateTimeEdit->setDateTime( temporalProperties->fixedTemporalRange().end() );

  mTemporalGroupBox->setChecked( temporalProperties->isActive() );

  mLabelRedrawLayer->setEnabled( mModeRedrawLayer->isChecked() );

  for ( QgsMapLayerConfigWidget *widget : std::as_const( mExtraWidgets ) )
  {
    widget->syncToLayer( mLayer );
  }
}

void QgsRasterLayerTemporalPropertiesWidget::addWidget( QgsMapLayerConfigWidget *widget )
{
  mExtraWidgets << widget;
  mExtraWidgetLayout->insertWidget( mExtraWidgetLayout->count() - 1, widget );
}

void QgsRasterLayerTemporalPropertiesWidget::temporalGroupBoxChecked( bool checked )
{
  for ( QgsMapLayerConfigWidget *widget : std::as_const( mExtraWidgets ) )
  {
    widget->emit dynamicTemporalControlToggled( checked );
  }
}
