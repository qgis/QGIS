/***************************************************************************
    qgsbackgroundcachedfeaturesource.h
    ----------------------------------
    begin                : October 2019
    copyright            : (C) 2016-2019 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSBACKGROUNDCACHEDFEATURESOURCE_H
#define QGSBACKGROUNDCACHEDFEATURESOURCE_H

#include "qgsfeatureiterator.h"
#include "qgsfeaturerequest.h" // QgsFeatureRequest, QgsAbstractFeatureSource

class QgsBackgroundCachedSharedData;

//! Feature source
class QgsBackgroundCachedFeatureSource final : public QgsAbstractFeatureSource
{
  public:
    explicit QgsBackgroundCachedFeatureSource( std::shared_ptr<QgsBackgroundCachedSharedData> shared );

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

  private:
    std::shared_ptr<QgsBackgroundCachedSharedData> mShared; //!< Mutable data shared between provider and feature sources
};

#endif // QGSBACKGROUNDCACHEDFEATURESOURCE_H
