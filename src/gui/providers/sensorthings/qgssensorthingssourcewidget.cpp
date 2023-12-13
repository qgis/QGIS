/***************************************************************************
    qgssensorthingssourcewidget.cpp
     --------------------------------------
    Date                 : December 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssensorthingssourcewidget.h"
///@cond PRIVATE

#include "qgsproviderregistry.h"
#include "qgssensorthingsutils.h"
#include "qgssensorthingsprovider.h"
#include <QHBoxLayout>
#include <QLabel>


QgsSensorThingsSourceWidget::QgsSensorThingsSourceWidget( QWidget *parent )
  : QgsProviderSourceWidget( parent )
{
  setupUi( this );

  for ( Qgis::SensorThingsEntity type :
        {
          Qgis::SensorThingsEntity::Thing,
          Qgis::SensorThingsEntity::Location,
          Qgis::SensorThingsEntity::HistoricalLocation,
          Qgis::SensorThingsEntity::Datastream,
          Qgis::SensorThingsEntity::Sensor,
          Qgis::SensorThingsEntity::ObservedProperty,
          Qgis::SensorThingsEntity::Observation,
          Qgis::SensorThingsEntity::FeatureOfInterest,
        } )
  {
    mComboEntityType->addItem( QgsSensorThingsUtils::displayString( type, true ), QVariant::fromValue( type ) );
  }


  connect( mComboEntityType, qOverload< int >( &QComboBox::currentIndexChanged ), this, &QgsSensorThingsSourceWidget::validate );
}

void QgsSensorThingsSourceWidget::setSourceUri( const QString &uri )
{
  mSourceParts = QgsProviderRegistry::instance()->decodeUri(
                   QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY,
                   uri
                 );

  const Qgis::SensorThingsEntity type = QgsSensorThingsUtils::stringToEntity( mSourceParts.value( QStringLiteral( "entity" ) ).toString() );
  mComboEntityType->setCurrentIndex( mComboEntityType->findData( QVariant::fromValue( type ) ) );

  mIsValid = true;
}

QString QgsSensorThingsSourceWidget::sourceUri() const
{
  QVariantMap parts = mSourceParts;

  parts.insert( QStringLiteral( "entity" ), qgsEnumValueToKey( mComboEntityType->currentData().value< Qgis::SensorThingsEntity >() ) );

  return QgsProviderRegistry::instance()->encodeUri(
           QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY,
           parts
         );
}

void QgsSensorThingsSourceWidget::validate()
{
  const bool valid = mComboEntityType->currentIndex() >= 0;

  if ( valid != mIsValid )
    emit validChanged( valid );

  mIsValid = valid;
}


///@endcond
