/***************************************************************************
    qgsmemoryfeatureiterator.h
    ---------------------
    begin                : Juli 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMEMORYFEATUREITERATOR_H
#define QGSMEMORYFEATUREITERATOR_H

#include "qgsfeatureiterator.h"
#include "qgsexpressioncontext.h"
#include "qgsfields.h"
#include "qgsgeometry.h"

class QgsMemoryProvider;

typedef QMap<QgsFeatureId, QgsFeature> QgsFeatureMap;

class QgsSpatialIndex;


class QgsMemoryFeatureSource : public QgsAbstractFeatureSource
{
  public:
    explicit QgsMemoryFeatureSource( const QgsMemoryProvider* p );
    ~QgsMemoryFeatureSource();

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request ) override;

  protected:
    QgsFields mFields;
    QgsFeatureMap mFeatures;
    QgsSpatialIndex* mSpatialIndex = nullptr;
    QString mSubsetString;
    QgsExpressionContext mExpressionContext;

    friend class QgsMemoryFeatureIterator;
};


class QgsMemoryFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsMemoryFeatureSource>
{
  public:
    QgsMemoryFeatureIterator( QgsMemoryFeatureSource* source, bool ownSource, const QgsFeatureRequest& request );

    ~QgsMemoryFeatureIterator();

    virtual bool rewind() override;
    virtual bool close() override;

  protected:

    virtual bool fetchFeature( QgsFeature& feature ) override;

    bool nextFeatureUsingList( QgsFeature& feature );
    bool nextFeatureTraverseAll( QgsFeature& feature );

    QgsGeometry mSelectRectGeom;
    QgsFeatureMap::const_iterator mSelectIterator;
    bool mUsingFeatureIdList;
    QList<QgsFeatureId> mFeatureIdList;
    QList<QgsFeatureId>::const_iterator mFeatureIdListIterator;
    QgsExpression* mSubsetExpression = nullptr;

};

#endif // QGSMEMORYFEATUREITERATOR_H
