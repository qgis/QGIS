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

#include "qgsdelimitedtextprovider.h"

class QgsDelimitedTextFeatureSource : public QgsAbstractFeatureSource
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
    int mWktFieldIndex;
    bool mWktHasPrefix;
    QgsWkbTypes::GeometryType mGeometryType;
    QString mDecimalPoint;
    bool mXyDms;
    QList<int> attributeColumns;
    QgsCoordinateReferenceSystem mCrs;

    friend class QgsDelimitedTextFeatureIterator;
};


class QgsDelimitedTextFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsDelimitedTextFeatureSource>
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

    // Tests whether the geometry is required, given that testGeometry is true.
    bool wantGeometry( const QgsPointXY &point ) const;
    bool wantGeometry( const QgsGeometry &geom ) const;

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
};


#endif // QGSDELIMITEDTEXTFEATUREITERATOR_H
