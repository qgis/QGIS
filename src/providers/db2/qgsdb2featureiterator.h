/***************************************************************************
  qgsdb2featureiterator.h - DB2 spatial feature processing
  --------------------------------------
  Date      : 2016-01-27
  Copyright : (C) 2016 by David Adler
                          Shirley Xiao, David Nguyen
  Email     : dadler at adtechgeospatial.com
              xshirley2012 at yahoo.com, davidng0123 at gmail.com
****************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/

#ifndef QGSDB2FEATUREITERATOR_H
#define QGSDB2FEATUREITERATOR_H

#include "qgsfeatureiterator.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

class QgsDb2Provider;

class QgsDb2FeatureSource : public QgsAbstractFeatureSource
{
  public:
    explicit QgsDb2FeatureSource( const QgsDb2Provider* p );
    ~QgsDb2FeatureSource();

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request ) override;

  protected:
    QgsFields mFields;
    QString mFidColName;
    long mSRId;

    QString mGeometryColName;
    QString mGeometryColType;

    // current layer name
    QString mSchemaName;
    QString mTableName;

    // server access
    QString mConnInfo;

    // SQL statement used to limit the features retrieved
    QString mSqlWhereClause;

    // Return True if this feature source has spatial attributes.
    bool isSpatial() { return !mGeometryColName.isEmpty() || !mGeometryColType.isEmpty(); }

    friend class QgsDb2FeatureIterator;
    friend class QgsDb2ExpressionCompiler;
};

class QgsDb2FeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsDb2FeatureSource>
{
  public:
    QgsDb2FeatureIterator( QgsDb2FeatureSource* source, bool ownSource, const QgsFeatureRequest& request );

    ~QgsDb2FeatureIterator();

    //! reset the iterator to the starting position
    virtual bool rewind() override;

    //! end of iterating: free the resources / lock
    virtual bool close() override;

  protected:
    void BuildStatement( const QgsFeatureRequest& request );

    //! fetch next feature, return true on success
    virtual bool fetchFeature( QgsFeature& feature ) override;

    //! fetch next feature filter expression
    bool nextFeatureFilterExpression( QgsFeature& f ) override;

  private:

    virtual bool prepareOrderBy( const QList<QgsFeatureRequest::OrderByClause> &orderBys ) override;


    // The current database
    QSqlDatabase mDatabase;
    QString mOrderByClause;

    // The current sql query
    QSqlQuery* mQuery;

    // The current sql statement
    QString mStatement;

    // Field index of FID column
    long mFidCol;

    // List of attribute indices to fetch with nextFeature calls
    QgsAttributeList mAttributesToFetch;

    bool mExpressionCompiled;
    bool mOrderByCompiled;

    int mFetchCount;
};

#endif // QGSDB2FEATUREITERATOR_H
