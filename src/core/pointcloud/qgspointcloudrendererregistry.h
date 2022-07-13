/***************************************************************************
    qgspointcloudrendererregistry.h
    ---------------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPOINTCLOUDRENDERERREGISTRY_H
#define QGSPOINTCLOUDRENDERERREGISTRY_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgspointcloudclassifiedrenderer.h"
#include <QIcon>
#include <QMap>
#include <QStringList>
#include <QDomElement>

class QgsPointCloudRenderer;
class QgsReadWriteContext;
class QgsPointCloudLayer;
class QgsStyle;
#ifndef SIP_RUN
class QgsPointCloudRendererWidget SIP_EXTERNAL;
#endif

class QgsPointCloudAttributeCollection;
class QgsPointCloudDataProvider;

/**
 * \ingroup core
 * \brief Stores metadata about one point cloud renderer class.
 *
 * \note It's necessary to implement createRenderer() function.
 *   In C++ you can use QgsPointCloudRendererMetadata convenience class.
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudRendererAbstractMetadata
{
  public:

    /**
     * Constructor for QgsPointCloudRendererAbstractMetadata, with the specified \a name.
     *
     * The \a visibleName argument gives a translated, user friendly string identifying the renderer type.
     *
     * The \a icon argument can be used to specify an icon representing the renderer.
     */
    QgsPointCloudRendererAbstractMetadata( const QString &name, const QString &visibleName, const QIcon &icon = QIcon() )
      : mName( name )
      , mVisibleName( visibleName )
      , mIcon( icon )
    {}
    virtual ~QgsPointCloudRendererAbstractMetadata() = default;

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
    virtual QgsPointCloudRenderer *createRenderer( QDomElement &elem, const QgsReadWriteContext &context ) = 0 SIP_FACTORY;

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
    virtual QgsPointCloudRendererWidget *createRendererWidget( QgsPointCloudLayer *layer, QgsStyle *style, QgsPointCloudRenderer *oldRenderer ) SIP_FACTORY
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

typedef QgsPointCloudRenderer *( *QgsPointCloudRendererCreateFunc )( QDomElement &, const QgsReadWriteContext & ) SIP_SKIP;
typedef QgsPointCloudRendererWidget *( *QgsPointCloudRendererWidgetFunc )( QgsPointCloudLayer *, QgsStyle *, QgsPointCloudRenderer * ) SIP_SKIP;

/**
 * \ingroup core
 * \brief Convenience metadata class that uses static functions to create point cloud renderer and its widget.
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudRendererMetadata : public QgsPointCloudRendererAbstractMetadata
{
  public:

    /**
     * Construct metadata
     * \note not available in Python bindings
     */
    QgsPointCloudRendererMetadata( const QString &name,
                                   const QString &visibleName,
                                   QgsPointCloudRendererCreateFunc pfCreate,
                                   const QIcon &icon = QIcon(),
                                   QgsPointCloudRendererWidgetFunc pfWidget = nullptr ) SIP_SKIP
  : QgsPointCloudRendererAbstractMetadata( name, visibleName, icon )
    , mCreateFunc( pfCreate )
    , mWidgetFunc( pfWidget )
    {}

    QgsPointCloudRenderer *createRenderer( QDomElement &elem, const QgsReadWriteContext &context ) override SIP_FACTORY
    { return mCreateFunc ? mCreateFunc( elem, context ) : nullptr; }

#ifndef SIP_RUN
    QgsPointCloudRendererWidget *createRendererWidget( QgsPointCloudLayer *layer, QgsStyle *style, QgsPointCloudRenderer *renderer ) override SIP_FACTORY
    { return mWidgetFunc ? mWidgetFunc( layer, style, renderer ) : nullptr; }
#endif

    //! \note not available in Python bindings
    QgsPointCloudRendererCreateFunc createFunction() const { return mCreateFunc; } SIP_SKIP
    //! \note not available in Python bindings
    QgsPointCloudRendererWidgetFunc widgetFunction() const { return mWidgetFunc; } SIP_SKIP

    //! \note not available in Python bindings
    void setWidgetFunction( QgsPointCloudRendererWidgetFunc f ) { mWidgetFunc = f; } SIP_SKIP

  protected:
    //! pointer to function that creates an instance of the renderer when loading project / style
    QgsPointCloudRendererCreateFunc mCreateFunc;
    //! pointer to function that creates a widget for configuration of renderer's params
    QgsPointCloudRendererWidgetFunc mWidgetFunc;

  private:
#ifdef SIP_RUN
    QgsPointCloudRendererMetadata();
#endif

};


/**
 * \ingroup core
 * \class QgsPointCloudRendererRegistry
 * \brief Registry of 2D renderers for point clouds.
 *
 * QgsPointCloudRendererRegistry is not usually directly created, but rather accessed through
 * QgsApplication::pointCloudRendererRegistry().
 *
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsPointCloudRendererRegistry
{
  public:

    QgsPointCloudRendererRegistry();
    ~QgsPointCloudRendererRegistry();

    //! QgsPointCloudRendererRegistry cannot be copied.
    QgsPointCloudRendererRegistry( const QgsPointCloudRendererRegistry &rh ) = delete;
    //! QgsPointCloudRendererRegistry cannot be copied.
    QgsPointCloudRendererRegistry &operator=( const QgsPointCloudRendererRegistry &rh ) = delete;

    /**
     * Adds a renderer to the registry. Takes ownership of the metadata object.
     * \param metadata renderer metadata
     * \returns TRUE if renderer was added successfully, or FALSE if renderer could not
     * be added (e.g., a renderer with a duplicate name already exists)
     */
    bool addRenderer( QgsPointCloudRendererAbstractMetadata *metadata SIP_TRANSFER );

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
    QgsPointCloudRendererAbstractMetadata *rendererMetadata( const QString &rendererName );

    /**
     * Returns a list of available renderers.
     */
    QStringList renderersList() const;

    /**
     * Returns a new default point cloud renderer for a specified \a layer.
     *
     * Caller takes ownership of the returned renderer.
     */
    static QgsPointCloudRenderer *defaultRenderer( const QgsPointCloudLayer *layer ) SIP_FACTORY;

    /**
     * Returns a list of categories using the available Classification classes of a specified \a layer, along with
     * default colors and translated names for the 19 classes (0-18) of point data record formats 6-10
     */
    static QgsPointCloudCategoryList classificationAttributeCategories( const QgsPointCloudLayer *layer );
  private:
#ifdef SIP_RUN
    QgsPointCloudRendererRegistry( const QgsPointCloudRendererRegistry &rh );
#endif

    //! Map of name to renderer
    QMap<QString, QgsPointCloudRendererAbstractMetadata *> mRenderers;

    //! List of renderers, maintained in the order that they have been added
    QStringList mRenderersOrder;
};

#endif // QGSPOINTCLOUDRENDERERREGISTRY_H
