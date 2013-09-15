/***************************************************************************
    qgsgrassprovider.h  -  Data provider for GRASS format
                             -------------------
    begin                : March, 2004
    copyright            : (C) 2004 by Gary E.Sherman, Radim Blazek
    email                : sherman@mrcc.com, blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASSPROVIDER_H
#define QGSGRASSPROVIDER_H

class QgsFeature;
class QgsField;

class QgsGrassFeatureIterator;

#include <QDateTime>

#include "qgsvectordataprovider.h"
#include <vector>

/* Update.
 * Vectors are updated (reloaded) if:
 * 1) Vector was find to be outdated on some occasion. Currently the only occasion is the beginning
 *    of QgsGrassProvider::select()
 *    Note that if the vector was rewritten by GRASS module and it was not yet updated in QGIS,
 *    it should read without problems from old (deleted file), at least on local disk on *nix system.
 *    (NFS - cache, Cygwin?)
 *
 * 2) Editing is closed by closeEdit
 *
 * Member variables which must be updated after updateMap() are marked !UPDATE! in this file.
 */

/* Freezing.
 * Because open file cannot be deleted on Windows it is necessary to
 * close output vector from GRASS tools before a module is run.
 * This is not however solution for multiple instances of QGIS.
 */

/* Editing.
 * If editing is started by startEdit, vector map is reopened in update mode, and GMAP.update
 * is set to true. All data loaded from the map to QgsGrassProvider remain unchanged
 * untill closeEdit is called.
 * During editing:
 * nextFeature() and getFirstFeature() returns 0
 * featureCount() returns 0
 * fieldCount() returns original (old) number of fields
 */

/* Attributes. Cache of database attributes (because selection from database is slow). */
struct GATT
{
  int cat;       // category
  char **values; // pointer to array of pointers to values
};

/* Grass layer (unique vector+field). */
struct GLAYER
{
  QString path;                  // path to the layer gisdbase+location+mapset+mapName
  int     field;                 // field number
  bool    valid;                 // valid is true if layer is opened, once the layer is closed,
  // valid is set to false and no more used
  int     mapId;                 // map ID in maps vector
  struct  Map_info   *map;       // map header
  struct  field_info *fieldInfo; // field info
  int     nColumns;              // number of columns in database table, if 0, attributes are not available
  // and category (column name 'cat') is used instead
  int     keyColumn;             // number of key column
  QgsFields fields;              // description of layer fields
  int     nAttributes;           // number of attributes read to the memory (may be < nRecords)
  GATT    *attributes;           // vector of attributes
  double( *minmax )[2];          // minimum and maximum values of attributes
  int     nUsers;                // number of instances using this layer, increased by open(),
  // decreased by close()
};

/* Grass vector map. */
struct GMAP
{
  QString gisdbase;      // map gisdabase
  QString location;      // map location name (not path!)
  QString mapset;        // map mapset
  QString mapName;       // map name
  QString path;          // path to the layer gisdbase+location+mapset+mapName
  bool    valid;         // true if map is opened, once the map is closed,
  // valid is set to false and no more used

  // Vector temporally disabled. Necessary for GRASS Tools on Windows
  bool frozen;

  struct  Map_info *map; // map header
  int     nUsers;        // number layers using this map
  int     update;        // true if the map is opened in update mode -> disabled standard reading
  // through nextFeature(), featureCount() returns 0
  QDateTime lastModified; // last modified time of the vector directory, when the map was opened
  QDateTime lastAttributesModified; // last modified time of the vector 'dbln' file, when the map was opened
  // or attributes were updated. The 'dbln' file is updated by v.to.db etc.
  // when attributes are changed
  int     version;       // version, increased by each closeEdit() and updateMap()
};

/**
  \class QgsGrassProvider
  \brief Data provider for GRASS vectors
*/
class GRASS_LIB_EXPORT QgsGrassProvider : public QgsVectorDataProvider
{
    Q_OBJECT

  public:

    QgsGrassProvider( QString uri = QString() );

    virtual ~QgsGrassProvider();

    /**
      *   Returns the permanent storage type for this layer as a friendly name.
      */
    virtual QString storageType() const;

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request );

    /**
     * Get the feature type as defined in WkbType (qgis.h).
     * @return int representing the feature type
     */
    QGis::WkbType geometryType() const;


    /**
     * Get the number of features in the layer
     */
    long featureCount() const;


    /** Return the extent for this data layer
     */
    virtual QgsRectangle extent();

    /**
     * Get the field information for the layer
     */
    const QgsFields & fields() const;

    // ! Key (category) field index
    int keyField();

    /** Restart reading features from previous select operation */
    void rewind();

    /** Returns the minimum value of an attributs
     *  @param index the index of the attribute */
    QVariant minimumValue( int index );

    /** Returns the maximum value of an attributs
     *  @param index the index of the attribute */
    QVariant maxValue( int index );

    /** Update (reload) non static members (marked !UPDATE!) from the static layer and the map.
     *   This method MUST be called whenever lastUpdate of the map is later then mapLastUpdate
     *   of the instance.
     */
    void update();

    /** Load info (mNumberFeatures, mCidxFieldIndex, mCidxFieldNumCats)  from map */
    void loadMapInfo();

    /**Returns true if this is a valid layer
     */
    bool isValid();

    QgsCoordinateReferenceSystem crs();

    // ----------------------------------- Edit ----------------------------------

    /** Is the layer editable? I.e. the layer is valid and current user is owner of the mapset
     *   @return true the layer editable
     *   @return false the is not editable
     */
    bool isGrassEditable();

    /** Returns true if the layer is currently edited (opened in update mode)
     *   @return true in update mode
     *   @return false not edited
     */
    bool isEdited();

    /** Returns true if the layer is currently froze, i.e. a module
     *  from GRASS Tools is writing to this vector
     *   @return true in update mode
     *   @return false not edited
     */
    bool isFrozen();

    /** Start editing. Reopen the vector for update and set GMAP.update = true
     *   @return true is frozen
     *   @return false is not frozen
     */
    bool startEdit();

    /** Freeze vector.
     */
    void freeze();

    /** Thaw vector.
     */
    void thaw();

    /** Close editing. Rebuild topology, GMAP.update = false
     *   @param newMap set to true if a new map was created
     *          and it is not yet used as layer
     *   @return true success
     *   @return false failed to close vector or vector was not in update mode
     */
    bool closeEdit( bool newMap = false );

    /** Get current number of lines.
     *   @return number of lines
     */
    int numLines( void );

    /** Get current number of nodes.
     *   @return number of nodes
     */
    int numNodes( void );

    /** Read line
     *   @param Points pointer to existing structure or NULL
     *   @param Cats pointer to existing structure or NULL
     *   @param line line number
     *   @return line type
     *   @return <0 deadline or error
     */
    int readLine( struct line_pnts * Points, struct line_cats * Cats, int line );

    /** Read node coordinates
     *   @param line line number
     *   @return true node is alive
     *   @return false node is dead
     */
    bool nodeCoor( int node, double *x, double *y );

    /** Read line nodes
     *   @param line line number
     *   @return true line is alive
     *   @return false line is dead
     */
    bool lineNodes( int line, int *node1, int *node2 );

    /** Read boundary areas
     *   @param line line number
     *   @return true line is alive
     *   @return false line is dead
     */
    bool lineAreas( int line, int *left, int *right );

    /** Get isle area
     *   @param isle number
     *   @return area number
     */
    int isleArea( int isle );

    /** Get centroid area
     *   @param centroid line number
     *   @return area number (negative for island)
     */
    int centroidArea( int centroid );

    /** Get number of lines at node
     *   @param node node number
     *   @return number of lines at node (including dead lines)
     */
    int nodeNLines( int node );

    /** Get line number of line at node for given line index
     *   @param node node number
     *   @param idx line index
     *   @return line number
     */
    int nodeLine( int node, int idx );

    /** True if line is alive
     *   @param line line number
     *   @return true alive
     *   @return false dead
     */
    int lineAlive( int line );

    /** True if node is alive
     *   @param node node number
     *   @return true alive
     *   @return false dead
     */
    int nodeAlive( int node );

    /** Write a new line into vector.
     *   @return line number
     *   @return -1 error
     */
    int writeLine( int type, struct line_pnts *Points, struct line_cats *Cats );

    /** Rewrite line.
     *   @return line number
     *   @return -1 error
     */
    int rewriteLine( int line, int type, struct line_pnts *Points, struct line_cats *Cats );

    /** Delete line
     *   @return 0 OK
     *   @return -1 error
     */
    int deleteLine( int line );

    /** Number of updated lines
     */
    int numUpdatedLines( void );

    /** Number of updated nodes
     */
    int numUpdatedNodes( void );

    /** Get updated line
     */
    int updatedLine( int idx );

    /** Get updated node
     */
    int updatedNode( int idx );

    /** Find nearest line
     *   @param threshold maximum distance
     *   @return line number
     *   @return 0 nothing found
     */
    int findLine( double x, double y, int type, double threshold );

    /** Find nearest node
     *   @param threshold maximum distance
     *   @return node number
     *   @return 0 nothing found
     */
    int findNode( double x, double y, double threshold );

    /** Get columns' definitions
     *   @param field
     *   @param cat
     *   @return vector of attributes
     */
    QVector<QgsField> *columns( int field );

    /** Read attributes from DB
     *   @param field
     *   @param cat
     *   @return vector of attributes
     */
    QgsAttributeMap *attributes( int field, int cat );

    /** Key (cat) column name
     *   @param field
     *   @return Key column name or empty string
     */
    QString *key( int field );

    /** Get number of db links
     *   @return number of links
     */
    int numDbLinks( void );

    /** Get db link field
     *  @param link
     *   @return field number or 0
     */
    int dbLinkField( int link );

    /** Execute SQL statement
     *   @param field
     *   @param sql
     *   @return empty string or error message
     */
    QString *executeSql( int field, const QString &sql );

    /** Update attributes
     *   @param field
     *   @param cat
     *   @param update comma separated update string, e.g.: col1 = 5, col2 = 'Val d''Aosta'
     *   @return empty string or error message
     */
    QString *updateAttributes( int field, int cat, const QString &values );

    /** Insert new attributes to the table (it does not check if attributes already exists)
     *   @param field
     *   @param cat
     *   @return empty string or error message
     */
    QString *insertAttributes( int field, int cat );

    /** Delete attributes from the table
     *   @param field
     *   @param cat
     *   @return empty string or error message
     */
    QString *deleteAttribute( int field, int cat );

    /** Check if a database row exists and it is orphan (no more lines with
     *  that category)
     *   @param field
     *   @param cat
     *   @param orphan set to true if a record exits and it is orphan
     *   @return empty string or error message
     */
    QString *isOrphan( int field, int cat, int *orphan );

    /** Create table and link vector to this table
     *   @param field
     *   @param columns SQL definition for columns, e.g. cat integer, label varchar(10)
     *   @return empty string or error message
     */
    QString *createTable( int field, const QString &key, const QString &columns );

    /** Add column to table
     *   @param field
     *   @param column SQL definition for columns, e.g. label varchar(10)
     *   @return empty string or error message
     */
    QString *addColumn( int field, const QString &column );

    /* Following functions work only until first edit operation! (category index used) */

    /** Get number of fields in category index */
    int cidxGetNumFields( void );

    /** Get field number for index */
    int cidxGetFieldNumber( int idx );

    /** Get maximum category for field index */
    int cidxGetMaxCat( int idx );

    /** Returns GRASS layer number */
    int grassLayer();

    /** Returns GRASS layer number for given layer name or -1 if cannot
     *  get layer number
     */
    static int grassLayer( QString );

    /** Returns GRASS layer type (GV_POINT, GV_LINES, GV_AREA) for
     *  given layer name or -1 if cannot get layer type
     */
    static int grassLayerType( QString );

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





  private:
    // Layer type (layerType)
    enum    TYPE     // layer name:
    {
      POINT = 1,   // <field>_point
      LINE,        // <field>_line
      FACE,        // <field>_face
      POLYGON,     // <field>_polygon
      BOUNDARY,    // boundary (currently not used)
      CENTROID,    // centroid (currently not used)
      // topology layers, may be used to display internal GRASS topology info
      // useful for debugging of GRASS topology and modules using topology
      TOPO_POINT,  // all points with topology id
      TOPO_LINE,   // all lines with topology id
      TOPO_NODE    // topology nodes
    };

    QString mGisdbase;      // map gisdabase
    QString mLocation;      // map location name (not path!)
    QString mMapset;        // map mapset
    QString mMapName;       // map name
    QString mLayer;         // layer name
    int     mLayerField;    // field part of layer or -1 if no field specified
    int     mLayerType;     // layer type POINT, LINE, ...
    int     mGrassType;     // grass feature type: GV_POINT, GV_LINE | GV_BOUNDARY, GV_AREA,
    // ( GV_BOUNDARY, GV_CENTROID )
    QGis::WkbType mQgisType;// WKBPoint, WKBLineString, ...
    int     mLayerId;       // ID used in layers
    struct  Map_info *mMap; // vector header pointer
    int     mMapVersion;    // The version of the map for which the instance was last time updated

    int    mCidxFieldIndex;    // !UPDATE! Index for layerField in category index or -1 if no such field
    int    mCidxFieldNumCats;  // !UPDATE! Number of records in field index

    bool    mValid;                // !UPDATE!
    long    mNumberFeatures;       // !UPDATE!

    // create QgsFeatureId from GRASS geometry object id and cat
    static QgsFeatureId makeFeatureId( int grassId, int cat );

    // Reopen map after edit or freeze
    bool reopenMap();

    // -----------------------------------------------------------------------------------------
    /* Static variables and methods.
     * These methods opens GRASS vectors and loads some parts of vectors to the memory.
     * it maintains the list of opened layers so that sources are not duplicated in the memory.
     * Layers are identified by layer ID.
     * The layers have unique URI, if next layer of the same URI is requested,
     * nUsers is increased and ID of the layer which is already opened is returned.
     * Attributes are loaded from DB and stored in the memory when layer is opened.
     */

    /*! Open layer. Layer for QgsGrassVector means Map+field
     *  @param gisdbase
     *  @param location
     *  @param mapset
     *  @param mapName
     *  @param field
     *  @return layer ID
     *  @return -1 cannot open
     */
    static int openLayer( QString gisdbase, QString location, QString mapset, QString mapName, int field );

    /*! Load sources from the map.
     *  Must be set: layer.mapId, layer.map, layer.field
     *  Updates: layer.fieldInfo, layer.nColumns, layer.nAttributes, layer.attributes, layer.keyColumn
     *  Unchanged: layer.valid
     *
     *  Old sources are released, namely: layer.fields and layer.attributes
     *
     *  layer.attributes must be pointer to existing array or 0
     */
    static void loadLayerSourcesFromMap( GLAYER &layer );

    /*! Load attributes from database table.
     *  Must be set: layer.mapId, layer.map, layer.field
     *  Updates: layer.fieldInfo, layer.nColumns, layer.nAttributes, layer.attributes, layer.keyColumn
     *  Unchanged: layer.valid
     *
     *  Old sources are released, namely: layer.attributes
     *
     *  layer.attributes must be pointer to existing array or 0
     */
    static void loadAttributes( GLAYER &layer );

    /*! Close layer.
     *  @param layerId
     */
    static void closeLayer( int layerId );

    /*! Open map.
     *  @param gisdbase
     *  @param location
     *  @param mapset
     *  @param mapName
     *  @return map ID
     *  @return -1 cannot open
     */
    static int openMap( QString gisdbase, QString location, QString mapset, QString mapName );

    /*! Close map.
     *  @param mapId
     */
    static void closeMap( int mapId );

    /*! Update map. Close and reopen vector, all layers in mLayers using this map are also updated.
     *  Instances of QgsGrassProvider are not updated and should call update() method.
     *  @param mapId
     */
    static void updateMap( int mapId );

    /*! The map is outdated. The map was for example rewritten by GRASS module outside QGIS.
     *  This function checks internal timestamp stored in QGIS.
     *  @param mapId
     */
    static bool mapOutdated( int mapId );

    /*! The attributes are outdated. The table was for example updated by GRASS module outside QGIS.
     *  This function checks internal timestamp stored in QGIS.
     *  @param mapId
     */
    static bool attributesOutdated( int mapId );

    /*! Get layer map.
     *  @param layerId
     *  @return pointer to Map_info structure
     */
    static struct Map_info *layerMap( int layerId );

    /*! Get attribute by category(key) and attribute number.
     *  @param layerId
     *  @param category (key)
     *  @param column column number ( < nColumns )
     *  @return pointer to string representation of the value or NULL, this value must not be changed
     */
    static char *attribute( int layerId, int cat, int column );

    /*! Set feature attributes.
     *  @param layerId
     *  @param feature
     *  @param cat category number
     */
    void setFeatureAttributes( int layerId, int cat, QgsFeature *feature );

    /*! Set feature attributes.
     *  @param layerId
     *  @param feature
     *  @param cat category number
     *  @param attlist a list containing the index number of the fields to set
     */
    void setFeatureAttributes( int layerId, int cat, QgsFeature *feature, const QgsAttributeList & attlist );

    /** check if provider is outdated and update if necessary */
    void ensureUpdated();

    /** check if layer is topology layer TOPO_POINT, TOPO_NODE, TOPO_LINE */
    bool isTopoType() const;

    void setTopoFields();

    /* Get name of GRASS primitive type */
    static QString primitiveTypeName( int type );

    /* Static arrays of opened layers and vectors */
    static QVector<GLAYER> mLayers; // Map + field/attributes
    static QVector<GMAP> mMaps;     // Map

    /** Fields used for topo layers */
    QgsFields mTopoFields;

    friend class QgsGrassFeatureIterator;
    QSet< QgsGrassFeatureIterator *> mActiveIterators;
};

#endif // QGSGRASSPROVIDER_H
