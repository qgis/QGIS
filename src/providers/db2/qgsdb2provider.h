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
#include "qgsvectorlayerexporter.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsgeometry.h"
#include "qgsfields.h"
#include <QtSql>

/**
 * \class QgsDb2Provider
 * \brief Data provider for DB2 server.
 */
class QgsDb2Provider : public QgsVectorDataProvider
{
    Q_OBJECT

  public:
    explicit QgsDb2Provider( const QString &uri = QString() );

    virtual ~QgsDb2Provider();

    /**
     * Returns a QSqlDatabase object that can connect to DB2 for LUW or z/OS.
     *
     * If service is provided, then username and password is required.
     * If service is not provided, the remaining arguments are required.
     *
     * \param connInfo A string containing all connection information.
     */
    static QSqlDatabase getDatabase( const QString &connInfo, QString &errMsg );

    static bool openDatabase( QSqlDatabase db );

    virtual QgsAbstractFeatureSource *featureSource() const override;

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const override;

    virtual QgsWkbTypes::Type wkbType() const override;

    virtual long featureCount() const override;

    /**
     * Update the extent for this layer.
     */
    void updateStatistics() const;


    virtual QgsFields fields() const override;

    virtual QgsCoordinateReferenceSystem crs() const override;
    virtual QgsRectangle extent() const override;
    virtual bool isValid() const override;
    QString subsetString() const override;

    bool setSubsetString( const QString &theSQL, bool updateFeatureCount = true ) override;

    virtual bool supportsSubsetString() const override { return true; }

    virtual QString name() const override;

    virtual QString description() const override;

    QgsAttributeList pkAttributeIndexes() const override;

    virtual QgsVectorDataProvider::Capabilities capabilities() const override;

    virtual bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = 0 ) override;

    virtual bool deleteFeatures( const QgsFeatureIds &id ) override;

    virtual bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;

    virtual bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;

    //! Import a vector layer into the database
    static QgsVectorLayerExporter::ExportError createEmptyLayer(
      const QString &uri,
      const QgsFields &fields,
      QgsWkbTypes::Type wkbType,
      const QgsCoordinateReferenceSystem &srs,
      bool overwrite,
      QMap<int, int> *oldToNewAttrIdxMap,
      QString *errorMessage = nullptr,
      const QMap<QString, QVariant> *options = nullptr
    );

    //! Convert a QgsField to work with DB2
    static bool convertField( QgsField &field );

    //! Convert a QgsField to work with DB2
    static QString qgsFieldToDb2Field( const QgsField &field );

  protected:
    //! Loads fields from input file to member attributeFields
    QVariant::Type decodeSqlType( int typeId );
    void loadMetadata();
    void loadFields();

  private:
    static void db2WkbTypeAndDimension( QgsWkbTypes::Type wkbType, QString &geometryType, int &dim );
    static QString db2TypeName( int typeId );

    QgsFields mAttributeFields; //fields
    QMap<int, QVariant> mDefaultValues;
    mutable QgsRectangle mExtent; //layer extent
    bool mValid;
    bool mUseEstimatedMetadata;
    bool mSkipFailures;
    long mNumberFeatures = 0;
    int mFidColIdx = -1;
    QString mFidColName;
    QString mExtents;
    mutable long mSRId;
    mutable int  mEnvironment;
    mutable QString mSrsName;
    mutable QString mGeometryColName, mGeometryColType;
    QString mLastError; //string containing the last reported error message
    mutable QgsCoordinateReferenceSystem mCrs; //coordinate reference system
    QgsWkbTypes::Type mWkbType = QgsWkbTypes::Unknown;
    QSqlQuery mQuery; //current SQL query
    QString mConnInfo; // full connection information
    QString mSchemaName, mTableName; //current layer schema/name
    QString mSqlWhereClause; //SQL statement used to limit the features retrieved
    QSqlDatabase mDatabase; //the database object
    static int sConnectionId;

    //sets the error messages
    void setLastError( const QString &error )
    {
      mLastError = error;
    }

    friend class QgsDb2FeatureSource;
};

#endif
