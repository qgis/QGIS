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

#include <QObject>
#include <QMutex>
#include "qgsfields.h"
#include "qgsfeature.h"
#include "qgsdatasourceuri.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"

class QgsFeedback;

/**
 * \brief This class holds data, shared between QgsAfsProvider and QgsAfsFeatureIterator
 */
class QgsAfsSharedData : public QObject
{
    Q_OBJECT
  public:
    QgsAfsSharedData() = default;
    long long objectIdCount() const;
    long long featureCount() const;
    bool isDeleted( QgsFeatureId id ) const { return mDeletedFeatureIds.contains( id ); }
    const QgsFields &fields() const { return mFields; }
    QgsRectangle extent() const { return mExtent; }
    QgsCoordinateReferenceSystem crs() const { return mSourceCRS; }
    void clearCache();

    bool getObjectIds( QString &errorMessage );

    quint32 featureIdToObjectId( QgsFeatureId id );

    bool getFeature( QgsFeatureId id, QgsFeature &f, const QgsRectangle &filterRect = QgsRectangle(), QgsFeedback *feedback = nullptr );
    QgsFeatureIds getFeatureIdsInExtent( const QgsRectangle &extent, QgsFeedback *feedback );

    bool deleteFeatures( const QgsFeatureIds &id, QString &error, QgsFeedback *feedback );
    bool addFeatures( QgsFeatureList &features, QString &error, QgsFeedback *feedback );
    bool updateFeatures( const QgsFeatureList &features, bool includeGeometries, bool includeAttributes, QString &error, QgsFeedback *feedback );
    bool addFields( const QString &adminUrl, const QList<QgsField> &attributes, QString &error, QgsFeedback *feedback );
    bool deleteFields( const QString &adminUrl, const QgsAttributeIds &attributes, QString &error, QgsFeedback *feedback );

    bool hasCachedAllFeatures() const;

  private:

    QVariantMap postData( const QUrl &url, const QByteArray &payload, QgsFeedback *feedback, bool &ok, QString &errorText ) const;

    friend class QgsAfsProvider;
    mutable QReadWriteLock mReadWriteLock{ QReadWriteLock::Recursive };
    QgsDataSourceUri mDataSource;
    bool mLimitBBox = false;
    QgsRectangle mExtent;
    QgsWkbTypes::Type mGeometryType = QgsWkbTypes::Unknown;
    QgsFields mFields;

    QString mObjectIdFieldName;
    int mObjectIdFieldIdx = -1;

    QList<quint32> mObjectIds;
    QSet<QgsFeatureId> mDeletedFeatureIds;
    QMap<QgsFeatureId, QgsFeature> mCache;
    QgsCoordinateReferenceSystem mSourceCRS;
};

#endif
