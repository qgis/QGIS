/***************************************************************************
                         qgsvectorlayertemporalpropertieswidget.cpp
                         ------------------------------
    begin                : May 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayertemporalpropertieswidget.h"
#include "qgsgui.h"
#include "qgsproject.h"
#include "qgsprojecttimesettings.h"
#include "qgsvectordataprovidertemporalcapabilities.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayertemporalproperties.h"


QgsVectorLayerTemporalPropertiesWidget::QgsVectorLayerTemporalPropertiesWidget( QWidget *parent, QgsVectorLayer *layer )
  : QWidget( parent )
  , mLayer( layer )
{
  Q_ASSERT( mLayer );
  setupUi( this );

  mModeComboBox->addItem( tr( "Fixed Time Range" ), QgsVectorLayerTemporalProperties::ModeFixedTemporalRange );
  mModeComboBox->addItem( tr( "Single Field with Date/Time" ), QgsVectorLayerTemporalProperties::ModeFeatureDateTimeInstantFromField );
  mModeComboBox->addItem( tr( "Separate Fields for Start and End Date/Time" ), QgsVectorLayerTemporalProperties::ModeFeatureDateTimeStartAndEndFromFields );
  mModeComboBox->addItem( tr( "Redraw Layer Only" ), QgsVectorLayerTemporalProperties::ModeRedrawLayerOnly );

  const QgsVectorLayerTemporalProperties *properties = qobject_cast< QgsVectorLayerTemporalProperties * >( layer->temporalProperties() );
  mTemporalGroupBox->setChecked( properties->isActive() );

  mModeComboBox->setCurrentIndex( mModeComboBox->findData( properties->mode() ) );

  connect( mModeComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), mStackedWidget, &QStackedWidget::setCurrentIndex );

  mStackedWidget->setCurrentIndex( static_cast< int >( properties->mode() ) );

  mStartTemporalDateTimeEdit->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );
  mEndTemporalDateTimeEdit->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );

  mStartTemporalDateTimeEdit->setDateTime( properties->fixedTemporalRange().begin() );
  mEndTemporalDateTimeEdit->setDateTime( properties->fixedTemporalRange().end() );

  mSingleFieldComboBox->setLayer( layer );
  mStartFieldComboBox->setLayer( layer );
  mEndFieldComboBox->setLayer( layer );
  mSingleFieldComboBox->setFilters( QgsFieldProxyModel::DateTime | QgsFieldProxyModel::Date );
  mStartFieldComboBox->setFilters( QgsFieldProxyModel::DateTime | QgsFieldProxyModel::Date );
  mStartFieldComboBox->setAllowEmptyFieldName( true );
  mEndFieldComboBox->setFilters( QgsFieldProxyModel::DateTime | QgsFieldProxyModel::Date );
  mEndFieldComboBox->setAllowEmptyFieldName( true );

  if ( !properties->startField().isEmpty() )
  {
    mSingleFieldComboBox->setField( properties->startField() );
    mStartFieldComboBox->setField( properties->startField() );
  }
  if ( !properties->endField().isEmpty() )
  {
    mEndFieldComboBox->setField( properties->endField() );
  }
}

void QgsVectorLayerTemporalPropertiesWidget::saveTemporalProperties()
{
  QgsVectorLayerTemporalProperties *properties = qobject_cast< QgsVectorLayerTemporalProperties * >( mLayer->temporalProperties() );

  properties->setIsActive( mTemporalGroupBox->isChecked() );
  properties->setMode( static_cast< QgsVectorLayerTemporalProperties::TemporalMode >( mModeComboBox->currentData().toInt() ) );

  QgsDateTimeRange normalRange = QgsDateTimeRange( mStartTemporalDateTimeEdit->dateTime(),
                                 mEndTemporalDateTimeEdit->dateTime() );

  properties->setFixedTemporalRange( normalRange );

  switch ( properties->mode() )
  {
    case QgsVectorLayerTemporalProperties::ModeFeatureDateTimeInstantFromField:
    case QgsVectorLayerTemporalProperties::ModeFixedTemporalRange:
    case QgsVectorLayerTemporalProperties::ModeRedrawLayerOnly:
      properties->setStartField( mSingleFieldComboBox->currentField() );
      break;

    case QgsVectorLayerTemporalProperties::ModeFeatureDateTimeStartAndEndFromFields:
      properties->setStartField( mStartFieldComboBox->currentField() );
      break;
  }

  properties->setEndField( mEndFieldComboBox->currentField() );
}
