/***************************************************************************
  qgsoraclefeatureiterator.h -  QGIS data provider for Oracle layers
                             -------------------
    begin                : December 2012
    copyright            : (C) 2012 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSORACLEFEATUREITERATOR_H
#define QGSORACLEFEATUREITERATOR_H

#include "qgsfeatureiterator.h"

#include <QSqlQuery>

#include "qgsoracleprovider.h"

class QgsOracleConn;
class QgsOracleProvider;


class QgsOracleFeatureSource : public QgsAbstractFeatureSource
{
  public:
    explicit QgsOracleFeatureSource( const QgsOracleProvider* p );

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request );

  protected:
    QgsDataSourceURI mUri;
    QgsFields mFields;

    QString mGeometryColumn;          //! name of the geometry column
    int mSrid;                        //! srid of column
    bool mHasSpatialIndex;            //! has spatial index of geometry column
    QGis::WkbType mDetectedGeomType;  //! geometry type detected in the database
    QGis::WkbType mRequestedGeomType; //! geometry type requested in the uri
    QString mSqlWhereClause;
    QgsOraclePrimaryKeyType mPrimaryKeyType;
    QList<int> mPrimaryKeyAttrs;
    QString mQuery;

    QSharedPointer<QgsOracleSharedData> mShared;

    friend class QgsOracleFeatureIterator;
    friend class QgsOracleExpressionCompiler;
};


class QgsOracleFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsOracleFeatureSource>
{
  public:
    QgsOracleFeatureIterator( QgsOracleFeatureSource* source, bool ownSource, const QgsFeatureRequest &request );

    ~QgsOracleFeatureIterator();

    //! reset the iterator to the starting position
    virtual bool rewind() override;

    //! end of iterating: free the resources / lock
    virtual bool close() override;

  protected:
    //! fetch next feature, return true on success
    virtual bool fetchFeature( QgsFeature& feature ) override;

    //! fetch next feature filter expression
    bool nextFeatureFilterExpression( QgsFeature& f ) override;

    bool openQuery( QString whereClause, bool showLog = true );

    QgsOracleConn *mConnection;
    QSqlQuery mQry;
    bool mRewind;
    bool mExpressionCompiled;
    bool mFetchGeometry;
    QgsAttributeList mAttributeList;
    QString mSql;
};

#endif // QGSORACLEFEATUREITERATOR_H
