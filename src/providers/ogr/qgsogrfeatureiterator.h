/***************************************************************************
    qgsogrfeatureiterator.h
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
#ifndef QGSOGRFEATUREITERATOR_H
#define QGSOGRFEATUREITERATOR_H

#include "qgsfeatureiterator.h"

#include <ogr_api.h>

class QgsOgrProvider;

class QgsOgrFeatureIterator : public QgsAbstractFeatureIterator
{
  public:
    QgsOgrFeatureIterator( QgsOgrProvider* p, const QgsFeatureRequest& request );

    ~QgsOgrFeatureIterator();

    //! reset the iterator to the starting position
    virtual bool rewind();

    //! end of iterating: free the resources / lock
    virtual bool close();

  protected:
    //! fetch next feature, return true on success
    virtual bool fetchFeature( QgsFeature& feature );

    QgsOgrProvider* P;

    void ensureRelevantFields();

    bool readFeature( OGRFeatureH fet, QgsFeature& feature );

    //! Get an attribute associated with a feature
    void getFeatureAttribute( OGRFeatureH ogrFet, QgsFeature & f, int attindex );

    bool mFeatureFetched;

    OGRDataSourceH ogrDataSource;
    OGRLayerH ogrLayer;

    bool mSubsetStringSet;

    //! Set to true, if geometry is in the requested columns
    bool mFetchGeometry;
};


#endif // QGSOGRFEATUREITERATOR_H
