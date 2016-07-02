
typedef QList<int> QgsAttributeList;
typedef QSet<int> QgsAttributeIds;


struct QgsVectorJoinInfo
{
%TypeHeaderCode
#include "qgsvectorlayer.h"
%End
  QgsVectorJoinInfo();

  /** Join field in the target layer*/
  QString targetFieldName;
  /** Source layer*/
  QString joinLayerId;
  /** Join field in the source layer*/
  QString joinFieldName;
  /** True if the join is cached in virtual memory*/
  bool memoryCache;
  /** True if the cached join attributes need to be updated*/
  bool cacheDirty;
  /** Cache for joined attributes to provide fast lookup (size is 0 if no memory caching)
   * @note not available in python bindings
   */
  // QHash< QString, QgsAttributeMap> cachedAttributes;

  /** An optional prefix. If it is a Null string "{layername}_" will be used
   * @note Added in 2.8
   */
  QString prefix;

  bool operator==( const QgsVectorJoinInfo& other ) const;

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
    struct GroupData
    {
      GroupData();
      GroupData( const QString& name, const QList<QString>& fields );
      QString mName;
      QList<QString> mFields;
    };

    struct TabData
    {
      TabData();
      TabData( const QString& name, const QList<QString>& fields, const QList<QgsVectorLayer::GroupData>& groups );
      QString mName;
      QList<QString> mFields;
      QList<QgsVectorLayer::GroupData> mGroups;
    };

    enum EditorLayout
    {
      GeneratedLayout,
      TabLayout,
      UiFileLayout
    };

    struct RangeData
    {
      //! @deprecated Use the editorWidgetV2() system instead
      RangeData() /Deprecated/;
      //! @deprecated Use the editorWidgetV2() system instead
      RangeData( const QVariant& theMin, const QVariant& theMax, const QVariant& theStep ) /Deprecated/;

      QVariant mMin;
      QVariant mMax;
      QVariant mStep;
    };

    struct ValueRelationData
    {
      ValueRelationData();
      ValueRelationData( const QString& layer, const QString& key, const QString& value, bool allowNull, bool orderByValue,
                         bool allowMulti = false,
                         const QString& filterExpression = QString::null );

      QString mLayer;
      QString mKey;
      QString mValue;
      QString mFilterExpression;
      bool mAllowNull;
      bool mOrderByValue;
      bool mAllowMulti;  /* allow selection of multiple keys */
    };

    /**
     * Types of feature form suppression after feature creation
     * @note added in 2.1
     * @deprecated in 2.14, Use QgsEditFormConfig instead
     */
    enum FeatureFormSuppress
    {
      SuppressDefault, // use the application-wide setting
      SuppressOn,
      SuppressOff,
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

    //! Result of an edit operation
    enum EditResult
    {
      Success, /**< Edit operation was successful */
      EmptyGeometry, /**< Edit operation resulted in an empty geometry */
      EditFailed, /**< Edit operation failed */
      FetchFeatureFailed, /**< Unable to fetch requested feature */
      InvalidLayer, /**< Edit failed due to invalid layer */
    };

    //! Selection behaviour
    enum SelectBehaviour
    {
      SetSelection, /**< Set selection, removing any existing selection */
      AddToSelection, /**< Add selection to current selection */
      IntersectSelection, /**< Modify current selection to include only select features which match */
      RemoveFromSelection, /**<  Remove from current selection */
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
    QgsVectorLayer( const QString& path = QString::null, const QString& baseName = QString::null,
                    const QString& providerLib = QString::null, bool loadDefaultStyleFlag = true );

    /** Destructor */
    virtual ~QgsVectorLayer();

    /** Returns the permanent storage type for this layer as a friendly name. */
    QString storageType() const;

    /** Capabilities for this layer in a friendly format. */
    QString capabilitiesString() const;

    /** Returns a comment for the data in the layer */
    QString dataComment() const;

    /** Set the primary display field to be used in the identify results dialog */
    void setDisplayField( const QString& fldName = "" );

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
     * @note not available in python bindings
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

    /** Removes a vector layer join
      @returns true if join was found and successfully removed */
    bool removeJoin( const QString& joinLayerId );

    const QList<QgsVectorJoinInfo> vectorJoins() const;

    /**
     * Get the list of layer ids on which this layer depends. This in particular determines the order of layer loading.
     */
    virtual QSet<QString> layerDependencies() const;

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

    /** Get the label rendering properties associated with this layer */
    QgsLabel *label();

    /** Get the label rendering properties associated with this layer
     * @note not available in python bindings
     */
    // const QgsLabel *label() const;

    QgsActionManager *actions();

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
     * @see selectByExpression()
     * @deprecated use selectByRect() instead
     */
    void select( QgsRectangle & rect, bool addToSelection ) /Deprecated/;

    /**
     * Select features found within the search rectangle (in layer's coordinates)
     * @param rect search rectangle
     * @param behaviour selection type, allows adding to current selection, removing
     * from selection, etc.
     * @see invertSelectionInRectangle(QgsRectangle & rect)
     * @see selectByExpression()
     * @see selectByIds()
     */
    void selectByRect( QgsRectangle & rect, SelectBehaviour behaviour = SetSelection );

    /** Select matching features using an expression.
     * @param expression expression to evaluate to select features
     * @param behaviour selection type, allows adding to current selection, removing
     * from selection, etc.
     * @note added in QGIS 2.16
     * @see selectByRect()
     * @see selectByIds()
     */
    void selectByExpression( const QString& expression, SelectBehaviour behaviour = SetSelection );

    /** Select matching features using a list of feature IDs. Will emit the
     * selectionChanged() signal with the clearAndSelect flag set.
     * @param ids feature IDs to select
     * @param behaviour selection type, allows adding to current selection, removing
     * from selection, etc.
     * @note added in QGIS 2.16
     * @see selectByRect()
     * @see selectByExpression()
     */
    void selectByIds( const QgsFeatureIds &ids, SelectBehaviour behaviour = SetSelection );

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
     * @see selectByExpression()
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
     * Will emit the { @link selectionChanged( const QgsFeatureIds&, const QgsFeatureIds&qt_check_for_QOBJECT_macro, bool ) } signal with the
     * clearAndSelect flag set.
     *
     * @param ids   The ids which will be the new selection
     * @deprecated use selectByIds() instead
     */
    void setSelectedFeatures( const QgsFeatureIds &ids ) /Deprecated/;

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

    /** Return const renderer V2.
     * @note not available in python bindings
     */
    // const QgsFeatureRendererV2* rendererV2() const { return mRendererV2; }

    /**
     * Set renderer which will be invoked to represent this layer.
     * Ownership is transferred.
     */
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
     * @note Called by QgsMapLayer::readXML().
     */
    virtual bool readXml( const QDomNode& layer_node );

    /** Write vector layer specific state to project file Dom node.
     * @note Called by QgsMapLayer::writeXML().
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
    virtual void saveStyleToDatabase( const QString& name, const QString& description,
                                      bool useAsDefault, const QString& uiFileContent,
                                      QString &msgError );

    /**
     * Lists all the style in db split into related to the layer and not related to
     * @param ids the list in which will be stored the style db ids
     * @param names the list in which will be stored the style names
     * @param descriptions the list in which will be stored the style descriptions
     * @param msgError
     * @return the number of styles related to current layer
     */
    virtual int listStylesInDatabase( QStringList &ids /Out/, QStringList &names /Out/,
                                      QStringList &descriptions /Out/, QString &msgError /Out/ );

    /**
     * Will return the named style corresponding to style id provided
     */
    virtual QString getStyleFromDatabase( const QString& styleId, QString &msgError );

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
    virtual bool applyNamedStyle( const QString& namedStyle, QString &errorMsg /Out/ ) /Deprecated/;

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

    /** Read the style for the current layer from the Dom node supplied.
     * @param node node that will contain the style definition for this layer.
     * @param errorMessage reference to string that will be updated with any error messages
     * @return true in case of success.
     */
    bool readStyle( const QDomNode& node, QString& errorMessage );

    /** Write the symbology for the layer into the docment provided.
     *  @param node the node that will have the style element added to it.
     *  @param doc the document that will have the QDomNode added.
     *  @param errorMessage reference to string that will be updated with any error messages
     *  @return true in case of success.
     */
    bool writeSymbology( QDomNode& node, QDomDocument& doc, QString& errorMessage ) const;

    /** Write just the style information for the layer into the document
     *  @param node the node that will have the style element added to it.
     *  @param doc the document that will have the QDomNode added.
     *  @param errorMessage reference to string that will be updated with any error messages
     *  @return true in case of success.
     */
    bool writeStyle( QDomNode& node, QDomDocument& doc, QString& errorMessage ) const;

    bool writeSld( QDomNode& node, QDomDocument& doc, QString& errorMessage ) const;
    bool readSld( const QDomNode& node, QString& errorMessage );

    /**
     * Number of features rendered with specified symbol. Features must be first
     * calculated by countSymbolFeatures()
     * @param symbol the symbol
     * @return number of features rendered by symbol or -1 if failed or counts are not available
     */
    long featureCount( QgsSymbolV2* symbol );

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
    void setDataSource( const QString& dataSource, const QString& baseName, const QString& provider, bool loadDefaultStyleFlag = false );

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
    virtual bool setSubsetString( const QString& subset );

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

    /** Moves the vertex at the given position number,
     * ring and item (first number is index 0), and feature
     * to the given coordinates
     * @note available in python as moveVertexV2
     */
    bool moveVertex( const QgsPointV2& p, QgsFeatureId atFeatureId, int atVertex ) /PyName=moveVertexV2/;

    /** Deletes a vertex from a feature
     * @deprecated use deleteVertexV2() instead
     */
    bool deleteVertex( QgsFeatureId atFeatureId, int atVertex ) /Deprecated/;

    /** Deletes a vertex from a feature.
     * @param featureId ID of feature to remove vertex from
     * @param vertex index of vertex to delete
     * @note added in QGIS 2.14
     */
    //TODO QGIS 3.0 - rename back to deleteVertex
    EditResult deleteVertexV2( QgsFeatureId featureId, int vertex );

    /** Deletes the selected features
     *  @return true in case of success and false otherwise
     */
    bool deleteSelectedFeatures( int *deletedCount = 0 );

    /** Adds a ring to polygon/multipolygon features
     * @param ring ring to add
     * @param featureId if specified, feature ID for feature ring was added to will be stored in this parameter
     * @return
     *  0 in case of success,
     *  1 problem with feature type,
     *  2 ring not closed,
     *  3 ring not valid,
     *  4 ring crosses existing rings,
     *  5 no feature found where ring can be inserted
     *  6 layer not editable
     */
    int addRing( const QList<QgsPoint>& ring, QgsFeatureId* featureId = 0 );

    /** Adds a ring to polygon/multipolygon features (takes ownership)
     * @param ring ring to add
     * @param featureId if specified, feature ID for feature ring was added to will be stored in this parameter
     * @return
     *  0 in case of success
     *  1 problem with feature type
     *  2 ring not closed
     *  6 layer not editable
     * @note available in python as addCurvedRing
     */
    int addRing( QgsCurveV2* ring /Transfer/, QgsFeatureId* featureId = 0 ) /PyName=addCurvedRing/;

    /** Adds a new part polygon to a multipart feature
     * @return
     *   0 in case of success,
     *   1 if selected feature is not multipart,
     *   2 if ring is not a valid geometry,
     *   3 if new polygon ring not disjoint with existing rings,
     *   4 if no feature was selected,
     *   5 if several features are selected,
     *   6 if selected geometry not found
     *   7 layer not editable
     */
    int addPart( const QList<QgsPoint>& ring );

    /** Adds a new part polygon to a multipart feature
     * @return
     *   0 in case of success,
     *   1 if selected feature is not multipart,
     *   2 if ring is not a valid geometry,
     *   3 if new polygon ring not disjoint with existing rings,
     *   4 if no feature was selected,
     *   5 if several features are selected,
     *   6 if selected geometry not found
     *   7 layer not editable
     * @note available in python bindings as addPartV2
     */
    int addPart( const QList<QgsPointV2>& ring ) /PyName=addPartV2/;

    //! @note available in python as addCurvedPart
    int addPart( QgsCurveV2* ring /Transfer/ ) /PyName=addCurvedPart/;

    /** Translates feature by dx, dy
     *  @param featureId id of the feature to translate
     *  @param dx translation of x-coordinate
     *  @param dy translation of y-coordinate
     *  @return 0 in case of success
     */
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
    int removePolygonIntersections( QgsGeometry* geom, const QgsFeatureIds& ignoreFeatures = QgsFeatureIds() ) /Deprecated/;

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

    /** Access to labeling configuration.
     * @note added in 2.12
     * @note not available in Python bindings
     */
    // const QgsAbstractVectorLayerLabeling* labeling() const { return mLabeling; }

    /** Set labeling configuration. Takes ownership of the object.
     * @note added in 2.12
     * @note not available in Python bindings
     */
    // void setLabeling( QgsAbstractVectorLayerLabeling* labeling );

    /** Returns true if the provider is in editing mode */
    virtual bool isEditable() const;

    virtual bool isSpatial() const;

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
     * @deprecated will be removed in QGIS 3.0
     */
    void drawLabels( QgsRenderContext& rendererContext ) /Deprecated/;

    /** Return the extent of the layer */
    QgsRectangle extent();

    /**
     * Returns the list of fields of this layer.
     * This also includes fields which have not yet been saved to the provider.
     *
     * @return A list of fields
     */
    QgsFields fields() const;

    /**
     * Returns the list of fields of this layer.
     * This also includes fields which have not yet been saved to the provider.
     * Alias for {@link fields()}
     *
     * @return A list of fields
     */
    QgsFields pendingFields() const;

    /**
     * Returns list of attribute indexes. i.e. a list from 0 ... fieldCount()
     * Alias for {@link attributeList()}
     */
    QgsAttributeList pendingAllAttributesList() const;

    /**
     * Returns list of attribute indexes. i.e. a list from 0 ... fieldCount()
     * Alias for {@link attributeList()}
     */
    QgsAttributeList attributeList() const;

    /**
     * Returns list of attributes making up the primary key
     * Alias for {@link pkAttributeList()}
     */
    QgsAttributeList pendingPkAttributesList() const;

    /** Returns list of attributes making up the primary key */
    QgsAttributeList pkAttributeList() const;

    /**
     * Returns feature count including changes which have not yet been committed
     * Alias for {@link featureCount()}
     */
    long pendingFeatureCount() const;

    /**
     * Returns feature count including changes which have not yet been committed
     * If you need only the count of committed features call this method on this layer's provider.
     */
    long featureCount() const;

    /** Make layer read-only (editing disabled) or not
     *  @return false if the layer is in editing yet
     */
    bool setReadOnly( bool readonly = true );

    /** Change feature's geometry */
    bool changeGeometry( QgsFeatureId fid, QgsGeometry* geom );

    /**
     * Changes an attribute value (but does not commit it)
     *
     * @deprecated The emitSignal parameter is obsolete and not considered at the moment. It will
     *             be removed in future releases. Remove it to be prepared for the future. (Since 2.1)
     */
    bool changeAttributeValue( QgsFeatureId fid, int field, const QVariant& value, bool emitSignal ) /Deprecated/;

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
     * returns true if the field was added
     */
    bool addAttribute( const QgsField &field );

    /** Sets an alias (a display name) for attributes to display in dialogs */
    void addAttributeAlias( int attIndex, const QString& aliasString );

    /** Removes an alias (a display name) for attributes to display in dialogs */
    void remAttributeAlias( int attIndex );

    /** Renames an attribute field  (but does not commit it).
     * @param attIndex attribute index
     * @param newName new name of field
     * @note added in QGIS 2.16
    */
    bool renameAttribute( int attIndex, const QString& newName );

    /**
     * Adds a tab (for the attribute editor form) holding groups and fields
     *
     * @deprecated Use `editFormConfig()->addTab()` instead
     */
    void addAttributeEditorWidget( QgsAttributeEditorElement* data ) /Deprecated/;

    /**
     * Get the id for the editor widget used to represent the field at the given index
     *
     * @param fieldIdx  The index of the field
     *
     * @return The id for the editor widget or a NULL string if not applicable
     *
     * @deprecated Use `editFormConfig()->widgetType()` instead
     */
    const QString editorWidgetV2( int fieldIdx ) const /Deprecated/;

    /**
     * Get the id for the editor widget used to represent the field at the given index
     *
     * @param fieldName  The name of the field
     *
     * @return The id for the editor widget or a NULL string if not applicable
     *
     * @note python method name editorWidgetV2ByName
     *
     * @deprecated Use `editFormConfig()->widgetType()` instead
     */
    const QString editorWidgetV2( const QString& fieldName ) const /PyName=editorWidgetV2ByName, Deprecated/;

    /**
     * Get the configuration for the editor widget used to represent the field at the given index
     *
     * @param fieldIdx  The index of the field
     *
     * @return The configuration for the editor widget or an empty config if the field does not exist
     *
     * @deprecated Use `editFormConfig()->widgetConfig()` instead
     */
    const QgsEditorWidgetConfig editorWidgetV2Config( int fieldIdx ) const /Deprecated/;

    /**
     * Get the configuration for the editor widget used to represent the field with the given name
     *
     * @param fieldName The name of the field
     *
     * @return The configuration for the editor widget or an empty config if the field does not exist
     *
     * @note python method name is editorWidgetV2ConfigByName
     *
     * @deprecated Use `editFormConfig()->widgetConfig()` instead
     */
    const QgsEditorWidgetConfig editorWidgetV2Config( const QString& fieldName ) const /PyName=editorWidgetV2ConfigByName, Deprecated/;

    /**
     * Returns a list of tabs holding groups and fields
     *
     * @deprecated Use `editFormConfig()->tabs()` instead
     */
    QList< QgsAttributeEditorElement* > attributeEditorElements() /Deprecated/;

    /**
     * Get the configuration of the form used to represent this vector layer.
     * This is a writable configuration that can directly be changed in place.
     *
     * @return The configuration of this layers' form
     */
    QgsEditFormConfig* editFormConfig() const;

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
     * Deletes a set of features from the layer (but does not commit it)
     * @param fids The feature ids to delete
     *
     * @return false if the layer is not in edit mode or does not support deleting
     *         in case of an active transaction depends on the provider implementation
     */
    bool deleteFeatures( const QgsFeatureIds& fids );

    /**
     * Attempts to commit any changes to disk.  Returns the result of the attempt.
     * If a commit fails, the in-memory changes are left alone.
     *
     * This allows editing to continue if the commit failed on e.g. a
     * disallowed value in a Postgres database - the user can re-edit and try
     * again.
     *
     * The commits occur in distinct stages,
     * (add attributes, add features, change attribute values, change
     * geometries, delete features, delete attributes)
     * so if a stage fails, it's difficult to roll back cleanly.
     * Therefore any error message also includes which stage failed so
     * that the user has some chance of repairing the damage cleanly.
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
     * @deprecated Use `editFormConfig()->widgetType()` instead
     */
    EditType editType( int idx ) /Deprecated/;

    /**
     * Set edit type
     *
     * @deprecated Use `editFormConfig()->setWidgetType()` instead
     */
    void setEditType( int idx, EditType edit ) /Deprecated/;

    /**
     * Get the active layout for the attribute editor for this layer
     * @deprecated Use `editFormConfig()->layout()` instead
     */
    EditorLayout editorLayout() /Deprecated/;

    /**
     * Set the active layout for the attribute editor for this layer
     * @deprecated Use `editFormConfig()->setLayout()` instead
     */
    void setEditorLayout( EditorLayout editorLayout ) /Deprecated/;

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
     *
     * @deprecated Use `editFormConfig()->setWidgetType()` instead
     */
    void setEditorWidgetV2( int attrIdx, const QString& widgetType ) /Deprecated/;

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
     *
     * @deprecated Use `editFormConfig()->setWidgetConfig()` instead
     */
    void setEditorWidgetV2Config( int attrIdx, const QMap<QString, QVariant>& config ) /Deprecated/;

    /**
     * Set string representing 'true' for a checkbox
     *
     * @deprecated Use @deprecated Use `editFormConfig()->setWidgetConfig()` instead
     */
    void setCheckedState( int idx, const QString& checked, const QString& notChecked ) /Deprecated/;

    /**
     * Get edit form
     *
     * @deprecated Use `editFormConfig()->uiForm()` instead
     */
    QString editForm() /Deprecated/;

    /**
     * Set edit form
     * @deprecated Use `editFormConfig()->setUiForm()` instead
     */
    void setEditForm( const QString& ui ) /Deprecated/;

    /** Type of feature form pop-up suppression after feature creation (overrides app setting)
     * @note added in 2.1
     * @deprecated Use `editFormConfig()->suppress()` instead
     */
    QgsVectorLayer::FeatureFormSuppress featureFormSuppress() const /Deprecated/;

    /** Set type of feature form pop-up suppression after feature creation (overrides app setting)
     * @note added in 2.1
     * @deprecated Use `editFormConfig()->setSuppress()` instead
     */
    void setFeatureFormSuppress( QgsVectorLayer::FeatureFormSuppress s ) /Deprecated/;

    /** Get annotation form */
    QString annotationForm() const;

    /** Set annotation form for layer */
    void setAnnotationForm( const QString& ui );

    /**
     * Get python function for edit form initialization
     *
     * @deprecated Use `editFormConfig()->initFunction()` instead
     */
    QString editFormInit() /Deprecated/;

    /**
     * Set python function for edit form initialization
     *
     * @deprecated Use `editFormConfig()->setInitFunction()` instead
     */
    void setEditFormInit( const QString& function ) /Deprecated/;

    /**
     * Access value map
     * @deprecated Use `editFormConfig()->widgetConfig()` instead
     */
    QMap<QString, QVariant> valueMap( int idx ) /Deprecated/;

    /**
     * Access range widget config data
     *
     * @deprecated Use `editFormConfig()->widgetConfig()` instead
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
     * @deprecated Use `editFormConfig()->widgetConfig()` instead
     */
    QString dateFormat( int idx ) /Deprecated/;

    /**
     * Access widget size for photo and webview widget
     *
     * @deprecated Use `editFormConfig()->widgetConfig()` instead
     */
    QSize widgetSize( int idx ) /Deprecated/;

    /**
     * Is edit widget editable
     *
     * @deprecated Use `editFormConfig()->fieldEditable()` instead
     */
    bool fieldEditable( int idx );

    /**
     * Label widget on top
     * @deprecated Use `editFormConfig()->labelOnTop()` instead
     */
    bool labelOnTop( int idx );

    /**
     * Set edit widget editable
     * @deprecated Use `editFormConfig()->setFieldEditable()` instead
     */
    void setFieldEditable( int idx, bool editable );

    /**
     * Label widget on top
     * @deprecated Use `editFormConfig()->setLabelOnTop()` instead
     */
    void setLabelOnTop( int idx, bool onTop );

    //! Buffer with uncommitted editing operations. Only valid after editing has been turned on.
    QgsVectorLayerEditBuffer* editBuffer();

    /**
     * Create edit command for undo/redo operations
     * @param text text which is to be displayed in undo window
     */
    void beginEditCommand( const QString& text );

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
     * @param index column index for attribute
     * @param uniqueValues out: result list
     * @param limit maximum number of values to return (-1 if unlimited)
     */
    void uniqueValues( int index, QList<QVariant> &uniqueValues /Out/, int limit = -1 );

    /** Returns minimum value for an attribute column or invalid variant in case of error */
    QVariant minimumValue( int index );

    /** Returns maximum value for an attribute column or invalid variant in case of error */
    QVariant maximumValue( int index );

    /** Calculates an aggregated value from the layer's features.
     * @param aggregate aggregate to calculate
     * @param fieldOrExpression source field or expression to use as basis for aggregated values.
     * @param parameters parameters controlling aggregate calculation
     * @param context expression context for expressions and filters
     * @param ok if specified, will be set to true if aggregate calculation was successful
     * @return calculated aggregate value
     * @note added in QGIS 2.16
     */
    QVariant aggregate( QgsAggregateCalculator::Aggregate aggregate,
                        const QString& fieldOrExpression,
                        const QgsAggregateCalculator::AggregateParameters& parameters = QgsAggregateCalculator::AggregateParameters(),
                        QgsExpressionContext* context = nullptr,
                        bool* ok = nullptr );

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
    void setFeatureBlendMode( QPainter::CompositionMode blendMode );
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

    /**
     * @brief Return the conditional styles that are set for this layer. Style information is
     * used to render conditional formatting in the attribute table.
     * @return Return a QgsConditionalLayerStyles object holding the conditional attribute
     * style information. Style information is generic and can be used for anything.
     * @note added in QGIS 2.12
     */
    QgsConditionalLayerStyles *conditionalStyles() const;

    /**
     * Get the attribute table configuration object.
     * This defines the appearance of the attribute table.
     */
    QgsAttributeTableConfig attributeTableConfig() const;

    /**
     * Set the attribute table configuration object.
     * This defines the appearance of the attribute table.
     */
    void setAttributeTableConfig( const QgsAttributeTableConfig& attributeTableConfig );

  public slots:
    /**
     * Select feature by its ID
     *
     * @param featureId  The id of the feature to select
     *
     * @see select(QgsFeatureIds)
     */
    void select( QgsFeatureId featureId );

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
    void checkJoinLayerRemove( const QString& theLayerId );

    /**
     * Make layer editable.
     * This starts an edit session on this layer. Changes made in this edit session will not
     * be made persistent until {@link commitChanges()} is called and can be reverted by calling
     * {@link rollBack()}.
     */
    bool startEditing();


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
    void selectionChanged( const QgsFeatureIds& selected, const QgsFeatureIds& deselected, const bool clearAndSelect );

    /** This signal is emitted when selection was changed */
    void selectionChanged();

    /** This signal is emitted when modifications has been done on layer */
    void layerModified();

    /** Is emitted, when layer is checked for modifications. Use for last-minute additions */
    void beforeModifiedCheck() const;

    /** Is emitted, before editing on this layer is started */
    void beforeEditingStarted();

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
     * Applies only to types {@link QgsFields::OriginEdit}, {@link QgsFields::OriginProvider} and {@link QgsFields::OriginExpression }
     *
     * @param idx The index of the new attribute
     *
     * @see updatedFields()
     */
    void attributeAdded( int idx );

    /**
     * Will be emitted, when an expression field is going to be added to this vector layer.
     * Applies only to types {@link QgsFields::OriginExpression }
     *
     * @param fieldName The name of the attribute to be added
     */
    void beforeAddingExpressionField( const QString& fieldName );

    /**
     * Will be emitted, when an attribute has been deleted from this vector layer.
     * Applies only to types {@link QgsFields::OriginEdit}, {@link QgsFields::OriginProvider} and {@link QgsFields::OriginExpression }
     *
     * @param idx The index of the deleted attribute
     *
     * @see updatedFields()
     */
    void attributeDeleted( int idx );

    /**
     * Will be emitted, when an expression field is going to be deleted from this vector layer.
     * Applies only to types {@link QgsFields::OriginExpression }
     *
     * @param idx The index of the attribute to be deleted
     */
    void beforeRemovingExpressionField( int idx );

    /**
     * Emitted when a new feature has been added to the layer
     *
     * @param fid The id of the new feature
     */
    void featureAdded( QgsFeatureId fid );

    /**
     * Emitted when a feature has been deleted.
     *
     * If you do expensive operations in a slot connected to this, you should prever to use
     * {@link featuresDeleted( const QgsFeatureIds& )}.
     *
     * @param fid The id of the feature which has been deleted
     */
    void featureDeleted( QgsFeatureId fid );

    /**
     * Emitted when features have been deleted.
     *
     * If features are deleted within an edit command, this will only be emitted once at the end
     * to allow connected slots to minimize the overhead.
     * If features are deleted outside of an edit command, this signal will be emitted once per feature.
     *
     * @param fids The feature ids that have been deleted.
     */
    void featuresDeleted( const QgsFeatureIds& fids );

    /**
     * Is emitted, whenever the fields available from this layer have been changed.
     * This can be due to manually adding attributes or due to a join.
     */
    void updatedFields();

    /**
     * @deprecated use QObject::destroyed instead
     */
    void layerDeleted() /Deprecated/;

    /**
     * Is emitted whenever an attribute value change is done in the edit buffer.
     * Note that at this point the attribute change is not yet saved to the provider.
     *
     * @param fid The id of the changed feature
     * @param idx The attribute index of the changed attribute
     * @param value The new value of the attribute
     */
    void attributeValueChanged( QgsFeatureId fid, int idx, const QVariant& value );

    /**
     * Is emitted whenever a geometry change is done in the edit buffer.
     * Note that at this point the geometry change is not yet saved to the provider.
     *
     * @param fid The id of the changed feature
     * @param geometry The new geometry
     */
    void geometryChanged( QgsFeatureId fid, QgsGeometry& geometry );

    /** This signal is emitted, when attributes are deleted from the provider */
    void committedAttributesDeleted( const QString& layerId, const QgsAttributeList& deletedAttributes );
    /** This signal is emitted, when attributes are added to the provider */
    void committedAttributesAdded( const QString& layerId, const QList<QgsField>& addedAttributes );
    /** This signal is emitted, when features are added to the provider */
    void committedFeaturesAdded( const QString& layerId, const QgsFeatureList& addedFeatures );
    /** This signal is emitted, when features are deleted from the provider */
    void committedFeaturesRemoved( const QString& layerId, const QgsFeatureIds& deletedFeatureIds );
    /** This signal is emitted, when attribute value changes are saved to the provider */
    void committedAttributeValuesChanges( const QString& layerId, const QgsChangedAttributesMap& changedAttributesValues );
    /** This signal is emitted, when geometry changes are saved to the provider */
    void committedGeometriesChanges( const QString& layerId, const QgsGeometryMap& changedGeometries );

    /** Deprecated: This signal has never been emitted */
    void saveLayerToProject() /Deprecated/;

    /** Emitted when the font family defined for labeling layer is not found on system */
    void labelingFontNotFound( QgsVectorLayer* layer, const QString& fontfamily );

    /** Signal emitted when setFeatureBlendMode() is called */
    void featureBlendModeChanged( QPainter::CompositionMode blendMode );

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

    /**
     * Signals an error related to this vector layer.
     */
    void raiseError( const QString& msg );

  protected:
    /** Set the extent */
    void setExtent( const QgsRectangle &rect );

  private:                       // Private methods

    /** Vector layers are not copyable */
    QgsVectorLayer( const QgsVectorLayer & rhs );


};

