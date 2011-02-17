/***************************************************************************
                          qgsvectorlayer.h  -  description
                             -------------------
    begin                : Oct 29, 2003
    copyright            : (C) 2003 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSVECTORLAYER_H
#define QGSVECTORLAYER_H

#include <QMap>
#include <QSet>
#include <QList>
#include <QStringList>

#include "qgis.h"
#include "qgsmaplayer.h"
#include "qgsfeature.h"
#include "qgssnapper.h"
#include "qgsfield.h"


class QPainter;
class QImage;

class QgsAttributeAction;
class QgsCoordinateTransform;
class QgsGeometry;
class QgsGeometryVertexIndex;
class QgsMapToPixel;
class QgsLabel;
class QgsRectangle;
class QgsRenderer;
class QgsUndoCommand;
class QgsVectorDataProvider;
class QgsVectorOverlay;
class QgsSingleSymbolRendererV2;
class QgsRectangle;
class QgsVectorLayerJoinBuffer;

class QgsFeatureRendererV2;

typedef QList<int> QgsAttributeList;
typedef QSet<int> QgsFeatureIds;
typedef QSet<int> QgsAttributeIds;

struct CORE_EXPORT QgsVectorJoinInfo
{
  /**Join field in the target layer*/
  int targetField;
  /**Source layer*/
  QString joinLayerId;
  /**Join field in the source layer*/
  int joinField;
  /**True if the join is cached in virtual memory*/
  bool memoryCache;
  /**Cache for joined attributes to provide fast lookup (size is 0 if no memory caching)*/
  QHash< QString, QgsAttributeMap> cachedAttributes;
};

/**Join information prepared for fast attribute id mapping in QgsVectorLayerJoinBuffer::updateFeatureAttributes().
  Created in the select() method of QgsVectorLayerJoinBuffer for the joins that contain fetched attributes*/
struct CORE_EXPORT QgsFetchJoinInfo
{
  const QgsVectorJoinInfo* joinInfo;
  QgsAttributeList attributes; //attributes to fetch
  int indexOffset; //index offset between this layer and join layer
};

/** \ingroup core
 * Vector layer backed by a data source provider.
 */
class CORE_EXPORT QgsVectorLayer : public QgsMapLayer
{
    Q_OBJECT

  public:
    enum EditType
    {
      LineEdit,
      UniqueValues,
      UniqueValuesEditable,
      ValueMap,
      Classification,
      EditRange,
      SliderRange,
      CheckBox,    /* added in 1.4 */
      FileName,
      Enumeration,
      Immutable,   /* The attribute value should not be changed in the attribute form*/
      Hidden,      /* The attribute value should not be shown in the attribute form @added in 1.4 */
      TextEdit,    /* multiline edit @added in 1.4*/
      Calendar,    /* calendar widget @added in 1.5 */
      DialRange,   /* dial range @added in 1.5 */
    };

    struct RangeData
    {
      RangeData() {}
      RangeData( QVariant theMin, QVariant theMax, QVariant theStep )
          : mMin( theMin ), mMax( theMax ), mStep( theStep ) {}

      QVariant mMin;
      QVariant mMax;
      QVariant mStep;
    };

    /** Constructor */
    QgsVectorLayer( QString path = QString::null, QString baseName = QString::null,
                    QString providerLib = QString::null, bool loadDefaultStyleFlag = true );

    /** Destructor */
    virtual ~QgsVectorLayer();

    /** Returns the permanent storage type for this layer as a friendly name. */
    QString storageType() const;

    /** Capabilities for this layer in a friendly format. */
    QString capabilitiesString() const;

    /** Returns a comment for the data in the layer */
    QString dataComment() const;

    /** Set the primary display field to be used in the identify results dialog */
    void setDisplayField( QString fldName = "" );

    /** Returns the primary display field name used in the identify results dialog */
    const QString displayField() const;

    /** Returns the data provider */
    QgsVectorDataProvider* dataProvider();

    /** Returns the data provider in a const-correct manner */
    const QgsVectorDataProvider* dataProvider() const;

    /** Sets the textencoding of the data provider */
    void setProviderEncoding( const QString& encoding );

    /** Setup the coordinate system tranformation for the layer */
    void setCoordinateSystem();

    /**Joins another vector layer to this layer
      @param joinInfo join object containing join layer id, target and source field
      @param cacheInMemory if true: caches the content of the join layer in virtual memory*/
    void addJoin( QgsVectorJoinInfo joinInfo );

    /**Removes  a vector layer join*/
    void removeJoin( const QString& joinLayerId );

    const QList< QgsVectorJoinInfo >& vectorJoins() const;

    /** Get the label object associated with this layer */
    QgsLabel *label();

    const QgsLabel *label() const;

    QgsAttributeAction* actions() { return mActions; }

    /** The number of features that are selected in this layer */
    int selectedFeatureCount();

    /** Select features found within the search rectangle (in layer's coordinates) */
    void select( QgsRectangle & rect, bool lock );

    /** Select not selected features and deselect selected ones */
    void invertSelection();

    /** Invert selection of features found within the search rectangle (in layer's coordinates) */
    void invertSelectionInRectangle( QgsRectangle & rect );

    /** Get a copy of the user-selected features */
    QgsFeatureList selectedFeatures();

    /** Return reference to identifiers of selected features */
    const QgsFeatureIds& selectedFeaturesIds() const;

    /** Change selection to the new set of features */
    void setSelectedFeatures( const QgsFeatureIds& ids );

    /** Returns the bounding box of the selected features. If there is no selection, QgsRectangle(0,0,0,0) is returned */
    QgsRectangle boundingBoxOfSelected();

    /** Copies the symbology settings from another layer. Returns true in case of success */
    bool copySymbologySettings( const QgsMapLayer& other );

    /** Returns true if this layer can be in the same symbology group with another layer */
    bool hasCompatibleSymbology( const QgsMapLayer& other ) const;

    /** Returns a pointer to the renderer */
    const QgsRenderer* renderer() const;

    /** Sets the renderer. If a renderer is already present, it is deleted */
    void setRenderer( QgsRenderer * r );

    /** Return renderer V2.
     * @note added in 1.4 */
    QgsFeatureRendererV2* rendererV2();
    /** Set renderer V2.
     * @note ddded in 1.4
     */
    void setRendererV2( QgsFeatureRendererV2* r );
    /** Return whether using renderer V2.
     * @note added in 1.4
     */
    bool isUsingRendererV2();
    /** set whether to use renderer V2 for drawing.
     * @note added in 1.4
     */
    void setUsingRendererV2( bool usingRendererV2 );

    /** Draw layer with renderer V2.
     * @note added in 1.4
     */
    void drawRendererV2( QgsRenderContext& rendererContext, bool labeling );

    /** Draw layer with renderer V2 using symbol levels.
     * @note added in 1.4
     */
    void drawRendererV2Levels( QgsRenderContext& rendererContext, bool labeling );

    /** Returns point, line or polygon */
    QGis::GeometryType geometryType() const;

    /** Returns true if this is a geometry layer and false in case of NoGeometry (table only) or UnknownGeometry
      @note added in 1.7*/
    bool hasGeometryType() const;

    /**Returns the WKBType or WKBUnknown in case of error*/
    QGis::WkbType wkbType() const;

    /** Return the provider type for this layer */
    QString providerType() const;

    /** reads vector layer specific state from project file Dom node.
     *  @note Called by QgsMapLayer::readXML().
     */
    virtual bool readXml( QDomNode & layer_node );

    /** write vector layer specific state to project file Dom node.
     *  @note Called by QgsMapLayer::writeXML().
     */
    virtual bool writeXml( QDomNode & layer_node, QDomDocument & doc );

    /** Read the symbology for the current layer from the Dom node supplied.
    * @param node node that will contain the symbology definition for this layer.
    * @param errorMessage reference to string that will be updated with any error messages
    * @return true in case of success.
    */
    bool readSymbology( const QDomNode& node, QString& errorMessage );

    /** Write the symbology for the layer into the docment provided.
     *  @param node the node that will have the style element added to it.
     *  @param doc the document that will have the QDomNode added.
     *  @param errorMessage reference to string that will be updated with any error messages
     *  @return true in case of success.
     */
    bool writeSymbology( QDomNode& node, QDomDocument& doc, QString& errorMessage ) const;


    /**
     * Number of features in the layer. This is necessary if features are
     * added/deleted or the layer has been subsetted. If the data provider
     * chooses not to support this feature, the total number of features
     * can be returned.
     * @return long containing number of features
     */
    virtual long featureCount() const;

    /** This function does nothing useful, it's kept only for compatibility.
     * @todo to be removed
     */
    virtual long updateFeatureCount() const;

    /**
     * Set the string (typically sql) used to define a subset of the layer
     * @param subset The subset string. This may be the where clause of a sql statement
     *               or other defintion string specific to the underlying dataprovider
     *               and data store.
     * @return true, when setting the subset string was successful, false otherwise (added in 1.4)
     */
    virtual bool setSubsetString( QString subset );

    /**
     * Get the string (typically sql) used to define a subset of the layer
     * @return The subset string or QString::null if not implemented by the provider
     */
    virtual QString subsetString();

    /**
     * Select features with or without attributes in a given window.
     * @param fetchAttributes indizes of attributes to fetch
     * @param rect window (QgsRectangle() for all)
     * @param fetchGeometry fetch features with geometry
     * @param useIntersect fetch only features that actually intersect the window (not just the bounding box)
     */
    void select( QgsAttributeList fetchAttributes,
                 QgsRectangle rect = QgsRectangle(),
                 bool fetchGeometry = true,
                 bool useIntersect = false );

    /**
     * fetch a feature (after select)
     * @param feature buffer to read the feature into
     * @return true, if a feature was fetched, false, if there are no more features
     */
    bool nextFeature( QgsFeature& feature );

    /**Gets the feature at the given feature id. Considers the changed, added, deleted and permanent features
     @return true in case of success*/
    bool featureAtId( int featureId, QgsFeature &f, bool fetchGeometries = true, bool fetchAttributes = true );

    /** Adds a feature
        @param f feature to add
        @param alsoUpdateExtent If True, will also go to the effort of e.g. updating the extents.
        @return                    True in case of success and False in case of error
     */
    bool addFeature( QgsFeature& f, bool alsoUpdateExtent = true );


    /** Insert a new vertex before the given vertex number,
     *  in the given ring, item (first number is index 0), and feature
     *  Not meaningful for Point geometries
     */
    bool insertVertex( double x, double y, int atFeatureId, int beforeVertex );

    /** Moves the vertex at the given position number,
     *  ring and item (first number is index 0), and feature
     *  to the given coordinates
     */
    bool moveVertex( double x, double y, int atFeatureId, int atVertex );

    /** Deletes a vertex from a feature
     */
    bool deleteVertex( int atFeatureId, int atVertex );

    /** Deletes the selected features
     *  @return true in case of success and false otherwise
     */
    bool deleteSelectedFeatures();

    /**Adds a ring to polygon/multipolygon features
     @return
       0 in case of success,
       1 problem with feature type,
       2 ring not closed,
       3 ring not valid,
       4 ring crosses existing rings,
       5 no feature found where ring can be inserted*/
    int addRing( const QList<QgsPoint>& ring );

    /**Adds a new island polygon to a multipolygon feature
     @return
       0 in case of success,
       1 if selected feature is not multipolygon,
       2 if ring is not a valid geometry,
       3 if new polygon ring not disjoint with existing rings,
       4 if no feature was selected,
       5 if several features are selected,
       6 if selected geometry not found*/
    int addIsland( const QList<QgsPoint>& ring );

    /**Translates feature by dx, dy
       @param featureId id of the feature to translate
       @param dx translation of x-coordinate
       @param dy translation of y-coordinate
       @return 0 in case of success*/
    int translateFeature( int featureId, double dx, double dy );

    /**Splits features cut by the given line
       @param splitLine line that splits the layer features
       @param topologicalEditing true if topological editing is enabled
       @return 0 in case of success, 4 if there is a selection but no feature split*/
    int splitFeatures( const QList<QgsPoint>& splitLine, bool topologicalEditing = false );

    /**Changes the specified geometry such that it has no intersections with other
       polygon (or multipolygon) geometries in this vector layer
    @param geom geometry to modify
    @return 0 in case of success*/
    int removePolygonIntersections( QgsGeometry* geom );

    /**Adds topological points for every vertex of the
     geometry
    @param geom the geometry where each vertex is added to segments of other features
    Note: geom is not going to be modified by the function
    @return 0 in case of success*/
    int addTopologicalPoints( QgsGeometry* geom );

    /**Adds a vertex to segments which intersect point p but don't
     already have a vertex there. If a feature already has a vertex at position p,
     no additional vertex is inserted. This method is useful for topological
     editing.
    @param p position of the vertex
    @return 0 in case of success*/
    int addTopologicalPoints( const QgsPoint& p );

    /**Inserts vertices to the snapped segments.
    This is useful for topological editing if snap to segment is enabled.
    @param snapResults results collected from the snapping operation
    @return 0 in case of success*/
    int insertSegmentVerticesForSnap( const QList<QgsSnappingResult>& snapResults );

    /** Set labels on */
    void enableLabels( bool on );

    /** Label is on */
    bool hasLabelsEnabled( void ) const;

    /** Returns true if the provider is in editing mode */
    virtual bool isEditable() const;

    /** Returns true if the provider is in read-only mode
     * @note added in 1.6 */
    virtual bool isReadOnly() const;

    /** Returns true if the provider has been modified since the last commit */
    virtual bool isModified() const;

    /**Snaps a point to the closest vertex if there is one within the snapping tolerance
       @param point       The point which is set to the position of a vertex if there is one within the snapping tolerance.
       If there is no point within this tolerance, point is left unchanged.
       @param tolerance   The snapping tolerance
       @return true if point has been snapped, false if no vertex within search tolerance*/
    bool snapPoint( QgsPoint& point, double tolerance );

    /**Snaps to segment or vertex within given tolerance
       @param startPoint point to snap (in layer coordinates)
       @param snappingTolerance distance tolerance for snapping
       @param snappingResults snapping results. Key is the distance between startPoint and snapping target
       @param snap_to to segment / to vertex
       @return 0 in case of success
    */
    int snapWithContext( const QgsPoint& startPoint,
                         double snappingTolerance,
                         QMultiMap < double,
                         QgsSnappingResult > & snappingResults,
                         QgsSnapper::SnappingType snap_to );

    /**Synchronises with changes in the datasource
      @note added in version 1.6*/
    virtual void reload();

    /** Draws the layer
     *  @return false if an error occurred during drawing
     */
    bool draw( QgsRenderContext& rendererContext );

    /** Draws the layer labels using coordinate transformation */
    void drawLabels( QgsRenderContext& rendererContext );

    /** returns field list in the to-be-committed state */
    const QgsFieldMap &pendingFields() const;

    /** returns list of attributes */
    QgsAttributeList pendingAllAttributesList();

    /** returns feature count after commit */
    int pendingFeatureCount();

    /** Make layer read-only (editing disabled) or not
     *  @return false if the layer is in editing yet
     *  @note added in 1.6
     */
    bool setReadOnly( bool readonly = true );

    /** Sets whether some features are modified or not */
    void setModified( bool modified = true, bool onlyGeometryWasModified = false );

    /** Make layer editable */
    bool startEditing();

    /** change feature's geometry
      @note added in version 1.2 */
    bool changeGeometry( int fid, QgsGeometry* geom );

    /** changed an attribute value (but does not commit it) */
    bool changeAttributeValue( int fid, int field, QVariant value, bool emitSignal = true );

    /** add an attribute field (but does not commit it)
        returns true if the field was added
      @note added in version 1.2 */
    bool addAttribute( const QgsField &field );

    /** add an attribute field (but does not commit it)
      returns true if the field was added
      @deprecated */
    bool addAttribute( QString name, QString type );

    /**Sets an alias (a display name) for attributes to display in dialogs
      @note added in version 1.2*/
    void addAttributeAlias( int attIndex, QString aliasString );

    /**Returns the alias of an attribute name or an empty string if there is no alias
      @note added in version 1.2*/
    QString attributeAlias( int attributeIndex ) const;

    /**Convenience function that returns the attribute alias if defined or the field name else
      @note added in version 1.2*/
    QString attributeDisplayName( int attributeIndex ) const;

    /** delete an attribute field (but does not commit it) */
    bool deleteAttribute( int attr );

    /** Insert a copy of the given features into the layer  (but does not commit it) */
    bool addFeatures( QgsFeatureList features, bool makeSelected = true );

    /** delete a feature from the layer (but does not commit it) */
    bool deleteFeature( int fid );

    /**
      Attempts to commit any changes to disk.  Returns the result of the attempt.
      If a commit fails, the in-memory changes are left alone.

      This allows editing to continue if the commit failed on e.g. a
      disallowed value in a Postgres database - the user can re-edit and try
      again.

      The commits occur in distinct stages,
      (add attributes, add features, change attribute values, change
      geometries, delete features, delete attributes)
      so if a stage fails, it's difficult to roll back cleanly.
      Therefore any error message also includes which stage failed so
      that the user has some chance of repairing the damage cleanly.
     */
    bool commitChanges();
    const QStringList &commitErrors();

    /** Stop editing and discard the edits */
    bool rollBack();

    /**get edit type*/
    EditType editType( int idx );

    /**set edit type*/
    void setEditType( int idx, EditType edit );

    /** set string representing 'true' for a checkbox (added in 1.4) */
    void setCheckedState( int idx, QString checked, QString notChecked );

    /** return string representing 'true' for a checkbox (added in 1.4) */
    QPair<QString, QString> checkedState( int idx );

    /** get edit form (added in 1.4) */
    QString editForm();

    /** set edit form (added in 1.4) */
    void setEditForm( QString ui );

    /** get annotation form (added in 1.5)*/
    QString annotationForm() const { return mAnnotationForm; }

    /** set annotation form for layer (added in 1.5)*/
    void setAnnotationForm( const QString& ui );

    /** get python function for edit form initialization (added in 1.4) */
    QString editFormInit();

    /** set python function for edit form initialization (added in 1.4) */
    void setEditFormInit( QString function );

    /**access value map*/
    QMap<QString, QVariant> &valueMap( int idx );

    /**access range */
    RangeData &range( int idx );

    /**Adds a new overlay to this class. QgsVectorLayer takes ownership of the object
    @note this method was added in version 1.1
    */
    void addOverlay( QgsVectorOverlay* overlay );

    /**Removes all overlays of a given type
    @note this method was added in version 1.1
    */
    void removeOverlay( const QString& typeName );

    /**Returns pointers to the overlays of this layer
    @note this method was added in version 1.1
    */
    void vectorOverlays( QList<QgsVectorOverlay*>& overlayList );

    /**Returns the (first) overlay of a type, e.g. diagram or label
    @note this method was added in version 1.1
    */
    QgsVectorOverlay* findOverlayByType( const QString& typeName );


    /**
     * Create edit command for undo/redo operations
     * @param text text which is to be displayed in undo window
     */
    void beginEditCommand( QString text );

    /** Finish edit command and add it to undo/redo stack */
    void endEditCommand();

    /** Destroy active command and reverts all changes in it */
    void destroyEditCommand();

    /** Execute undo operation. To be called only from QgsVectorLayerUndoCommand. */
    void undoEditCommand( QgsUndoCommand* cmd );

    /** Execute redo operation. To be called only from QgsVectorLayerUndoCommand. */
    void redoEditCommand( QgsUndoCommand* cmd );

    /** Returns the index of a field name or -1 if the field does not exist
      @note this method was added in version 1.4
     */
    int fieldNameIndex( const QString& fieldName ) const;

    /** Editing vertex markers
      @note public from version 1.4 */
    enum VertexMarkerType
    {
      SemiTransparentCircle,
      Cross,
      NoMarker  /* added in version 1.1 */
    };

    /** Draws a vertex symbol at (screen) coordinates x, y. (Useful to assist vertex editing.)
      @note public and static from version 1.4 */
    static void drawVertexMarker( double x, double y, QPainter& p, QgsVectorLayer::VertexMarkerType type, int vertexSize );

    /**Assembles mUpdatedFields considering provider fields, joined fields and added fields
     @note added in version 1.6*/
    void updateFieldMap();

    /**Caches joined attributes if required (and not already done)*/
    void createJoinCaches();


  public slots:
    /** Select feature by its ID, optionally emit signal selectionChanged() */
    void select( int featureId, bool emitSignal = true );

    /** Deselect feature by its ID, optionally emit signal selectionChanged() */
    void deselect( int featureId, bool emitSignal = true );

    /** Clear selection */
    void removeSelection( bool emitSignal = true );

    void triggerRepaint();

    /** Update the extents for the layer. This is necessary if features are
     *  added/deleted or the layer has been subsetted.
     */
    virtual void updateExtents();

    /**Check if there is a join with a layer that will be removed*/
    void checkJoinLayerRemove( QString theLayerId );

  signals:

    /** This signal is emited when selection was changed */
    void selectionChanged();

    /** This signal is emitted when modifications has been done on layer */
    void layerModified( bool onlyGeometry );

    void editingStarted();
    void editingStopped();
    void attributeAdded( int idx );
    void attributeDeleted( int idx );
    void featureAdded( int fid );  // added in 1.7
    void featureDeleted( int fid );
    void layerDeleted();

    void attributeValueChanged( int fid, int idx, const QVariant & );

    /** Signals emitted after committing changes
      \note added in v1.6 */
    void committedAttributesDeleted( const QString& layerId, const QgsAttributeIds& deletedAttributeIds );
    void committedAttributesAdded( const QString& layerId, const QList<QgsField>& addedAttributes );
    void committedFeaturesAdded( const QString& layerId, const QgsFeatureList& addedFeatures );
    void committedFeaturesRemoved( const QString& layerId, const QgsFeatureIds& deletedFeatureIds );
    void committedAttributeValuesChanges( const QString& layerId, const QgsChangedAttributesMap& changedAttributesValues );
    void committedGeometriesChanges( const QString& layerId, const QgsGeometryMap& changedGeometries );

  private:                       // Private methods

    /** vector layers are not copyable */
    QgsVectorLayer( QgsVectorLayer const & rhs );

    /** vector layers are not copyable */
    QgsVectorLayer & operator=( QgsVectorLayer const & rhs );

    /** bind layer to a specific data provider
       @param provider should be "postgres", "ogr", or ??
       @todo XXX should this return bool?  Throw exceptions?
    */
    bool setDataProvider( QString const & provider );

    /** Draws features. May cause projections exceptions to be generated
     *  (i.e., code that calls this function needs to catch them) */
    void drawFeature( QgsRenderContext &renderContext,
                      QgsFeature& fet,
                      QImage* marker );

    /** Convenience function to transform the given point */
    void transformPoint( double& x, double& y,
                         const QgsMapToPixel* mtp, const QgsCoordinateTransform* ct );

    void transformPoints( std::vector<double>& x, std::vector<double>& y, std::vector<double>& z, QgsRenderContext &renderContext );

    /** Draw the linestring as given in the WKB format. Returns a pointer
     * to the byte after the end of the line string binary data stream (WKB).
     */
    unsigned char *drawLineString( unsigned char *WKBlinestring, QgsRenderContext &renderContext );

    /** Draw the polygon as given in the WKB format. Returns a pointer to
     *  the byte after the end of the polygon binary data stream (WKB).
     */
    unsigned char *drawPolygon( unsigned char *WKBpolygon, QgsRenderContext &renderContext );

    /** Goes through all features and finds a free id (e.g. to give it temporarily to a not-commited feature) */
    int findFreeId();

    /**Deletes the geometries in mCachedGeometries*/
    void deleteCachedGeometries();

    /**Snaps to a geometry and adds the result to the multimap if it is within the snapping result
     @param startPoint start point of the snap
     @param featureId id of feature
     @param geom geometry to snap
     @param sqrSnappingTolerance squared search tolerance of the snap
     @param snappingResults list to which the result is appended
     @param snap_to snap to vertex or to segment
    */
    void snapToGeometry( const QgsPoint& startPoint, int featureId, QgsGeometry* geom, double sqrSnappingTolerance,
                         QMultiMap<double, QgsSnappingResult>& snappingResults, QgsSnapper::SnappingType snap_to ) const;

    /**Little helper function that gives bounding box from a list of points.
    @return 0 in case of success*/
    int boundingBoxFromPointList( const QList<QgsPoint>& list, double& xmin, double& ymin, double& xmax, double& ymax ) const;

    /**Reads vertex marker type from settings*/
    static QgsVectorLayer::VertexMarkerType currentVertexMarkerType();

    /**Reads vertex marker size from settings*/
    static int currentVertexMarkerSize();

    /**Update feature with uncommited attribute updates and joined attributes*/
    void updateFeatureAttributes( QgsFeature &f, bool all = false );

    /**Adds joined attributes to a feature
      @param f the feature to add the attributes
      @param joinInfo vector join
      @param joinFieldName name of the (source) join Field
      @param joinValue lookup value for join
      @param attributes (join layer) attribute indices to add
      @param attributeIndexOffset index offset to get from join layer attribute index to layer index*/
    void addJoinedFeatureAttributes( QgsFeature& f, const QgsVectorJoinInfo& joinInfo, const QString& joinFieldName, const QVariant& joinValue,
                                     const QgsAttributeList& attributes, int attributeIndexOffset );

    /**Update feature with uncommited geometry updates*/
    void updateFeatureGeometry( QgsFeature &f );

    /** Record changed geometry, store in active command (if any) */
    void editGeometryChange( int featureId, QgsGeometry& geometry );

    /** Record added feature, store in active command (if any) */
    void editFeatureAdd( QgsFeature& feature );

    /** Record deleted feature, store in active command (if any) */
    void editFeatureDelete( int featureId );

    /** Record changed attribute, store in active command (if any) */
    void editAttributeChange( int featureId, int field, QVariant value );

    /** Stop version 2 renderer and selected renderer (if required) */
    void stopRendererV2( QgsRenderContext& rendererContext, QgsSingleSymbolRendererV2* selRenderer );

    /**Updates an index in an attribute map to a new value (usually necessary because of a join operation)*/
    void updateAttributeMapIndex( QgsAttributeMap& map, int oldIndex, int newIndex ) const;

  private:                       // Private attributes

    /** Update threshold for drawing features as they are read. A value of zero indicates
     *  that no features will be drawn until all have been read
     */
    int mUpdateThreshold;

    /** Pointer to data provider derived from the abastract base class QgsDataProvider */
    QgsVectorDataProvider *mDataProvider;

    /** index of the primary label field */
    QString mDisplayField;

    /** Data provider key */
    QString mProviderKey;

    /** The user-defined actions that are accessed from the Identify Results dialog box */
    QgsAttributeAction* mActions;

    /** Flag indicating whether the layer is in editing mode or not */
    bool mEditable;

    /** Flag indicating whether the layer is in read-only mode (editing disabled) or not */
    bool mReadOnly;

    /** Flag indicating whether the layer has been modified since the last commit */
    bool mModified;

    /** cache of the committed geometries retrieved *for the current display* */
    QgsGeometryMap mCachedGeometries;

    /** extent for which there are cached geometries */
    QgsRectangle mCachedGeometriesRect;

    /** Set holding the feature IDs that are activated.  Note that if a feature
        subsequently gets deleted (i.e. by its addition to mDeletedFeatureIds),
        it always needs to be removed from mSelectedFeatureIds as well.
     */
    QgsFeatureIds mSelectedFeatureIds;

    /** Deleted feature IDs which are not commited.  Note a feature can be added and then deleted
        again before the change is committed - in that case the added feature would be removed
        from mAddedFeatures only and *not* entered here.
     */
    QgsFeatureIds mDeletedFeatureIds;

    /** New features which are not commited.  Note a feature can be added and then changed,
        therefore the details here can be overridden by mChangedAttributeValues and mChangedGeometries.
     */
    QgsFeatureList mAddedFeatures;

    /** Changed attributes values which are not commited */
    QgsChangedAttributesMap mChangedAttributeValues;

    /** deleted attributes fields which are not commited */
    QgsAttributeIds mDeletedAttributeIds;

    /** added attributes fields which are not commited */
    QgsAttributeIds mAddedAttributeIds;

    /** Changed geometries which are not commited. */
    QgsGeometryMap mChangedGeometries;

    /** field map to commit */
    QgsFieldMap mUpdatedFields;

    /**Map that stores the aliases for attributes. Key is the attribute index and value the alias for that attribute*/
    QMap<int, QString> mAttributeAliasMap;

    /** max field index */
    int mMaxUpdatedIndex;

    /** Geometry type as defined in enum WkbType (qgis.h) */
    int mWkbType;

    QgsUndoCommand * mActiveCommand;

    /** Renderer object which holds the information about how to display the features */
    QgsRenderer *mRenderer;

    /** Renderer V2 */
    QgsFeatureRendererV2 *mRendererV2;

    /** whether to use V1 or V2 renderer */
    bool mUsingRendererV2;

    /** Label */
    QgsLabel *mLabel;

    /** Display labels */
    bool mLabelOn;

    /**The current type of editing marker*/
    QgsVectorLayer::VertexMarkerType mCurrentVertexMarkerType;

    /** The current size of editing marker */
    int mCurrentVertexMarkerSize;

    /**Flag if the vertex markers should be drawn only for selection (true) or for all features (false)*/
    bool mVertexMarkerOnlyForSelection;

    /**List of overlays. Vector overlays will be rendered on top of all maplayers*/
    QList<QgsVectorOverlay*> mOverlays;

    QStringList mCommitErrors;

    QMap< QString, EditType > mEditTypes;
    QMap< QString, QMap<QString, QVariant> > mValueMaps;
    QMap< QString, RangeData > mRanges;
    QMap< QString, QPair<QString, QString> > mCheckedStates;

    QString mEditForm, mEditFormInit;
    //annotation form for this layer
    QString mAnnotationForm;

    bool mFetching;
    QgsRectangle mFetchRect;
    QgsAttributeList mFetchAttributes;
    QgsAttributeList mFetchProvAttributes;
    bool mFetchGeometry;

    QSet<int> mFetchConsidered;
    QgsGeometryMap::iterator mFetchChangedGeomIt;
    QgsFeatureList::iterator mFetchAddedFeaturesIt;

    //stores information about joined layers
    QgsVectorLayerJoinBuffer* mJoinBuffer;
};

#endif
