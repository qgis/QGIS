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
class QgsFillSymbol;

/**
 * \ingroup core
 * \brief Stores metadata about one layout item class.
 *
 * A companion class, QgsLayoutItemAbstractGuiMetadata, handles the
 * GUI behavior of QgsLayoutItems.
 *
 * \note In C++ you can use QgsLayoutItemMetadata convenience class.
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
     * Creates a layout item of this class for a specified \a layout.
     */
    virtual QgsLayoutItem *createItem( QgsLayout *layout ) = 0 SIP_FACTORY;

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
typedef std::function<QgsLayoutItem *( QgsLayout * )> QgsLayoutItemCreateFunc SIP_SKIP;

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
     * and \a visibleName, and function pointers for the various item creation functions.
     */
    QgsLayoutItemMetadata( int type, const QString &visibleName, const QIcon &icon,
                           QgsLayoutItemCreateFunc pfCreate,
                           QgsLayoutItemPathResolverFunc pfPathResolver = nullptr )
      : QgsLayoutItemAbstractMetadata( type, visibleName )
      , mIcon( icon )
      , mCreateFunc( pfCreate )
      , mPathResolverFunc( pfPathResolver )
    {}

    /**
     * Returns the classes' item creation function.
     */
    QgsLayoutItemCreateFunc createFunction() const { return mCreateFunc; }

    /**
     * Returns the classes' path resolver function.
     */
    QgsLayoutItemPathResolverFunc pathResolverFunction() const { return mPathResolverFunc; }

    QIcon icon() const override { return mIcon.isNull() ? QgsLayoutItemAbstractMetadata::icon() : mIcon; }
    QgsLayoutItem *createItem( QgsLayout *layout ) override { return mCreateFunc ? mCreateFunc( layout ) : nullptr; }

    void resolvePaths( QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving ) override
    {
      if ( mPathResolverFunc )
        mPathResolverFunc( properties, pathResolver, saving );
    }

  protected:
    QIcon mIcon;
    QgsLayoutItemCreateFunc mCreateFunc = nullptr;
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
 * A companion class, QgsLayoutItemGuiRegistry, handles the GUI behavior
 * of layout items.
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
      LayoutGroup, //!< Grouped item

      // known item types
      LayoutPage, //!< Page items
      LayoutMap, //!< Map item
      LayoutPicture, //!< Picture item
      LayoutLabel, //!< Label item
      LayoutLegend, //!< Legend item
      LayoutShape, //!< Shape item
      LayoutPolygon, //!< Polygon shape item
      LayoutPolyline, //!< Polyline shape item

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
    //! QgsLayoutItemRegistry cannot be copied.
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
     * Creates a new instance of a layout item given the item \a type, and target \a layout.
     */
    QgsLayoutItem *createItem( int type, QgsLayout *layout ) const SIP_FACTORY;

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

};

#ifndef SIP_RUN
///@cond TEMPORARY
//simple item for testing
#ifdef ANDROID
// For some reason, the Android NDK toolchain requires this to link properly.
// Note to self: Please try to remove this again once Qt ships their libs built with gcc-5
class CORE_EXPORT TestLayoutItem : public QgsLayoutItem
#else
class TestLayoutItem : public QgsLayoutItem
#endif
{
    Q_OBJECT

  public:

    TestLayoutItem( QgsLayout *layout );
    ~TestLayoutItem() = default;

    //implement pure virtual methods
    int type() const override { return QgsLayoutItemRegistry::LayoutItem + 1002; }
    virtual QString stringType() const override { return QStringLiteral( "ItemTest" ); }
    void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = nullptr ) override;

  private:
    QColor mColor;
    QgsFillSymbol *mShapeStyleSymbol = nullptr;
};
///@endcond
#endif

#endif //QGSLAYOUTITEMREGISTRY_H



