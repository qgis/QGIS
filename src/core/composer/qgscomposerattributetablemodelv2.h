/***************************************************************************
                      qgscomposerattributetablemodelv2.h
                         --------------------
    begin                : September 2014
    copyright            : (C) 2014 by Nyall Dawson
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

#ifndef QGSCOMPOSERATTRIBUTETABLEMODELV2_H
#define QGSCOMPOSERATTRIBUTETABLEMODELV2_H

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

class QgsComposerAttributeTableV2;
class QgsComposerTableColumn;

//QgsComposerAttributeTableColumnModelV2

/**A model for displaying columns shown in a QgsComposerAttributeTableV2*/
class CORE_EXPORT QgsComposerAttributeTableColumnModelV2: public QAbstractTableModel
{
    Q_OBJECT

  public:

    /*! Controls whether a row/column is shifted up or down
     */
    enum ShiftDirection
    {
      ShiftUp, /*!< shift the row/column up */
      ShiftDown /*!< shift the row/column down */
    };

    /**Constructor for QgsComposerAttributeTableColumnModel.
     * @param composerTable QgsComposerAttributeTable the model is attached to
     * @param parent optional parent
     */
    QgsComposerAttributeTableColumnModelV2( QgsComposerAttributeTableV2 *composerTable, QObject *parent = 0 );
    virtual ~QgsComposerAttributeTableColumnModelV2();

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual QVariant data( const QModelIndex &index, int role ) const;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );
    Qt::ItemFlags flags( const QModelIndex &index ) const;
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() );
    bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() );
    QModelIndex index( int row, int column, const QModelIndex &parent ) const;
    QModelIndex parent( const QModelIndex &child ) const;

    /**Moves the specified row up or down in the model. Used for rearranging the attribute tables
     * columns.
     * @returns true if the move is allowed
     * @param row row in model representing attribute table column to move
     * @param direction direction to move the attribute table column
     * @note added in 2.3
     */
    bool moveRow( int row, ShiftDirection direction );

    /**Resets the attribute table's columns to match the source layer's fields. Remove all existing
     * attribute table columns and column customisations.
     * @note added in 2.3
     */
    void resetToLayer();

    /**Returns the QgsComposerTableColumn corresponding to an index in the model.
     * @returns QgsComposerTableColumn for specified index
     * @param index a QModelIndex
     * @note added in 2.3
     * @see indexFromColumn
     */
    QgsComposerTableColumn* columnFromIndex( const QModelIndex & index ) const;

    /**Returns a QModelIndex corresponding to a QgsComposerTableColumn in the model.
     * @returns QModelIndex for specified QgsComposerTableColumn
     * @param column a QgsComposerTableColumn
     * @note added in 2.3
     * @see columnFromIndex
     */
    QModelIndex indexFromColumn( QgsComposerTableColumn *column );

    /**Sets a specified column as a sorted column in the QgsComposerAttributeTable. The column will be
     * added to the end of the sort rank list, ie it will take the next largest available sort rank.
     * @param column a QgsComposerTableColumn
     * @param order sort order for column
     * @note added in 2.3
     * @see removeColumnFromSort
     * @see moveColumnInSortRank
     */
    void setColumnAsSorted( QgsComposerTableColumn *column, Qt::SortOrder order );

    /**Sets a specified column as an unsorted column in the QgsComposerAttributeTable. The column will be
     * removed from the sort rank list.
     * @param column a QgsComposerTableColumn
     * @note added in 2.3
     * @see setColumnAsSorted
     */
    void setColumnAsUnsorted( QgsComposerTableColumn * column );

    /**Moves a column up or down in the sort rank for the QgsComposerAttributeTable.
     * @param column a QgsComposerTableColumn
     * @param direction direction to move the column in the sort rank list
     * @note added in 2.3
     * @see setColumnAsSorted
     */
    bool moveColumnInSortRank( QgsComposerTableColumn * column, ShiftDirection direction );

  private:
    QgsComposerAttributeTableV2 * mComposerTable;

};


//QgsComposerTableSortColumnsProxyModelV2

/**Allows for filtering QgsComposerAttributeTable columns by columns which are sorted or unsorted*/
class CORE_EXPORT QgsComposerTableSortColumnsProxyModelV2: public QSortFilterProxyModel
{
    Q_OBJECT

  public:

    /*! Controls whether the proxy model shows sorted or unsorted columns
     */
    enum ColumnFilterType
    {
      ShowSortedColumns, /*!< show only sorted columns */
      ShowUnsortedColumns/*!< show only unsorted columns */
    };

    /**Constructor for QgsComposerTableSortColumnsProxyModel.
     * @param composerTable QgsComposerAttributeTable the model is attached to
     * @param filterType filter for columns, controls whether sorted or unsorted columns are shown
     * @param parent optional parent
     */
    QgsComposerTableSortColumnsProxyModelV2( QgsComposerAttributeTableV2 *composerTable, ColumnFilterType filterType, QObject *parent = 0 );

    virtual ~QgsComposerTableSortColumnsProxyModelV2();

    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;
    virtual QVariant data( const QModelIndex &index, int role ) const;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    Qt::ItemFlags flags( const QModelIndex &index ) const;
    virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );

    /**Returns the QgsComposerTableColumn corresponding to a row in the proxy model.
     * @returns QgsComposerTableColumn for specified row
     * @param row a row number
     * @note added in 2.3
     * @see columnFromIndex
     */
    QgsComposerTableColumn* columnFromRow( int row );

    /**Returns the QgsComposerTableColumn corresponding to an index in the proxy model.
     * @returns QgsComposerTableColumn for specified index
     * @param index a QModelIndex
     * @note added in 2.3
     * @see columnFromRow
     * @see columnFromSourceIndex
     */
    QgsComposerTableColumn* columnFromIndex( const QModelIndex & index ) const;

    /**Returns the QgsComposerTableColumn corresponding to an index from the source
     * QgsComposerAttributeTableColumnModel model.
     * @returns QgsComposerTableColumn for specified index from QgsComposerAttributeTableColumnModel
     * @param sourceIndex a QModelIndex
     * @note added in 2.3
     * @see columnFromRow
     * @see columnFromIndex
     */
    QgsComposerTableColumn* columnFromSourceIndex( const QModelIndex& sourceIndex ) const;

    /**Invalidates the current filter used by the proxy model
     * @note added in 2.3
     */
    void resetFilter();

  protected:
    bool filterAcceptsRow( int source_row, const QModelIndex & source_parent ) const;

  private:
    QgsComposerAttributeTableV2 * mComposerTable;
    ColumnFilterType mFilterType;

    /**Returns a list of QgsComposerTableColumns without a set sort rank
     * @returns QgsComposerTableColumns in attribute table without a sort rank
     * @note added in 2.3
     */
    QList<QgsComposerTableColumn*> columnsWithoutSortRank() const;

};
#endif // QGSCOMPOSERATTRIBUTETABLEMODELV2_H
