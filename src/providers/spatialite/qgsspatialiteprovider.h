/***************************************************************************
            qgsspatialiteprovider.h Data provider for SpatiaLite DBMS
begin                : Dec 2008
copyright            : (C) 2008 Sandro Furieri
email                : a.furieri@lqt.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

extern "C"
{
#include <sys/types.h>
#include <sqlite3.h>
#include <spatialite/gaiageo.h>
#include <spatialite.h>
}

#include "qgsvectordataprovider.h"
#include "qgsrectangle.h"
#include "qgsvectorlayerimport.h"
#include <list>
#include <queue>
#include <fstream>
#include <set>

class QgsFeature;
class QgsField;

#include "qgsdatasourceuri.h"

/**
  \class QgsSpatiaLiteProvider
  \brief Data provider for SQLite/SpatiaLite layers.

  This provider implements the
  interface defined in the QgsDataProvider class to provide access to spatial
  data residing in a SQLite/SpatiaLite enabled database.
  */
class QgsSpatiaLiteProvider: public QgsVectorDataProvider
{
  Q_OBJECT public:

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

    /**
     * Constructor of the vector provider
     * @param uri  uniform resource locator (URI) for a dataset
     */
    QgsSpatiaLiteProvider( QString const &uri = "" );

    //! Destructor
    virtual ~ QgsSpatiaLiteProvider();

    /**
        *   Returns the permanent storage type for this layer as a friendly name.
        */
    virtual QString storageType() const;

    /*! Get the QgsCoordinateReferenceSystem for this layer
     * @note Must be reimplemented by each provider.
     * If the provider isn't capable of returning
     * its projection an empty srs will be return, ti will return 0
     */
    virtual QgsCoordinateReferenceSystem crs();

    /** Select features based on a bounding rectangle. Features can be retrieved with calls to nextFeature.
      *  @param fetchAttributes list of attributes which should be fetched
      *  @param rect spatial filter
      *  @param fetchGeometry true if the feature geometry should be fetched
      *  @param useIntersect true if an accurate intersection test should be used,
      *                     false if a test based on bounding box is sufficient
      */
    virtual bool featureAtId( QgsFeatureId featureId,
                              QgsFeature & feature, bool fetchGeometry = true, QgsAttributeList fetchAttributes = QgsAttributeList() );

    /** Accessor for sql where clause used to limit dataset */
    virtual QString subsetString();

    /** mutator for sql where clause used to limit dataset size */
    virtual bool setSubsetString( QString theSQL, bool updateFeatureCount = true );

    virtual bool supportsSubsetString() { return true; }

    /** Select features based on a bounding rectangle. Features can be retrieved with calls to nextFeature.
     *  @param fetchAttributes list of attributes which should be fetched
     *  @param rect spatial filter
     *  @param fetchGeometry true if the feature geometry should be fetched
     *  @param useIntersect true if an accurate intersection test should be used,
     *                     false if a test based on bounding box is sufficient
     */
    virtual void select( QgsAttributeList fetchAttributes = QgsAttributeList(),
                         QgsRectangle rect = QgsRectangle(), bool fetchGeometry = true, bool useIntersect = false );

    /**
     * Get the next feature resulting from a select operation.
     * @param feature feature which will receive data from the provider
     * @return true when there was a feature to fetch, false when end was hit
     */
    virtual bool nextFeature( QgsFeature & feature );

    /** Get the feature type. This corresponds to
     * WKBPoint,
     * WKBLineString,
     * WKBPolygon,
     * WKBMultiPoint,
     * WKBMultiLineString or
     * WKBMultiPolygon
     * as defined in qgis.h
     */
    QGis::WkbType geometryType() const;

    /** return the number of layers for the current data source

    @note

    Should this be subLayerCount() instead?
    */
    size_t layerCount() const;

    /**
     * Get the number of features in the layer
     */
    long featureCount() const;

    /**
     * Get the number of fields in the layer
     */
    uint fieldCount() const;

    /** Return the extent for this data layer
    */
    virtual QgsRectangle extent();

    /** Update the extent for this data layer
    */
    virtual void updateExtents();

    /**  * Get the name of the primary key for the layer
    */
    QString getPrimaryKey();

    /**
      * Get the field information for the layer
      * @return vector of QgsField objects
      */
    const QgsFieldMap & fields() const;

    /** Reset the layer */
    void rewind();

    /** Returns the minimum value of an attribute
     *  @param index the index of the attribute */
    QVariant minimumValue( int index );

    /** Returns the maximum value of an attribute
     *  @param index the index of the attribute */
    QVariant maximumValue( int index );

    /** Return the unique values of an attribute
     *  @param index the index of the attribute
     *  @param values reference to the list of unique values
     *  @param limit maximum number of values (added in 1.4) */
    virtual void uniqueValues( int index, QList < QVariant > &uniqueValues, int limit = -1 );

    /**Returns true if layer is valid
    */
    bool isValid();

    /**Adds a list of features
      @return true in case of success and false in case of failure*/
    bool addFeatures( QgsFeatureList & flist );

    /**Deletes a list of features
      @param id list of feature ids
      @return true in case of success and false in case of failure*/
    bool deleteFeatures( const QgsFeatureIds & id );

    /**Adds new attributes
      @param name map with attribute name as key and type as value
      @return true in case of success and false in case of failure*/
    bool addAttributes( const QList<QgsField> &attributes );

    /**Changes attribute values of existing features
      @param attr_map a map containing the new attributes. The integer is the feature id,
      the first QString is the attribute name and the second one is the new attribute value
      @return true in case of success and false in case of failure*/
    bool changeAttributeValues( const QgsChangedAttributesMap & attr_map );

    /**
       Changes geometries of existing features
       @param geometry_map   A std::map containing the feature IDs to change the geometries of.
                             the second map parameter being the new geometries themselves
       @return               true in case of success and false in case of failure
     */
    bool changeGeometryValues( QgsGeometryMap & geometry_map );

    /**Returns a bitmask containing the supported capabilities*/
    int capabilities() const;

    /** The SpatiaLite provider does its own transforms so we return
     * true for the following three functions to indicate that transforms
     * should not be handled by the QgsCoordinateTransform object. See the
     * documentation on QgsVectorDataProvider for details on these functions.
     */
    // XXX For now we have disabled native transforms in the SpatiaLite
    //   (following the PostgreSQL provider example)
    bool supportsNativeTransform()
    {
      return false;
    }

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

  signals:
    /**
     *   This is emitted whenever the worker thread has fully calculated the
     *   extents for this layer, and its event has been received by this
     *   provider.
     */
    void fullExtentCalculated();

    /**
     *   This is emitted when this provider is satisfied that all objects
     *   have had a chance to adjust themselves after they'd been notified that
     *   the full extent is available.
     *
     *   \note  It currently isn't being emitted because we don't have an easy way
     *          for the overview canvas to only be repainted.  In the meantime
     *          we are satisfied for the overview to reflect the new extent
     *          when the user adjusts the extent of the main map canvas.
     */
    void repaintRequested();

  private:

    /** loads fields from input file to member attributeFields */
    void loadFields();

    /** Check if a table/view has any triggers.  Triggers can be used on views to make them editable.*/
    bool hasTriggers();

    /** convert a QgsField to work with SL */
    static bool convertField( QgsField &field );

    QgsFieldMap attributeFields;
    /**
       * Flag indicating if the layer data source is a valid SpatiaLite layer
       */
    bool valid;
    /**
       * Flag indicating if the layer data source is based on a query
       */
    bool isQuery;
    /**
       * Flag indicating if the layer data source is based on a plain Table
       */
    bool mTableBased;
    /**
       * Flag indicating if the layer data source is based on a View
       */
    bool mViewBased;
    /**
       * Flag indicating if the layer data source is based on a VirtualShape
       */
    bool mVShapeBased;
    /**
       * Flag indicating if the layer data source has ReadOnly restrictions
       */
    bool mReadOnly;
    /**
       * DB full path
       */
    QString mSqlitePath;
    /**
     * Name of the table with no schema
     */
    QString mTableName;
    /**
       * Name of the table or subquery
       */
    QString mQuery;
    /**
       * Name of the primary key column in the table
       */
    QString mPrimaryKey;
    /**
       * Name of the geometry column in the table
       */
    QString mGeometryColumn;
    /**
     * Name of the SpatialIndex table
     */
    QString mIndexTable;
    /**
       * Name of the SpatialIndex geometry column
       */
    QString mIndexGeometry;
    /**
     * Geometry type
     */
    QGis::WkbType geomType;
    /**
     * SQLite handle
     */
    sqlite3 *sqliteHandle;
    /**
      * SQLite statement handle
     */
    sqlite3_stmt *sqliteStatement;
    /**
     * String used to define a subset of the layer
     */
    QString mSubsetString;
    /**
       * CoodDimensions of the layer
       */
    int nDims;
    /**
       * Spatial reference id of the layer
       */
    int mSrid;
    /**
      * auth id
     */
    QString mAuthId;
    /**
      * proj4text
     */
    QString mProj4text;
    /**
     * Rectangle that contains the extent (bounding box) of the layer
     */
    QgsRectangle layerExtent;

    /**
     * Number of features in the layer
     */
    long numberFeatures;
    /**
     * this Geometry is supported by an R*Tree spatial index
     */
    bool spatialIndexRTree;
    /**
    * this Geometry is supported by an MBR cache spatial index
    */
    bool spatialIndexMbrCache;

    int enabledCapabilities;

    const QgsField & field( int index ) const;

    /** geometry column index used when fetching geometry */
    int mGeomColIdx;

    /**
    * internal utility functions used to handle common SQLite tasks
    */
    //void sqliteOpen();
    void closeDb();
    bool checkLayerType();
    bool getGeometryDetails();
    bool getTableGeometryDetails();
    bool getViewGeometryDetails();
    bool getVShapeGeometryDetails();
    bool getQueryGeometryDetails();
    bool getSridDetails();
    bool getTableSummary();
    bool prepareStatement( sqlite3_stmt *&stmt,
                           const QgsAttributeList &fetchAttributes,
                           bool fetchGeometry,
                           QString whereClause );
    bool getFeature( sqlite3_stmt *stmt, bool fetchGeometry,
                     QgsFeature &feature,
                     const QgsAttributeList &fetchAttributes );
    void convertToGeosWKB( const unsigned char *blob, size_t blob_size,
                           unsigned char **wkb, size_t *geom_size );
    int computeSizeFromMultiWKB2D( const unsigned char *p_in, int nDims,
                                   int little_endian,
                                   int endian_arch );
    int computeSizeFromMultiWKB3D( const unsigned char *p_in, int nDims,
                                   int little_endian,
                                   int endian_arch );
    void convertFromGeosWKB2D( const unsigned char *blob, size_t blob_size,
                               unsigned char *wkb, size_t geom_size,
                               int nDims, int little_endian, int endian_arch );
    void convertFromGeosWKB3D( const unsigned char *blob, size_t blob_size,
                               unsigned char *wkb, size_t geom_size,
                               int nDims, int little_endian, int endian_arch );
    int computeMultiWKB3Dsize( const unsigned char *p_in, int little_endian,
                               int endian_arch );
    void convertFromGeosWKB( const unsigned char *blob, size_t blob_size,
                             unsigned char **wkb, size_t *geom_size,
                             int dims );
    int computeSizeFromGeosWKB3D( const unsigned char *blob, size_t size,
                                  int type, int nDims, int little_endian,
                                  int endian_arch );
    int computeSizeFromGeosWKB2D( const unsigned char *blob, size_t size,
                                  int type, int nDims, int little_endian,
                                  int endian_arch );

    enum GEOS_3D
    {
      GEOS_3D_POINT              = -2147483647,
      GEOS_3D_LINESTRING         = -2147483646,
      GEOS_3D_POLYGON            = -2147483645,
      GEOS_3D_MULTIPOINT         = -2147483644,
      GEOS_3D_MULTILINESTRING    = -2147483643,
      GEOS_3D_MULTIPOLYGON       = -2147483642,
      GEOS_3D_GEOMETRYCOLLECTION = -2147483641,
    };

    struct SLFieldNotFound {}; //! Exception to throw

  public:
    static QString quotedIdentifier( QString id );
    static QString quotedValue( QString value );

    class SqliteHandles
    {
        //
        // a class allowing to reuse the same sqlite handle for more layers
        //
      public:
        SqliteHandles( sqlite3 * handle ):
            ref( 1 ), sqlite_handle( handle )
        {
        }

        sqlite3 *handle()
        {
          return sqlite_handle;
        }

        //
        // libsqlite3 wrapper
        //
        void sqliteClose();

        static SqliteHandles *openDb( const QString & dbPath );
        static bool checkMetadata( sqlite3 * handle );
        static void closeDb( SqliteHandles * &handle );
        static void closeDb( QMap < QString, SqliteHandles * >&handlesRO, SqliteHandles * &handle );

      private:
        int ref;
        sqlite3 *sqlite_handle;

        static QMap < QString, SqliteHandles * >handles;
    };

    struct SLException
    {
      SLException( char *msg ) : errMsg( msg )
      {
      }

      SLException( const SLException &e ) : errMsg( e.errMsg )
      {
      }

      ~SLException()
      {
        if ( errMsg )
          sqlite3_free( errMsg );
      }

      QString errorMessage() const
      {
        return errMsg ? QString::fromUtf8( errMsg ) : "unknown cause";
      }

    private:
      char *errMsg;
    };

    /**
     * sqlite3 handles pointer
     */
    SqliteHandles *handle;
};
