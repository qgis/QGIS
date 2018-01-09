/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaplayer.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsMapLayer : QObject
{
%Docstring
Base class for all map layer types.
This is the base class for all map layer types (vector, raster).
%End

%TypeHeaderCode
#include "qgsmaplayer.h"
%End
%ConvertToSubClassCode
    QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( sipCpp );

    sipType = 0;

    if ( layer )
    {
      switch ( layer->type() )
      {
        case QgsMapLayer::VectorLayer:
          sipType = sipType_QgsVectorLayer;
          break;
        case QgsMapLayer::RasterLayer:
          sipType = sipType_QgsRasterLayer;
          break;
        case QgsMapLayer::PluginLayer:
          sipType = sipType_QgsPluginLayer;
          break;
        default:
          sipType = nullptr;
          break;
      }
    }
%End
  public:

    enum LayerType
    {
      VectorLayer,
      RasterLayer,
      PluginLayer
    };

    enum PropertyType
    {
      Style,
      Metadata,
    };

    QgsMapLayer( QgsMapLayer::LayerType type = VectorLayer, const QString &name = QString(), const QString &source = QString() );
%Docstring
Constructor for QgsMapLayer

:param type: layer type
:param name: display name for the layer
:param source: datasource of layer
%End

    ~QgsMapLayer();


    virtual QgsMapLayer *clone() const = 0;
%Docstring
Returns a new instance equivalent to this one except for the id which
is still unique.

:return: a new layer instance

.. versionadded:: 3.0
%End

    QgsMapLayer::LayerType type() const;
%Docstring
Returns the type of the layer.
%End

    static QString extensionPropertyType( PropertyType type );
%Docstring
Returns the extension of a Property.

:return: The extension

.. versionadded:: 3.0
%End

    QString id() const;
%Docstring
Returns the layer's unique ID, which is used to access this layer from :py:class:`QgsProject`.
%End

    void setName( const QString &name );
%Docstring
Set the display ``name`` of the layer.

.. versionadded:: 2.16

.. seealso:: :py:func:`name`
%End

    QString name() const;
%Docstring
Returns the display name of the layer.

.. seealso:: :py:func:`setName`
%End

    virtual QgsDataProvider *dataProvider();
%Docstring
Returns the layer's data provider.
%End


    void setShortName( const QString &shortName );
%Docstring
Sets the short name of the layer
used by QGIS Server to identify the layer.

:return: the layer short name

.. seealso:: :py:func:`shortName`
%End

    QString shortName() const;
%Docstring
Returns the short name of the layer
used by QGIS Server to identify the layer.

.. seealso:: :py:func:`setShortName`
%End

    void setTitle( const QString &title );
%Docstring
Sets the title of the layer
used by QGIS Server in GetCapabilities request.

.. seealso:: :py:func:`title`
%End

    QString title() const;
%Docstring
Returns the title of the layer
used by QGIS Server in GetCapabilities request.

:return: the layer title

.. seealso:: :py:func:`setTitle`
%End

    void setAbstract( const QString &abstract );
%Docstring
Sets the abstract of the layer
used by QGIS Server in GetCapabilities request.

:return: the layer abstract

.. seealso:: :py:func:`abstract`
%End

    QString abstract() const;
%Docstring
Returns the abstract of the layer
used by QGIS Server in GetCapabilities request.

:return: the layer abstract

.. seealso:: :py:func:`setAbstract`
%End

    void setKeywordList( const QString &keywords );
%Docstring
Sets the keyword list of the layer
used by QGIS Server in GetCapabilities request.

:return: the layer keyword list

.. seealso:: :py:func:`keywordList`
%End

    QString keywordList() const;
%Docstring
Returns the keyword list of the layer
used by QGIS Server in GetCapabilities request.

:return: the layer keyword list

.. seealso:: :py:func:`setKeywordList`
%End


    void setDataUrl( const QString &dataUrl );
%Docstring
Sets the DataUrl of the layer
used by QGIS Server in GetCapabilities request.
DataUrl is a a link to the underlying data represented by a particular layer.

:return: the layer DataUrl

.. seealso:: :py:func:`dataUrl`
%End

    QString dataUrl() const;
%Docstring
Returns the DataUrl of the layer
used by QGIS Server in GetCapabilities request.
DataUrl is a a link to the underlying data represented by a particular layer.

:return: the layer DataUrl

.. seealso:: :py:func:`setDataUrl`
%End

    void setDataUrlFormat( const QString &dataUrlFormat );
%Docstring
Sets the DataUrl format of the layer
used by QGIS Server in GetCapabilities request.
DataUrl is a a link to the underlying data represented by a particular layer.

:return: the layer DataUrl format

.. seealso:: :py:func:`dataUrlFormat`
%End

    QString dataUrlFormat() const;
%Docstring
Returns the DataUrl format of the layer
used by QGIS Server in GetCapabilities request.
DataUrl is a a link to the underlying data represented by a particular layer.

:return: the layer DataUrl format

.. seealso:: :py:func:`setDataUrlFormat`
%End


    void setAttribution( const QString &attrib );
%Docstring
Sets the attribution of the layer
used by QGIS Server in GetCapabilities request.
Attribution indicates the provider of a layer or collection of layers.

:return: the layer attribution

.. seealso:: :py:func:`attribution`
%End

    QString attribution() const;
%Docstring
Returns the attribution of the layer
used by QGIS Server in GetCapabilities request.
Attribution indicates the provider of a layer or collection of layers.

:return: the layer attribution

.. seealso:: :py:func:`setAttribution`
%End

    void setAttributionUrl( const QString &attribUrl );
%Docstring
Sets the attribution URL of the layer
used by QGIS Server in GetCapabilities request.
Attribution indicates the provider of a layer or collection of layers.

:return: the layer attribution URL

.. seealso:: :py:func:`attributionUrl`
%End

    QString attributionUrl() const;
%Docstring
Returns the attribution URL of the layer
used by QGIS Server in GetCapabilities request.
Attribution indicates the provider of a layer or collection of layers.

:return: the layer attribution URL

.. seealso:: :py:func:`setAttributionUrl`
%End


    void setMetadataUrl( const QString &metaUrl );
%Docstring
Sets the metadata URL of the layer
used by QGIS Server in GetCapabilities request.
MetadataUrl is a a link to the detailed, standardized metadata about the data.

:return: the layer metadata URL

.. seealso:: :py:func:`metadataUrl`
%End

    QString metadataUrl() const;
%Docstring
Returns the metadata URL of the layer
used by QGIS Server in GetCapabilities request.
MetadataUrl is a a link to the detailed, standardized metadata about the data.

:return: the layer metadata URL

.. seealso:: :py:func:`setMetadataUrl`
%End

    void setMetadataUrlType( const QString &metaUrlType );
%Docstring
Set the metadata type of the layer
used by QGIS Server in GetCapabilities request
MetadataUrlType indicates the standard to which the metadata complies.

:return: the layer metadata type

.. seealso:: :py:func:`metadataUrlType`
%End

    QString metadataUrlType() const;
%Docstring
Returns the metadata type of the layer
used by QGIS Server in GetCapabilities request.
MetadataUrlType indicates the standard to which the metadata complies.

:return: the layer metadata type

.. seealso:: :py:func:`setMetadataUrlType`
%End

    void setMetadataUrlFormat( const QString &metaUrlFormat );
%Docstring
Sets the metadata format of the layer
used by QGIS Server in GetCapabilities request.
MetadataUrlType indicates how the metadata is structured.

:return: the layer metadata format

.. seealso:: :py:func:`metadataUrlFormat`
%End

    QString metadataUrlFormat() const;
%Docstring
Returns the metadata format of the layer
used by QGIS Server in GetCapabilities request.
MetadataUrlType indicates how the metadata is structured.

:return: the layer metadata format

.. seealso:: :py:func:`setMetadataUrlFormat`
%End

    void setBlendMode( QPainter::CompositionMode blendMode );
%Docstring
Set the blending mode used for rendering a layer.

:param blendMode: new blending mode

.. seealso:: :py:func:`blendMode`
%End

    QPainter::CompositionMode blendMode() const;
%Docstring
Returns the current blending mode for a layer.

.. seealso:: :py:func:`setBlendMode`
%End

    bool readOnly() const;
%Docstring
Returns if this layer is read only.
%End

    virtual void reload();
%Docstring
Synchronises with changes in the datasource
%End

    virtual QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) = 0 /Factory/;
%Docstring
Return new instance of QgsMapLayerRenderer that will be used for rendering of given context

.. versionadded:: 2.4
%End

    virtual QgsRectangle extent() const;
%Docstring
Returns the extent of the layer.
%End

    bool isValid() const;
%Docstring
Return the status of the layer. An invalid layer is one which has a bad datasource
or other problem. Child classes set this flag when initialized.

:return: true if the layer is valid and can be accessed
%End

    QString publicSource() const;
%Docstring
Gets a version of the internal layer definition that has sensitive
bits removed (for example, the password). This function should
be used when displaying the source name for general viewing.

.. seealso:: :py:func:`source`
%End

    QString source() const;
%Docstring
Returns the source for the layer. This source may contain usernames, passwords
and other sensitive information.

.. seealso:: :py:func:`publicSource`
%End

    virtual QStringList subLayers() const;
%Docstring
Returns the sublayers of this layer.
(Useful for providers that manage their own layers, such as WMS).
%End

    virtual void setLayerOrder( const QStringList &layers );
%Docstring
Reorders the *previously selected* sublayers of this layer from bottom to top.
(Useful for providers that manage their own layers, such as WMS).
%End

    virtual void setSubLayerVisibility( const QString &name, bool visible );
%Docstring
Set the visibility of the given sublayer name.

:param name: sublayer name
:param visible: sublayer visibility
%End

    virtual bool isEditable() const;
%Docstring
Returns true if the layer can be edited.
%End

    virtual bool isSpatial() const;
%Docstring
Returns true if the layer is considered a spatial layer, ie it has some form of geometry associated with it.

.. versionadded:: 2.16
%End

    bool readLayerXml( const QDomElement &layerElement, const QgsReadWriteContext &context );
%Docstring
Sets state from Dom document

:param layerElement: The Dom element corresponding to ``maplayer'' tag
:param context: writing context (e.g. for conversion between relative and absolute paths)
\note

The Dom node corresponds to a Dom document project file XML element read
by :py:class:`QgsProject`.

This, in turn, calls readXml(), which is over-rideable by sub-classes so
that they can read their own specific state from the given Dom node.

Invoked by :py:func:`QgsProject.read()`

:return: true if successful
%End

    bool writeLayerXml( QDomElement &layerElement, QDomDocument &document, const QgsReadWriteContext &context ) const;
%Docstring
Stores state in Dom node

:param layerElement: is a Dom element corresponding to ``maplayer'' tag
:param document: is a the dom document being written
:param context: reading context (e.g. for conversion between relative and absolute paths)
\note

The Dom node corresponds to a Dom document project file XML element to be
written by :py:class:`QgsProject`.

This, in turn, calls writeXml(), which is over-rideable by sub-classes so
that they can write their own specific state to the given Dom node.

Invoked by :py:func:`QgsProject.write()`

:return: true if successful
%End

    virtual void resolveReferences( QgsProject *project );
%Docstring
Resolve references to other layers (kept as layer IDs after reading XML) into layer objects.

.. versionadded:: 3.0
%End

    QStringList customPropertyKeys() const;
%Docstring
Returns list of all keys within custom properties. Properties are stored in a map and saved in project file.

.. seealso:: :py:func:`customProperty`

.. versionadded:: 3.0
%End

    void setCustomProperty( const QString &key, const QVariant &value );
%Docstring
Set a custom property for layer. Properties are stored in a map and saved in project file.

.. seealso:: :py:func:`customProperty`

.. seealso:: :py:func:`removeCustomProperty`
%End

    QVariant customProperty( const QString &value, const QVariant &defaultValue = QVariant() ) const;
%Docstring
Read a custom property from layer. Properties are stored in a map and saved in project file.

.. seealso:: :py:func:`setCustomProperty`
%End

    void setCustomProperties( const QgsObjectCustomProperties &properties );
%Docstring
Set custom properties for layer. Current properties are dropped.

.. versionadded:: 3.0
%End

    void removeCustomProperty( const QString &key );
%Docstring
Remove a custom property from layer. Properties are stored in a map and saved in project file.

.. seealso:: :py:func:`setCustomProperty`
%End

    virtual QgsError error() const;
%Docstring
Get current status error. This error describes some principal problem
for which layer cannot work and thus is not valid. It is not last error
after accessing data by draw() etc.
%End

    QgsCoordinateReferenceSystem crs() const;
%Docstring
Returns the layer's spatial reference system.

.. versionadded:: 1.4
%End

    void setCrs( const QgsCoordinateReferenceSystem &srs, bool emitSignal = true );
%Docstring
Sets layer's spatial reference system
%End

    static QString formatLayerName( const QString &name );
%Docstring
A convenience function to capitalize and format a layer ``name``.

.. versionadded:: 3.0
%End

    virtual QString metadataUri() const;
%Docstring
Retrieve the metadata URI for this layer
(either as a .qmd file on disk or as a
record in the users style table in their personal qgis.db)

:return: a QString with the metadata file name

.. versionadded:: 3.0
%End

    void exportNamedMetadata( QDomDocument &doc, QString &errorMsg ) const;
%Docstring
Export the current metadata of this layer as named metadata in a QDomDocument

:param doc: the target QDomDocument
:param errorMsg: this QString will be initialized on error

.. versionadded:: 3.0
%End

    virtual QString saveDefaultMetadata( bool &resultFlag /Out/ );
%Docstring
Save the current metadata of this layer as the default metadata
(either as a .qmd file on disk or as a
record in the users style table in their personal qgis.db)

:param resultFlag: a reference to a flag that will be set to false if
we did not manage to save the default metadata.

:return: a QString with any status messages

.. versionadded:: 3.0
%End

    QString saveNamedMetadata( const QString &uri, bool &resultFlag );
%Docstring
Save the current metadata of this layer as a named metadata
(either as a .qmd file on disk or as a
record in the users style table in their personal qgis.db)

:param uri: the file name or other URI for the
metadata file. First an attempt will be made to see if this
is a file and save to that, if that fails the qgis.db metadata
table will be used to create a metadata entry who's
key matches the URI.
:param resultFlag: a reference to a flag that will be set to false if
we did not manage to save the default metadata.

:return: a QString with any status messages

.. versionadded:: 3.0
%End

    virtual QString loadNamedMetadata( const QString &uri, bool &resultFlag /Out/ );
%Docstring
Retrieve a named metadata for this layer if one
exists (either as a .qmd file on disk or as a
record in the users style table in their personal qgis.db)

:param uri: - the file name or other URI for the
metadata file. First an attempt will be made to see if this
is a file and load that, if that fails the qgis.db metadata
table will be consulted to see if there is a metadata who's
key matches the URI.
:param resultFlag: a reference to a flag that will be set to false if
we did not manage to load the default metadata.

:return: a QString with any status messages

.. versionadded:: 3.0
%End

    QString loadDefaultMetadata( bool &resultFlag );
%Docstring
Retrieve the default metadata for this layer if one
exists (either as a .qmd file on disk or as a
record in the users metadata table in their personal qgis.db)

:param resultFlag: a reference to a flag that will be set to false if
we did not manage to load the default metadata.

:return: a QString with any status messages

.. versionadded:: 3.0
%End

    bool loadNamedMetadataFromDatabase( const QString &db, const QString &uri, QString &qmd );
%Docstring
Retrieve a named metadata for this layer from a sqlite database.

:param db: path to sqlite database
:param uri: uri for table
:param qmd: will be set to QMD xml metadata content from database

:return: true if style was successfully loaded

.. versionadded:: 3.0
%End

    bool importNamedMetadata( QDomDocument &document, QString &errorMessage );
%Docstring
Import the metadata of this layer from a QDomDocument

:param document: source QDomDocument
:param errorMessage: this QString will be initialized on error

:return: true on success

.. versionadded:: 3.0
%End

    virtual QString styleURI() const;
%Docstring
Retrieve the style URI for this layer
(either as a .qml file on disk or as a
record in the users style table in their personal qgis.db)

:return: a QString with the style file name

.. seealso:: :py:func:`also`
%End

    virtual QString loadDefaultStyle( bool &resultFlag /Out/ );
%Docstring
Retrieve the default style for this layer if one
exists (either as a .qml file on disk or as a
record in the users style table in their personal qgis.db)

:param resultFlag: a reference to a flag that will be set to false if
we did not manage to load the default style.

:return: a QString with any status messages

.. seealso:: :py:func:`also`
%End

    virtual QString loadNamedStyle( const QString &uri, bool &resultFlag /Out/ );
%Docstring
Retrieve a named style for this layer if one
exists (either as a .qml file on disk or as a
record in the users style table in their personal qgis.db)

:param uri: - the file name or other URI for the
style file. First an attempt will be made to see if this
is a file and load that, if that fails the qgis.db styles
table will be consulted to see if there is a style who's
key matches the URI.
:param resultFlag: a reference to a flag that will be set to false if
we did not manage to load the default style.

:return: a QString with any status messages

.. seealso:: :py:func:`also`
%End

    virtual bool loadNamedStyleFromDatabase( const QString &db, const QString &uri, QString &qml /Out/ );
%Docstring
Retrieve a named style for this layer from a sqlite database.

:param db: path to sqlite database
:param uri: uri for table
:param qml: will be set to QML style content from database

:return: true if style was successfully loaded
%End

    virtual bool importNamedStyle( QDomDocument &doc, QString &errorMsg /Out/ );
%Docstring
Import the properties of this layer from a QDomDocument

:param doc: source QDomDocument
:param errorMsg: this QString will be initialized on error
during the execution of readSymbology

:return: true on success

.. versionadded:: 2.8
%End

    virtual void exportNamedStyle( QDomDocument &doc, QString &errorMsg ) const;
%Docstring
Export the properties of this layer as named style in a QDomDocument

:param doc: the target QDomDocument
:param errorMsg: this QString will be initialized on error
during the execution of writeSymbology
%End


    virtual void exportSldStyle( QDomDocument &doc, QString &errorMsg ) const;
%Docstring
Export the properties of this layer as SLD style in a QDomDocument

:param doc: the target QDomDocument
:param errorMsg: this QString will be initialized on error
during the execution of writeSymbology
%End

    virtual QString saveDefaultStyle( bool &resultFlag /Out/ );
%Docstring
Save the properties of this layer as the default style
(either as a .qml file on disk or as a
record in the users style table in their personal qgis.db)

:param resultFlag: a reference to a flag that will be set to false if
we did not manage to save the default style.

:return: a QString with any status messages

.. seealso:: :py:func:`loadNamedStyle`
%End

    virtual QString saveNamedStyle( const QString &uri, bool &resultFlag /Out/ );
%Docstring
Save the properties of this layer as a named style
(either as a .qml file on disk or as a
record in the users style table in their personal qgis.db)

:param uri: the file name or other URI for the
style file. First an attempt will be made to see if this
is a file and save to that, if that fails the qgis.db styles
table will be used to create a style entry who's
key matches the URI.
:param resultFlag: a reference to a flag that will be set to false if
we did not manage to save the default style.

:return: a QString with any status messages

.. seealso:: :py:func:`saveDefaultStyle`
%End

    virtual QString saveSldStyle( const QString &uri, bool &resultFlag ) const;
%Docstring
Saves the properties of this layer to an SLD format file.

:param uri: uri of destination for exported SLD file.
:param resultFlag: a reference to a flag that will be set to false if
the SLD file could not be generated

:return: a string with any status or error messages

.. seealso:: :py:func:`loadSldStyle`
%End

    virtual QString loadSldStyle( const QString &uri, bool &resultFlag );
%Docstring
Attempts to style the layer using the formatting from an SLD type file.

:param uri: uri of source SLD file
:param resultFlag: a reference to a flag that will be set to false if
the SLD file could not be loaded

:return: a string with any status or error messages

.. seealso:: :py:func:`saveSldStyle`
%End

    virtual bool readSld( const QDomNode &node, QString &errorMessage );



    virtual bool readSymbology( const QDomNode &node, QString &errorMessage, const QgsReadWriteContext &context ) = 0;
%Docstring
Read the symbology for the current layer from the Dom node supplied.

:param node: node that will contain the symbology definition for this layer.
:param errorMessage: reference to string that will be updated with any error messages
:param context: reading context (used for transform from relative to absolute paths)

:return: true in case of success.
%End

    virtual bool readStyle( const QDomNode &node, QString &errorMessage, const QgsReadWriteContext &context );
%Docstring
Read the style for the current layer from the Dom node supplied.

:param node: node that will contain the style definition for this layer.
:param errorMessage: reference to string that will be updated with any error messages
:param context: reading context (used for transform from relative to absolute paths)

:return: true in case of success.

.. versionadded:: 2.16

.. note::

   To be implemented in subclasses. Default implementation does nothing and returns false.
%End

    virtual bool writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context ) const = 0;
%Docstring
Write the symbology for the layer into the docment provided.

:param node: the node that will have the style element added to it.
:param doc: the document that will have the QDomNode added.
:param errorMessage: reference to string that will be updated with any error messages
:param context: writing context (used for transform from absolute to relative paths)

:return: true in case of success.
%End

    virtual bool writeStyle( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context ) const;
%Docstring
Write just the style information for the layer into the document

:param node: the node that will have the style element added to it.
:param doc: the document that will have the QDomNode added.
:param errorMessage: reference to string that will be updated with any error messages
:param context: writing context (used for transform from absolute to relative paths)

:return: true in case of success.

.. versionadded:: 2.16

.. note::

   To be implemented in subclasses. Default implementation does nothing and returns false.
%End

    QUndoStack *undoStack();
%Docstring
Return pointer to layer's undo stack
%End

    QUndoStack *undoStackStyles();
%Docstring
Return pointer to layer's style undo stack

.. versionadded:: 2.16
%End

    void setLegendUrl( const QString &legendUrl );
%Docstring
Sets the URL for the layer's legend.
%End

    QString legendUrl() const;
%Docstring
Returns the URL for the layer's legend.
%End

    void setLegendUrlFormat( const QString &legendUrlFormat );
%Docstring
Sets the format for a URL based layer legend.
%End

    QString legendUrlFormat() const;
%Docstring
Returns the format for a URL based layer legend.
%End

    void setLegend( QgsMapLayerLegend *legend /Transfer/ );
%Docstring
Assign a legend controller to the map layer. The object will be responsible for providing legend items.

:param legend: Takes ownership of the object. Can be null pointer

.. versionadded:: 2.6
%End

    QgsMapLayerLegend *legend() const;
%Docstring
Can be null.

.. versionadded:: 2.6
%End

    QgsMapLayerStyleManager *styleManager() const;
%Docstring
Get access to the layer's style manager. Style manager allows switching between multiple styles.

.. versionadded:: 2.8
%End

    void setRenderer3D( QgsAbstract3DRenderer *renderer /Transfer/ );
%Docstring
Sets 3D renderer for the layer. Takes ownership of the renderer.

.. versionadded:: 3.0
%End

    QgsAbstract3DRenderer *renderer3D() const;
%Docstring
Returns 3D renderer associated with the layer. May be null.

.. versionadded:: 3.0
%End

    bool isInScaleRange( double scale ) const;
%Docstring
Tests whether the layer should be visible at the specified ``scale``.
The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.

:return: true if the layer is visible at the given scale.

.. versionadded:: 2.16

.. seealso:: :py:func:`minimumScale`

.. seealso:: :py:func:`maximumScale`

.. seealso:: :py:func:`hasScaleBasedVisibility`
%End

    double minimumScale() const;
%Docstring
Returns the minimum map scale (i.e. most "zoomed out" scale) at which the layer will be visible.
The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
A scale of 0 indicates no minimum scale visibility.

.. note::

   Scale based visibility is only used if setScaleBasedVisibility() is set to true.

.. seealso:: :py:func:`setMinimumScale`

.. seealso:: :py:func:`maximumScale`

.. seealso:: :py:func:`hasScaleBasedVisibility`

.. seealso:: :py:func:`isInScaleRange`
%End

    double maximumScale() const;
%Docstring
Returns the maximum map scale (i.e. most "zoomed in" scale) at which the layer will be visible.
The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
A scale of 0 indicates no maximum scale visibility.

.. note::

   Scale based visibility is only used if setScaleBasedVisibility() is set to true.

.. seealso:: :py:func:`setMaximumScale`

.. seealso:: :py:func:`minimumScale`

.. seealso:: :py:func:`hasScaleBasedVisibility`

.. seealso:: :py:func:`isInScaleRange`
%End

    bool hasScaleBasedVisibility() const;
%Docstring
Returns whether scale based visibility is enabled for the layer.

:return: true if scale based visibility is enabled

.. seealso:: :py:func:`minimumScale`

.. seealso:: :py:func:`maximumScale`

.. seealso:: :py:func:`setScaleBasedVisibility`

.. seealso:: :py:func:`isInScaleRange`
%End

    bool hasAutoRefreshEnabled() const;
%Docstring
Returns true if auto refresh is enabled for the layer.

.. versionadded:: 3.0

.. seealso:: :py:func:`autoRefreshInterval`

.. seealso:: :py:func:`setAutoRefreshEnabled`
%End

    int autoRefreshInterval() const;
%Docstring
Returns the auto refresh interval (in milliseconds). Note that
auto refresh is only active when hasAutoRefreshEnabled() is true.

.. versionadded:: 3.0

.. seealso:: :py:func:`autoRefreshEnabled`

.. seealso:: :py:func:`setAutoRefreshInterval`
%End

    void setAutoRefreshInterval( int interval );
%Docstring
Sets the auto refresh interval (in milliseconds) for the layer. This
will cause the layer to be automatically redrawn on a matching interval.
Note that auto refresh must be enabled by calling setAutoRefreshEnabled().

Note that auto refresh triggers deferred repaints of the layer. Any map
canvas must be refreshed separately in order to view the refreshed layer.

.. versionadded:: 3.0

.. seealso:: :py:func:`autoRefreshInterval`

.. seealso:: :py:func:`setAutoRefreshEnabled`
%End

    void setAutoRefreshEnabled( bool enabled );
%Docstring
Sets whether auto refresh is enabled for the layer.

.. versionadded:: 3.0

.. seealso:: :py:func:`hasAutoRefreshEnabled`

.. seealso:: :py:func:`setAutoRefreshInterval`
%End

    virtual const QgsLayerMetadata &metadata() const;
%Docstring
Returns a reference to the layer's metadata store.

.. versionadded:: 3.0

.. seealso:: :py:func:`setMetadata`

.. seealso:: :py:func:`metadataChanged`
%End

    virtual void setMetadata( const QgsLayerMetadata &metadata );
%Docstring
Sets the layer's ``metadata`` store.

.. versionadded:: 3.0

.. seealso:: :py:func:`metadata`

.. seealso:: :py:func:`metadataChanged`
%End

    virtual QString htmlMetadata() const;
%Docstring
Obtain a formatted HTML string containing assorted metadata for this layer.

.. versionadded:: 3.0
%End

    virtual QDateTime timestamp() const;
%Docstring
Time stamp of data source in the moment when data/metadata were loaded by provider
%End

    virtual QSet<QgsMapLayerDependency> dependencies() const;
%Docstring
Gets the list of dependencies. This includes data dependencies set by the user (:py:func:`setDataDependencies`)
as well as dependencies given by the provider

:return: a set of QgsMapLayerDependency

.. versionadded:: 3.0
%End

    QString refreshOnNotifyMessage() const;
%Docstring
Returns the message that should be notified by the provider to triggerRepaint

.. versionadded:: 3.0
%End

    bool isRefreshOnNotifyEnabled() const;
%Docstring
Returns true if the refresh on provider nofification is enabled

.. versionadded:: 3.0
%End

  public slots:

    void setMinimumScale( double scale );
%Docstring
Sets the minimum map ``scale`` (i.e. most "zoomed out" scale) at which the layer will be visible.
The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
A ``scale`` of 0 indicates no minimum scale visibility.

.. note::

   Scale based visibility is only used if setScaleBasedVisibility() is set to true.

.. seealso:: :py:func:`minimumScale`

.. seealso:: :py:func:`setMaximumScale`

.. seealso:: :py:func:`setScaleBasedVisibility`
%End

    void setMaximumScale( double scale );
%Docstring
Sets the maximum map ``scale`` (i.e. most "zoomed in" scale) at which the layer will be visible.
The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
A ``scale`` of 0 indicates no maximum scale visibility.

.. note::

   Scale based visibility is only used if setScaleBasedVisibility() is set to true.

.. seealso:: :py:func:`maximumScale`

.. seealso:: :py:func:`setMinimumScale`

.. seealso:: :py:func:`setScaleBasedVisibility`
%End

    void setScaleBasedVisibility( const bool enabled );
%Docstring
Sets whether scale based visibility is enabled for the layer.

:param enabled: set to true to enable scale based visibility

.. seealso:: :py:func:`setMinimumScale`

.. seealso:: :py:func:`setMaximumScale`

.. seealso:: :py:func:`scaleBasedVisibility`
%End

    void triggerRepaint( bool deferredUpdate = false );
%Docstring
Will advise the map canvas (and any other interested party) that this layer requires to be repainted.
Will emit a repaintRequested() signal.
If ``deferredUpdate`` is true then the layer will only be repainted when the canvas is next
re-rendered, and will not trigger any canvas redraws itself.

.. note::

   in 2.6 function moved from vector/raster subclasses to QgsMapLayer
%End

    void emitStyleChanged();
%Docstring
Triggers an emission of the styleChanged() signal.

.. versionadded:: 2.16
%End

    virtual bool setDependencies( const QSet<QgsMapLayerDependency> &layers );
%Docstring
Sets the list of dependencies.

.. seealso:: :py:func:`dependencies`

:param layers: set of QgsMapLayerDependency. Only user-defined dependencies will be added

:return: false if a dependency cycle has been detected

.. versionadded:: 3.0
%End

    void setRefreshOnNotifyEnabled( bool enabled );
%Docstring
Set whether provider notification is connected to triggerRepaint

.. versionadded:: 3.0
%End

    void setRefreshOnNofifyMessage( const QString &message );
%Docstring
Set the notification message that triggers repaine
If refresh on notification is enabled, the notification will triggerRepaint only

if the notification message is equal to:param message:

.. versionadded:: 3.0
%End

  signals:

    void statusChanged( const QString &status );
%Docstring
Emit a signal with status (e.g. to be caught by QgisApp and display a msg on status bar)
%End

    void nameChanged();
%Docstring
Emitted when the name has been changed

.. versionadded:: 2.16
%End

    void crsChanged();
%Docstring
Emit a signal that layer's CRS has been reset
%End

    void repaintRequested( bool deferredUpdate = false );
%Docstring
By emitting this signal the layer tells that either appearance or content have been changed
and any view showing the rendered layer should refresh itself.
If ``deferredUpdate`` is true then the layer will only be repainted when the canvas is next
re-rendered, and will not trigger any canvas redraws itself.
%End

    void recalculateExtents() const;
%Docstring
This is used to send a request that any mapcanvas using this layer update its extents
%End

    void dataChanged();
%Docstring
Data of layer changed
%End

    void blendModeChanged( QPainter::CompositionMode blendMode );
%Docstring
Signal emitted when the blend mode is changed, through :py:func:`QgsMapLayer.setBlendMode()`
%End

    void rendererChanged();
%Docstring
Signal emitted when renderer is changed.

.. seealso:: :py:func:`styleChanged`
%End

    void styleChanged();
%Docstring
Signal emitted whenever a change affects the layer's style. Ie this may be triggered
by renderer changes, label style changes, or other style changes such as blend
mode or layer opacity changes.

.. versionadded:: 2.16

.. seealso:: :py:func:`rendererChanged`
%End

    void legendChanged();
%Docstring
Signal emitted when legend of the layer has changed

.. versionadded:: 2.6
%End

    void renderer3DChanged();
%Docstring
Signal emitted when 3D renderer associated with the layer has changed.

.. versionadded:: 3.0
%End

    void configChanged();
%Docstring
Emitted whenever the configuration is changed. The project listens to this signal
to be marked as dirty.
%End

    void dependenciesChanged();
%Docstring
Emitted when dependencies are changed.
%End

    void willBeDeleted();
%Docstring
Emitted in the destructor when the layer is about to be deleted,
but it is still in a perfectly valid state: the last chance for
other pieces of code for some cleanup if they use the layer.

.. versionadded:: 3.0
%End

    void autoRefreshIntervalChanged( int interval );
%Docstring
Emitted when the auto refresh interval changes.

.. seealso:: :py:func:`setAutoRefreshInterval`

.. versionadded:: 3.0
%End

    void metadataChanged();
%Docstring
Emitted when the layer's metadata is changed.

.. seealso:: :py:func:`setMetadata`

.. seealso:: :py:func:`metadata`

.. versionadded:: 3.0
%End

  protected:

    void clone( QgsMapLayer *layer ) const;
%Docstring
Copies attributes like name, short name, ... into another layer.

:param layer: The copy recipient

.. versionadded:: 3.0
%End

    virtual void setExtent( const QgsRectangle &rect );
%Docstring
Set the extent
%End

    void setValid( bool valid );
%Docstring
Set whether layer is valid or not - should be used in constructor.
%End

    virtual bool readXml( const QDomNode &layer_node, const QgsReadWriteContext &context );
%Docstring
Called by readLayerXML(), used by children to read state specific to them from
project files.
%End

    virtual bool writeXml( QDomNode &layer_node, QDomDocument &document, const QgsReadWriteContext &context ) const;
%Docstring
Called by writeLayerXML(), used by children to write state specific to them to
project files.
%End

    void readCustomProperties( const QDomNode &layerNode, const QString &keyStartsWith = QString() );
%Docstring
Read custom properties from project file.

:param layerNode: note to read from
:param keyStartsWith: reads only properties starting with the specified string (or all if the string is empty)*
%End

    void writeCustomProperties( QDomNode &layerNode, QDomDocument &doc ) const;
%Docstring
Write custom properties to project file.
%End

    void readStyleManager( const QDomNode &layerNode );
%Docstring
Read style manager's configuration (if any). To be called by subclasses.
%End
    void writeStyleManager( QDomNode &layerNode, QDomDocument &doc ) const;
%Docstring
Write style manager's configuration (if exists). To be called by subclasses.
%End

    void writeCommonStyle( QDomElement &layerElement, QDomDocument &document, const QgsReadWriteContext &context ) const;
%Docstring
Write style data common to all layer types

.. versionadded:: 3.0
%End

    void readCommonStyle( const QDomElement &layerElement, const QgsReadWriteContext &context );
%Docstring
Read style data common to all layer types

.. versionadded:: 3.0
%End


    void appendError( const QgsErrorMessage &error );
%Docstring
Add error message
%End
    void setError( const QgsError &error );
%Docstring
Set error message
%End













    bool hasDependencyCycle( const QSet<QgsMapLayerDependency> &layers ) const;
%Docstring
Checks whether a new set of dependencies will introduce a cycle
%End


};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsmaplayer.h                                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
