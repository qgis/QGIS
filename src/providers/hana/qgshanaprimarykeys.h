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
    QMap<QgsFeatureId, QVariantList> mFidToKey;      // map feature back to fea
};

class QgsHanaPrimaryKeyUtils
{
  public:
    QgsHanaPrimaryKeyUtils() = delete;

    static QPair<QgsHanaPrimaryKeyType, QList<int>> determinePrimaryKeyFromColumns(const QStringList& columnNames, const QgsFields &fields);
    static QPair<QgsHanaPrimaryKeyType, QList<int>> determinePrimaryKeyFromUriKeyColumn(const QString& primaryKey, const QgsFields &fields);
    static int fidToInt(QgsFeatureId id);
    static QgsFeatureId intToFid(int id);
    static QgsHanaPrimaryKeyType getPrimaryKeyType( const QgsField &field );
    static QString buildWhereClause( const QgsFields &fields, QgsHanaPrimaryKeyType pkType,
                                         const QList<int> &pkAttrs );
    static QString buildWhereClause( QgsFeatureId featureId, const QgsFields &fields, QgsHanaPrimaryKeyType pkType,
                                         const QList<int> &pkAttrs, QgsHanaPrimaryKeyContext &primaryKeyCntx );
    static QString buildWhereClause( const QgsFeatureIds& featureIds, const QgsFields &fields, QgsHanaPrimaryKeyType pkType,
                                         const QList<int> &pkAttrs, QgsHanaPrimaryKeyContext &primaryKeyCntx );
};


#endif // QGSHANAPRIMARYKEYS_H
