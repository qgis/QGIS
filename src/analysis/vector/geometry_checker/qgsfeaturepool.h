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

#define SIP_NO_FILE

#ifndef QGS_FEATUREPOOL_H
#define QGS_FEATUREPOOL_H

#include <QCache>
#include <QLinkedList>
#include <QMap>
#include <QMutex>
#include "qgis_analysis.h"
#include "qgsfeature.h"
#include "qgsspatialindex.h"
#include "qgsgeometrycheckerutils.h"

class QgsVectorLayer;

class ANALYSIS_EXPORT QgsFeaturePool : public QObject
{
    Q_OBJECT
  public:
    QgsFeaturePool( QgsVectorLayer *layer, double layerToMapUnits, const QgsCoordinateTransform &layerToMapTransform, bool selectedOnly = false );
    bool get( QgsFeatureId id, QgsFeature &feature );
    void addFeature( QgsFeature &feature );
    void updateFeature( QgsFeature &feature );
    void deleteFeature( QgsFeatureId fid );
    QgsFeatureIds getIntersects( const QgsRectangle &rect ) const;
    QgsVectorLayer *getLayer() const { return mLayer; }
    const QgsFeatureIds &getFeatureIds() const { return mFeatureIds; }
    double getLayerToMapUnits() const { return mLayerToMapUnits; }
    const QgsCoordinateTransform &getLayerToMapTransform() const { return mLayerToMapTransform; }
    bool getSelectedOnly() const { return mSelectedOnly; }
    void clearLayer() { mLayer = nullptr; }

  private:
    struct MapEntry
    {
      MapEntry( QgsFeature *_feature, QLinkedList<QgsFeatureId>::iterator _ageIt )
        : feature( _feature )
        , ageIt( _ageIt )
      {}
      QgsFeature *feature = nullptr;
      QLinkedList<QgsFeatureId>::iterator ageIt;
    };

    static const int CACHE_SIZE = 1000;

    QCache<QgsFeatureId, QgsFeature> mFeatureCache;
    QgsVectorLayer *mLayer = nullptr;
    QgsFeatureIds mFeatureIds;
    QMutex mLayerMutex;
    mutable QMutex mIndexMutex;
    QgsSpatialIndex mIndex;
    double mLayerToMapUnits;
    QgsCoordinateTransform mLayerToMapTransform;
    bool mSelectedOnly;

    bool getTouchingWithSharedEdge( QgsFeature &feature, QgsFeatureId &touchingId, const double & ( *comparator )( const double &, const double & ), double init );
};

#endif // QGS_FEATUREPOOL_H
