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
  setupUi( this );
  init();
}

void QgsRasterLayerTemporalPropertiesWidget::init()
{
  QLocale locale;
  mStartTemporalDateTimeEdit->setDisplayFormat(
    locale.dateTimeFormat( QLocale::ShortFormat ) );

  mEndTemporalDateTimeEdit->setDisplayFormat(
    locale.dateTimeFormat( QLocale::ShortFormat ) );

  if ( mLayer && mLayer->temporalProperties() )
    mTemporalGroupBox->setChecked( mLayer->temporalProperties()->isActive() );
}

void QgsRasterLayerTemporalPropertiesWidget::saveTemporalProperties()
{
  mLayer->temporalProperties()->setIsActive( mTemporalGroupBox->isChecked() );

  QgsDateTimeRange normalRange = QgsDateTimeRange( mStartTemporalDateTimeEdit->dateTime(),
                                 mEndTemporalDateTimeEdit->dateTime() );
  mLayer->temporalProperties()->setTemporalRange( normalRange );
  mLayer->temporalProperties()->setTemporalSource( QgsMapLayerTemporalProperties::TemporalSource::Layer );

}
