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
#include "qgsstringutils.h"
#include "qgsexpressioncontextutils.h"

QgsVectorLayerTemporalPropertiesWidget::QgsVectorLayerTemporalPropertiesWidget( QWidget *parent, QgsVectorLayer *layer )
  : QWidget( parent )
  , mLayer( layer )
{
  Q_ASSERT( mLayer );
  setupUi( this );

  mModeComboBox->addItem( tr( "Fixed Time Range" ), QgsVectorLayerTemporalProperties::ModeFixedTemporalRange );
  mModeComboBox->addItem( tr( "Single Field with Date/Time" ), QgsVectorLayerTemporalProperties::ModeFeatureDateTimeInstantFromField );
  mModeComboBox->addItem( tr( "Separate Fields for Start and End Date/Time" ), QgsVectorLayerTemporalProperties::ModeFeatureDateTimeStartAndEndFromFields );
  mModeComboBox->addItem( tr( "Separate Fields for Start and Event Duration" ), QgsVectorLayerTemporalProperties::ModeFeatureDateTimeStartAndDurationFromFields );
  mModeComboBox->addItem( tr( "Start and End Date/Time from Expressions" ), QgsVectorLayerTemporalProperties::ModeFeatureDateTimeStartAndEndFromExpressions );
  mModeComboBox->addItem( tr( "Redraw Layer Only" ), QgsVectorLayerTemporalProperties::ModeRedrawLayerOnly );

  connect( mModeComboBox, qOverload<int>( &QComboBox::currentIndexChanged ), mStackedWidget, &QStackedWidget::setCurrentIndex );

  mStartTemporalDateTimeEdit->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );
  mEndTemporalDateTimeEdit->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );

  mSingleFieldComboBox->setLayer( layer );
  mStartFieldComboBox->setLayer( layer );
  mEndFieldComboBox->setLayer( layer );
  mDurationStartFieldComboBox->setLayer( layer );
  mDurationFieldComboBox->setLayer( layer );
  mSingleFieldComboBox->setFilters( QgsFieldProxyModel::DateTime | QgsFieldProxyModel::Date );
  mStartFieldComboBox->setFilters( QgsFieldProxyModel::DateTime | QgsFieldProxyModel::Date );
  mStartFieldComboBox->setAllowEmptyFieldName( true );
  mEndFieldComboBox->setFilters( QgsFieldProxyModel::DateTime | QgsFieldProxyModel::Date );
  mEndFieldComboBox->setAllowEmptyFieldName( true );
  mDurationStartFieldComboBox->setFilters( QgsFieldProxyModel::DateTime | QgsFieldProxyModel::Date );
  mDurationFieldComboBox->setFilters( QgsFieldProxyModel::Numeric );

  mFixedDurationSpinBox->setMinimum( 0 );
  mFixedDurationSpinBox->setClearValue( 0 );

  for ( QgsUnitTypes::TemporalUnit u :
        {
          QgsUnitTypes::TemporalMilliseconds,
          QgsUnitTypes::TemporalSeconds,
          QgsUnitTypes::TemporalMinutes,
          QgsUnitTypes::TemporalHours,
          QgsUnitTypes::TemporalDays,
          QgsUnitTypes::TemporalWeeks,
          QgsUnitTypes::TemporalMonths,
          QgsUnitTypes::TemporalYears,
          QgsUnitTypes::TemporalDecades,
          QgsUnitTypes::TemporalCenturies
        } )
  {
    const QString title = ( QgsGui::higFlags() & QgsGui::HigDialogTitleIsTitleCase ) ? QgsStringUtils::capitalize( QgsUnitTypes::toString( u ), QgsStringUtils::TitleCase )
                          : QgsUnitTypes::toString( u );
    mDurationUnitsComboBox->addItem( title, u );
    mFixedDurationUnitsComboBox->addItem( title, u );
  }

  mFixedDurationUnitsComboBox->setEnabled( !mAccumulateCheckBox->isChecked() );
  mFixedDurationSpinBox->setEnabled( !mAccumulateCheckBox->isChecked() );
  connect( mAccumulateCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mFixedDurationUnitsComboBox->setEnabled( !checked );
    mFixedDurationSpinBox->setEnabled( !checked );
  } );

  mStartExpressionWidget->setAllowEmptyFieldName( true );
  mEndExpressionWidget->setAllowEmptyFieldName( true );
  mStartExpressionWidget->setLayer( layer );
  mEndExpressionWidget->setLayer( layer );
  mStartExpressionWidget->registerExpressionContextGenerator( this );
  mEndExpressionWidget->registerExpressionContextGenerator( this );

  syncToLayer();
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
    case QgsVectorLayerTemporalProperties::ModeFeatureDateTimeStartAndEndFromExpressions:
      properties->setStartField( mSingleFieldComboBox->currentField() );
      properties->setDurationUnits( static_cast< QgsUnitTypes::TemporalUnit >( mFixedDurationUnitsComboBox->currentData().toInt() ) );
      break;

    case QgsVectorLayerTemporalProperties::ModeFeatureDateTimeStartAndEndFromFields:
      properties->setStartField( mStartFieldComboBox->currentField() );
      properties->setDurationUnits( static_cast< QgsUnitTypes::TemporalUnit >( mFixedDurationUnitsComboBox->currentData().toInt() ) );
      break;

    case QgsVectorLayerTemporalProperties::ModeFeatureDateTimeStartAndDurationFromFields:
      properties->setStartField( mDurationStartFieldComboBox->currentField() );
      properties->setDurationUnits( static_cast< QgsUnitTypes::TemporalUnit >( mDurationUnitsComboBox->currentData().toInt() ) );
      break;
  }

  properties->setEndField( mEndFieldComboBox->currentField() );
  properties->setDurationField( mDurationFieldComboBox->currentField() );
  properties->setFixedDuration( mFixedDurationSpinBox->value() );
  properties->setAccumulateFeatures( mAccumulateCheckBox->isChecked() );
  properties->setStartExpression( mStartExpressionWidget->currentField() );
  properties->setEndExpression( mEndExpressionWidget->currentField() );
}

QgsExpressionContext QgsVectorLayerTemporalPropertiesWidget::createExpressionContext() const
{
  QgsExpressionContext context;
  context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );
  return context;
}

void QgsVectorLayerTemporalPropertiesWidget::syncToLayer()
{
  const QgsVectorLayerTemporalProperties *properties = qobject_cast< QgsVectorLayerTemporalProperties * >( mLayer->temporalProperties() );
  mTemporalGroupBox->setChecked( properties->isActive() );

  mModeComboBox->setCurrentIndex( mModeComboBox->findData( properties->mode() ) );
  mStackedWidget->setCurrentIndex( static_cast< int >( properties->mode() ) );

  mStartTemporalDateTimeEdit->setDateTime( properties->fixedTemporalRange().begin() );
  mEndTemporalDateTimeEdit->setDateTime( properties->fixedTemporalRange().end() );

  mFixedDurationSpinBox->setValue( properties->fixedDuration() );

  mSingleFieldComboBox->setField( properties->startField() );
  mStartFieldComboBox->setField( properties->startField() );
  mDurationStartFieldComboBox->setField( properties->startField() );
  mEndFieldComboBox->setField( properties->endField() );
  mDurationFieldComboBox->setField( properties->durationField() );
  mDurationUnitsComboBox->setCurrentIndex( mDurationUnitsComboBox->findData( properties->durationUnits() ) );
  mFixedDurationUnitsComboBox->setCurrentIndex( mDurationUnitsComboBox->findData( properties->durationUnits() ) );

  mAccumulateCheckBox->setChecked( properties->accumulateFeatures() );

  mStartExpressionWidget->setField( properties->startExpression() );
  mEndExpressionWidget->setField( properties->endExpression() );
}
