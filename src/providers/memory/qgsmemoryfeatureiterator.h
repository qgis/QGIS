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

class QgsMemoryProvider;

typedef QMap<QgsFeatureId, QgsFeature> QgsFeatureMap;


class QgsMemoryFeatureIterator : public QgsAbstractFeatureIterator
{
  public:
    QgsMemoryFeatureIterator( QgsMemoryProvider* p, const QgsFeatureRequest& request );

    ~QgsMemoryFeatureIterator();

    //! fetch next feature, return true on success
    virtual bool nextFeature( QgsFeature& feature );

    //! reset the iterator to the starting position
    virtual bool rewind();

    //! end of iterating: free the resources / lock
    virtual bool close();

  protected:

    bool nextFeatureUsingList( QgsFeature& feature );
    bool nextFeatureTraverseAll( QgsFeature& feature );

    QgsMemoryProvider* P;

    QgsGeometry* mSelectRectGeom;
    QgsFeatureMap::iterator mSelectIterator;
    bool mUsingFeatureIdList;
    QList<QgsFeatureId> mFeatureIdList;
    QList<QgsFeatureId>::iterator mFeatureIdListIterator;

};

#endif // QGSMEMORYFEATUREITERATOR_H
