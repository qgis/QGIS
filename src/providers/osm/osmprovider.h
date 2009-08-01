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
#include <QDateTime>




typedef QMap<int, QgsFeature> QgsFeatureMap;



class QgsOSMDataProvider: public QgsVectorDataProvider
{
  Q_OBJECT

  public:

    /**
     * Constructor of the vector provider.
     * @param uri  uniform resource locator (URI) for a dataset
     */
    QgsOSMDataProvider( QString uri );

    /**
     * Destructor.
     */
    virtual ~QgsOSMDataProvider();


    // Implementation of functions from QgsVectorDataProvider

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


    // Implementation of functions from QgsDataProvider

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
    enum { PointType, LineType, PolygonType } mFeatureType;
    enum Attribute { TimestampAttr = 0, UserAttr = 1, TagAttr, CustomTagAttr };
    const static int DEFAULT_EXTENT = 100;

    static const char* attr[];

    QString mFileName;
    QString mDatabaseFileName;
    QDateTime mOsmFileLastModif;
    bool mValid;

    sqlite3 *mDatabase;
    sqlite3_stmt *mDatabaseStmt;

    char *mError;

    //! object that receives notifications from init
    QObject* mInitObserver;

    double xMin, xMax, yMin, yMax;   // boundary

    // selection
    QgsAttributeList mAttributesToFetch;
    QgsFieldMap mAttributeFields;
    QgsRectangle mSelectionRectangle;
    QgsGeometry* mSelectionRectangleGeom;

    // flags
    bool mSelectUseIntersect;

    // private methods
    sqlite3_stmt *mTagsStmt;
    bool mTagsRetrieval;
    QString tagsForObject( const char* type, int id );

    sqlite3_stmt *mCustomTagsStmt;
    QStringList mCustomTagsList;
    QString tagForObject( const char* type, int id, QString tagKey );

    sqlite3_stmt *mWayStmt;
    sqlite3_stmt *mNodeStmt;

    QString mStyleFileName;
    QString mStyle;

    // manipulation with sqlite database

    bool isDatabaseCompatibleWithInput( QString mFileName );
    bool isDatabaseCompatibleWithPlugin();

    /**
     * Create Open Street Map database schema, using c++ library for attempt to sqlite database.
     * @return true in case of success and false in case of failure
     */
    bool createDatabaseSchema();

    /**
     * Create indexes for OSM database schema, using c++ library for attempt to sqlite database.
     * @return true in case of success and false in case of failure
     */
    bool createIndexes();
    bool createTriggers();

    /**
     * Drop the whole OSM database schema, using c++ library for attempt to sqlite database.
     * @return true in case of success and false in case of failure
     */
    bool dropDatabaseSchema();

    /**
     * Open sqlite3 database.
     * @return true in case of success and false in case of failure
     */
    bool openDatabase();

    /**
     * Close opened sqlite3 database.
     */
    bool closeDatabase();

    /**
     * Process Open Street Map file, parse it and store data in sqlite database.
     * Function doesn't require much memory: uses simple SAX XML parser
     * and stores data directly to database while processing OSM file.
     * @param osm_filename name of file with OSM data to parse into sqlite3 database
     * @return true in case of success and false in case of failure
     */
    bool loadOsmFile( QString osm_filename );

    bool updateWayWKB( int wayId, int isClosed, char **geo, int *geolen );
    bool updateNodes();
    bool removeIncorrectWays();


    /**
     * This function is part of postparsing. OpenStreetMaps nodes have to be divided in two categories here for better manipulation.
     * First category is "significant OSM nodes" - these nodes are loaded to Point vector layer and hold some significant information (in tags),
     * except the fact that they may be parts of ways geometries. The second category are "not significant OSM nodes". These are considered
     * to be a part of some way geometry only but nothing more. These nodes are not loaded to Point layer, they don't have any significant tags
     * like "name","ref",etc; OSM plugin even doesn't care of these nodes when some way geometry is changing. Plugin will just remove
     * all not significant nodes of that way and will create new ones instead of them.
     * @return true in case of success and false in case of failure
     */
    bool splitNodes();

    /**
     * This function is postprocess after osm file parsing. Parsing stored all information into database, but
     * such database schema is not optimal e.g. for way selection, that is called very often. It should be better
     * to have way members (with their coordinates) store directly in way table - in WKB (well known binary) format
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
     * Get number of members of specified way.
     * @param wayId way identifier
     * @return number of way members
     */
    int wayMemberCount( int wayId );

    int relationMemberCount( int relId );

    // fetch node from current statement
    bool fetchNode( QgsFeature& feature, sqlite3_stmt* stmt, bool fetchGeometry, QgsAttributeList& fetchAttrs );

    // fetch way from current statement
    bool fetchWay( QgsFeature& feature, sqlite3_stmt* stmt, bool fetchGeometry, QgsAttributeList& fetchAttrs );

    // Change geometry of one feature (used by changeGeometryValues())
    bool changeGeometryValue( const int & featid, QgsGeometry & geom );

    struct wkbPoint
    {
      char byteOrder;
      unsigned wkbType;
      double x;
      double y;
    };
    wkbPoint mWKBpt;
};

