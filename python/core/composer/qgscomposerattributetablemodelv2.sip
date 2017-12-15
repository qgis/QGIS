/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerattributetablemodelv2.h                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsComposerAttributeTableColumnModelV2: QAbstractTableModel
{
%Docstring
A model for displaying columns shown in a QgsComposerAttributeTableV2
%End

%TypeHeaderCode
#include "qgscomposerattributetablemodelv2.h"
%End
  public:

    enum ShiftDirection
    {
      ShiftUp,
      ShiftDown
    };

    QgsComposerAttributeTableColumnModelV2( QgsComposerAttributeTableV2 *composerTable, QObject *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsComposerAttributeTableColumnModel.
:param composerTable: QgsComposerAttributeTable the model is attached to
:param parent: optional parent
%End

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual QVariant data( const QModelIndex &index, int role ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );
    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

    virtual bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() );

    virtual bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() );

    virtual QModelIndex index( int row, int column, const QModelIndex &parent ) const;

    virtual QModelIndex parent( const QModelIndex &child ) const;


    bool moveRow( int row, ShiftDirection direction );
%Docstring
Moves the specified row up or down in the model. Used for rearranging the attribute tables
columns.

:return: true if the move is allowed
:rtype: bool

:param row: row in model representing attribute table column to move
:param direction: direction to move the attribute table column

.. versionadded:: 2.3
%End

    void resetToLayer();
%Docstring
Resets the attribute table's columns to match the source layer's fields. Remove all existing
attribute table columns and column customisations.

.. versionadded:: 2.3
%End

    QgsComposerTableColumn *columnFromIndex( const QModelIndex &index ) const;
%Docstring
Returns the QgsComposerTableColumn corresponding to an index in the model.

:return: QgsComposerTableColumn for specified index
:rtype: QgsComposerTableColumn

:param index: a QModelIndex

.. versionadded:: 2.3

.. seealso:: :py:func:`indexFromColumn`
%End

    QModelIndex indexFromColumn( QgsComposerTableColumn *column );
%Docstring
Returns a QModelIndex corresponding to a QgsComposerTableColumn in the model.

:return: QModelIndex for specified QgsComposerTableColumn
:rtype: QModelIndex

:param column: a QgsComposerTableColumn

.. versionadded:: 2.3

.. seealso:: :py:func:`columnFromIndex`
%End

    void setColumnAsSorted( QgsComposerTableColumn *column, Qt::SortOrder order );
%Docstring
Sets a specified column as a sorted column in the QgsComposerAttributeTable. The column will be
added to the end of the sort rank list, ie it will take the next largest available sort rank.
:param column: a QgsComposerTableColumn
:param order: sort order for column

.. versionadded:: 2.3

.. seealso:: :py:func:`removeColumnFromSort`

.. seealso:: :py:func:`moveColumnInSortRank`
%End

    void setColumnAsUnsorted( QgsComposerTableColumn *column );
%Docstring
Sets a specified column as an unsorted column in the QgsComposerAttributeTable. The column will be
removed from the sort rank list.
:param column: a QgsComposerTableColumn

.. versionadded:: 2.3

.. seealso:: :py:func:`setColumnAsSorted`
%End

    bool moveColumnInSortRank( QgsComposerTableColumn *column, ShiftDirection direction );
%Docstring
Moves a column up or down in the sort rank for the QgsComposerAttributeTable.
:param column: a QgsComposerTableColumn
:param direction: direction to move the column in the sort rank list

:rtype: bool

.. versionadded:: 2.3

.. seealso:: :py:func:`setColumnAsSorted`
%End

};



class QgsComposerTableSortColumnsProxyModelV2: QSortFilterProxyModel
{
%Docstring
Allows for filtering QgsComposerAttributeTable columns by columns which are sorted or unsorted
%End

%TypeHeaderCode
#include "qgscomposerattributetablemodelv2.h"
%End
  public:

    enum ColumnFilterType
    {
      ShowSortedColumns,
      ShowUnsortedColumns
    };

    QgsComposerTableSortColumnsProxyModelV2( QgsComposerAttributeTableV2 *composerTable, ColumnFilterType filterType, QObject *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsComposerTableSortColumnsProxyModel.
:param composerTable: QgsComposerAttributeTable the model is attached to
:param filterType: filter for columns, controls whether sorted or unsorted columns are shown
:param parent: optional parent
%End

    virtual bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;

    virtual QVariant data( const QModelIndex &index, int role ) const;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

    virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

    QgsComposerTableColumn *columnFromRow( int row );
%Docstring
Returns the QgsComposerTableColumn corresponding to a row in the proxy model.

:return: QgsComposerTableColumn for specified row
:rtype: QgsComposerTableColumn

:param row: a row number

.. versionadded:: 2.3

.. seealso:: :py:func:`columnFromIndex`
%End

    QgsComposerTableColumn *columnFromIndex( const QModelIndex &index ) const;
%Docstring
Returns the QgsComposerTableColumn corresponding to an index in the proxy model.

:return: QgsComposerTableColumn for specified index
:rtype: QgsComposerTableColumn

:param index: a QModelIndex

.. versionadded:: 2.3

.. seealso:: :py:func:`columnFromRow`

.. seealso:: :py:func:`columnFromSourceIndex`
%End

    QgsComposerTableColumn *columnFromSourceIndex( const QModelIndex &sourceIndex ) const;
%Docstring
Returns the QgsComposerTableColumn corresponding to an index from the source
QgsComposerAttributeTableColumnModel model.

:return: QgsComposerTableColumn for specified index from QgsComposerAttributeTableColumnModel
:rtype: QgsComposerTableColumn

:param sourceIndex: a QModelIndex

.. versionadded:: 2.3

.. seealso:: :py:func:`columnFromRow`

.. seealso:: :py:func:`columnFromIndex`
%End

    void resetFilter();
%Docstring
Invalidates the current filter used by the proxy model

.. versionadded:: 2.3
%End

  protected:
    virtual bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const;


};
/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerattributetablemodelv2.h                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
