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

#include <memory>
#include <set>

class QgsOgrFeatureIterator;
class QgsOgrProvider;
class QgsOgrDataset;
using QgsOgrDatasetSharedPtr = std::shared_ptr< QgsOgrDataset>;

class QgsOgrFeatureSource : public QgsAbstractFeatureSource
{
  public:
    explicit QgsOgrFeatureSource( const QgsOgrProvider *p );
    ~QgsOgrFeatureSource() override;

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

  private:
    QString mDataSource;
    bool mShareSameDatasetAmongLayers;
    QString mLayerName;
    int mLayerIndex;
    QString mSubsetString;
    QTextCodec *mEncoding = nullptr;
    QgsFields mFields;
    bool mFirstFieldIsFid;
    QgsFields mFieldsWithoutFid;
    OGRwkbGeometryType mOgrGeometryTypeFilter;
    QString mDriverName;
    QgsCoordinateReferenceSystem mCrs;
    QgsWkbTypes::Type mWkbType = QgsWkbTypes::Unknown;
    QgsOgrDatasetSharedPtr mSharedDS = nullptr;

    friend class QgsOgrFeatureIterator;
    friend class QgsOgrExpressionCompiler;
};

class QgsOgrFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsOgrFeatureSource>
{
  public:
    QgsOgrFeatureIterator( QgsOgrFeatureSource *source, bool ownSource, const QgsFeatureRequest &request );

    ~QgsOgrFeatureIterator() override;

    bool rewind() override;
    bool close() override;

  protected:
    bool checkFeature( gdal::ogr_feature_unique_ptr &fet, QgsFeature &feature ) ;
    bool fetchFeature( QgsFeature &feature ) override;
    bool nextFeatureFilterExpression( QgsFeature &f ) override;

  private:

    bool readFeature( gdal::ogr_feature_unique_ptr fet, QgsFeature &feature ) const;

    //! Gets an attribute associated with a feature
    void getFeatureAttribute( OGRFeatureH ogrFet, QgsFeature &f, int attindex ) const;

    QgsOgrConn *mConn = nullptr;
    OGRLayerH mOgrLayer = nullptr; // when mOgrLayerUnfiltered != null and mOgrLayer != mOgrLayerUnfiltered, this is a SQL layer
    OGRLayerH mOgrLayerOri = nullptr; // only set when there's a mSubsetString. In which case this a regular OGR layer. Potentially == mOgrLayer

    //! Sets to true, if geometry is in the requested columns
    bool mFetchGeometry = false;

    bool mExpressionCompiled = false;
    // use std::set to get sorted ids (needed for efficient QgsFeatureRequest::FilterFids requests on OSM datasource)
    std::set<QgsFeatureId> mFilterFids;
    std::set<QgsFeatureId>::iterator mFilterFidsIt;

    QgsRectangle mFilterRect;
    QgsCoordinateTransform mTransform;
    QgsOgrDatasetSharedPtr mSharedDS = nullptr;

    bool mFirstFieldIsFid = false;
    QgsFields mFieldsWithoutFid;

    bool fetchFeatureWithId( QgsFeatureId id, QgsFeature &feature ) const;

    void resetReading();
};

#endif // QGSOGRFEATUREITERATOR_H
