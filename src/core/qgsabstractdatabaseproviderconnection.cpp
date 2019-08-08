/***************************************************************************
  qgsabstractdatabaseproviderconnection.cpp - QgsAbstractDatabaseProviderConnection

 ---------------------
 begin                : 2.8.2019
 copyright            : (C) 2019 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsexception.h"
#include <QVariant>
#include <QObject>

QgsAbstractDatabaseProviderConnection::QgsAbstractDatabaseProviderConnection( const QString &name ):
  QgsAbstractProviderConnection( name )
{

}

QgsAbstractDatabaseProviderConnection::QgsAbstractDatabaseProviderConnection( const QString &name, const QString &uri ):
  QgsAbstractProviderConnection( name, uri )
{

}
QgsAbstractDatabaseProviderConnection::Capabilities QgsAbstractDatabaseProviderConnection::capabilities() const
{
  return mCapabilities;
}

void QgsAbstractDatabaseProviderConnection::checkCapability( QgsAbstractDatabaseProviderConnection::Capability cap )
{
  if ( ! mCapabilities & cap )
  {
    static QMetaEnum metaEnum = QMetaEnum::fromType<QgsAbstractDatabaseProviderConnection::Capability>();
    const QString capName { metaEnum.valueToKey( cap ) };
    throw QgsProviderConnectionException( QObject::tr( "Operation '%1' is not supported for this connection" ).arg( capName ) );
  }
}

void QgsAbstractDatabaseProviderConnection::createVectorTable( const QString &schema,
    const QString &name,
    const QgsFields &fields,
    QgsWkbTypes::Type wkbType,
    const QgsCoordinateReferenceSystem &srs,
    bool overwrite,
    const QMap<QString, QVariant> *
    options )
{
  Q_UNUSED( schema );
  Q_UNUSED( name );
  Q_UNUSED( fields );
  Q_UNUSED( srs );
  Q_UNUSED( overwrite );
  Q_UNUSED( options );
  Q_UNUSED( wkbType );
  throw QgsProviderConnectionException( QObject::tr( "Operation 'createVectorTable' is not supported" ) );
}

void QgsAbstractDatabaseProviderConnection::renameTable( const QString &, const QString &, const QString & )
{
  checkCapability( Capability::RenameTable );
}

void QgsAbstractDatabaseProviderConnection::dropVectorTable( const QString &, const QString & )
{
  checkCapability( Capability::DropVectorTable );
}

void QgsAbstractDatabaseProviderConnection::dropRasterTable( const QString &, const QString & )
{
  checkCapability( Capability::DropRasterTable );
}

void QgsAbstractDatabaseProviderConnection::createSchema( const QString & )
{
  checkCapability( Capability::CreateSchema );
}

void QgsAbstractDatabaseProviderConnection::dropSchema( const QString &, bool )
{
  checkCapability( Capability::DropSchema );
}

void QgsAbstractDatabaseProviderConnection::renameSchema( const QString &, const QString & )
{
  checkCapability( Capability::RenameSchema );
}

void QgsAbstractDatabaseProviderConnection::executeSql( const QString & )
{
  checkCapability( Capability::ExecuteSql );
}

void QgsAbstractDatabaseProviderConnection::vacuum( const QString &, const QString & )
{
  checkCapability( Capability::Vacuum );
}

QList<QgsAbstractDatabaseProviderConnection::TableProperty> QgsAbstractDatabaseProviderConnection::tables( const QString &, const QgsAbstractDatabaseProviderConnection::TableFlags & )
{
  throw QgsProviderConnectionException( QObject::tr( "Operation 'tables' is not supported" ) );
}

QStringList QgsAbstractDatabaseProviderConnection::schemas( )
{
  throw QgsProviderConnectionException( QObject::tr( "Operation 'schemas' is not supported" ) );
}

void QgsAbstractDatabaseProviderConnection::TableProperty::appendGeometryColumnType( const QgsWkbTypes::Type &type )
{
  geometryColumnTypes.push_back( type );
}

QList<int> QgsAbstractDatabaseProviderConnection::TableProperty::geometryTypes()
{
  QList<int> types;
  for ( const auto &t : qgis::as_const( geometryColumnTypes ) )
  {
    types.push_back( static_cast<int>( t ) );
  }
  return types;
}

int QgsAbstractDatabaseProviderConnection::TableProperty::layerTypeCount() const
{
  Q_ASSERT( geometryColumnTypes.size() == srids.size() );
  return geometryColumnTypes.size();
}

QString QgsAbstractDatabaseProviderConnection::TableProperty::defaultName() const
{
  QString n = name;
  if ( geometryColumnCount > 1 ) n += '.' + geometryColumn;
  return n;
}

QgsAbstractDatabaseProviderConnection::TableProperty QgsAbstractDatabaseProviderConnection::TableProperty::at( int i ) const
{
  TableProperty property;

  Q_ASSERT( i >= 0 && i < layerTypeCount() );

  property.geometryColumnTypes << geometryColumnTypes[ i ];
  property.srids << srids[ i ];
  property.schema = schema;
  property.name = name;
  property.geometryColumn = geometryColumn;
  property.pkColumns = pkColumns;
  property.geometryColumnCount = geometryColumnCount;
  property.sql = sql;
  property.tableComment = tableComment;
  property.flags = flags;
  return property;
}
