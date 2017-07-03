/***************************************************************************
                            qgslayoutitemregistry.h
                            ------------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTITEMREGISTRY_H
#define QGSLAYOUTITEMREGISTRY_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgspathresolver.h"
#include <QGraphicsItem> //for QGraphicsItem::UserType
#include <functional>

class QgsLayout;
class QgsLayoutItem;

/**
 * \ingroup core
 * \brief Stores metadata about one layout item class.
 * \note In C++ you can use QgsSymbolLayerMetadata convenience class.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemAbstractMetadata
{
  public:

    /**
     * Constructor for QgsLayoutItemAbstractMetadata with the specified class \a type
     * and \a visibleName.
     */
    QgsLayoutItemAbstractMetadata( int type, const QString &visibleName )
      : mType( type )
      , mVisibleName( visibleName )
    {}

    virtual ~QgsLayoutItemAbstractMetadata() = default;

    /**
     * Returns the unique item type code for the layout item class.
     */
    int type() const { return mType; }

    /**
     * Returns a translated, user visible name for the layout item class.
     */
    QString visibleName() const { return mVisibleName; }

    /**
     * Creates a layout item of this class for a specified \a layout, given the map of \a properties.
     */
    virtual QgsLayoutItem *createItem( QgsLayout *layout, const QVariantMap &properties ) = 0 SIP_FACTORY;

    /**
     * Creates a configuration widget for layout items of this type. Can return nullptr if no configuration GUI is required.
     */
    virtual QWidget *createItemWidget() SIP_FACTORY { return nullptr; }

    /**
     * Resolve paths in the item's \a properties (if there are any paths).
     * When \a saving is true, paths are converted from absolute to relative,
     * when \a saving is false, paths are converted from relative to absolute.
     * This ensures that paths in project files can be relative, but in item
     * instances the paths are always absolute.
     */
    virtual void resolvePaths( QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving )
    {
      Q_UNUSED( properties );
      Q_UNUSED( pathResolver );
      Q_UNUSED( saving );
    }

  private:

    int mType = -1;
    QString mVisibleName;
};

//! Layout item creation function
typedef std::function<QgsLayoutItem *( QgsLayout *, const QVariantMap & )> QgsLayoutItemCreateFunc SIP_SKIP;

//! Layout item configuration widget creation function
typedef std::function<QWidget *()> QgsLayoutItemWidgetFunc SIP_SKIP;

//! Layout item path resolver function
typedef std::function<void( QVariantMap &, const QgsPathResolver &, bool )> QgsLayoutItemPathResolverFunc SIP_SKIP;

#ifndef SIP_RUN

/**
 * \ingroup core
 * Convenience metadata class that uses static functions to create layout items and their configuration widgets.
 * \since QGIS 3.0
 * \note not available in Python bindings
 */
class CORE_EXPORT QgsLayoutItemMetadata : public QgsLayoutItemAbstractMetadata
{
  public:

    /**
     * Constructor for QgsLayoutItemMetadata with the specified class \a type
     * and \a visibleName, and function pointers for the various item and
     * configuration widget creation functions.
     */
    QgsLayoutItemMetadata( int type, const QString &visibleName,
                           QgsLayoutItemCreateFunc pfCreate,
                           QgsLayoutItemPathResolverFunc pfPathResolver = nullptr,
                           QgsLayoutItemWidgetFunc pfWidget = nullptr )
      : QgsLayoutItemAbstractMetadata( type, visibleName )
      , mCreateFunc( pfCreate )
      , mWidgetFunc( pfWidget )
      , mPathResolverFunc( pfPathResolver )
    {}

    /**
     * Returns the classes' item creation function.
     */
    QgsLayoutItemCreateFunc createFunction() const { return mCreateFunc; }

    /**
     * Returns the classes' configuration widget creation function.
     * \see setWidgetFunction()
     */
    QgsLayoutItemWidgetFunc widgetFunction() const { return mWidgetFunc; }

    /**
     * Returns the classes' path resolver function.
     */
    QgsLayoutItemPathResolverFunc pathResolverFunction() const { return mPathResolverFunc; }

    /**
     * Sets the classes' configuration widget creation \a function.
     * \see widgetFunction()
     */
    void setWidgetFunction( QgsLayoutItemWidgetFunc function ) { mWidgetFunc = function; }

    virtual QgsLayoutItem *createItem( QgsLayout *layout, const QVariantMap &properties ) override { return mCreateFunc ? mCreateFunc( layout, properties ) : nullptr; }
    virtual QWidget *createItemWidget() override { return mWidgetFunc ? mWidgetFunc() : nullptr; }
    virtual void resolvePaths( QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving ) override
    {
      if ( mPathResolverFunc )
        mPathResolverFunc( properties, pathResolver, saving );
    }

  protected:
    QgsLayoutItemCreateFunc mCreateFunc = nullptr;
    QgsLayoutItemWidgetFunc mWidgetFunc = nullptr;
    QgsLayoutItemPathResolverFunc mPathResolverFunc = nullptr;

};

#endif



/**
 * \ingroup core
 * \class QgsLayoutItemRegistry
 * \brief Registry of available layout item types.
 *
 * QgsLayoutItemRegistry is not usually directly created, but rather accessed through
 * QgsApplication::layoutItemRegistry().
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemRegistry : public QObject
{
    Q_OBJECT

  public:

    //! Item types
    enum ItemType
    {
      LayoutItem = QGraphicsItem::UserType + 100, //!< Base class for items

      // known item types
      LayoutPage, //!< Page items

      // item types provided by plugins
      PluginItem, //!< Starting point for plugin item types
    };

    /**
     * Creates a registry and populates it with standard item types.
     *
     * QgsLayoutItemRegistry is not usually directly created, but rather accessed through
     * QgsApplication::layoutItemRegistry().
    */
    QgsLayoutItemRegistry( QObject *parent = nullptr );

    ~QgsLayoutItemRegistry();

    //! QgsLayoutItemRegistry cannot be copied.
    QgsLayoutItemRegistry( const QgsLayoutItemRegistry &rh ) = delete;
    //! QgsLayoutItemRegistryQgsLayoutItemRegistry cannot be copied.
    QgsLayoutItemRegistry &operator=( const QgsLayoutItemRegistry &rh ) = delete;

    /**
     * Returns the metadata for the specified item \a type. Returns nullptr if
     * a corresponding type was not found in the registry.
     */
    QgsLayoutItemAbstractMetadata *itemMetadata( int type ) const;

    /**
     * Registers a new layout item type. Takes ownership of the metadata instance.
     */
    bool addLayoutItemType( QgsLayoutItemAbstractMetadata *metadata SIP_TRANSFER );

    /**
     * Creates a new instance of a layout item given the item \a type, target \a layout and \a properties.
     */
    QgsLayoutItem *createItem( int type, QgsLayout *layout, const QVariantMap &properties = QVariantMap() ) const SIP_FACTORY;

    /**
     * Creates a new instance of a layout item configuration widget for the specified item \a type.
     */
    QWidget *createItemWidget( int type ) const SIP_FACTORY;

    /**
     * Resolve paths in properties of a particular symbol layer.
     * This normally means converting relative paths to absolute paths when loading
     * and converting absolute paths to relative paths when saving.
     */
    void resolvePaths( int type, QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving ) const;

    /**
     * Returns a map of available item types to translated name.
     */
    QMap< int, QString> itemTypes() const;

  private:
#ifdef SIP_RUN
    QgsLayoutItemRegistry( const QgsLayoutItemRegistry &rh );
#endif

    QMap<int, QgsLayoutItemAbstractMetadata *> mMetadata;

};

#endif //QGSLAYOUTITEMREGISTRY_H



