/***************************************************************************
      qgsmssqlprovider.h  -  Data provider for mssql server
                             -------------------
    begin                : 2011-10-08
    copyright            : (C) 2011 by Tamas Szekeres
    email                : szekerest at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsvectordataprovider.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectorlayerimport.h"

#include <QStringList>
#include <QFile>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

class QgsFeature;
class QgsField;
class QFile;
class QTextStream;

#include "qgsdatasourceuri.h"
#include "qgsgeometry.h"


/**
\class QgsMssqlGeometryParser
\brief Geometry parser for SqlGeometry/SqlGeography.
*
*/

class QgsMssqlGeometryParser
{

  protected:
    unsigned char* pszData;
    unsigned char* pszWkb;
    int nWkbLen;
    int nWkbMaxLen;
    /* byte order */
    char chByteOrder;
    /* serialization properties */
    char chProps;
    /* point array */
    int nPointSize;
    int nPointPos;
    int nNumPoints;
    /* figure array */
    int nFigurePos;
    int nNumFigures;
    /* shape array */
    int nShapePos;
    int nNumShapes;
    int nSRSId;

  protected:
    void CopyBytes( void* src, int len );
    void CopyCoordinates( unsigned char* src );
    void CopyPoint( int iPoint );
    void ReadPoint( int iShape );
    void ReadMultiPoint( int iShape );
    void ReadLineString( int iShape );
    void ReadMultiLineString( int iShape );
    void ReadPolygon( int iShape );
    void ReadMultiPolygon( int iShape );
    void ReadGeometryCollection( int iShape );

  public:
    QgsMssqlGeometryParser();
    unsigned char* ParseSqlGeometry( unsigned char* pszInput, int nLen );
    int GetSRSId() { return nSRSId; };
    int GetWkbLen() { return nWkbLen; };
    void DumpMemoryToLog( const char* pszMsg, unsigned char* pszInput, int nLen );
    /* sql geo type */
    bool IsGeography;
};


/**
\class QgsMssqlProvider
\brief Data provider for mssql server.
*
*/
class QgsMssqlProvider : public QgsVectorDataProvider
{
    Q_OBJECT

  public:

    QgsMssqlProvider( QString uri = QString() );

    virtual ~QgsMssqlProvider();

    static QSqlDatabase GetDatabase( QString driver, QString host, QString database, QString username, QString password );
    static bool OpenDatabase( QSqlDatabase db );

    /* Implementation of functions from QgsVectorDataProvider */

    /**
     * Returns the permanent storage type for this layer as a friendly name.
     */
    virtual QString storageType() const;

    /**
     * Sub-layers handled by this provider, in order from bottom to top
     *
     * Sub-layers are used when the provider's source can combine layers
     * it knows about in some way before it hands them off to the provider.
     */
    virtual QStringList subLayers() const;

    /** Select features based on a bounding rectangle. Features can be retrieved with calls to nextFeature.
     *  @param fetchAttributes list of attributes which should be fetched
     *  @param rect spatial filter
     *  @param fetchGeometry true if the feature geometry should be fetched
     *  @param useIntersect true if an accurate intersection test should be used,
     *                     false if a test based on bounding box is sufficient
     */
    virtual void select( QgsAttributeList fetchAttributes = QgsAttributeList(),
                         QgsRectangle rect = QgsRectangle(),
                         bool fetchGeometry = true,
                         bool useIntersect = false );

    /**
     * Get the next feature resulting from a select operation.
     * @param feature feature which will receive data from the provider
     * @return true when there was a feature to fetch, false when end was hit
     *
     * mFile should be open with the file pointer at the record of the next
     * feature, or EOF.  The feature found on the current line is parsed.
     */
    virtual bool nextFeature( QgsFeature& feature );

    /**
     * Gets the feature at the given feature ID.
     * @param featureId id of the feature
     * @param feature feature which will receive the data
     * @param fetchGeoemtry if true, geometry will be fetched from the provider
     * @param fetchAttributes a list containing the indexes of the attribute fields to copy
     * @return True when feature was found, otherwise false
     */
    virtual bool featureAtId( QgsFeatureId featureId,
                              QgsFeature& feature,
                              bool fetchGeometry = true,
                              QgsAttributeList fetchAttributes = QgsAttributeList() );

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
     * Number of attribute fields for a feature in the layer
     */
    virtual uint fieldCount() const;

    /** update the extent, feature count, wkb type and srid for this layer */
    void UpdateStatistics( bool estimate );

    /**
     * Return a map of indexes with field names for this layer
     * @return map of fields
     */
    virtual const QgsFieldMap & fields() const;

    /** Restart reading features from previous select operation */
    virtual void rewind();

    /** Returns a bitmask containing the supported capabilities
        Note, some capabilities may change depending on whether
        a spatial filter is active on this provider, so it may
        be prudent to check this value per intended operation.
     */
    virtual int capabilities() const;


    /* Implementation of functions from QgsDataProvider */

    /** return a provider name

        Essentially just returns the provider key.  Should be used to build file
        dialogs so that providers can be shown with their supported types. Thus
        if more than one provider supports a given format, the user is able to
        select a specific provider to open that file.

        @note

        Instead of being pure virtual, might be better to generalize this
        behavior and presume that none of the sub-classes are going to do
        anything strange with regards to their name or description?
     */
    QString name() const;

    /** return description

        Return a terse string describing what the provider is.

        @note

        Instead of being pure virtual, might be better to generalize this
        behavior and presume that none of the sub-classes are going to do
        anything strange with regards to their name or description?
     */
    QString description() const;

    /**
     * Return the extent for this data layer
     */
    virtual QgsRectangle extent();

    /**
     * Returns true if this is a valid data source
     */
    bool isValid();

    /**Writes a list of features to the database*/
    virtual bool addFeatures( QgsFeatureList & flist );

    /**Deletes a feature*/
    virtual bool deleteFeatures( const QgsFeatureIds & id );

    /**
     * Adds new attributes
     * @param attributes list of new attributes
     * @return true in case of success and false in case of failure
     * @note added in 1.2
     */
    virtual bool addAttributes( const QList<QgsField> &attributes );

    /**
     * Deletes existing attributes
     * @param attributes a set containing names of attributes
     * @return true in case of success and false in case of failure
     */
    virtual bool deleteAttributes( const QgsAttributeIds &attributes );

    /**Changes attribute values of existing features */
    virtual bool changeAttributeValues( const QgsChangedAttributesMap & attr_map );

    /**Changes existing geometries*/
    virtual bool changeGeometryValues( QgsGeometryMap & geometry_map );

    /**
     * Create a spatial index for the current layer
     */
    virtual bool createSpatialIndex();

    /**Create an attribute index on the datasource*/
    virtual bool createAttributeIndex( int field );

    /** convert a QgsField to work with MSSQL */
    static bool convertField( QgsField &field );

    /** Import a vector layer into the database */
    static QgsVectorLayerImport::ImportError createEmptyLayer(
      const QString& uri,
      const QgsFieldMap &fields,
      QGis::WkbType wkbType,
      const QgsCoordinateReferenceSystem *srs,
      bool overwrite,
      QMap<int, int> *oldToNewAttrIdxMap,
      QString *errorMessage = 0,
      const QMap<QString, QVariant> *options = 0
    );

    virtual QgsCoordinateReferenceSystem crs();

  protected:
    /** loads fields from input file to member attributeFields */
    QVariant::Type DecodeSqlType( QString sqlTypeName );
    void loadFields();
    void loadMetadata();

  private:

    //! Fields
    QgsFieldMap mAttributeFields;

    QgsMssqlGeometryParser parser;

    int mFieldCount;  // Note: this includes field count for wkt field

    //! Layer extent
    QgsRectangle mExtent;

    bool mValid;

    bool mUseWkb;
    bool mUseEstimatedMetadata;
    bool mSkipFailures;

    int mGeomType;

    long mNumberFeatures;
    long mFidCol;
    QString mFidColName;
    long mSRId;
    long mGeometryCol;
    QString mGeometryColName;
    QString mGeometryColType;

    // QString containing the last reported error message
    QString mLastError;

    // Coordinate reference sytem
    QgsCoordinateReferenceSystem mCrs;

    QGis::WkbType mWkbType;

    // The database object
    QSqlDatabase mDatabase;

    // The current sql query
    QSqlQuery mQuery;

    // The current sql statement
    QString mStatement;

    // current layer name
    QString mSchemaName;
    QString mTableName;
    // available tables
    QStringList mTables;

    // Sets the error messages
    void setLastError( QString error )
    {
      mLastError = error;
    }

    static void mssqlWkbTypeAndDimension( QGis::WkbType wkbType, QString &geometryType, int &dim );
    static QGis::WkbType getWkbType( QString geometryType, int dim );
};
