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
#include "qgsfields.h"

#include <ogr_api.h>

class QgsOgrFeatureIterator;
class QgsOgrProvider;

class QgsOgrFeatureSource : public QgsAbstractFeatureSource
{
  public:
    explicit QgsOgrFeatureSource( const QgsOgrProvider *p );
    ~QgsOgrFeatureSource();

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

  protected:
    const QgsOgrProvider *mProvider = nullptr;
    QString mDataSource;
    QString mLayerName;
    QString mGeometryName;
    int mLayerIndex;
    QString mSubsetString;
    QTextCodec *mEncoding = nullptr;
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
    QgsOgrFeatureIterator( QgsOgrFeatureSource *source, bool ownSource, const QgsFeatureRequest &request );

    ~QgsOgrFeatureIterator();

    virtual bool rewind() override;
    virtual bool close() override;

  protected:
    virtual bool fetchFeature( QgsFeature &feature ) override;
    bool nextFeatureFilterExpression( QgsFeature &f ) override;

    bool readFeature( OGRFeatureH fet, QgsFeature &feature ) const;

    //! Get an attribute associated with a feature
    void getFeatureAttribute( OGRFeatureH ogrFet, QgsFeature &f, int attindex ) const;

    bool mFeatureFetched;

    QgsOgrConn *mConn = nullptr;
    OGRLayerH ogrLayer;

    bool mSubsetStringSet;

    //! Set to true, if geometry is in the requested columns
    bool mFetchGeometry;

  private:
    bool mExpressionCompiled;
    QgsFeatureIds mFilterFids;
    QgsFeatureIds::const_iterator mFilterFidsIt;

    bool fetchFeatureWithId( QgsFeatureId id, QgsFeature &feature ) const;
};

#endif // QGSOGRFEATUREITERATOR_H
