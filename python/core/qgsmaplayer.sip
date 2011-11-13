
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
    sipClass = sipClass_QgsMapLayer;
    QgsMapLayer* layer = qobject_cast<QgsMapLayer*>(sipCpp);
    if (layer->type() == QgsMapLayer::VectorLayer)
    {
      sipClass = sipClass_QgsVectorLayer;
    }
    else if (layer->type() == QgsMapLayer::RasterLayer)
    {
      sipClass = sipClass_QgsRasterLayer;
    }
    else if (layer->type() == QgsMapLayer::PluginLayer)
    {
      sipClass = sipClass_QgsPluginLayer;
    }
  }
  else
  {
    sipClass = 0;
  }

%End

public:
    /** Layers enum defining the types of layers that can be added to a map */
    enum LayerType
    {
        VectorLayer,
        RasterLayer,
        PluginLayer
    };

    /** Constructor
     * @param type Type of layer as defined in QgsMapLayer::LayerType enum
     * @param lyrname Display Name of the layer
     */
    QgsMapLayer(QgsMapLayer::LayerType type = QgsMapLayer::VectorLayer, QString lyrname = QString::null, QString source = QString::null);

    /** Destructor */
    virtual ~QgsMapLayer();

    /** Get the type of the layer
     * @return Integer matching a value in the QgsMapLayer::LayerType enum
     */
    QgsMapLayer::LayerType type() const;

    /** Get this layer's unique ID, this ID is used to access this layer from map layer registry
     * @note added in 1.7
     */
    QString id() const;

    /** Get this layer's unique ID, this ID is used to access this layer from map layer registry
     * @deprecated in 1.7
     */
    QString getLayerID() const /Deprecated/;

    /** Set the display name of the layer
     * @param name New name for the layer
     */
    void setLayerName(const QString & name);

    /** Get the display name of the layer
     * @return the layer name
     */
    const QString & name() const;

    /**Synchronises with changes in the datasource
      @note added in version 1.6*/
    virtual void reload();

    /** Render the layer, to be overridden in child classes
     */
    virtual bool draw(QgsRenderContext& rendererContext);

    /** Draw labels
     * @TODO to be removed: used only in vector layers
     */
    virtual void drawLabels(QgsRenderContext& rendererContext);

    /** Return the extent of the layer as a QRect */
    const QgsRectangle extent();

    /*! Return the status of the layer. An invalid layer is one which has a bad datasource
     * or other problem. Child classes set this flag when intialized
     * @return True if the layer is valid and can be accessed
     */
    bool isValid();

    /*! Gets a version of the internal layer definition that has sensitive 
      *  bits removed (for example, the password). This function should 
      * be used when displaying the source name for general viewing. 
     */ 
    QString publicSource() const;

    /** Returns the source for the layer */
    const QString & source() const;

    /**
     * Returns the sublayers of this layer
     * (Useful for providers that manage their own layers, such as WMS)
     */
    virtual QStringList subLayers();
    
    /**
     * Reorders the *previously selected* sublayers of this layer from bottom to top
     * (Useful for providers that manage their own layers, such as WMS)
     */
    virtual void setLayerOrder(QStringList layers);
    
    /** Set the visibility of the given sublayer name */
    virtual void setSubLayerVisibility(QString name, bool vis);


    /** True if the layer can be edited */
    virtual bool isEditable() const;

    /** sets state from Dom document
       @param layer_node is Dom node corresponding to ``maplayer'' tag
       @note

       The Dom node corresponds to a Dom document project file XML element read
       by QgsProject.

       This, in turn, calls readXml(), which is over-rideable by sub-classes so
       that they can read their own specific state from the given Dom node.

       Invoked by QgsProject::read().

       @returns true if successful
     */
    bool readXML(const QDomNode& layer_node);


    /** stores state in Dom node
       @param layer_node is Dom node corresponding to ``projectlayers'' tag
       @note

       The Dom node corresponds to a Dom document project file XML element to be
       written by QgsProject.

       This, in turn, calls writeXml(), which is over-rideable by sub-classes so
       that they can write their own specific state to the given Dom node.

       Invoked by QgsProject::write().

       @returns true if successful
    */
    bool writeXML(QDomNode & layer_node, QDomDocument & document) const;

    /** Set a custom property for layer. Properties are stored in a map and saved in project file.
     *  @note Added in v1.4 */
    void setCustomProperty( const QString& key, const QVariant& value );
    /** Read a custom property from layer. Properties are stored in a map and saved in project file.
     *  @note Added in v1.4 */
    QVariant customProperty( const QString& value, const QVariant& defaultValue = QVariant() ) const;
    /** Remove a custom property from layer. Properties are stored in a map and saved in project file.
     *  @note Added in v1.4 */
    void removeCustomProperty( const QString& key );

    /** Read the symbology for the current layer from the Dom node supplied. 
     * @param QDomNode node that will contain the symbology definition for this layer.
     * @param errorMessage reference to string that will be updated with any error messages
     * @return true in case of success.
    */
    virtual bool readSymbology(const QDomNode& node, QString& errorMessage) = 0;

    /** Write the symbology for the layer into the docment provided.
     *  @param QDomNode the node that will have the style element added to it.
     *  @param QDomDocument the document that will have the QDomNode added.
     * @param errorMessage reference to string that will be updated with any error messages
     *  @return true in case of success.
     */
    virtual bool writeSymbology(QDomNode&, QDomDocument& doc, QString& errorMessage) const = 0;


    /** Copies the symbology settings from another layer. Returns true in case of success */
    virtual bool copySymbologySettings(const QgsMapLayer& other) = 0;

    /** Returns true if this layer can be in the same symbology group with another layer */
    virtual bool hasCompatibleSymbology(const QgsMapLayer& other) const = 0;

    /** Accessor for transparency level. */
    unsigned int getTransparency();

    /** Mutator for transparency level. Should be between 0 and 255 */
    void setTransparency(unsigned int);
    
    /**
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     */
    virtual QString lastErrorTitle();
  
    /**
     * If an operation returns 0 (e.g. draw()), this function
     * returns the text of the error associated with the failure.
     * Interactive users of this provider can then, for example,
     * call a QMessageBox to display the contents.
     */
    virtual QString lastError();

    /** Returns layer's spatial reference system
    @note This was introduced in QGIS 1.4
    */
    
    const QgsCoordinateReferenceSystem& crs();

    /** Returns layer's spatial reference system
    @note This method is here for API compatibility
    and will be deprecated in 2.0
    @see crs()
    */
    const QgsCoordinateReferenceSystem& srs() /Deprecated/;
    
    /** Sets layer's spatial reference system
    @note emitSignal added in 1.4 */
    void setCrs( const QgsCoordinateReferenceSystem& srs, bool emitSignal = true );

    /** A convenience function to capitalise the layer name */
    static QString capitaliseLayerName(const QString name);

	/** Retrieve the default style for this layer if one
     * exists (either as a .qml file on disk or as a
     * record in the users style table in their personal qgis.db)
     * @param a reference to a flag that will be set to false if
     * we did not manage to load the default style.
     * @return a QString with any status messages
     * @see also loadNamedStyle ();
     */
    virtual QString loadDefaultStyle( bool & theResultFlag /Out/ );

    /** Retrieve a named style for this layer if one
     * exists (either as a .qml file on disk or as a
     * record in the users style table in their personal qgis.db)
     * @param QString theURI - the file name or other URI for the
     * style file. First an attempt will be made to see if this
     * is a file and load that, if that fails the qgis.db styles
     * table will be consulted to see if there is a style who's
     * key matches the URI.
     * @param a reference to a flag that will be set to false if
     * we did not manage to load the default style.
     * @return a QString with any status messages
     * @see also loadDefaultStyle ();
     */
    virtual QString loadNamedStyle( const QString theURI, bool & theResultFlag /Out/ );

    virtual bool loadNamedStyleFromDb( const QString db, const QString theURI, QString &qml /Out/ );

    /** Save the properties of this layer as the default style
     * (either as a .qml file on disk or as a
     * record in the users style table in their personal qgis.db)
     * @param a reference to a flag that will be set to false if
     * we did not manage to save the default style.
     * @return a QString with any status messages
     * @see also loadNamedStyle () and saveNamedStyle()
     */
    virtual QString saveDefaultStyle( bool & theResultFlag /Out/ );

    /** Save the properties of this layer as a named style
     * (either as a .qml file on disk or as a
     * record in the users style table in their personal qgis.db)
     * @param QString theURI - the file name or other URI for the
     * style file. First an attempt will be made to see if this
     * is a file and save to that, if that fails the qgis.db styles
     * table will be used to create a style entry who's
     * key matches the URI.
     * @param a reference to a flag that will be set to false if
     * we did not manage to save the default style.
     * @return a QString with any status messages
     * @see also saveDefaultStyle ();
     */
    virtual QString saveNamedStyle( const QString theURI, bool & theResultFlag /Out/ );
    
    /** Return pointer to layer's undo stack */
    QUndoStack* undoStack();

    /** Get the QImage used for caching render operations
     * @note This method was added in QGIS 1.4 **/
    QImage *cacheImage();
    /** Set the QImage used for caching render operations 
     * @note This method was added in QGIS 1.4 **/
    void setCacheImage( QImage * thepImage /Transfer/ ); 

public slots:

    /** Event handler for when a coordinate transform fails due to bad vertex error */
    virtual void invalidTransformInput();

    /** Accessor and mutator for the minimum scale member */
    void setMinimumScale(float theMinScale);
    float minimumScale();

    /** Accessor and mutator for the maximum scale member */
    void setMaximumScale(float theMaxScale);
    float maximumScale();

    /** Accessor and mutator for the scale based visilibility flag */
    void toggleScaleBasedVisibility( bool theVisibilityFlag);
    bool hasScaleBasedVisibility();

signals:

    /** Emit a signal to notify of a progress event */
    void setProgress(int theProgress, int theTotalSteps);

    /** Emit a signal with status (e.g. to be caught by QgiAapp and display a msg on status bar) */
    void setStatus(QString theStatusQString);

    /** Emit a signal that the layer name has been changed */
    void layerNameChanged();

    /** Emit a signal that layer's CRS has been reset
     added in 1.4
     */
    void layerCrsChanged();

    /** This signal should be connected with the slot QgsMapCanvas::refresh()
     * @TODO: to be removed - GUI dependency
     */
    void repaintRequested();

    /** This is used to send a request that any mapcanvas using this layer update its extents */
    void recalculateExtents();

protected:

    /** set whether layer is valid or not - should be used in constructor */
    void setValid( bool valid );

    /** called by readXML(), used by children to read state specific to them from
        project files.
    */
    virtual bool readXml( const QDomNode& layer_node );

    /** called by writeXML(), used by children to write state specific to them to
        project files.
    */
    virtual bool writeXml( QDomNode & layer_node, QDomDocument & document );

    /** debugging member - invoked when a connect() is made to this object */
    void connectNotify( const char * signal );

private:

    /** private copy constructor - QgsMapLayer not copyable */
    QgsMapLayer( const QgsMapLayer & );


};

