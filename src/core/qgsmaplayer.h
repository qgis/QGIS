/***************************************************************************
                          qgsmaplayer.h  -  description
                             -------------------
    begin                : Fri Jun 28 2002
    copyright            : (C) 2002 by Gary E.Sherman
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

#ifndef QGSMAPLAYER_H
#define QGSMAPLAYER_H

#include "qgis_core.h"
#include <QDateTime>
#include <QDomNode>
#include <QImage>
#include <QObject>
#include <QPainter>
#include <QUndoStack>
#include <QVariant>
#include <QIcon>
#include <QSet>

#include "qgis_sip.h"
#include "qgserror.h"
#include "qgsobjectcustomproperties.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsmaplayerdependency.h"
#include "qgslayermetadata.h"
#include "qgsmaplayerserverproperties.h"
#include "qgsreadwritecontext.h"
#include "qgsdataprovider.h"
#include "qgis.h"
#include "qgslogger.h"

class QgsAbstract3DRenderer;
class QgsDataProvider;
class QgsMapLayerLegend;
class QgsMapLayerRenderer;
class QgsMapLayerStyleManager;
class QgsProject;
class QgsStyleEntityVisitorInterface;
class QgsMapLayerTemporalProperties;
class QgsMapLayerElevationProperties;
class QgsMapLayerSelectionProperties;
class QgsSldExportContext;

class QDomDocument;
class QKeyEvent;
class QPainter;
class QgsRenderContext;
class QgsBox3D;

/*
 * Constants used to describe copy-paste MIME types
 */
#define QGSCLIPBOARD_MAPLAYER_MIME "application/qgis.maplayer"


/**
 * \ingroup core
 * \brief Base class for all map layer types.
 * This is the base class for all map layer types (vector, raster).
 */
class CORE_EXPORT QgsMapLayer : public QObject
{
    Q_OBJECT

    Q_PROPERTY( QString id READ id WRITE setId NOTIFY idChanged )
    Q_PROPERTY( QString name READ name WRITE setName NOTIFY nameChanged )
    Q_PROPERTY( int autoRefreshInterval READ autoRefreshInterval WRITE setAutoRefreshInterval NOTIFY autoRefreshIntervalChanged )
    Q_PROPERTY( QgsLayerMetadata metadata READ metadata WRITE setMetadata NOTIFY metadataChanged )
    Q_PROPERTY( QgsCoordinateReferenceSystem crs READ crs WRITE setCrs NOTIFY crsChanged )
    Q_PROPERTY( QgsCoordinateReferenceSystem verticalCrs READ verticalCrs WRITE setVerticalCrs NOTIFY verticalCrsChanged )
    Q_PROPERTY( QgsCoordinateReferenceSystem crs3D READ crs3D NOTIFY crs3DChanged )
    Q_PROPERTY( Qgis::LayerType type READ type CONSTANT )
    Q_PROPERTY( bool isValid READ isValid NOTIFY isValidChanged )
    Q_PROPERTY( double opacity READ opacity WRITE setOpacity NOTIFY opacityChanged )
    Q_PROPERTY( QString mapTipTemplate READ mapTipTemplate WRITE setMapTipTemplate NOTIFY mapTipTemplateChanged )
    Q_PROPERTY( bool mapTipsEnabled READ mapTipsEnabled WRITE setMapTipsEnabled NOTIFY mapTipsEnabledChanged )

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    QgsMapLayer *layer = qobject_cast<QgsMapLayer *>( sipCpp );

    sipType = 0;

    if ( layer )
    {
      switch ( layer->type() )
      {
        case Qgis::LayerType::Vector:
          sipType = sipType_QgsVectorLayer;
          break;
        case Qgis::LayerType::Raster:
          sipType = sipType_QgsRasterLayer;
          break;
        case Qgis::LayerType::Plugin:
          sipType = sipType_QgsPluginLayer;
          break;
        case Qgis::LayerType::Mesh:
          sipType = sipType_QgsMeshLayer;
          break;
        case Qgis::LayerType::VectorTile:
          sipType = sipType_QgsVectorTileLayer;
          break;
        case Qgis::LayerType::Annotation:
          sipType = sipType_QgsAnnotationLayer;
          break;
        case Qgis::LayerType::PointCloud:
          sipType = sipType_QgsPointCloudLayer;
          break;
        case Qgis::LayerType::Group:
          sipType = sipType_QgsGroupLayer;
          break;
        case Qgis::LayerType::TiledScene:
          sipType = sipType_QgsTiledSceneLayer;
          break;
        default:
          sipType = nullptr;
          break;
      }
    }
    SIP_END
#endif

  public:

    /**
     * Maplayer has a style and a metadata property
     */
    enum PropertyType
    {
      Style = 0,
      Metadata,
    };

    /**
     * Flags for the map layer
     * \note Flags are options specified by the user used for the UI but are not preventing any API call.
     * \since QGIS 3.4
     */
    enum LayerFlag SIP_ENUM_BASETYPE( IntFlag )
    {
      Identifiable = 1 << 0, //!< If the layer is identifiable using the identify map tool and as a WMS layer.
      Removable = 1 << 1,    //!< If the layer can be removed from the project. The layer will not be removable from the legend menu entry but can still be removed with an API call.
      Searchable = 1 << 2,   //!< Only for vector-layer, determines if the layer is used in the 'search all layers' locator.
      Private = 1 << 3,       //!< Determines if the layer is meant to be exposed to the GUI, i.e. visible in the layer legend tree.
    };
    Q_ENUM( LayerFlag )
    Q_DECLARE_FLAGS( LayerFlags, LayerFlag )
    Q_FLAG( LayerFlags )

    /**
     * Categories of style to distinguish appropriate sections for import/export
     * \since QGIS 3.4
     */
    enum StyleCategory SIP_ENUM_BASETYPE( IntFlag )
    {
      LayerConfiguration = 1 << 0,  //!< General configuration: identifiable, removable, searchable, display expression, read-only
      Symbology          = 1 << 1,  //!< Symbology
      Symbology3D        = 1 << 2,  //!< 3D symbology
      Labeling           = 1 << 3,  //!< Labeling
      Fields             = 1 << 4,  //!< Aliases, widgets, WMS/WFS, expressions, constraints, virtual fields
      Forms              = 1 << 5,  //!< Feature form
      Actions            = 1 << 6,  //!< Actions
      MapTips            = 1 << 7,  //!< Map tips
      Diagrams           = 1 << 8,  //!< Diagrams
      AttributeTable     = 1 << 9,  //!< Attribute table settings: choice and order of columns, conditional styling
      Rendering          = 1 << 10, //!< Rendering: scale visibility, simplify method, opacity
      CustomProperties   = 1 << 11, //!< Custom properties (by plugins for instance)
      GeometryOptions    = 1 << 12, //!< Geometry validation configuration
      Relations          = 1 << 13, //!< Relations
      Temporal           = 1 << 14, //!< Temporal properties (since QGIS 3.14)
      Legend             = 1 << 15, //!< Legend settings (since QGIS 3.16)
      Elevation          = 1 << 16, //!< Elevation settings (since QGIS 3.18)
      Notes              = 1 << 17, //!< Layer user notes (since QGIS 3.20)
      AllStyleCategories = LayerConfiguration | Symbology | Symbology3D | Labeling | Fields | Forms | Actions |
                           MapTips | Diagrams | AttributeTable | Rendering | CustomProperties | GeometryOptions | Relations | Temporal | Legend | Elevation | Notes,
    };
    Q_ENUM( StyleCategory )
    Q_DECLARE_FLAGS( StyleCategories, StyleCategory )
    Q_FLAG( StyleCategories )

    /**
     * Constructor for QgsMapLayer
     * \param type layer type
     * \param name display name for the layer
     * \param source datasource of layer
     */
    QgsMapLayer( Qgis::LayerType type = Qgis::LayerType::Vector, const QString &name = QString(), const QString &source = QString() );

    ~QgsMapLayer() override;

    //! QgsMapLayer cannot be copied
    QgsMapLayer( QgsMapLayer const & ) = delete;
    //! QgsMapLayer cannot be copied
    QgsMapLayer &operator=( QgsMapLayer const & ) = delete;

    /**
     * Returns a new instance equivalent to this one except for the id which
     *  is still unique.
     * \returns a new layer instance
     */
    virtual QgsMapLayer *clone() const = 0;

    /**
     * Returns the type of the layer.
     */
    Qgis::LayerType type() const;

    /**
     * Returns the flags for this layer.
     * \note Flags are options specified by the user used for the UI but are not preventing any API call.
     * For instance, even if the Removable flag is not set, the layer can still be removed with the API
     * but the action will not be listed in the legend menu.
     *
     * \see properties()
     *
     * \since QGIS 3.4
     */
    QgsMapLayer::LayerFlags flags() const;

    /**
     * Returns the flags for this layer.
     * \note Flags are options specified by the user used for the UI but are not preventing any API call.
     * For instance, even if the Removable flag is not set, the layer can still be removed with the API
     * but the action will not be listed in the legend menu.
     *
     * \see properties()
     *
     * \since QGIS 3.4
     */
    void setFlags( QgsMapLayer::LayerFlags flags );

    /**
     * Returns the map layer properties of this layer.
     *
     * \note properties() differ from flags() in that flags() are user settable, and reflect options that
     * users can enable for map layers. In contrast properties() are reflections of inherent capabilities
     * for the layer, which cannot be directly changed by users.
     *
     * \since QGIS 3.22
     */
    virtual Qgis::MapLayerProperties properties() const;

    /**
     * Returns the extension of a Property.
     * \returns The extension
     */
    static QString extensionPropertyType( PropertyType type );

    /**
     * Returns the layer's unique ID, which is used to access this layer from QgsProject.
     *
     * \see setId()
     * \see idChanged()
     */
    QString id() const;

    /**
     * Sets the layer's \a id.
     *
     * Returns TRUE if the layer ID was successfully changed, or FALSE if it could not be changed (e.g. because
     * the layer is owned by a QgsProject or QgsMapLayerStore).
     *
     * \warning It is the caller's responsibility to ensure that the layer ID is unique in the desired context.
     * Generally this method should not be called, and the auto generated ID should be used instead.
     *
     * \warning This method should not be called on layers which already belong to a QgsProject or QgsMapLayerStore.
     *
     * \see id()
     * \see idChanged()
     * \since QGIS 3.38
     */
    bool setId( const QString &id );

    /**
     * Set the display \a name of the layer.
     * \see name()
     */
    void setName( const QString &name );

    /**
     * Returns the display name of the layer.
     * \see setName()
     */
    QString name() const;

    /**
     * Returns the layer's data provider, it may be NULLPTR.
     */
    Q_INVOKABLE virtual QgsDataProvider *dataProvider();

    /**
     * Returns the layer's data provider in a const-correct manner, it may be NULLPTR.
     * \note not available in Python bindings
     */
    virtual const QgsDataProvider *dataProvider() const SIP_SKIP;

    /**
     * Sets the short name of the layer used by QGIS Server to identify the layer.
     *
     * \deprecated since QGIS 3.38 use serverProperties()->setShortName() instead.
     */
    Q_DECL_DEPRECATED void setShortName( const QString &shortName ) SIP_DEPRECATED;

    /**
     * Returns the short name of the layer used by QGIS Server to identify the layer.
     *
     * \deprecated since QGIS 3.38 use serverProperties()->shortName() instead.
     */
    Q_DECL_DEPRECATED QString shortName() const SIP_DEPRECATED;

    /**
     * Sets the title of the layer used by QGIS Server in GetCapabilities request.
     *
     * \deprecated since QGIS 3.38 use serverProperties()->setTitle() instead.
     */
    Q_DECL_DEPRECATED void setTitle( const QString &title ) SIP_DEPRECATED;

    /**
     * Returns the title of the layer used by QGIS Server in GetCapabilities request.
     *
     * \deprecated since QGIS 3.38 use serverProperties()->title() instead.
     */
    Q_DECL_DEPRECATED QString title() const SIP_DEPRECATED;

    /**
     * Sets the abstract of the layer used by QGIS Server in GetCapabilities request.
     *
     * \deprecated since QGIS 3.38 use serverProperties()->setAbstract() instead.
     */
    Q_DECL_DEPRECATED void setAbstract( const QString &abstract ) SIP_DEPRECATED;

    /**
     * Returns the abstract of the layer used by QGIS Server in GetCapabilities request.
     *
     * \deprecated since QGIS 3.38 use serverProperties()->abstract() instead.
     */
    Q_DECL_DEPRECATED QString abstract() const SIP_DEPRECATED;

    /**
     * Sets the keyword list of the layerused by QGIS Server in GetCapabilities request.
     *
     * \deprecated since QGIS 3.38 use serverProperties()->setKeywordList() instead.
     */
    Q_DECL_DEPRECATED void setKeywordList( const QString &keywords ) SIP_DEPRECATED;

    /**
     * Returns the keyword list of the layer used by QGIS Server in GetCapabilities request.
     *
     * \deprecated since QGIS 3.38 use serverProperties()->keywordList() instead.
     */
    Q_DECL_DEPRECATED QString keywordList() const SIP_DEPRECATED;

    /**
     * Sets the DataUrl of the layer used by QGIS Server in GetCapabilities request.
     *
     * \deprecated since QGIS 3.38 use serverProperties()->setDataUrl() instead.
     */
    Q_DECL_DEPRECATED void setDataUrl( const QString &dataUrl ) SIP_DEPRECATED;

    /**
     * Returns the DataUrl of the layer used by QGIS Server in GetCapabilities request.
     *
     * \deprecated since QGIS 3.38 use serverProperties()->dataUrl() instead.
     */
    Q_DECL_DEPRECATED QString dataUrl() const SIP_DEPRECATED;

    /**
     * Sets the DataUrl format of the layer used by QGIS Server in GetCapabilities request.
     *
     * \deprecated since QGIS 3.38 use serverProperties()->setDataUrlFormat() instead.
     */
    Q_DECL_DEPRECATED void setDataUrlFormat( const QString &dataUrlFormat ) SIP_DEPRECATED;

    /**
     * Returns the DataUrl format of the layer used by QGIS Server in GetCapabilities request.
     *
     * \deprecated since QGIS 3.38 use serverProperties()->dataUrlFormat() instead.
     */
    Q_DECL_DEPRECATED QString dataUrlFormat() const SIP_DEPRECATED;

    /**
     * Sets the attribution of the layerused by QGIS Server in GetCapabilities request.
     *
     * \deprecated since QGIS 3.38 use serverProperties()->setAttribution() instead.
     */
    Q_DECL_DEPRECATED void setAttribution( const QString &attrib ) SIP_DEPRECATED;

    /**
     * Returns the attribution of the layer used by QGIS Server in GetCapabilities request.
     *
     * \deprecated since QGIS 3.38 use serverProperties()->attribution() instead.
     */
    Q_DECL_DEPRECATED QString attribution() const SIP_DEPRECATED;

    /**
     * Sets the attribution URL of the layer used by QGIS Server in GetCapabilities request.
     *
     * \deprecated since QGIS 3.38 use serverProperties()->setAttributionUrl() instead.
     */
    Q_DECL_DEPRECATED void setAttributionUrl( const QString &attribUrl ) SIP_DEPRECATED;

    /**
     * Returns the attribution URL of the layer used by QGIS Server in GetCapabilities request.
     *
     * \deprecated since QGIS 3.38 use serverProperties()->attributionUrl() instead.
     */
    Q_DECL_DEPRECATED QString attributionUrl() const SIP_DEPRECATED;

    /* Layer metadataUrl information */

    /**
     * Returns QGIS Server Properties for the map layer
     * \since QGIS 3.22
     */
    QgsMapLayerServerProperties *serverProperties() { return mServerProperties.get(); };

    /**
     * Returns QGIS Server Properties const for the map layer
     * \since QGIS 3.22
     */
    const QgsMapLayerServerProperties *serverProperties() const { return mServerProperties.get(); } SIP_SKIP;

    /**
     * Sets the metadata URL of the layer
     *  used by QGIS Server in GetCapabilities request.
     *  MetadataUrl is a a link to the detailed, standardized metadata about the data.
     *  Since QGIS 3.22, it edits the first metadata URL link.
     * \see serverProperties()
     * \deprecated since QGIS 3.22
     */
    Q_DECL_DEPRECATED void setMetadataUrl( const QString &metaUrl ) SIP_DEPRECATED;

    /**
     * Returns the metadata URL of the layer
     *  used by QGIS Server in GetCapabilities request.
     *  MetadataUrl is a a link to the detailed, standardized metadata about the data.
     * Since QGIS 3.22, it returns the first metadata URL link.
     * \returns the layer metadata URL
     * \see serverProperties()
     * \deprecated since QGIS 3.22
     */
    Q_DECL_DEPRECATED QString metadataUrl() const SIP_DEPRECATED;

    /**
     * Set the metadata type of the layer
     *  used by QGIS Server in GetCapabilities request
     *  MetadataUrlType indicates the standard to which the metadata complies.
     *  Since QGIS 3.22, it edits the first metadata URL type.
     * \see serverProperties()
     * \deprecated since QGIS 3.22
     */
    Q_DECL_DEPRECATED void setMetadataUrlType( const QString &metaUrlType ) SIP_DEPRECATED;

    /**
     * Returns the metadata type of the layer
     *  used by QGIS Server in GetCapabilities request.
     *  MetadataUrlType indicates the standard to which the metadata complies.
     * Since QGIS 3.22, it returns the first metadata URL type.
     * \returns the layer metadata type
     * \see serverProperties()
     * \deprecated since QGIS 3.22
     */
    Q_DECL_DEPRECATED QString metadataUrlType() const SIP_DEPRECATED;

    /**
     * Sets the metadata format of the layer
     *  used by QGIS Server in GetCapabilities request.
     *  MetadataUrlType indicates how the metadata is structured.
     *  Since QGIS 3.22, it edits the first metadata URL format.
     * \see serverProperties()
     * \deprecated since QGIS 3.22
     */
    Q_DECL_DEPRECATED void setMetadataUrlFormat( const QString &metaUrlFormat ) SIP_DEPRECATED;

    /**
     * Returns the metadata format of the layer
     *  used by QGIS Server in GetCapabilities request.
     *  MetadataUrlType indicates how the metadata is structured.
     * Since QGIS 3.22, it returns the first metadata URL format.
     * \returns the layer metadata format
     * \see serverProperties()
     * \deprecated since QGIS 3.22
     */
    Q_DECL_DEPRECATED QString metadataUrlFormat() const SIP_DEPRECATED;

    /**
     * Set the blending mode used for rendering a layer.
     * \param blendMode new blending mode
     * \see blendMode()
    */
    void setBlendMode( QPainter::CompositionMode blendMode );

    /**
     * Returns the current blending mode for a layer.
     * \see setBlendMode()
    */
    QPainter::CompositionMode blendMode() const;

    /**
     * Sets the \a opacity for the layer, where \a opacity is a value between 0 (totally transparent)
     * and 1.0 (fully opaque).
     * \see opacity()
     * \see opacityChanged()
     * \note Prior to QGIS 3.18, this method was available for vector layers only
     * \since QGIS 3.18
     */
    virtual void setOpacity( double opacity );

    /**
     * Returns the opacity for the layer, where opacity is a value between 0 (totally transparent)
     * and 1.0 (fully opaque).
     * \see setOpacity()
     * \see opacityChanged()
     * \note Prior to QGIS 3.18, this method was available for vector layers only
     * \since QGIS 3.18
     */
    virtual double opacity() const;

    //! Returns if this layer is read only.
    bool readOnly() const { return isReadOnly(); }

    /**
     * Synchronises with changes in the datasource
     */
    Q_INVOKABLE virtual void reload() {}

    /**
     * Returns new instance of QgsMapLayerRenderer that will be used for rendering of given context
     */
    virtual QgsMapLayerRenderer *createMapRenderer( QgsRenderContext &rendererContext ) = 0 SIP_FACTORY;

    //! Returns the extent of the layer.
    virtual QgsRectangle extent() const;

    /**
     * Returns the 3D extent of the layer.
     * \since QGIS 3.36
     */
    virtual QgsBox3D extent3D() const;

    /**
     * Returns the WGS84 extent (EPSG:4326) of the layer according to
     * ReadFlag::FlagTrustLayerMetadata. If that flag is activated, then the
     * WGS84 extent read in the qgs project is returned. Otherwise, the actual
     * WGS84 extent is returned.
     * \param forceRecalculate True to return the current WGS84 extent whatever the read flags
     * \since QGIS 3.20
     */
    QgsRectangle wgs84Extent( bool forceRecalculate = false ) const;

    /**
     * Returns the status of the layer. An invalid layer is one which has a bad datasource
     * or other problem. Child classes set this flag when initialized.
     * \returns TRUE if the layer is valid and can be accessed
     */
    bool isValid() const;

    /**
     * Gets a version of the internal layer definition that has sensitive
      *  bits removed (for example, the password). This function should
      * be used when displaying the source name for general viewing.
      * \param hidePassword False, if the password should be removed or replaced by an arbitrary string, since QGIS 3.34
      * \see source()
     */
    QString publicSource( bool hidePassword = false ) const;

    /**
     * Returns the source for the layer. This source may contain usernames, passwords
     * and other sensitive information.
     * \see publicSource()
     */
    QString source() const;

    /**
     * Returns the sublayers of this layer.
     * (Useful for providers that manage their own layers, such as WMS).
     */
    virtual QStringList subLayers() const;

    /**
     * Reorders the *previously selected* sublayers of this layer from bottom to top.
     * (Useful for providers that manage their own layers, such as WMS).
     */
    virtual void setLayerOrder( const QStringList &layers );

    /**
     * Set the visibility of the given sublayer name.
     * \param name sublayer name
     * \param visible sublayer visibility
    */
    virtual void setSubLayerVisibility( const QString &name, bool visible );

    /**
     * Returns whether the layer supports editing or not.
     * \returns FALSE if the layer is read only or the data provider has no editing capabilities.
     * \note default implementation returns FALSE.
     * \since QGIS 3.22 in the base class QgsMapLayer.
     */
    virtual bool supportsEditing() const;

    //! Returns TRUE if the layer can be edited.
    virtual bool isEditable() const;

    /**
     * Returns TRUE if the layer has been modified since last commit/save.
     * \note default implementation returns FALSE.
     * \since QGIS 3.22 in the base class QgsMapLayer.
     */
    virtual bool isModified() const;

    /**
     * Returns TRUE if the layer is considered a spatial layer, ie it has some form of geometry associated with it.
     */
    virtual bool isSpatial() const;

    /**
     * Returns TRUE if the layer is considered a temporary layer.
     *
     * These include memory-only layers such as those created by the "memory" data provider, layers
     * stored inside a local temporary folder (such as the "/tmp" folder on Linux)
     * or layer with temporary data (as temporary mesh layer dataset)
     *
     * \since QGIS 3.10.1
     */
    virtual bool isTemporary() const;

    /**
     * Flags which control project read behavior.
     * \since QGIS 3.10
     */
    enum ReadFlag SIP_ENUM_BASETYPE( IntFlag )
    {
      FlagDontResolveLayers = 1 << 0, //!< Don't resolve layer paths or create data providers for layers.
      FlagTrustLayerMetadata = 1 << 1, //!< Trust layer metadata. Improves layer load time by skipping expensive checks like primary key unicity, geometry type and srid and by using estimated metadata on layer load. Since QGIS 3.16
      FlagReadExtentFromXml = 1 << 2, //!< Read extent from xml and skip get extent from provider.
      FlagForceReadOnly = 1 << 3, //!< Force open as read only.
    };
    Q_DECLARE_FLAGS( ReadFlags, ReadFlag )

    /**
     * Sets state from DOM document
     * \param layerElement The DOM element corresponding to ``maplayer'' tag
     * \param context writing context (e.g. for conversion between relative and absolute paths)
     * \param flags optional argument which can be used to control layer reading behavior.
     * \param preloadedProvider optional preloaded data provider that will be used as data provider for this layer, takes ownership (since QGIS 3.32)
     * \note
     *
     * The DOM node corresponds to a DOM document project file XML element read
     * by QgsProject.
     *
     * This, in turn, calls readXml() (and then readSymbology()), which is overridable
     * by sub-classes so that they can read their own specific state from the given DOM node.
     *
     * Invoked by QgsProject::read().
     *
     * \returns TRUE if successful
     */
    bool readLayerXml( const QDomElement &layerElement, QgsReadWriteContext &context,
                       QgsMapLayer::ReadFlags flags = QgsMapLayer::ReadFlags(), QgsDataProvider *preloadedProvider SIP_TRANSFER = nullptr );

    /**
     * Stores state in DOM node
     * \param layerElement is a DOM element corresponding to ``maplayer'' tag
     * \param document is a the DOM document being written
     * \param context reading context (e.g. for conversion between relative and absolute paths)
     * \note
     *
     * The DOM node corresponds to a DOM document project file XML element to be
     * written by QgsProject.
     *
     * This, in turn, calls writeXml() (and then writeSymbology), which is over-rideable
     * by sub-classes so that they can write their own specific state to the given DOM node.
     *
     * Invoked by QgsProject::write().
     *
     * \returns TRUE if successful
     */
    bool writeLayerXml( QDomElement &layerElement, QDomDocument &document, const QgsReadWriteContext &context ) const;

    /**
     * Resolve references to other layers (kept as layer IDs after reading XML) into layer objects.
     */
    virtual void resolveReferences( QgsProject *project );

    /**
     * Returns list of all keys within custom properties. Properties are stored in a map and saved in project file.
     * \see customProperty()
     */
    Q_INVOKABLE QStringList customPropertyKeys() const;

    /**
     * Set a custom property for layer. Properties are stored in a map and saved in project file.
     * \see customProperty()
     * \see removeCustomProperty()
     */
    Q_INVOKABLE void setCustomProperty( const QString &key, const QVariant &value );

    /**
     * Read a custom property from layer. Properties are stored in a map and saved in project file.
     * \see setCustomProperty()
    */
    Q_INVOKABLE QVariant customProperty( const QString &value, const QVariant &defaultValue = QVariant() ) const;

    /**
     * Set custom properties for layer. Current properties are dropped.
     */
    void setCustomProperties( const QgsObjectCustomProperties &properties );

    /**
     * Read all custom properties from layer. Properties are stored in a map and saved in project file.
     * \see setCustomProperties
     * \since QGIS 3.14
     */
    const QgsObjectCustomProperties &customProperties() const;

    /**
     * Lists all the style in db split into related to the layer and not related to
     * \param ids the list in which will be stored the style db ids
     * \param names the list in which will be stored the style names
     * \param descriptions the list in which will be stored the style descriptions
     * \param msgError will be set to a descriptive error message if any occurs
     * \returns the number of styles related to current layer (-1 on not implemented)
     * \note Since QGIS 3.2 Styles related to the layer are ordered with the default style first then by update time for Postgres, MySQL and Spatialite.
     */
    virtual int listStylesInDatabase( QStringList &ids SIP_OUT, QStringList &names SIP_OUT,
                                      QStringList &descriptions SIP_OUT, QString &msgError SIP_OUT );

    /**
     * Returns the named style corresponding to style id provided
     */
    virtual QString getStyleFromDatabase( const QString &styleId, QString &msgError SIP_OUT );

    /**
     * Deletes a style from the database
     * \param styleId the provider's layer_styles table id of the style to delete
     * \param msgError will be set to a descriptive error message if any occurs
     * \returns TRUE in case of success
     */
    virtual bool deleteStyleFromDatabase( const QString &styleId, QString &msgError SIP_OUT );

    /**
     * Saves named and sld style of the layer to the style table in the db.
     * \param name Style name
     * \param description A description of the style
     * \param useAsDefault Set to TRUE if style should be used as the default style for the layer
     * \param uiFileContent
     * \param msgError will be set to a descriptive error message if any occurs
     * \param categories the style categories to be saved.
     *
     *
     * \note Prior to QGIS 3.24, this method would show a message box warning when a
     * style with the same \a styleName already existed to confirm replacing the style with the user.
     * Since 3.24, calling this method will ALWAYS overwrite any existing style with the same name.
     * Use QgsProviderRegistry::styleExists() to test in advance if a style already exists and handle this appropriately
     * in your client code.
     */
    virtual void saveStyleToDatabase( const QString &name, const QString &description,
                                      bool useAsDefault, const QString &uiFileContent,
                                      QString &msgError SIP_OUT,
                                      QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories );


    // TODO QGIS 4.0 -- fix this. We incorrectly have a single boolean flag which in which false is used inconsistently for "a style WAS found but an error occurred loading it" vs "no style was found".
    // The first (style found, error occurred loading it) should trigger a user-facing warning, whereas the second (no style found) isn't reflective of an error at all.

    /**
     * Loads a named style from file/local db/datasource db
     * \param theURI the URI of the style or the URI of the layer
     * \param resultFlag will be set to TRUE if a named style is correctly loaded
     * \param loadFromLocalDb if TRUE forces to load from local db instead of datasource one
     * \param categories the style categories to be loaded.
     * \param flags flags controlling how the style should be loaded (since QGIS 3.38)
     */
    virtual QString loadNamedStyle( const QString &theURI, bool &resultFlag SIP_OUT, bool loadFromLocalDb,
                                    QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories,
                                    Qgis::LoadStyleFlags flags = Qgis::LoadStyleFlags() );

#ifndef SIP_RUN

    /**
     * Returns the property value for a property based on an enum.
     * This forces the output to be a valid and existing entry of the enum.
     * Hence if the property value is incorrect, the given default value is returned.
     * This tries first with property as a string (as the enum) and then as an integer value.
     * \note The enum needs to be declared with Q_ENUM, and flags with Q_FLAG (not Q_FLAGS).
     * \see setCustomEnumProperty
     * \see customFlagProperty
     * \since QGIS 3.22
     */
    template <class T>
    T customEnumProperty( const QString &key, const T &defaultValue )
    {
      const QMetaEnum metaEnum = QMetaEnum::fromType<T>();
      Q_ASSERT( metaEnum.isValid() );
      if ( !metaEnum.isValid() )
      {
        QgsDebugError( QStringLiteral( "Invalid metaenum. Enum probably misses Q_ENUM or Q_FLAG declaration." ) );
      }

      T v;
      bool ok = false;

      if ( metaEnum.isValid() )
      {
        // read as string
        QByteArray ba = customProperty( key, metaEnum.valueToKey( static_cast<int>( defaultValue ) ) ).toString().toUtf8();
        const char *vs = ba.data();
        v = static_cast<T>( metaEnum.keyToValue( vs, &ok ) );
        if ( ok )
          return v;
      }

      // if failed, try to read as int (old behavior)
      // this code shall be removed later
      // then the method could be marked as const
      v = static_cast<T>( customProperty( key, static_cast<int>( defaultValue ) ).toInt( &ok ) );
      if ( metaEnum.isValid() )
      {
        if ( !ok || !metaEnum.valueToKey( static_cast<int>( v ) ) )
        {
          v = defaultValue;
        }
        else
        {
          // found property as an integer
          // convert the property to the new form (string)
          setCustomEnumProperty( key, v );
        }
      }

      return v;
    }

    /**
     * Set the value of a property based on an enum.
     * The property will be saved as string.
     * \note The enum needs to be declared with Q_ENUM, and flags with Q_FLAG (not Q_FLAGS).
     * \see customEnumProperty
     * \see setCustomFlagProperty
     * \since QGIS 3.22
     */
    template <class T>
    void setCustomEnumProperty( const QString &key, const T &value )
    {
      const QMetaEnum metaEnum = QMetaEnum::fromType<T>();
      Q_ASSERT( metaEnum.isValid() );
      if ( metaEnum.isValid() )
      {
        setCustomProperty( key, metaEnum.valueToKey( static_cast<int>( value ) ) );
      }
      else
      {
        QgsDebugError( QStringLiteral( "Invalid metaenum. Enum probably misses Q_ENUM or Q_FLAG declaration." ) );
      }
    }

    /**
     * Returns the property value for a property based on a flag.
     * This forces the output to be a valid and existing entry of the flag.
     * Hence if the property value is incorrect, the given default value is returned.
     * This tries first with property as a string (using a byte array) and then as an integer value.
     * \note The flag needs to be declared with Q_FLAG (not Q_FLAGS).
     * \note for Python bindings, a custom implementation is achieved in Python directly.
     * \see setCustomFlagProperty
     * \see customEnumProperty
     * \since QGIS 3.22
     */
    template <class T>
    T customFlagProperty( const QString &key, const T &defaultValue )
    {
      const QMetaEnum metaEnum = QMetaEnum::fromType<T>();
      Q_ASSERT( metaEnum.isValid() );
      if ( !metaEnum.isValid() )
      {
        QgsDebugError( QStringLiteral( "Invalid metaenum. Enum probably misses Q_ENUM or Q_FLAG declaration." ) );
      }

      T v = defaultValue;
      bool ok = false;

      if ( metaEnum.isValid() )
      {
        // read as string
        QByteArray ba = customProperty( key, metaEnum.valueToKeys( defaultValue ) ).toString().toUtf8();
        const char *vs = ba.data();
        v = static_cast<T>( metaEnum.keysToValue( vs, &ok ) );
      }
      if ( !ok )
      {
        // if failed, try to read as int
        const int intValue = customProperty( key, static_cast<int>( defaultValue ) ).toInt( &ok );
        if ( metaEnum.isValid() )
        {
          if ( ok )
          {
            // check that the int value does correspond to a flag
            // see https://stackoverflow.com/a/68495949/1548052
            const QByteArray keys = metaEnum.valueToKeys( intValue );
            const int intValueCheck = metaEnum.keysToValue( keys );
            if ( intValue != intValueCheck )
            {
              v = defaultValue;
            }
            else
            {
              // found property as an integer
              v = T( intValue );
              // convert the property to the new form (string)
              // this code could be removed
              // then the method could be marked as const
              setCustomFlagProperty( key, v );
            }
          }
          else
          {
            v = defaultValue;
          }
        }
      }

      return v;
    }

    /**
     * Set the value of a property based on a flag.
     * The property will be saved as string.
     * \note The flag needs to be declared with Q_FLAG (not Q_FLAGS).
     * \see customFlagProperty
     * \see customEnumProperty
     * \since QGIS 3.22
     */
    template <class T>
    void setCustomFlagProperty( const QString &key, const T &value )
    {
      const QMetaEnum metaEnum = QMetaEnum::fromType<T>();
      Q_ASSERT( metaEnum.isValid() );
      if ( metaEnum.isValid() )
      {
        setCustomProperty( key, metaEnum.valueToKeys( value ) );
      }
      else
      {
        QgsDebugError( QStringLiteral( "Invalid metaenum. Enum probably misses Q_ENUM or Q_FLAG declaration." ) );
      }
    }
#endif


    /**
     * Remove a custom property from layer. Properties are stored in a map and saved in project file.
     * \see setCustomProperty()
     */
    void removeCustomProperty( const QString &key );

    /**
     * Gets current status error. This error describes some principal problem
     *  for which layer cannot work and thus is not valid. It is not last error
     *  after accessing data by draw() etc.
     */
    virtual QgsError error() const;

    /**
     * Returns the layer's spatial reference system.
     *
     * \warning Since QGIS 3.38, consider using crs3D() whenever transforming 3D data or whenever
     * z/elevation value handling is important.
     *
     * \see setCrs()
     * \see crs3D()
     * \see verticalCrs()
     * \see crsChanged()
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Returns the layer's vertical coordinate reference system.
     *
     * If the layer crs() is a compound CRS, then the CRS returned will
     * be the vertical component of crs(). Otherwise it will be the value
     * explicitly set by a call to setVerticalCrs().
     *
     * The returned CRS will be invalid if the layer has no vertical CRS.
     *
     * \note Consider also using crs3D(), which will return a CRS which takes into account
     * both crs() and verticalCrs().
     *
     * \see crs()
     * \see crs3D()
     * \see setVerticalCrs()
     *
     * \since QGIS 3.38
     */
    QgsCoordinateReferenceSystem verticalCrs() const;

    /**
     * Returns the CRS to use for the layer when transforming 3D data, or when z/elevation
     * value handling is important.
     *
     * The returned CRS will take into account verticalCrs() when appropriate, e.g. it may return a compound
     * CRS consisting of crs() + verticalCrs(). This method may still return a 2D CRS, e.g in the
     * case that crs() is a 2D CRS and no verticalCrs() has been set for the layer. Check QgsCoordinateReferenceSystem::type()
     * on the returned value to determine the type of CRS returned by this method.
     *
     * \warning It is NOT guaranteed that the returned CRS will actually be a 3D CRS, but rather
     * it is guaranteed that the returned CRS is ALWAYS the most appropriate CRS to use when handling 3D data.
     *
     * \see crs()
     * \see verticalCrs()
     * \see crs3DChanged()
     *
     * \since QGIS 3.38
     */
    QgsCoordinateReferenceSystem crs3D() const;

    /**
     * Sets layer's spatial reference system.
     *
     * If \a emitSignal is TRUE, changing the CRS will trigger a crsChanged() signal. Additionally, if \a crs is a compound
     * CRS, then the verticalCrsChanged() signal will also be emitted.
     *
     * \see crs()
     * \see crsChanged()
     * \see setVerticalCrs()
     */
    void setCrs( const QgsCoordinateReferenceSystem &srs, bool emitSignal = true );

    /**
     * Sets the layer's vertical coordinate reference system.
     *
     * The verticalCrsChanged() signal will be raised if the vertical CRS is changed.
     *
     * \note If the layer crs() is a compound CRS, then the CRS returned for
     * verticalCrs() will be the vertical component of crs(). Otherwise it will be the value
     * explicitly set by this call.
     *
     * \param crs the vertical CRS
     * \param errorMessage will be set to a descriptive message if the vertical CRS could not be set
     *
     * \returns TRUE if vertical CRS was successfully set
     *
     * \see verticalCrs()
     * \see setCrs()
     *
     * \since QGIS 3.38
     */
    bool setVerticalCrs( const QgsCoordinateReferenceSystem &crs, QString *errorMessage SIP_OUT = nullptr );

    /**
     * Returns the layer data provider coordinate transform context
     * or a default transform context if the layer does not have a valid data provider.
    * \since QGIS 3.8
     */
    QgsCoordinateTransformContext transformContext( ) const;


    /**
     * A convenience function to capitalize and format a layer \a name.
     *
     */
    static QString formatLayerName( const QString &name );

    /**
     * Retrieve the metadata URI for this layer
     * (either as a .qmd file on disk or as a
     * record in the users style table in their personal qgis.db)
     * \returns a QString with the metadata file name
     */
    virtual QString metadataUri() const;

    /**
     * Export the current metadata of this layer as named metadata in a QDomDocument
     * \param doc the target QDomDocument
     * \param errorMsg this QString will be initialized on error
     */
    void exportNamedMetadata( QDomDocument &doc, QString &errorMsg ) const;

    /**
     * Save the current metadata of this layer as the default metadata
     * (either as a .qmd file on disk or as a
     * record in the users style table in their personal qgis.db)
     * \param resultFlag a reference to a flag that will be set to FALSE if
     * we did not manage to save the default metadata.
     * \returns a QString with any status messages
     */
    virtual QString saveDefaultMetadata( bool &resultFlag SIP_OUT );

    /**
     * Save the current metadata of this layer as a named metadata
     * (either as a .qmd file on disk or as a
     * record in the users style table in their personal qgis.db)
     * \param uri the file name or other URI for the
     * metadata file. First an attempt will be made to see if this
     * is a file and save to that, if that fails the qgis.db metadata
     * table will be used to create a metadata entry who's
     * key matches the URI.
     * \param resultFlag a reference to a flag that will be set to FALSE if
     * we did not manage to save the default metadata.
     * \returns a QString with any status messages
     */
    QString saveNamedMetadata( const QString &uri, bool &resultFlag );

    // TODO QGIS 4.0 -- fix this. We incorrectly have a single boolean flag which in which false is used inconsistently for "metadata WAS found but an error occurred loading it" vs "no metadata was found".
    // The first (metadata found, error occurred loading it) should trigger a user-facing warning, whereas the second (no metadata found) isn't reflective of an error at all.

    /**
     * Retrieve a named metadata for this layer if one
     * exists (either as a .qmd file on disk or as a
     * record in the users style table in their personal qgis.db)
     * \param uri - the file name or other URI for the
     * metadata file. First an attempt will be made to see if this
     * is a file and load that, if that fails the qgis.db metadata
     * table will be consulted to see if there is a metadata who's
     * key matches the URI.
     * \param resultFlag a reference to a flag that will be set to FALSE if
     * we did not manage to load the default metadata.
     * \returns a QString with any status messages
     */
    virtual QString loadNamedMetadata( const QString &uri, bool &resultFlag SIP_OUT );

    // TODO QGIS 4.0 -- fix this. We incorrectly have a single boolean flag which in which false is used inconsistently for "metadata WAS found but an error occurred loading it" vs "no metadata was found".
    // The first (metadata found, error occurred loading it) should trigger a user-facing warning, whereas the second (no metadata found) isn't reflective of an error at all.

    /**
     * Retrieve the default metadata for this layer if one
     * exists (either as a .qmd file on disk or as a
     * record in the users metadata table in their personal qgis.db)
     * \param resultFlag a reference to a flag that will be set to FALSE if
     * we did not manage to load the default metadata.
     * \returns a QString with any status messages
     */
    virtual QString loadDefaultMetadata( bool &resultFlag );

    /**
     * Retrieve a named metadata for this layer from a sqlite database.
     * \param db path to sqlite database
     * \param uri uri for table
     * \param qmd will be set to QMD xml metadata content from database
     * \returns TRUE if style was successfully loaded
     */
    bool loadNamedMetadataFromDatabase( const QString &db, const QString &uri, QString &qmd );

    /**
     * Import the metadata of this layer from a QDomDocument
     * \param document source QDomDocument
     * \param errorMessage this QString will be initialized on error
     * \returns TRUE on success
     */
    bool importNamedMetadata( QDomDocument &document, QString &errorMessage );

    /**
     * Retrieve the style URI for this layer
     * (either as a .qml file on disk or as a
     * record in the users style table in their personal qgis.db)
     * \returns a QString with the style file name
     * \see loadNamedStyle()
     * \see saveNamedStyle()
     */
    virtual QString styleURI() const;

    // TODO QGIS 4.0 -- fix this. We incorrectly have a single boolean flag which in which false is used inconsistently for "a style WAS found but an error occurred loading it" vs "no style was found".
    // The first (style found, error occurred loading it) should trigger a user-facing warning, whereas the second (no style found) isn't reflective of an error at all.

    /**
     * Retrieve the default style for this layer if one
     * exists (either as a .qml file on disk or as a
     * record in the users style table in their personal qgis.db)
     * \param resultFlag a reference to a flag that will be set to FALSE if
     * we did not manage to load the default style.
     * \returns a QString with any status messages
     * \see loadNamedStyle()
     */
    virtual QString loadDefaultStyle( bool &resultFlag SIP_OUT );

    // TODO QGIS 4.0 -- fix this. We incorrectly have a single boolean flag which in which false is used inconsistently for "a style WAS found but an error occurred loading it" vs "no style was found".
    // The first (style found, error occurred loading it) should trigger a user-facing warning, whereas the second (no style found) isn't reflective of an error at all.

    /**
     * Retrieve a named style for this layer if one
     * exists (either as a .qml file on disk or as a
     * record in the users style table in their personal qgis.db)
     * \param uri - the file name or other URI for the
     * style file. First an attempt will be made to see if this
     * is a file and load that, if that fails the qgis.db styles
     * table will be consulted to see if there is a style who's
     * key matches the URI.
     * \param resultFlag a reference to a flag that will be set to FALSE if
     * we did not manage to load the default style.
     * \param categories the style categories to be loaded.
     * \param flags flags controlling how the style should be loaded (since QGIS 3.38)
     * \returns a QString with any status messages
     * \see loadDefaultStyle()
     */
    virtual QString loadNamedStyle( const QString &uri, bool &resultFlag SIP_OUT, QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories, Qgis::LoadStyleFlags flags = Qgis::LoadStyleFlags() );

    /**
     * Retrieve a named style for this layer from a sqlite database.
     * \param db path to sqlite database
     * \param uri uri for table
     * \param qml will be set to QML style content from database
     * \returns TRUE if style was successfully loaded
     */
    virtual bool loadNamedStyleFromDatabase( const QString &db, const QString &uri, QString &qml SIP_OUT );

    /**
     * Import the properties of this layer from a QDomDocument
     * \param doc source QDomDocument
     * \param errorMsg this QString will be initialized on error
     * during the execution of readSymbology
     * \param categories the style categories to import
     * \returns TRUE on success
     */
    virtual bool importNamedStyle( QDomDocument &doc, QString &errorMsg SIP_OUT,
                                   QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories );

    /**
     * Export the properties of this layer as named style in a QDomDocument
     * \param doc the target QDomDocument
     * \param errorMsg this QString will be initialized on error
     * \param context read write context
     * \param categories the style categories to export
     * during the execution of writeSymbology
     */
    virtual void exportNamedStyle( QDomDocument &doc, QString &errorMsg SIP_OUT, const QgsReadWriteContext &context = QgsReadWriteContext(),
                                   QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories ) const;


    /**
     * Export the properties of this layer as SLD style in a QDomDocument
     * \param doc the target QDomDocument
     * \param errorMsg this QString will be initialized on error
     * during the execution of writeSymbology
     * \see exportSldStyleV2()
     */
    virtual void exportSldStyle( QDomDocument &doc, QString &errorMsg ) const;

    /**
     * Export the properties of this layer as SLD style in a QDomDocument
     * \param doc the target QDomDocument
     * \param errorMsg this QString will be initialized on error
     *                 during the execution of writeSymbology
     * \param exportContext SLD export context
     * \since QGIS 3.30
     */
    virtual void exportSldStyleV2( QDomDocument &doc, QString &errorMsg, const QgsSldExportContext &exportContext ) const;

    /**
     * Save the properties of this layer as the default style
     * (either as a .qml file on disk or as a
     * record in the users style table in their personal qgis.db)
     * \param resultFlag a reference to a flag that will be set to FALSE if
     * we did not manage to save the default style.
     * \param categories the style categories to be saved (since QGIS 3.26)
     * \returns a QString with any status messages
     * \see loadNamedStyle()
     * \see saveNamedStyle()
     */
    virtual QString saveDefaultStyle( bool &resultFlag SIP_OUT, StyleCategories categories );

    /**
     * Save the properties of this layer as the default style
     * (either as a .qml file on disk or as a
     * record in the users style table in their personal qgis.db)
     * \param resultFlag a reference to a flag that will be set to FALSE if
     * we did not manage to save the default style.
     * \returns a QString with any status messages
     * \see loadNamedStyle()
     * \see saveNamedStyle()
     * \deprecated since QGIS 3.26
     */
    Q_DECL_DEPRECATED virtual QString saveDefaultStyle( bool &resultFlag SIP_OUT ) SIP_DEPRECATED;

    /**
     * Save the properties of this layer as a named style
     * (either as a .qml file on disk or as a
     * record in the users style table in their personal qgis.db)
     * \param uri the file name or other URI for the
     * style file. First an attempt will be made to see if this
     * is a file and save to that, if that fails the qgis.db styles
     * table will be used to create a style entry who's
     * key matches the URI.
     * \param resultFlag a reference to a flag that will be set to FALSE if
     * we did not manage to save the default style.
     * \param categories the style categories to be saved.
     * \returns a QString with any status messages
     * \see saveDefaultStyle()
     */
    virtual QString saveNamedStyle( const QString &uri, bool &resultFlag SIP_OUT, StyleCategories categories = AllStyleCategories );

    /**
     * Saves the properties of this layer to an SLD format file.
     * \param uri uri of destination for exported SLD file.
     * \param resultFlag a reference to a flag that will be set to FALSE if
     * the SLD file could not be generated
     * \returns a string with any status or error messages
     * \see loadSldStyle()
     * \see saveSldStyleV2()
     */
    virtual QString saveSldStyle( const QString &uri, bool &resultFlag ) const;

    /**
     * Saves the properties of this layer to an SLD format file.
     * \param resultFlag a reference to a flag that will be set to FALSE if
     *        the SLD file could not be generated
     * \param exportContext SLD export context
     * \returns a string with any status or error messages
     *
     * \see loadSldStyle()
     * \since QGIS 3.30
     */
    virtual QString saveSldStyleV2( bool &resultFlag SIP_OUT, const QgsSldExportContext &exportContext ) const;

    /**
     * Attempts to style the layer using the formatting from an SLD type file.
     * \param uri uri of source SLD file
     * \param resultFlag a reference to a flag that will be set to FALSE if
     * the SLD file could not be loaded
     * \returns a string with any status or error messages
     * \see saveSldStyle()
     */
    virtual QString loadSldStyle( const QString &uri, bool &resultFlag );

    virtual bool readSld( const QDomNode &node, QString &errorMessage )
    { Q_UNUSED( node ) errorMessage = QStringLiteral( "Layer type %1 not supported" ).arg( static_cast<int>( type() ) ); return false; }



    /**
     * Read the symbology for the current layer from the DOM node supplied.
     * \param node node that will contain the symbology definition for this layer.
     * \param errorMessage reference to string that will be updated with any error messages
     * \param context reading context (used for transform from relative to absolute paths)
     * \param categories the style categories to be read
     * \returns TRUE in case of success.
     */
    virtual bool readSymbology( const QDomNode &node, QString &errorMessage,
                                QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories ) = 0;

    /**
     * Read the style for the current layer from the DOM node supplied.
     * \param node node that will contain the style definition for this layer.
     * \param errorMessage reference to string that will be updated with any error messages
     * \param context reading context (used for transform from relative to absolute paths)
     * \param categories the style categories to be read
     * \returns TRUE in case of success.
     * \note To be implemented in subclasses. Default implementation does nothing and returns FALSE.
     */
    virtual bool readStyle( const QDomNode &node, QString &errorMessage,
                            QgsReadWriteContext &context, StyleCategories categories = AllStyleCategories );

    /**
     * Write the style for the layer into the document provided.
     *  \param node the node that will have the style element added to it.
     *  \param doc the document that will have the QDomNode added.
     *  \param errorMessage reference to string that will be updated with any error messages
     *  \param context writing context (used for transform from absolute to relative paths)
     *  \param categories the style categories to be written
     *  \note There is a confusion of terms with the GUI. This method actually writes what is called a style in the application.
     *  \returns TRUE in case of success.
     */
    virtual bool writeSymbology( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context,
                                 StyleCategories categories = AllStyleCategories ) const = 0;

    /**
     * Write just the symbology information for the layer into the document
     *  \param node the node that will have the style element added to it.
     *  \param doc the document that will have the QDomNode added.
     *  \param errorMessage reference to string that will be updated with any error messages
     *  \param context writing context (used for transform from absolute to relative paths)
     *  \param categories the style categories to be written
     *  \returns TRUE in case of success.
     *  \note To be implemented in subclasses. Default implementation does nothing and returns FALSE.
     *  \note There is a confusion of terms with the GUI. This method actually writes what is known as the symbology in the application.
     */
    virtual bool writeStyle( QDomNode &node, QDomDocument &doc, QString &errorMessage, const QgsReadWriteContext &context,
                             StyleCategories categories = AllStyleCategories ) const;


    /**
     * Updates the data source of the layer.
     *
     * The \a dataSource argument must specify the new data source string for the layer. The format varies depending on
     * the specified data \a provider in use. See QgsDataSourceUri and the documentation for the various QgsMapLayer
     * subclasses for further details on data source strings.
     *
     * The \a baseName argument specifies the user-visible name to use for the layer. (See name() or setName()).
     *
     * The \a provider argument is used to specify the unique key of the data provider to use for
     * the layer. This must match one of the values returned by QgsProviderRegistry::instance()->providerList().
     * (See providerType()).
     *
     * If \a loadDefaultStyleFlag is set to TRUE then the layer's existing style will be reset to the default
     * for the data source.
     *
     * \note If \a loadDefaultStyleFlag is FALSE then the layer's renderer and legend will be preserved only
     * if the geometry type of the new data source matches the current geometry type of the layer.
     *
     * After setting a new data source callers can test isValid() to determine whether the new source
     * and provider are valid and ready for use. If setting the new data source fails and the layer
     * returns FALSE to isValid(), then descriptive errors relating to setting the data source can
     * be retrieved by calling error().
     *
     * This method was defined in QgsVectorLayer since 2.10 and was marked as deprecated since 3.2
     *
     * \see dataSourceChanged()
     * \since QGIS 3.20
     */
    void setDataSource( const QString &dataSource, const QString &baseName, const QString &provider, bool loadDefaultStyleFlag = false );

    /**
     * Updates the data source of the layer.
     *
     * The \a dataSource argument must specify the new data source string for the layer. The format varies depending on
     * the specified data \a provider in use. See QgsDataSourceUri and the documentation for the various QgsMapLayer
     * subclasses for further details on data source strings.
     *
     * The \a baseName argument specifies the user-visible name to use for the layer. (See name() or setName()).
     *
     * The \a provider argument is used to specify the unique key of the data provider to use for
     * the layer. This must match one of the values returned by QgsProviderRegistry::instance()->providerList().
     * (See providerType()).
     *
     * The \a options argument can be used to pass additional layer properties to the new data provider.
     *
     * If \a loadDefaultStyleFlag is set to TRUE then the layer's existing style will be reset to the default
     * for the data source.
     *
     * \note If \a loadDefaultStyleFlag is FALSE then the layer's renderer and legend will be preserved only
     * if the geometry type of the new data source matches the current geometry type of the layer.
     *
     * After setting a new data source callers can test isValid() to determine whether the new source
     * and provider are valid and ready for use. If setting the new data source fails and the layer
     * returns FALSE to isValid(), then descriptive errors relating to setting the data source can
     * be retrieved by calling error().
     *
     * \see dataSourceChanged()
     * \since QGIS 3.6
     */
    void setDataSource( const QString &dataSource, const QString &baseName, const QString &provider, const QgsDataProvider::ProviderOptions &options, bool loadDefaultStyleFlag = false );

    /**
     * Updates the data source of the layer.
     *
     * The \a dataSource argument must specify the new data source string for the layer. The format varies depending on
     * the specified data \a provider in use. See QgsDataSourceUri and the documentation for the various QgsMapLayer
     * subclasses for further details on data source strings.
     *
     * The \a baseName argument specifies the user-visible name to use for the layer. (See name() or setName()).
     *
     * The \a provider argument is used to specify the unique key of the data provider to use for
     * the layer. This must match one of the values returned by QgsProviderRegistry::instance()->providerList().
     * (See providerType()).
     *
     * The \a options argument can be used to pass additional layer properties to the new data provider.
     *
     * The \a flags argument specifies provider read flags which control the data provider construction,
     * such as QgsDataProvider::ReadFlag::FlagTrustDataSource, QgsDataProvider::ReadFlag::FlagLoadDefaultStyle, etc.
     *
     * \note The layer's renderer and legend will be preserved only
     * if the geometry type of the new data source matches the current geometry type of the layer.
     *
     * After setting a new data source callers can test isValid() to determine whether the new source
     * and provider are valid and ready for use. If setting the new data source fails and the layer
     * returns FALSE to isValid(), then descriptive errors relating to setting the data source can
     * be retrieved by calling error().
     *
     * \see dataSourceChanged()
     * \since QGIS 3.20
     */
    void setDataSource( const QString &dataSource, const QString &baseName, const QString &provider, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags );

    /**
     * Returns the provider type (provider key) for this layer
     */
    QString providerType() const;

    //! Returns pointer to layer's undo stack
    QUndoStack *undoStack();

    /**
     * Returns pointer to layer's style undo stack
     */
    QUndoStack *undoStackStyles();

    /**
     * Sets the URL for the layer's legend.
    */
    void setLegendUrl( const QString &legendUrl ) { mLegendUrl = legendUrl; }

    /**
     * Returns the URL for the layer's legend.
     */
    QString legendUrl() const { return mLegendUrl; }

    /**
     * Sets the format for a URL based layer legend.
     */
    void setLegendUrlFormat( const QString &legendUrlFormat ) { mLegendUrlFormat = legendUrlFormat; }

    /**
     * Returns the format for a URL based layer legend.
     */
    QString legendUrlFormat() const { return mLegendUrlFormat; }

    /**
     * Assign a legend controller to the map layer. The object will be responsible for providing legend items.
     * \param legend Takes ownership of the object. Can be NULLPTR.
     */
    void setLegend( QgsMapLayerLegend *legend SIP_TRANSFER );

    /**
     * Can be NULLPTR.
     */
    QgsMapLayerLegend *legend() const;

    /**
     * Gets access to the layer's style manager. Style manager allows switching between multiple styles.
     */
    QgsMapLayerStyleManager *styleManager() const;

    /**
     * Sets 3D renderer for the layer. Takes ownership of the renderer.
     */
    void setRenderer3D( QgsAbstract3DRenderer *renderer SIP_TRANSFER );

    /**
     * Returns 3D renderer associated with the layer. May be NULLPTR.
     */
    QgsAbstract3DRenderer *renderer3D() const;

    /**
     * Tests whether the layer should be visible at the specified \a scale.
     *  The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \returns TRUE if the layer is visible at the given scale.
     * \see minimumScale()
     * \see maximumScale()
     * \see hasScaleBasedVisibility()
     */
    bool isInScaleRange( double scale ) const;

    /**
     * Returns the minimum map scale (i.e. most "zoomed out" scale) at which the layer will be visible.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * A scale of 0 indicates no minimum scale visibility.
     * \note Scale based visibility is only used if setScaleBasedVisibility() is set to TRUE.
     * \see setMinimumScale()
     * \see maximumScale()
     * \see hasScaleBasedVisibility()
     * \see isInScaleRange()
     */
    double minimumScale() const;

    /**
     * Returns the maximum map scale (i.e. most "zoomed in" scale) at which the layer will be visible.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * A scale of 0 indicates no maximum scale visibility.
     * \note Scale based visibility is only used if setScaleBasedVisibility() is set to TRUE.
     * \see setMaximumScale()
     * \see minimumScale()
     * \see hasScaleBasedVisibility()
     * \see isInScaleRange()
     */
    double maximumScale() const;

    /**
     * Returns whether scale based visibility is enabled for the layer.
     * \returns TRUE if scale based visibility is enabled
     * \see minimumScale()
     * \see maximumScale()
     * \see setScaleBasedVisibility()
     * \see isInScaleRange()
     */
    bool hasScaleBasedVisibility() const;

    /**
     * Returns TRUE if auto refresh is enabled for the layer.
     * \see autoRefreshInterval()
     * \see setAutoRefreshEnabled()
     * \deprecated use autoRefreshMode() instead.
     */
    Q_DECL_DEPRECATED bool hasAutoRefreshEnabled() const SIP_DEPRECATED;

    /**
     * Returns the layer's automatic refresh mode.
     * \see autoRefreshInterval()
     * \see setAutoRefreshMode()
     * \since QGIS 3.34
     */
    Qgis::AutoRefreshMode autoRefreshMode() const;

    /**
     * Returns the auto refresh interval (in milliseconds). Note that
     * auto refresh is only active when hasAutoRefreshEnabled() is TRUE.
     * \see hasAutoRefreshEnabled()
     * \see setAutoRefreshInterval()
     */
    int autoRefreshInterval() const;

    /**
     * Sets the auto refresh interval (in milliseconds) for the layer. This
     * will cause the layer to be automatically redrawn on a matching interval.
     * Note that auto refresh must be enabled by calling setAutoRefreshMode().
     *
     * Note that auto refresh triggers deferred repaints of the layer. Any map
     * canvas must be refreshed separately in order to view the refreshed layer.
     * \see autoRefreshInterval()
     * \see setAutoRefreshEnabled()
     */
    void setAutoRefreshInterval( int interval );

    /**
     * Sets whether auto refresh is enabled for the layer.
     * \see hasAutoRefreshEnabled()
     * \see setAutoRefreshInterval()
     * \deprecated Use setAutoRefreshMode() instead.
     */
    Q_DECL_DEPRECATED void setAutoRefreshEnabled( bool enabled ) SIP_DEPRECATED;

    /**
     * Sets the automatic refresh mode for the layer.
     * \see autoRefreshMode()
     * \see setAutoRefreshInterval()
     * \since QGIS 3.34
     */
    void setAutoRefreshMode( Qgis::AutoRefreshMode mode );

    /**
     * Returns a reference to the layer's metadata store.
     * \see setMetadata()
     * \see metadataChanged()
     */
    virtual const QgsLayerMetadata &metadata() const;

    /**
     * Sets the layer's \a metadata store.
     * \see metadata()
     * \see metadataChanged()
     */
    virtual void setMetadata( const QgsLayerMetadata &metadata );

    /**
     * Obtain a formatted HTML string containing assorted metadata for this layer.
     */
    virtual QString htmlMetadata() const;

    //! Time stamp of data source in the moment when data/metadata were loaded by provider
    virtual QDateTime timestamp() const;

    /**
     * Gets the list of dependencies. This includes data dependencies set by the user (\see setDataDependencies)
     * as well as dependencies given by the provider
     *
     * \returns a set of QgsMapLayerDependency
     */
    virtual QSet<QgsMapLayerDependency> dependencies() const;

    /**
     * Returns the message that should be notified by the provider to triggerRepaint
     *
     */
    QString refreshOnNotifyMessage() const { return mRefreshOnNofifyMessage; }

    /**
     * Returns TRUE if the refresh on provider nofification is enabled
     *
     */
    bool isRefreshOnNotifyEnabled() const { return mIsRefreshOnNofifyEnabled; }

    /**
     * Returns the XML properties of the original layer as they were when the layer
     * was first read from the project file. In case of new layers this is normally empty.
     *
     * The storage format for the XML is qlr
     *
     * \since QGIS 3.6
     */
    QString originalXmlProperties() const;

    /**
     * Sets the original XML properties for the layer to  \a originalXmlProperties
     *
     * The storage format for the XML is qlr
     *
     * \since QGIS 3.6
     */
    void setOriginalXmlProperties( const QString &originalXmlProperties );

    /**
     * Generates an unique identifier for this layer, the generate ID is prefixed by \a layerName
     * \since QGIS 3.8
     */
    static QString generateId( const QString &layerName );

    /**
     * Accepts the specified symbology \a visitor, causing it to visit all symbols associated
     * with the layer.
     *
     * Returns TRUE if the visitor should continue visiting other objects, or FALSE if visiting
     * should be canceled.
     *
     * \since QGIS 3.10
     */
    virtual bool accept( QgsStyleEntityVisitorInterface *visitor ) const;

    /**
     * Returns the layer's selection properties. This may be NULLPTR, depending on the layer type.
     *
     * \since QGIS 3.34
     */
    virtual QgsMapLayerSelectionProperties *selectionProperties() { return nullptr; }

    /**
     * Returns the layer's temporal properties. This may be NULLPTR, depending on the layer type.
     *
     * \since QGIS 3.14
     */
    virtual QgsMapLayerTemporalProperties *temporalProperties() { return nullptr; }

    /**
     * Returns the layer's elevation properties. This may be NULLPTR, depending on the layer type.
     *
     * \since QGIS 3.18
     */
    virtual QgsMapLayerElevationProperties *elevationProperties() { return nullptr; }

    /**
     * Returns path to the placeholder image or an empty string if a generated legend is shown
     * \return placeholder image path
     * \since QGIS 3.22
     */
    QString legendPlaceholderImage() const { return mLegendPlaceholderImage;}

    /**
     * Set placeholder image for legend. If the string is empty, a generated legend will be shown.
     * \param imgPath file path to the placeholder image
     * \since QGIS 3.22
     */
    void setLegendPlaceholderImage( const QString &imgPath ) { mLegendPlaceholderImage = imgPath; }

    /**
     * Returns TRUE if the layer contains map tips.
     *
     * \see mapTipTemplate()
     * \see setMapTipTemplate()
     */
    virtual bool hasMapTips() const;

    /**
     * The mapTip is a pretty, html representation for feature information.
     *
     * It may also contain embedded expressions.
     *
     * \since QGIS 3.30
     */
    QString mapTipTemplate() const;

    /**
     * The mapTip is a pretty, html representation for feature information.
     *
     * It may also contain embedded expressions.
     *
     * \since QGIS 3.30
     */
    void setMapTipTemplate( const QString &mapTipTemplate );

    /**
     *  Enable or disable map tips for this layer
     *
     *  \param enabled Whether map tips are enabled for this layer
     *  \since QGIS 3.32
     */
    void setMapTipsEnabled( bool enabled );

    /**
     *  Returns true if map tips are enabled for this layer
     *  \since QGIS 3.32
     */
    bool mapTipsEnabled() const;

    /**
     * Returns provider read flag deduced from layer read flags \a layerReadFlags and a dom node \a layerNode
     * that describes a layer (corresponding to ``maplayer'' tag in a DOM document project file read by QgsProject).
     * This static method is used when loading a project.
     *
     * \since QGIS 3.32
     */
    static QgsDataProvider::ReadFlags providerReadFlags( const QDomNode &layerNode, QgsMapLayer::ReadFlags layerReadFlags );

  public slots:

    /**
     * Sets the minimum map \a scale (i.e. most "zoomed out" scale) at which the layer will be visible.
     * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * A \a scale of 0 indicates no minimum scale visibility.
     * \note Scale based visibility is only used if setScaleBasedVisibility() is set to TRUE.
     * \see minimumScale()
     * \see setMaximumScale()
     * \see setScaleBasedVisibility()
     */
    void setMinimumScale( double scale );

    /**
     * Sets the maximum map \a scale (i.e. most "zoomed in" scale) at which the layer will be visible.
     * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * A \a scale of 0 indicates no maximum scale visibility.
     * \note Scale based visibility is only used if setScaleBasedVisibility() is set to TRUE.
     * \see maximumScale()
     * \see setMinimumScale()
     * \see setScaleBasedVisibility()
     */
    void setMaximumScale( double scale );

    /**
     * Sets whether scale based visibility is enabled for the layer.
     * \param enabled set to TRUE to enable scale based visibility
     * \see setMinimumScale
     * \see setMaximumScale
     * \see hasScaleBasedVisibility
     */
    void setScaleBasedVisibility( bool enabled );

    /**
     * Will advise the map canvas (and any other interested party) that this layer requires to be repainted.
     * Will emit a repaintRequested() signal.
     * If \a deferredUpdate is TRUE then the layer will only be repainted when the canvas is next
     * re-rendered, and will not trigger any canvas redraws itself.
     *
     * \note in 2.6 function moved from vector/raster subclasses to QgsMapLayer
     */
    void triggerRepaint( bool deferredUpdate = false );

    /**
     * Will advise any 3D maps that this layer requires to be updated in the scene.
     * Will emit a request3DUpdate() signal.
     *
     * \since QGIS 3.18
     */
    void trigger3DUpdate();

    /**
     * Triggers an emission of the styleChanged() signal.
     */
    void emitStyleChanged();

    /**
     * Sets the list of dependencies.
     * \see dependencies()
     *
     * \param layers set of QgsMapLayerDependency. Only user-defined dependencies will be added
     * \returns FALSE if a dependency cycle has been detected
     */
    virtual bool setDependencies( const QSet<QgsMapLayerDependency> &layers );

    /**
     * Set whether provider notification is connected to triggerRepaint
     *
     */
    void setRefreshOnNotifyEnabled( bool enabled );

    /**
     * Set the notification message that triggers repaint
     * If refresh on notification is enabled, the notification will triggerRepaint only
     * if the notification message is equal to \param message
     *
     */
    void setRefreshOnNofifyMessage( const QString &message ) { mRefreshOnNofifyMessage = message; }

    /**
     * Sets the coordinate transform context to \a transformContext
     *
     * \since QGIS 3.8
     */
    virtual void setTransformContext( const QgsCoordinateTransformContext &transformContext ) = 0;

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsMapLayer: '%1' (%2)>" ).arg( sipCpp->name(), sipCpp->dataProvider() ? sipCpp->dataProvider()->name() : QStringLiteral( "Invalid" ) );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    /**
     * Returns the parent project if this map layer is added to a project.
     * Otherwise returns NULLPTR
     *
     * \since QGIS 3.18
     */
    QgsProject *project() const;

  signals:

    /**
     * Emitted when all layers are loaded and references can be resolved,
     * just before the references of this layer are resolved.
     *
     * \since QGIS 3.10
     */
    void beforeResolveReferences( QgsProject *project );

    //! Emit a signal with status (e.g. to be caught by QgisApp and display a msg on status bar)
    void statusChanged( const QString &status );

    /**
     * Emitted when the layer's ID has been changed.
     *
     * \see id()
     * \see setId()
     *
     * \since QGIS 3.38
     */
    void idChanged( const QString &id );

    /**
     * Emitted when the name has been changed
     */
    void nameChanged();

    /**
     * Emitted when the crs() of the layer has changed.
     *
     * \see crs()
     * \see setCrs()
     * \see verticalCrsChanged()
     * \see crs3DChanged()
     */
    void crsChanged();

    /**
     * Emitted when the crs3D() of the layer has changed.
     *
     * \see crs3D()
     * \see crsChanged()
     * \see verticalCrsChanged()
     *
     * \since QGIS 3.38
     */
    void crs3DChanged();

    /**
     * Emitted when the verticalCrs() of the layer has changed.
     *
     * This signal will be emitted whenever the vertical CRS of the layer is changed, either
     * as a direct result of a call to setVerticalCrs() or when setCrs() is called with a compound
     * CRS.
     *
     * \see crsChanged()
     * \see crs3DChanged()
     * \see setCrs()
     * \see setVerticalCrs()
     * \see verticalCrs()
     *
     * \since QGIS 3.38
     */
    void verticalCrsChanged();

    /**
     * By emitting this signal the layer tells that either appearance or content have been changed
     * and any view showing the rendered layer should refresh itself.
     * If \a deferredUpdate is TRUE then the layer will only be repainted when the canvas is next
     * re-rendered, and will not trigger any canvas redraws itself.
     */
    void repaintRequested( bool deferredUpdate = false );

    //! This is used to send a request that any mapcanvas using this layer update its extents
    void recalculateExtents() const;

    //! Data of layer changed
    void dataChanged();

    //! Signal emitted when the blend mode is changed, through QgsMapLayer::setBlendMode()
    void blendModeChanged( QPainter::CompositionMode blendMode );

    /**
     * Emitted when the layer's opacity is changed, where \a opacity is a value between 0 (transparent)
     * and 1 (opaque).
     * \see setOpacity()
     * \see opacity()
     * \note Prior to QGIS 3.18, this signal was available for vector layers only
     * \since QGIS 3.18
     */
    void opacityChanged( double opacity );

    /**
     * Signal emitted when renderer is changed.
     * \see styleChanged()
    */
    void rendererChanged();

    /**
     * Signal emitted whenever a change affects the layer's style. Ie this may be triggered
     * by renderer changes, label style changes, or other style changes such as blend
     * mode or layer opacity changes.
     *
     * \warning This signal should never be manually emitted. Instead call the emitStyleChanged() method
     * to ensure that the signal is only emitted when appropriate.
     *
     * \see rendererChanged()
    */
    void styleChanged();

    /**
     * Signal emitted when legend of the layer has changed
     */
    void legendChanged();

    /**
     * Signal emitted when 3D renderer associated with the layer has changed.
     */
    void renderer3DChanged();

    /**
     * Signal emitted when a layer requires an update in any 3D maps.
     *
     * \since QGIS 3.18
     */
    void request3DUpdate();

    /**
     * Emitted whenever the configuration is changed. The project listens to this signal
     * to be marked as dirty.
     */
    void configChanged();

    /**
     * Emitted when dependencies are changed.
     */
    void dependenciesChanged();

    /**
     * Emitted in the destructor when the layer is about to be deleted,
     * but it is still in a perfectly valid state: the last chance for
     * other pieces of code for some cleanup if they use the layer.
     */
    void willBeDeleted();

    /**
     * Emitted when the auto refresh interval changes.
     * \see setAutoRefreshInterval()
     */
    void autoRefreshIntervalChanged( int interval );

    /**
     * Emitted when the layer's metadata is changed.
     * \see setMetadata()
     * \see metadata()
     */
    void metadataChanged();

    /**
     * Emitted when layer's flags have been modified.
     * \see setFlags()
     * \see flags()
     * \since QGIS 3.4
     */
    void flagsChanged();

    /**
     * Emitted whenever the layer's data source has been changed.
     *
     * \see setDataSource()
     *
     * \since QGIS 3.5
     */
    void dataSourceChanged();

    /**
     * Emitted when a style has been loaded
     * \param categories style categories
     * \since QGIS 3.12
     */
    void styleLoaded( QgsMapLayer::StyleCategories categories );

    /**
     * Emitted when the validity of this layer changed.
     *
     * \since QGIS 3.16
     */
    void isValidChanged();

    /**
     * Emitted when a custom property of the layer has been changed or removed.
     *
     * \since QGIS 3.18
     */
    void customPropertyChanged( const QString &key );

    /**
     * Emitted when editing on this layer has started.
     * \since QGIS 3.22 in the QgsMapLayer base class
     */
    void editingStarted();

    /**
     * Emitted when edited changes have been successfully written to the data provider.
     * \since QGIS 3.22 in the QgsMapLayer base class
     */
    void editingStopped();

    /**
     * Emitted when modifications has been done on layer
     * \since QGIS 3.22 in the QgsMapLayer base class
     */
    void layerModified();

    /**
     * Emitted when the map tip template changes
     *
     * \since QGIS 3.30
     */
    void mapTipTemplateChanged();

    /**
     * Emitted when map tips are enabled or disabled for the layer.
     *
     * \see setMapTipsEnabled()
     * \since QGIS 3.32
     */
    void mapTipsEnabledChanged();

  private slots:

    void onNotified( const QString &message );

    /**
     * Updates the data source of the layer. The layer's renderer and legend will be preserved only
     * if the geometry type of the new data source matches the current geometry type of the layer.
     *
     * Called by setDataSource()
     * Subclasses should override this method: default implementation does nothing.
     *
     * \param dataSource new layer data source
     * \param baseName base name of the layer
     * \param provider provider string
     * \param options provider options
     * \param flags provider read flags
     * \see dataSourceChanged()
     * \since QGIS 3.20
     */
    virtual void setDataSourcePrivate( const QString &dataSource, const QString &baseName, const QString &provider, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags );

  protected:

    /**
     * Copies attributes like name, short name, ... into another layer.
     * \param layer The copy recipient
     */
    void clone( QgsMapLayer *layer ) const;

    //! Sets the extent
    virtual void setExtent( const QgsRectangle &rect );

    /**
     * Sets the extent
     * \since QGIS 3.36
     */
    virtual void setExtent3D( const QgsBox3D &box );

    //! Sets whether layer is valid or not
    void setValid( bool valid );

    /**
     * Called by readLayerXML(), used by children to read state specific to them from
     *  project files.
     */
    virtual bool readXml( const QDomNode &layer_node, QgsReadWriteContext &context );

    /**
     * Called by writeLayerXML(), used by children to write state specific to them to
     *  project files.
     */
    virtual bool writeXml( QDomNode &layer_node, QDomDocument &document, const QgsReadWriteContext &context ) const;

    /**
     * Called by writeLayerXML(), used by derived classes to encode provider's specific data
     * source to project files. Typically resolving absolute or relative paths, usernames and
     * passwords or drivers prefixes ("HDF5:")
     *
     * \param source data source to encode, typically QgsMapLayer::source()
     * \param context writing context (e.g. for conversion between relative and absolute paths)
     * \return encoded source, typically to be written in the DOM element "datasource"
     *
     * \since QGIS 3.2
     */
    virtual QString encodedSource( const QString &source, const QgsReadWriteContext &context ) const;

    /**
     * Called by readLayerXML(), used by derived classes to decode provider's specific data
     * source from project files. Typically resolving absolute or relative paths, usernames and
     * passwords or drivers prefixes ("HDF5:")
     *
     * \param source data source to decode, typically read from layer's DOM element "datasource"
     * \param dataProvider string identification of data provider (e.g. "ogr"), typically read from layer's DOM element
     * \param context reading context (e.g. for conversion between relative and absolute paths)
     * \return decoded source, typically to be used as the layer's datasource
     *
     * \since QGIS 3.2
     */
    virtual QString decodedSource( const QString &source, const QString &dataProvider, const QgsReadWriteContext &context ) const;

    /**
     * Read custom properties from project file.
     * \param layerNode note to read from
     * \param keyStartsWith reads only properties starting with the specified string (or all if the string is empty)
    */
    void readCustomProperties( const QDomNode &layerNode, const QString &keyStartsWith = QString() );

    //! Write custom properties to project file.
    void writeCustomProperties( QDomNode &layerNode, QDomDocument &doc ) const;

    //! Read style manager's configuration (if any). To be called by subclasses.
    void readStyleManager( const QDomNode &layerNode );
    //! Write style manager's configuration (if exists). To be called by subclasses.
    void writeStyleManager( QDomNode &layerNode, QDomDocument &doc ) const;

    /**
     * Write style data common to all layer types
     */
    void writeCommonStyle( QDomElement &layerElement, QDomDocument &document,
                           const QgsReadWriteContext &context,
                           StyleCategories categories = AllStyleCategories ) const;

    /**
     * Read style data common to all layer types
     */
    void readCommonStyle( const QDomElement &layerElement, const QgsReadWriteContext &context,
                          StyleCategories categories = AllStyleCategories );

    //! Sets the \a providerType (provider key)
    void setProviderType( const QString &providerType );

#ifndef SIP_RUN
#if 0
    //! Debugging member - invoked when a connect() is made to this object
    void connectNotify( const char *signal ) override;
#endif
#endif

    //! Add error message
    void appendError( const QgsErrorMessage &error ) { mError.append( error );}
    //! Sets error message
    void setError( const QgsError &error ) { mError = error;}

    /**
     * Invalidates the WGS84 extent. If FlagTrustLayerMetadata is enabled,
     * the extent is not invalidated because we want to trust metadata whatever
     * happens.
     * \since QGIS 3.20
     */
    void invalidateWgs84Extent();

    //! Indicates if the layer is valid and can be drawn
    bool mValid = false;

    //! Data source description string, varies by layer type
    QString mDataSource;

    //! Name of the layer - used for display
    QString mLayerName;

    //! WMS legend
    QString mLegendUrl;
    QString mLegendUrlFormat;

    //! \brief Error
    QgsError mError;

    //! List of layers that may modify this layer on modification
    QSet<QgsMapLayerDependency> mDependencies;

    /**
     * Checks whether a new set of dependencies will introduce a cycle
     * this method is now deprecated and always return FALSE, because circular dependencies are now correctly managed.
     * \deprecated since QGIS 3.10
     */
    Q_DECL_DEPRECATED bool hasDependencyCycle( const QSet<QgsMapLayerDependency> & ) const {return false;}

    bool mIsRefreshOnNofifyEnabled = false;
    QString mRefreshOnNofifyMessage;

    //! Data provider key (name of the data provider)
    QString mProviderKey;

    //TODO QGIS 4 - move to readXml as a new argument (breaks API)

    //! Read flags. It's up to the subclass to respect these when restoring state from XML
    QgsMapLayer::ReadFlags mReadFlags = QgsMapLayer::ReadFlags();

    /**
     * TRUE if the layer's CRS should be validated and invalid CRSes are not permitted.
     *
     * \since QGIS 3.10
     */
    bool mShouldValidateCrs = true;

    /**
     * Layer opacity.
     *
     * \since QGIS 3.18
     */
    double mLayerOpacity = 1.0;

    /**
     * If non-zero, the styleChanged signal should not be emitted.
     *
     * \since QGIS 3.20
     */
    int mBlockStyleChangedSignal = 0;

#ifndef SIP_RUN

    /**
     * Returns a HTML fragment containing the layer's CRS metadata, for use
     * in the htmlMetadata() method.
     *
     * \note Not available in Python bindings.
     *
     * \since QGIS 3.20
     */
    QString crsHtmlMetadata() const;
#endif

#ifndef SIP_RUN

    /**
     * Returns an HTML fragment containing general  metadata information, for use
     * in the htmlMetadata() method.
     *
     * \note Not available in Python bindings.
     *
     * \since QGIS 3.22
     */
    QString generalHtmlMetadata() const;
#endif

#ifndef SIP_RUN

    /**
     * Optionally used when loading a project, it is released when the layer is effectively created
     *
     * \note Not available in Python bindings.
     *
     * \since QGIS 3.32
     */
    std::unique_ptr<QgsDataProvider> mPreloadedProvider;
#endif

  private:

    virtual QString baseURI( PropertyType type ) const;
    QString saveNamedProperty( const QString &uri, QgsMapLayer::PropertyType type,
                               bool &resultFlag, StyleCategories categories = AllStyleCategories );
    QString loadNamedProperty( const QString &uri, QgsMapLayer::PropertyType type,
                               bool &namedPropertyExists, bool &propertySuccessfullyLoaded, StyleCategories categories = AllStyleCategories, Qgis::LoadStyleFlags flags = Qgis::LoadStyleFlags() );
    bool loadNamedPropertyFromDatabase( const QString &db, const QString &uri, QString &xml, QgsMapLayer::PropertyType type );

    // const method because extents are mutable
    void updateExtent( const QgsRectangle &extent ) const;
    void updateExtent( const QgsBox3D &extent ) const;

    bool rebuildCrs3D( QString *error = nullptr );

    /**
     * This method returns TRUE by default but can be overwritten to specify
     * that a certain layer is writable.
     */
    virtual bool isReadOnly() const;

    /**
     * Layer's spatial reference system.
     * private to make sure setCrs must be used and crsChanged() is emitted.
    */
    QgsCoordinateReferenceSystem mCRS;
    QgsCoordinateReferenceSystem mVerticalCrs;
    QgsCoordinateReferenceSystem mCrs3D;

    //! Unique ID of this layer - used to refer to this layer in map layer registry
    QString mID;

    //! Type of the layer (e.g., vector, raster)
    Qgis::LayerType mLayerType;

    LayerFlags mFlags = LayerFlags( Identifiable | Removable | Searchable );

    //! Blend mode for the layer
    QPainter::CompositionMode mBlendMode = QPainter::CompositionMode_SourceOver;

    //! Tag for embedding additional information
    QString mTag;

    //set some generous  defaults for scale based visibility

    //! Minimum scale denominator at which this layer should be displayed
    double mMinScale = 0;
    //! Maximum scale denominator at which this layer should be displayed
    double mMaxScale = 100000000;
    //! A flag that tells us whether to use the above vars to restrict layer visibility
    bool mScaleBasedVisibility = false;

    /**
     * Stores information about server properties
     */
    std::unique_ptr< QgsMapLayerServerProperties > mServerProperties;

    //! Collection of undoable operations for this layer.
    QUndoStack *mUndoStack = nullptr;

    QUndoStack *mUndoStackStyles = nullptr;

    //! Layer's persistent storage of additional properties (may be used by plugins)
    QgsObjectCustomProperties mCustomProperties;

    //! Controller of legend items of this layer
    QgsMapLayerLegend *mLegend = nullptr;

    //! Manager of multiple styles available for a layer (may be NULLPTR)
    QgsMapLayerStyleManager *mStyleManager = nullptr;

    Qgis::AutoRefreshMode mAutoRefreshMode = Qgis::AutoRefreshMode::Disabled;

    //! Timer for triggering automatic refreshes of the layer
    QTimer *mRefreshTimer = nullptr;

    QgsLayerMetadata mMetadata;

    //! Renderer for 3D views
    QgsAbstract3DRenderer *m3DRenderer = nullptr;

    //! 3D Extent of the layer
    mutable QgsBox3D mExtent3D;

    //! 2D Extent of the layer
    mutable QgsRectangle mExtent2D;

    //! Extent of the layer in EPSG:4326
    mutable QgsRectangle mWgs84Extent;

    /**
     * Stores the original XML properties of the layer when loaded from the project
     *
     * This information can be used to pass through the bad layers or to reset changes on a good layer
     */
    QString mOriginalXmlProperties;

    //! To avoid firing multiple time repaintRequested signal on circular layer circular dependencies
    bool mRepaintRequestedFired = false;

    //! Path to placeholder image for layer legend. If the string is empty, a generated legend is shown
    QString mLegendPlaceholderImage;

    //! Maptip template
    QString mMapTipTemplate;

    //! Flag indicating whether map tips are enabled for this layer or not
    bool mMapTipsEnabled = true;

    friend class QgsVectorLayer;
    friend class TestQgsProject;
    friend class TestQgsMapLayer;
};

Q_DECLARE_METATYPE( QgsMapLayer * )
Q_DECLARE_OPERATORS_FOR_FLAGS( QgsMapLayer::LayerFlags )
Q_DECLARE_OPERATORS_FOR_FLAGS( QgsMapLayer::StyleCategories )
Q_DECLARE_OPERATORS_FOR_FLAGS( QgsMapLayer::ReadFlags )


#ifndef SIP_RUN

/**
 * Weak pointer for QgsMapLayer
 * \note not available in Python bindings
 */
typedef QPointer< QgsMapLayer > QgsWeakMapLayerPointer;

/**
 * A list of weak pointers to QgsMapLayers.
 * \note not available in Python bindings
 */
typedef QList< QgsWeakMapLayerPointer > QgsWeakMapLayerPointerList;
#endif

#endif
