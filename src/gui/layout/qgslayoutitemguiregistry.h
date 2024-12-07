/***************************************************************************
                            qgslayoutitemguiregistry.h
                            --------------------------
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
#ifndef QGSLAYOUTITEMGUIREGISTRY_H
#define QGSLAYOUTITEMGUIREGISTRY_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgspathresolver.h"
#include "qgslayoutitemregistry.h"
#include <QGraphicsItem> //for QGraphicsItem::UserType
#include <QIcon>
#include <functional>

#include "qgslayoutitem.h" // temporary

class QgsLayout;
class QgsLayoutView;
class QgsLayoutItem;
class QgsLayoutViewRubberBand;
class QgsLayoutItemBaseWidget;

/**
 * \ingroup gui
 * \brief Stores GUI metadata about one layout item class.
 *
 * This is a companion to QgsLayoutItemAbstractMetadata, storing only
 * the components related to the GUI behavior of a layout item.
 *
 * \note In C++ you can use QgsLayoutItemGuiMetadata convenience class.
 */
class GUI_EXPORT QgsLayoutItemAbstractGuiMetadata
{
  public:
    //! Flags for controlling how a items behave in the GUI
    enum Flag SIP_ENUM_BASETYPE( IntFlag )
    {
      FlagNoCreationTools = 1 << 1, //!< Do not show item creation tools for the item type
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Constructor for QgsLayoutItemAbstractGuiMetadata with the specified class \a type.
     *
     * \a visibleName should be set to a translated, user visible name identifying the corresponding layout item.
     *
     * An optional \a groupId can be set, which allows grouping of related layout item classes. See QgsLayoutItemGuiMetadata for details.
     *
     * If \a isNodeBased is TRUE, then the corresponding item is a node based item.
     */
    QgsLayoutItemAbstractGuiMetadata( int type, const QString &visibleName, const QString &groupId = QString(), bool isNodeBased = false, Flags flags = QgsLayoutItemAbstractGuiMetadata::Flags() )
      : mType( type )
      , mGroupId( groupId )
      , mIsNodeBased( isNodeBased )
      , mName( visibleName )
      , mFlags( flags )
    {}

    virtual ~QgsLayoutItemAbstractGuiMetadata() = default;

    /**
     * Returns the unique item type code for the layout item class.
     */
    int type() const { return mType; }

    /**
     * Returns item flags.
     */
    Flags flags() const { return mFlags; }

    /**
     * Returns the item group ID, if set.
     */
    QString groupId() const { return mGroupId; }

    /**
     * Returns TRUE if the associated item is a node based item.
     */
    bool isNodeBased() const { return mIsNodeBased; }

    /**
     * Returns a translated, user visible name identifying the corresponding layout item.
     */
    QString visibleName() const { return mName; }

    /**
     * Returns an icon representing creation of the layout item type.
     */
    virtual QIcon creationIcon() const { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicRectangle.svg" ) ); }

    /*
     * IMPORTANT: While it seems like /Factory/ would be the correct annotations here, that's not
     * the case.
     * As per Phil Thomson's advice on https://www.riverbankcomputing.com/pipermail/pyqt/2017-July/039450.html:
     *
     * "
     * /Factory/ is used when the instance returned is guaranteed to be new to Python.
     * In this case it isn't because it has already been seen when being returned by QgsProcessingAlgorithm::createInstance()
     * (However for a different sub-class implemented in C++ then it would be the first time it was seen
     * by Python so the /Factory/ on create() would be correct.)
     *
     * You might try using /TransferBack/ on create() instead - that might be the best compromise.
     * "
     */

    /**
     * Creates a configuration widget for an \a item of this type. Can return NULLPTR if no configuration GUI is required.
     */
    virtual QgsLayoutItemBaseWidget *createItemWidget( QgsLayoutItem *item ) SIP_TRANSFERBACK
    {
      Q_UNUSED( item )
      return nullptr;
    }

    /**
     * Creates a rubber band for use when creating layout items of this type. Can return NULLPTR if no rubber band
     * should be created. The default behavior is to create a rectangular rubber band.
     * \see createNodeRubberBand()
     */
    virtual QgsLayoutViewRubberBand *createRubberBand( QgsLayoutView *view ) SIP_TRANSFERBACK;

    /**
     * Creates a rubber band for use when creating layout node based items of this type. Can return NULLPTR if no rubber band
     * should be created. The default behavior is to return NULLPTR.
     * \see createRubberBand()
     */
    virtual QGraphicsItem *createNodeRubberBand( QgsLayoutView *view ) SIP_TRANSFERBACK;

    /**
     * Creates an instance of the corresponding item type.
     */
    virtual QgsLayoutItem *createItem( QgsLayout *layout ) SIP_TRANSFERBACK;

    /**
     * Called when a newly created item of the associated type has been added to a layout.
     *
     * This is only called for additions which result from GUI operations - i.e. it is not
     * called for items added from templates.
     */
    virtual void newItemAddedToLayout( QgsLayoutItem *item );

    /**
     * Called when a layout item is double-clicked.
     * The action parameter is used to specify which mouse handle, if any, was clicked
     * If no mouse handle is selected, Qgis::MouseHandlesAction::NoAction is used
     *
     * \since QGIS 3.42
     */
    virtual void handleDoubleClick( QgsLayoutItem *item, Qgis::MouseHandlesAction action );

  private:
    int mType = -1;
    QString mGroupId;
    bool mIsNodeBased = false;
    QString mName;
    Flags mFlags;
};

//! Layout item configuration widget creation function
typedef std::function<QgsLayoutItemBaseWidget *( QgsLayoutItem * )> QgsLayoutItemWidgetFunc SIP_SKIP;

//! Layout rubber band creation function
typedef std::function<QgsLayoutViewRubberBand *( QgsLayoutView * )> QgsLayoutItemRubberBandFunc SIP_SKIP;

//! Layout node based rubber band creation function
typedef std::function<QGraphicsItem *( QgsLayoutView * )> QgsLayoutNodeItemRubberBandFunc SIP_SKIP;

//! Layout item added to layout callback
typedef std::function<void( QgsLayoutItem *, const QVariantMap & )> QgsLayoutItemAddedToLayoutFunc SIP_SKIP;

//! Layout item double clicked
typedef std::function<void( QgsLayoutItem *, Qgis::MouseHandlesAction action )> QgsLayoutItemDoubleClickedFunc SIP_SKIP;

#ifndef SIP_RUN

/**
 * \ingroup gui
 * \brief Convenience metadata class that uses static functions to handle layout item GUI behavior.
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsLayoutItemGuiMetadata : public QgsLayoutItemAbstractGuiMetadata
{
  public:
    /**
     * Constructor for QgsLayoutItemGuiMetadata with the specified class \a type
     * and \a creationIcon, and function pointers for the various
     * configuration widget creation functions.
     *
     * \a visibleName should be set to a translated, user visible name identifying the corresponding layout item.
     *
     * An optional \a groupId can be set, which allows grouping of related layout item classes. See QgsLayoutItemGuiMetadata for details.
     *
     * If \a isNodeBased is TRUE, then the corresponding item is a node based item.
     */
    QgsLayoutItemGuiMetadata( int type, const QString &visibleName, const QIcon &creationIcon, const QgsLayoutItemWidgetFunc &pfWidget = nullptr, const QgsLayoutItemRubberBandFunc &pfRubberBand = nullptr, const QString &groupId = QString(), bool isNodeBased = false, QgsLayoutItemAbstractGuiMetadata::Flags flags = QgsLayoutItemAbstractGuiMetadata::Flags(), const QgsLayoutItemCreateFunc &pfCreateFunc = nullptr )
      : QgsLayoutItemAbstractGuiMetadata( type, visibleName, groupId, isNodeBased, flags )
      , mIcon( creationIcon )
      , mWidgetFunc( pfWidget )
      , mRubberBandFunc( pfRubberBand )
      , mCreateFunc( pfCreateFunc )
    {}

    /**
     * Returns the classes' configuration widget creation function.
     * \see setWidgetFunction()
     */
    QgsLayoutItemWidgetFunc widgetFunction() const { return mWidgetFunc; }

    /**
     * Sets the classes' configuration widget creation \a function.
     * \see widgetFunction()
     */
    void setWidgetFunction( const QgsLayoutItemWidgetFunc &function ) { mWidgetFunc = function; }

    /**
     * Returns the classes' rubber band creation function.
     * \see setRubberBandCreationFunction()
     */
    QgsLayoutItemRubberBandFunc rubberBandCreationFunction() const { return mRubberBandFunc; }

    /**
     * Sets the classes' rubber band creation \a function.
     * \see rubberBandCreationFunction()
     */
    void setRubberBandCreationFunction( const QgsLayoutItemRubberBandFunc &function ) { mRubberBandFunc = function; }

    /**
     * Returns the classes' node based rubber band creation function.
     * \see setNodeRubberBandCreationFunction()
     */
    QgsLayoutNodeItemRubberBandFunc nodeRubberBandCreationFunction() const { return mNodeRubberBandFunc; }

    /**
     * Sets the classes' node based rubber band creation \a function.
     * \see nodeRubberBandCreationFunction()
     */
    void setNodeRubberBandCreationFunction( const QgsLayoutNodeItemRubberBandFunc &function ) { mNodeRubberBandFunc = function; }

    /**
     * Returns the classes' item creation function.
     * \see setItemCreationFunction()
     */
    QgsLayoutItemCreateFunc itemCreationFunction() const { return mCreateFunc; }

    /**
     * Sets the classes' item creation \a function.
     * \see itemCreationFunction()
     */
    void setItemCreationFunction( const QgsLayoutItemCreateFunc &function ) { mCreateFunc = function; }

    /**
     * Returns the classes' item added to layout function.
     * \see setItemAddedToLayoutFunction()
     */
    QgsLayoutItemAddedToLayoutFunc itemAddToLayoutFunction() const { return mAddedToLayoutFunc; }

    /**
     * Sets the classes' item creation \a function.
     * \see itemAddToLayoutFunction()
     */
    void setItemAddedToLayoutFunction( const QgsLayoutItemAddedToLayoutFunc &function ) { mAddedToLayoutFunc = function; }

    /**
     * Returns the classes' item double clicked function.
     * \see setItemAddedToLayoutFunction()
     */
    QgsLayoutItemDoubleClickedFunc itemDoubleClickedFunction() const { return mDoubleClickedFunc; }

    /**
     * Sets the classes' item double clicked \a function.
     * \see itemDoubleClickedFunction()
     */
    void setItemDoubleClickedFunction( const QgsLayoutItemDoubleClickedFunc &function ) { mDoubleClickedFunc = function; }

    void handleDoubleClick( QgsLayoutItem *item, Qgis::MouseHandlesAction action ) override;

    QIcon creationIcon() const override { return mIcon.isNull() ? QgsLayoutItemAbstractGuiMetadata::creationIcon() : mIcon; }
    QgsLayoutItemBaseWidget *createItemWidget( QgsLayoutItem *item ) override { return mWidgetFunc ? mWidgetFunc( item ) : nullptr; }
    QgsLayoutViewRubberBand *createRubberBand( QgsLayoutView *view ) override { return mRubberBandFunc ? mRubberBandFunc( view ) : nullptr; }
    QGraphicsItem *createNodeRubberBand( QgsLayoutView *view ) override { return mNodeRubberBandFunc ? mNodeRubberBandFunc( view ) : nullptr; }

    QgsLayoutItem *createItem( QgsLayout *layout ) override;
    void newItemAddedToLayout( QgsLayoutItem *item ) override;

    /**
     * Called when a newly created item of the associated type has been added to a layout.
     *
     * This is only called for additions which result from GUI operations - i.e. it is not
     * called for items added from templates.
     *
     * The \a properties map will be filled with any custom properties which were specified during
     * the item creation.
     *
     * \since QGIS 3.18
     */
    void newItemAddedToLayout( QgsLayoutItem *item, const QVariantMap &properties );

  protected:
    QIcon mIcon;
    QgsLayoutItemWidgetFunc mWidgetFunc = nullptr;
    QgsLayoutItemRubberBandFunc mRubberBandFunc = nullptr;
    QgsLayoutNodeItemRubberBandFunc mNodeRubberBandFunc = nullptr;
    QgsLayoutItemCreateFunc mCreateFunc = nullptr;
    QgsLayoutItemAddedToLayoutFunc mAddedToLayoutFunc = nullptr;
    QgsLayoutItemDoubleClickedFunc mDoubleClickedFunc = nullptr;
};

#endif

/**
 * \ingroup gui
 * \brief Stores GUI metadata about a group of layout item classes.
 *
 * QgsLayoutItemGuiGroup stores settings about groups of related layout item classes
 * which should be presented to users grouped together.
 *
 * For instance, the various basic shape creation tools would use QgsLayoutItemGuiGroup
 * to display grouped within designer dialogs.
 *
 */
class GUI_EXPORT QgsLayoutItemGuiGroup
{
  public:
    /**
     * Constructor for QgsLayoutItemGuiGroup.
     */
    QgsLayoutItemGuiGroup( const QString &id = QString(), const QString &name = QString(), const QIcon &icon = QIcon() )
      : id( id )
      , name( name )
      , icon( icon )
    {}

    /**
     * Unique (untranslated) group ID string.
     */
    QString id;

    /**
     * Translated group name.
     */
    QString name;

    /**
     * Icon for group.
     */
    QIcon icon;
};


/**
 * \ingroup core
 * \class QgsLayoutItemGuiRegistry
 * \brief Registry of available layout item GUI behavior.
 *
 * QgsLayoutItemGuiRegistry is not usually directly created, but rather accessed through
 * QgsGui::layoutItemGuiRegistry().
 *
 * This acts as a companion to QgsLayoutItemRegistry, handling only
 * the components related to the GUI behavior of layout items.
 *
 */
class GUI_EXPORT QgsLayoutItemGuiRegistry : public QObject
{
    Q_OBJECT

  public:
    /**
     * Creates a new empty item GUI registry.
     *
     * QgsLayoutItemGuiRegistry is not usually directly created, but rather accessed through
     * QgsGui::layoutItemGuiRegistry().
    */
    QgsLayoutItemGuiRegistry( QObject *parent = nullptr );

    ~QgsLayoutItemGuiRegistry() override;

    QgsLayoutItemGuiRegistry( const QgsLayoutItemGuiRegistry &rh ) = delete;
    QgsLayoutItemGuiRegistry &operator=( const QgsLayoutItemGuiRegistry &rh ) = delete;

    /**
     * Returns the metadata for the specified item \a metadataId. Returns NULLPTR if
     * a corresponding \a metadataId was not found in the registry.
     */
    QgsLayoutItemAbstractGuiMetadata *itemMetadata( int metadataId ) const;

    /**
     * Returns the GUI item metadata ID which corresponds to the specified layout item \a type.
     *
     * In the case that multiple GUI metadata classes exist for a single layout item \a type then
     * only the first encountered GUI metadata ID will be returned.
     *
     * Returns -1 if no matching metadata is found in the GUI registry.
     *
     * \since QGIS 3.18
     */
    int metadataIdForItemType( int type ) const;

    /**
     * Registers the gui metadata for a new layout item type. Takes ownership of the metadata instance.
     */
    bool addLayoutItemGuiMetadata( QgsLayoutItemAbstractGuiMetadata *metadata SIP_TRANSFER );

    /**
     * Registers a new item group with the registry. This must be done before calling
     * addLayoutItemGuiMetadata() for any item types associated with the group.
     *
     * Returns TRUE if group was added, or FALSE if group could not be added (e.g. due to
     * duplicate id value).
     *
     * \see itemGroup()
     */
    bool addItemGroup( const QgsLayoutItemGuiGroup &group );

    /**
     * Returns a reference to the item group with matching \a id.
     * \see addItemGroup()
     */
    const QgsLayoutItemGuiGroup &itemGroup( const QString &id );

    /*
     * IMPORTANT: While it seems like /Factory/ would be the correct annotations here, that's not
     * the case.
     * As per Phil Thomson's advice on https://www.riverbankcomputing.com/pipermail/pyqt/2017-July/039450.html:
     *
     * "
     * /Factory/ is used when the instance returned is guaranteed to be new to Python.
     * In this case it isn't because it has already been seen when being returned by QgsProcessingAlgorithm::createInstance()
     * (However for a different sub-class implemented in C++ then it would be the first time it was seen
     * by Python so the /Factory/ on create() would be correct.)
     *
     * You might try using /TransferBack/ on create() instead - that might be the best compromise.
     * "
     */

    /**
     * Creates a new instance of a layout item given the item metadata \a metadataId, target \a layout.
     */
    QgsLayoutItem *createItem( int metadataId, QgsLayout *layout ) const SIP_TRANSFERBACK;

    /**
     * Called when a newly created item of the associated metadata \a metadataId has been added to a layout.
     *
     * This is only called for additions which result from GUI operations - i.e. it is not
     * called for items added from templates.
     *
     * Since QGIS 3.18 the optional \a properties argument can be used to pass custom properties to the
     * QgsLayoutItemGuiMetadata::newItemAddedToLayout() function.
     */
    void newItemAddedToLayout( int metadataId, QgsLayoutItem *item, const QVariantMap &properties = QVariantMap() );

    /*
     * IMPORTANT: While it seems like /Factory/ would be the correct annotations here, that's not
     * the case.
     * As per Phil Thomson's advice on https://www.riverbankcomputing.com/pipermail/pyqt/2017-July/039450.html:
     *
     * "
     * /Factory/ is used when the instance returned is guaranteed to be new to Python.
     * In this case it isn't because it has already been seen when being returned by QgsProcessingAlgorithm::createInstance()
     * (However for a different sub-class implemented in C++ then it would be the first time it was seen
     * by Python so the /Factory/ on create() would be correct.)
     *
     * You might try using /TransferBack/ on create() instead - that might be the best compromise.
     * "
     */

    /**
     * Creates a new instance of a layout item configuration widget for the specified \a item.
     */
    QgsLayoutItemBaseWidget *createItemWidget( QgsLayoutItem *item ) const SIP_TRANSFERBACK;

    /**
     * Creates a new rubber band item for the specified item \a metadataId and destination \a view.
     * \note not available from Python bindings
     * \see createNodeItemRubberBand()
     */
    QgsLayoutViewRubberBand *createItemRubberBand( int metadataId, QgsLayoutView *view ) const SIP_SKIP;

    /**
     * Creates a rubber band for the specified item \a metadataId and destination \a view.
     * Can return NULLPTR if no node based rubber band should be created or is applicable for the item.
     * \see createItemRubberBand()
     * \note not available from Python bindings
     */
    QGraphicsItem *createNodeItemRubberBand( int metadataId, QgsLayoutView *view ) SIP_SKIP;

    /**
     * Returns a list of available item metadata ids handled by the registry.
     */
    QList<int> itemMetadataIds() const;

  signals:

    /**
     * Emitted whenever a new item type is added to the registry, with the specified
     * \a metadataId.
     */
    void typeAdded( int metadataId );

  private:
#ifdef SIP_RUN
    QgsLayoutItemGuiRegistry( const QgsLayoutItemGuiRegistry &rh );
#endif

    QMap<int, QgsLayoutItemAbstractGuiMetadata *> mMetadata;

    QMap<QString, QgsLayoutItemGuiGroup> mItemGroups;
};

#endif //QGSLAYOUTITEMGUIREGISTRY_H
