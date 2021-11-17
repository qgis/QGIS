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
#include "qgsogrutils.h"
#include "qgscoordinatetransform.h"

#include <ogr_api.h>

#include <memory>
#include <set>
#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsOgrFeatureIterator;
class QgsOgrProvider;
class QgsOgrDataset;
using QgsOgrDatasetSharedPtr = std::shared_ptr< QgsOgrDataset>;

class QgsOgrFeatureSource final: public QgsAbstractFeatureSource
{
  public:
    explicit QgsOgrFeatureSource( const QgsOgrProvider *p );
    ~QgsOgrFeatureSource() override;

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

  private:
    QString mDataSource;
    QString mAuthCfg;
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
    QgsTransaction *mTransaction = nullptr;

    friend class QgsOgrFeatureIterator;
    friend class QgsOgrExpressionCompiler;
};

class QgsOgrFeatureIterator final: public QgsAbstractFeatureIteratorFromSource<QgsOgrFeatureSource>
{
  public:
    QgsOgrFeatureIterator( QgsOgrFeatureSource *source, bool ownSource, const QgsFeatureRequest &request, QgsTransaction *transaction );

    ~QgsOgrFeatureIterator() override;

    bool rewind() override;
    bool close() override;

    void setInterruptionChecker( QgsFeedback *interruptionChecker ) override;

  protected:
    bool checkFeature( gdal::ogr_feature_unique_ptr &fet, QgsFeature &feature ) ;
    bool fetchFeature( QgsFeature &feature ) override;
    bool nextFeatureFilterExpression( QgsFeature &f ) override;

  private:

    bool readFeature( const gdal::ogr_feature_unique_ptr &fet, QgsFeature &feature ) const;

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
    QString mAuthCfg;

    QgsFeedback *mInterruptionChecker = nullptr;

    Qgis::SymbolType mSymbolType = Qgis::SymbolType::Hybrid;

    /* This flag tells the iterator when to skip all calls that might reset the reading (rewind),
     * to be used when the request is for a single fid or for a list of fids and we are inside
     * a transaction for SQLITE-based layers */
    bool mAllowResetReading = true;

    QgsGeometry mDistanceWithinGeom;
    std::unique_ptr< QgsGeometryEngine > mDistanceWithinEngine;

    bool fetchFeatureWithId( QgsFeatureId id, QgsFeature &feature ) const;

    void resetReading();
};

///@endcond
#endif // QGSOGRFEATUREITERATOR_H
