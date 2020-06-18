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

QgsRasterLayerTemporalPropertiesWidget::QgsRasterLayerTemporalPropertiesWidget( QWidget *parent, QgsRasterLayer *layer )
  : QWidget( parent )
  , mLayer( layer )
{
  Q_ASSERT( mLayer );
  setupUi( this );

  connect( mModeFixedRangeRadio, &QRadioButton::toggled, mFixedTimeRangeFrame, &QWidget::setEnabled );

  mStartTemporalDateTimeEdit->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );
  mEndTemporalDateTimeEdit->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );

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
    temporalProperties->setMode( QgsRasterLayerTemporalProperties::ModeTemporalRangeFromDataProvider );
  else if ( mModeFixedRangeRadio->isChecked() )
    temporalProperties->setMode( QgsRasterLayerTemporalProperties::ModeFixedTemporalRange );
  temporalProperties->setFixedTemporalRange( normalRange );
}

void QgsRasterLayerTemporalPropertiesWidget::syncToLayer()
{
  const QgsRasterLayerTemporalProperties *temporalProperties = qobject_cast< const QgsRasterLayerTemporalProperties * >( mLayer->temporalProperties() );
  switch ( temporalProperties->mode() )
  {
    case QgsRasterLayerTemporalProperties::ModeTemporalRangeFromDataProvider:
      mModeAutomaticRadio->setChecked( true );
      break;
    case QgsRasterLayerTemporalProperties::ModeFixedTemporalRange:
      mModeFixedRangeRadio->setChecked( true );
      break;
  }

  mStartTemporalDateTimeEdit->setDateTime( temporalProperties->fixedTemporalRange().begin() );
  mEndTemporalDateTimeEdit->setDateTime( temporalProperties->fixedTemporalRange().end() );

  mTemporalGroupBox->setChecked( temporalProperties->isActive() );
}
