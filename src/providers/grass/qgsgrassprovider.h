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
    static int sLastType;

    QgsGrassProvider( const QString &uri = QString() );

    ~QgsGrassProvider() override;

    QgsVectorDataProvider::Capabilities capabilities() const override;
    QgsAbstractFeatureSource *featureSource() const override;
    QString storageType() const override;
    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) const override;
    QgsWkbTypes::Type wkbType() const override;
    long featureCount() const override;
    QgsRectangle extent() const override;
    QgsFields fields() const override;

    // ! Key (category) field index
    int keyField();

    //! Restart reading features from previous select operation
    void rewind();

    QVariant minimumValue( int index ) const override;

    /**
     * Returns the maximum value of an attribute
     *  \param index the index of the attribute */
    QVariant maxValue( int index );

    /**
     * Update (reload) non static members (marked !UPDATE!) from the static layer and the map.
     *   This method MUST be called whenever lastUpdate of the map is later then mapLastUpdate
     *   of the instance.
     */
    void update();

    //! Load info (mNumberFeatures, mCidxFieldIndex, mCidxFieldNumCats)  from map
    void loadMapInfo();

    bool isValid() const override;

    QgsCoordinateReferenceSystem crs() const override;

    // ----------------------------------- New edit --------------------------------
    // Changes are written during editing.
    // TODO: implement also these functions but disable during manual layer editing
    bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = nullptr ) override { Q_UNUSED( flist ) Q_UNUSED( flags ); return true; }
    bool deleteFeatures( const QgsFeatureIds &id ) override { Q_UNUSED( id ) return true; }
    bool addAttributes( const QList<QgsField> &attributes ) override;
    bool deleteAttributes( const QgsAttributeIds &attributes ) override;
    bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override  { Q_UNUSED( attr_map ) return true; }
    bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override { Q_UNUSED( geometry_map ) return true; }


    //----------------------------------------------------------------------------
    QgsGrassObject grassObject() const { return mGrassObject; }


    // ----------------------------------- Edit ----------------------------------

    /**
     * Is the layer editable? I.e. the layer is valid and current user is owner of the mapset
     *   \returns true the layer editable
     *   \returns false the is not editable
     */
    bool isGrassEditable();

    /**
     * Returns true if the layer is currently edited (opened in update mode)
     *   \returns true in update mode
     *   \returns false not edited
     */
    bool isEdited();

    /**
     * Returns true if the layer is currently froze, i.e. a module
     *  from GRASS Tools is writing to this vector
     *   \returns true in update mode
     *   \returns false not edited
     */
    bool isFrozen();

    /* Start standard QGIS editing */
    //void startEditing( QgsVectorLayerEditBuffer* buffer );
    void startEditing( QgsVectorLayer *vectorLayer );

    //! Freeze vector.
    void freeze();

    //! Thaw vector.
    void thaw();

    /**
     * Close editing. Rebuild topology, GMAP.update = false
     *   \param newMap set to true if a new map was created
     *          and it is not yet used as layer
     *   \returns true success
     *   \returns false failed to close vector or vector was not in update mode
     */
    bool closeEdit( bool newMap = false, QgsVectorLayer *vectorLayer = nullptr );

    /**
     * Gets current number of lines.
     *   \returns number of lines
     */
    int numLines( void );

    /**
     * Gets current number of nodes.
     *   \returns number of nodes
     */
    int numNodes( void );

    /**
     * Read line
     *   \param Points pointer to existing structure or NULL
     *   \param Cats pointer to existing structure or NULL
     *   \param line line number
     *   \returns line type
     *   \returns <0 deadline or error
     */
    int readLine( struct line_pnts *Points, struct line_cats *Cats, int line );

    /**
     * Read node coordinates
     *   \param line line number
     *   \returns true node is alive
     *   \returns false node is dead
     */
    bool nodeCoor( int node, double *x, double *y );

    /**
     * Read line nodes
     *   \param line line number
     *   \returns true line is alive
     *   \returns false line is dead
     */
    bool lineNodes( int line, int *node1, int *node2 );

    /**
     * Read boundary areas
     *   \param line line number
     *   \returns true line is alive
     *   \returns false line is dead
     */
    bool lineAreas( int line, int *left, int *right );

    /**
     * Gets isle area
     *   \param isle number
     *   \returns area number
     */
    int isleArea( int isle );

    /**
     * Gets centroid area
     *   \param centroid line number
     *   \returns area number (negative for island)
     */
    int centroidArea( int centroid );

    /**
     * Gets number of lines at node
     *   \param node node number
     *   \returns number of lines at node (including dead lines)
     */
    int nodeNLines( int node );

    /**
     * Gets line number of line at node for given line index
     *   \param node node number
     *   \param idx line index
     *   \returns line number
     */
    int nodeLine( int node, int idx );

    /**
     * True if line is alive
     *   \param line line number
     *   \returns true alive
     *   \returns false dead
     */
    int lineAlive( int line );

    /**
     * True if node is alive
     *   \param node node number
     *   \returns true alive
     *   \returns false dead
     */
    int nodeAlive( int node );

    /**
     * Write a new line into vector.
     *   \returns line number
     *   \returns -1 error
     */
    int writeLine( int type, struct line_pnts *Points, struct line_cats *Cats );

    /**
     * Rewrite line.
     *   \returns line number
     *   \returns -1 error
     */
    int rewriteLine( int lid, int type, struct line_pnts *Points, struct line_cats *Cats );

    /**
     * Delete line
     *   \returns 0 OK
     *   \returns -1 error
     */
    int deleteLine( int line );

    /**
     * Number of updated lines
     */
    int numUpdatedLines( void );

    /**
     * Number of updated nodes
     */
    int numUpdatedNodes( void );

    /**
     * Gets updated line
     */
    int updatedLine( int idx );

    /**
     * Gets updated node
     */
    int updatedNode( int idx );

    /**
     * Find nearest line
     *   \param threshold maximum distance
     *   \returns line number
     *   \returns 0 nothing found
     */
    int findLine( double x, double y, int type, double threshold );

    /**
     * Find nearest node
     *   \param threshold maximum distance
     *   \returns node number
     *   \returns 0 nothing found
     */
    int findNode( double x, double y, double threshold );

    // TODO is it used?

    /**
     * Read attributes from DB
     *   \param field
     *   \param cat
     *   \returns vector of attributes
     */
    QgsAttributeMap *attributes( int field, int cat );

    /**
     * Key (cat) column name
     *   \param field
     *   \returns Key column name or empty string
     */
    QString key( int field );

    /**
     * Gets number of db links
     *   \returns number of links
     */
    int numDbLinks( void );

    /**
     * Gets db link field
     *  \param link
     *   \returns field number or 0
     */
    int dbLinkField( int link );


    /* Following functions work only until first edit operation! (category index used) */

    //! Gets number of fields in category index
    int cidxGetNumFields( void );

    //! Gets field number for index
    int cidxGetFieldNumber( int idx );

    //! Gets maximum category for field index
    int cidxGetMaxCat( int idx );

    //! Returns GRASS layer number
    int grassLayer();

    /**
     * Returns GRASS layer number for given layer name or -1 if cannot
     *  get layer number
     */
    static int grassLayer( const QString & );

    /**
     * Returns GRASS layer type (GV_POINT, GV_LINES, GV_AREA) for
     *  given layer name or -1 if cannot get layer type
     */
    static int grassLayerType( const QString & );

    QString name() const override;
    QString description() const override;

    // Layer type (layerType)
    enum Type      // layer name:
    {
      Point = 1,   //!< <field>_point
      Line,        //!< <field>_line
      Face,        //!< <field>_face
      Polygon,     //!< <field>_polygon
      Boundary,    //!< Boundary (currently not used)
      Centroid,    //!< Centroid (currently not used)
      // topology layers, may be used to display internal GRASS topology info
      // useful for debugging of GRASS topology and modules using topology
      TopoPoint,  //!< All points with topology id
      TopoLine,   //!< All lines with topology id
      TopoNode    //!< Topology nodes
    };

    // Set type for next digitized feature (GV_POINT,GV_LINE, GV_BOUNDARY, GV_CENTROID, GV_AREA)
    void setNewFeatureType( int type ) { mNewFeatureType = type; }

  public slots:
    void onFeatureAdded( QgsFeatureId fid );
    void onFeatureDeleted( QgsFeatureId fid );
    void onGeometryChanged( QgsFeatureId fid, const QgsGeometry &geom );
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
    struct Map_info *map() const;
    void setMapset();
    bool openLayer();
    // update topo symbol of new features
    void setAddedFeaturesSymbol();
    // get new, not yet used cat
    int getNewCat();

    QgsGrassObject mGrassObject;
    // field part of layer or -1 if no field specified
    int mLayerField = -1;
    // layer type POINT, LINE, ...
    int mLayerType = Point;
    // grass feature type: GV_POINT, GV_LINE | GV_BOUNDARY, GV_AREA, ( GV_BOUNDARY, GV_CENTROID )
    int mGrassType = 0;
    // WKBPoint, WKBLineString, ...
    QgsWkbTypes::Type mQgisType = QgsWkbTypes::Unknown;
    QString mLayerName;
    QgsGrassVectorMapLayer *mLayer = nullptr;
    // The version of the map for which the instance was last time updated
    int mMapVersion = 0;

    bool mValid;
    long mNumberFeatures = 0;

    // create QgsFeatureId from GRASS geometry object id and cat
    static QgsFeatureId makeFeatureId( int grassId, int cat );

    /**
     * Gets attribute by category(key) and attribute number.
     *  \param layerId
     *  \param category (key)
     *  \param column column number ( < nColumns )
     *  \returns pointer to string representation of the value or NULL, this value must not be changed
     */
    static char *attribute( int layerId, int cat, int column );

    //! Check if provider is outdated and update if necessary
    void ensureUpdated() const;

    //! Check if layer is topology layer TOPO_POINT, TOPO_NODE, TOPO_LINE
    bool isTopoType() const;

    static bool isTopoType( int layerType );

    void setTopoFields();

    void setPoints( struct line_pnts *points, const QgsAbstractGeometry *geometry );

    // Get other edited layer, returns 0 if layer does not exist
    QgsGrassVectorMapLayer *otherEditLayer( int layerField );

    //! Fields used for topo layers
    QgsFields mTopoFields;

    //QgsFields mEditFields;

    QgsVectorLayerEditBuffer *mEditBuffer = nullptr;
    QgsVectorLayer *mEditLayer = nullptr;

    //  next digitized feature GRASS type
    int mNewFeatureType = 0;

    // Last version of layer fields during editing, updated after addAttribute and deleteAttribute
    QgsFields mEditLayerFields;

    // List of other layers opened for editing
    QList<QgsGrassVectorMapLayer *> mOtherEditLayers;

    // points and cats used only for editing
    struct line_pnts *mPoints = nullptr;
    struct line_cats *mCats = nullptr;

    // last geometry GV_* type, used e.g. for splitting features
    int mLastType = 0;

    // number of currently being edited providers
    static int sEditedCount;

    friend class QgsGrassFeatureSource;
    friend class QgsGrassFeatureIterator;
    friend class QgsGrassUndoCommandChangeAttribute;
};

#endif // QGSGRASSPROVIDER_H
