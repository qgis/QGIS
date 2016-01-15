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

#include <QDateTime>

#include "qgsvectordataprovider.h"

#include "qgsgrassvectormap.h"
#include "qgsgrassvectormaplayer.h"

class QgsFeature;
class QgsField;
class QgsVectorLayerEditBuffer;

class QgsGrassFeatureIterator;

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

/**
  \class QgsGrassProvider
  \brief Data provider for GRASS vectors
*/
class GRASS_LIB_EXPORT QgsGrassProvider : public QgsVectorDataProvider
{
    Q_OBJECT

  public:
    static int LAST_TYPE;

    QgsGrassProvider( QString uri = QString() );

    virtual ~QgsGrassProvider();

    virtual int capabilities() const override;

    virtual QgsAbstractFeatureSource* featureSource() const override;

    /**
      *   Returns the permanent storage type for this layer as a friendly name.
      */
    virtual QString storageType() const override;

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request ) override;

    /**
     * Get the feature type as defined in WkbType (qgis.h).
     * @return int representing the feature type
     */
    QGis::WkbType geometryType() const override;


    /**
     * Get the number of features in the layer
     */
    long featureCount() const override;


    /** Return the extent for this data layer
     */
    virtual QgsRectangle extent() override;

    /**
     * Get the field information for the layer
     */
    const QgsFields & fields() const override;

    // ! Key (category) field index
    int keyField();

    /** Restart reading features from previous select operation */
    void rewind();

    /** Returns the minimum value of an attributs
     *  @param index the index of the attribute */
    QVariant minimumValue( int index ) override;

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

    /** Returns true if this is a valid layer
     */
    bool isValid() override;

    QgsCoordinateReferenceSystem crs() override;

    // ----------------------------------- New edit --------------------------------
    // Changes are written during editing.
    // TODO: implement also these functions but disable during manual layer editing
    virtual bool addFeatures( QgsFeatureList & flist ) override { Q_UNUSED( flist ); return true; }
    virtual bool deleteFeatures( const QgsFeatureIds & id ) override { Q_UNUSED( id ); return true; }
    virtual bool addAttributes( const QList<QgsField> &attributes ) override;
    virtual bool deleteAttributes( const QgsAttributeIds &attributes ) override;
    virtual bool changeAttributeValues( const QgsChangedAttributesMap & attr_map ) override  { Q_UNUSED( attr_map ); return true; }
    virtual bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override { Q_UNUSED( geometry_map ); return true; }


    //----------------------------------------------------------------------------
    QgsGrassObject grassObject() const { return mGrassObject; }


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

    /* Start standard QGIS editing */
    //void startEditing( QgsVectorLayerEditBuffer* buffer );
    void startEditing( QgsVectorLayer *vectorLayer );

    /** Freeze vector. */
    void freeze();

    /** Thaw vector. */
    void thaw();

    /** Close editing. Rebuild topology, GMAP.update = false
     *   @param newMap set to true if a new map was created
     *          and it is not yet used as layer
     *   @return true success
     *   @return false failed to close vector or vector was not in update mode
     */
    bool closeEdit( bool newMap = false, QgsVectorLayer *vectorLayer = 0 );

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
    int rewriteLine( int lid, int type, struct line_pnts *Points, struct line_cats *Cats );

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

    // TODO is it used?
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
    QString key( int field );

    /** Get number of db links
     *   @return number of links
     */
    int numDbLinks( void );

    /** Get db link field
     *  @param link
     *   @return field number or 0
     */
    int dbLinkField( int link );


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

    /** Return a provider name */
    QString name() const override;

    /** Return description */
    QString description() const override;

    // Layer type (layerType)
    enum TYPE      // layer name:
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

    // Set type for next digitized feature (GV_POINT,GV_LINE, GV_BOUNDARY, GV_CENTROID, GV_AREA)
    void setNewFeatureType( int type ) { mNewFeatureType = type; }

  public slots:
    void onFeatureAdded( QgsFeatureId fid );
    void onFeatureDeleted( QgsFeatureId fid );
    void onGeometryChanged( QgsFeatureId fid, QgsGeometry &geom );
    void onAttributeValueChanged( QgsFeatureId fid, int idx, const QVariant &value );
    void onAttributeAdded( int idx );
    void onAttributeDeleted( int idx );
    void onBeforeCommitChanges();
    void onBeforeRollBack();
    void onEditingStopped();
    void onUndoIndexChanged( int currentIndex );

    void onDataChanged();

  signals:
    // TODO: move to QGIS core?
    // Emitted when a fields was added/deleted so that other layers sharing the same layer
    // may be updated
    void fieldsChanged();

  protected:
    // used by QgsGrassFeatureSource
    QgsGrassVectorMapLayer *openLayer() const;

  private:
    struct Map_info * map();
    void setMapset();
    bool openLayer();
    // update topo symbol of new features
    void setAddedFeaturesSymbol();
    // get new, not yet used cat
    int getNewCat();

    QgsGrassObject mGrassObject;
    // field part of layer or -1 if no field specified
    int mLayerField;
    // layer type POINT, LINE, ...
    int mLayerType;
    // grass feature type: GV_POINT, GV_LINE | GV_BOUNDARY, GV_AREA, ( GV_BOUNDARY, GV_CENTROID )
    int mGrassType;
    // WKBPoint, WKBLineString, ...
    QGis::WkbType mQgisType;
    QString mLayerName;
    QgsGrassVectorMapLayer *mLayer;
    // The version of the map for which the instance was last time updated
    int mMapVersion;

    bool mValid;
    long mNumberFeatures;

    // create QgsFeatureId from GRASS geometry object id and cat
    static QgsFeatureId makeFeatureId( int grassId, int cat );

    /** Get attribute by category(key) and attribute number.
     *  @param layerId
     *  @param category (key)
     *  @param column column number ( < nColumns )
     *  @return pointer to string representation of the value or NULL, this value must not be changed
     */
    static char *attribute( int layerId, int cat, int column );

    /** Check if provider is outdated and update if necessary */
    void ensureUpdated();

    /** Check if layer is topology layer TOPO_POINT, TOPO_NODE, TOPO_LINE */
    bool isTopoType() const;

    static bool isTopoType( int layerType );

    void setTopoFields();

    void setPoints( struct line_pnts *points, const QgsAbstractGeometryV2 * geometry );

    // Get other edited layer, returns 0 if layer does not exist
    QgsGrassVectorMapLayer * otherEditLayer( int layerField );

    /** Fields used for topo layers */
    QgsFields mTopoFields;

    //QgsFields mEditFields;

    QgsVectorLayerEditBuffer* mEditBuffer;
    QgsVectorLayer* mEditLayer;

    //  next digitized feature GRASS type
    int mNewFeatureType;

    // Last version of layer fields during editing, updated after addAttribute and deleteAttribute
    QgsFields mEditLayerFields;

    // List of other layers opened for editing
    QList<QgsGrassVectorMapLayer *> mOtherEditLayers;

    // points and cats used only for editing
    struct line_pnts *mPoints;
    struct line_cats *mCats;

    // last geometry GV_* type, used e.g. for splitting features
    int mLastType;

    // number of currently being edited providers
    static int mEditedCount;

    friend class QgsGrassFeatureSource;
    friend class QgsGrassFeatureIterator;
    friend class QgsGrassUndoCommandChangeAttribute;
};

#endif // QGSGRASSPROVIDER_H
