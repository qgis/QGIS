/***************************************************************************
   qgshanaprimarykeys.h
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
#ifndef QGSHANAPRIMARYKEYS_H
#define QGSHANAPRIMARYKEYS_H

#include "qgsfeature.h"

#include <QMap>
#include <QMutex>
#include <QVariantList>

enum QgsHanaPrimaryKeyType
{
  PktUnknown,
  PktInt,
  PktInt64,
  PktFidMap
};

class QgsHanaPrimaryKeyContext
{
  public:
    QgsHanaPrimaryKeyContext() = default;

    // FID lookups
    QgsFeatureId lookupFid( const QVariantList &v ); // lookup existing mapping or add a new one
    QVariantList removeFid( QgsFeatureId fid );
    void insertFid( QgsFeatureId fid, const QVariantList &k );
    QVariantList lookupKey( QgsFeatureId featureId );

  protected:
    QMutex mMutex; //!< Access to all data members is guarded by the mutex

    QgsFeatureId mFidCounter = 0;                    // next feature id if map is used
    QMap<QVariantList, QgsFeatureId> mKeyToFid;      // map key values to feature id
    QMap<QgsFeatureId, QVariantList> mFidToKey;      // map feature id back to key values
};

class QgsHanaPrimaryKeyUtils
{
  public:
    QgsHanaPrimaryKeyUtils() = delete;

    static QPair<QgsHanaPrimaryKeyType, QList<int>> determinePrimaryKeyFromColumns( const QStringList &columnNames, const QgsFields &fields );
    static QPair<QgsHanaPrimaryKeyType, QList<int>> determinePrimaryKeyFromUriKeyColumn( const QString &primaryKey, const QgsFields &fields );
    static int fidToInt( QgsFeatureId id );
    static QgsFeatureId intToFid( int id );
    static QgsHanaPrimaryKeyType getPrimaryKeyType( const QgsField &field );
    static QString buildWhereClause( const QgsFields &fields, QgsHanaPrimaryKeyType pkType,
                                     const QList<int> &pkAttrs );
    static QString buildWhereClause( QgsFeatureId featureId, const QgsFields &fields, QgsHanaPrimaryKeyType pkType,
                                     const QList<int> &pkAttrs, QgsHanaPrimaryKeyContext &primaryKeyCntx );
    static QString buildWhereClause( const QgsFeatureIds &featureIds, const QgsFields &fields, QgsHanaPrimaryKeyType pkType,
                                     const QList<int> &pkAttrs, QgsHanaPrimaryKeyContext &primaryKeyCntx );
    static QString buildUriKey( const QStringList &columns );
    static QStringList parseUriKey( const QString &key );
};


#endif // QGSHANAPRIMARYKEYS_H
