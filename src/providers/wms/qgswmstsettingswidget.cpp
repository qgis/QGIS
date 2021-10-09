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
#include "qgstemporalutils.h"

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
  connect( mStaticTemporalRangeRadio, &QRadioButton::toggled, mStaticWmstRangeFrame, &QWidget::setEnabled );
  connect( mStaticTemporalRangeRadio, &QRadioButton::toggled, mStaticWmstChoiceFrame, &QWidget::setEnabled );

  mStaticWmstRangeFrame->setEnabled( false );
  mStaticWmstChoiceFrame->setEnabled( false );
  mReferenceTimeCombo->hide();
  mReferenceDateTimeEdit->show();

  syncToLayer( mRasterLayer );

  if ( mRasterLayer->temporalProperties() )
    connect( mRasterLayer->temporalProperties(), &QgsRasterLayerTemporalProperties::changed, this, &QgsWmstSettingsWidget::temporalPropertiesChange );

  QgsDateTimeRange range;
  if ( QgsProject::instance()->timeSettings() )
    range = QgsProject::instance()->timeSettings()->temporalRange();

  if ( range.begin().isValid() && range.end().isValid() )
    mProjectTemporalRangeLabel->setText( tr( "Project temporal range is set from %1 to %2" ).arg(
                                           range.begin().toString( QStringLiteral( "yyyy-MM-dd HH:mm:ss" ) ),
                                           range.end().toString( QStringLiteral( "yyyy-MM-dd HH:mm:ss" ) )
                                         ) );
  else
  {
    mProjectTemporalRangeRadio->setEnabled( false );
    mProjectTemporalRangeLabel->setText( tr( "The project does not have a temporal range set. "
                                         "Update the project temporal range via the Project Properties "
                                         "with valid values in order to use it here." ) );
    mProjectTemporalRangeLabel->setEnabled( false );
  }

  connect( this, &QgsMapLayerConfigWidget::dynamicTemporalControlToggled, this, [ = ]( bool checked )
  {
    if ( checked )
    {
      mStaticWmstStackedWidget->setCurrentIndex( 0 );
      // why do we hide this widget? well, the second page of the stacked widget is considerably higher
      // then the first and we don't want to show a whole bunch of empty vertical space which Qt will give
      // in order to accommodate the vertical height of the non-visible second page!
      mStaticStackedWidgetFrame->hide();
    }
    else
    {
      mStaticWmstStackedWidget->setCurrentIndex( 1 );
      mStaticStackedWidgetFrame->show();
    }
    mStaticWmstStackedWidget->updateGeometry();
  } );
}

void QgsWmstSettingsWidget::syncToLayer( QgsMapLayer *layer )
{
  mRasterLayer = qobject_cast< QgsRasterLayer * >( layer );
  if ( mRasterLayer->dataProvider() && mRasterLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() )
  {
    const QgsDateTimeRange availableProviderRange = mRasterLayer->dataProvider()->temporalCapabilities()->availableTemporalRange();
    const QgsDateTimeRange availableReferenceRange = mRasterLayer->dataProvider()->temporalCapabilities()->availableReferenceTemporalRange();

    const QList< QgsDateTimeRange > allAvailableRanges = mRasterLayer->dataProvider()->temporalCapabilities()->allAvailableTemporalRanges();
    // determine if available ranges are a set of non-contiguous instants, and if so, we show a combobox to users instead of the free-form date widgets
    if ( allAvailableRanges.size() < 50 && std::all_of( allAvailableRanges.cbegin(), allAvailableRanges.cend(), []( const QgsDateTimeRange & range ) { return range.isInstant(); } ) )
    {
      mStaticWmstRangeFrame->hide();
      mStaticWmstChoiceFrame->show();
      mStaticWmstRangeCombo->clear();
      for ( const QgsDateTimeRange &range : allAvailableRanges )
      {
        mStaticWmstRangeCombo->addItem( range.begin().toString( Qt::ISODate ), QVariant::fromValue( range ) );
      }
      mStaticTemporalRangeRadio->setText( tr( "Predefined date" ) );
    }
    else
    {
      mStaticWmstRangeFrame->show();
      mStaticWmstChoiceFrame->hide();
      mStaticTemporalRangeRadio->setText( tr( "Predefined range" ) );
    }

    QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata(
                                      mRasterLayer->providerType() );

    const QVariantMap uri = metadata->decodeUri( mRasterLayer->dataProvider()->dataSourceUri() );

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
      const QStringList parts = time.split( '/' );
      const QgsDateTimeRange range( QDateTime::fromString( parts.at( 0 ), Qt::ISODateWithMs ), QDateTime::fromString( parts.at( 1 ), Qt::ISODateWithMs ) );

      mStartStaticDateTimeEdit->setDateTime( range.begin() );
      mEndStaticDateTimeEdit->setDateTime( range.end() );

      if ( const int index = mStaticWmstRangeCombo->findData( QVariant::fromValue( range ) ); index >= 0 )
        mStaticWmstRangeCombo->setCurrentIndex( index );
      else if ( mStaticWmstRangeCombo->count() > 0 )
        mStaticWmstRangeCombo->setCurrentIndex( 0 );
    }
    else if ( mStaticWmstRangeCombo->count() > 0 )
    {
      mStaticWmstRangeCombo->setCurrentIndex( 0 );
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

      bool ok = false;
      bool maxValuesExceeded = false;
      const QList< QDateTime > availableReferenceTimes = QgsTemporalUtils::calculateDateTimesFromISO8601( referenceTimeExtent, ok, maxValuesExceeded, 30 );
      mReferenceTimeCombo->clear();
      if ( ok && !maxValuesExceeded )
      {
        mReferenceTimeCombo->show();
        mReferenceDateTimeEdit->hide();
        for ( const QDateTime &date : availableReferenceTimes )
        {
          mReferenceTimeCombo->addItem( date.toString( Qt::ISODate ), QVariant::fromValue( date ) );
        }
      }
      else
      {
        mReferenceTimeCombo->hide();
        mReferenceDateTimeEdit->show();
      }
    }
    if ( !referenceTime.isEmpty() && !referenceTimeExtent.isEmpty() )
    {
      const QDateTime referenceDateTime = QDateTime::fromString( referenceTime, Qt::ISODateWithMs );
      mReferenceDateTimeEdit->setDateTime( referenceDateTime );

      if ( const int index = mReferenceTimeCombo->findData( QVariant::fromValue( referenceDateTime ) ); index >= 0 )
        mReferenceTimeCombo->setCurrentIndex( index );
      else if ( mReferenceTimeCombo->count() > 0 )
        mReferenceTimeCombo->setCurrentIndex( 0 );
    }

    mFetchModeComboBox->addItem( tr( "Use Whole Temporal Range" ), static_cast< int >( Qgis::TemporalIntervalMatchMethod::MatchUsingWholeRange ) );
    mFetchModeComboBox->addItem( tr( "Match to Start of Range" ), static_cast< int >( Qgis::TemporalIntervalMatchMethod::MatchExactUsingStartOfRange ) );
    mFetchModeComboBox->addItem( tr( "Match to End of Range" ), static_cast< int >( Qgis::TemporalIntervalMatchMethod::MatchExactUsingEndOfRange ) );
    mFetchModeComboBox->addItem( tr( "Closest Match to Start of Range" ), static_cast< int >( Qgis::TemporalIntervalMatchMethod::FindClosestMatchToStartOfRange ) );
    mFetchModeComboBox->addItem( tr( "Closest Match to End of Range" ), static_cast< int >( Qgis::TemporalIntervalMatchMethod::FindClosestMatchToEndOfRange ) );
    mFetchModeComboBox->setCurrentIndex( mFetchModeComboBox->findData( static_cast< int >( qobject_cast< QgsRasterLayerTemporalProperties * >( mRasterLayer->temporalProperties() )->intervalHandlingMethod() ) ) );

    const QString temporalSource = uri.value( QStringLiteral( "temporalSource" ) ).toString();
    mDisableTime->setChecked( !uri.value( QStringLiteral( "enableTime" ), true ).toBool() );

    const bool useTemporal = uri.value( QStringLiteral( "allowTemporalUpdates" ), false ).toBool();
    if ( useTemporal && temporalSource == QLatin1String( "provider" ) && !time.isEmpty() )
      mStaticTemporalRangeRadio->setChecked( true );
    else if ( useTemporal && temporalSource == QLatin1String( "project" ) && !time.isEmpty() )
      mProjectTemporalRangeRadio->setChecked( true );
    else
      mDefaultRadio->setChecked( true );

    if ( mRasterLayer->temporalProperties()->isActive() )
    {
      mStaticWmstStackedWidget->setCurrentIndex( 0 );
      // why do we hide this widget? well, the second page of the stacked widget is considerably higher
      // then the first and we don't want to show a whole bunch of empty vertical space which Qt will give
      // in order to accommodate the vertical height of the non-visible second page!
      mStaticStackedWidgetFrame->hide();
    }
    else
    {
      mStaticWmstStackedWidget->setCurrentIndex( 1 );
      mStaticStackedWidgetFrame->show();
    }
    mStaticWmstStackedWidget->updateGeometry();
  }
}

void QgsWmstSettingsWidget::apply()
{
  QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( mRasterLayer->providerType() );
  const QVariantMap currentUri = metadata->decodeUri( mRasterLayer->dataProvider()->dataSourceUri() );

  QVariantMap uri = currentUri;

  if ( mStaticWmstStackedWidget->currentIndex() == 1 )
    uri[ QStringLiteral( "allowTemporalUpdates" ) ] = !mDefaultRadio->isChecked();
  else
    uri[ QStringLiteral( "allowTemporalUpdates" ) ] = true; // using dynamic temporal mode

  if ( mRasterLayer->dataProvider() &&
       mRasterLayer->dataProvider()->temporalCapabilities()->hasTemporalCapabilities() )
  {
    uri[ QStringLiteral( "enableTime" ) ] = !mDisableTime->isChecked();
    qobject_cast< QgsRasterLayerTemporalProperties * >( mRasterLayer->temporalProperties() )->setIntervalHandlingMethod( static_cast< Qgis::TemporalIntervalMatchMethod >(
          mFetchModeComboBox->currentData().toInt() ) );

    if ( mReferenceTimeGroupBox->isChecked() )
    {
      const QString referenceTime = mReferenceTimeCombo->count() > 0  ? mReferenceTimeCombo->currentData().value< QDateTime >().toString( Qt::ISODateWithMs )
                                    : mReferenceDateTimeEdit->dateTime().toString( Qt::ISODateWithMs );
      uri[ QStringLiteral( "referenceTime" ) ] = referenceTime;
    }
    else
    {
      uri.remove( QStringLiteral( "referenceTime" ) );
    }

    // Don't do static temporal updates if temporal properties are active
    if ( !mRasterLayer->temporalProperties()->isActive() && !mDefaultRadio->isChecked() )
    {
      if ( mStaticTemporalRangeRadio->isChecked() )
      {
        const QString time = mStaticWmstRangeCombo->count() > 0
                             ? ( mStaticWmstRangeCombo->currentData().value< QgsDateTimeRange >().begin().toString( Qt::ISODateWithMs ) + '/' + mStaticWmstRangeCombo->currentData().value< QgsDateTimeRange >().end().toString( Qt::ISODateWithMs ) )
                             : ( mStartStaticDateTimeEdit->dateTime().toString( Qt::ISODateWithMs ) + '/' + mEndStaticDateTimeEdit->dateTime().toString( Qt::ISODateWithMs ) );
        uri[ QStringLiteral( "time" ) ] = time;
        uri[ QStringLiteral( "temporalSource" ) ] = QLatin1String( "provider" );
      }
      else if ( mProjectTemporalRangeRadio->isChecked() )
      {
        QgsDateTimeRange range;

        if ( QgsProject::instance()->timeSettings() )
          range = QgsProject::instance()->timeSettings()->temporalRange();
        if ( range.begin().isValid() && range.end().isValid() )
        {
          const QString time = range.begin().toString( Qt::ISODateWithMs ) + '/' +
                               range.end().toString( Qt::ISODateWithMs );

          uri[ QStringLiteral( "time" ) ] = time;
          uri[ QStringLiteral( "temporalSource" ) ] = QLatin1String( "project" );
        }
      }
    }
    else
    {
      uri.remove( QStringLiteral( "temporalSource" ) );
      uri.remove( QStringLiteral( "time" ) );
    }
  }

  if ( currentUri != uri )
    mRasterLayer->setDataSource( metadata->encodeUri( uri ), mRasterLayer->name(), mRasterLayer->providerType(), QgsDataProvider::ProviderOptions() );
}

void QgsWmstSettingsWidget::temporalPropertiesChange()
{
  if ( mRasterLayer->temporalProperties()->isActive() )
  {
    mStaticWmstStackedWidget->setCurrentIndex( 0 );
    // why do we hide this widget? well, the second page of the stacked widget is considerably higher
    // then the first and we don't want to show a whole bunch of empty vertical space which Qt will give
    // in order to accommodate the vertical height of the non-visible second page!
    mStaticStackedWidgetFrame->hide();
  }
  else
  {
    mStaticWmstStackedWidget->setCurrentIndex( 1 );
    mStaticStackedWidgetFrame->show();
  }
  mStaticWmstStackedWidget->updateGeometry();
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
