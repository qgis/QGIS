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
#include "qgsiconutils.h"
#include <QHBoxLayout>
#include <QLabel>


QgsSensorThingsSourceWidget::QgsSensorThingsSourceWidget( QWidget *parent )
  : QgsProviderSourceWidget( parent )
{
  setupUi( this );

  mSpinPageSize->setClearValue( 0, tr( "Default" ) );

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
  mComboEntityType->setCurrentIndex( mComboEntityType->findData( QVariant::fromValue( Qgis::SensorThingsEntity::Location ) ) );

  rebuildGeometryTypes( Qgis::SensorThingsEntity::Location );

  connect( mComboEntityType, qOverload< int >( &QComboBox::currentIndexChanged ), this, &QgsSensorThingsSourceWidget::entityTypeChanged );
  connect( mComboGeometryType, qOverload< int >( &QComboBox::currentIndexChanged ), this, &QgsSensorThingsSourceWidget::validate );
  connect( mSpinPageSize, qOverload< int >( &QSpinBox::valueChanged ), this, &QgsSensorThingsSourceWidget::validate );
  validate();
}

void QgsSensorThingsSourceWidget::setSourceUri( const QString &uri )
{
  mSourceParts = QgsProviderRegistry::instance()->decodeUri(
                   QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY,
                   uri
                 );

  const Qgis::SensorThingsEntity type = QgsSensorThingsUtils::stringToEntity( mSourceParts.value( QStringLiteral( "entity" ) ).toString() );
  mComboEntityType->setCurrentIndex( mComboEntityType->findData( QVariant::fromValue( type ) ) );
  rebuildGeometryTypes( type );
  setCurrentGeometryTypeFromString( mSourceParts.value( QStringLiteral( "geometryType" ) ).toString() );

  bool ok = false;
  const int maxPageSizeParam = mSourceParts.value( QStringLiteral( "pageSize" ) ).toInt( &ok );
  if ( ok )
  {
    mSpinPageSize->setValue( maxPageSizeParam );
  }
  else
  {
    mSpinPageSize->clear();
  }

  mIsValid = true;
}

QString QgsSensorThingsSourceWidget::sourceUri() const
{
  return updateUriFromGui( QgsProviderRegistry::instance()->encodeUri(
                             QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY,
                             mSourceParts
                           ) );
}

QString QgsSensorThingsSourceWidget::groupTitle() const
{
  return QObject::tr( "SensorThings Configuration" );
}

QString QgsSensorThingsSourceWidget::updateUriFromGui( const QString &connectionUri ) const
{
  QVariantMap parts = QgsProviderRegistry::instance()->decodeUri(
                        QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY,
                        connectionUri
                      );

  const Qgis::SensorThingsEntity entityType = mComboEntityType->currentData().value< Qgis::SensorThingsEntity >();
  parts.insert( QStringLiteral( "entity" ), qgsEnumValueToKey( entityType ) );
  if ( !QgsSensorThingsUtils::entityTypeHasGeometry( entityType ) )
  {
    parts.remove( QStringLiteral( "geometryType" ) );
  }
  else
  {
    const Qgis::WkbType newWkbType = mComboGeometryType->currentData().value< Qgis::WkbType >();
    switch ( newWkbType )
    {
      case Qgis::WkbType::Point:
        parts.insert( QStringLiteral( "geometryType" ), QStringLiteral( "point" ) );
        break;
      case Qgis::WkbType::MultiPoint:
        parts.insert( QStringLiteral( "geometryType" ), QStringLiteral( "multipoint" ) );
        break;
      case Qgis::WkbType::MultiLineString:
        parts.insert( QStringLiteral( "geometryType" ), QStringLiteral( "line" ) );
        break;
      case Qgis::WkbType::MultiPolygon:
        parts.insert( QStringLiteral( "geometryType" ), QStringLiteral( "polygon" ) );
        break;
      default:
        break;
    }
  }

  if ( mSpinPageSize->value() > 0 )
  {
    parts.insert( QStringLiteral( "pageSize" ), QString::number( mSpinPageSize->value() ) );
  }
  else
  {
    parts.remove( QStringLiteral( "pageSize" ) );
  }

  return QgsProviderRegistry::instance()->encodeUri(
           QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY,
           parts
         );
}

void QgsSensorThingsSourceWidget::entityTypeChanged()
{
  const Qgis::SensorThingsEntity entityType = mComboEntityType->currentData().value< Qgis::SensorThingsEntity >();
  rebuildGeometryTypes( entityType );

  validate();
}

void QgsSensorThingsSourceWidget::validate()
{
  bool valid = mComboEntityType->currentIndex() >= 0;

  const Qgis::SensorThingsEntity entityType = mComboEntityType->currentData().value< Qgis::SensorThingsEntity >();
  if ( QgsSensorThingsUtils::entityTypeHasGeometry( entityType ) )
    valid = valid && mComboGeometryType->currentIndex() >= 0;

  if ( valid == mIsValid )
    return;

  mIsValid = valid;
  emit validChanged( mIsValid );
}

void QgsSensorThingsSourceWidget::rebuildGeometryTypes( Qgis::SensorThingsEntity type )
{
  if ( QgsSensorThingsUtils::entityTypeHasGeometry( type ) && mComboGeometryType->findData( QVariant::fromValue( Qgis::WkbType::Point ) ) < 0 )
  {
    mComboGeometryType->clear();
    mComboGeometryType->addItem( QgsIconUtils::iconForWkbType( Qgis::WkbType::Point ), tr( "Point" ), QVariant::fromValue( Qgis::WkbType::Point ) );
    mComboGeometryType->addItem( QgsIconUtils::iconForWkbType( Qgis::WkbType::MultiPoint ), tr( "Multipoint" ), QVariant::fromValue( Qgis::WkbType::MultiPoint ) );
    mComboGeometryType->addItem( QgsIconUtils::iconForWkbType( Qgis::WkbType::MultiLineString ), tr( "Line" ), QVariant::fromValue( Qgis::WkbType::MultiLineString ) );
    mComboGeometryType->addItem( QgsIconUtils::iconForWkbType( Qgis::WkbType::MultiPolygon ), tr( "Polygon" ), QVariant::fromValue( Qgis::WkbType::MultiPolygon ) );
    setCurrentGeometryTypeFromString( mSourceParts.value( QStringLiteral( "geometryType" ) ).toString() );
  }
  else if ( !QgsSensorThingsUtils::entityTypeHasGeometry( type ) && mComboGeometryType->findData( QVariant::fromValue( Qgis::WkbType::NoGeometry ) ) < 0 )
  {
    mComboGeometryType->clear();
    mComboGeometryType->addItem( QgsIconUtils::iconForWkbType( Qgis::WkbType::NoGeometry ), tr( "No Geometry" ), QVariant::fromValue( Qgis::WkbType::NoGeometry ) );
  }
}

void QgsSensorThingsSourceWidget::setCurrentGeometryTypeFromString( const QString &geometryType )
{
  if ( geometryType.compare( QLatin1String( "point" ), Qt::CaseInsensitive ) == 0 )
  {
    mComboGeometryType->setCurrentIndex( mComboGeometryType->findData( QVariant::fromValue( Qgis::WkbType::Point ) ) );
  }
  else if ( geometryType.compare( QLatin1String( "multipoint" ), Qt::CaseInsensitive ) == 0 )
  {
    mComboGeometryType->setCurrentIndex( mComboGeometryType->findData( QVariant::fromValue( Qgis::WkbType::MultiPoint ) ) );
  }
  else if ( geometryType.compare( QLatin1String( "line" ), Qt::CaseInsensitive ) == 0 )
  {
    mComboGeometryType->setCurrentIndex( mComboGeometryType->findData( QVariant::fromValue( Qgis::WkbType::MultiLineString ) ) );
  }
  else if ( geometryType.compare( QLatin1String( "polygon" ), Qt::CaseInsensitive ) == 0 )
  {
    mComboGeometryType->setCurrentIndex( mComboGeometryType->findData( QVariant::fromValue( Qgis::WkbType::MultiPolygon ) ) );
  }
  else if ( geometryType.isEmpty() )
  {
    mComboGeometryType->setCurrentIndex( 0 );
  }
}


///@endcond
