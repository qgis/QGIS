/***************************************************************************
      qgspostgresutils.h  -  Utils for PostgreSQL/PostGIS 
                             -------------------
    begin                : Jan 2, 2004
    copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOSTGRESUTILS_H
#define QGSPOSTGRESUTILS_H

#include "qgspostgresconn.h"

/**
 * Data shared between provider class and its feature sources. Ideally there should
 *  be as few members as possible because there could be simultaneous reads/writes
 *  from different threads and therefore locking has to be involved.
*/
class QgsPostgresSharedData
{
  public:
    QgsPostgresSharedData() = default;

    long long featuresCounted();
    void setFeaturesCounted( long long count );
    void addFeaturesCounted( long long diff );
    void ensureFeaturesCountedAtLeast( long long fetched );

    // FID lookups
    QgsFeatureId lookupFid( const QVariantList &v ); // lookup existing mapping or add a new one
    QVariantList removeFid( QgsFeatureId fid );
    void insertFid( QgsFeatureId fid, const QVariantList &k );
    QVariantList lookupKey( QgsFeatureId featureId );
    void clear();

    void clearSupportsEnumValuesCache();
    bool fieldSupportsEnumValuesIsSet( int index );
    bool fieldSupportsEnumValues( int index );
    void setFieldSupportsEnumValues( int index, bool isSupported );

  protected:
    QMutex mMutex; //!< Access to all data members is guarded by the mutex

    long long mFeaturesCounted = -1; //!< Number of features in the layer

    QgsFeatureId mFidCounter = 0;               // next feature id if map is used
    QMap<QVariantList, QgsFeatureId> mKeyToFid; // map key values to feature id
    QMap<QgsFeatureId, QVariantList> mFidToKey; // map feature id back to key values
    QMap<int, bool> mFieldSupportsEnumValues;   // map field index to bool flag supports enum values
};

//! Assorted Postgres utility functions
class QgsPostgresUtils
{
  public:
    static bool deleteLayer( const QString &uri, QString &errCause );
    static bool deleteSchema( const QString &schema, const QgsDataSourceUri &uri, QString &errCause, bool cascade = false );

    static QString whereClause( QgsFeatureId featureId, const QgsFields &fields, QgsPostgresConn *conn, QgsPostgresPrimaryKeyType pkType, const QList<int> &pkAttrs, const std::shared_ptr<QgsPostgresSharedData> &sharedData );

    static QString whereClause( const QgsFeatureIds &featureIds, const QgsFields &fields, QgsPostgresConn *conn, QgsPostgresPrimaryKeyType pkType, const QList<int> &pkAttrs, const std::shared_ptr<QgsPostgresSharedData> &sharedData );

    static QString andWhereClauses( const QString &c1, const QString &c2 );

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

    //! Replaces invalid XML chars with UTF-8[<char_code>]
    static void replaceInvalidXmlChars( QString &xml );

    //! Replaces UTF-8[<char_code>] with the actual unicode char
    static void restoreInvalidXmlChars( QString &xml );

    static bool createStylesTable( QgsPostgresConn *conn, QString loggedClass );

    static bool columnExists( QgsPostgresConn *conn, const QString &table, const QString &column );

    static bool tableExists( QgsPostgresConn *conn, const QString &name );
};

#endif
