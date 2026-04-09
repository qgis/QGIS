/***************************************************************************
    qgsafsshareddata.h
    ---------------------
    begin                : June 2017
    copyright            : (C) 2017 by Sandro Mani
    email                : manisandro at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAFSSHAREDDATA_H
#define QGSAFSSHAREDDATA_H

#include "qgscoordinatereferencesystem.h"
#include "qgsdatasourceuri.h"
#include "qgsfeature.h"
#include "qgsfields.h"
#include "qgsvectordataprovider.h"

#include <QMutex>

class QgsFeedback;

/**
 * \brief This class holds data, shared between QgsAfsProvider and QgsAfsFeatureIterator
 */
class QgsAfsSharedData
{
  public:
    QgsAfsSharedData( const QgsDataSourceUri &uri );

    bool ensureObjectIdsFetched( QString &errorMessage );

    //! Creates a deep copy of this shared data
    std::shared_ptr<QgsAfsSharedData> clone() const;

    // ensureObjectIdsFetched MUST have been called!
    long long objectIdCount() const;
    long long featureCount( QString &errorMessage );
    bool isDeleted( QgsFeatureId id ) const { return mDeletedFeatureIds.contains( id ); }
    const QgsFields &fields() const { return mFields; }
    QgsRectangle extent() const;
    QgsCoordinateReferenceSystem crs() const { return mSourceCRS; }

    QString subsetString() const;
    bool setSubsetString( const QString &subset );

    void clearCache();

    bool getObjectIds( QString &errorMessage );

    quint32 featureIdToObjectId( QgsFeatureId id, QString &error );

    // lock must already be obtained by caller, and ensureObjectIdsFetched MUST have been called!
    QgsFeatureId objectIdToFeatureId( quint32 oid );

    /**
     * Retrieves a feature by \a id.
     *
     * \param id target feature ID
     * \param f feature to be populated
     * \param pendingFeatureIds optional list of other features which are desirable to request in a batch operation, if a network request is required to fetch the target feature.
     * \param feedback
     * \returns TRUE if matching feature was retrieved
     *
     * \warning ensureObjectIdsFetched() MUST have been called before calling this!
     */
    bool getFeature( QgsFeatureId id, QgsFeature &f, const QList<QgsFeatureId> &pendingFeatureIds = QList< QgsFeatureId >(), QgsFeedback *feedback = nullptr );

    // ensureObjectIdsFetched MUST have been called!
    QgsFeatureIds getFeatureIdsInExtent( const QgsRectangle &extent, QgsFeedback *feedback );

    bool deleteFeatures( const QgsFeatureIds &id, QString &error, QgsFeedback *feedback );
    bool addFeatures( QgsFeatureList &features, QString &error, QgsFeedback *feedback );
    bool updateFeatures( const QgsFeatureList &features, bool includeGeometries, bool includeAttributes, QString &error, QgsFeedback *feedback );
    bool addFields( const QString &adminUrl, const QList<QgsField> &attributes, QString &error, QgsFeedback *feedback );
    bool deleteFields( const QString &adminUrl, const QgsAttributeIds &attributes, QString &error, QgsFeedback *feedback );
    bool addAttributeIndex( const QString &adminUrl, int attribute, QString &error, QgsFeedback *feedback );

    bool hasCachedAllFeatures();

    int objectIdFieldIndex() const;

  private:
    QVariantMap postData( const QUrl &url, const QByteArray &payload, QgsFeedback *feedback, bool &ok, QString &errorText ) const;

    friend class QgsAfsProvider;
    mutable QReadWriteLock mReadWriteLock { QReadWriteLock::Recursive };
    QgsDataSourceUri mDataSource;
    bool mHasFetchedObjectIds = false;
    bool mLimitBBox = false;
    QgsRectangle mExtent;
    Qgis::WkbType mGeometryType = Qgis::WkbType::Unknown;
    QgsFields mFields;
    int mMaximumFetchObjectsCount = 100;

    QString mObjectIdFieldName;
    int mObjectIdFieldIdx = -1;

    // list index is feature id, value is object ID
    QList<quint32> mFeatureIdsToObjectIds;
    // hash key is object ID, value is feature ID
    QHash<quint32, QgsFeatureId> mObjectIdToFeatureId;

    QSet<QgsFeatureId> mDeletedFeatureIds;
    QMap<QgsFeatureId, QgsFeature> mCache;
    QgsCoordinateReferenceSystem mSourceCRS;
};

#endif
