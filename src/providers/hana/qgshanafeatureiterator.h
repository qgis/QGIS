/***************************************************************************
   qgshanafeatureiterator.h
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
***************************************************************************/
#ifndef QGSHANAFEATUREITERATOR_H
#define QGSHANAFEATUREITERATOR_H

#include "qgsfeatureiterator.h"
#include "qgshanaconnectionpool.h"
#include "qgshanaprimarykeys.h"
#include "qgshanaprovider.h"
#include "qgshanaresultset.h"
#include "qgscoordinatetransform.h"

#include "odbc/Forwards.h"

class QgsHanaFeatureSource : public QgsAbstractFeatureSource
{
  public:
    explicit QgsHanaFeatureSource( const QgsHanaProvider *p );
    ~QgsHanaFeatureSource() override;

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

  private:
    bool isSpatial() const { return !mGeometryColumn.isEmpty() && mGeometryType != QgsWkbTypes::Unknown; }

  private:
    QVersionNumber mDatabaseVersion;
    QgsDataSourceUri mUri;
    QString mQuery;
    QString mQueryWhereClause;
    QgsHanaPrimaryKeyType mPrimaryKeyType = QgsHanaPrimaryKeyType::PktUnknown;
    QList<int> mPrimaryKeyAttrs;
    std::shared_ptr<QgsHanaPrimaryKeyContext> mPrimaryKeyCntx;
    QgsFields mFields;
    QString mGeometryColumn;
    QgsWkbTypes::Type mGeometryType;
    int mSrid;
    QgsRectangle mSrsExtent;
    QgsCoordinateReferenceSystem mCrs;

    friend class QgsHanaFeatureIterator;
    friend class QgsHanaExpressionCompiler;
};

class QgsHanaFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsHanaFeatureSource>
{
  public:
    QgsHanaFeatureIterator(
      QgsHanaFeatureSource *source,
      bool ownSource,
      const QgsFeatureRequest &request );

    ~QgsHanaFeatureIterator() override;

    bool rewind() override;
    bool close() override;

  protected:
    bool fetchFeature( QgsFeature &feature ) override;
    bool nextFeatureFilterExpression( QgsFeature &feature ) override;

  private:
    bool prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause> &orderBys ) override;

    QString buildSqlQuery( const QgsFeatureRequest &request );
    QVariantList buildSqlQueryParameters( ) const;
    QString getBBOXFilter() const;
    QgsRectangle getFilterRect() const;

  private:
    const QVersionNumber mDatabaseVersion;
    QgsHanaConnectionRef mConnection;
    QgsHanaResultSetRef mResultSet;
    QString mSqlQuery;
    QVariantList mSqlQueryParams;
    QgsRectangle mFilterRect;
    QgsGeometry mDistanceWithinGeom;
    std::unique_ptr< QgsGeometryEngine > mDistanceWithinEngine;
    QgsAttributeList mAttributesToFetch;
    QgsCoordinateTransform mTransform;
    bool mHasAttributes = false;
    bool mHasGeometryColumn = false;
    bool mExpressionCompiled = false;
    bool mOrderByCompiled = false;
};

#endif // QGSHANAFEATUREITERATOR_H
