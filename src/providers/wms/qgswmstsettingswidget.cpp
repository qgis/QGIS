/***************************************************************************
      qgswmstsettingswidget.cpp
      ------------------
    begin                : March 2021
    copyright            : (C) 2021 by Nyall Dawson
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

#include "qgswmstsettingswidget.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsprojecttimesettings.h"
#include "qgsrasterlayertemporalproperties.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"

QgsWmstSettingsWidget::QgsWmstSettingsWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
  , mRasterLayer( qobject_cast< QgsRasterLayer* >( layer ) )
{
  Q_ASSERT( mRasterLayer );
  Q_ASSERT( mRasterLayer->dataProvider() );
  Q_ASSERT( mRasterLayer->providerType() == QLatin1String( "wms" ) );

  setupUi( this );

  connect( mSetEndAsStartStaticButton, &QPushButton::clicked, this, [ = ]
  {
    mEndStaticDateTimeEdit->setDateTime( mStartStaticDateTimeEdit->dateTime() );
  } );
  connect( mProjectTemporalRange, &QRadioButton::toggled, this, &QgsWmstSettingsWidget::passProjectTemporalRange_toggled );

  connect( mStaticTemporalRange, &QRadioButton::toggled, mStaticWmstFrame, &QWidget::setEnabled );

  syncToLayer( mRasterLayer );

  if ( mRasterLayer->temporalProperties() )
    connect( mRasterLayer->temporalProperties(), &QgsRasterLayerTemporalProperties::changed, this, &QgsWmstSettingsWidget::temporalPropertiesChange );
}

void QgsWmstSettingsWidget::syncToLayer( QgsMapLayer *layer )
{
  mRasterLayer = qobject_cast< QgsRasterLayer * >( layer );
  if ( mRasterLayer->dataProvider() && mRasterLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() )
  {
    const QgsDateTimeRange availableProviderRange = mRasterLayer->dataProvider()->temporalCapabilities()->availableTemporalRange();
    const QgsDateTimeRange availableReferenceRange = mRasterLayer->dataProvider()->temporalCapabilities()->availableReferenceTemporalRange();

    QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata(
                                      mRasterLayer->providerType() );

    QVariantMap uri = metadata->decodeUri( mRasterLayer->dataProvider()->dataSourceUri() );

    mStartStaticDateTimeEdit->setDisplayFormat( QStringLiteral( "yyyy-MM-dd HH:mm:ss" ) );
    mEndStaticDateTimeEdit->setDisplayFormat( QStringLiteral( "yyyy-MM-dd HH:mm:ss" ) );
    mReferenceDateTimeEdit->setDisplayFormat( QStringLiteral( "yyyy-MM-dd HH:mm:ss" ) );

    // setup maximum extents for widgets, based on provider's capabilities
    if ( availableProviderRange.begin().isValid() && availableProviderRange.end().isValid() )
    {
      mStartStaticDateTimeEdit->setDateTimeRange( availableProviderRange.begin(),
          availableProviderRange.end() );
      mStartStaticDateTimeEdit->setDateTime( availableProviderRange.begin() );
      mEndStaticDateTimeEdit->setDateTimeRange( availableProviderRange.begin(),
          availableProviderRange.end() );
      mEndStaticDateTimeEdit->setDateTime( availableProviderRange.end() );
    }
    if ( availableReferenceRange.begin().isValid() && availableReferenceRange.end().isValid() )
    {
      mReferenceDateTimeEdit->setDateTimeRange( availableReferenceRange.begin(),
          availableReferenceRange.end() );
      mReferenceDateTimeEdit->setDateTime( availableReferenceRange.begin() );
    }

    const QString time = uri.value( QStringLiteral( "time" ) ).toString();
    if ( !time.isEmpty() )
    {
      QStringList parts = time.split( '/' );
      mStartStaticDateTimeEdit->setDateTime( QDateTime::fromString( parts.at( 0 ), Qt::ISODateWithMs ) );
      mEndStaticDateTimeEdit->setDateTime( QDateTime::fromString( parts.at( 1 ), Qt::ISODateWithMs ) );
    }

    const QString referenceTimeExtent = uri.value( QStringLiteral( "referenceTimeDimensionExtent" ) ).toString();
    const QString referenceTime = uri.value( QStringLiteral( "referenceTime" ) ).toString();

    if ( referenceTimeExtent.isEmpty() )
    {
      mReferenceTimeExtentLabel->setText( tr( "No reference time is reported in the layer's capabilities." ) );
      mReferenceTimeGroupBox->setChecked( false );
      mReferenceTimeGroupBox->setEnabled( false );
    }
    else
    {
      mReferenceTimeExtentLabel->setText( tr( "Reported reference time extent: <i>%1</i>" ).arg( referenceTimeExtent ) );
      mReferenceTimeGroupBox->setEnabled( true );
      mReferenceTimeGroupBox->setChecked( !referenceTime.isEmpty() );
    }
    if ( !referenceTime.isEmpty() && !referenceTimeExtent.isEmpty() )
    {
      mReferenceDateTimeEdit->setDateTime( QDateTime::fromString( referenceTime, Qt::ISODateWithMs ) );
    }

    mFetchModeComboBox->addItem( tr( "Use Whole Temporal Range" ), QgsRasterDataProviderTemporalCapabilities::MatchUsingWholeRange );
    mFetchModeComboBox->addItem( tr( "Match to Start of Range" ), QgsRasterDataProviderTemporalCapabilities::MatchExactUsingStartOfRange );
    mFetchModeComboBox->addItem( tr( "Match to End of Range" ), QgsRasterDataProviderTemporalCapabilities::MatchExactUsingEndOfRange );
    mFetchModeComboBox->addItem( tr( "Closest Match to Start of Range" ), QgsRasterDataProviderTemporalCapabilities::FindClosestMatchToStartOfRange );
    mFetchModeComboBox->addItem( tr( "Closest Match to End of Range" ), QgsRasterDataProviderTemporalCapabilities::FindClosestMatchToEndOfRange );
    mFetchModeComboBox->setCurrentIndex( mFetchModeComboBox->findData( qobject_cast< QgsRasterLayerTemporalProperties * >( mRasterLayer->temporalProperties() )->intervalHandlingMethod() ) );

    const QString temporalSource = uri.value( QStringLiteral( "temporalSource" ) ).toString();
    bool enableTime = uri.value( QStringLiteral( "enableTime" ), true ).toBool();

    if ( temporalSource == QLatin1String( "provider" ) )
      mStaticTemporalRange->setChecked( !time.isEmpty() );
    else if ( temporalSource == QLatin1String( "project" ) )
      mProjectTemporalRange->setChecked( !time.isEmpty() );

    mDisableTime->setChecked( !enableTime );

    mWmstOptions->setEnabled( !mRasterLayer->temporalProperties()->isActive() );

    if ( mRasterLayer->temporalProperties()->isActive() )
      mWmstOptionsLabel->setText( tr( "The static temporal options below are disabled because the layer "
                                      "temporal properties are active, to enable them disable temporal properties "
                                      "in the temporal tab. " ) );
    QgsDateTimeRange range;
    if ( QgsProject::instance()->timeSettings() )
      range = QgsProject::instance()->timeSettings()->temporalRange();

    if ( !range.begin().isValid() || !range.end().isValid() )
    {
      mProjectTemporalRange->setEnabled( false );
      mProjectTemporalRangeLabel->setText( tr( "The option below is disabled because the project temporal range "
                                           "is not valid, update the project temporal range in the project properties "
                                           "with valid values in order to use it here." ) );
    }

    mWmstGroup->setChecked( uri.contains( QStringLiteral( "allowTemporalUpdates" ) ) &&
                            uri.value( QStringLiteral( "allowTemporalUpdates" ), true ).toBool() );
  }
}

void QgsWmstSettingsWidget::apply()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( mRasterLayer->providerType() );
  const QVariantMap currentUri = metadata->decodeUri( mRasterLayer->dataProvider()->dataSourceUri() );

  QVariantMap uri = currentUri;

  if ( mWmstGroup->isVisibleTo( this ) )
    uri[ QStringLiteral( "allowTemporalUpdates" ) ] = mWmstGroup->isChecked();

  if ( mWmstGroup->isEnabled() &&
       mRasterLayer->dataProvider() &&
       mRasterLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() )
  {
    bool enableTime = !mDisableTime->isChecked();

    uri[ QStringLiteral( "enableTime" ) ] = enableTime;
    qobject_cast< QgsRasterLayerTemporalProperties * >( mRasterLayer->temporalProperties() )->setIntervalHandlingMethod( static_cast< QgsRasterDataProviderTemporalCapabilities::IntervalHandlingMethod >(
          mFetchModeComboBox->currentData().toInt() ) );

    // Don't do static temporal updates if temporal properties are active
    if ( !mRasterLayer->temporalProperties()->isActive() )
    {
      if ( mStaticTemporalRange->isChecked() )
      {
        QString time = mStartStaticDateTimeEdit->dateTime().toString( Qt::ISODateWithMs ) + '/' +
                       mEndStaticDateTimeEdit->dateTime().toString( Qt::ISODateWithMs );
        uri[ QStringLiteral( "time" ) ] = time;
        uri[ QStringLiteral( "temporalSource" ) ] = QLatin1String( "provider" );
      }

      if ( mProjectTemporalRange->isChecked() )
      {
        QgsDateTimeRange range;

        if ( QgsProject::instance()->timeSettings() )
          range = QgsProject::instance()->timeSettings()->temporalRange();
        if ( range.begin().isValid() && range.end().isValid() )
        {
          QString time = range.begin().toString( Qt::ISODateWithMs ) + '/' +
                         range.end().toString( Qt::ISODateWithMs );

          uri[ QStringLiteral( "time" ) ] = time;
          uri[ QStringLiteral( "temporalSource" ) ] = QLatin1String( "project" );
        }
      }
    }

    if ( mReferenceTimeGroupBox->isChecked() )
    {
      QString referenceTime = mReferenceDateTimeEdit->dateTime().toString( Qt::ISODateWithMs );
      uri[ QStringLiteral( "referenceTime" ) ] = referenceTime;
    }
    else
    {
      if ( uri.contains( QStringLiteral( "referenceTime" ) ) )
        uri.remove( QStringLiteral( "referenceTime" ) );
    }
  }

  if ( currentUri != uri )
    mRasterLayer->setDataSource( metadata->encodeUri( uri ), mRasterLayer->name(), mRasterLayer->providerType(), QgsDataProvider::ProviderOptions() );
}

void QgsWmstSettingsWidget::passProjectTemporalRange_toggled( bool checked )
{
  if ( checked )
  {
    QgsDateTimeRange range;
    if ( QgsProject::instance()->timeSettings() )
      range = QgsProject::instance()->timeSettings()->temporalRange();

    if ( range.begin().isValid() && range.end().isValid() )
      mProjectTemporalRangeLabel->setText( tr( "Project temporal range is set from %1 to %2" ).arg(
                                             range.begin().toString( QStringLiteral( "yyyy-MM-dd HH:mm:ss" ) ),
                                             range.end().toString( QStringLiteral( "yyyy-MM-dd HH:mm:ss" ) )
                                           ) );
    else
      mProjectTemporalRangeLabel->setText( tr( "The option below is disabled because the project temporal range "
                                           "is not valid, update the project temporal range in the project properties "
                                           "with valid values in order to use it here." ) );
  }
}

void QgsWmstSettingsWidget::temporalPropertiesChange()
{
  if ( mRasterLayer->temporalProperties()->isActive() )
    mWmstOptionsLabel->setText( tr( "The static temporal options below are disabled because the layer "
                                    "temporal properties are active, to enable them disable temporal properties "
                                    "in the temporal tab." ) );
  else
    mWmstOptionsLabel->clear();

  mWmstOptions->setEnabled( !mRasterLayer->temporalProperties()->isActive() );
}


//
// QgsWmstSettingsConfigWidgetFactory
//

bool QgsWmstSettingsConfigWidgetFactory::supportLayerPropertiesDialog() const
{
  return true;
}

bool QgsWmstSettingsConfigWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer && layer->isValid() && layer->providerType() == QLatin1String( "wms" ) && layer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities();
}

QgsMapLayerConfigWidgetFactory::ParentPage QgsWmstSettingsConfigWidgetFactory::parentPage() const
{
  return ParentPage::Temporal;
}

QgsMapLayerConfigWidget *QgsWmstSettingsConfigWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool, QWidget *parent ) const
{
  return new QgsWmstSettingsWidget( layer, canvas, parent );
}
