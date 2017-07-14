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

    QgsLayoutItemAbstractGuiMetadata( int type );
%Docstring
 Constructor for QgsLayoutItemAbstractGuiMetadata with the specified class ``type``.
%End

    virtual ~QgsLayoutItemAbstractGuiMetadata();

    int type() const;
%Docstring
 Returns the unique item type code for the layout item class.
 :rtype: int
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
