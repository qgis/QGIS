/***************************************************************************
  qgsdb2provider.h - Data provider for DB2 server
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

#ifndef QGSDB2PROVIDER_H
#define QGSDB2PROVIDER_H

#include "qgsvectordataprovider.h"
#include "qgsvectorlayerimport.h"
#include <qgscoordinatereferencesystem.h>
#include "qgsgeometry.h"
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
     * @param connInfo A string containing all connection information.
     */
    static QSqlDatabase getDatabase( const QString &connInfo, QString &errMsg );

    static bool openDatabase( QSqlDatabase db );

    virtual QgsAbstractFeatureSource* featureSource() const override;

    /**
     * Get feature iterator.
     * @return QgsFeatureIterator to iterate features.
     */
    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request = QgsFeatureRequest() ) override;

    /**
     * Get feature type.
     * @return int representing the feature type
     */
    virtual QGis::WkbType geometryType() const override;

    /**
     * Number of features in the layer
     * @return long containing number of features
     */
    virtual long featureCount() const override;

    /**
     * Update the extent for this layer.
     */
    void updateStatistics();

    /**
     * Return a map of indexes with field names for this layer.
     * @return map of fields
     */
    virtual const QgsFields &fields() const override;

    virtual QgsCoordinateReferenceSystem crs() override;

    /**
     * Return the extent for this data layer.
     */
    virtual QgsRectangle extent() override;

    /**
     * Returns true if this is a valid data source.
     */
    virtual bool isValid() override;

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
    virtual QString name() const override;

    /** Return description

        Return a terse string describing what the provider is.
     */
    virtual QString description() const override;

    /** Returns a bitmask containing the supported capabilities
        Note, some capabilities may change depending on whether
        a spatial filter is active on this provider, so it may
        be prudent to check this value per intended operation.
     */
    virtual int capabilities() const override;

    /** Writes a list of features to the database*/
    virtual bool addFeatures( QgsFeatureList & flist ) override;

    /** Deletes a feature*/
    virtual bool deleteFeatures( const QgsFeatureIds & id ) override;

    /** Changes attribute values of existing features */
    virtual bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;

    /** Changes existing geometries*/
    virtual bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;

    /** Import a vector layer into the database */
    static QgsVectorLayerImport::ImportError createEmptyLayer(
      const QString& uri,
      const QgsFields &fields,
      QGis::WkbType wkbType,
      const QgsCoordinateReferenceSystem *srs,
      bool overwrite,
      QMap<int, int> *oldToNewAttrIdxMap,
      QString *errorMessage = nullptr,
      const QMap<QString, QVariant> *options = nullptr
    );

    /** Convert a QgsField to work with DB2 */
    static bool convertField( QgsField &field );

    /** Convert a QgsField to work with DB2 */
    static QString qgsFieldToDb2Field( QgsField field );

  protected:
    /** Loads fields from input file to member attributeFields */
    QVariant::Type decodeSqlType( int typeId );
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
    int  mEnvironment;
    QString mSrsName;
    QString mGeometryColName, mGeometryColType;
    QString mLastError; //string containing the last reported error message
    QgsCoordinateReferenceSystem mCrs; //coordinate reference system
    QGis::WkbType mWkbType;
    QSqlQuery mQuery; //current SQL query
    QString mConnInfo; // full connection information
    QString mSchemaName, mTableName; //current layer schema/name
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