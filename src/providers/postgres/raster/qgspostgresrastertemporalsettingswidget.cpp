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
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsprojecttimesettings.h"
#include "qgsrasterlayertemporalproperties.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"

QgsPostgresRasterTemporalSettingsWidget::QgsPostgresRasterTemporalSettingsWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
  , mRasterLayer( qobject_cast< QgsRasterLayer* >( layer ) )
{
  Q_ASSERT( mRasterLayer );
  Q_ASSERT( mRasterLayer->dataProvider() );
  Q_ASSERT( mRasterLayer->providerType() == QLatin1String( "postgresraster" ) );

  setupUi( this );

  mPostgresRasterTemporalGroup->setEnabled( true );
  mPostgresRasterTemporalGroup->setVisible( true );
  mPostgresRasterTemporalGroup->setChecked( false );

  mPostgresRasterTemporalFieldComboBox->setFilters( QgsFieldProxyModel::Filter::Date |
      QgsFieldProxyModel::Filter::DateTime |
      QgsFieldProxyModel::Filter::String );
  mPostgresRasterTemporalFieldComboBox->setAllowEmptyFieldName( true );
  connect( mPostgresRasterTemporalFieldComboBox, &QgsFieldComboBox::fieldChanged, this, [ = ]( const QString & fieldName )
  {
    mPostgresRasterDefaultTime->setEnabled( ! fieldName.isEmpty() );
  } );
  mPostgresRasterDefaultTime->setAllowNull( true );
  mPostgresRasterDefaultTime->setEmpty();

  syncToLayer( mRasterLayer );
}

void QgsPostgresRasterTemporalSettingsWidget::syncToLayer( QgsMapLayer *layer )
{
  mRasterLayer = qobject_cast< QgsRasterLayer * >( layer );
  const QgsFields fields { mRasterLayer->dataProvider()->fields() };
  mPostgresRasterTemporalFieldComboBox->setFields( fields );

  if ( mRasterLayer->dataProvider()->uri().hasParam( QStringLiteral( "temporalFieldIndex" ) ) )
  {
    bool ok;
    const int fieldIdx {  mRasterLayer->dataProvider()->uri().param( QStringLiteral( "temporalFieldIndex" ) ).toInt( &ok ) };
    if ( ok && fields.exists( fieldIdx ) )
    {
      mPostgresRasterTemporalGroup->setChecked( true );
      mPostgresRasterTemporalFieldComboBox->setField( fields.field( fieldIdx ).name() );
      if ( mRasterLayer->dataProvider()->uri().hasParam( QStringLiteral( "temporalDefaultTime" ) ) )
      {
        const QDateTime defaultDateTime { QDateTime::fromString( mRasterLayer->dataProvider()->uri().param( QStringLiteral( "temporalDefaultTime" ) ), Qt::DateFormat::ISODate ) };
        if ( defaultDateTime.isValid() )
        {
          mPostgresRasterDefaultTime->setDateTime( defaultDateTime );
        }
      }
    }
  }
}

void QgsPostgresRasterTemporalSettingsWidget::apply()
{
  QgsDataSourceUri uri { mRasterLayer->dataProvider()->uri() };
  if ( mPostgresRasterTemporalGroup->isEnabled() &&
       mPostgresRasterTemporalGroup->isChecked() &&
       ! mPostgresRasterTemporalFieldComboBox->currentField().isEmpty() )
  {
    const QString originaUri { uri.uri() };
    const int fieldIdx { mRasterLayer->dataProvider()->fields().lookupField( mPostgresRasterTemporalFieldComboBox->currentField() ) };
    uri.removeParam( QStringLiteral( "temporalFieldIndex" ) );
    uri.removeParam( QStringLiteral( "temporalDefaultTime" ) );
    if ( fieldIdx >= 0 )
    {
      uri.setParam( QStringLiteral( "temporalFieldIndex" ), QString::number( fieldIdx ) );
      if ( mPostgresRasterDefaultTime->dateTime().isValid() )
      {
        QDateTime defaultDateTime  { mPostgresRasterDefaultTime->dateTime() };
        const QTime defaultTime { defaultDateTime.time() };
        // Set secs to 0
        defaultDateTime.setTime( { defaultTime.hour(), defaultTime.minute(), 0 } );
        uri.setParam( QStringLiteral( "temporalDefaultTime" ), defaultDateTime.toString( Qt::DateFormat::ISODate ) );
      }
      if ( uri.uri( ) != originaUri )
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
