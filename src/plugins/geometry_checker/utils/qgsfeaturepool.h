/***************************************************************************
 *  qgsfeaturepool.h                                                       *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS_FEATUREPOOL_H
#define QGS_FEATUREPOOL_H

#include <QCache>
#include <QLinkedList>
#include <QMap>
#include <QMutex>
#include "qgsfeature.h"
#include "qgsspatialindex.h"
#include "qgsgeomutils.h"

class QgsVectorLayer;

class QgsFeaturePool
{
  public:
    QgsFeaturePool( QgsVectorLayer* layer, bool selectedOnly = false );
    bool get( QgsFeatureId id, QgsFeature& feature );
    void addFeature( QgsFeature &feature );
    void updateFeature( QgsFeature &feature );
    void deleteFeature( QgsFeature &feature );
    QgsFeatureIds getIntersects( const QgsRectangle& rect );
    QgsVectorLayer* getLayer() const { return mLayer; }
    const QgsFeatureIds& getFeatureIds() const { return mFeatureIds; }
    bool getSelectedOnly() const { return mSelectedOnly; }
    void clearLayer() { mLayer = nullptr; }

  private:
    struct MapEntry
    {
      MapEntry( QgsFeature* _feature, QLinkedList<QgsFeatureId>::iterator _ageIt )
          : feature( _feature )
          , ageIt( _ageIt )
      {}
      QgsFeature* feature;
      QLinkedList<QgsFeatureId>::iterator ageIt;
    };

    static const int sCacheSize = 1000;

    QCache<QgsFeatureId, QgsFeature> mFeatureCache;
    QgsVectorLayer* mLayer;
    QgsFeatureIds mFeatureIds;
    QMutex mLayerMutex;
    QMutex mIndexMutex;
    QgsSpatialIndex mIndex;
    bool mSelectedOnly;

    bool getTouchingWithSharedEdge( QgsFeature &feature, QgsFeatureId &touchingId, const double& ( *comparator )( const double&, const double& ), double init );
};

#endif // QGS_FEATUREPOOL_H
