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
#ifndef QGSRENDERERV2REGISTRY_H
#define QGSRENDERERV2REGISTRY_H

#include <QIcon>
#include <QMap>
#include <QStringList>
#include <QDomElement>

#include "qgis.h"

class QgsFeatureRenderer;
class QgsVectorLayer;
class QgsStyle;
class QgsRendererWidget;

/** \ingroup core
 Stores metadata about one renderer class.

 @note It's necessary to implement createRenderer() function.
   In C++ you can use QgsRendererMetadata convenience class.
 */
class CORE_EXPORT QgsRendererAbstractMetadata
{
  public:

    //! Layer types the renderer is compatible with
    //! @note added in QGIS 2.16
    enum LayerType
    {
      PointLayer = 1, //!< Compatible with point layers
      LineLayer = 2, //!< Compatible with line layers
      PolygonLayer = 4, //!< Compatible with polygon layers
      All = PointLayer | LineLayer | PolygonLayer, //!< Compatible with all vector layers
    };
    Q_DECLARE_FLAGS( LayerTypes, LayerType )

    QgsRendererAbstractMetadata( const QString& name, const QString& visibleName, const QIcon& icon = QIcon() )
        : mName( name )
        , mVisibleName( visibleName )
        , mIcon( icon )
    {}
    virtual ~QgsRendererAbstractMetadata() {}

    QString name() const { return mName; }
    QString visibleName() const { return mVisibleName; }

    QIcon icon() const { return mIcon; }
    void setIcon( const QIcon& icon ) { mIcon = icon; }

    /** Returns flags indicating the types of layer the renderer is compatible with.
     * @note added in QGIS 2.16
     */
    virtual LayerTypes compatibleLayerTypes() const { return All; }

    /** Return new instance of the renderer given the DOM element. Returns NULL on error.
     * Pure virtual function: must be implemented in derived classes.  */
    virtual QgsFeatureRenderer* createRenderer( QDomElement& elem ) = 0;
    /** Return new instance of settings widget for the renderer. Returns NULL on error.
     *
     * The \a oldRenderer argument may refer to previously used renderer (or it is null).
     * If not null, it may be used to initialize GUI of the widget from the previous settings.
     * The old renderer does not have to be of the same type as returned by createRenderer().
     * When using \a oldRenderer make sure to make a copy of it - it will be deleted afterwards.
     */
    virtual QgsRendererWidget* createRendererWidget( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* oldRenderer )
    { Q_UNUSED( layer ); Q_UNUSED( style ); Q_UNUSED( oldRenderer ); return nullptr; }

    virtual QgsFeatureRenderer* createRendererFromSld( QDomElement& elem, QgsWkbTypes::GeometryType geomType )
    { Q_UNUSED( elem ); Q_UNUSED( geomType ); return nullptr; }

  protected:
    //! name used within QGIS for identification (the same what renderer's type() returns)
    QString mName;
    //! name visible for users (translatable)
    QString mVisibleName;
    //! icon to be shown in the renderer properties dialog
    QIcon mIcon;
};


Q_DECLARE_OPERATORS_FOR_FLAGS( QgsRendererAbstractMetadata::LayerTypes )


typedef QgsFeatureRenderer*( *QgsRendererCreateFunc )( QDomElement& );
typedef QgsRendererWidget*( *QgsRendererWidgetFunc )( QgsVectorLayer*, QgsStyle*, QgsFeatureRenderer* );
typedef QgsFeatureRenderer*( *QgsRendererCreateFromSldFunc )( QDomElement&, QgsWkbTypes::GeometryType geomType );

/** \ingroup core
 Convenience metadata class that uses static functions to create renderer and its widget.
 */
class CORE_EXPORT QgsRendererMetadata : public QgsRendererAbstractMetadata
{
  public:

    /** Construct metadata */
    //! @note not available in python bindings
    QgsRendererMetadata( const QString& name,
                         const QString& visibleName,
                         QgsRendererCreateFunc pfCreate,
                         const QIcon& icon = QIcon(),
                         QgsRendererWidgetFunc pfWidget = nullptr,
                         QgsRendererAbstractMetadata::LayerTypes layerTypes = QgsRendererAbstractMetadata::All )
        : QgsRendererAbstractMetadata( name, visibleName, icon )
        , mCreateFunc( pfCreate )
        , mWidgetFunc( pfWidget )
        , mCreateFromSldFunc( nullptr )
        , mLayerTypes( layerTypes )
    {}

    //! @note not available in python bindings
    QgsRendererMetadata( const QString& name,
                         const QString& visibleName,
                         QgsRendererCreateFunc pfCreate,
                         QgsRendererCreateFromSldFunc pfCreateFromSld,
                         const QIcon& icon = QIcon(),
                         QgsRendererWidgetFunc pfWidget = nullptr,
                         QgsRendererAbstractMetadata::LayerTypes layerTypes = QgsRendererAbstractMetadata::All )
        : QgsRendererAbstractMetadata( name, visibleName, icon )
        , mCreateFunc( pfCreate )
        , mWidgetFunc( pfWidget )
        , mCreateFromSldFunc( pfCreateFromSld )
        , mLayerTypes( layerTypes )
    {}

    virtual ~QgsRendererMetadata();

    virtual QgsFeatureRenderer* createRenderer( QDomElement& elem ) override { return mCreateFunc ? mCreateFunc( elem ) : nullptr; }
    virtual QgsRendererWidget* createRendererWidget( QgsVectorLayer* layer, QgsStyle* style, QgsFeatureRenderer* renderer ) override
      { return mWidgetFunc ? mWidgetFunc( layer, style, renderer ) : nullptr; }
    virtual QgsFeatureRenderer* createRendererFromSld( QDomElement& elem, QgsWkbTypes::GeometryType geomType ) override
      { return mCreateFromSldFunc ? mCreateFromSldFunc( elem, geomType ) : nullptr; }

    //! @note not available in python bindings
    QgsRendererCreateFunc createFunction() const { return mCreateFunc; }
    //! @note not available in python bindings
    QgsRendererWidgetFunc widgetFunction() const { return mWidgetFunc; }
    //! @note not available in python bindings
    QgsRendererCreateFromSldFunc createFromSldFunction() const { return mCreateFromSldFunc; }

    //! @note not available in python bindings
    void setWidgetFunction( QgsRendererWidgetFunc f ) { mWidgetFunc = f; }

    virtual QgsRendererAbstractMetadata::LayerTypes compatibleLayerTypes() const override { return mLayerTypes; }

  protected:
    //! pointer to function that creates an instance of the renderer when loading project / style
    QgsRendererCreateFunc mCreateFunc;
    //! pointer to function that creates a widget for configuration of renderer's params
    QgsRendererWidgetFunc mWidgetFunc;
    //! pointer to function that creates an instance of the renderer from SLD
    QgsRendererCreateFromSldFunc mCreateFromSldFunc;

  private:

    QgsRendererAbstractMetadata::LayerTypes mLayerTypes;
};


/** \ingroup core
 * \class QgsRendererRegistry
 * \brief Registry of renderers.
 *
 * This is a singleton, renderers can be added / removed at any time
 */

class CORE_EXPORT QgsRendererRegistry
{
  public:

    //! Returns a pointer to the QgsRendererRegistry singleton
    static QgsRendererRegistry* instance();

    //! Adds a renderer to the registry. Takes ownership of the metadata object.
    //! @param metadata renderer metadata
    //! @returns true if renderer was added successfully, or false if renderer could not
    //! be added (eg a renderer with a duplicate name already exists)
    bool addRenderer( QgsRendererAbstractMetadata* metadata );

    //! Removes a renderer from registry.
    //! @param rendererName name of renderer to remove from registry
    //! @returns true if renderer was sucessfully removed, or false if matching
    //! renderer could not be found
    bool removeRenderer( const QString& rendererName );

    //! Returns the metadata for a specified renderer. Returns NULL if a matching
    //! renderer was not found in the registry.
    QgsRendererAbstractMetadata* rendererMetadata( const QString& rendererName );

    //! Returns a list of available renderers.
    //! @param layerTypes flags to filter the renderers by compatible layer types
    QStringList renderersList( QgsRendererAbstractMetadata::LayerTypes layerTypes = QgsRendererAbstractMetadata::All ) const;

    //! Returns a list of available renderers which are compatible with a specified layer.
    //! @param layer vector layer
    //! @note added in QGIS 2.16
    QStringList renderersList( const QgsVectorLayer* layer ) const;

  protected:
    //! protected constructor
    QgsRendererRegistry();
    ~QgsRendererRegistry();

    //! Map of name to renderer
    QMap<QString, QgsRendererAbstractMetadata*> mRenderers;

    //! List of renderers, maintained in the order that they have been added
    QStringList mRenderersOrder;

  private:
    QgsRendererRegistry( const QgsRendererRegistry& rh );
    QgsRendererRegistry& operator=( const QgsRendererRegistry& rh );
};

#endif // QGSRENDERERV2REGISTRY_H
