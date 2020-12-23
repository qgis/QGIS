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
    return x <= ( ( INT32PK_OFFSET ) / 2.0 ) ? x : -( INT32PK_OFFSET - x );
  }

  QStringList parseUriKey( const QString &key )
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
            col.clear();
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
}

QgsFeatureId QgsHanaPrimaryKeyContext::lookupFid( const QVariantList &v )
{
  QMutexLocker locker( &mMutex );

  QMap<QVariantList, QgsFeatureId>::const_iterator it = mKeyToFid.constFind( v );

  if ( it != mKeyToFid.constEnd() )
    return it.value();

  mFidToKey.insert( ++mFidCounter, v );
  mKeyToFid.insert( v, mFidCounter );

  return mFidCounter;
}

QVariantList QgsHanaPrimaryKeyContext::removeFid( QgsFeatureId fid )
{
  QMutexLocker locker( &mMutex );

  QVariantList v = mFidToKey[ fid ];
  mFidToKey.remove( fid );
  mKeyToFid.remove( v );
  return v;
}

void QgsHanaPrimaryKeyContext::insertFid( QgsFeatureId fid, const QVariantList &k )
{
  QMutexLocker locker( &mMutex );

  mFidToKey.insert( fid, k );
  mKeyToFid.insert( k, fid );
}

QVariantList QgsHanaPrimaryKeyContext::lookupKey( QgsFeatureId featureId )
{
  QMutexLocker locker( &mMutex );

  QMap<QgsFeatureId, QVariantList>::const_iterator it = mFidToKey.find( featureId );
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
    int idx = fields.indexFromName( clmName );
    if ( idx < 0 )
    {
      attrs.clear();
      QgsMessageLog::logMessage( QObject::tr( "Key field '%1' for view/query not found." ).arg( clmName ), QObject::tr( "HANA" ) );
      break;
    }
    attrs << idx;
  }

  if ( !attrs.isEmpty() )
    keyType = ( attrs.size() == 1 ) ? getPrimaryKeyType( fields.at( attrs[0] ) ) : PktFidMap;
  else
    QgsMessageLog::logMessage( QObject::tr( "Keys for view/query undefined." ), QObject::tr( "HANA" ) );

  return qMakePair( keyType, attrs );
}

QPair<QgsHanaPrimaryKeyType, QList<int>> QgsHanaPrimaryKeyUtils::determinePrimaryKeyFromUriKeyColumn( const QString &primaryKey, const QgsFields &fields )
{
  if ( primaryKey.isEmpty() )
  {
    QgsMessageLog::logMessage( QObject::tr( "No key field for view/query given." ), QObject::tr( "HANA" ) );
    return qMakePair( PktUnknown, QList<int>() );
  }

  return determinePrimaryKeyFromColumns( parseUriKey( primaryKey ), fields );
}

int QgsHanaPrimaryKeyUtils::fidToInt(QgsFeatureId id)
{
    return fid_to_int32pk(id);
}

QgsFeatureId QgsHanaPrimaryKeyUtils::intToFid(int id)
{
    return int32pk_to_fid(id);
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
        QString columnName = fields.at( pkAttrs[0] ).name() ;
        return QStringLiteral( "%1 = ?" ).arg( QgsHanaUtils::quotedIdentifier( columnName ) );
      }
      case PktFidMap:
        // TODO
        return QString();
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
      QString fieldName = fields.at( pkAttrs[0] ).name();
      return QStringLiteral( "%1=%2" ).arg( QgsHanaUtils::quotedIdentifier( fieldName ) ).arg( fidToInt( featureId ) );
    }
    case PktInt64:
    {
      QString whereClause;
      Q_ASSERT( pkAttrs.size() == 1 );
      QVariantList pkVals = primaryKeyCntx.lookupKey( featureId );
      if ( !pkVals.isEmpty() )
      {
        QgsField fld = fields.at( pkAttrs[0] );
        whereClause = QgsHanaUtils::quotedIdentifier( fld.name() );
        if ( !pkVals[0].isNull() )
          whereClause += '=' + pkVals[0].toString();
        else
          whereClause += QLatin1String( " IS NULL" );
      }

      return whereClause;
    }
    case PktFidMap:
    {
      QVariantList pkVals = primaryKeyCntx.lookupKey( featureId );
      if ( !pkVals.isEmpty() )
      {
        Q_ASSERT( pkVals.size() == pkAttrs.size() );

        // TODO
        return QString();
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "FAILURE: Key values for feature %1 not found." ).arg( featureId ) );
        return QStringLiteral( "NULL" );
      }
    }

    case PktUnknown:
      return QString();
  }

  return QString(); // avoid warning
}

QString QgsHanaPrimaryKeyUtils::buildWhereClause( const QgsFeatureIds &featureIds, const QgsFields &fields, QgsHanaPrimaryKeyType pkType,
                              const QList<int> &pkAttrs, QgsHanaPrimaryKeyContext & primaryKeyCntx)
{
  if ( featureIds.isEmpty() )
    return QString();

  switch ( pkType )
  {
    case PktInt:
    case PktInt64:
    {
      QStringList fids;
      for ( QgsFeatureId featureId : featureIds )
      {
        if ( pkType == PktInt )
          fids.push_back( QString::number( fidToInt( featureId ) ) );
        else
        {
          QVariantList pkVals = primaryKeyCntx.lookupKey( featureId );
          if ( !pkVals.isEmpty() )
            fids.push_back( pkVals[0].toString() );
        }
      }

      QString columnName = fields.at( pkAttrs[0] ).name();
      return QStringLiteral( "%1 IN (%2)" ).arg( QgsHanaUtils::quotedIdentifier( columnName ), fids.join( ',' ) );
    }
    case PktFidMap:
      // TODO
      return QString();
    case PktUnknown:
      return QString();
  }

  return QString(); //avoid warning
}
