/***************************************************************************
 *  qgsgeometrysnapper.h                                                   *
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

#ifndef QGS_GEOMETRY_SNAPPER_H
#define QGS_GEOMETRY_SNAPPER_H

#include <QMutex>
#include <QFuture>
#include <QStringList>
#include "qgsspatialindex.h"

class QgsMapSettings;
class QgsVectorLayer;
class QgsAbstractGeometryV2;

class QgsGeometrySnapper : public QObject
{
    Q_OBJECT

  public:
    QgsGeometrySnapper( QgsVectorLayer* adjustLayer, QgsVectorLayer* referenceLayer, bool selectedOnly, double snapToleranceMapUnits, const QgsMapSettings* mapSettings );
    QFuture<void> processFeatures();
    const QStringList& getErrors() const { return mErrors; }

  signals:
    void progressRangeChanged( int min, int max );
    void progressStep();

  private:
    struct ProcessFeatureWrapper
    {
      QgsGeometrySnapper* instance;
      explicit ProcessFeatureWrapper( QgsGeometrySnapper* _instance ) : instance( _instance ) {}
      void operator()( QgsFeatureId id ) { instance->processFeature( id ); }
    };

    enum PointFlag { SnappedToRefNode, SnappedToRefSegment, Unsnapped };

    QgsVectorLayer* mAdjustLayer;
    QgsVectorLayer* mReferenceLayer;
    double mSnapToleranceMapUnits;
    const QgsMapSettings* mMapSettings;
    QgsFeatureIds mFeatures;
    QgsSpatialIndex mIndex;
    QStringList mErrors;
    QMutex mErrorMutex;
    QMutex mIndexMutex;
    QMutex mAdjustLayerMutex;
    QMutex mReferenceLayerMutex;

    void processFeature( QgsFeatureId id );
    bool getFeature( QgsVectorLayer* layer, QMutex& mutex, QgsFeatureId id, QgsFeature& feature );
    int polyLineSize( const QgsAbstractGeometryV2* geom, int iPart, int iRing ) const;
};

#endif // QGS_GEOMETRY_SNAPPER_H
