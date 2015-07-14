
typedef QList<int> QgsAttributeList;
typedef QSet<int> QgsAttributeIds;

class QgsAttributeEditorElement : QObject
{
%TypeHeaderCode
#include "qgsvectorlayer.h"
%End

%ConvertToSubClassCode
  QgsAttributeEditorElement* e = qobject_cast<QgsAttributeEditorElement*>( sipCpp );

  sipClass = 0;
  if ( e )
  {
    switch ( e->type() )
    {
      case QgsAttributeEditorElement::AeTypeContainer: sipClass = sipClass_QgsAttributeEditorContainer; break;
      case QgsAttributeEditorElement::AeTypeField:   sipClass = sipClass_QgsAttributeEditorField; break;
      case QgsAttributeEditorElement::AeTypeRelation:  sipClass = sipClass_QgsAttributeEditorRelation; break;
    }
  }
%End


  public:

    enum AttributeEditorType
    {
      AeTypeContainer,
      AeTypeField,
      AeTypeRelation,
      AeTypeInvalid
    };

    QgsAttributeEditorElement( AttributeEditorType type, QString name, QObject *parent /TransferThis/ = NULL );

    virtual ~QgsAttributeEditorElement();

    QString name() const;
    AttributeEditorType type() const;

    virtual QDomElement toDomElement( QDomDocument& doc ) const = 0;
};

class QgsAttributeEditorContainer : QgsAttributeEditorElement
{
%TypeHeaderCode
#include "qgsvectorlayer.h"
%End

  public:
    QgsAttributeEditorContainer( QString name, QObject *parent  /TransferThis/ );

    ~QgsAttributeEditorContainer();

    virtual QDomElement toDomElement( QDomDocument& doc ) const;
    virtual void addChildElement( QgsAttributeEditorElement *widget );
    virtual void setIsGroupBox( bool isGroupBox );
    virtual bool isGroupBox() const;
    virtual void setName( const QString& name );
    QList<QgsAttributeEditorElement*> children() const;
};

class QgsAttributeEditorField : QgsAttributeEditorElement
{
%TypeHeaderCode
#include "qgsvectorlayer.h"
%End
  public:
    QgsAttributeEditorField( QString name, int idx, QObject *parent /TransferThis/ );

    ~QgsAttributeEditorField();

    virtual QDomElement toDomElement( QDomDocument& doc ) const;
    int idx() const;
};

/** @note Added in 2.1 */
class QgsAttributeEditorRelation : QgsAttributeEditorElement
{
  public:
    QgsAttributeEditorRelation( QString name, const QString &relationId, QObject *parent /TransferThis/ );

    QgsAttributeEditorRelation( QString name, const QgsRelation& relation, QObject *parent /TransferThis/);

    ~QgsAttributeEditorRelation();

    virtual QDomElement toDomElement( QDomDocument& doc ) const;
    const QgsRelation& relation() const;

    /**
     * Initializes the relation from the id
     *
     * @param relManager The relation manager to use for the initialization
     * @return true if the relation was found in the relationmanager
     */
    bool init( QgsRelationManager *relManager );
};

struct QgsVectorJoinInfo
{
%TypeHeaderCode
#include "qgsvectorlayer.h"
%End

  /** Join field in the target layer*/
  QString targetFieldName;
  /** Source layer*/
  QString joinLayerId;
  /** Join field in the source layer*/
  QString joinFieldName;
  /** True if the join is cached in virtual memory*/
  bool memoryCache;
  /** Cache for joined attributes to provide fast lookup (size is 0 if no memory caching)
    @note not available in python bindings
    */
  // QHash< QString, QgsAttributeMap> cachedAttributes;

  bool operator==( const QgsVectorJoinInfo& other ) const;

  /** An optional prefix. If it is a Null string "{layername}_" will be used
   * @note Added in 2.8
   */
  QString prefix;

  /** Set subset of fields to be used from joined layer. Takes ownership of the passed pointer. Null pointer tells to use all fields.
    @note added in 2.6 */
  void setJoinFieldNamesSubset( QStringList* fieldNamesSubset /Transfer/ );
  /** Get subset of fields to be used from joined layer. All fields will be used if null is returned.
    @note added in 2.6 */
  QStringList* joinFieldNamesSubset() const;
};


class QgsVectorLayer : QgsMapLayer
{
%TypeHeaderCode
#include "qgsvectorlayer.h"
%End

  public:
    /** The different types to layout the attribute editor. */
    enum EditorLayout
    {
      GeneratedLayout,
      TabLayout,
      UiFileLayout
    };

    /**
     * @deprecated Use the editorWidgetV2() system instead
     */
    enum EditType
    {
      LineEdit,
      UniqueValues,
      UniqueValuesEditable,
      ValueMap,
      Classification,
      EditRange,
      SliderRange,
      CheckBox,
      FileName,
      Enumeration,
      Immutable,      /**< The attribute value should not be changed in the attribute form */
      Hidden,         /**< The attribute value should not be shown in the attribute form */
      TextEdit,       /**< multiline edit */
      Calendar,       /**< calendar widget */
      DialRange,      /**< dial range */
      ValueRelation,  /**< value map from an table */
      UuidGenerator,  /**< uuid generator - readonly and automatically intialized */
      Photo,          /**< phote widget */
      WebView,        /**< webview widget */
      Color,          /**< color */
      EditorWidgetV2, /**< modularized edit widgets @note added in 2.1 */
    };

    /** Types of feature form suppression after feature creation
     * @note added in 2.1 */
    enum FeatureFormSuppress
    {
      SuppressDefault, // use the application-wide setting
      SuppressOn,
      SuppressOff,
    };

    struct RangeData
    {
      RangeData();
      RangeData( QVariant theMin, QVariant theMax, QVariant theStep );

      QVariant mMin;
      QVariant mMax;
      QVariant mStep;
    };

    struct ValueRelationData
    {
      ValueRelationData();
      ValueRelationData( QString layer, QString key, QString value, bool allowNull, bool orderByValue,
                         bool allowMulti = false,
                         QString filterExpression = QString::null );

      QString mLayer;
      QString mKey;
      QString mValue;
      QString mFilterExpression;
      bool mAllowNull;
      bool mOrderByValue;
      bool mAllowMulti;  /* allow selection of multiple keys */
    };

    struct GroupData
    {
      GroupData();
      GroupData( QString name, QList<QString> fields );
      QString mName;
      QList<QString> mFields;
    };

    struct TabData
    {
      TabData();
      TabData( QString name, QList<QString> fields, QList<QgsVectorLayer::GroupData> groups );
      QString mName;
      QList<QString> mFields;
      QList<QgsVectorLayer::GroupData> mGroups;
    };

    /** Constructor - creates a vector layer
     *
     * The QgsVectorLayer is constructed by instantiating a data provider.  The provider
     * interprets the supplied path (url) of the data source to connect to and access the
     * data.
     *
     * @param  path  The path or url of the parameter.  Typically this encodes
     *               parameters used by the data provider as url query items.
     * @param  baseName The name used to represent the layer in the legend
     * @param  providerLib  The name of the data provider, eg "memory", "postgres"
     * @param  loadDefaultStyleFlag whether to load the default style
     *
     */
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

    /** Set the preview expression, used to create a human readable preview string.
     *  Used e.g. in the attribute table feature list. Uses { @link QgsExpression }.
     *
     *  @param displayExpression The expression which will be used to preview features
     *                           for this layer
     */
    void setDisplayExpression( const QString &displayExpression );

    /**
     *  Get the preview expression, used to create a human readable preview string.
     *  Uses { @link QgsExpression }
     *
     *  @return The expression which will be used to preview features for this layer
     */
    const QString displayExpression();

    /** Returns the data provider */
    QgsVectorDataProvider* dataProvider();

    /** Returns the data provider in a const-correct manner
        @note not available in python bindings
      */
    // const QgsVectorDataProvider* dataProvider() const;

    /** Sets the textencoding of the data provider */
    void setProviderEncoding( const QString& encoding );

    /** Setup the coordinate system transformation for the layer */
    void setCoordinateSystem();

    /** Joins another vector layer to this layer
      @param joinInfo join object containing join layer id, target and source field
      @note since 2.6 returns bool indicating whether the join can be added */
    bool addJoin( const QgsVectorJoinInfo& joinInfo );

    /** Removes a vector layer join */
    void removeJoin( const QString& joinLayerId );

    const QList< QgsVectorJoinInfo > vectorJoins() const;

    /**
     * Add a new field which is calculated by the expression specified
     *
     * @param exp The expression which calculates the field
     * @param fld The field to calculate
     *
     * @return The index of the new field
     *
     * @note added in 2.6, return value added in 2.9
     */
    int addExpressionField( const QString& exp, const QgsField& fld );

    /**
     * Remove an expression field
     *
     * @param index The index of the field
     *
     * @note added in 2.6
     */
    void removeExpressionField( int index );

    /**
     * Returns the expressoin used for a given expression field
     *
     * @param index An index of an epxression based (virtual) field
     *
     * @return The expression for the field at index
     *
     * @note added in 2.9
     */
    const QString expressionField( int index );

    /**
     * Changes the expression used to define an expression based (virtual) field
     *
     * @param index The index of the expression to change
     *
     * @param exp The new expression to set
     *
     * @note added in 2.9
     */
    void updateExpressionField( int index, const QString& exp );

    /** Get the label object associated with this layer */
    QgsLabel *label();

    // const QgsLabel *label() const;

    QgsAttributeAction *actions();

    /**
     * The number of features that are selected in this layer
     *
     * @return See description
     */
    int selectedFeatureCount();

    /**
     * Select features found within the search rectangle (in layer's coordinates)
     *
     * @param rect            The search rectangle
     * @param addToSelection  If set to true will not clear before selecting
     *
     * @see   invertSelectionInRectangle(QgsRectangle & rect)
     */
    void select( QgsRectangle & rect, bool addToSelection );

    /**
     * Modifies the current selection on this layer
     *
     * @param selectIds    Select these ids
     * @param deselectIds  Deselect these ids
     *
     * @see   select(QgsFeatureIds)
     * @see   select(QgsFeatureId)
     * @see   deselect(QgsFeatureIds)
     * @see   deselect(QgsFeatureId)
     */
    void modifySelection( QgsFeatureIds selectIds, QgsFeatureIds deselectIds );

    /** Select not selected features and deselect selected ones */
    void invertSelection();

    /** Select all the features */
    void selectAll();

    /** Get all feature Ids */
    QgsFeatureIds allFeatureIds();

    /**
     * Invert selection of features found within the search rectangle (in layer's coordinates)
     *
     * @param rect  The rectangle in which the selection of features will be inverted
     *
     * @see   invertSelection()
     */
    void invertSelectionInRectangle( QgsRectangle & rect );

    /**
     * Get a copy of the user-selected features
     *
     * @return A list of { @link QgsFeature } 's
     *
     * @see    selectedFeaturesIds()
     * @see    selectedFeaturesIterator() which is more memory friendly when handling large selections
     */
    QgsFeatureList selectedFeatures();

    /**
     * Get an iterator of the selected features
     *
     * @param request You may specify a request, e.g. to limit the set of requested attributes.
     *                Any filter on the request will be discarded.
     *
     * @return Iterator over the selected features
     *
     * @see    selectedFeaturesIds()
     * @see    selectedFeatures()
     */
    QgsFeatureIterator selectedFeaturesIterator( QgsFeatureRequest request = QgsFeatureRequest() );

    /**
     * Return reference to identifiers of selected features
     *
     * @return A list of { @link QgsFeatureId } 's
     * @see selectedFeatures()
     */
    const QgsFeatureIds &selectedFeaturesIds() const;

    /**
     * Change selection to the new set of features. Dismisses the current selection.
     * Will emit the { @link selectionChanged( QgsFeatureIds, QgsFeatureIds, bool ) } signal with the
     * clearAndSelect flag set.
     *
     * @param ids   The ids which will be the new selection
     */
    void setSelectedFeatures( const QgsFeatureIds &ids );

    /** Returns the bounding box of the selected features. If there is no selection, QgsRectangle(0,0,0,0) is returned */
    QgsRectangle boundingBoxOfSelected();

    /** Returns whether the layer contains labels which are enabled and should be drawn.
     * @return true if layer contains enabled labels
     * @note added in QGIS 2.9
     */
    bool labelsEnabled() const;

    /** Returns whether the layer contains diagrams which are enabled and should be drawn.
     * @return true if layer contains enabled diagrams
     * @note added in QGIS 2.9
     */
    bool diagramsEnabled() const;

    /** Sets diagram rendering object (takes ownership) */
    void setDiagramRenderer( QgsDiagramRendererV2* r /Transfer/ );
    const QgsDiagramRendererV2* diagramRenderer() const;

    void setDiagramLayerSettings( const QgsDiagramLayerSettings& s );
    const QgsDiagramLayerSettings *diagramLayerSettings() const;

    /** Return renderer V2. */
    QgsFeatureRendererV2* rendererV2();
    /** Set renderer V2. */
    void setRendererV2( QgsFeatureRendererV2* r /Transfer/ );

    /** Returns point, line or polygon */
    QGis::GeometryType geometryType() const;

    /** Returns true if this is a geometry layer and false in case of NoGeometry (table only) or UnknownGeometry */
    bool hasGeometryType() const;

    /** Returns the WKBType or WKBUnknown in case of error*/
    QGis::WkbType wkbType() const;

    /** Return the provider type for this layer */
    QString providerType() const;

    /** Reads vector layer specific state from project file Dom node.
     *  @note Called by QgsMapLayer::readXML().
     */
    virtual bool readXml( const QDomNode& layer_node );

    /** Write vector layer specific state to project file Dom node.
     *  @note Called by QgsMapLayer::writeXML().
     */
    virtual bool writeXml( QDomNode & layer_node, QDomDocument & doc );

    /**
     * Save named and sld style of the layer to the style table in the db.
     * @param name
     * @param description
     * @param useAsDefault
     * @param uiFileContent
     * @param msgError
     */
    virtual void saveStyleToDatabase( QString name, QString description,
                                      bool useAsDefault, QString uiFileContent,
                                      QString &msgError );

    /**
     * Lists all the style in db split into related to the layer and not related to
     * @param ids the list in which will be stored the style db ids
     * @param names the list in which will be stored the style names
     * @param descriptions the list in which will be stored the style descriptions
     * @param msgError
     * @return the number of styles related to current layer
     */
    virtual int listStylesInDatabase( QStringList &ids, QStringList &names,
                                      QStringList &descriptions, QString &msgError );

    /**
     * Will return the named style corresponding to style id provided
     */
    virtual QString getStyleFromDatabase( QString styleId, QString &msgError );

    /**
     * Load a named style from file/local db/datasource db
     * @param theURI the URI of the style or the URI of the layer
     * @param theResultFlag will be set to true if a named style is correctly loaded
     * @param loadFromLocalDb if true forces to load from local db instead of datasource one
     */
    virtual QString loadNamedStyle( const QString &theURI, bool &theResultFlag, bool loadFromLocalDb );

    /**
     * Calls loadNamedStyle( theURI, theResultFlag, false );
     * Retained for backward compatibility
     */
    virtual QString loadNamedStyle( const QString &theURI, bool &theResultFlag );

    /**
     * Will load a named style from a provided QML string.
     *
     * @param namedStyle  A QML string
     * @param errorMsg    An error message indicating problems if any
     *
     * @return true on success
     *
     * @deprecated Will be removed for QGIS 3 in favor of importNamedStyle
     */
    virtual bool applyNamedStyle( QString namedStyle, QString &errorMsg /Out/ );

    /** Convert a saved attribute editor element into a AttributeEditor structure as it's used internally.
     * @param elem the DOM element
     * @param parent the QObject which will own this object
     */
    QgsAttributeEditorElement* attributeEditorElementFromDomElement( QDomElement &elem, QObject* parent );

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

    bool writeSld( QDomNode& node, QDomDocument& doc, QString& errorMessage ) const;
    bool readSld( const QDomNode& node, QString& errorMessage );

    /**
     * Number of features in the layer. This is necessary if features are
     * added/deleted or the layer has been subsetted. If the data provider
     * chooses not to support this feature, the total number of features
     * can be returned.
     * @return long containing number of features
     */
    virtual long featureCount() const;

    /**
     * Update the data source of the layer. The layer's renderer and legend will be preserved only
     * if the geometry type of the new data source matches the current geometry type of the layer.
     * @param dataSource new layer data source
     * @param baseName base name of the layer
     * @param provider provider string
     * @param loadDefaultStyleFlag set to true to reset the layer's style to the default for the
     * data source
     * @note added in QGIS 2.10
     */
    void setDataSource( QString dataSource, QString baseName, QString provider, bool loadDefaultStyleFlag = false );

    /**
     * Number of features rendered with specified symbol. Features must be first
     * calculated by countSymbolFeatures()
     * @param symbol the symbol
     * @return number of features rendered by symbol or -1 if failed or counts are not available
     */
    long featureCount( QgsSymbolV2* symbol );

    /**
     * Count features for symbols. Feature counts may be get by featureCount( QgsSymbolV2*).
     * @param showProgress show progress dialog
     * @return true if calculated, false if failed or was canceled by user
     */
    bool countSymbolFeatures( bool showProgress = true );

    /**
     * Set the string (typically sql) used to define a subset of the layer
     * @param subset The subset string. This may be the where clause of a sql statement
     *               or other defintion string specific to the underlying dataprovider
     *               and data store.
     * @return true, when setting the subset string was successful, false otherwise
     */
    virtual bool setSubsetString( QString subset );

    /**
     * Get the string (typically sql) used to define a subset of the layer
     * @return The subset string or QString::null if not implemented by the provider
     */
    virtual QString subsetString();

    /**
     * Query the provider for features specified in request.
     */
    QgsFeatureIterator getFeatures( const QgsFeatureRequest& request = QgsFeatureRequest() );

    /** Adds a feature
        @param f feature to add
        @param alsoUpdateExtent If True, will also go to the effort of e.g. updating the extents.
        @return                    True in case of success and False in case of error
     */
    bool addFeature( QgsFeature& f, bool alsoUpdateExtent = true );

    /** Updates an existing feature. This method needs to query the datasource
        on every call. Consider using {@link changeAttributeValue()} or
        {@link changeGeometry()} instead.
        @param f  Feature to update
        @return   True in case of success and False in case of error
     */
    bool updateFeature( QgsFeature &f );

    /** Insert a new vertex before the given vertex number,
     *  in the given ring, item (first number is index 0), and feature
     *  Not meaningful for Point geometries
     */
    bool insertVertex( double x, double y, QgsFeatureId atFeatureId, int beforeVertex );

    /** Moves the vertex at the given position number,
     *  ring and item (first number is index 0), and feature
     *  to the given coordinates
     */
    bool moveVertex( double x, double y, QgsFeatureId atFeatureId, int atVertex );

    /** Deletes a vertex from a feature
     */
    bool deleteVertex( QgsFeatureId atFeatureId, int atVertex );

    /** Deletes the selected features
     *  @return true in case of success and false otherwise
     */
    bool deleteSelectedFeatures( int *deletedCount = 0 );

    /** Adds a ring to polygon/multipolygon features
     @return
       0 in case of success,
       1 problem with feature type,
       2 ring not closed,
       3 ring not valid,
       4 ring crosses existing rings,
       5 no feature found where ring can be inserted
       6 layer not editable */
    int addRing( const QList<QgsPoint>& ring );

    /** Adds a new part polygon to a multipart feature
     @return
       0 in case of success,
       1 if selected feature is not multipart,
       2 if ring is not a valid geometry,
       3 if new polygon ring not disjoint with existing rings,
       4 if no feature was selected,
       5 if several features are selected,
       6 if selected geometry not found
       7 layer not editable */
    int addPart( const QList<QgsPoint>& ring );

    /** Translates feature by dx, dy
       @param featureId id of the feature to translate
       @param dx translation of x-coordinate
       @param dy translation of y-coordinate
       @return 0 in case of success*/
    int translateFeature( QgsFeatureId featureId, double dx, double dy );

    /** Splits parts cut by the given line
     *  @param splitLine line that splits the layer features
     *  @param topologicalEditing true if topological editing is enabled
     *  @return
     *   0 in case of success,
     *   4 if there is a selection but no feature split
     */
    int splitParts( const QList<QgsPoint>& splitLine, bool topologicalEditing = false );

    /** Splits features cut by the given line
     *  @param splitLine line that splits the layer features
     *  @param topologicalEditing true if topological editing is enabled
     *  @return
     *   0 in case of success,
     *   4 if there is a selection but no feature split
     */
    int splitFeatures( const QList<QgsPoint>& splitLine, bool topologicalEditing = false );

    /** Changes the specified geometry such that it has no intersections with other
     *  polygon (or multipolygon) geometries in this vector layer
     *  @param geom geometry to modify
     *  @param ignoreFeatures list of feature ids where intersections should be ignored
     *  @return 0 in case of success
     *
     *  @deprecated since 2.2 - not being used for "avoid intersections" functionality anymore
     */
    int removePolygonIntersections( QgsGeometry* geom, QgsFeatureIds ignoreFeatures = QgsFeatureIds() ) /Deprecated/;

    /** Adds topological points for every vertex of the geometry.
     * @param geom the geometry where each vertex is added to segments of other features
     * @note geom is not going to be modified by the function
     * @return 0 in case of success
     */
    int addTopologicalPoints( const QgsGeometry* geom );

    /** Adds a vertex to segments which intersect point p but don't
     * already have a vertex there. If a feature already has a vertex at position p,
     * no additional vertex is inserted. This method is useful for topological
     * editing.
     * @param p position of the vertex
     * @return 0 in case of success
     */
    int addTopologicalPoints( const QgsPoint& p );

    /** Inserts vertices to the snapped segments.
     * This is useful for topological editing if snap to segment is enabled.
     * @param snapResults results collected from the snapping operation
     * @return 0 in case of success
     */
    int insertSegmentVerticesForSnap( const QList<QgsSnappingResult>& snapResults );

    /** Set labels on
     * @deprecated this method is for the old labeling engine
    */
    void enableLabels( bool on ) /Deprecated/;

    /** Label is on
     * @deprecated this method is for the old labeling engine, use labelsEnabled instead
    */
    bool hasLabelsEnabled() const /Deprecated/;

    /** Returns true if the provider is in editing mode */
    virtual bool isEditable() const;

    /** Returns true if the provider is in read-only mode */
    virtual bool isReadOnly() const;

    /** Returns true if the provider has been modified since the last commit */
    virtual bool isModified() const;

    /** Snaps a point to the closest vertex if there is one within the snapping tolerance
     *  @param point       The point which is set to the position of a vertex if there is one within the snapping tolerance.
     *  If there is no point within this tolerance, point is left unchanged.
     *  @param tolerance   The snapping tolerance
     *  @return true if the point has been snapped, false if no vertex within search tolerance
     */
    bool snapPoint( QgsPoint& point, double tolerance );

    /** Snaps to segment or vertex within given tolerance
     * @param startPoint point to snap (in layer coordinates)
     * @param snappingTolerance distance tolerance for snapping
     * @param snappingResults snapping results. Key is the distance between startPoint and snapping target
     * @param snap_to to segment / to vertex
     * @return 0 in case of success
     */
    int snapWithContext( const QgsPoint& startPoint,
                         double snappingTolerance,
                         QMultiMap < double, QgsSnappingResult > &snappingResults /Out/,
                         QgsSnapper::SnappingType snap_to );

    /** Synchronises with changes in the datasource */
    virtual void reload();

    /** Return new instance of QgsMapLayerRenderer that will be used for rendering of given context
     * @note added in 2.4
     */
    virtual QgsMapLayerRenderer* createMapRenderer( QgsRenderContext& rendererContext ) /Factory/;

    /** Draws the layer
     *  @return false if an error occurred during drawing
     */
    bool draw( QgsRenderContext& rendererContext );

    /** Draws the layer labels using the old labeling engine
     * @note deprecated
    */
    void drawLabels( QgsRenderContext& rendererContext ) /Deprecated/;

    /** Return the extent of the layer as a QRect */
    QgsRectangle extent();

    /**
     * Returns the list of fields of this layer.
     * This also includes fields which have not yet been saved to the provider.
     *
     * @return A list of fields
     */
    const QgsFields &pendingFields() const;

    /** Returns list of attributes */
    QList<int> pendingAllAttributesList();

    /** Returns list of attribute making up the primary key */
    QList<int> pendingPkAttributesList();

    /** Returns feature count after commit */
    int pendingFeatureCount();

    /** Make layer read-only (editing disabled) or not
     *  @return false if the layer is in editing yet
     */
    bool setReadOnly( bool readonly = true );

    /** Make layer editable */
    bool startEditing();

    /** Change feature's geometry */
    bool changeGeometry( QgsFeatureId fid, QgsGeometry* geom );

    /**
     * Changes an attribute value (but does not commit it)
     *
     * @deprecated The emitSignal parameter is obsolete and not considered at the moment. It will
     *             be removed in future releases. Remove it to be prepared for the future. (Since 2.1)
     */
    bool changeAttributeValue( QgsFeatureId fid, int field, QVariant value, bool emitSignal ) /Deprecated/;

    /**
     * Changes an attribute value (but does not commit it)
     *
     * @param fid   The feature id of the feature to be changed
     * @param field The index of the field to be updated
     * @param newValue The value which will be assigned to the field
     * @param oldValue The previous value to restore on undo (will otherwise be retrieved)
     *
     * @return true in case of success
     */
    bool changeAttributeValue( QgsFeatureId fid, int field, const QVariant &newValue, const QVariant &oldValue = QVariant() );

    /** Add an attribute field (but does not commit it)
        returns true if the field was added */
    bool addAttribute( const QgsField &field );

    /** Sets an alias (a display name) for attributes to display in dialogs */
    void addAttributeAlias( int attIndex, QString aliasString );

    /** Removes an alias (a display name) for attributes to display in dialogs */
    void remAttributeAlias( int attIndex );

    /**
     * Adds a tab (for the attribute editor form) holding groups and fields
     */
    void addAttributeEditorWidget( QgsAttributeEditorElement* data );

    /**
     * Get the id for the editor widget used to represent the field at the given index
     *
     * @param fieldIdx  The index of the field
     *
     * @return The id for the editor widget or a NULL string if not applicable
     */
    const QString editorWidgetV2( int fieldIdx ) const;

    /**
     * Get the id for the editor widget used to represent the field at the given index
     *
     * @param fieldName  The name of the field
     *
     * @return The id for the editor widget or a NULL string if not applicable
     *
     * @note python method name editorWidgetV2ByName
     */
    const QString editorWidgetV2( const QString& fieldName ) const /PyName=editorWidgetV2ByName/;

    /**
     * Get the configuration for the editor widget used to represent the field at the given index
     *
     * @param fieldIdx  The index of the field
     *
     * @return The configuration for the editor widget or an empty config if the field does not exist
     */
    const QgsEditorWidgetConfig editorWidgetV2Config( int fieldIdx ) const;

    /**
     * Get the configuration for the editor widget used to represent the field at the given index
     *
     * @param fieldName The name of the field
     *
     * @return The configuration for the editor widget or an empty config if the field does not exist
     *
     * @note python method name is editorWidgetV2ConfigByName
     */
    const QgsEditorWidgetConfig editorWidgetV2Config( const QString& fieldName ) const /PyName=editorWidgetV2ConfigByName/;

    /**
     * Returns a list of tabs holding groups and fields
     */
    QList< QgsAttributeEditorElement* > &attributeEditorElements();
    /**
     * Clears all the tabs for the attribute editor form
     */
    void clearAttributeEditorWidgets();

    /** Returns the alias of an attribute name or an empty string if there is no alias */
    QString attributeAlias( int attributeIndex ) const;

    /** Convenience function that returns the attribute alias if defined or the field name else */
    QString attributeDisplayName( int attributeIndex ) const;

    const QMap< QString, QString >& attributeAliases() const;

    const QSet<QString>& excludeAttributesWMS() const;
    void setExcludeAttributesWMS( const QSet<QString>& att );

    const QSet<QString>& excludeAttributesWFS() const;
    void setExcludeAttributesWFS( const QSet<QString>& att );

    /** Delete an attribute field (but does not commit it) */
    bool deleteAttribute( int attr );

    /**
     * Deletes a list of attribute fields (but does not commit it)
     *
     * @param  attrs the indices of the attributes to delete
     * @return true if at least one attribute has been deleted
     *
     */
    bool deleteAttributes( QList<int> attrs );

    /** Insert a copy of the given features into the layer  (but does not commit it) */
    bool addFeatures( QList<QgsFeature> features, bool makeSelected = true );

    /** Delete a feature from the layer (but does not commit it) */
    bool deleteFeature( QgsFeatureId fid );

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

    /** Stop editing and discard the edits
     * @param deleteBuffer whether to delete editing buffer
     */
    bool rollBack( bool deleteBuffer = true );

    /**
     * Get edit type
     *
     * @deprecated Use editorWidgetV2() instead
     */
    EditType editType( int idx ) /Deprecated/;

    /**
     * Get edit type
     *
     * @deprecated Use setEditorWidgetV2() instead
     */
    void setEditType( int idx, EditType edit ) /Deprecated/;

    /** Get the active layout for the attribute editor for this layer */
    EditorLayout editorLayout();

    /** Set the active layout for the attribute editor for this layer */
    void setEditorLayout( EditorLayout editorLayout );

    /**
     * Set the editor widget type for a field
     *
     * QGIS ships the following widget types, additional types may be available depending
     * on plugins.
     *
     * <ul>
     * <li>CheckBox (QgsCheckboxWidgetWrapper)</li>
     * <li>Classification (QgsClassificationWidgetWrapper)</li>
     * <li>Color (QgsColorWidgetWrapper)</li>
     * <li>DateTime (QgsDateTimeEditWrapper)</li>
     * <li>Enumeration (QgsEnumerationWidgetWrapper)</li>
     * <li>FileName (QgsFileNameWidgetWrapper)</li>
     * <li>Hidden (QgsHiddenWidgetWrapper)</li>
     * <li>Photo (QgsPhotoWidgetWrapper)</li>
     * <li>Range (QgsRangeWidgetWrapper)</li>
     * <li>RelationReference (QgsRelationReferenceWidgetWrapper)</li>
     * <li>TextEdit (QgsTextEditWrapper)</li>
     * <li>UniqueValues (QgsUniqueValuesWidgetWrapper)</li>
     * <li>UuidGenerator (QgsUuidWidgetWrapper)</li>
     * <li>ValueMap (QgsValueMapWidgetWrapper)</li>
     * <li>ValueRelation (QgsValueRelationWidgetWrapper)</li>
     * <li>WebView (QgsWebViewWidgetWrapper)</li>
     * </ul>
     *
     * @param attrIdx     Index of the field
     * @param widgetType  Type id of the editor widget to use
     */
    void setEditorWidgetV2( int attrIdx, const QString& widgetType );

    /**
     * Set the editor widget config for a field.
     *
     * Python: Will accept a map.
     *
     * Example:
     * \code{.py}
     *   layer.setEditorWidgetV2Config( 1, { 'Layer': 'otherlayerid_1234', 'Key': 'Keyfield', 'Value': 'ValueField' } )
     * \endcode
     *
     * @param attrIdx     Index of the field
     * @param config      The config to set for this field
     *
     * @see setEditorWidgetV2() for a list of widgets and choose the widget to see the available options.
     */
    void setEditorWidgetV2Config( int attrIdx, const QMap<QString, QVariant>& config );

    /**
     * Set string representing 'true' for a checkbox
     *
     * @deprecated Use setEditorWidgetV2Config() instead
     */
    void setCheckedState( int idx, QString checked, QString notChecked ) /Deprecated/;

    /** Get edit form */
    QString editForm();

    /** Set edit form */
    void setEditForm( QString ui );

    /** Type of feature form pop-up suppression after feature creation (overrides app setting)
     * @note added in 2.1 */
    QgsVectorLayer::FeatureFormSuppress featureFormSuppress() const;

    /** Set type of feature form pop-up suppression after feature creation (overrides app setting)
     * @note added in 2.1 */
    void setFeatureFormSuppress( QgsVectorLayer::FeatureFormSuppress s );

    /** Get annotation form */
    QString annotationForm() const;

    /** Set annotation form for layer */
    void setAnnotationForm( const QString& ui );

    /** Get python function for edit form initialization */
    QString editFormInit();

    /** Set python function for edit form initialization */
    void setEditFormInit( QString function );

    /**
     * Access value map
     * @deprecated Use editorWidgetV2Config() instead
     */
    QMap<QString, QVariant> valueMap( int idx ) /Deprecated/;

    /**
     * Access range widget config data
     *
     * @deprecated Use editorWidgetV2Config() instead
     */
    RangeData range( int idx ) /Deprecated/;

    /** Access value relation widget data */
    ValueRelationData valueRelation( int idx );

    /**
     * Get relations, where the foreign key is on this layer
     *
     * @param idx Only get relations, where idx forms part of the foreign key
     * @return A list of relations
     */
    QList<QgsRelation> referencingRelations( int idx );

    /**
     * Access date format
     *
     * @deprecated Use setEditorWidgetV2Config() instead
     */
    QString dateFormat( int idx ) /Deprecated/;

    /**
     * Access widget size for photo and webview widget
     *
     * @deprecated Use setEditorWidgetV2Config() instead
     */
    QSize widgetSize( int idx ) /Deprecated/;

    /** Is edit widget editable **/
    bool fieldEditable( int idx );

    /** Label widget on top **/
    bool labelOnTop( int idx );

    /** Set edit widget editable **/
    void setFieldEditable( int idx, bool editable );

    /** Label widget on top **/
    void setLabelOnTop( int idx, bool onTop );

    //! Buffer with uncommitted editing operations. Only valid after editing has been turned on.
    QgsVectorLayerEditBuffer* editBuffer();

    /**
     * Create edit command for undo/redo operations
     * @param text text which is to be displayed in undo window
     */
    void beginEditCommand( QString text );

    /** Finish edit command and add it to undo/redo stack */
    void endEditCommand();

    /** Destroy active command and reverts all changes in it */
    void destroyEditCommand();

    /** Returns the index of a field name or -1 if the field does not exist */
    int fieldNameIndex( const QString& fieldName ) const;

    /** Editing vertex markers */
    enum VertexMarkerType
    {
      SemiTransparentCircle,
      Cross,
      NoMarker
    };

    /** Draws a vertex symbol at (screen) coordinates x, y. (Useful to assist vertex editing.) */
    static void drawVertexMarker( double x, double y, QPainter& p, QgsVectorLayer::VertexMarkerType type, int vertexSize );

    /** Assembles mUpdatedFields considering provider fields, joined fields and added fields */
    void updateFields();

    /** Caches joined attributes if required (and not already done) */
    void createJoinCaches();

    /** Returns unique values for column
      @param index column index for attribute
      @param uniqueValues out: result list
      @param limit maximum number of values to return (-1 if unlimited) */
    void uniqueValues( int index, QList<QVariant> &uniqueValues /Out/, int limit = -1 );

    /** Returns minimum value for an attribute column or invalid variant in case of error */
    QVariant minimumValue( int index );

    /** Returns maximum value for an attribute column or invalid variant in case of error */
    QVariant maximumValue( int index );

    /** Fetches all values from a specified field name or expression.
     * @param fieldOrExpression field name or an expression string
     * @param ok will be set to false if field or expression is invalid, otherwise true
     * @param selectedOnly set to true to get values from selected features only
     * @returns list of fetched values
     * @note added in QGIS 2.9
     * @see getDoubleValues
     */
    QList< QVariant > getValues( const QString &fieldOrExpression, bool &ok, bool selectedOnly = false );

    /** Fetches all double values from a specified field name or expression. Null values or
     * invalid expression results are skipped.
     * @param fieldOrExpression field name or an expression string evaluating to a double value
     * @param ok will be set to false if field or expression is invalid, otherwise true
     * @param selectedOnly set to true to get values from selected features only
     * @param nullCount optional pointer to integer to store number of null values encountered in
     * @returns list of fetched values
     * @note added in QGIS 2.9
     * @see getValues
     */
    QList< double > getDoubleValues( const QString &fieldOrExpression, bool &ok, bool selectedOnly = false, int* nullCount = 0 );

    /** Set the blending mode used for rendering each feature */
    void setFeatureBlendMode( const QPainter::CompositionMode &blendMode );
    /** Returns the current blending mode for features */
    QPainter::CompositionMode featureBlendMode() const;

    /** Set the transparency for the vector layer */
    void setLayerTransparency( int layerTransparency );
    /** Returns the current transparency for the vector layer */
    int layerTransparency() const;

    QString metadata();

    /** @note not available in python bindings */
    // inline QgsGeometryCache* cache();

    /** Set the simplification settings for fast rendering of features
     *  @note added in 2.2
     */
    void setSimplifyMethod( const QgsVectorSimplifyMethod& simplifyMethod );
    /** Returns the simplification settings for fast rendering of features
     *  @note added in 2.2
     */
    const QgsVectorSimplifyMethod& simplifyMethod() const;

    /** Returns whether the VectorLayer can apply the specified simplification hint
     *  @note Do not use in 3rd party code - may be removed in future version!
     *  @note added in 2.2
     */
    bool simplifyDrawingCanbeApplied( const QgsRenderContext& renderContext, QgsVectorSimplifyMethod::SimplifyHint simplifyHint ) const;

  public slots:
    /**
     * Select feature by its ID
     *
     * @param featureId  The id of the feature to select
     *
     * @see select(QgsFeatureIds)
     */
    void select( const QgsFeatureId &featureId );

    /**
     * Select features by their ID
     *
     * @param featureIds The ids of the features to select
     *
     * @see select(QgsFeatureId)
     */
    void select( const QgsFeatureIds& featureIds );

    /**
     * Deselect feature by its ID
     *
     * @param featureId  The id of the feature to deselect
     *
     * @see deselect(QgsFeatureIds)
     */
    void deselect( const QgsFeatureId featureId );

    /**
     * Deselect features by their ID
     *
     * @param featureIds The ids of the features to deselect
     *
     * @see deselect(QgsFeatureId)
     */
    void deselect( const QgsFeatureIds& featureIds );

    /**
     * Clear selection
     *
     * @see setSelectedFeatures(const QgsFeatureIds&)
     */
    void removeSelection();

    /** Update the extents for the layer. This is necessary if features are
     *  added/deleted or the layer has been subsetted.
     */
    virtual void updateExtents();

    /** Check if there is a join with a layer that will be removed */
    void checkJoinLayerRemove( QString theLayerId );

  protected slots:
    void invalidateSymbolCountedFlag();

  signals:

    /**
     * This signal is emitted when selection was changed
     *
     * @param selected        Newly selected feature ids
     * @param deselected      Ids of all features which have previously been selected but are not any more
     * @param clearAndSelect  In case this is set to true, the old selection was dismissed and the new selection corresponds to selected
     */
    void selectionChanged( const QgsFeatureIds selected, const QgsFeatureIds deselected, const bool clearAndSelect );

    /** This signal is emitted when selection was changed */
    void selectionChanged();

    /** This signal is emitted when modifications has been done on layer */
    void layerModified();

    /** Is emitted, when layer is checked for modifications. Use for last-minute additions */
    void beforeModifiedCheck() const;

    /** Is emitted, when editing on this layer has started*/
    void editingStarted();

    /** Is emitted, when edited changes successfully have been written to the data provider */
    void editingStopped();

    /** Is emitted, before changes are commited to the data provider */
    void beforeCommitChanges();

    /** Is emitted, before changes are rolled back*/
    void beforeRollBack();

    /**
     * Will be emitted, when a new attribute has been added to this vector layer.
     * Applies only to types {@link QgsFields::OriginEdit} and {@link QgsFields::OriginProvider}
     *
     * @param idx The index of the new attribute
     *
     * @see updatedFields()
     */
    void attributeAdded( int idx );
    /**
     * Will be emitted, when an attribute has been deleted from this vector layer.
     * Applies only to types {@link QgsFields::OriginEdit} and {@link QgsFields::OriginProvider}
     *
     * @param idx The index of the deleted attribute
     *
     * @see updatedFields()
     */
    void attributeDeleted( int idx );
    void featureAdded( QgsFeatureId fid );
    void featureDeleted( QgsFeatureId fid );
    /**
     * Emitted when features have been deleted.
     *
     * If features are deleted within an edit command, this will only be emitted once at the end
     * to allow connected slots to minimize the overhead.
     * If features are delted outside of an edit command, this signal will be emitted once per feature.
     *
     * @param fids The feature ids that have been deleted.
     */
    void featuresDeleted( QgsFeatureIds fids );
    /**
     * Is emitted, whenever the fields available from this layer have been changed.
     * This can be due to manually adding attributes or due to a join.
     */
    void updatedFields();
    void layerDeleted();

    void attributeValueChanged( QgsFeatureId fid, int idx, const QVariant & );
    void geometryChanged( QgsFeatureId fid, QgsGeometry &geom );

    /** Signals emitted after committing changes */
    void committedAttributesDeleted( const QString& layerId, const QgsAttributeList& deletedAttributes );
    void committedAttributesAdded( const QString& layerId, const QList<QgsField>& addedAttributes );
    void committedFeaturesAdded( const QString& layerId, const QgsFeatureList& addedFeatures );
    void committedFeaturesRemoved( const QString& layerId, const QgsFeatureIds& deletedFeatureIds );
    void committedAttributeValuesChanges( const QString& layerId, const QgsChangedAttributesMap& changedAttributesValues );
    void committedGeometriesChanges( const QString& layerId, const QgsGeometryMap& changedGeometries );

    void saveLayerToProject();

    /** Emitted when the font family defined for labeling layer is not found on system */
    void labelingFontNotFound( QgsVectorLayer* layer, const QString& fontfamily );

    /** Signal emitted when setFeatureBlendMode() is called */
    void featureBlendModeChanged( const QPainter::CompositionMode &blendMode );

    /** Signal emitted when setLayerTransparency() is called */
    void layerTransparencyChanged( int layerTransparency );

    /**
     * Signal emitted when a new edit command has been started
     *
     * @param text Description for this edit command
     */
    void editCommandStarted( const QString& text );

    /**
     * Signal emitted, when an edit command successfully ended
     * @note This does not mean it is also committed, only that it is written
     * to the edit buffer. See {@link beforeCommitChanges()}
     */
    void editCommandEnded();

    /**
     * Signal emitted, whan an edit command is destroyed
     * @note This is not a rollback, it is only related to the current edit command.
     * See {@link beforeRollBack()}
     */
    void editCommandDestroyed();

    /**
     * Signal emitted whenever the symbology (QML-file) for this layer is being read.
     * If there is custom style information saved in the file, you can connect to this signal
     * and update the layer style accordingly.
     *
     * @param element The XML layer style element.
     *
     * @param errorMessage Write error messages into this string.
     */
    void readCustomSymbology( const QDomElement& element, QString& errorMessage );

    /**
     * Signal emitted whenever the symbology (QML-file) for this layer is being written.
     * If there is custom style information you want to save to the file, you can connect
     * to this signal and update the element accordingly.
     *
     * @param element  The XML element where you can add additional style information to.
     * @param doc      The XML document that you can use to create new XML nodes.
     * @param errorMessage Write error messages into this string.
     */
    void writeCustomSymbology( QDomElement& element, QDomDocument& doc, QString& errorMessage ) const;

  private slots:
    void onRelationsLoaded();
    void onJoinedFieldsChanged();

  protected:
    /** Set the extent */
    void setExtent( const QgsRectangle &rect );

  private:                       // Private methods

    /** Vector layers are not copyable */
    QgsVectorLayer( const QgsVectorLayer & rhs );


};

