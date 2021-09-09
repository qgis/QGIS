/***************************************************************************
                            qgsannotationitemguiregistry.h
                            --------------------------
    begin                : September 2021
    copyright            : (C) 2021 by Nyall Dawson
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
#ifndef QGSANNOTATIONITEMGUIREGISTRY_H
#define QGSANNOTATIONITEMGUIREGISTRY_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgspathresolver.h"
#include "qgsannotationitemregistry.h"
#include "qgis.h"
#include <QIcon>
#include <functional>

class QgsAnnotationLayer;
class QgsAnnotationItem;
class QgsAnnotationItemBaseWidget;
class QgsCreateAnnotationItemMapToolInterface;
class QgsMapCanvas;
class QgsAdvancedDigitizingDockWidget;

/**
 * \ingroup gui
 * \brief Stores GUI metadata about one annotation item class.
 *
 * This is a companion to QgsAnnotationItemAbstractMetadata, storing only
 * the components related to the GUI behavior of an annotation item.
 *
 * \note In C++ you can use QgsAnnotationItemGuiMetadata convenience class.
 * \since QGIS 3.22
 */
class GUI_EXPORT QgsAnnotationItemAbstractGuiMetadata
{
  public:

    /**
     * Constructor for QgsAnnotationItemAbstractGuiMetadata with the specified class \a type.
     *
     * \a visibleName should be set to a translated, user visible name identifying the corresponding annotation item.
     *
     * An optional \a groupId can be set, which allows grouping of related annotation item classes. See QgsAnnotationItemGuiMetadata for details.
     */
    QgsAnnotationItemAbstractGuiMetadata( const QString &type, const QString &visibleName, const QString &groupId = QString(), Qgis::AnnotationItemGuiFlags flags = Qgis::AnnotationItemGuiFlags() )
      : mType( type )
      , mGroupId( groupId )
      , mName( visibleName )
      , mFlags( flags )
    {}

    virtual ~QgsAnnotationItemAbstractGuiMetadata() = default;

    /**
     * Returns the unique item type code for the annotation item class.
     */
    QString type() const { return mType; }

    /**
     * Returns item flags.
     */
    Qgis::AnnotationItemGuiFlags flags() const { return mFlags; }

    /**
     * Returns the item group ID, if set.
     */
    QString groupId() const { return mGroupId; }

    /**
     * Returns a translated, user visible name identifying the corresponding annotation item.
     */
    QString visibleName() const { return mName; }

    /**
     * Returns an icon representing creation of the annotation item type.
     */
    virtual QIcon creationIcon() const;

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
    virtual QgsAnnotationItemBaseWidget *createItemWidget( QgsAnnotationItem *item ) SIP_TRANSFERBACK;

    /**
     * Creates a map tool for a creating a new item of this type.
     *
     * May return NULLPTR if no map tool is available for creating the item.
     */
    virtual QgsCreateAnnotationItemMapToolInterface *createMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget ) SIP_TRANSFERBACK;

    /**
     * Creates an instance of the corresponding item type.
     */
    virtual QgsAnnotationItem *createItem() SIP_TRANSFERBACK;

    /**
     * Called when a newly created item of the associated type has been added to a \a layer.
     *
     * This is only called for additions which result from GUI operations - i.e. it is not
     * called for items added programmatically.
     */
    virtual void newItemAddedToLayer( QgsAnnotationItem *item, QgsAnnotationLayer *layer );

  private:

    QString mType;
    QString mGroupId;
    QString mName;
    Qgis::AnnotationItemGuiFlags mFlags;

};

//! Annotation item configuration widget creation function
typedef std::function<QgsAnnotationItemBaseWidget *( QgsAnnotationItem * )> QgsAnnotationItemWidgetFunc SIP_SKIP;

//! Create annotation map tool creation function
typedef std::function<QgsCreateAnnotationItemMapToolInterface *( QgsMapCanvas *, QgsAdvancedDigitizingDockWidget * )> QgsCreateAnnotationItemMapToolFunc SIP_SKIP;

//! Annotation item added to layer callback
typedef std::function<void ( QgsAnnotationItem *, QgsAnnotationLayer *layer )> QgsAnnotationItemAddedToLayerFunc SIP_SKIP;

#ifndef SIP_RUN

/**
 * \ingroup gui
 * \brief Convenience metadata class that uses static functions to handle annotation item GUI behavior.
 * \note not available in Python bindings
 * \since QGIS 3.22
 */
class GUI_EXPORT QgsAnnotationItemGuiMetadata : public QgsAnnotationItemAbstractGuiMetadata
{
  public:

    /**
     * Constructor for QgsAnnotationItemGuiMetadata with the specified class \a type
     * and \a creationIcon, and function pointers for the various
     * configuration widget creation functions.
     *
     * \a visibleName should be set to a translated, user visible name identifying the corresponding annotation item.
     *
     * An optional \a groupId can be set, which allows grouping of related annotation item classes. See QgsAnnotationItemGuiMetadata for details.
     */
    QgsAnnotationItemGuiMetadata( const QString &type, const QString &visibleName, const QIcon &creationIcon,
                                  const QgsAnnotationItemWidgetFunc &pfWidget = nullptr,
                                  const QString &groupId = QString(),
                                  Qgis::AnnotationItemGuiFlags flags = Qgis::AnnotationItemGuiFlags(),
                                  const QgsAnnotationItemCreateFunc &pfCreateFunc = nullptr,
                                  const QgsCreateAnnotationItemMapToolFunc &pfCreateMapToolFunc = nullptr )
      : QgsAnnotationItemAbstractGuiMetadata( type, visibleName, groupId, flags )
      , mIcon( creationIcon )
      , mWidgetFunc( pfWidget )
      , mCreateFunc( pfCreateFunc )
      , mCreateMapToolFunc( pfCreateMapToolFunc )
    {}

    /**
     * Returns the classes' configuration widget creation function.
     * \see setWidgetFunction()
     */
    QgsAnnotationItemWidgetFunc widgetFunction() const { return mWidgetFunc; }

    /**
     * Sets the classes' configuration widget creation \a function.
     * \see widgetFunction()
     */
    void setWidgetFunction( const QgsAnnotationItemWidgetFunc &function ) { mWidgetFunc = function; }

    /**
     * Returns the classes' create new item map tool creation function.
     * \see setCreateMapToolFunction()
     */
    QgsCreateAnnotationItemMapToolFunc createMapToolFunction() const { return mCreateMapToolFunc; }

    /**
     * Sets the classes' create new item map tool creation \a function.
     * \see createMapToolFunction()
     */
    void setCreateMapToolFunction( const QgsCreateAnnotationItemMapToolFunc &function ) { mCreateMapToolFunc = function; }

    /**
     * Returns the classes' item creation function.
     * \see setItemCreationFunction()
     */
    QgsAnnotationItemCreateFunc itemCreationFunction() const { return mCreateFunc; }

    /**
     * Sets the classes' item creation \a function.
     * \see itemCreationFunction()
     */
    void setItemCreationFunction( const QgsAnnotationItemCreateFunc &function ) { mCreateFunc = function; }

    /**
     * Returns the classes' item added to layer function.
     * \see setItemAddedToLayerFunction()
     */
    QgsAnnotationItemAddedToLayerFunc itemAddToLayerFunction() const { return mAddedToLayerFunc; }

    /**
     * Sets the classes' item creation \a function.
     * \see itemAddToLayerFunction()
     */
    void setItemAddedToLayerFunction( const QgsAnnotationItemAddedToLayerFunc &function ) { mAddedToLayerFunc = function; }

    QIcon creationIcon() const override;
    QgsAnnotationItemBaseWidget *createItemWidget( QgsAnnotationItem *item ) override;

    QgsAnnotationItem *createItem() override;
    void newItemAddedToLayer( QgsAnnotationItem *item, QgsAnnotationLayer *layer ) override;
    QgsCreateAnnotationItemMapToolInterface *createMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget ) override;

  protected:
    QIcon mIcon;
    QgsAnnotationItemWidgetFunc mWidgetFunc = nullptr;
    QgsAnnotationItemCreateFunc mCreateFunc = nullptr;
    QgsCreateAnnotationItemMapToolFunc mCreateMapToolFunc = nullptr;
    QgsAnnotationItemAddedToLayerFunc mAddedToLayerFunc = nullptr;

};

#endif

/**
 * \ingroup gui
 * \brief Stores GUI metadata about a group of annotation item classes.
 *
 * QgsAnnotationItemGuiGroup stores settings about groups of related annotation item classes
 * which should be presented to users grouped together.
 *
 * For instance, the various basic shape creation tools would use QgsAnnotationItemGuiGroup
 * to display grouped within toolbars.
 *
 * \since QGIS 3.22
 */
class GUI_EXPORT QgsAnnotationItemGuiGroup
{
  public:

    /**
     * Constructor for QgsAnnotationItemGuiGroup.
     */
    QgsAnnotationItemGuiGroup( const QString &id = QString(), const QString &name = QString(), const QIcon &icon = QIcon() )
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
 * \class QgsAnnotationItemGuiRegistry
 * \brief Registry of available annotation item GUI behavior.
 *
 * QgsAnnotationItemGuiRegistry is not usually directly created, but rather accessed through
 * QgsGui::annotationItemGuiRegistry().
 *
 * This acts as a companion to QgsAnnotationItemRegistry, handling only
 * the components related to the GUI behavior of annotation items.
 *
 * \since QGIS 3.22
 */
class GUI_EXPORT QgsAnnotationItemGuiRegistry : public QObject
{
    Q_OBJECT

  public:

    /**
     * Creates a new empty item GUI registry.
     *
     * QgsAnnotationItemGuiRegistry is not usually directly created, but rather accessed through
     * QgsGui::annotationItemGuiRegistry().
    */
    QgsAnnotationItemGuiRegistry( QObject *parent = nullptr );

    ~QgsAnnotationItemGuiRegistry() override;

    //! QgsAnnotationItemGuiRegistry cannot be copied.
    QgsAnnotationItemGuiRegistry( const QgsAnnotationItemGuiRegistry &rh ) = delete;
    //! QgsAnnotationItemGuiRegistry cannot be copied.
    QgsAnnotationItemGuiRegistry &operator=( const QgsAnnotationItemGuiRegistry &rh ) = delete;

    /**
     * Returns the metadata for the specified item \a metadataId. Returns NULLPTR if
     * a corresponding \a metadataId was not found in the registry.
     */
    QgsAnnotationItemAbstractGuiMetadata *itemMetadata( int metadataId ) const;

    /**
     * Returns the GUI item metadata ID which corresponds to the specified annotation item \a type.
     *
     * In the case that multiple GUI metadata classes exist for a single annotation item \a type then
     * only the first encountered GUI metadata ID will be returned.
     *
     * Returns -1 if no matching metadata is found in the GUI registry.
     */
    int metadataIdForItemType( const QString &type ) const;

    /**
     * Registers the gui metadata for a new annotation item type. Takes ownership of the metadata instance.
     */
    bool addAnnotationItemGuiMetadata( QgsAnnotationItemAbstractGuiMetadata *metadata SIP_TRANSFER );

    /**
     * Registers a new item group with the registry. This must be done before calling
     * addAnnotationItemGuiMetadata() for any item types associated with the group.
     *
     * Returns TRUE if group was added, or FALSE if group could not be added (e.g. due to
     * duplicate id value).
     *
     * \see itemGroup()
     */
    bool addItemGroup( const QgsAnnotationItemGuiGroup &group );

    /**
     * Returns a reference to the item group with matching \a id.
     * \see addItemGroup()
     */
    const QgsAnnotationItemGuiGroup &itemGroup( const QString &id );

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
     * Creates a new instance of an annotation item given the item metadata \a metadataId.
     */
    QgsAnnotationItem *createItem( int metadataId ) const SIP_TRANSFERBACK;

    /**
     * Called when a newly created item of the associated metadata \a metadataId has been added to a \a layer.
     *
     * This is only called for additions which result from GUI operations - i.e. it is not
     * called for items added programmatically.
     */
    void newItemAddedToLayer( int metadataId, QgsAnnotationItem *item, QgsAnnotationLayer *layer );

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
     * Creates a new instance of an annotation item configuration widget for the specified \a item.
     */
    QgsAnnotationItemBaseWidget *createItemWidget( QgsAnnotationItem *item ) const SIP_TRANSFERBACK;

    /**
     * Returns a list of available item metadata ids handled by the registry.
     */
    QList< int > itemMetadataIds() const;

    /**
     * Populates the registry with default items.
     */
    void addDefaultItems();

  signals:

    /**
     * Emitted whenever a new item type is added to the registry, with the specified
     * \a metadataId.
     */
    void typeAdded( int metadataId );

  private:
#ifdef SIP_RUN
    QgsAnnotationItemGuiRegistry( const QgsAnnotationItemGuiRegistry &rh );
#endif

    QMap< int, QgsAnnotationItemAbstractGuiMetadata *> mMetadata;

    QMap< QString, QgsAnnotationItemGuiGroup > mItemGroups;

};

#endif //QGSANNOTATIONITEMGUIREGISTRY_H



