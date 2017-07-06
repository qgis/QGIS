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
#include "qgsapplication.h"
#include "qgspathresolver.h"
#include <QGraphicsItem> //for QGraphicsItem::UserType
#include <QIcon>
#include <functional>

#include "qgslayoutitem.h" // temporary

class QgsLayout;
class QgsLayoutView;
class QgsLayoutItem;
class QgsLayoutViewRubberBand;

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
     * Returns an icon representing the layout item type.
     */
    virtual QIcon icon() const { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicRectangle.svg" ) ); }

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
     * Creates a rubber band for use when creating layout items of this type. Can return nullptr if no rubber band
     * should be created.
     * \note not available in Python bindings. Python item subclasses must use QgsLayoutItemRegistryGuiUtils
     * to override the default rubber band creation function.
     */
    virtual QgsLayoutViewRubberBand *createRubberBand( QgsLayoutView *view ) SIP_SKIP { Q_UNUSED( view ); return nullptr; }

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

//! Layout rubber band creation function
typedef std::function<QgsLayoutViewRubberBand *( QgsLayoutView * )> QgsLayoutItemRubberBandFunc SIP_SKIP;

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
    QgsLayoutItemMetadata( int type, const QString &visibleName, const QIcon &icon,
                           QgsLayoutItemCreateFunc pfCreate,
                           QgsLayoutItemPathResolverFunc pfPathResolver = nullptr,
                           QgsLayoutItemWidgetFunc pfWidget = nullptr )
      : QgsLayoutItemAbstractMetadata( type, visibleName )
      , mIcon( icon )
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

    /**
     * Returns the classes' rubber band creation function.
     * \see setRubberBandCreationFunction()
     */
    QgsLayoutItemRubberBandFunc rubberBandCreationFunction() const { return mRubberBandFunc; }

    /**
     * Sets the classes' rubber band creation \a function.
     * \see rubberBandCreationFunction()
     */
    void setRubberBandCreationFunction( QgsLayoutItemRubberBandFunc function ) { mRubberBandFunc = function; }

    QIcon icon() const override { return mIcon.isNull() ? QgsLayoutItemAbstractMetadata::icon() : mIcon; }
    QgsLayoutItem *createItem( QgsLayout *layout, const QVariantMap &properties ) override { return mCreateFunc ? mCreateFunc( layout, properties ) : nullptr; }
    QWidget *createItemWidget() override { return mWidgetFunc ? mWidgetFunc() : nullptr; }
    QgsLayoutViewRubberBand *createRubberBand( QgsLayoutView *view ) override { return mRubberBandFunc ? mRubberBandFunc( view ) : nullptr; }

    void resolvePaths( QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving ) override
    {
      if ( mPathResolverFunc )
        mPathResolverFunc( properties, pathResolver, saving );
    }

  protected:
    QIcon mIcon;
    QgsLayoutItemCreateFunc mCreateFunc = nullptr;
    QgsLayoutItemWidgetFunc mWidgetFunc = nullptr;
    QgsLayoutItemRubberBandFunc mRubberBandFunc = nullptr;
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
     * Creates a new empty item registry.
     *
     * QgsLayoutItemRegistry is not usually directly created, but rather accessed through
     * QgsApplication::layoutItemRegistry().
     *
     * \see populate()
    */
    QgsLayoutItemRegistry( QObject *parent = nullptr );

    ~QgsLayoutItemRegistry();

    /**
     * Populates the registry with standard item types. If called on a non-empty registry
     * then this will have no effect and will return false.
     */
    bool populate();

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
     * Creates a new rubber band item for the specified item \a type and destination \a view.
     * \note not available from Python bindings
     */
    QgsLayoutViewRubberBand *createItemRubberBand( int type, QgsLayoutView *view ) const SIP_SKIP;

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

  signals:

    /**
     * Emitted whenever a new item type is added to the registry, with the specified
     * \a type and visible \a name.
     */
    void typeAdded( int type, const QString &name );

  private:
#ifdef SIP_RUN
    QgsLayoutItemRegistry( const QgsLayoutItemRegistry &rh );
#endif

    QMap<int, QgsLayoutItemAbstractMetadata *> mMetadata;

    QMap<int, QgsLayoutItemRubberBandFunc > mRubberBandFunctions;

    friend class QgsLayoutItemRegistryGuiUtils;

};

#ifndef SIP_RUN
///@cond TEMPORARY
//simple item for testing
class TestLayoutItem : public QgsLayoutItem
{
  public:

    TestLayoutItem( QgsLayout *layout );
    ~TestLayoutItem() {}

    //implement pure virtual methods
    int type() const { return QgsLayoutItemRegistry::LayoutItem + 102; }
    void draw( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget );

  private:
    QColor mColor;
};

///@endcond
#endif

#endif //QGSLAYOUTITEMREGISTRY_H



