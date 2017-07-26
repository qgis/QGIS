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

    enum Orientation
    {
      Horizontal,
      Vertical,
    };

    QgsLayoutGuide( Orientation orientation, const QgsLayoutMeasurement &position );
%Docstring
 Constructor for a new guide with the specified ``orientation`` and
 initial ``position``.

 A layout must be set by calling setLayout() before the guide can be used.
 Adding the guide to a QgsLayoutGuideCollection will automatically set
 the corresponding layout for you.
%End

    QgsLayout *layout() const;
%Docstring
 Returns the layout the guide belongs to.
.. seealso:: setLayout()
 :rtype: QgsLayout
%End

    void setLayout( QgsLayout *layout );
%Docstring
 Sets the ``layout`` the guide belongs to.

.. note::

   Adding the guide to a QgsLayoutGuideCollection will automatically set
 the corresponding layout for you.

.. seealso:: layout()
%End

    Orientation orientation() const;
%Docstring
 Returns the guide's orientation.
 :rtype: Orientation
%End

    QgsLayoutMeasurement position() const;
%Docstring
 Returns the guide's position within the page.

 The position indicates either the horizontal or vertical position
 of the guide, depending on the guide's orientation().

.. seealso:: setPosition()
 :rtype: QgsLayoutMeasurement
%End

    void setPosition( const QgsLayoutMeasurement &position );
%Docstring
 Sets the guide's ``position`` within the page.

 The ``position`` argument indicates either the horizontal or vertical position
 of the guide, depending on the guide's orientation().

.. seealso:: position()
%End

    int page() const;
%Docstring
 Returns the page number of the guide.

 Page numbering begins at 0.

.. seealso:: setPage()
 :rtype: int
%End

    void setPage( int page );
%Docstring
 Sets the ``page`` number of the guide.

 Page numbering begins at 0.

.. seealso:: page()
%End

    void update();
%Docstring
 Updates the position of the guide's line item.
%End

    QGraphicsLineItem *item();
%Docstring
 Returns the guide's line item.
 :rtype: QGraphicsLineItem
%End

    double layoutPosition() const;
%Docstring
 Returns the guide's position in absolute layout units.
 :rtype: float
%End

  signals:

    void positionChanged();
%Docstring
 Emitted when the guide's position is changed.
%End

};

class QgsLayoutGuideCollection : QAbstractTableModel
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

    QgsLayoutGuideCollection( QgsLayout *layout );
%Docstring
 Constructor for QgsLayoutGuideCollection belonging to the specified layout.
%End
    ~QgsLayoutGuideCollection();

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

    void update();
%Docstring
 Updates the position (and visibility) of all guide line items.
%End

    QList< QgsLayoutGuide * > guides( QgsLayoutGuide::Orientation orientation, int page = -1 );
%Docstring
 Returns the list of guides contained in the collection with the specified
 ``orientation`` and on a matching ``page``.
 If ``page`` is -1, guides from all pages will be returned.
 :rtype: list of QgsLayoutGuide
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

    explicit QgsLayoutGuideProxyModel( QObject *parent /TransferThis/, QgsLayoutGuide::Orientation orientation, int page );
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
