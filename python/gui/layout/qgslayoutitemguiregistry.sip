/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutitemguiregistry.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayoutItemAbstractGuiMetadata
{
%Docstring
 Stores GUI metadata about one layout item class.

This is a companion to QgsLayoutItemAbstractMetadata, storing only
the components related to the GUI behavior of a layout item.

.. note::

   In C++ you can use QgsLayoutItemGuiMetadata convenience class.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemguiregistry.h"
%End
  public:

    enum Flag
    {
      FlagNoCreationTools,
    };
    typedef QFlags<QgsLayoutItemAbstractGuiMetadata::Flag> Flags;


    QgsLayoutItemAbstractGuiMetadata( int type, const QString &visibleName, const QString &groupId = QString(), bool isNodeBased = false, Flags flags = 0 );
%Docstring
Constructor for QgsLayoutItemAbstractGuiMetadata with the specified class ``type``.

``visibleName`` should be set to a translated, user visible name identifying the corresponding layout item.

An optional ``groupId`` can be set, which allows grouping of related layout item classes. See QgsLayoutItemGuiMetadata for details.

If ``isNodeBased`` is true, then the corresponding item is a node based item.
%End

    virtual ~QgsLayoutItemAbstractGuiMetadata();

    int type() const;
%Docstring
Returns the unique item type code for the layout item class.
%End

    Flags flags() const;
%Docstring
Returns item flags.
%End

    QString groupId() const;
%Docstring
Returns the item group ID, if set.
%End

    bool isNodeBased() const;
%Docstring
Returns true if the associated item is a node based item.
%End

    QString visibleName() const;
%Docstring
Returns a translated, user visible name identifying the corresponding layout item.
%End

    virtual QIcon creationIcon() const;
%Docstring
Returns an icon representing creation of the layout item type.
%End

    virtual QgsLayoutItemBaseWidget *createItemWidget( QgsLayoutItem *item ) /Factory/;
%Docstring
Creates a configuration widget for an ``item`` of this type. Can return None if no configuration GUI is required.
%End

    virtual QgsLayoutViewRubberBand *createRubberBand( QgsLayoutView *view ) /Factory/;
%Docstring
Creates a rubber band for use when creating layout items of this type. Can return None if no rubber band
should be created. The default behavior is to create a rectangular rubber band.

.. seealso:: :py:func:`createNodeRubberBand`
%End

    virtual QAbstractGraphicsShapeItem *createNodeRubberBand( QgsLayoutView *view ) /Factory/;
%Docstring
Creates a rubber band for use when creating layout node based items of this type. Can return None if no rubber band
should be created. The default behavior is to return None.

.. seealso:: :py:func:`createRubberBand`
%End

    virtual QgsLayoutItem *createItem( QgsLayout *layout ) /Factory/;
%Docstring
Creates an instance of the corresponding item type.
%End

    virtual void newItemAddedToLayout( QgsLayoutItem *item );
%Docstring
Called when a newly created item of the associated type has been added to a layout.

This is only called for additions which result from GUI operations - i.e. it is not
called for items added from templates.
%End

};






class QgsLayoutItemGuiGroup
{
%Docstring
 Stores GUI metadata about a group of layout item classes.

QgsLayoutItemGuiGroup stores settings about groups of related layout item classes
which should be presented to users grouped together.

For instance, the various basic shape creation tools would use :py:class:`QgsLayoutItemGuiGroup`
to display grouped within designer dialogs.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemguiregistry.h"
%End
  public:

    QgsLayoutItemGuiGroup( const QString &id = QString(), const QString &name = QString(), const QIcon &icon = QIcon() );
%Docstring
Constructor for QgsLayoutItemGuiGroup.
%End

    QString id;

    QString name;

    QIcon icon;

};


class QgsLayoutItemGuiRegistry : QObject
{
%Docstring
 Registry of available layout item GUI behavior.

QgsLayoutItemGuiRegistry is not usually directly created, but rather accessed through
:py:func:`QgsGui.layoutItemGuiRegistry()`

This acts as a companion to :py:class:`QgsLayoutItemRegistry`, handling only
the components related to the GUI behavior of layout items.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemguiregistry.h"
%End
  public:

    QgsLayoutItemGuiRegistry( QObject *parent = 0 );
%Docstring
Creates a new empty item GUI registry.

QgsLayoutItemGuiRegistry is not usually directly created, but rather accessed through
:py:func:`QgsGui.layoutItemGuiRegistry()`

.. seealso:: :py:func:`populate`
%End

    ~QgsLayoutItemGuiRegistry();


    QgsLayoutItemAbstractGuiMetadata *itemMetadata( int metadataId ) const;
%Docstring
Returns the metadata for the specified item ``metadataId``. Returns None if
a corresponding ``metadataId`` was not found in the registry.
%End

    bool addLayoutItemGuiMetadata( QgsLayoutItemAbstractGuiMetadata *metadata /Transfer/ );
%Docstring
Registers the gui metadata for a new layout item type. Takes ownership of the metadata instance.
%End

    bool addItemGroup( const QgsLayoutItemGuiGroup &group );
%Docstring
Registers a new item group with the registry. This must be done before calling
addLayoutItemGuiMetadata() for any item types associated with the group.

Returns true if group was added, or false if group could not be added (e.g. due to
duplicate id value).

.. seealso:: :py:func:`itemGroup`
%End

    const QgsLayoutItemGuiGroup &itemGroup( const QString &id );
%Docstring
Returns a reference to the item group with matching ``id``.

.. seealso:: :py:func:`addItemGroup`
%End

    QgsLayoutItem *createItem( int metadataId, QgsLayout *layout ) const /Factory/;
%Docstring
Creates a new instance of a layout item given the item metadata ``metadataId``, target ``layout``.
%End

    void newItemAddedToLayout( int metadataId, QgsLayoutItem *item );
%Docstring
Called when a newly created item of the associated metadata ``metadataId`` has been added to a layout.

This is only called for additions which result from GUI operations - i.e. it is not
called for items added from templates.
%End

    QgsLayoutItemBaseWidget *createItemWidget( QgsLayoutItem *item ) const /Factory/;
%Docstring
Creates a new instance of a layout item configuration widget for the specified ``item``.
%End



    QList< int > itemMetadataIds() const;
%Docstring
Returns a list of available item metadata ids handled by the registry.
%End

  signals:

    void typeAdded( int metadataId );
%Docstring
Emitted whenever a new item type is added to the registry, with the specified
``metadataId``.
%End

  private:
    QgsLayoutItemGuiRegistry( const QgsLayoutItemGuiRegistry &rh );
};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutitemguiregistry.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
