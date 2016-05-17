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
    explicit QgsDelimitedTextFeatureSource( const QgsDelimitedTextProvider* p );
    ~QgsDelimitedTextFeatureSource();

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request ) override;

  protected:
    QgsDelimitedTextProvider::GeomRepresentationType mGeomRep;
    QgsExpression *mSubsetExpression;
    QgsExpressionContext mExpressionContext;
    QgsRectangle mExtent;
    bool mUseSpatialIndex;
    QgsSpatialIndex *mSpatialIndex;
    bool mUseSubsetIndex;
    QList<quintptr> mSubsetIndex;
    QgsDelimitedTextFile *mFile;
    QgsFields mFields;
    int mFieldCount;  // Note: this includes field count for wkt field
    int mXFieldIndex;
    int mYFieldIndex;
    int mWktFieldIndex;
    bool mWktHasPrefix;
    QGis::GeometryType mGeometryType;
    QString mDecimalPoint;
    bool mXyDms;
    QList<int> attributeColumns;

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
    QgsDelimitedTextFeatureIterator( QgsDelimitedTextFeatureSource* source, bool ownSource, const QgsFeatureRequest& request );

    ~QgsDelimitedTextFeatureIterator();

    //! reset the iterator to the starting position
    virtual bool rewind() override;

    //! end of iterating: free the resources / lock
    virtual bool close() override;

    // Tests whether the geometry is required, given that testGeometry is true.
    bool wantGeometry( const QgsPoint & point ) const;
    bool wantGeometry( QgsGeometry *geom ) const;

  protected:
    //! fetch next feature, return true on success
    virtual bool fetchFeature( QgsFeature& feature ) override;

    bool setNextFeatureId( qint64 fid );

    bool nextFeatureInternal( QgsFeature& feature );
    QgsGeometry* loadGeometryWkt( const QStringList& tokens, bool &isNull );
    QgsGeometry* loadGeometryXY( const QStringList& tokens, bool &isNull );
    void fetchAttribute( QgsFeature& feature, int fieldIdx, const QStringList& tokens );

    QList<QgsFeatureId> mFeatureIds;
    IteratorMode mMode;
    long mNextId;
    bool mTestSubset;
    bool mTestGeometry;
    bool mTestGeometryExact;
    bool mLoadGeometry;
};


#endif // QGSDELIMITEDTEXTFEATUREITERATOR_H
