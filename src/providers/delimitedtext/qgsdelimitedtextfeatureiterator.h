/***************************************************************************
    qgsdelimitedtextfeatureiterator.h
    ---------------------
    begin                : Oktober 2012
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
#ifndef QGSDELIMITEDTEXTFEATUREITERATOR_H
#define QGSDELIMITEDTEXTFEATUREITERATOR_H

#include <QList>
#include "qgsfeatureiterator.h"
#include "qgsfeature.h"
#include "qgsexpressioncontext.h"
#include "qgscoordinatetransform.h"

#include "qgsdelimitedtextprovider.h"

class QgsDelimitedTextFeatureSource final: public QgsAbstractFeatureSource
{
  public:
    explicit QgsDelimitedTextFeatureSource( const QgsDelimitedTextProvider *p );

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

  private:
    QgsDelimitedTextProvider::GeomRepresentationType mGeomRep;
    std::unique_ptr< QgsExpression > mSubsetExpression;
    QgsExpressionContext mExpressionContext;
    QgsRectangle mExtent;
    bool mUseSpatialIndex;
    std::unique_ptr< QgsSpatialIndex > mSpatialIndex;
    bool mUseSubsetIndex;
    QList<quintptr> mSubsetIndex;
    std::unique_ptr< QgsDelimitedTextFile > mFile;
    QgsFields mFields;
    int mFieldCount;  // Note: this includes field count for wkt field
    int mXFieldIndex;
    int mYFieldIndex;
    int mZFieldIndex;
    int mMFieldIndex;
    int mWktFieldIndex;
    bool mWktHasPrefix;
    QgsWkbTypes::GeometryType mGeometryType;
    QString mDecimalPoint;
    bool mXyDms;
    QList<int> attributeColumns;
    QgsCoordinateReferenceSystem mCrs;
    QMap<int, QPair<QString, QString>> mFieldBooleanLiterals;

    friend class QgsDelimitedTextFeatureIterator;
};


class QgsDelimitedTextFeatureIterator final: public QgsAbstractFeatureIteratorFromSource<QgsDelimitedTextFeatureSource>
{
    enum IteratorMode
    {
      FileScan,
      SubsetIndex,
      FeatureIds
    };
  public:
    QgsDelimitedTextFeatureIterator( QgsDelimitedTextFeatureSource *source, bool ownSource, const QgsFeatureRequest &request );

    ~QgsDelimitedTextFeatureIterator() override;

    bool rewind() override;
    bool close() override;

    /**
     * Check to see if the point is within the selection rectangle or within
     * the desired distance from the reference geometry.
     */
    bool testSpatialFilter( const QgsPointXY &point ) const;

    /**
     * Check to see if the geometry is within the selection rectangle or within
     * the desired distance from the reference geometry.
     */
    bool testSpatialFilter( const QgsGeometry &geom ) const;

  protected:
    bool fetchFeature( QgsFeature &feature ) override;

  private:

    bool setNextFeatureId( qint64 fid );

    bool nextFeatureInternal( QgsFeature &feature );
    QgsGeometry loadGeometryWkt( const QStringList &tokens, bool &isNull );
    QgsGeometry loadGeometryXY( const QStringList &tokens, bool &isNull );
    void fetchAttribute( QgsFeature &feature, int fieldIdx, const QStringList &tokens );

    QList<QgsFeatureId> mFeatureIds;
    IteratorMode mMode = FileScan;
    long mNextId = 0;
    bool mTestSubset = false;
    bool mTestGeometry = false;
    bool mTestGeometryExact = false;
    bool mLoadGeometry = false;
    QgsRectangle mFilterRect;
    QgsCoordinateTransform mTransform;

    QgsGeometry mDistanceWithinGeom;
    std::unique_ptr< QgsGeometryEngine > mDistanceWithinEngine;
};


#endif // QGSDELIMITEDTEXTFEATUREITERATOR_H
