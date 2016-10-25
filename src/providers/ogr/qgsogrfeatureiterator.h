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
#include "qgsogrconnpool.h"

#include <ogr_api.h>

class QgsOgrFeatureIterator;
class QgsOgrProvider;

class QgsOgrFeatureSource : public QgsAbstractFeatureSource
{
  public:
    explicit QgsOgrFeatureSource( const QgsOgrProvider* p );
    ~QgsOgrFeatureSource();

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request ) override;

  protected:
    const QgsOgrProvider* mProvider;
    QString mDataSource;
    QString mLayerName;
    int mLayerIndex;
    QString mSubsetString;
    QTextCodec* mEncoding;
    QgsFields mFields;
    bool mFirstFieldIsFid;
    QgsFields mFieldsWithoutFid;
    OGRwkbGeometryType mOgrGeometryTypeFilter;
    QString mDriverName;
    // mSubLayerList entry that was actually used to open the layer
    QString mSubLayerString;

    friend class QgsOgrFeatureIterator;
    friend class QgsOgrExpressionCompiler;
};

class QgsOgrFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsOgrFeatureSource>
{
  public:
    /** QgsOgrFeatureIterator
     * @param source QgsOgrFeatureSource
     * @note
     *
     * Up to Gdal 2.* each geometry was 1 layer and could be retrieved using an index
     * Starting with Gdal 2.0, each table is one layer, that can contain more than 1 geometry
     * - therefore it is important to use the correct method to open the Layer
     *
     * QgsOgrFeatureSource contains the const QgsOgrProvider
     * - at this point the Layer has been opened
     * @see QgsOgrProvider::OGRGetLayerWrapper()
     *
     * To open the Layer again, the information stored in 'mSubLayerString' should be extracted
     * @see QgsOgrProvider::mSubLayerString
     * - if 'OGRGetLayerNameWrapper' was used to successfully load the Layer, the index value in 'mSubLayerString' will be set to '-1'
     * - the Layer-nname stored in 'mSubLayerString' should be used when calling OGRGetLayerNameWrapper
     * @see QgsOgrProviderUtils::OGRGetLayerNameWrapper()
     * - otherwise the index value stored in 'mSubLayerString' should be used when calling OGRGetLayerIndexWrapper
     * @see QgsOgrProviderUtils::OGRGetLayerIndexWrapper()
     *
     */
    QgsOgrFeatureIterator( QgsOgrFeatureSource* source, bool ownSource, const QgsFeatureRequest& request );

    ~QgsOgrFeatureIterator();

    //! reset the iterator to the starting position
    virtual bool rewind() override;

    //! end of iterating: free the resources / lock
    virtual bool close() override;

  protected:
    //! fetch next feature, return true on success
    virtual bool fetchFeature( QgsFeature& feature ) override;

    //! fetch next feature filter expression
    bool nextFeatureFilterExpression( QgsFeature& f ) override;

    bool readFeature( OGRFeatureH fet, QgsFeature& feature ) const;

    //! Get an attribute associated with a feature
    void getFeatureAttribute( OGRFeatureH ogrFet, QgsFeature & f, int attindex ) const;

    bool mFeatureFetched;

    QgsOgrConn* mConn;
    OGRLayerH ogrLayer;

    bool mSubsetStringSet;

    //! Set to true, if geometry is in the requested columns
    bool mFetchGeometry;

  private:
    bool mExpressionCompiled;
    QgsFeatureIds mFilterFids;
    QgsFeatureIds::const_iterator mFilterFidsIt;

    bool fetchFeatureWithId( QgsFeatureId id, QgsFeature& feature ) const;

};

#endif // QGSOGRFEATUREITERATOR_H
