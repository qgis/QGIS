/***************************************************************************
                             qgsfieldsitem.cpp
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfieldsitem.h"

#include <memory>

#include "qgsapplication.h"
#include "qgsfieldmodel.h"
#include "qgsiconutils.h"
#include "qgslogger.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgsvectorlayer.h"

#include "moc_qgsfieldsitem.cpp"

QgsFieldsItem::QgsFieldsItem( QgsDataItem *parent,
                              const QString &path,
                              const QString &connectionUri,
                              const QString &providerKey,
                              const QString &schema,
                              const QString &tableName )
  : QgsDataItem( Qgis::BrowserItemType::Fields, parent, tr( "Fields" ), path, providerKey )
  , mSchema( schema )
  , mTableName( tableName )
  , mConnectionUri( connectionUri )
{
  mCapabilities |= ( Qgis::BrowserItemCapability::Fertile | Qgis::BrowserItemCapability::Collapse | Qgis::BrowserItemCapability::RefreshChildrenWhenItemIsRefreshed );
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( providerKey ) };
  if ( md )
  {
    try
    {
      std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( mConnectionUri, {} ) ) };
      mTableProperty = std::make_unique<QgsAbstractDatabaseProviderConnection::TableProperty>( conn->table( schema, tableName ) );
      if ( conn->capabilities() & QgsAbstractDatabaseProviderConnection::Capability::RenameField )
      {
        mCanRename = true;
      }
    }
    catch ( QgsProviderConnectionException &ex )
    {
      QgsDebugError( u"Error creating fields item: %1"_s.arg( ex.what() ) );
    }
  }
}

QgsFieldsItem::~QgsFieldsItem()
{

}

QVector<QgsDataItem *> QgsFieldsItem::createChildren()
{
  QVector<QgsDataItem *> children;
  try
  {
    QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( providerKey() ) };
    if ( md )
    {
      std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( mConnectionUri, {} ) ) };
      if ( conn )
      {
        int i = 0;
        const QgsFields constFields { conn->fields( mSchema, mTableName ) };
        for ( const auto &f : constFields )
        {
          QgsFieldItem *fieldItem { new QgsFieldItem( this, f ) };
          fieldItem->setSortKey( i++ );
          children.push_back( fieldItem );
        }
      }
    }
  }
  catch ( const QgsProviderConnectionException &ex )
  {
    children.push_back( new QgsErrorItem( this, ex.what(), path() + u"/error"_s ) );
  }
  return children;
}

QIcon QgsFieldsItem::icon()
{
  return QgsApplication::getThemeIcon( u"mSourceFields.svg"_s );
}

QString QgsFieldsItem::connectionUri() const
{
  return mConnectionUri;
}

QgsVectorLayer *QgsFieldsItem::layer()
{
  std::unique_ptr<QgsVectorLayer> vl;
  QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( providerKey() ) };
  if ( md )
  {
    try
    {
      std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn { static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( mConnectionUri, {} ) ) };
      if ( conn )
      {
        vl = std::make_unique<QgsVectorLayer>( conn->tableUri( mSchema, mTableName ), u"temp_layer"_s, providerKey() );
        if ( vl->isValid() )
        {
          return vl.release();
        }
      }
    }
    catch ( const QgsProviderConnectionException & )
    {
      // This should never happen!
      QgsDebugError( u"Error getting connection from %1"_s.arg( mConnectionUri ) );
    }
  }
  else
  {
    // This should never happen!
    QgsDebugError( u"Error getting metadata for provider %1"_s.arg( providerKey() ) );
  }
  return nullptr;
}

QgsAbstractDatabaseProviderConnection::TableProperty *QgsFieldsItem::tableProperty() const
{
  return mTableProperty.get();
}

QString QgsFieldsItem::tableName() const
{
  return mTableName;
}

QString QgsFieldsItem::schema() const
{
  return mSchema;
}

QgsFieldItem::QgsFieldItem( QgsDataItem *parent, const QgsField &field )
  : QgsDataItem( Qgis::BrowserItemType::Field, parent, field.name(), parent->path() + '/' + field.name(), parent->providerKey() )
  , mField( field )
{
  // Precondition
  QgsFieldsItem *fieldsItem = qgis::down_cast<QgsFieldsItem *>( parent );
  Q_ASSERT( fieldsItem );

  if ( fieldsItem->canRenameFields() )
    mCapabilities |= Qgis::BrowserItemCapability::Rename;

  setState( Qgis::BrowserItemState::Populated );

  setToolTip( QgsFieldModel::fieldToolTip( field ) );
}

QgsFieldItem::~QgsFieldItem()
{
}

QIcon QgsFieldItem::icon()
{
  // Check if this is a geometry column and show the right icon
  QgsFieldsItem *parentFields { static_cast<QgsFieldsItem *>( parent() ) };
  if ( parentFields && parentFields->tableProperty() &&
       parentFields->tableProperty()->geometryColumn() == mName &&
       !parentFields->tableProperty()->geometryColumnTypes().isEmpty() )
  {
    if ( mField.typeName() == "raster"_L1 )
    {
      return QgsIconUtils::iconRaster();
    }
    const Qgis::GeometryType geomType { QgsWkbTypes::geometryType( parentFields->tableProperty()->geometryColumnTypes().first().wkbType ) };
    switch ( geomType )
    {
      case Qgis::GeometryType::Line:
        return QgsIconUtils::iconLine();
      case Qgis::GeometryType::Point:
        return QgsIconUtils::iconPoint();
      case Qgis::GeometryType::Polygon:
        return QgsIconUtils::iconPolygon();
      case Qgis::GeometryType::Unknown:
        return QgsIconUtils::iconGeometryCollection();
      case Qgis::GeometryType::Null:
        return QgsIconUtils::iconDefaultLayer();
    }
  }
  const QIcon icon { QgsFields::iconForFieldType( mField.type(), mField.subType(), mField.typeName() ) };
  // Try subtype if icon is null
  if ( icon.isNull() )
  {
    return QgsFields::iconForFieldType( mField.subType() );
  }
  return icon;
}

bool QgsFieldItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }

  const QgsFieldItem *o = qobject_cast<const QgsFieldItem *>( other );
  if ( !o )
    return false;

  return ( mPath == o->mPath && mName == o->mName && mField == o->mField && mField.comment() == o->mField.comment() );
}

