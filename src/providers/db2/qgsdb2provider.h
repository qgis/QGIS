/***************************************************************************
  qgsdb2provider.h - Data provider for DB2 server
  --------------------------------------
  Date      : 2016-01-27
  Copyright : (C) 2016 by David Adler
                          Shirley Xiao, David Nguyen
  Email     : dadler at adtechgeospatial.com
              xshirley2012 at yahoo.com, davidng0123 at gmail.com
/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/

#ifndef QGSDB2PROVIDER_H
#define QGSDB2PROVIDER_H

#include <qgsvectordataprovider.h>
#include <qgscoordinatereferencesystem.h>
#include <QtSql>

/**
 * @class QgsDb2Provider
 * @brief Data provider for DB2 server.
 */
class QgsDb2Provider : public QgsVectorDataProvider
{
    Q_OBJECT

  public:
    explicit QgsDb2Provider( QString uri = QString() );

    virtual ~QgsDb2Provider();

    /**
     * Returns a QSqlDatabase object that can connect to DB2 for LUW or z/OS.
     *
     * If service is provided, then username and password is required.
     * If service is not provided, the remaining arguments are required.
     *
     * @param service The DSN name.
     * @param driver The full driver name.
     * @param host The host name.
     * @param port The port number.
     * @param location The database/location name.
     * @param username The username.
     * @param password The password.
     */
    static QSqlDatabase GetDatabase( QString service, QString driver, QString host, int port, QString location, QString username, QString password );

    static bool OpenDatabase( QSqlDatabase db );

    virtual QgsAbstractFeatureSource* featureSource() const override;

    /**
     * Get feature iterator.
     * @return QgsFeatureIterator to iterate features.
     */
    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request = QgsFeatureRequest() );

    /**
     * Get feature type.
     * @return int representing the feature type
     */
    virtual QGis::WkbType geometryType() const;

    /**
     * Number of features in the layer
     * @return long containing number of features
     */
    virtual long featureCount() const;

    /**
     * Update the extent for this layer.
     */
    void UpdateStatistics();

    /**
     * Return a map of indexes with field names for this layer.
     * @return map of fields
     */
    virtual const QgsFields &fields() const;

    virtual QgsCoordinateReferenceSystem crs();

    /**
     * Return the extent for this data layer.
     */
    virtual QgsRectangle extent();

    /**
     * Returns true if this is a valid data source.
     */
    virtual bool isValid();

    /**
     * Accessor for SQL WHERE clause used to limit dataset.
     */
    QString subsetString() override;

    /**
     * Mutator for SQL WHERE clause used to limit dataset size.
     */
    bool setSubsetString( const QString& theSQL, bool updateFeatureCount = true ) override;

    virtual bool supportsSubsetString() override { return true; }

    /** Return a provider name

        Essentially just returns the provider key.  Should be used to build file
        dialogs so that providers can be shown with their supported types. Thus
        if more than one provider supports a given format, the user is able to
        select a specific provider to open that file.
     */
    virtual QString name() const;

    /** Return description

        Return a terse string describing what the provider is.
     */
    virtual QString description() const;

  protected:
    /** Loads fields from input file to member attributeFields */
    QVariant::Type DecodeSqlType( int typeId );
    void loadMetadata();
    void loadFields();

  private:
    static void db2WkbTypeAndDimension( QGis::WkbType wkbType, QString &geometryType, int &dim );
    static QString db2TypeName( int typeId );

    QgsFields mAttributeFields; //fields
    QMap<int, QVariant> mDefaultValues;
    QgsRectangle mExtent; //layer extent
    bool mValid;
    bool mUseEstimatedMetadata;
    bool mSkipFailures;
    long mNumberFeatures;
    QString mFidColName;
    QString mExtents;
    long mSRId;
    QString mGeometryColName, mGeometryColType;
    QString mLastError; //string containing the last reported error message
    QgsCoordinateReferenceSystem mCrs; //coordinate reference system
    QGis::WkbType mWkbType;
    QSqlQuery mQuery; //current SQL query
    QString mSchemaName, mTableName; //current layer schema/name
    QString mUserName, mPassword; //login
    QString mService, mDatabaseName, mDriver, mHost, mPort; //server access
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