/***************************************************************************
      qgssensorthingsfeatureiterator.h
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
#ifndef QGSSENSORTHINGSFEATUREITERATOR_H
#define QGSSENSORTHINGSFEATUREITERATOR_H

#include "qgsfeatureiterator.h"
#include "qgssensorthingsshareddata.h"
#include "qgscoordinatetransform.h"

#define SIP_NO_FILE

///@cond PRIVATE

class QgsSensorThingsFeatureSource : public QgsAbstractFeatureSource
{

  public:
    QgsSensorThingsFeatureSource( const std::shared_ptr<QgsSensorThingsSharedData> &sharedData );
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

    QgsSensorThingsSharedData *sharedData() const;

  protected:
    std::shared_ptr<QgsSensorThingsSharedData> mSharedData;

    friend class QgsSensorThingsFeatureIterator;
};

class QgsSensorThingsFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsSensorThingsFeatureSource>
{
  public:
    QgsSensorThingsFeatureIterator( QgsSensorThingsFeatureSource *source, bool ownSource, const QgsFeatureRequest &request );
    ~QgsSensorThingsFeatureIterator() override;
    bool rewind() final;
    bool close() final;
    void setInterruptionChecker( QgsFeedback *interruptionChecker ) final;

  protected:
    bool fetchFeature( QgsFeature &f ) final;

  private:
    QgsFeatureId mFeatureIterator = 0;

    QList< QgsFeatureId > mRequestFeatureIdList;

    // Feature IDs from current page to fetch
    QList< QgsFeatureId > mCurrentPageFeatureIdList;
    // Remaining feature IDs which haven't yet been fetched
    QList< QgsFeatureId > mRemainingFeatureIds;

    QgsFeatureIds mAlreadyFetchedIds;

    QgsCoordinateTransform mTransform;
    QgsRectangle mFilterRect;
    QgsRectangle mGeometryTestFilterRect;
    QgsGeometry mDistanceWithinGeom;
    std::unique_ptr< QgsGeometryEngine > mDistanceWithinEngine;

    QgsFeedback *mInterruptionChecker = nullptr;
    bool mDeferredFeaturesInFilterRectCheck = false;

    QString mCurrentPage;
    QString mNextPage;
};

///@endcond PRIVATE

#endif // QGSSENSORTHINGSFEATUREITERATOR_H
