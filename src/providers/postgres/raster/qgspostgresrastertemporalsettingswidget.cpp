/***************************************************************************
      qgspostgresrastertemporalsettingswidget.cpp
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

#include "qgspostgresrastertemporalsettingswidget.h"
#include "moc_qgspostgresrastertemporalsettingswidget.cpp"
#include "qgsmaplayer.h"
#include "qgsrasterlayer.h"

QgsPostgresRasterTemporalSettingsWidget::QgsPostgresRasterTemporalSettingsWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
  , mRasterLayer( qobject_cast<QgsRasterLayer *>( layer ) )
{
  Q_ASSERT( mRasterLayer );
  Q_ASSERT( mRasterLayer->dataProvider() );
  Q_ASSERT( mRasterLayer->providerType() == QLatin1String( "postgresraster" ) );

  setupUi( this );

  mPostgresRasterTemporalGroup->setEnabled( true );
  mPostgresRasterTemporalGroup->setVisible( true );
  mPostgresRasterTemporalGroup->setChecked( false );

  mPostgresRasterTemporalFieldComboBox->setFilters( QgsFieldProxyModel::Filter::Date | QgsFieldProxyModel::Filter::DateTime | QgsFieldProxyModel::Filter::String );
  mPostgresRasterTemporalFieldComboBox->setAllowEmptyFieldName( true );
  connect( mPostgresRasterTemporalFieldComboBox, &QgsFieldComboBox::fieldChanged, this, [=]( const QString &fieldName ) {
    mPostgresRasterDefaultTime->setEnabled( !fieldName.isEmpty() );
  } );
  mPostgresRasterDefaultTime->setAllowNull( true );
  mPostgresRasterDefaultTime->setEmpty();
  mDefaultTimeStackedWidget->setCurrentIndex( 0 );

  syncToLayer( mRasterLayer );
}

void QgsPostgresRasterTemporalSettingsWidget::syncToLayer( QgsMapLayer *layer )
{
  mRasterLayer = qobject_cast<QgsRasterLayer *>( layer );
  const QgsFields fields { mRasterLayer->dataProvider()->fields() };
  mPostgresRasterTemporalFieldComboBox->setFields( fields );

  mDefaultTimeStackedWidget->setCurrentIndex( 0 );
  mDefaultTimeComboBox->clear();

  if ( mRasterLayer->dataProvider()->uri().hasParam( QStringLiteral( "temporalFieldIndex" ) ) )
  {
    bool ok;
    const int fieldIdx { mRasterLayer->dataProvider()->uri().param( QStringLiteral( "temporalFieldIndex" ) ).toInt( &ok ) };
    if ( ok && fields.exists( fieldIdx ) )
    {
      mPostgresRasterTemporalGroup->setChecked( true );
      mPostgresRasterTemporalFieldComboBox->setField( fields.field( fieldIdx ).name() );

      const QList<QgsDateTimeRange> allRanges = mRasterLayer->dataProvider()->temporalCapabilities()->allAvailableTemporalRanges();
      if ( !allRanges.empty() && allRanges.size() < 50 )
      {
        // if an appropriate number of unique ranges is known, show a combo box with these options instead of the free-form
        // date picker widget
        mDefaultTimeStackedWidget->setCurrentIndex( 1 );
        for ( const QgsDateTimeRange &range : allRanges )
        {
          mDefaultTimeComboBox->addItem( range.begin().toString( Qt::ISODate ), QVariant::fromValue( range.begin() ) );
        }
      }

      if ( mRasterLayer->dataProvider()->uri().hasParam( QStringLiteral( "temporalDefaultTime" ) ) )
      {
        const QDateTime defaultDateTime { QDateTime::fromString( mRasterLayer->dataProvider()->uri().param( QStringLiteral( "temporalDefaultTime" ) ), Qt::DateFormat::ISODate ) };
        if ( defaultDateTime.isValid() )
        {
          mPostgresRasterDefaultTime->setDateTime( defaultDateTime );

          if ( const int index = mDefaultTimeComboBox->findData( QVariant::fromValue( defaultDateTime ) ); index >= 0 )
            mDefaultTimeComboBox->setCurrentIndex( index );
          else if ( mDefaultTimeComboBox->count() > 0 )
            mDefaultTimeComboBox->setCurrentIndex( 0 );
        }
      }
    }
  }
}

void QgsPostgresRasterTemporalSettingsWidget::apply()
{
  QgsDataSourceUri uri { mRasterLayer->dataProvider()->uri() };
  if ( mPostgresRasterTemporalGroup->isEnabled() && mPostgresRasterTemporalGroup->isChecked() && !mPostgresRasterTemporalFieldComboBox->currentField().isEmpty() )
  {
    const QString originaUri { uri.uri() };
    const int fieldIdx { mRasterLayer->dataProvider()->fields().lookupField( mPostgresRasterTemporalFieldComboBox->currentField() ) };
    uri.removeParam( QStringLiteral( "temporalFieldIndex" ) );
    uri.removeParam( QStringLiteral( "temporalDefaultTime" ) );
    if ( fieldIdx >= 0 )
    {
      uri.setParam( QStringLiteral( "temporalFieldIndex" ), QString::number( fieldIdx ) );

      QDateTime defaultDateTime;
      if ( mDefaultTimeStackedWidget->currentIndex() == 0 )
      {
        if ( mPostgresRasterDefaultTime->dateTime().isValid() )
        {
          defaultDateTime = mPostgresRasterDefaultTime->dateTime();
        }
      }
      else
      {
        if ( mDefaultTimeComboBox->currentData().isValid() )
        {
          defaultDateTime = mDefaultTimeComboBox->currentData().value<QDateTime>();
        }
      }

      if ( defaultDateTime.isValid() )
      {
        const QTime defaultTime { defaultDateTime.time() };
        // Set secs to 0
        defaultDateTime.setTime( { defaultTime.hour(), defaultTime.minute(), 0 } );
        uri.setParam( QStringLiteral( "temporalDefaultTime" ), defaultDateTime.toString( Qt::DateFormat::ISODate ) );
      }

      if ( uri.uri() != originaUri )
        mRasterLayer->setDataSource( uri.uri(), mRasterLayer->name(), mRasterLayer->providerType(), QgsDataProvider::ProviderOptions() );
    }
  }
  else if ( uri.hasParam( QStringLiteral( "temporalFieldIndex" ) ) )
  {
    uri.removeParam( QStringLiteral( "temporalFieldIndex" ) );
    uri.removeParam( QStringLiteral( "temporalDefaultTime" ) );
    mRasterLayer->setDataSource( uri.uri(), mRasterLayer->name(), mRasterLayer->providerType(), QgsDataProvider::ProviderOptions() );
  }
}


//
// QgsPostgresRasterTemporalSettingsConfigWidgetFactory
//

bool QgsPostgresRasterTemporalSettingsConfigWidgetFactory::supportLayerPropertiesDialog() const
{
  return true;
}

bool QgsPostgresRasterTemporalSettingsConfigWidgetFactory::supportsLayer( QgsMapLayer *layer ) const
{
  return layer && layer->isValid() && layer->providerType() == QLatin1String( "postgresraster" );
}

QgsMapLayerConfigWidgetFactory::ParentPage QgsPostgresRasterTemporalSettingsConfigWidgetFactory::parentPage() const
{
  return ParentPage::Temporal;
}

QgsMapLayerConfigWidget *QgsPostgresRasterTemporalSettingsConfigWidgetFactory::createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool, QWidget *parent ) const
{
  return new QgsPostgresRasterTemporalSettingsWidget( layer, canvas, parent );
}
