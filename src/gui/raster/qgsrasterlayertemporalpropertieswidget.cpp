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


QgsRasterLayerTemporalPropertiesWidget::QgsRasterLayerTemporalPropertiesWidget( QWidget *parent, QgsRasterLayer *layer )
  : QWidget( parent )
  , mLayer( layer )
{
  Q_ASSERT( mLayer );
  setupUi( this );

  connect( mModeFixedRangeRadio, &QRadioButton::toggled, mFixedTimeRangeFrame, &QWidget::setEnabled );
  init();
}

void QgsRasterLayerTemporalPropertiesWidget::init()
{
  mStartTemporalDateTimeEdit->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );

  mEndTemporalDateTimeEdit->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );

  switch ( mLayer->temporalProperties()->mode() )
  {
    case QgsRasterLayerTemporalProperties::ModeTemporalRangeFromDataProvider:
      mModeAutomaticRadio->setChecked( true );
      break;
    case QgsRasterLayerTemporalProperties::ModeFixedTemporalRange:
      mModeFixedRangeRadio->setChecked( true );
      break;
  }

  mStartTemporalDateTimeEdit->setDateTime( mLayer->temporalProperties()->fixedTemporalRange().begin() );
  mEndTemporalDateTimeEdit->setDateTime( mLayer->temporalProperties()->fixedTemporalRange().end() );

  if ( !mLayer->dataProvider() || !mLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() )
  {
    mModeAutomaticRadio->setEnabled( false );
    mModeAutomaticRadio->setChecked( false );
    mModeFixedRangeRadio->setChecked( true );
  }

  mTemporalGroupBox->setChecked( mLayer->temporalProperties()->isActive() );
}

void QgsRasterLayerTemporalPropertiesWidget::saveTemporalProperties()
{
  mLayer->temporalProperties()->setIsActive( mTemporalGroupBox->isChecked() );

  QgsDateTimeRange normalRange = QgsDateTimeRange( mStartTemporalDateTimeEdit->dateTime(),
                                 mEndTemporalDateTimeEdit->dateTime() );

  if ( mModeAutomaticRadio->isChecked() )
    mLayer->temporalProperties()->setMode( QgsRasterLayerTemporalProperties::ModeTemporalRangeFromDataProvider );
  else if ( mModeFixedRangeRadio->isChecked() )
    mLayer->temporalProperties()->setMode( QgsRasterLayerTemporalProperties::ModeFixedTemporalRange );
  mLayer->temporalProperties()->setFixedTemporalRange( normalRange );
}
