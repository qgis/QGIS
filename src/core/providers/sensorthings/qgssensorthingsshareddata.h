/***************************************************************************
      qgssensorthingsshareddata.h
      ----------------
    begin                : November 2023
    copyright            : (C) 2013 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSENSORTHINGSSHAREDDATA_H
#define QGSSENSORTHINGSSHAREDDATA_H

#include "qgsfields.h"
#include "qgscoordinatereferencesystem.h"
#include "qgshttpheaders.h"
#include "qgsfeature.h"
#include "qgsspatialindex.h"
#include "qgssensorthingsutils.h"

#include <QReadWriteLock>

class QgsFeedback;

#define SIP_NO_FILE
///@cond PRIVATE

/**
 * \brief This class holds data shared between QgsSensorThingsProvider and QgsSensorThingsFeatureSource instances.
 */
class QgsSensorThingsSharedData
{

  public:
    QgsSensorThingsSharedData( const QString &uri );

    /**
    * Parses and processes a \a url.
    */
    static QUrl parseUrl( const QUrl &url, bool *isTestEndpoint = nullptr );

    /**
     * Returns the error message obtained from the last operation.
     */
    QString error() const { return mError; }

    QgsCoordinateReferenceSystem crs() const { return mSourceCRS; }
    QgsRectangle extent() const;
    long long featureCount( QgsFeedback *feedback = nullptr ) const;
    QString subsetString() const;

    bool hasCachedAllFeatures() const;
    bool getFeature( QgsFeatureId id, QgsFeature &f, QgsFeedback *feedback = nullptr );
    QgsFeatureIds getFeatureIdsInExtent( const QgsRectangle &extent, QgsFeedback *feedback, const QString &thisPage, QString &nextPage,
                                         const QgsFeatureIds &alreadyFetchedIds );

    void clearCache();

  private:

    bool processFeatureRequest( QString &nextPage, QgsFeedback *feedback,
                                const std::function< void( const QgsFeature & ) > &fetchedFeatureCallback,
                                const std::function< bool() > &continueFetchingCallback,
                                const std::function< void() > &onNoMoreFeaturesCallback );

    friend class QgsSensorThingsProvider;
    mutable QReadWriteLock mReadWriteLock{ QReadWriteLock::Recursive };

    QString mAuthCfg;
    QgsHttpHeaders mHeaders;
    QString mRootUri;

    mutable QString mError;

    QString mEntityBaseUri;
    QString mSubsetString;

    Qgis::SensorThingsEntity mEntityType = Qgis::SensorThingsEntity::Invalid;

    int mFeatureLimit = 0;
    Qgis::WkbType mGeometryType = Qgis::WkbType::Unknown;
    QString mGeometryField;
    QgsFields mFields;

    QgsRectangle mFilterExtent;

    //! Extent calculated from features actually fetched so far
    QgsRectangle mFetchedFeatureExtent;

    QgsCoordinateReferenceSystem mSourceCRS;

    mutable long long mFeatureCount = static_cast< long long >( Qgis::FeatureCountState::Uncounted );

    QHash<QString, QgsFeatureId> mIotIdToFeatureId;
    QMap<QgsFeatureId, QgsFeature> mCachedFeatures;
    QgsGeometry mCachedExtent;

    QgsFeatureId mNextFeatureId = 0;
    bool mHasCachedAllFeatures = false;

    int mMaximumPageSize = QgsSensorThingsUtils::DEFAULT_PAGE_SIZE;

    QgsSpatialIndex mSpatialIndex;
    mutable QString mNextPage;
};

///@endcond PRIVATE

#endif // QGSSENSORTHINGSSHAREDDATA_H
