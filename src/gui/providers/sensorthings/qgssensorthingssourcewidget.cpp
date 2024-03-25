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
#include "qgssensorthingsconnectionpropertiestask.h"
#include "qgsapplication.h"
#include "qgsextentwidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QStandardItemModel>

QgsSensorThingsSourceWidget::QgsSensorThingsSourceWidget( QWidget *parent )
  : QgsProviderSourceWidget( parent )
{
  setupUi( this );

  QVBoxLayout *vl = new QVBoxLayout();
  vl->setContentsMargins( 0, 0, 0, 0 );
  mExtentWidget = new QgsExtentWidget( nullptr, QgsExtentWidget::CondensedStyle );
  mExtentWidget->setNullValueAllowed( true, tr( "Not set" ) );
  mExtentWidget->setOutputCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  vl->addWidget( mExtentWidget );
  mExtentLimitFrame->setLayout( vl );

  mSpinPageSize->setClearValue( 0, tr( "Default (%1)" ).arg( QgsSensorThingsUtils::DEFAULT_PAGE_SIZE ) );
  mSpinFeatureLimit->setClearValue( 0, tr( "No limit" ) );
  // set a relatively conservative feature limit by default, to make it so they have to opt-in to shoot themselves in the foot!
  mSpinFeatureLimit->setValue( QgsSensorThingsUtils::DEFAULT_FEATURE_LIMIT );

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
          Qgis::SensorThingsEntity::MultiDatastream,
        } )
  {
    mComboEntityType->addItem( QgsSensorThingsUtils::displayString( type, true ), QVariant::fromValue( type ) );
  }
  mComboEntityType->setCurrentIndex( mComboEntityType->findData( QVariant::fromValue( Qgis::SensorThingsEntity::Location ) ) );

  rebuildGeometryTypes( Qgis::SensorThingsEntity::Location );

  connect( mComboEntityType, qOverload< int >( &QComboBox::currentIndexChanged ), this, &QgsSensorThingsSourceWidget::entityTypeChanged );
  connect( mComboGeometryType, qOverload< int >( &QComboBox::currentIndexChanged ), this, &QgsSensorThingsSourceWidget::validate );
  connect( mSpinPageSize, qOverload< int >( &QSpinBox::valueChanged ), this, &QgsSensorThingsSourceWidget::validate );
  connect( mRetrieveTypesButton, &QToolButton::clicked, this, &QgsSensorThingsSourceWidget::retrieveTypes );
  mRetrieveTypesButton->setEnabled( false );
  connect( mExtentWidget, &QgsExtentWidget::extentChanged, this, &QgsSensorThingsSourceWidget::validate );
  validate();
}

QgsSensorThingsSourceWidget::~QgsSensorThingsSourceWidget()
{
  if ( mPropertiesTask )
  {
    disconnect( mPropertiesTask, &QgsTask::taskCompleted, this, &QgsSensorThingsSourceWidget::connectionPropertiesTaskCompleted );
    mPropertiesTask->cancel();
    mPropertiesTask = nullptr;
  }
}

void QgsSensorThingsSourceWidget::setSourceUri( const QString &uri )
{
  mSourceParts = QgsProviderRegistry::instance()->decodeUri(
                   QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY,
                   uri
                 );

  const Qgis::SensorThingsEntity type = QgsSensorThingsUtils::stringToEntity( mSourceParts.value( QStringLiteral( "entity" ) ).toString() );
  if ( type != Qgis::SensorThingsEntity::Invalid )
    mComboEntityType->setCurrentIndex( mComboEntityType->findData( QVariant::fromValue( type ) ) );

  rebuildGeometryTypes( mComboEntityType->currentData().value< Qgis::SensorThingsEntity >() );
  setCurrentGeometryTypeFromString( mSourceParts.value( QStringLiteral( "geometryType" ) ).toString() );

  bool ok = false;
  const int maxPageSizeParam = mSourceParts.value( QStringLiteral( "pageSize" ) ).toInt( &ok );
  if ( ok )
  {
    mSpinPageSize->setValue( maxPageSizeParam );
  }

  ok = false;
  const int featureLimitParam = mSourceParts.value( QStringLiteral( "featureLimit" ) ).toInt( &ok );
  if ( ok )
  {
    mSpinFeatureLimit->setValue( featureLimitParam );
  }
  else if ( type != Qgis::SensorThingsEntity::Invalid )
  {
    // if not setting an initial uri for a new layer, use "no limit" if it's not present in the uri
    mSpinFeatureLimit->clear();
  }
  else
  {
    // when setting an initial uri, use the default, not "no limit"
    mSpinFeatureLimit->setValue( QgsSensorThingsUtils::DEFAULT_FEATURE_LIMIT );
  }

  const QgsRectangle bounds = mSourceParts.value( QStringLiteral( "bounds" ) ).value< QgsRectangle >();
  if ( !bounds.isNull() )
  {
    mExtentWidget->setCurrentExtent( bounds, QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
    mExtentWidget->setOutputExtentFromUser( bounds, QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  }
  else
  {
    mExtentWidget->clear();
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

void QgsSensorThingsSourceWidget::setMapCanvas( QgsMapCanvas *mapCanvas )
{
  QgsProviderSourceWidget::setMapCanvas( mapCanvas );
  mExtentWidget->setMapCanvas( mapCanvas, false );
}

Qgis::SensorThingsEntity QgsSensorThingsSourceWidget::currentEntityType() const
{
  return mComboEntityType->currentData().value< Qgis::SensorThingsEntity >();
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
      case Qgis::WkbType::NoGeometry:
        parts.remove( QStringLiteral( "geometryType" ) );
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

  if ( mSpinFeatureLimit->value() > 0 )
  {
    parts.insert( QStringLiteral( "featureLimit" ), QString::number( mSpinFeatureLimit->value() ) );
  }
  else
  {
    parts.remove( QStringLiteral( "featureLimit" ) );
  }

  if ( mExtentWidget->outputExtent().isNull() )
    parts.remove( QStringLiteral( "bounds" ) );
  else
    parts.insert( QStringLiteral( "bounds" ), QVariant::fromValue( mExtentWidget->outputExtent() ) );

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

void QgsSensorThingsSourceWidget::retrieveTypes()
{
  if ( mPropertiesTask )
  {
    disconnect( mPropertiesTask, &QgsTask::taskCompleted, this, &QgsSensorThingsSourceWidget::connectionPropertiesTaskCompleted );
    mPropertiesTask->cancel();
    mPropertiesTask = nullptr;
  }

  mPropertiesTask = new QgsSensorThingsConnectionPropertiesTask( mSourceParts.value( QStringLiteral( "url" ) ).toString(),
      mComboEntityType->currentData().value< Qgis::SensorThingsEntity >() );
  connect( mPropertiesTask, &QgsTask::taskCompleted, this, &QgsSensorThingsSourceWidget::connectionPropertiesTaskCompleted );
  QgsApplication::taskManager()->addTask( mPropertiesTask );
  mRetrieveTypesButton->setEnabled( false );
}

void QgsSensorThingsSourceWidget::connectionPropertiesTaskCompleted()
{
  const QList< Qgis::GeometryType > availableTypes = mPropertiesTask->geometryTypes();
  const Qgis::WkbType currentWkbType = mComboGeometryType->currentData().value< Qgis::WkbType >();
  mComboGeometryType->clear();

  if ( availableTypes.contains( Qgis::GeometryType::Point ) )
  {
    mComboGeometryType->addItem( QgsIconUtils::iconForWkbType( Qgis::WkbType::Point ), tr( "Point" ), QVariant::fromValue( Qgis::WkbType::Point ) );
    mComboGeometryType->addItem( QgsIconUtils::iconForWkbType( Qgis::WkbType::MultiPoint ), tr( "Multipoint" ), QVariant::fromValue( Qgis::WkbType::MultiPoint ) );
  }
  if ( availableTypes.contains( Qgis::GeometryType::Line ) )
  {
    mComboGeometryType->addItem( QgsIconUtils::iconForWkbType( Qgis::WkbType::MultiLineString ), tr( "Line" ), QVariant::fromValue( Qgis::WkbType::MultiLineString ) );
  }
  if ( availableTypes.contains( Qgis::GeometryType::Polygon ) )
  {
    mComboGeometryType->addItem( QgsIconUtils::iconForWkbType( Qgis::WkbType::MultiPolygon ), tr( "Polygon" ), QVariant::fromValue( Qgis::WkbType::MultiPolygon ) );
  }

  mComboGeometryType->setCurrentIndex( mComboGeometryType->findData( QVariant::fromValue( currentWkbType ) ) );
  if ( mComboGeometryType->currentIndex() < 0 )
    mComboGeometryType->setCurrentIndex( 0 );
}

void QgsSensorThingsSourceWidget::rebuildGeometryTypes( Qgis::SensorThingsEntity type )
{
  if ( mPropertiesTask )
  {
    disconnect( mPropertiesTask, &QgsTask::taskCompleted, this, &QgsSensorThingsSourceWidget::connectionPropertiesTaskCompleted );
    mPropertiesTask->cancel();
    mPropertiesTask = nullptr;
  }

  mRetrieveTypesButton->setEnabled( QgsSensorThingsUtils::geometryTypeForEntity( type ) == Qgis::GeometryType::Unknown
                                    && !mSourceParts.value( QStringLiteral( "url" ) ).toString().isEmpty() );
  const Qgis::GeometryType geometryTypeForEntity = QgsSensorThingsUtils::geometryTypeForEntity( type );
  if ( geometryTypeForEntity == Qgis::GeometryType::Unknown && mComboGeometryType->findData( QVariant::fromValue( Qgis::WkbType::Point ) ) < 0 )
  {
    mComboGeometryType->clear();
    mComboGeometryType->addItem( QgsIconUtils::iconForWkbType( Qgis::WkbType::Point ), tr( "Point" ), QVariant::fromValue( Qgis::WkbType::Point ) );
    mComboGeometryType->addItem( QgsIconUtils::iconForWkbType( Qgis::WkbType::MultiPoint ), tr( "Multipoint" ), QVariant::fromValue( Qgis::WkbType::MultiPoint ) );
    mComboGeometryType->addItem( QgsIconUtils::iconForWkbType( Qgis::WkbType::MultiLineString ), tr( "Line" ), QVariant::fromValue( Qgis::WkbType::MultiLineString ) );
    mComboGeometryType->addItem( QgsIconUtils::iconForWkbType( Qgis::WkbType::MultiPolygon ), tr( "Polygon" ), QVariant::fromValue( Qgis::WkbType::MultiPolygon ) );
    setCurrentGeometryTypeFromString( mSourceParts.value( QStringLiteral( "geometryType" ) ).toString() );
  }
  else if ( geometryTypeForEntity == Qgis::GeometryType::Null && mComboGeometryType->findData( QVariant::fromValue( Qgis::WkbType::NoGeometry ) ) < 0 )
  {
    mComboGeometryType->clear();
    mComboGeometryType->addItem( QgsIconUtils::iconForWkbType( Qgis::WkbType::NoGeometry ), tr( "No Geometry" ), QVariant::fromValue( Qgis::WkbType::NoGeometry ) );
  }
  else if ( ( geometryTypeForEntity != Qgis::GeometryType::Null && geometryTypeForEntity != Qgis::GeometryType::Unknown )
            && mComboGeometryType->findData( QVariant::fromValue( geometryTypeForEntity ) ) < 0 )
  {
    mComboGeometryType->clear();
    switch ( geometryTypeForEntity )
    {
      case Qgis::GeometryType::Point:
        mComboGeometryType->addItem( QgsIconUtils::iconForWkbType( Qgis::WkbType::Point ), tr( "Point" ), QVariant::fromValue( Qgis::WkbType::Point ) );
        mComboGeometryType->addItem( QgsIconUtils::iconForWkbType( Qgis::WkbType::MultiPoint ), tr( "Multipoint" ), QVariant::fromValue( Qgis::WkbType::MultiPoint ) );
        break;
      case Qgis::GeometryType::Line:
        mComboGeometryType->addItem( QgsIconUtils::iconForWkbType( Qgis::WkbType::MultiLineString ), tr( "Line" ), QVariant::fromValue( Qgis::WkbType::MultiLineString ) );
        break;

      case Qgis::GeometryType::Polygon:
        mComboGeometryType->addItem( QgsIconUtils::iconForWkbType( Qgis::WkbType::MultiPolygon ), tr( "Polygon" ), QVariant::fromValue( Qgis::WkbType::MultiPolygon ) );
        break;

      case Qgis::GeometryType::Unknown:
      case Qgis::GeometryType::Null:
        break;
    }
    // we always add a "no geometry" option here, as some services don't correctly respect the mandated geometry types for eg MultiDatastreams
    mComboGeometryType->addItem( QgsIconUtils::iconForWkbType( Qgis::WkbType::NoGeometry ), tr( "No Geometry" ), QVariant::fromValue( Qgis::WkbType::NoGeometry ) );
    setCurrentGeometryTypeFromString( mSourceParts.value( QStringLiteral( "geometryType" ) ).toString() ); mComboGeometryType->setCurrentIndex( 0 );
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
  else if ( geometryType.isEmpty() && mComboGeometryType->findData( QVariant::fromValue( Qgis::WkbType::NoGeometry ) ) >= 0 )
  {
    mComboGeometryType->setCurrentIndex( mComboGeometryType->findData( QVariant::fromValue( Qgis::WkbType::NoGeometry ) ) );
  }
  else if ( geometryType.isEmpty() && mComboGeometryType->currentIndex() < 0 )
  {
    mComboGeometryType->setCurrentIndex( 0 );
  }
}


///@endcond
