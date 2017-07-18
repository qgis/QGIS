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

    QgsLayoutItemAbstractGuiMetadata( int type, const QString &groupId = QString() );
%Docstring
 Constructor for QgsLayoutItemAbstractGuiMetadata with the specified class ``type``.

 An optional ``groupId`` can be set, which allows grouping of related layout item classes. See QgsLayoutItemGuiMetadata for details.
%End

    virtual ~QgsLayoutItemAbstractGuiMetadata();

    int type() const;
%Docstring
 Returns the unique item type code for the layout item class.
 :rtype: int
%End

    QString groupId() const;
%Docstring
 Returns the item group ID, if set.
 :rtype: str
%End

    virtual QIcon creationIcon() const;
%Docstring
 Returns an icon representing creation of the layout item type.
 :rtype: QIcon
%End

    virtual QWidget *createItemWidget() /Factory/;
%Docstring
 Creates a configuration widget for layout items of this type. Can return None if no configuration GUI is required.
 :rtype: QWidget
%End

    virtual QgsLayoutViewRubberBand *createRubberBand( QgsLayoutView *view ) /Factory/;
%Docstring
 Creates a rubber band for use when creating layout items of this type. Can return None if no rubber band
 should be created. The default behavior is to create a rectangular rubber band.
 :rtype: QgsLayoutViewRubberBand
%End

};




class QgsLayoutItemGuiGroup
{
%Docstring
 Stores GUI metadata about a group of layout item classes.

 QgsLayoutItemGuiGroup stores settings about groups of related layout item classes
 which should be presented to users grouped together.

 For instance, the various basic shape creation tools would use QgsLayoutItemGuiGroup
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
%Docstring
 Unique (untranslated) group ID string.
%End

    QString name;
%Docstring
 Translated group name.
%End

    QIcon icon;
%Docstring
 Icon for group.
%End

};


class QgsLayoutItemGuiRegistry : QObject
{
%Docstring
 Registry of available layout item GUI behavior.

 QgsLayoutItemGuiRegistry is not usually directly created, but rather accessed through
 QgsGui.layoutItemGuiRegistry().

 This acts as a companion to QgsLayoutItemRegistry, handling only
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
 QgsGui.layoutItemGuiRegistry().

.. seealso:: populate()
%End

    ~QgsLayoutItemGuiRegistry();

    bool populate();
%Docstring
 Populates the registry with standard item types. If called on a non-empty registry
 then this will have no effect and will return false.
 :rtype: bool
%End


    QgsLayoutItemAbstractGuiMetadata *itemMetadata( int type ) const;
%Docstring
 Returns the metadata for the specified item ``type``. Returns None if
 a corresponding type was not found in the registry.
 :rtype: QgsLayoutItemAbstractGuiMetadata
%End

    bool addLayoutItemGuiMetadata( QgsLayoutItemAbstractGuiMetadata *metadata /Transfer/ );
%Docstring
 Registers the gui metadata for a new layout item type. Takes ownership of the metadata instance.
 :rtype: bool
%End

    bool addItemGroup( const QgsLayoutItemGuiGroup &group );
%Docstring
 Registers a new item group with the registry. This must be done before calling
 addLayoutItemGuiMetadata() for any item types associated with the group.

 Returns true if group was added, or false if group could not be added (e.g. due to
 duplicate id value).

.. seealso:: itemGroup()
 :rtype: bool
%End

    const QgsLayoutItemGuiGroup &itemGroup( const QString &id );
%Docstring
 Returns a reference to the item group with matching ``id``.
.. seealso:: addItemGroup()
 :rtype: QgsLayoutItemGuiGroup
%End

    QWidget *createItemWidget( int type ) const /Factory/;
%Docstring
 Creates a new instance of a layout item configuration widget for the specified item ``type``.
 :rtype: QWidget
%End


    QList< int > itemTypes() const;
%Docstring
 Returns a list of available item types handled by the registry.
 :rtype: list of int
%End

  signals:

    void typeAdded( int type );
%Docstring
 Emitted whenever a new item type is added to the registry, with the specified
 ``type``.
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
