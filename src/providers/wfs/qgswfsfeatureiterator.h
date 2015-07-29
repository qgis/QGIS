/***************************************************************************
    qgswfsfeatureiterator.h
    ---------------------
    begin                : Januar 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWFSFEATUREITERATOR_H
#define QGSWFSFEATUREITERATOR_H

#include "qgsfeatureiterator.h"

class QgsWFSProvider;
class QgsSpatialIndex;
typedef QMap<QgsFeatureId, QgsFeature*> QgsFeaturePtrMap;


class QgsWFSFeatureSource : public QObject, public QgsAbstractFeatureSource
{
    Q_OBJECT

  public:
    QgsWFSFeatureSource( const QgsWFSProvider* p );
    ~QgsWFSFeatureSource();

    QgsFeatureIterator getFeatures( const QgsFeatureRequest& request ) override;

  signals:
    void extentRequested( const QgsRectangle & );

  protected:

    QgsFields mFields;
    QgsFeaturePtrMap mFeatures;
    QgsSpatialIndex* mSpatialIndex;

    friend class QgsWFSFeatureIterator;
};

class QgsWFSFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsWFSFeatureSource>
{
  public:
    QgsWFSFeatureIterator( QgsWFSFeatureSource* source, bool ownSource, const QgsFeatureRequest& request );
    ~QgsWFSFeatureIterator();

    bool rewind() override;
    bool close() override;

  protected:
    bool fetchFeature( QgsFeature& f ) override;

    /** Copies feature attributes / geometry from f to feature*/
    void copyFeature( const QgsFeature* f, QgsFeature& feature, bool fetchGeometry );

  private:
    QList<QgsFeatureId> mSelectedFeatures;
    QList<QgsFeatureId>::const_iterator mFeatureIterator;
};

#endif // QGSWFSFEATUREITERATOR_H
