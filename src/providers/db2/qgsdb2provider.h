#ifndef QGSDB2PROVIDER_H
#define QGSDB2PROVIDER_H

#include <qgsvectordataprovider.h>
#include <qgscoordinatereferencesystem.h>
#include <QtSql>

class QgsDb2Provider : public QgsVectorDataProvider
{
	Q_OBJECT

  public:
    explicit QgsDb2Provider( QString uri = QString() );

    virtual ~QgsDb2Provider();

    static QSqlDatabase GetDatabase( QString service, QString driver, QString host, int port, QString location, QString username, QString password );

    static bool OpenDatabase( QSqlDatabase db );

    virtual QgsAbstractFeatureSource* featureSource() const override;

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request = QgsFeatureRequest() );

    virtual QGis::WkbType geometryType() const;

    virtual long featureCount() const;

    /** Update the extent for this layer */
    void UpdateStatistics();

    virtual const QgsFields &fields() const;

    virtual QgsCoordinateReferenceSystem crs();

    virtual QgsRectangle extent();

    virtual bool isValid();

    /** Accessor for SQL WHERE clause used to limit dataset */
    QString subsetString() override;

    /** Mutator for SQL WHERE clause used to limit dataset size */
    bool setSubsetString( const QString& theSQL, bool updateFeatureCount = true ) override;

    virtual bool supportsSubsetString() override { return true; }

    virtual QString name() const;

    virtual QString description() const;

  protected:
    /** Loads fields from input file to member attributeFields */
    QVariant::Type DecodeSqlType( int typeId );
    void loadMetadata();
    void loadFields();

  private:
    static void db2WkbTypeAndDimension( QGis::WkbType wkbType, QString &geometryType, int &dim );
    static QString db2TypeName( int typeId );
    static QGis::WkbType getWkbType( const QString& geometryType, int dim );

    QgsFields mAttributeFields; //fields
    QMap<int, QVariant> mDefaultValues;
    QgsRectangle mExtent; //layer extent
    bool mValid;
    bool mUseWkb;
    bool mUseEstimatedMetadata;
    bool mSkipFailures;
    long mNumberFeatures;
    QString mFidColName;
    long mSRId;
    QString mGeometryColName, mGeometryColType;
    QString mLastError; //string containing the last reported error message
    QgsCoordinateReferenceSystem mCrs; //coordinate reference system
    QGis::WkbType mWkbType;
    QSqlQuery mQuery; //current SQL query
    QString mSchemaName, mTableName; //current layer schema/name
    QString mUserName, mPassword; //login
    QString mService, mDatabaseName, mDriver, mHost, mPort; //server access
    QStringList mTables; //available tables
    QString mSqlWhereClause; //SQL statement used to limit the features retrieved
    QSqlDatabase mDatabase; //the database object
    static int sConnectionId;

    //sets the error messages
    void setLastError( const QString& error )
    {
      mLastError = error;
    }

    friend class QgsDb2FeatureSource;
};

#endif