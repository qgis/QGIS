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

    // login
    QString mUserName;
    QString mPassword;

    // server access
    QString mService;
    QString mDatabaseName;
    QString mHost;



    QString mDriver;
    QString mPort;


    // SQL statement used to limit the features retrieved
    QString mSqlWhereClause;

    // Return True if this feature source has spatial attributes.
    bool isSpatial() { return !mGeometryColName.isEmpty() || !mGeometryColType.isEmpty(); }

    friend class QgsDb2FeatureIterator;
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

  private:
    //! fetch next feature, return true on success
    virtual bool fetchFeature( QgsFeature& feature ) override;

    // The current database
    QSqlDatabase mDatabase;

    // The current sql query
    QSqlQuery* mQuery;

    // The current sql statement
    QString mStatement;

    // Field index of FID column
    long mFidCol;

    // List of attribute indices to fetch with nextFeature calls
    QgsAttributeList mAttributesToFetch;
};

#endif // QGSDB2FEATUREITERATOR_H
