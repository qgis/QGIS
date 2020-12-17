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
#ifndef HSLLCPDALFEATUREITERATOR_H
#define HSLLCPDALFEATUREITERATOR_H

#include "qgsfeatureiterator.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgsgeometryengine.h"
#include <memory>
#include <set>

#include <Geometry.h>
#include <PointArray.h>

class QgsPdalFeatureIterator;
class QgsPdalProvider;
//class QgsPdalDataset;
//using QgsPdalDatasetSharedPtr = std::shared_ptr< QgsPdalDataset>;

class QgsPdalFeatureSource : public QgsAbstractFeatureSource
{
  public:
    explicit QgsPdalFeatureSource( const QgsPdalProvider *p );
    ~QgsPdalFeatureSource() override;

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;
	std::shared_ptr<Geometry> mPointCloudData;


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

    QgsCoordinateReferenceSystem mCrs;
    QgsWkbTypes::Type mWkbType = QgsWkbTypes::MultiPointZ;


    friend class QgsPdalFeatureIterator;
};

class QgsPdalFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsPdalFeatureSource>
{
  public:
    QgsPdalFeatureIterator( QgsPdalFeatureSource *source, bool ownSource, const QgsFeatureRequest &request );

    ~QgsPdalFeatureIterator() override;

    bool rewind() override;
    bool close() override;

  protected:
    bool checkFeature(std::shared_ptr<Geometry>, QgsFeature &feature) ;
    bool fetchFeature( QgsFeature &feature ) override;
    bool nextFeatureFilterExpression( QgsFeature &f ) override;

	bool checkFeatureSinglePoint(std::shared_ptr<Geometry> geom, QgsFeature & feature);

  private:

	QgsGeometry mSelectRectGeom;
	std::unique_ptr< QgsGeometryEngine > mSelectRectEngine;
	bool mClosed = false;
	V3f* m_P;
    bool readFeature( ) const;

    //! Sets to true, if geometry is in the requested columns
    bool mFetchGeometry = false;

    bool mExpressionCompiled = false;
    // use std::set to get sorted ids (needed for efficient QgsFeatureRequest::FilterFids requests on OSM datasource)
    std::set<QgsFeatureId> mFilterFids;
    std::set<QgsFeatureId>::iterator mFilterFidsIt;

    QgsRectangle mFilterRect;
    QgsCoordinateTransform mTransform;
	bool mUsingFeatureIdList = false;
	DrawCount mdrawlist;// �洢�����Ƶ��list  

    bool mFirstFieldIsFid = false;
    QgsFields mFieldsWithoutFid;
	QgsPdalFeatureSource *m_source;
   // bool fetchFeatureWithId( QgsFeatureId id, QgsFeature &feature ) const;
	const std::vector<PointCloudGeomField>* m_pointarrayfields =nullptr;
   // void resetReading();
	int decimal_step=1;
	int count = 0;
	int m_iterration = 0;
	size_t classificationindex = 0;
	size_t Positionindex = 0;
	size_t intensityindex = 0;
	size_t returnNumberindex = 0;
	size_t numberofreturnindex = 0;
	size_t pointsoureindex = 0;
	size_t colorindex = 0;
	V3d offset;
};

#endif // HSLLCPDALFEATUREITERATOR_H
