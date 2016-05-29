/***************************************************************************
    qgsglobefeaturecursor.h
     --------------------------------------
    Date                 : 11.7.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGLOBEFEATURECURSOR_H
#define QGSGLOBEFEATURECURSOR_H

#include <osgEarthFeatures/FeatureCursor>
#include <qgsfeature.h>
#include <qgsfeatureiterator.h>
#include <qgsvectorlayer.h>

#include "qgsglobefeatureutils.h"


class QgsGlobeFeatureCursor : public osgEarth::Features::FeatureCursor
{
  public:
    QgsGlobeFeatureCursor( QgsVectorLayer* layer, const QgsFeatureIterator& iterator )
        : mIterator( iterator )
        , mLayer( layer )
    {
      mIterator.nextFeature( mFeature );
    }

    bool hasMore() const override
    {
      return mFeature.isValid();
    }

    osgEarth::Features::Feature* nextFeature() override
    {
      if ( mFeature.isValid() )
      {
        osgEarth::Features::Feature *feat = QgsGlobeFeatureUtils::featureFromQgsFeature( mLayer, mFeature );
        mIterator.nextFeature( mFeature );
        return feat;
      }
      else
      {
        QgsDebugMsg( "WARNING: Returning NULL feature to osgEarth" );
        return NULL;
      }
    }

  private:
    QgsFeatureIterator mIterator;
    QgsVectorLayer* mLayer;
    // Cached feature which will be returned next.
    // Always contains the next feature which will be returned
    // (Because hasMore() needs to know if we are able to return a next feature)
    QgsFeature mFeature;
};

#endif // QGSGLOBEFEATURECURSOR_H
