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

void QgsAbstractDatabaseProviderConnection::checkCapability( QgsAbstractDatabaseProviderConnection::Capability capability ) const
{
  if ( ! mCapabilities & capability )
  {
    static QMetaEnum metaEnum = QMetaEnum::fromType<QgsAbstractDatabaseProviderConnection::Capability>();
    const QString capName { metaEnum.valueToKey( capability ) };
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
    options ) const
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

void QgsAbstractDatabaseProviderConnection::renameTable( const QString &, const QString &, const QString & ) const
{
  checkCapability( Capability::RenameTable );
}

void QgsAbstractDatabaseProviderConnection::dropVectorTable( const QString &, const QString & ) const
{
  checkCapability( Capability::DropVectorTable );
}

bool QgsAbstractDatabaseProviderConnection::tableExists( const QString &schema, const QString &name ) const
{
  checkCapability( Capability::TableExists );
  const QList<QgsAbstractDatabaseProviderConnection::TableProperty> constTables { tables( schema ) };
  for ( const auto &t : constTables )
  {
    if ( t.tableName() == name )
    {
      return true;
    }
  }
  return false;
}

void QgsAbstractDatabaseProviderConnection::dropRasterTable( const QString &, const QString & ) const
{
  checkCapability( Capability::DropRasterTable );
}

void QgsAbstractDatabaseProviderConnection::createSchema( const QString & ) const
{
  checkCapability( Capability::CreateSchema );
}

void QgsAbstractDatabaseProviderConnection::dropSchema( const QString &, bool ) const
{
  checkCapability( Capability::DropSchema );
}

void QgsAbstractDatabaseProviderConnection::renameSchema( const QString &, const QString & ) const
{
  checkCapability( Capability::RenameSchema );
}

QList<QList<QVariant>> QgsAbstractDatabaseProviderConnection::executeSql( const QString & ) const
{
  checkCapability( Capability::ExecuteSql );
  return QList<QList<QVariant>>();
}

void QgsAbstractDatabaseProviderConnection::vacuum( const QString &, const QString & ) const
{
  checkCapability( Capability::Vacuum );
}

QList<QgsAbstractDatabaseProviderConnection::TableProperty> QgsAbstractDatabaseProviderConnection::tables( const QString &, const QgsAbstractDatabaseProviderConnection::TableFlags & ) const
{
  checkCapability( Capability::Tables );
  return QList<QgsAbstractDatabaseProviderConnection::TableProperty>();
}

QList<QgsAbstractDatabaseProviderConnection::TableProperty> QgsAbstractDatabaseProviderConnection::tablesInt( const QString &schema, const int flags ) const
{
  return tables( schema, static_cast<QgsAbstractDatabaseProviderConnection::TableFlags>( flags ) );
}


QStringList QgsAbstractDatabaseProviderConnection::schemas( ) const
{
  checkCapability( Capability::Schemas );
  return QStringList();
}

QString QgsAbstractDatabaseProviderConnection::TableProperty::tableName() const
{
  return mTableName;
}

void QgsAbstractDatabaseProviderConnection::TableProperty::setTableName( const QString &name )
{
  mTableName = name;
}

void QgsAbstractDatabaseProviderConnection::TableProperty::addGeometryType( const QgsWkbTypes::Type &type, const QgsCoordinateReferenceSystem &crs )
{
  mGeometryTypes.push_back( qMakePair<int, QgsCoordinateReferenceSystem>( static_cast<int>( type ), crs ) );
}

QList<QPair<int, QgsCoordinateReferenceSystem> > QgsAbstractDatabaseProviderConnection::TableProperty::geometryTypes() const
{
  return mGeometryTypes;
}


QString QgsAbstractDatabaseProviderConnection::TableProperty::defaultName() const
{
  QString n = mTableName;
  if ( mGeometryColumnCount > 1 ) n += '.' + mGeometryColumn;
  return n;
}

QgsAbstractDatabaseProviderConnection::TableProperty QgsAbstractDatabaseProviderConnection::TableProperty::at( int index ) const
{
  TableProperty property;

  Q_ASSERT( index >= 0 && index < geometryTypes().size() );

  property.mGeometryTypes << mGeometryTypes[ index ];
  property.mSchema = mSchema;
  property.mTableName = mTableName;
  property.mGeometryColumn = mGeometryColumn;
  property.mPkColumns = mPkColumns;
  property.mGeometryColumnCount = mGeometryColumnCount;
  property.mFlags = mFlags;
  property.mComment = mComment;
  property.mInfo = mInfo;
  return property;
}

void QgsAbstractDatabaseProviderConnection::TableProperty::setFlag( const QgsAbstractDatabaseProviderConnection::TableFlag &flag )
{
  mFlags.setFlag( flag );
}

void QgsAbstractDatabaseProviderConnection::TableProperty::setGeometryTypes( const QList<QPair<int, QgsCoordinateReferenceSystem> > &geometryTypes )
{
  mGeometryTypes = geometryTypes;
}


int QgsAbstractDatabaseProviderConnection::TableProperty::geometryColumnCount() const
{
  return mGeometryColumnCount;
}

void QgsAbstractDatabaseProviderConnection::TableProperty::setGeometryColumnCount( int geometryColumnCount )
{
  mGeometryColumnCount = geometryColumnCount;
}

QVariantMap QgsAbstractDatabaseProviderConnection::TableProperty::info() const
{
  return mInfo;
}

void QgsAbstractDatabaseProviderConnection::TableProperty::setInfo( const QVariantMap &info )
{
  mInfo = info;
}

QString QgsAbstractDatabaseProviderConnection::TableProperty::comment() const
{
  return mComment;
}

void QgsAbstractDatabaseProviderConnection::TableProperty::setComment( const QString &comment )
{
  mComment = comment;
}

QgsAbstractDatabaseProviderConnection::TableFlags QgsAbstractDatabaseProviderConnection::TableProperty::flags() const
{
  return mFlags;
}

void QgsAbstractDatabaseProviderConnection::TableProperty::setFlags( const QgsAbstractDatabaseProviderConnection::TableFlags &flags )
{
  mFlags = flags;
}

QList<QgsCoordinateReferenceSystem> QgsAbstractDatabaseProviderConnection::TableProperty::crsList() const
{
  QList<QgsCoordinateReferenceSystem> crss;
  for ( const auto &t : qgis::as_const( mGeometryTypes ) )
  {
    crss.push_back( t.second );
  }
  return crss;
}

QStringList QgsAbstractDatabaseProviderConnection::TableProperty::primaryKeyColumns() const
{
  return mPkColumns;
}

void QgsAbstractDatabaseProviderConnection::TableProperty::setPrimaryKeyColumns( const QStringList &pkColumns )
{
  mPkColumns = pkColumns;
}

QString QgsAbstractDatabaseProviderConnection::TableProperty::geometryColumn() const
{
  return mGeometryColumn;
}

void QgsAbstractDatabaseProviderConnection::TableProperty::setGeometryColumn( const QString &geometryColumn )
{
  mGeometryColumn = geometryColumn;
}

QString QgsAbstractDatabaseProviderConnection::TableProperty::schema() const
{
  return mSchema;
}

void QgsAbstractDatabaseProviderConnection::TableProperty::setSchema( const QString &schema )
{
  mSchema = schema;
}

