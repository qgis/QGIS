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
class QgsLayoutMultiFrame;

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
     * Returns a translated, user visible name for the layout item class.
     */
    QString visibleName() const { return mVisibleName; }

    /**
     * Creates a layout item of this class for a specified \a layout.
     */
    virtual QgsLayoutItem *createItem( QgsLayout *layout ) = 0 SIP_FACTORY;

    /**
     * Resolve paths in the item's \a properties (if there are any paths).
     * When \a saving is TRUE, paths are converted from absolute to relative,
     * when \a saving is FALSE, paths are converted from relative to absolute.
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
 * \note not available in Python bindings
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemMetadata : public QgsLayoutItemAbstractMetadata
{
  public:

    /**
     * Constructor for QgsLayoutItemMetadata with the specified class \a type
     * and \a visibleName, and function pointers for the various item creation functions.
     */
    QgsLayoutItemMetadata( int type, const QString &visibleName,
                           const QgsLayoutItemCreateFunc &pfCreate,
                           const QgsLayoutItemPathResolverFunc &pfPathResolver = nullptr )
      : QgsLayoutItemAbstractMetadata( type, visibleName )
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

    QgsLayoutItem *createItem( QgsLayout *layout ) override { return mCreateFunc ? mCreateFunc( layout ) : nullptr; }

    void resolvePaths( QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving ) override
    {
      if ( mPathResolverFunc )
        mPathResolverFunc( properties, pathResolver, saving );
    }

  protected:
    QgsLayoutItemCreateFunc mCreateFunc = nullptr;
    QgsLayoutItemPathResolverFunc mPathResolverFunc = nullptr;

};

#endif

/**
 * \ingroup core
 * \brief Stores metadata about one layout multiframe class.
 *
 * A companion class, QgsLayoutMultiFrameAbstractGuiMetadata, handles the
 * GUI behavior of QgsLayoutMultiFrames.
 *
 * \note In C++ you can use QgsLayoutMultiFrameMetadata convenience class.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutMultiFrameAbstractMetadata
{
  public:

    /**
     * Constructor for QgsLayoutMultiFrameAbstractMetadata with the specified class \a type
     * and \a visibleName.
     */
    QgsLayoutMultiFrameAbstractMetadata( int type, const QString &visibleName )
      : mType( type )
      , mVisibleName( visibleName )
    {}

    virtual ~QgsLayoutMultiFrameAbstractMetadata() = default;

    /**
     * Returns the unique item type code for the layout multiframe class.
     */
    int type() const { return mType; }

    /**
     * Returns an icon representing the layout multiframe type.
     */
    virtual QIcon icon() const { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicRectangle.svg" ) ); }

    /**
     * Returns a translated, user visible name for the layout multiframe class.
     */
    QString visibleName() const { return mVisibleName; }

    /**
     * Creates a layout multiframe of this class for a specified \a layout.
     */
    virtual QgsLayoutMultiFrame *createMultiFrame( QgsLayout *layout ) = 0 SIP_FACTORY;

    /**
     * Resolve paths in the item's \a properties (if there are any paths).
     * When \a saving is TRUE, paths are converted from absolute to relative,
     * when \a saving is FALSE, paths are converted from relative to absolute.
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

//! Layout multiframe creation function
typedef std::function<QgsLayoutMultiFrame *( QgsLayout * )> QgsLayoutMultiFrameCreateFunc SIP_SKIP;

//! Layout multiframe path resolver function
typedef std::function<void( QVariantMap &, const QgsPathResolver &, bool )> QgsLayoutMultiFramePathResolverFunc SIP_SKIP;

#ifndef SIP_RUN

/**
 * \ingroup core
 * Convenience metadata class that uses static functions to create layout multiframes and their configuration widgets.
 * \note not available in Python bindings
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutMultiFrameMetadata : public QgsLayoutMultiFrameAbstractMetadata
{
  public:

    /**
     * Constructor for QgsLayoutMultiFrameMetadata with the specified class \a type
     * and \a visibleName, and function pointers for the various item creation functions.
     */
    QgsLayoutMultiFrameMetadata( int type, const QString &visibleName,
                                 const QgsLayoutMultiFrameCreateFunc &pfCreate,
                                 const QgsLayoutMultiFramePathResolverFunc &pfPathResolver = nullptr )
      : QgsLayoutMultiFrameAbstractMetadata( type, visibleName )
      , mCreateFunc( pfCreate )
      , mPathResolverFunc( pfPathResolver )
    {}

    /**
     * Returns the classes' multiframe creation function.
     */
    QgsLayoutMultiFrameCreateFunc createFunction() const { return mCreateFunc; }

    /**
     * Returns the classes' path resolver function.
     */
    QgsLayoutMultiFramePathResolverFunc pathResolverFunction() const { return mPathResolverFunc; }

    QgsLayoutMultiFrame *createMultiFrame( QgsLayout *layout ) override { return mCreateFunc ? mCreateFunc( layout ) : nullptr; }

    void resolvePaths( QVariantMap &properties, const QgsPathResolver &pathResolver, bool saving ) override
    {
      if ( mPathResolverFunc )
        mPathResolverFunc( properties, pathResolver, saving );
    }

  protected:
    QgsLayoutMultiFrameCreateFunc mCreateFunc = nullptr;
    QgsLayoutMultiFramePathResolverFunc mPathResolverFunc = nullptr;

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

      // WARNING!!!! SIP CASTING OF QgsLayoutItem and QgsLayoutMultiFrame DEPENDS on these
      // values, and must be updated if any additional types are added

      LayoutPage, //!< Page items
      LayoutMap, //!< Map item
      LayoutPicture, //!< Picture item
      LayoutLabel, //!< Label item
      LayoutLegend, //!< Legend item
      LayoutShape, //!< Shape item
      LayoutPolygon, //!< Polygon shape item
      LayoutPolyline, //!< Polyline shape item
      LayoutScaleBar, //!< Scale bar item
      LayoutFrame, //!< Frame item, part of a QgsLayoutMultiFrame object

      // known multi-frame types

      // WARNING!!!! SIP CASTING OF QgsLayoutItem and QgsLayoutMultiFrame DEPENDS on these
      // values, and must be updated if any additional types are added

      LayoutHtml, //!< Html multiframe item
      LayoutAttributeTable, //!< Attribute table
      LayoutTextTable, //!< Preset text table

      Layout3DMap,  //!< 3D map item

      // item types provided by plugins
      PluginItem = LayoutTextTable + 10000, //!< Starting point for plugin item types
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

    ~QgsLayoutItemRegistry() override;

    /**
     * Populates the registry with standard item types. If called on a non-empty registry
     * then this will have no effect and will return FALSE.
     */
    bool populate();

    //! QgsLayoutItemRegistry cannot be copied.
    QgsLayoutItemRegistry( const QgsLayoutItemRegistry &rh ) = delete;
    //! QgsLayoutItemRegistry cannot be copied.
    QgsLayoutItemRegistry &operator=( const QgsLayoutItemRegistry &rh ) = delete;

    /**
     * Returns the metadata for the specified item \a type. Returns NULLPTR if
     * a corresponding type was not found in the registry.
     * \see multiFrameMetadata()
     */
    QgsLayoutItemAbstractMetadata *itemMetadata( int type ) const;

    /**
     * Returns the metadata for the specified multiframe \a type. Returns NULLPTR if
     * a corresponding type was not found in the registry.
     * \see itemMetadata()
     */
    QgsLayoutMultiFrameAbstractMetadata *multiFrameMetadata( int type ) const;

    /**
     * Registers a new layout item type. Takes ownership of the metadata instance.
     * \see addLayoutMultiFrameType()
     */
    bool addLayoutItemType( QgsLayoutItemAbstractMetadata *metadata SIP_TRANSFER );

    /**
     * Registers a new layout multiframe type. Takes ownership of the metadata instance.
     * \see addLayoutItemType()
     */
    bool addLayoutMultiFrameType( QgsLayoutMultiFrameAbstractMetadata *metadata SIP_TRANSFER );

    /**
     * Creates a new instance of a layout item given the item \a type, and target \a layout.
     * \see createMultiFrame()
     */
    QgsLayoutItem *createItem( int type, QgsLayout *layout ) const SIP_FACTORY;

    /**
     * Creates a new instance of a layout multiframe given the multiframe \a type, and target \a layout.
     * \see createItem()
     */
    QgsLayoutMultiFrame *createMultiFrame( int type, QgsLayout *layout ) const SIP_FACTORY;

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

    /**
     * Emitted whenever a new multiframe type is added to the registry, with the specified
     * \a type and visible \a name.
     */
    void multiFrameTypeAdded( int type, const QString &name );

  private:
#ifdef SIP_RUN
    QgsLayoutItemRegistry( const QgsLayoutItemRegistry &rh );
#endif

    QMap<int, QgsLayoutItemAbstractMetadata *> mMetadata;
    QMap<int, QgsLayoutMultiFrameAbstractMetadata *> mMultiFrameMetadata;

};

#if 0
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
    void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = nullptr ) override;

  private:
    QColor mColor;
    QgsFillSymbol *mShapeStyleSymbol = nullptr;
};
///@endcond
#endif
#endif

#endif //QGSLAYOUTITEMREGISTRY_H



