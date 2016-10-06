
/** \class QgsMapLayer
 *  \brief Base class for all map layer types.
 * This class is the base class for all map layer types (vector, raster).
 */
class QgsMapLayer : QObject
{
%TypeHeaderCode
#include <qgsmaplayer.h>
%End

%ConvertToSubClassCode
  if (sipCpp->inherits("QgsMapLayer"))
  {
    sipType = sipType_QgsMapLayer;
    QgsMapLayer* layer = qobject_cast<QgsMapLayer*>(sipCpp);
    if (layer->type() == QgsMapLayer::VectorLayer)
    {
      sipType = sipType_QgsVectorLayer;
    }
    else if (layer->type() == QgsMapLayer::RasterLayer)
    {
      sipType = sipType_QgsRasterLayer;
    }
    else if (layer->type() == QgsMapLayer::PluginLayer)
    {
      sipType = sipType_QgsPluginLayer;
    }
  }
  else
  {
    sipType = 0;
  }

%End

  public:

    //! Types of layers that can be added to a map
    enum LayerType
    {
      VectorLayer,
      RasterLayer,
      PluginLayer
    };

    /** Constructor for QgsMapLayer
     * @param type layer type
     * @param name display name for the layer
     * @param source datasource of layer
     */
    QgsMapLayer( QgsMapLayer::LayerType type = VectorLayer, const QString& name = QString::null, const QString& source = QString::null );

    virtual ~QgsMapLayer();

    /** Returns the type of the layer.
     */
    QgsMapLayer::LayerType type() const;

    /** Returns the layer's unique ID, which is used to access this layer from QgsMapLayerRegistry. */
    QString id() const;

    /**
     * Set the display name of the layer
     * @param name new name for the layer
     * @note added in 2.16
     * @see name()
     */
    void setName( const QString& name );

    /** Returns the display name of the layer.
     * @return the layer name
     * @see setName()
     */
    QString name() const;

    /** Returns the original name of the layer.
     * @return the original layer name
     */
    QString originalName() const;

    /** Sets the short name of the layer
     *  used by QGIS Server to identify the layer.
     * @return the layer short name
     * @see shortName()
     */
    void setShortName( const QString& shortName );

    /** Returns the short name of the layer
     *  used by QGIS Server to identify the layer.
     * @return the layer short name
     * @see setShortName()
     */
    QString shortName() const;

    /** Sets the title of the layer
     *  used by QGIS Server in GetCapabilities request.
     * @return the layer title
     * @see title()
     */
    void setTitle( const QString& title );

    /** Returns the title of the layer
     *  used by QGIS Server in GetCapabilities request.
     * @return the layer title
     * @see setTitle()
     */
    QString title() const;

    /** Sets the abstract of the layer
     *  used by QGIS Server in GetCapabilities request.
     * @return the layer abstract
     * @see abstract()
     */
    void setAbstract( const QString& abstract );

    /** Returns the abstract of the layer
     *  used by QGIS Server in GetCapabilities request.
     * @return the layer abstract
     * @see setAbstract()
     */
    QString abstract() const;

    /** Sets the keyword list of the layer
     *  used by QGIS Server in GetCapabilities request.
     * @return the layer keyword list
     * @see keywordList()
     */
    void setKeywordList( const QString& keywords );

    /** Returns the keyword list of the layer
     *  used by QGIS Server in GetCapabilities request.
     * @return the layer keyword list
     * @see setKeywordList()
     */
    QString keywordList() const;

    /* Layer dataUrl information */

    /** Sets the DataUrl of the layer
     *  used by QGIS Server in GetCapabilities request.
     *  DataUrl is a a link to the underlying data represented by a particular layer.
     * @return the layer DataUrl
     * @see dataUrl()
     */
    void setDataUrl( const QString& dataUrl );

    /** Returns the DataUrl of the layer
     *  used by QGIS Server in GetCapabilities request.
     *  DataUrl is a a link to the underlying data represented by a particular layer.
     * @return the layer DataUrl
     * @see setDataUrl()
     */
    QString dataUrl() const;

    /** Sets the DataUrl format of the layer
     *  used by QGIS Server in GetCapabilities request.
     *  DataUrl is a a link to the underlying data represented by a particular layer.
     * @return the layer DataUrl format
     * @see dataUrlFormat()
     */
    void setDataUrlFormat( const QString& dataUrlFormat );

    /** Returns the DataUrl format of the layer
     *  used by QGIS Server in GetCapabilities request.
     *  DataUrl is a a link to the underlying data represented by a particular layer.
     * @return the layer DataUrl format
     * @see setDataUrlFormat()
     */
    QString dataUrlFormat() const;

    /* Layer attribution information */

    /** Sets the attribution of the layer
     *  used by QGIS Server in GetCapabilities request.
     *  Attribution indicates the provider of a layer or collection of layers.
     * @return the layer attribution
     * @see attribution()
     */
    void setAttribution( const QString& attrib );

    /** Returns the attribution of the layer
     *  used by QGIS Server in GetCapabilities request.
     *  Attribution indicates the provider of a layer or collection of layers.
     * @return the layer attribution
     * @see setAttribution()
     */
    QString attribution() const;

    /** Sets the attribution URL of the layer
     *  used by QGIS Server in GetCapabilities request.
     *  Attribution indicates the provider of a layer or collection of layers.
     * @return the layer attribution URL
     * @see attributionUrl()
     */
    void setAttributionUrl( const QString& attribUrl );

    /** Returns the attribution URL of the layer
     *  used by QGIS Server in GetCapabilities request.
     *  Attribution indicates the provider of a layer or collection of layers.
     * @return the layer attribution URL
     * @see setAttributionUrl()
     */
    QString attributionUrl() const;

    /* Layer metadataUrl information */

    /** Sets the metadata URL of the layer
     *  used by QGIS Server in GetCapabilities request.
     *  MetadataUrl is a a link to the detailed, standardized metadata about the data.
     * @return the layer metadata URL
     * @see metadataUrl()
     */
    void setMetadataUrl( const QString& metaUrl );

    /** Returns the metadata URL of the layer
     *  used by QGIS Server in GetCapabilities request.
     *  MetadataUrl is a a link to the detailed, standardized metadata about the data.
     * @return the layer metadata URL
     * @see setMetadataUrl()
     */
    QString metadataUrl() const;

    /** Set the metadata type of the layer
     *  used by QGIS Server in GetCapabilities request
     *  MetadataUrlType indicates the standard to which the metadata complies.
     * @return the layer metadata type
     * @see metadataUrlType()
     */
    void setMetadataUrlType( const QString& metaUrlType );

    /** Returns the metadata type of the layer
     *  used by QGIS Server in GetCapabilities request.
     *  MetadataUrlType indicates the standard to which the metadata complies.
     * @return the layer metadata type
     * @see setMetadataUrlType()
     */
    QString metadataUrlType() const;

    /** Sets the metadata format of the layer
     *  used by QGIS Server in GetCapabilities request.
     *  MetadataUrlType indicates how the metadata is structured.
     * @return the layer metadata format
     * @see metadataUrlFormat()
     */
    void setMetadataUrlFormat( const QString& metaUrlFormat );

    /** Returns the metadata format of the layer
     *  used by QGIS Server in GetCapabilities request.
     *  MetadataUrlType indicates how the metadata is structured.
     * @return the layer metadata format
     * @see setMetadataUrlFormat()
     */
    QString metadataUrlFormat() const;

    /** Set the blending mode used for rendering a layer.
     * @param blendMode new blending mode
     * @see blendMode()
    */
    void setBlendMode( QPainter::CompositionMode blendMode );

    /** Returns the current blending mode for a layer.
     * @see setBlendMode()
    */
    QPainter::CompositionMode blendMode() const;

    /** Returns if this layer is read only. */
    bool readOnly() const;

    /** Synchronises with changes in the datasource
        */
    virtual void reload();

    /** Return new instance of QgsMapLayerRenderer that will be used for rendering of given context
     * @note added in 2.4
     */
    virtual QgsMapLayerRenderer* createMapRenderer( QgsRenderContext& rendererContext ) = 0 /Factory/;

    /** Returns the extent of the layer. */
    virtual QgsRectangle extent() const;

    /** Return the status of the layer. An invalid layer is one which has a bad datasource
     * or other problem. Child classes set this flag when initialized.
     * @return true if the layer is valid and can be accessed
     */
    bool isValid() const;

    /** Gets a version of the internal layer definition that has sensitive
      *  bits removed (for example, the password). This function should
      * be used when displaying the source name for general viewing.
      * @see source()
     */
    QString publicSource() const;

    /** Returns the source for the layer. This source may contain usernames, passwords
     * and other sensitive information.
     * @see publicSource()
     */
    QString source() const;

    /**
     * Returns the sublayers of this layer
     * (Useful for providers that manage their own layers, such as WMS)
     */
    virtual QStringList subLayers() const;

    /**
     * Reorders the *previously selected* sublayers of this layer from bottom to top.
     * (Useful for providers that manage their own layers, such as WMS).
     */
    virtual void setLayerOrder( const QStringList &layers );

    /** Set the visibility of the given sublayer name.
     * @param name sublayer name
     * @param visible sublayer visibility
    */
    virtual void setSubLayerVisibility( const QString& name, bool visible );

    /** Returns true if the layer can be edited. */
    virtual bool isEditable() const;

    /** Returns true if the layer is considered a spatial layer, ie it has some form of geometry associated with it.
     * @note added in QGIS 2.16
     */
    virtual bool isSpatial() const;

    /** Sets state from Dom document
       @param layerElement The Dom element corresponding to ``maplayer'' tag
       @note

       The Dom node corresponds to a Dom document project file XML element read
       by QgsProject.

       This, in turn, calls readXml(), which is over-rideable by sub-classes so
       that they can read their own specific state from the given Dom node.

       Invoked by QgsProject::read().

       @returns true if successful
     */
    bool readLayerXml( const QDomElement& layerElement );

    /** Stores state in Dom node
     * @param layerElement is a Dom element corresponding to ``maplayer'' tag
     * @param document is a the dom document being written
     * @param relativeBasePath base path for relative paths
     * @note
     *
     * The Dom node corresponds to a Dom document project file XML element to be
     * written by QgsProject.
     *
     * This, in turn, calls writeXml(), which is over-rideable by sub-classes so
     * that they can write their own specific state to the given Dom node.
     *
     * Invoked by QgsProject::write().
     *
     * @returns true if successful
     */
    bool writeLayerXml( QDomElement& layerElement, QDomDocument& document, const QString& relativeBasePath = QString::null ) const;

    /** Returns the given layer as a layer definition document
     *  Layer definitions store the data source as well as styling and custom properties.
     *
     *  Layer definitions can be used to load a layer and styling all from a single file.
     */
    static QDomDocument asLayerDefinition( const QList<QgsMapLayer*>& layers, const QString& relativeBasePath = QString::null );

    /** Creates a new layer from a layer defininition document
     */
    static QList<QgsMapLayer*> fromLayerDefinition( QDomDocument& document, bool addToRegistry = false, bool addToLegend = false );
    static QList<QgsMapLayer*> fromLayerDefinitionFile( const QString &qlrfile );

    /** Set a custom property for layer. Properties are stored in a map and saved in project file.
     * @see customProperty()
     * @see removeCustomProperty()
     */
    void setCustomProperty( const QString& key, const QVariant& value );
    /** Read a custom property from layer. Properties are stored in a map and saved in project file.
     * @see setCustomProperty()
    */
    QVariant customProperty( const QString& value, const QVariant& defaultValue = QVariant() ) const;
    /** Remove a custom property from layer. Properties are stored in a map and saved in project file.
     * @see setCustomProperty()
     */
    void removeCustomProperty( const QString& key );

    /** Get current status error. This error describes some principal problem
     *  for which layer cannot work and thus is not valid. It is not last error
     *  after accessing data by draw() etc.
     */
    virtual QgsError error() const;

    /** Returns the layer's spatial reference system.
    @note This was introduced in QGIS 1.4
     */
    QgsCoordinateReferenceSystem crs() const;

    /** Sets layer's spatial reference system */
    void setCrs( const QgsCoordinateReferenceSystem& srs, bool emitSignal = true );

    /** A convenience function to (un)capitalise the layer name */
    static QString capitaliseLayerName( const QString& name );

    /** Retrieve the style URI for this layer
     * (either as a .qml file on disk or as a
     * record in the users style table in their personal qgis.db)
     * @return a QString with the style file name
     * @see also loadNamedStyle () and saveNamedStyle ();
     */
    virtual QString styleURI() const;

    /** Retrieve the default style for this layer if one
     * exists (either as a .qml file on disk or as a
     * record in the users style table in their personal qgis.db)
     * @param resultFlag a reference to a flag that will be set to false if
     * we did not manage to load the default style.
     * @return a QString with any status messages
     * @see also loadNamedStyle ();
     */
    virtual QString loadDefaultStyle( bool & resultFlag /Out/ );

    /** Retrieve a named style for this layer if one
     * exists (either as a .qml file on disk or as a
     * record in the users style table in their personal qgis.db)
     * @param uri - the file name or other URI for the
     * style file. First an attempt will be made to see if this
     * is a file and load that, if that fails the qgis.db styles
     * table will be consulted to see if there is a style who's
     * key matches the URI.
     * @param resultFlag a reference to a flag that will be set to false if
     * we did not manage to load the default style.
     * @return a QString with any status messages
     * @see also loadDefaultStyle ();
     */
    virtual QString loadNamedStyle( const QString& uri, bool &resultFlag /Out/ );

    /** Retrieve a named style for this layer from a sqlite database.
     * @param db path to sqlite database
     * @param uri uri for table
     * @param qml will be set to QML style content from database
     * @returns true if style was successfully loaded
     */
    virtual bool loadNamedStyleFromDb( const QString &db, const QString &uri, QString &qml /Out/ );

    /**
     * Import the properties of this layer from a QDomDocument
     * @param doc source QDomDocument
     * @param errorMsg this QString will be initialized on error
     * during the execution of readSymbology
     * @return true on success
     * @note added in 2.8
     */
    virtual bool importNamedStyle( QDomDocument& doc, QString &errorMsg /Out/ );

    /**
     * Export the properties of this layer as named style in a QDomDocument
     * @param doc the target QDomDocument
     * @param errorMsg this QString will be initialized on error
     * during the execution of writeSymbology
     */
    virtual void exportNamedStyle( QDomDocument &doc, QString &errorMsg ) const;

    /**
     * Export the properties of this layer as SLD style in a QDomDocument
     * @param doc the target QDomDocument
     * @param errorMsg this QString will be initialized on error
     * during the execution of writeSymbology
     */
    virtual void exportSldStyle( QDomDocument &doc, QString &errorMsg ) const;

    /** Save the properties of this layer as the default style
     * (either as a .qml file on disk or as a
     * record in the users style table in their personal qgis.db)
     * @param resultFlag a reference to a flag that will be set to false if
     * we did not manage to save the default style.
     * @return a QString with any status messages
     * @sa loadNamedStyle() and @see saveNamedStyle()
     */
    virtual QString saveDefaultStyle( bool & resultFlag /Out/ );

    /** Save the properties of this layer as a named style
     * (either as a .qml file on disk or as a
     * record in the users style table in their personal qgis.db)
     * @param uri the file name or other URI for the
     * style file. First an attempt will be made to see if this
     * is a file and save to that, if that fails the qgis.db styles
     * table will be used to create a style entry who's
     * key matches the URI.
     * @param resultFlag a reference to a flag that will be set to false if
     * we did not manage to save the default style.
     * @return a QString with any status messages
     * @sa saveDefaultStyle()
     */
    virtual QString saveNamedStyle( const QString &uri, bool &resultFlag /Out/ );

    /** Saves the properties of this layer to an SLD format file.
     * @param uri uri of destination for exported SLD file.
     * @param resultFlag a reference to a flag that will be set to false if
     * the SLD file could not be generated
     * @returns a string with any status or error messages
     * @see loadSldStyle()
     */
    virtual QString saveSldStyle( const QString &uri, bool &resultFlag ) const;

    /** Attempts to style the layer using the formatting from an SLD type file.
     * @param uri uri of source SLD file
     * @param resultFlag a reference to a flag that will be set to false if
     * the SLD file could not be loaded
     * @returns a string with any status or error messages
     * @see saveSldStyle()
     */
    virtual QString loadSldStyle( const QString &uri, bool &resultFlag );

    virtual bool readSld( const QDomNode &node, QString &errorMessage );

    /** Read the symbology for the current layer from the Dom node supplied.
     * @param node node that will contain the symbology definition for this layer.
     * @param errorMessage reference to string that will be updated with any error messages
     * @return true in case of success.
     */
    virtual bool readSymbology( const QDomNode& node, QString& errorMessage ) = 0;

    /** Read the style for the current layer from the Dom node supplied.
     * @param node node that will contain the style definition for this layer.
     * @param errorMessage reference to string that will be updated with any error messages
     * @return true in case of success.
     * @note added in 2.16
     * @note To be implemented in subclasses. Default implementation does nothing and returns false.
     */
    virtual bool readStyle( const QDomNode& node, QString& errorMessage );

    /** Write the symbology for the layer into the docment provided.
     *  @param node the node that will have the style element added to it.
     *  @param doc the document that will have the QDomNode added.
     *  @param errorMessage reference to string that will be updated with any error messages
     *  @return true in case of success.
     */
    virtual bool writeSymbology( QDomNode &node, QDomDocument& doc, QString& errorMessage ) const = 0;

    /** Write just the style information for the layer into the document
     *  @param node the node that will have the style element added to it.
     *  @param doc the document that will have the QDomNode added.
     *  @param errorMessage reference to string that will be updated with any error messages
     *  @return true in case of success.
     *  @note added in 2.16
     *  @note To be implemented in subclasses. Default implementation does nothing and returns false.
     */
    virtual bool writeStyle( QDomNode& node, QDomDocument& doc, QString& errorMessage ) const;

    /** Return pointer to layer's undo stack */
    QUndoStack *undoStack();

    /** Return pointer to layer's style undo stack
     *  @note added in 2.16
     */
    QUndoStack *undoStackStyles();

    /* Layer legendUrl information */
    void setLegendUrl( const QString& legendUrl );
    QString legendUrl() const;
    void setLegendUrlFormat( const QString& legendUrlFormat );
    QString legendUrlFormat() const;

    /**
     * Assign a legend controller to the map layer. The object will be responsible for providing legend items.
     * @param legend Takes ownership of the object. Can be null pointer
     * @note added in 2.6
     */
    void setLegend( QgsMapLayerLegend* legend /Transfer/ );
    /**
     * Can be null.
     * @note added in 2.6
     */
    QgsMapLayerLegend* legend() const;

    /**
     * Get access to the layer's style manager. Style manager allows switching between multiple styles.
     * @note added in 2.8
     */
    QgsMapLayerStyleManager* styleManager() const;

    /** Tests whether the layer should be visible at the specified scale.
     * @param scale scale denominator to test
     * @returns true if the layer is visible at the given scale.
     * @note added in QGIS 2.16
     * @see minimumScale()
     * @see maximumScale()
     * @see hasScaleBasedVisibility()
     */
    bool isInScaleRange( double scale ) const;

    /** Returns the minimum scale denominator at which the layer is visible.
     * Scale based visibility is only used if hasScaleBasedVisibility is true.
     * @returns minimum scale denominator at which the layer will render
     * @see setMinimumScale()
     * @see maximumScale()
     * @see hasScaleBasedVisibility()
     * @see isInScaleRange()
     */
    double minimumScale() const;

    /** Returns the maximum scale denominator at which the layer is visible.
     * Scale based visibility is only used if hasScaleBasedVisibility is true.
     * @returns minimum scale denominator at which the layer will render
     * @see setMaximumScale()
     * @see minimumScale()
     * @see hasScaleBasedVisibility()
     * @see isInScaleRange()
     */
    double maximumScale() const;

    /** Returns whether scale based visibility is enabled for the layer.
     * @returns true if scale based visibility is enabled
     * @see minimumScale()
     * @see maximumScale()
     * @see setScaleBasedVisibility()
     * @see isInScaleRange()
     */
    bool hasScaleBasedVisibility() const;

  public slots:

    /** Event handler for when a coordinate transform fails due to bad vertex error */
    virtual void invalidTransformInput();

    /** Sets the minimum scale denominator at which the layer will be visible.
     * Scale based visibility is only used if setScaleBasedVisibility is set to true.
     * @param scale minimum scale denominator at which the layer should render
     * @see minimumScale
     * @see setMaximumScale
     * @see setScaleBasedVisibility
     */
    void setMinimumScale( double scale );

    /** Sets the maximum scale denominator at which the layer will be visible.
     * Scale based visibility is only used if setScaleBasedVisibility is set to true.
     * @param scale maximum scale denominator at which the layer should render
     * @see maximumScale
     * @see setMinimumScale
     * @see setScaleBasedVisibility
     */
    void setMaximumScale( double scale );

    /** Sets whether scale based visibility is enabled for the layer.
     * @param enabled set to true to enable scale based visibility
     * @see setMinimumScale
     * @see setMaximumScale
     * @see scaleBasedVisibility
     */
    void setScaleBasedVisibility( const bool enabled );

    /**
     * Will advice the map canvas (and any other interested party) that this layer requires to be repainted.
     * Will emit a repaintRequested() signal.
     *
     * @note in 2.6 function moved from vector/raster subclasses to QgsMapLayer
     */
    void triggerRepaint();

    /** \brief Obtain Metadata for this layer */
    virtual QString metadata() const;

    /** Time stamp of data source in the moment when data/metadata were loaded by provider */
    virtual QDateTime timestamp() const;

    /** Triggers an emission of the styleChanged() signal.
     * @note added in QGIS 2.16
     */
    void emitStyleChanged();

    /**
     * Sets the list of dependencies.
     * @see dependencies()
     *
     * @param layers set of QgsMapLayerDependency. Only user-defined dependencies will be added
     * @returns false if a dependency cycle has been detected
     * @note added in QGIS 3.0
     */
    virtual bool setDependencies( const QSet<QgsMapLayerDependency>& layers );

    /**
     * Gets the list of dependencies. This includes data dependencies set by the user (@see setDataDependencies)
     * as well as dependencies given by the provider
     *
     * @returns a set of QgsMapLayerDependency
     * @note added in QGIS 3.0
     */
    virtual QSet<QgsMapLayerDependency> dependencies() const;

  signals:

    /** Emit a signal with status (e.g. to be caught by QgisApp and display a msg on status bar) */
    void statusChanged( const QString& status );

    /**
     * Emitted when the name has been changed
     *
     * @note added in 2.16
     */
    void nameChanged();

    /** Emit a signal that layer's CRS has been reset */
    void crsChanged();

    /** By emitting this signal the layer tells that either appearance or content have been changed
     * and any view showing the rendered layer should refresh itself.
     */
    void repaintRequested();

    /** This is used to send a request that any mapcanvas using this layer update its extents */
    void recalculateExtents() const;

    /** Data of layer changed */
    void dataChanged();

    /** Signal emitted when the blend mode is changed, through QgsMapLayer::setBlendMode() */
    void blendModeChanged( QPainter::CompositionMode blendMode );

    /** Signal emitted when renderer is changed.
     * @see styleChanged()
    */
    void rendererChanged();

    /** Signal emitted whenever a change affects the layer's style. Ie this may be triggered
     * by renderer changes, label style changes, or other style changes such as blend
     * mode or layer opacity changes.
     * @note added in QGIS 2.16
     * @see rendererChanged()
    */
    void styleChanged();

    /**
     * Signal emitted when legend of the layer has changed
     * @note added in 2.6
     */
    void legendChanged();

    /**
     * Emitted whenever the configuration is changed. The project listens to this signal
     * to be marked as dirty.
     */
    void configChanged();

    /**
     * Emitted when dependencies are changed.
     */
    void dependenciesChanged();

  protected:
    /** Set the extent */
    virtual void setExtent( const QgsRectangle &rect );

    /** Set whether layer is valid or not - should be used in constructor. */
    void setValid( bool valid );

    /** Called by readLayerXML(), used by children to read state specific to them from
     *  project files.
     */
    virtual bool readXml( const QDomNode& layer_node );

    /** Called by writeLayerXML(), used by children to write state specific to them to
     *  project files.
     */
    virtual bool writeXml( QDomNode & layer_node, QDomDocument & document ) const;


    /** Read custom properties from project file.
      @param layerNode note to read from
      @param keyStartsWith reads only properties starting with the specified string (or all if the string is empty)*/
    void readCustomProperties( const QDomNode& layerNode, const QString& keyStartsWith = "" );

    /** Write custom properties to project file. */
    void writeCustomProperties( QDomNode & layerNode, QDomDocument & doc ) const;

    /** Read style manager's configuration (if any). To be called by subclasses. */
    void readStyleManager( const QDomNode& layerNode );
    /** Write style manager's configuration (if exists). To be called by subclasses. */
    void writeStyleManager( QDomNode& layerNode, QDomDocument& doc ) const;


    /** Add error message */
    void appendError( const QgsErrorMessage &error );
    /** Set error message */
    void setError( const QgsError &error );

    //! Checks if new change dependency candidates introduce a cycle
    bool hasDependencyCycle( const QSet<QgsMapLayerDependency>& layersIds ) const;
};
