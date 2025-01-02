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
#include "moc_qgssensorthingssourcewidget.cpp"
///@cond PRIVATE

#include "qgsproviderregistry.h"
#include "qgssensorthingsutils.h"
#include "qgssensorthingsprovider.h"
#include "qgsiconutils.h"
#include "qgssensorthingsconnectionpropertiestask.h"
#include "qgssensorthingssubseteditor.h"
#include "qgsapplication.h"
#include "qgsextentwidget.h"
#include "qgsspinbox.h"
#include "qgsstringutils.h"
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QStandardItemModel>
#include <QDialogButtonBox>
#include <QTableView>

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

  mExpansionsModel = new QgsSensorThingsExpansionsModel( this );
  mExpansionsTable->setModel( mExpansionsModel );

  mExpansionsTableDelegate = new QgsSensorThingsExpansionsDelegate( mExpansionsTable );
  mExpansionsTable->setItemDelegateForColumn( QgsSensorThingsExpansionsModel::Column::Entity, mExpansionsTableDelegate );
  mExpansionsTable->setItemDelegateForColumn( QgsSensorThingsExpansionsModel::Column::Limit, mExpansionsTableDelegate );
  mExpansionsTable->setItemDelegateForColumn( QgsSensorThingsExpansionsModel::Column::OrderBy, mExpansionsTableDelegate );
  mExpansionsTable->setItemDelegateForColumn( QgsSensorThingsExpansionsModel::Column::SortOrder, mExpansionsTableDelegate );
  mExpansionsTable->setItemDelegateForColumn( QgsSensorThingsExpansionsModel::Column::Filter, mExpansionsTableDelegate );

  QgsSensorThingsRemoveExpansionDelegate *removeDelegate = new QgsSensorThingsRemoveExpansionDelegate( mExpansionsTable );
  mExpansionsTable->setItemDelegateForColumn( QgsSensorThingsExpansionsModel::Column::Actions, removeDelegate );
  mExpansionsTable->viewport()->installEventFilter( removeDelegate );
  connect( mExpansionsTable, &QTableView::clicked, this, [this]( const QModelIndex &index ) {
    if ( index.column() == QgsSensorThingsExpansionsModel::Column::Actions )
    {
      mExpansionsModel->removeRows( index.row(), 1 );
    }
  } );

  mExpansionsTable->setEditTriggers( QAbstractItemView::AllEditTriggers );
  mExpansionsTable->verticalHeader()->hide();
  const QFontMetrics fm( font() );
  mExpansionsTable->horizontalHeader()->resizeSection( QgsSensorThingsExpansionsModel::Column::Entity, fm.horizontalAdvance( '0' ) * 30 );
  mExpansionsTable->horizontalHeader()->resizeSection( QgsSensorThingsExpansionsModel::Column::Limit, fm.horizontalAdvance( '0' ) * 15 );
  mExpansionsTable->horizontalHeader()->resizeSection( QgsSensorThingsExpansionsModel::Column::OrderBy, fm.horizontalAdvance( '0' ) * 30 );
  mExpansionsTable->horizontalHeader()->resizeSection( QgsSensorThingsExpansionsModel::Column::Filter, fm.horizontalAdvance( '0' ) * 20 );
  mExpansionsTable->horizontalHeader()->resizeSection( QgsSensorThingsExpansionsModel::Column::SortOrder, fm.horizontalAdvance( '0' ) * 15 );
  mExpansionsTable->horizontalHeader()->resizeSection( QgsSensorThingsExpansionsModel::Column::Actions, fm.horizontalAdvance( '0' ) * 5 );

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

  setCurrentEntityType( Qgis::SensorThingsEntity::Location );

  connect( mComboEntityType, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsSensorThingsSourceWidget::entityTypeChanged );
  connect( mComboGeometryType, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsSensorThingsSourceWidget::validate );
  connect( mSpinPageSize, qOverload<int>( &QSpinBox::valueChanged ), this, &QgsSensorThingsSourceWidget::validate );
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

  setCurrentEntityType( mComboEntityType->currentData().value<Qgis::SensorThingsEntity>() );
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

  const QgsRectangle bounds = mSourceParts.value( QStringLiteral( "bounds" ) ).value<QgsRectangle>();
  if ( !bounds.isNull() )
  {
    mExtentWidget->setCurrentExtent( bounds, QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
    mExtentWidget->setOutputExtentFromUser( bounds, QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  }
  else
  {
    mExtentWidget->clear();
  }

  const QVariantList expandTo = mSourceParts.value( QStringLiteral( "expandTo" ) ).toList();
  QList<QgsSensorThingsExpansionDefinition> expansions;
  QStringList expansionsLabelText;
  for ( const QVariant &expandVariant : expandTo )
  {
    const QgsSensorThingsExpansionDefinition definition = expandVariant.value<QgsSensorThingsExpansionDefinition>();
    if ( definition.isValid() )
    {
      expansions.append( definition );
      expansionsLabelText.append( QgsSensorThingsUtils::displayString( definition.childEntity() ) );
    }
  }
  mExpansionsModel->setExpansions( expansions );

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
  return mComboEntityType->currentData().value<Qgis::SensorThingsEntity>();
}

QString QgsSensorThingsSourceWidget::updateUriFromGui( const QString &connectionUri ) const
{
  QVariantMap parts = QgsProviderRegistry::instance()->decodeUri(
    QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY,
    connectionUri
  );

  const Qgis::SensorThingsEntity entityType = mComboEntityType->currentData().value<Qgis::SensorThingsEntity>();
  parts.insert( QStringLiteral( "entity" ), qgsEnumValueToKey( entityType ) );
  if ( !QgsSensorThingsUtils::entityTypeHasGeometry( entityType ) )
  {
    parts.remove( QStringLiteral( "geometryType" ) );
  }
  else
  {
    const Qgis::WkbType newWkbType = mComboGeometryType->currentData().value<Qgis::WkbType>();
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

  const QList<QgsSensorThingsExpansionDefinition> expansions = mExpansionsModel->expansions();
  if ( expansions.isEmpty() )
  {
    parts.remove( QStringLiteral( "expandTo" ) );
  }
  else
  {
    QVariantList expansionsList;
    for ( const QgsSensorThingsExpansionDefinition &def : std::as_const( expansions ) )
    {
      expansionsList.append( QVariant::fromValue( def ) );
    }
    parts.insert( QStringLiteral( "expandTo" ), expansionsList );
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
  const Qgis::SensorThingsEntity entityType = mComboEntityType->currentData().value<Qgis::SensorThingsEntity>();
  setCurrentEntityType( entityType );

  validate();
}

void QgsSensorThingsSourceWidget::validate()
{
  bool valid = mComboEntityType->currentIndex() >= 0;

  const Qgis::SensorThingsEntity entityType = mComboEntityType->currentData().value<Qgis::SensorThingsEntity>();
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

  mPropertiesTask = new QgsSensorThingsConnectionPropertiesTask( mSourceParts.value( QStringLiteral( "url" ) ).toString(), mComboEntityType->currentData().value<Qgis::SensorThingsEntity>() );
  connect( mPropertiesTask, &QgsTask::taskCompleted, this, &QgsSensorThingsSourceWidget::connectionPropertiesTaskCompleted );
  QgsApplication::taskManager()->addTask( mPropertiesTask );
  mRetrieveTypesButton->setEnabled( false );
}

void QgsSensorThingsSourceWidget::connectionPropertiesTaskCompleted()
{
  const QList<Qgis::GeometryType> availableTypes = mPropertiesTask->geometryTypes();
  const Qgis::WkbType currentWkbType = mComboGeometryType->currentData().value<Qgis::WkbType>();
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

void QgsSensorThingsSourceWidget::setCurrentEntityType( Qgis::SensorThingsEntity type )
{
  mExpansionsTableDelegate->setBaseEntityType( type );
  mExpansionsModel->setExpansions( {} );

  if ( mPropertiesTask )
  {
    disconnect( mPropertiesTask, &QgsTask::taskCompleted, this, &QgsSensorThingsSourceWidget::connectionPropertiesTaskCompleted );
    mPropertiesTask->cancel();
    mPropertiesTask = nullptr;
  }

  mRetrieveTypesButton->setEnabled( QgsSensorThingsUtils::geometryTypeForEntity( type ) == Qgis::GeometryType::Unknown && !mSourceParts.value( QStringLiteral( "url" ) ).toString().isEmpty() );
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
    setCurrentGeometryTypeFromString( mSourceParts.value( QStringLiteral( "geometryType" ) ).toString() );
    mComboGeometryType->setCurrentIndex( 0 );
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

//
// QgsSensorThingsExpansionsModel
//

QgsSensorThingsExpansionsModel::QgsSensorThingsExpansionsModel( QObject *parent )
  : QAbstractItemModel( parent )
{
}

int QgsSensorThingsExpansionsModel::columnCount( const QModelIndex & ) const
{
  return 6;
}

int QgsSensorThingsExpansionsModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;
  return mExpansions.size();
}

QModelIndex QgsSensorThingsExpansionsModel::index( int row, int column, const QModelIndex &parent ) const
{
  if ( hasIndex( row, column, parent ) )
  {
    return createIndex( row, column, row );
  }

  return QModelIndex();
}

QModelIndex QgsSensorThingsExpansionsModel::parent( const QModelIndex &child ) const
{
  Q_UNUSED( child )
  return QModelIndex();
}

Qt::ItemFlags QgsSensorThingsExpansionsModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::ItemFlags();

  if ( index.row() < 0 || index.row() >= mExpansions.size() || index.column() < 0 || index.column() >= columnCount() )
    return Qt::ItemFlags();

  switch ( index.column() )
  {
    case Column::Entity:
    case Column::Limit:
    case Column::OrderBy:
    case Column::SortOrder:
    case Column::Filter:
      return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsEditable | Qt::ItemFlag::ItemIsSelectable;
    case Column::Actions:
      return Qt::ItemFlag::ItemIsEnabled;
    default:
      break;
  }

  return Qt::ItemFlags();
}

QVariant QgsSensorThingsExpansionsModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  if ( index.row() < 0 || index.row() >= mExpansions.size() || index.column() < 0 || index.column() >= columnCount() )
    return QVariant();

  const QgsSensorThingsExpansionDefinition &expansion = mExpansions.at( index.row() );

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    {
      switch ( index.column() )
      {
        case Column::Entity:
          return QgsSensorThingsUtils::displayString( expansion.childEntity() );

        case Column::Limit:
          return !expansion.isValid() ? QVariant() : ( expansion.limit() > -1 ? QVariant( expansion.limit() ) : QVariant( tr( "No limit" ) ) );

        case Column::OrderBy:
          return !expansion.isValid() ? QVariant() : ( expansion.orderBy().isEmpty() ? QVariant() : expansion.orderBy() );

        case Column::SortOrder:
          return !expansion.isValid() ? QVariant() : ( expansion.sortOrder() == Qt::SortOrder::AscendingOrder ? tr( "Ascending" ) : tr( "Descending" ) );

        case Column::Filter:
          return !expansion.isValid() ? QVariant() : ( expansion.filter().isEmpty() ? QVariant() : expansion.filter() );

        case Column::Actions:
          return role == Qt::ToolTipRole ? tr( "Remove expansion" ) : QString();

        default:
          break;
      }
      break;
    }

    case Qt::EditRole:
    {
      switch ( index.column() )
      {
        case Column::Entity:
          return QVariant::fromValue( expansion.childEntity() );

        case Column::Limit:
          return expansion.limit();

        case Column::OrderBy:
          return expansion.orderBy().isEmpty() ? QVariant() : expansion.orderBy();

        case Column::Filter:
          return expansion.filter().isEmpty() ? QVariant() : expansion.filter();

        case Column::SortOrder:
          return expansion.sortOrder();

        default:
          break;
      }
      break;
    }

    case Qt::TextAlignmentRole:
    {
      switch ( index.column() )
      {
        case Column::Entity:
        case Column::OrderBy:
        case Column::SortOrder:
        case Column::Filter:
          return static_cast<Qt::Alignment::Int>( Qt::AlignLeft | Qt::AlignVCenter );

        case Column::Limit:
          return static_cast<Qt::Alignment::Int>( Qt::AlignRight | Qt::AlignVCenter );
        default:
          break;
      }
      break;
    }

    default:
      break;
  }
  return QVariant();
}

QVariant QgsSensorThingsExpansionsModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  if ( role == Qt::DisplayRole && orientation == Qt::Horizontal )
  {
    switch ( section )
    {
      case Column::Entity:
        return tr( "Entity" );
      case Column::Limit:
        return tr( "Limit" );
      case Column::OrderBy:
        return tr( "Order By" );
      case Column::SortOrder:
        return tr( "Sort Order" );
      case Column::Filter:
        return tr( "Filter" );
      case Column::Actions:
        return QString();
      default:
        break;
    }
  }
  return QAbstractItemModel::headerData( section, orientation, role );
}

bool QgsSensorThingsExpansionsModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  if ( index.row() > mExpansions.size() || index.row() < 0 )
    return false;

  QgsSensorThingsExpansionDefinition &expansion = mExpansions[index.row()];

  switch ( role )
  {
    case Qt::EditRole:
    {
      switch ( index.column() )
      {
        case Column::Entity:
        {
          const bool wasInvalid = !expansion.isValid();
          if ( !value.isValid() || value.value<Qgis::SensorThingsEntity>() == Qgis::SensorThingsEntity::Invalid )
          {
            if ( wasInvalid )
              break;
          }
          else
          {
            expansion.setChildEntity( value.value<Qgis::SensorThingsEntity>() );
            if ( wasInvalid )
            {
              expansion = QgsSensorThingsExpansionDefinition::defaultDefinitionForEntity( expansion.childEntity() );
              emit dataChanged( createIndex( index.row(), 0 ), createIndex( index.row(), columnCount() ) );
            }
          }
          emit dataChanged( index, index, QVector<int>() << role );
          if ( wasInvalid )
          {
            beginInsertRows( QModelIndex(), mExpansions.size(), mExpansions.size() );
            mExpansions.append( QgsSensorThingsExpansionDefinition() );
            endInsertRows();
          }
          break;
        }

        case Column::Limit:
        {
          bool ok = false;
          int newValue = value.toInt( &ok );
          if ( !ok )
            return false;

          expansion.setLimit( newValue );
          emit dataChanged( index, index, QVector<int>() << role );
          break;
        }

        case Column::OrderBy:
        {
          expansion.setOrderBy( value.toString() );
          emit dataChanged( index, index, QVector<int>() << role );
          break;
        }

        case Column::Filter:
        {
          expansion.setFilter( value.toString() );
          emit dataChanged( index, index, QVector<int>() << role );
          break;
        }

        case Column::SortOrder:
        {
          expansion.setSortOrder( value.value<Qt::SortOrder>() );
          emit dataChanged( index, index, QVector<int>() << role );
          break;
        }

        default:
          break;
      }
      return true;
    }

    default:
      break;
  }

  return false;
}

bool QgsSensorThingsExpansionsModel::insertRows( int position, int rows, const QModelIndex &parent )
{
  Q_UNUSED( parent )
  beginInsertRows( QModelIndex(), position, position + rows - 1 );
  for ( int i = 0; i < rows; ++i )
  {
    mExpansions.insert( position, QgsSensorThingsExpansionDefinition() );
  }
  endInsertRows();
  return true;
}

bool QgsSensorThingsExpansionsModel::removeRows( int position, int rows, const QModelIndex &parent )
{
  Q_UNUSED( parent )
  beginRemoveRows( QModelIndex(), position, position + rows - 1 );
  for ( int i = 0; i < rows; ++i )
    mExpansions.removeAt( position );
  endRemoveRows();

  if ( mExpansions.empty() )
  {
    beginInsertRows( QModelIndex(), 0, 0 );
    mExpansions.append( QgsSensorThingsExpansionDefinition() );
    endInsertRows();
  }
  return true;
}

void QgsSensorThingsExpansionsModel::setExpansions( const QList<QgsSensorThingsExpansionDefinition> &expansions )
{
  beginResetModel();
  mExpansions = expansions;
  // last entry should always be a blank entry
  if ( mExpansions.isEmpty() || mExpansions.last().isValid() )
    mExpansions.append( QgsSensorThingsExpansionDefinition() );
  endResetModel();
}

//
// QgsSensorThingsExpansionsDelegate
//

QgsSensorThingsExpansionsDelegate::QgsSensorThingsExpansionsDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{
}

void QgsSensorThingsExpansionsDelegate::setBaseEntityType( Qgis::SensorThingsEntity type )
{
  mBaseEntityType = type;
}

QWidget *QgsSensorThingsExpansionsDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index ) const
{
  switch ( index.column() )
  {
    case QgsSensorThingsExpansionsModel::Column::Entity:
    {
      // need to find out entity type for the previous row (or the base entity type for the first row)
      const Qgis::SensorThingsEntity entityType = index.row() == 0 ? mBaseEntityType
                                                                   : index.model()->data( index.model()->index( index.row() - 1, 0 ), Qt::EditRole ).value<Qgis::SensorThingsEntity>();

      QList<Qgis::SensorThingsEntity> compatibleEntities = QgsSensorThingsUtils::expandableTargets( entityType );
      // remove all entities which are already part of the expansion in previous rows -- we don't support "circular" expansion
      for ( int row = index.row() - 1; row >= 0; row-- )
      {
        const Qgis::SensorThingsEntity rowEntityType = index.model()->data( index.model()->index( row, 0 ), Qt::EditRole ).value<Qgis::SensorThingsEntity>();
        compatibleEntities.removeAll( rowEntityType );
      }

      QComboBox *combo = new QComboBox( parent );
      for ( Qgis::SensorThingsEntity type : compatibleEntities )
      {
        combo->addItem( QgsSensorThingsUtils::displayString( type, true ), QVariant::fromValue( type ) );
      }
      return combo;
    }

    case QgsSensorThingsExpansionsModel::Column::Limit:
    {
      QgsSpinBox *spin = new QgsSpinBox( parent );
      spin->setMinimum( -1 );
      spin->setMaximum( 999999999 );
      spin->setClearValue( -1, tr( "No limit" ) );
      return spin;
    }

    case QgsSensorThingsExpansionsModel::Column::OrderBy:
    {
      // need to find out entity type for this row
      const Qgis::SensorThingsEntity entityType = index.model()->data( index.model()->index( index.row(), 0 ), Qt::EditRole ).value<Qgis::SensorThingsEntity>();
      const QStringList availableProperties = QgsSensorThingsUtils::propertiesForEntityType( entityType );

      QComboBox *combo = new QComboBox( parent );
      combo->addItem( QString() );
      for ( const QString &property : availableProperties )
      {
        combo->addItem( property, property );
      }
      return combo;
    }

    case QgsSensorThingsExpansionsModel::Column::SortOrder:
    {
      QComboBox *combo = new QComboBox( parent );
      combo->addItem( tr( "Ascending" ), QVariant::fromValue( Qt::SortOrder::AscendingOrder ) );
      combo->addItem( tr( "Descending" ), QVariant::fromValue( Qt::SortOrder::DescendingOrder ) );
      return combo;
    }

    case QgsSensorThingsExpansionsModel::Column::Filter:
    {
      // need to find out entity type for this row
      const Qgis::SensorThingsEntity entityType = index.model()->data( index.model()->index( index.row(), 0 ), Qt::EditRole ).value<Qgis::SensorThingsEntity>();
      QgsSensorThingsFilterWidget *w = new QgsSensorThingsFilterWidget( parent, entityType );
      connect( w, &QgsSensorThingsFilterWidget::filterChanged, this, [=]() {
        const_cast<QgsSensorThingsExpansionsDelegate *>( this )->emit commitData( w );
      } );
      return w;
    }

    default:
      break;
  }
  return nullptr;
}

void QgsSensorThingsExpansionsDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  switch ( index.column() )
  {
    case QgsSensorThingsExpansionsModel::Column::Entity:
    case QgsSensorThingsExpansionsModel::Column::SortOrder:
    case QgsSensorThingsExpansionsModel::Column::OrderBy:
    {
      if ( QComboBox *combo = qobject_cast<QComboBox *>( editor ) )
      {
        combo->setCurrentIndex( combo->findData( index.data( Qt::EditRole ) ) );
        if ( combo->currentIndex() < 0 )
          combo->setCurrentIndex( 0 );
      }
      return;
    }

    case QgsSensorThingsExpansionsModel::Column::Filter:
    {
      if ( QgsSensorThingsFilterWidget *w = qobject_cast<QgsSensorThingsFilterWidget *>( editor ) )
      {
        w->setFilter( index.data( Qt::EditRole ).toString() );
      }
      return;
    }

    default:
      break;
  }
  QStyledItemDelegate::setEditorData( editor, index );
}

void QgsSensorThingsExpansionsDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  switch ( index.column() )
  {
    case QgsSensorThingsExpansionsModel::Column::Entity:
    {
      if ( QComboBox *combo = qobject_cast<QComboBox *>( editor ) )
      {
        model->setData( index, combo->currentData() );
      }
      break;
    }

    case QgsSensorThingsExpansionsModel::Column::Limit:
    {
      if ( QgsSpinBox *spin = qobject_cast<QgsSpinBox *>( editor ) )
      {
        model->setData( index, spin->value() );
      }
      break;
    }

    case QgsSensorThingsExpansionsModel::Column::OrderBy:
    {
      if ( QComboBox *combo = qobject_cast<QComboBox *>( editor ) )
      {
        model->setData( index, combo->currentData() );
      }
      break;
    }

    case QgsSensorThingsExpansionsModel::Column::SortOrder:
      if ( QComboBox *combo = qobject_cast<QComboBox *>( editor ) )
      {
        model->setData( index, combo->currentData() );
      }
      break;

    case QgsSensorThingsExpansionsModel::Column::Filter:
    {
      if ( QgsSensorThingsFilterWidget *w = qobject_cast<QgsSensorThingsFilterWidget *>( editor ) )
      {
        model->setData( index, w->filter() );
      }
      break;
    }

    default:
      break;
  }
}


//
// QgsSensorThingsRemoveExpansionDelegate
//

QgsSensorThingsRemoveExpansionDelegate::QgsSensorThingsRemoveExpansionDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{
}

bool QgsSensorThingsRemoveExpansionDelegate::eventFilter( QObject *obj, QEvent *event )
{
  if ( event->type() == QEvent::HoverEnter || event->type() == QEvent::HoverMove )
  {
    QHoverEvent *hoverEvent = static_cast<QHoverEvent *>( event );
    if ( QAbstractItemView *view = qobject_cast<QAbstractItemView *>( obj->parent() ) )
    {
      const QModelIndex indexUnderMouse = view->indexAt( hoverEvent->pos() );
      setHoveredIndex( indexUnderMouse );
      view->viewport()->update();
    }
  }
  else if ( event->type() == QEvent::HoverLeave )
  {
    setHoveredIndex( QModelIndex() );
    qobject_cast<QWidget *>( obj )->update();
  }
  return QStyledItemDelegate::eventFilter( obj, event );
}

void QgsSensorThingsRemoveExpansionDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QStyledItemDelegate::paint( painter, option, index );

  if ( index == mHoveredIndex )
  {
    QStyleOptionButton buttonOption;
    buttonOption.initFrom( option.widget );
    buttonOption.rect = option.rect;

    option.widget->style()->drawControl( QStyle::CE_PushButton, &buttonOption, painter );
  }

  const QIcon icon = QgsApplication::getThemeIcon( "/mIconClearItem.svg" );
  const QRect iconRect( option.rect.left() + ( option.rect.width() - 16 ) / 2, option.rect.top() + ( option.rect.height() - 16 ) / 2, 16, 16 );

  icon.paint( painter, iconRect );
}

void QgsSensorThingsRemoveExpansionDelegate::setHoveredIndex( const QModelIndex &index )
{
  mHoveredIndex = index;
}


//
// QgsSensorThingsFilterWidget
//

QgsSensorThingsFilterWidget::QgsSensorThingsFilterWidget( QWidget *parent, Qgis::SensorThingsEntity entity )
  : QWidget( parent )
  , mEntity( entity )
{
  QHBoxLayout *hl = new QHBoxLayout();
  hl->setContentsMargins( 0, 0, 0, 0 );
  hl->addStretch( 1 );
  QToolButton *button = new QToolButton();
  connect( button, &QToolButton::clicked, this, &QgsSensorThingsFilterWidget::setQuery );
  button->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFilter2.svg" ) ) );
  hl->addWidget( button );
  setLayout( hl );
}

void QgsSensorThingsFilterWidget::setFilter( const QString &filter )
{
  mFilter = filter;
}

QString QgsSensorThingsFilterWidget::filter() const
{
  return mFilter;
}

void QgsSensorThingsFilterWidget::setQuery()
{
  const QgsFields fields = QgsSensorThingsUtils::fieldsForEntityType( mEntity );
  QgsSensorThingsSubsetEditor editor( nullptr, fields, this );
  editor.setSubsetString( mFilter );
  if ( editor.exec() )
  {
    mFilter = editor.subsetString();
    emit filterChanged();
  }
}

///@endcond
