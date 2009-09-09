/***************************************************************************
    osmprovider.h - provider for OSM; stores OSM data in sqlite3 DB
    ------------------
    begin                : October 2008
    copyright            : (C) 2008 by Lukas Berka
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectordataprovider.h"

#include <sqlite3.h>
#include <QFile>


/**
 * Quantum GIS provider for OpenStreetMap data.
 */
class QgsOSMDataProvider: public QgsVectorDataProvider
{
    Q_OBJECT

  private:

    //! provider manages features with one of three geometry types; variable determines feature type of this provider
    enum { PointType, LineType, PolygonType } mFeatureType;

    //! supported feature attributes
    enum Attribute { TimestampAttr = 0, UserAttr = 1, TagAttr, CustomTagAttr };

    //! supported feature attributes
    static const char* attr[];

    //! constant that helps to set default map extent
    const static int DEFAULT_EXTENT = 100;

    //! absolute name of input OSM file
    QString mFileName;

    //! determines if this provider is in valid state (initialized correctly, etc.)
    bool mValid;

    //! holds information on error that occured in provider execution
    char *mError;

    //! object that receives notifications from init
    QObject* mInitObserver;

    //! boundary of all OSM data that provider manages
    double xMin, xMax, yMin, yMax;

    //! list of feature tags for which feature attributes are created
    QStringList mCustomTagsList;

    //! name of file with renderer style information
    QString mStyleFileName;

    //! determines style for rendering: one of "medium", "big", "small"
    QString mStyle;

    // sqlite3 database stuff:

    //! absolute name of local database file with OSM data
    QString mDatabaseFileName;

    //! pointer to sqlite3 database that keeps OSM data
    sqlite3 *mDatabase;

    //! pointer to main sqlite3 database statement object; this statement serves to select OSM data
    sqlite3_stmt *mDatabaseStmt;

    //! pointer to main sqlite3 database statement object; this statement serves to select OSM data
    sqlite3_stmt *mSelectFeatsStmt;

    //! pointer to main sqlite3 db stmt object; this stmt serves to select OSM data from some boundary
    sqlite3_stmt *mSelectFeatsInStmt;

    //! sqlite3 database statement ready to select all feature tags
    sqlite3_stmt *mTagsStmt;

    //! sqlite3 database statement ready to select concrete feature tag
    sqlite3_stmt *mCustomTagsStmt;

    //! sqlite3 database statement for exact way selection
    sqlite3_stmt *mWayStmt;

    //! sqlite3 database statement for exact node selection
    sqlite3_stmt *mNodeStmt;

    // variables used to select OSM data; used mainly in select(), nextFeature() functions:

    //! list of supported attribute fields
    QgsFieldMap mAttributeFields;

    //! which attributes should be fetched after calling of select() function
    QgsAttributeList mAttributesToFetch;

    //! features from which area should be fetched after calling of select() function?
    QgsRectangle mSelectionRectangle;

    //! geometry object of area from which features should be fetched after calling of select() function
    QgsGeometry* mSelectionRectangleGeom;

    //! determines if intersect should be used while selecting OSM data
    bool mSelectUseIntersect;



  public:

    /**
     * Constructor of vector provider.
     * @param uri uniform resource locator (URI) for a dataset
     */
    QgsOSMDataProvider( QString uri );

    /**
     * Destructor.
     */
    virtual ~QgsOSMDataProvider();


    // Implementation of QgsVectorDataProvider functions

    /**
     * Returns the permanent storage type for this layer as a friendly name.
     */
    virtual QString storageType() const;

    /** Select features based on a bounding rectangle. Features can be retrieved with calls to getNextFeature.
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
     */
    virtual bool nextFeature( QgsFeature& feature );

    /**
     * Gets the feature at the given feature ID.
     * @param featureId id of the feature
     * @param feature feature which will receive the data
     * @param fetchGeometry if true, geometry will be fetched from the provider
     * @param fetchAttributes a list containing the indexes of the attribute fields to copy
     * @return True when feature was found, otherwise false
     */
    virtual bool featureAtId( int featureId,
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

    /**
     * Return a map of indexes with field names for this layer
     * @return map of fields
     */
    virtual const QgsFieldMap & fields() const;

    /**
     * Restart reading features from previous select operation.
     */
    virtual void rewind();

    /**
     * Changes attribute values of existing features.
     * @param attr_map a map containing changed attributes
     * @return true in case of success and false in case of failure
     */
    virtual bool changeAttributeValues( const QgsChangedAttributesMap & attr_map );

    /**
     * Returns a bitmask containing the supported capabilities
     * Note, some capabilities may change depending on whether
     * a spatial filter is active on this provider, so it may
     * be prudent to check this value per intended operation.
     */
    virtual int capabilities() const;


    // Implementation of QgsDataProvider functions

    /**
     * Returns a provider name.
     */
    virtual QString name() const;

    /**
     * Returns a provider description.
     */
    virtual QString description() const;

    /**
     * Return the extent for this data layer
     */
    virtual QgsRectangle extent();

    /**
     * Returns true if this is a valid provider
     */
    virtual bool isValid();

    /**
     * Get the QgsCoordinateReferenceSystem for this layer.
     */
    virtual QgsCoordinateReferenceSystem crs();


  private:
    /**
     * Finds out if current database belongs to (was created from) specified input file.
     * @param mFileName name of input OSM file
     * @return answer to that question
     */
    bool isDatabaseCompatibleWithInput( QString mFileName );

    /**
     * Finds out if current database and provider versions are compatible.
     * @return answer to that question
     */
    bool isDatabaseCompatibleWithProvider();

    /**
     * Creates Open Street Map database schema, using c++ library for attempt to sqlite database.
     * @return true in case of success and false in case of failure
     */
    bool createDatabaseSchema();

    /**
     * Creates indexes for OSM database schema, using c++ library for attempt to sqlite database.
     * @return true in case of success and false in case of failure
     */
    bool createIndexes();

    /**
     * Creates triggers for OSM database schema, using c++ library for attempt to sqlite database.
     * @return true in case of success and false in case of failure
     */
    bool createTriggers();

    /**
     * Drops the whole OSM database schema, using c++ library for attempt to sqlite database.
     * @return true in case of success and false in case of failure
     */
    bool dropDatabaseSchema();

    /**
     * Opens sqlite3 database.
     * @return true in case of success and false in case of failure
     */
    bool openDatabase();

    /**
     * Closes opened sqlite3 database.
     */
    bool closeDatabase();

    /**
     * Processes Open Street Map file, parse it and store data in sqlite database.
     * Function doesn't require much memory: uses simple SAX XML parser
     * and stores data directly to database while processing OSM file.
     * @param osm_filename name of file with OSM data to parse into sqlite3 database
     * @return true in case of success and false in case of failure
     */
    bool loadOsmFile( QString osm_filename );

    /**
     * Function computes WKB (well-known-binary) information on geometry of specified way
     * and store it into database. Later this enables faster displaying of features.
     * @param wayId way identifier
     * @param isClosed is this way closed? closed=polygon X notClosed=line
     * @param geo output; way geometry in wkb format
     * @param geolen output; len of wkb geometry
     */
    bool updateWayWKB( int wayId, int isClosed, char **geo, int *geolen );

    /**
     * Function performs all necessary postparsing manipulations with node records.
     */
    bool updateNodes();

    /**
     * Function removes all ways that are not correct. This is called after parsing input file is completed.
     * The main purpose is removing ways that contain nodes that are not included in loaded data.
     */
    bool removeIncorrectWays();

    /**
     * This function performs postprocessing after OSM file parsing.
     *
     * Parsing has stored all information into database, but such database schema is not optimal e.g. for way selection,
     * that must be done very often. It should be better to have way members (with their coordinates) stored directly
     * in way table - in WKB (well known binary) format. Also some other redundant/cached information can be useful.
     *
     * @return true in case of success and false in case of failure
     */
    bool postparsing();

    /**
     * Gets first free feature id in database. Searching for the biggest
     * NEGATIVE integer that is not assigned to any feature.
     * @return free id (negative) for feature, 0 if id cannot be returned
     */
    int freeFeatureId();

    /**
     * Gets number of members of specified way.
     * @param wayId way identifier
     * @return number of way members
     */
    int wayMemberCount( int wayId );

    /**
     * Function fetches one node from current sqlite3 statement.
     * @param feature output; feature representing fetched node
     * @param stmt database statement to fetch node from
     * @param fetchGeometry determines if node geometry should be fetched also
     * @param fetchAttrs list of attributes to be fetched with node
     * @return success of failure flag (true/false)
     */
    bool fetchNode( QgsFeature& feature, sqlite3_stmt* stmt, bool fetchGeometry, QgsAttributeList& fetchAttrs );

    /**
     * Function fetches one way from current sqlite3 statement.
     * @param feature output; feature representing fetched way
     * @param stmt database statement to fetch way from
     * @param fetchGeometry determines if way geometry should be fetched also
     * @param fetchAttrs list of attributes to be fetched with way
     * @return success of failure flag (true/false)
     */
    bool fetchWay( QgsFeature& feature, sqlite3_stmt* stmt, bool fetchGeometry, QgsAttributeList& fetchAttrs );

    /**
     * Function returns string of concatenated tags of specified feature.
     * @param type type of feature (one of "node","way","relation")
     * @param id feature identifier
     * @return string of tags concatenation
     */
    QString tagsForObject( const char* type, int id );

    /**
     * Function returns one tag value of specified feature and specified key.
     * @param type type of feature (one of "node","way","relation")
     * @param id feature identifier
     * @param tagKey tag key
     * @return tag value
     */
    QString tagForObject( const char* type, int id, QString tagKey );
};

