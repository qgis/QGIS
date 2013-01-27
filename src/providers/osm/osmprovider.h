#ifndef OSMPROVIDER_H
#define OSMPROVIDER_H

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

class QgsVectorLayer;

class QgsOSMFeatureIterator;

/**
 * Quantum GIS provider for OpenStreetMap data.
 */
class QgsOSMDataProvider: public QgsVectorDataProvider
{
    Q_OBJECT

  private:

    //! provider manages features with one of three geometry types; variable determines feature type of this provider
    enum OSMType { PointType, LineType, PolygonType };
    OSMType mFeatureType;

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
    QgsRectangle mExtent;

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

    //! list of supported attribute fields
    QgsFields mAttributeFields;

    friend class QgsOSMFeatureIterator;
    QgsOSMFeatureIterator* mActiveIterator; //!< pointer to currently active iterator (0 if none)


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
     * Return a map of indexes with field names for this layer
     * @return map of fields
     */
    virtual const QgsFields & fields() const;

    /**
     * Returns a bitmask containing the supported capabilities
     * Note, some capabilities may change depending on whether
     * a spatial filter is active on this provider, so it may
     * be prudent to check this value per intended operation.
     */
    virtual int capabilities() const;

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request );


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

  public slots:
    virtual void setRenderer( QgsVectorLayer *layer );

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
};

#endif
