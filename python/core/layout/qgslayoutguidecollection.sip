/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutguidecollection.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutGuide : QObject
{
%Docstring
 Contains the configuration for a single snap guide used by a layout.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutguidecollection.h"
%End
  public:

    QgsLayoutGuide( Qt::Orientation orientation, const QgsLayoutMeasurement &position, QgsLayoutItemPage *page );
%Docstring
Constructor for a new guide with the specified ``orientation`` and
initial ``position``.

A layout must be set by calling setLayout() before the guide can be used.
Adding the guide to a QgsLayoutGuideCollection will automatically set
the corresponding layout for you.
%End

    ~QgsLayoutGuide();

    QgsLayout *layout() const;
%Docstring
Returns the layout the guide belongs to.

.. seealso:: :py:func:`setLayout`
%End

    void setLayout( QgsLayout *layout );
%Docstring
Sets the ``layout`` the guide belongs to.

.. note::

   Adding the guide to a QgsLayoutGuideCollection will automatically set
the corresponding layout for you.

.. seealso:: :py:func:`layout`
%End

    Qt::Orientation orientation() const;
%Docstring
Returns the guide's orientation.
%End

    QgsLayoutMeasurement position() const;
%Docstring
Returns the guide's position within the page.

The position indicates either the horizontal or vertical position
of the guide, depending on the guide's orientation().

.. seealso:: :py:func:`setPosition`
%End

    void setPosition( const QgsLayoutMeasurement &position );
%Docstring
Sets the guide's ``position`` within the page.

The ``position`` argument indicates either the horizontal or vertical position
of the guide, depending on the guide's orientation().

.. seealso:: :py:func:`position`
%End

    QgsLayoutItemPage *page();
%Docstring
Returns the page the guide is contained within.

.. seealso:: :py:func:`setPage`
%End

    void setPage( QgsLayoutItemPage *page );
%Docstring
Sets the ``page`` the guide is contained within.

.. seealso:: :py:func:`page`
%End

    void update();
%Docstring
Updates the position of the guide's line item.
%End

    QGraphicsLineItem *item();
%Docstring
Returns the guide's line item.
%End

    double layoutPosition() const;
%Docstring
Returns the guide's position in absolute layout units.

.. seealso:: :py:func:`setLayoutPosition`
%End

    void setLayoutPosition( double position );
%Docstring
Sets the guide's ``position`` in absolute layout units.

.. seealso:: :py:func:`layoutPosition`
%End

  signals:

    void positionChanged();
%Docstring
Emitted when the guide's position is changed.
%End

};

class QgsLayoutGuideCollection : QAbstractTableModel, QgsLayoutSerializableObject
{
%Docstring
 Stores and manages the snap guides used by a layout.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutguidecollection.h"
%End
  public:

    enum Roles
    {
      OrientationRole,
      PositionRole,
      UnitsRole,
      PageRole,
      LayoutPositionRole,
    };

    QgsLayoutGuideCollection( QgsLayout *layout, QgsLayoutPageCollection *pageCollection );
%Docstring
Constructor for QgsLayoutGuideCollection belonging to the specified layout,
and linked to the specified ``pageCollection``.
%End
    ~QgsLayoutGuideCollection();

    virtual QString stringType() const;
    virtual QgsLayout *layout();


    virtual int rowCount( const QModelIndex & ) const;

    virtual int columnCount( const QModelIndex & ) const;

    virtual QVariant data( const QModelIndex &index, int role ) const;

    virtual bool setData( const QModelIndex &index, const QVariant &value, int role );

    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

     virtual QVariant headerData( int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole ) const;
    virtual bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() );


    void addGuide( QgsLayoutGuide *guide /Transfer/ );
%Docstring
Adds a ``guide`` to the collection. Ownership of the guide is transferred to the
collection, and the guide will automatically have the correct layout
set.
%End

    void removeGuide( QgsLayoutGuide *guide );
%Docstring
Removes the specified ``guide``, and deletes it.

.. seealso:: :py:func:`clear`
%End

    void setGuideLayoutPosition( QgsLayoutGuide *guide, double position );
%Docstring
Sets the absolute ``position`` (in layout coordinates) for ``guide`` within the layout.
%End

    void clear();
%Docstring
Removes all guides from the collection.

.. seealso:: :py:func:`removeGuide`
%End

    void applyGuidesToAllOtherPages( int sourcePage );
%Docstring
Resets all other pages' guides to match the guides from the specified ``sourcePage``.
%End

    void update();
%Docstring
Updates the position (and visibility) of all guide line items.
%End

    QList< QgsLayoutGuide * > guides();
%Docstring
Returns a list of all guides contained in the collection.
%End

    QList< QgsLayoutGuide * > guides( Qt::Orientation orientation, int page = -1 );
%Docstring
Returns the list of guides contained in the collection with the specified
``orientation`` and on a matching ``page``.
If ``page`` is -1, guides from all pages will be returned.

.. seealso:: :py:func:`guidesOnPage`
%End

    QList< QgsLayoutGuide * > guidesOnPage( int page );
%Docstring
Returns the list of guides contained on a matching ``page``.

.. seealso:: :py:func:`guides`
%End

    bool visible() const;
%Docstring
Returns true if the guide lines should be drawn.

.. seealso:: :py:func:`setVisible`
%End

    void setVisible( bool visible );
%Docstring
Sets whether the guide lines should be ``visible``.

.. seealso:: :py:func:`visible`
%End

    virtual bool writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const;

%Docstring
Stores the collection's state in a DOM element. The ``parentElement`` should refer to the parent layout's DOM element.

.. seealso:: :py:func:`readXml`
%End

    virtual bool readXml( const QDomElement &collectionElement, const QDomDocument &document, const QgsReadWriteContext &context );

%Docstring
Sets the collection's state from a DOM element. collectionElement is the DOM node corresponding to the collection.

.. seealso:: :py:func:`writeXml`
%End

};


class QgsLayoutGuideProxyModel : QSortFilterProxyModel
{
%Docstring
 Filters QgsLayoutGuideCollection models to guides of a single orientation (horizontal or vertical).

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutguidecollection.h"
%End
  public:

    explicit QgsLayoutGuideProxyModel( QObject *parent /TransferThis/, Qt::Orientation orientation, int page );
%Docstring
Constructor for QgsLayoutGuideProxyModel, filtered to guides of the specified ``orientation`` and ``page`` only.

Page numbers begin at 0.
%End

    void setPage( int page );
%Docstring
Sets the current ``page`` for filtering matching guides. Page numbers begin at 0.
%End

    virtual bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const;

    virtual bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutguidecollection.h                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
