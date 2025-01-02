/***************************************************************************
    qgstiledscenerendererregistry.h
    ---------------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSTILEDSCENERENDERERREGISTRY_H
#define QGSTILEDSCENERENDERERREGISTRY_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QIcon>
#include <QMap>
#include <QStringList>
#include <QDomElement>

class QgsTiledSceneRenderer;
class QgsReadWriteContext;
class QgsTiledSceneLayer;
class QgsStyle;
#ifndef SIP_RUN
class QgsTiledSceneRendererWidget SIP_EXTERNAL;
#endif


/**
 * \ingroup core
 * \brief Stores metadata about one tiled scene renderer class.
 *
 * \note It's necessary to implement createRenderer() function.
 *   In C++ you can use QgsTiledSceneRendererMetadata convenience class.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledSceneRendererAbstractMetadata
{
  public:

    /**
     * Constructor for QgsTiledSceneRendererAbstractMetadata, with the specified \a name.
     *
     * The \a visibleName argument gives a translated, user friendly string identifying the renderer type.
     *
     * The \a icon argument can be used to specify an icon representing the renderer.
     */
    QgsTiledSceneRendererAbstractMetadata( const QString &name, const QString &visibleName, const QIcon &icon = QIcon() )
      : mName( name )
      , mVisibleName( visibleName )
      , mIcon( icon )
    {}
    virtual ~QgsTiledSceneRendererAbstractMetadata() = default;

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
     * Returns new instance of the renderer given the DOM element. Returns NULLPTR on error.
     * Pure virtual function: must be implemented in derived classes.
    */
    virtual QgsTiledSceneRenderer *createRenderer( QDomElement &elem, const QgsReadWriteContext &context ) = 0 SIP_FACTORY;

#ifndef SIP_RUN

    /**
     * Returns new instance of settings widget for the renderer. Returns NULLPTR on error.
     *
     * The \a oldRenderer argument may refer to previously used renderer (or it is NULLPTR).
     * If not NULLPTR, it may be used to initialize GUI of the widget from the previous settings.
     * The old renderer does not have to be of the same type as returned by createRenderer().
     *
     * \note Not available in Python bindings
     */
    virtual QgsTiledSceneRendererWidget *createRendererWidget( QgsTiledSceneLayer *layer, QgsStyle *style, QgsTiledSceneRenderer *oldRenderer ) SIP_FACTORY
    { Q_UNUSED( layer ) Q_UNUSED( style ); Q_UNUSED( oldRenderer ); return nullptr; }
#endif

  protected:
    //! name used within QGIS for identification (the same what renderer's type() returns)
    QString mName;
    //! name visible for users (translatable)
    QString mVisibleName;
    //! icon to be shown in the renderer properties dialog
    QIcon mIcon;
};

typedef QgsTiledSceneRenderer *( *QgsTiledSceneRendererCreateFunc )( QDomElement &, const QgsReadWriteContext & ) SIP_SKIP;
typedef QgsTiledSceneRendererWidget *( *QgsTiledSceneRendererWidgetFunc )( QgsTiledSceneLayer *, QgsStyle *, QgsTiledSceneRenderer * ) SIP_SKIP;

/**
 * \ingroup core
 * \brief Convenience metadata class that uses static functions to create tiled scene renderer and its widget.
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledSceneRendererMetadata : public QgsTiledSceneRendererAbstractMetadata
{
  public:

    /**
     * Construct metadata
     * \note not available in Python bindings
     */
    QgsTiledSceneRendererMetadata( const QString &name,
                                   const QString &visibleName,
                                   QgsTiledSceneRendererCreateFunc pfCreate,
                                   const QIcon &icon = QIcon(),
                                   QgsTiledSceneRendererWidgetFunc pfWidget = nullptr ) SIP_SKIP
  : QgsTiledSceneRendererAbstractMetadata( name, visibleName, icon )
    , mCreateFunc( pfCreate )
    , mWidgetFunc( pfWidget )
    {}

    QgsTiledSceneRenderer *createRenderer( QDomElement &elem, const QgsReadWriteContext &context ) override SIP_FACTORY
    { return mCreateFunc ? mCreateFunc( elem, context ) : nullptr; }

#ifndef SIP_RUN
    QgsTiledSceneRendererWidget *createRendererWidget( QgsTiledSceneLayer *layer, QgsStyle *style, QgsTiledSceneRenderer *renderer ) override SIP_FACTORY
    { return mWidgetFunc ? mWidgetFunc( layer, style, renderer ) : nullptr; }
#endif

    //! \note not available in Python bindings
    QgsTiledSceneRendererCreateFunc createFunction() const SIP_SKIP { return mCreateFunc; }
    //! \note not available in Python bindings
    QgsTiledSceneRendererWidgetFunc widgetFunction() const SIP_SKIP { return mWidgetFunc; }

    //! \note not available in Python bindings
    void setWidgetFunction( QgsTiledSceneRendererWidgetFunc f ) SIP_SKIP { mWidgetFunc = f; }

  protected:
    //! pointer to function that creates an instance of the renderer when loading project / style
    QgsTiledSceneRendererCreateFunc mCreateFunc;
    //! pointer to function that creates a widget for configuration of renderer's params
    QgsTiledSceneRendererWidgetFunc mWidgetFunc;

  private:
#ifdef SIP_RUN
    QgsTiledSceneRendererMetadata();
#endif

};


/**
 * \ingroup core
 * \class QgsTiledSceneRendererRegistry
 * \brief Registry of 2D renderers for tiled scenes.
 *
 * QgsTiledSceneRendererRegistry is not usually directly created, but rather accessed through
 * QgsApplication::tiledSceneRendererRegistry().
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsTiledSceneRendererRegistry
{
  public:

    QgsTiledSceneRendererRegistry();
    ~QgsTiledSceneRendererRegistry();

    QgsTiledSceneRendererRegistry( const QgsTiledSceneRendererRegistry &rh ) = delete;
    QgsTiledSceneRendererRegistry &operator=( const QgsTiledSceneRendererRegistry &rh ) = delete;

    /**
     * Adds a renderer to the registry. Takes ownership of the metadata object.
     * \param metadata renderer metadata
     * \returns TRUE if renderer was added successfully, or FALSE if renderer could not
     * be added (e.g., a renderer with a duplicate name already exists)
     */
    bool addRenderer( QgsTiledSceneRendererAbstractMetadata *metadata SIP_TRANSFER );

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
    QgsTiledSceneRendererAbstractMetadata *rendererMetadata( const QString &rendererName );

    /**
     * Returns a list of available renderers.
     */
    QStringList renderersList() const;

    /**
     * Returns a new default tiled scene renderer for a specified \a layer.
     *
     * Caller takes ownership of the returned renderer.
     */
    static QgsTiledSceneRenderer *defaultRenderer( const QgsTiledSceneLayer *layer ) SIP_FACTORY;

  private:
#ifdef SIP_RUN
    QgsTiledSceneRendererRegistry( const QgsTiledSceneRendererRegistry &rh );
#endif

    //! Map of name to renderer
    QMap<QString, QgsTiledSceneRendererAbstractMetadata *> mRenderers;

    //! List of renderers, maintained in the order that they have been added
    QStringList mRenderersOrder;
};

#endif // QGSTILEDSCENERENDERERREGISTRY_H
