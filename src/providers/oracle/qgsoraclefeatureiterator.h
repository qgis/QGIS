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
    explicit QgsOracleFeatureSource( const QgsOracleProvider *p );

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) override;

  protected:
    QgsDataSourceUri mUri;
    QgsFields mFields;

    QString mGeometryColumn;          //! name of the geometry column
    int mSrid;                        //! srid of column
    bool mHasSpatialIndex;            //! has spatial index of geometry column
    QgsWkbTypes::Type mDetectedGeomType;  //! geometry type detected in the database
    QgsWkbTypes::Type mRequestedGeomType; //! geometry type requested in the uri
    QString mSqlWhereClause;
    QgsOraclePrimaryKeyType mPrimaryKeyType;
    QList<int> mPrimaryKeyAttrs;
    QString mQuery;
    QgsCoordinateReferenceSystem mCrs;

    std::shared_ptr<QgsOracleSharedData> mShared;

    friend class QgsOracleFeatureIterator;
    friend class QgsOracleExpressionCompiler;
};


class QgsOracleFeatureIterator : public QgsAbstractFeatureIteratorFromSource<QgsOracleFeatureSource>
{
  public:
    QgsOracleFeatureIterator( QgsOracleFeatureSource *source, bool ownSource, const QgsFeatureRequest &request );

    ~QgsOracleFeatureIterator() override;

    bool rewind() override;
    bool close() override;

  protected:
    bool fetchFeature( QgsFeature &feature ) override;
    bool nextFeatureFilterExpression( QgsFeature &f ) override;

    bool openQuery( const QString &whereClause, const QVariantList &args, bool showLog = true );

    QgsOracleConn *mConnection = nullptr;
    QSqlQuery mQry;
    bool mRewind = false;
    bool mExpressionCompiled = false;
    bool mFetchGeometry = false;
    QgsAttributeList mAttributeList;
    QString mSql;
    QVariantList mArgs;

    QgsCoordinateTransform mTransform;
    QgsRectangle mFilterRect;
};

#endif // QGSORACLEFEATUREITERATOR_H
