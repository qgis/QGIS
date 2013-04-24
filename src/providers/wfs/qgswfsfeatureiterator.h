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

class QgsWFSFeatureIterator: public QgsAbstractFeatureIterator
{
  public:
    QgsWFSFeatureIterator( QgsWFSProvider* provider, const QgsFeatureRequest& request );
    ~QgsWFSFeatureIterator();

    bool nextFeature( QgsFeature& f );
    bool rewind();
    bool close();

  private:
    QgsWFSProvider* mProvider;
    QList<QgsFeatureId> mSelectedFeatures;
    QList<QgsFeatureId>::const_iterator mFeatureIterator;
};

#endif // QGSWFSFEATUREITERATOR_H
