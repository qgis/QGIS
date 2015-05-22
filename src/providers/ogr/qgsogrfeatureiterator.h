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

class QgsOgrFeatureIterator;
class QgsOgrProvider;
class QgsOgrAbstractGeometrySimplifier;

class QgsOgrFeatureSource : public QgsAbstractFeatureSource
{
  public:
    QgsOgrFeatureSource( const QgsOgrProvider* p );

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request ) override;

  protected:
    QString mFilePath;
    QString mLayerName;
    int mLayerIndex;
    QString mSubsetString;
    QTextCodec* mEncoding;
    QgsFields mFields;
    OGRwkbGeometryType mOgrGeometryTypeFilter;
    QString mDriverName;

    friend class QgsOgrFeatureIterator;
};

class QgsOgrFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsOgrFeatureSource>
{
  public:
    QgsOgrFeatureIterator( QgsOgrFeatureSource* source, bool ownSource, const QgsFeatureRequest& request );

    ~QgsOgrFeatureIterator();

    //! reset the iterator to the starting position
    virtual bool rewind() override;

    //! end of iterating: free the resources / lock
    virtual bool close() override;

  protected:
    //! fetch next feature, return true on success
    virtual bool fetchFeature( QgsFeature& feature ) override;

    //! Setup the simplification of geometries to fetch using the specified simplify method
    virtual bool prepareSimplification( const QgsSimplifyMethod& simplifyMethod ) override;


    bool readFeature( OGRFeatureH fet, QgsFeature& feature );

    //! Get an attribute associated with a feature
    void getFeatureAttribute( OGRFeatureH ogrFet, QgsFeature & f, int attindex );

    bool mFeatureFetched;

    OGRDataSourceH ogrDataSource;
    OGRLayerH ogrLayer;

    bool mSubsetStringSet;

    //! Set to true, if geometry is in the requested columns
    bool mFetchGeometry;

  private:
    //! optional object to simplify OGR-geometries fecthed by this feature iterator
    QgsOgrAbstractGeometrySimplifier* mGeometrySimplifier;

    //! returns whether the iterator supports simplify geometries on provider side
    virtual bool providerCanSimplify( QgsSimplifyMethod::MethodType methodType ) const override;
};

#endif // QGSOGRFEATUREITERATOR_H
