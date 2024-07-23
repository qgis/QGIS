/***************************************************************************
    qgsrendererregistry.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRENDERERREGISTRY_H
#define QGSRENDERERREGISTRY_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QIcon>
#include <QMap>
#include <QStringList>
#include <QDomElement>

// #include "qgis.h"
#include "qgswkbtypes.h"

class QgsFeatureRenderer;
class QgsReadWriteContext;
class QgsVectorLayer;
class QgsStyle;
class QgsRendererWidget SIP_EXTERNAL;

/**
 * \ingroup core
 * \brief Stores metadata about one renderer class.
 *
 * \note It's necessary to implement createRenderer() function.
 *   In C++ you can use QgsRendererMetadata convenience class.
 */
class CORE_EXPORT QgsRendererAbstractMetadata
{
  public:

    /**
     * Layer types the renderer is compatible with
     */
    enum LayerType SIP_ENUM_BASETYPE( IntFlag )
    {
      PointLayer = 1, //!< Compatible with point layers
      LineLayer = 2, //!< Compatible with line layers
      PolygonLayer = 4, //!< Compatible with polygon layers
      All = PointLayer | LineLayer | PolygonLayer, //!< Compatible with all vector layers
    };
    Q_DECLARE_FLAGS( LayerTypes, LayerType )

    /**
     * Constructor for QgsRendererAbstractMetadata, with the specified \a name.
     *
     * The \a visibleName argument gives a translated, user friendly string identifying the renderer type.
     *
     * The \a icon argument can be used to specify an icon representing the renderer.
     */
    QgsRendererAbstractMetadata( const QString &name, const QString &visibleName, const QIcon &icon = QIcon() )
      : mName( name )
      , mVisibleName( visibleName )
      , mIcon( icon )
    {}
    virtual ~QgsRendererAbstractMetadata() = default;

    /**
     * Returns the unique name of the renderer. This value is not translated.
     * \see visibleName()
     */
    QString name() const { return mName; }

    /**
     * Returns a friendly display name of the renderer. This value is translated.
     * \see name()
     */
    QString visibleName() const { return mVisibleName; }

    /**
     * Returns an icon representing the renderer.
     * \see setIcon()
     */
    QIcon icon() const { return mIcon; }

    /**
     * Sets an \a icon representing the renderer.
     * \see icon()
     */
    void setIcon( const QIcon &icon ) { mIcon = icon; }

    /**
     * Returns flags indicating the types of layer the renderer is compatible with.
     */
    virtual QgsRendererAbstractMetadata::LayerTypes compatibleLayerTypes() const { return All; }

    /**
     * Returns new instance of the renderer given the DOM element. Returns NULLPTR on error.
     * Pure virtual function: must be implemented in derived classes.
    */
    virtual QgsFeatureRenderer *createRenderer( QDomElement &elem, const QgsReadWriteContext &context ) = 0 SIP_FACTORY;

    /**
     * Returns new instance of settings widget for the renderer. Returns NULLPTR on error.
     *
     * The \a oldRenderer argument may refer to previously used renderer (or it is NULLPTR).
     * If not NULLPTR, it may be used to initialize GUI of the widget from the previous settings.
     * The old renderer does not have to be of the same type as returned by createRenderer().
     */
    virtual QgsRendererWidget *createRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *oldRenderer ) SIP_FACTORY
    { Q_UNUSED( layer ) Q_UNUSED( style ); Q_UNUSED( oldRenderer ); return nullptr; }

    /**
     * Returns a new instance of the renderer, converted from an SLD XML element.
     */
    virtual QgsFeatureRenderer *createRendererFromSld( QDomElement &elem, Qgis::GeometryType geomType ) SIP_FACTORY
    { Q_UNUSED( elem ) Q_UNUSED( geomType ); return nullptr; }

  protected:
    //! name used within QGIS for identification (the same what renderer's type() returns)
    QString mName;
    //! name visible for users (translatable)
    QString mVisibleName;
    //! icon to be shown in the renderer properties dialog
    QIcon mIcon;
};


Q_DECLARE_OPERATORS_FOR_FLAGS( QgsRendererAbstractMetadata::LayerTypes )


typedef QgsFeatureRenderer *( *QgsRendererCreateFunc )( QDomElement &, const QgsReadWriteContext & ) SIP_SKIP;
typedef QgsRendererWidget *( *QgsRendererWidgetFunc )( QgsVectorLayer *, QgsStyle *, QgsFeatureRenderer * ) SIP_SKIP;
typedef QgsFeatureRenderer *( *QgsRendererCreateFromSldFunc )( QDomElement &, Qgis::GeometryType geomType ) SIP_SKIP;

/**
 * \ingroup core
 * \brief Convenience metadata class that uses static functions to create renderer and its widget.
 */
class CORE_EXPORT QgsRendererMetadata : public QgsRendererAbstractMetadata
{
  public:

    /**
     * Construct metadata
     * \note not available in Python bindings
     */
    QgsRendererMetadata( const QString &name,
                         const QString &visibleName,
                         QgsRendererCreateFunc pfCreate,
                         const QIcon &icon = QIcon(),
                         QgsRendererWidgetFunc pfWidget = nullptr,
                         QgsRendererAbstractMetadata::LayerTypes layerTypes = QgsRendererAbstractMetadata::All ) SIP_SKIP
  : QgsRendererAbstractMetadata( name, visibleName, icon )
    , mCreateFunc( pfCreate )
    , mWidgetFunc( pfWidget )
    , mCreateFromSldFunc( nullptr )
    , mLayerTypes( layerTypes )
    {}

    //! \note not available in Python bindings
    QgsRendererMetadata( const QString &name,
                         const QString &visibleName,
                         QgsRendererCreateFunc pfCreate,
                         QgsRendererCreateFromSldFunc pfCreateFromSld,
                         const QIcon &icon = QIcon(),
                         QgsRendererWidgetFunc pfWidget = nullptr,
                         QgsRendererAbstractMetadata::LayerTypes layerTypes = QgsRendererAbstractMetadata::All ) SIP_SKIP
  : QgsRendererAbstractMetadata( name, visibleName, icon )
    , mCreateFunc( pfCreate )
    , mWidgetFunc( pfWidget )
    , mCreateFromSldFunc( pfCreateFromSld )
    , mLayerTypes( layerTypes )
    {}

    QgsFeatureRenderer *createRenderer( QDomElement &elem, const QgsReadWriteContext &context ) override SIP_FACTORY
    { return mCreateFunc ? mCreateFunc( elem, context ) : nullptr; }
    QgsRendererWidget *createRendererWidget( QgsVectorLayer *layer, QgsStyle *style, QgsFeatureRenderer *renderer ) override SIP_FACTORY
    { return mWidgetFunc ? mWidgetFunc( layer, style, renderer ) : nullptr; }
    QgsFeatureRenderer *createRendererFromSld( QDomElement &elem, Qgis::GeometryType geomType ) override SIP_FACTORY
    { return mCreateFromSldFunc ? mCreateFromSldFunc( elem, geomType ) : nullptr; }

    //! \note not available in Python bindings
    QgsRendererCreateFunc createFunction() const { return mCreateFunc; } SIP_SKIP
    //! \note not available in Python bindings
    QgsRendererWidgetFunc widgetFunction() const { return mWidgetFunc; } SIP_SKIP
    //! \note not available in Python bindings
    QgsRendererCreateFromSldFunc createFromSldFunction() const { return mCreateFromSldFunc; } SIP_SKIP

    //! \note not available in Python bindings
    void setWidgetFunction( QgsRendererWidgetFunc f ) { mWidgetFunc = f; } SIP_SKIP

    QgsRendererAbstractMetadata::LayerTypes compatibleLayerTypes() const override { return mLayerTypes; }

  protected:
    //! pointer to function that creates an instance of the renderer when loading project / style
    QgsRendererCreateFunc mCreateFunc;
    //! pointer to function that creates a widget for configuration of renderer's params
    QgsRendererWidgetFunc mWidgetFunc;
    //! pointer to function that creates an instance of the renderer from SLD
    QgsRendererCreateFromSldFunc mCreateFromSldFunc;

  private:
#ifdef SIP_RUN
    QgsRendererMetadata();
#endif

    QgsRendererAbstractMetadata::LayerTypes mLayerTypes;
};


/**
 * \ingroup core
 * \class QgsRendererRegistry
 * \brief Registry of renderers.
 *
 * QgsRendererRegistry is not usually directly created, but rather accessed through
 * QgsApplication::rendererRegistry().
 *
 */

class CORE_EXPORT QgsRendererRegistry
{
  public:

    QgsRendererRegistry();
    ~QgsRendererRegistry();

    QgsRendererRegistry( const QgsRendererRegistry &rh ) = delete;
    QgsRendererRegistry &operator=( const QgsRendererRegistry &rh ) = delete;

    /**
     * Adds a renderer to the registry. Takes ownership of the metadata object.
     * \param metadata renderer metadata
     * \returns TRUE if renderer was added successfully, or FALSE if renderer could not
     * be added (e.g., a renderer with a duplicate name already exists)
     */
    bool addRenderer( QgsRendererAbstractMetadata *metadata SIP_TRANSFER );

    /**
     * Removes a renderer from registry.
     * \param rendererName name of renderer to remove from registry
     * \returns TRUE if renderer was successfully removed, or FALSE if matching
     * renderer could not be found
     */
    bool removeRenderer( const QString &rendererName );

    /**
     * Returns the metadata for a specified renderer. Returns NULLPTR if a matching
     * renderer was not found in the registry.
     */
    QgsRendererAbstractMetadata *rendererMetadata( const QString &rendererName );

    /**
     * Returns a list of available renderers.
     * \param layerTypes flags to filter the renderers by compatible layer types
     */
    QStringList renderersList( QgsRendererAbstractMetadata::LayerTypes layerTypes = QgsRendererAbstractMetadata::All ) const;

    /**
     * Returns a list of available renderers which are compatible with a specified layer.
     * \param layer vector layer
     */
    QStringList renderersList( const QgsVectorLayer *layer ) const;

  private:
#ifdef SIP_RUN
    QgsRendererRegistry( const QgsRendererRegistry &rh );
#endif

    //! Map of name to renderer
    QMap<QString, QgsRendererAbstractMetadata *> mRenderers;

    //! List of renderers, maintained in the order that they have been added
    QStringList mRenderersOrder;
};

#endif // QGSRENDERERREGISTRY_H
