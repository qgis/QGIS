/***************************************************************************
   qgshanaprimarykeys.cpp
   --------------------------------------
   Date      : 23-12-2020
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgshanaprimarykeys.h"
#include "qgshanautils.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

#include <QObject>

namespace
{
  // The code of fid_to_int32pk is taken from PostgreSQL provider
  static const qint64 INT32PK_OFFSET = 4294967296; // 2^32

  // We shift negative 32bit integers to above the max 32bit
  // positive integer to support the whole range of int32 values
  // See https://github.com/qgis/QGIS/issues/22258
  static qint64 int32pk_to_fid( qint32 x )
  {
    return x >= 0 ? x : x + INT32PK_OFFSET;
  }

  static qint32 fid_to_int32pk( qint64 x )
  {
    return x <= ( ( INT32PK_OFFSET ) / 2 ) ? x : -( INT32PK_OFFSET - x );
  }
}

QgsFeatureId QgsHanaPrimaryKeyContext::lookupFid( const QVariantList &v )
{
  const QMutexLocker locker( &mMutex );

  const QMap<QVariantList, QgsFeatureId>::const_iterator it = mKeyToFid.constFind( v );

  if ( it != mKeyToFid.constEnd() )
    return it.value();

  mFidToKey.insert( ++mFidCounter, v );
  mKeyToFid.insert( v, mFidCounter );

  return mFidCounter;
}

QVariantList QgsHanaPrimaryKeyContext::removeFid( QgsFeatureId fid )
{
  const QMutexLocker locker( &mMutex );

  QVariantList v = mFidToKey[ fid ];
  mFidToKey.remove( fid );
  mKeyToFid.remove( v );
  return v;
}

void QgsHanaPrimaryKeyContext::insertFid( QgsFeatureId fid, const QVariantList &k )
{
  const QMutexLocker locker( &mMutex );

  mFidToKey.insert( fid, k );
  mKeyToFid.insert( k, fid );
}

QVariantList QgsHanaPrimaryKeyContext::lookupKey( QgsFeatureId featureId )
{
  const QMutexLocker locker( &mMutex );

  const QMap<QgsFeatureId, QVariantList>::const_iterator it = mFidToKey.constFind( featureId );
  if ( it != mFidToKey.constEnd() )
    return it.value();
  return QVariantList();
}

QPair<QgsHanaPrimaryKeyType, QList<int>> QgsHanaPrimaryKeyUtils::determinePrimaryKeyFromColumns( const QStringList &columnNames, const QgsFields &fields )
{
  QgsHanaPrimaryKeyType keyType = PktUnknown;
  QList<int> attrs;

  for ( const QString &clmName : columnNames )
  {
    const int idx = fields.indexFromName( clmName );
    if ( idx < 0 )
    {
      attrs.clear();
      QgsMessageLog::logMessage( QObject::tr( "Key field '%1' for view/query not found." ).arg( clmName ), QObject::tr( "SAP HANA" ) );
      break;
    }
    attrs << idx;
  }

  if ( !attrs.isEmpty() )
    keyType = ( attrs.size() == 1 ) ? getPrimaryKeyType( fields.at( attrs[0] ) ) : PktFidMap;
  else
    QgsMessageLog::logMessage( QObject::tr( "Keys for view/query undefined. Some functionality might not be available." ), QObject::tr( "SAP HANA" ) );

  return qMakePair( keyType, attrs );
}

QPair<QgsHanaPrimaryKeyType, QList<int>> QgsHanaPrimaryKeyUtils::determinePrimaryKeyFromUriKeyColumn( const QString &primaryKey, const QgsFields &fields )
{
  if ( primaryKey.isEmpty() )
  {
    QgsMessageLog::logMessage( QObject::tr( "No key field for view/query given." ), QObject::tr( "SAP HANA" ) );
    return qMakePair( PktUnknown, QList<int>() );
  }

  return determinePrimaryKeyFromColumns( parseUriKey( primaryKey ), fields );
}

int QgsHanaPrimaryKeyUtils::fidToInt( QgsFeatureId id )
{
  return fid_to_int32pk( id );
}

QgsFeatureId QgsHanaPrimaryKeyUtils::intToFid( int id )
{
  return int32pk_to_fid( id );
}

QgsHanaPrimaryKeyType QgsHanaPrimaryKeyUtils::getPrimaryKeyType( const QgsField &field )
{
  switch ( field.type() )
  {
    case QVariant::LongLong:
      return PktInt64;
    case QVariant::Int:
      return PktInt;
    default:
      return PktFidMap;
  }
}

QString QgsHanaPrimaryKeyUtils::buildWhereClause( const QgsFields &fields, QgsHanaPrimaryKeyType pkType,
    const QList<int> &pkAttrs )
{
  switch ( pkType )
  {
    case PktInt:
    case PktInt64:
    {
      const QString columnName = fields.at( pkAttrs[0] ).name() ;
      return QStringLiteral( "%1=?" ).arg( QgsHanaUtils::quotedIdentifier( columnName ) );
    }
    case PktFidMap:
    {
      QList<QString> conditions;
      conditions.reserve( pkAttrs.size() );
      for ( const int idx : pkAttrs )
        conditions << QStringLiteral( "%1=?" ).arg( QgsHanaUtils::quotedIdentifier( fields[idx].name() ) );
      return conditions.join( QLatin1String( " AND " ) );
    }
    case PktUnknown:
      return QString();
  }
  return QString(); //avoid warning
}

QString QgsHanaPrimaryKeyUtils::buildWhereClause( QgsFeatureId featureId, const QgsFields &fields, QgsHanaPrimaryKeyType pkType,
    const QList<int> &pkAttrs, QgsHanaPrimaryKeyContext &primaryKeyCntx )
{
  switch ( pkType )
  {
    case PktInt:
    {
      Q_ASSERT( pkAttrs.size() == 1 );
      const QString fieldName = fields[pkAttrs[0]].name();
      return QStringLiteral( "%1=%2" ).arg( QgsHanaUtils::quotedIdentifier( fieldName ) ).arg( fidToInt( featureId ) );
    }
    case PktInt64:
    {
      Q_ASSERT( pkAttrs.size() == 1 );
      QVariantList pkValues = primaryKeyCntx.lookupKey( featureId );
      Q_ASSERT( pkValues.size() == 1 );
      if ( pkValues.isEmpty() )
        return QString();

      const QgsField &field = fields.at( pkAttrs[0] );
      return QStringLiteral( "%1=%2" ).arg( QgsHanaUtils::quotedIdentifier( field.name() ), QgsHanaUtils::toConstant( pkValues[0], field.type() ) );
    }
    case PktFidMap:
    {
      QVariantList pkValues = primaryKeyCntx.lookupKey( featureId );
      Q_ASSERT( pkValues.size() == pkAttrs.size() );
      if ( pkValues.isEmpty() )
        return QString();

      QStringList conditions;
      for ( int i = 0; i < pkAttrs.size(); i++ )
      {
        const QgsField &field  = fields.at( pkAttrs[i] );
        conditions << QStringLiteral( "%1=%2" ).arg( QgsHanaUtils::quotedIdentifier( field.name() ), QgsHanaUtils::toConstant( pkValues[i], field.type() ) );
      }
      return conditions.join( QLatin1String( " AND " ) );
    }
    case PktUnknown:
      return QString();
  }

  return QString(); // avoid warning
}

QString QgsHanaPrimaryKeyUtils::buildWhereClause( const QgsFeatureIds &featureIds, const QgsFields &fields, QgsHanaPrimaryKeyType pkType,
    const QList<int> &pkAttrs, QgsHanaPrimaryKeyContext &primaryKeyCntx )
{
  if ( featureIds.isEmpty() )
    return QString();

  switch ( pkType )
  {
    case PktInt:
    case PktInt64:
    {
      QStringList fids;
      for ( const QgsFeatureId featureId : featureIds )
      {
        if ( pkType == PktInt )
          fids << QString::number( fidToInt( featureId ) );
        else
        {
          QVariantList pkValues = primaryKeyCntx.lookupKey( featureId );
          if ( pkValues.isEmpty() )
            return QString();
          fids << pkValues[0].toString();
        }
      }

      const QgsField &field  = fields.at( pkAttrs[0] );
      return QStringLiteral( "%1 IN (%2)" ).arg( QgsHanaUtils::quotedIdentifier( field.name() ), fids.join( ',' ) );
    }
    case PktFidMap:
    {
      QStringList whereClauses;
      for ( const QgsFeatureId featureId : featureIds )
      {
        const QString fidWhereClause = buildWhereClause( featureId, fields, pkType, pkAttrs, primaryKeyCntx );
        if ( fidWhereClause.isEmpty() )
          return QString();
        whereClauses << fidWhereClause;
      }
      return whereClauses.join( QLatin1String( " OR " ) ).prepend( '(' ).append( ')' );
    }
    case PktUnknown:
      return QString();
  }

  return QString(); //avoid warning
}

QString QgsHanaPrimaryKeyUtils::buildUriKey( const QStringList &columns )
{
  QString ret;
  for ( auto i = 0; i < columns.size(); ++i )
  {
    ret += QgsHanaUtils::quotedIdentifier( columns[i] );
    if ( i != columns.size() - 1 )
      ret += ',';
  }
  return ret;
}

QStringList QgsHanaPrimaryKeyUtils::parseUriKey( const QString &key )
{
  if ( key.isEmpty() )
    return QStringList();

  QStringList cols;

  // remove quotes from key list
  if ( key.startsWith( '"' ) && key.endsWith( '"' ) )
  {
    int i = 1;
    QString col;
    while ( i < key.size() )
    {
      if ( key[i] == '"' )
      {
        if ( i + 1 < key.size() && key[i + 1] == '"' )
        {
          i++;
        }
        else
        {
          cols << col;
          col.clear();

          if ( ++i == key.size() )
            break;

          Q_ASSERT( key[i] == ',' );
          i++;
          Q_ASSERT( key[i] == '"' );
          i++;
          continue;
        }
      }

      col += key[i++];
    }
  }
  else if ( key.contains( ',' ) )
  {
    cols = key.split( ',' );
  }
  else
  {
    cols << key;
  }

  return cols;
}
