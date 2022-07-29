/***************************************************************************
      qgsafsfeatureiterator.h
      -----------------------
    begin                : Jun 03, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : smani@sourcepole.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAFSFEATUREITERATOR_H
#define QGSAFSFEATUREITERATOR_H

#include "qgsfeatureiterator.h"
#include "qgsafsshareddata.h"
#include "qgscoordinatetransform.h"
#include <memory>

class QgsSpatialIndex;

typedef QMap<QgsFeatureId, QgsFeature> QgsFeatureMap;

class QgsAfsFeatureSource : public QgsAbstractFeatureSource
{

  public:
    QgsAfsFeatureSource( const std::shared_ptr<QgsAfsSharedData> &sharedData );
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

    QgsAfsSharedData *sharedData() const;

  protected:
    std::shared_ptr<QgsAfsSharedData> mSharedData;

    friend class QgsAfsFeatureIterator;
};

class QgsAfsFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsAfsFeatureSource>
{
  public:
    QgsAfsFeatureIterator( QgsAfsFeatureSource *source, bool ownSource, const QgsFeatureRequest &request );
    ~QgsAfsFeatureIterator() override;
    bool rewind() override;
    bool close() override;

    void setInterruptionChecker( QgsFeedback *interruptionChecker ) override;

  protected:
    bool fetchFeature( QgsFeature &f ) override;

  private:
    QgsFeatureId mFeatureIterator = 0;

    QList< QgsFeatureId > mFeatureIdList;
    QList< QgsFeatureId > mRemainingFeatureIds;

    QgsCoordinateTransform mTransform;
    QgsRectangle mFilterRect;
    QgsGeometry mDistanceWithinGeom;
    std::unique_ptr< QgsGeometryEngine > mDistanceWithinEngine;

    QgsFeedback *mInterruptionChecker = nullptr;
    bool mDeferredFeaturesInFilterRectCheck = false;
};

#endif // QGSAFSFEATUREITERATOR_H
