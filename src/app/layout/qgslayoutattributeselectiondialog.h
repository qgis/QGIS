/***************************************************************************
                         qgslayoutattributeselectiondialog.h
                         -----------------------------------
    begin                : November 2017
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

#ifndef QGSLAYOUTATTRIBUTESELECTIONDIALOG_H
#define QGSLAYOUTATTRIBUTESELECTIONDIALOG_H

#include <QDialog>
#include <QMap>
#include <QSet>
#include <QItemDelegate>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include "qgis.h"
#include "ui_qgslayoutattributeselectiondialogbase.h"
#include "qgsexpressioncontextgenerator.h"

class QGridLayout;
class QgsVectorLayer;
class QPushButton;
class QgsLayoutItemAttributeTable;
class QgsLayoutAttributeTableColumnModel;
class QgsLayoutTableSortColumnsProxyModel;
class QgsLayoutTableAvailableSortProxyModel;
class QgsLayoutObject;
class QgsLayoutTableColumn;

//QgsLayoutAttributeTableColumnModel

/**
 * A model for displaying columns shown in a QgsLayoutAttributeTable
*/
class QgsLayoutAttributeTableColumnModel: public QAbstractTableModel
{
    Q_OBJECT

  public:

    /**
     * Controls whether a row/column is shifted up or down
     */
    enum ShiftDirection
    {
      ShiftUp, //!< Shift the row/column up
      ShiftDown //!< Shift the row/column down
    };

    /**
     * Constructor for QgsLayoutAttributeTableColumnModel.
     * \param table QgsLayoutItemAttributeTable the model is attached to
     * \param parent optional parent
     */
    QgsLayoutAttributeTableColumnModel( QgsLayoutItemAttributeTable *table, QObject *parent SIP_TRANSFERTHIS = nullptr );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;
    bool insertRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;
    QModelIndex index( int row, int column, const QModelIndex &parent ) const override;
    QModelIndex parent( const QModelIndex &child ) const override;

    /**
     * Moves the specified row up or down in the model. Used for rearranging the attribute tables
     * columns.
     * \returns true if the move is allowed
     * \param row row in model representing attribute table column to move
     * \param direction direction to move the attribute table column
     */
    bool moveRow( int row, ShiftDirection direction );

    /**
     * Resets the attribute table's columns to match the source layer's fields. Remove all existing
     * attribute table columns and column customizations.
     */
    void resetToLayer();

    /**
     * Returns the QgsLayoutTableColumn corresponding to an index in the model.
     * \see indexFromColumn()
     */
    QgsLayoutTableColumn *columnFromIndex( const QModelIndex &index ) const;

    /**
     * Returns a QModelIndex corresponding to a QgsLayoutTableColumn in the model.
     * \see columnFromIndex()
     */
    QModelIndex indexFromColumn( QgsLayoutTableColumn *column );

    /**
     * Sets a specified column as a sorted column in the QgsLayoutItemAttributeTable. The column will be
     * added to the end of the sort rank list, ie it will take the next largest available sort rank.
     * \see removeColumnFromSort()
     * \see moveColumnInSortRank()
     */
    void setColumnAsSorted( QgsLayoutTableColumn *column, Qt::SortOrder order );

    /**
     * Sets a specified column as an unsorted column in the QgsLayoutItemAttributeTable. The column will be
     * removed from the sort rank list.
     * \see setColumnAsSorted()
     */
    void setColumnAsUnsorted( QgsLayoutTableColumn *column );

    /**
     * Moves a column up or down in the sort rank for the QgsLayoutItemAttributeTable.
     * \see setColumnAsSorted()
     */
    bool moveColumnInSortRank( QgsLayoutTableColumn *column, ShiftDirection direction );

  private:
    QgsLayoutItemAttributeTable *mTable = nullptr;

};


//QgsLayoutTableSortColumnsProxyModel

/**
 * Allows for filtering QgsComposerAttributeTable columns by columns which are sorted or unsorted
*/
class QgsLayoutTableSortColumnsProxyModel: public QSortFilterProxyModel
{
    Q_OBJECT

  public:

    /**
     * Controls whether the proxy model shows sorted or unsorted columns
     */
    enum ColumnFilterType
    {
      ShowSortedColumns, //!< Show only sorted columns
      ShowUnsortedColumns//!< Show only unsorted columns
    };

    /**
     * Constructor for QgsLayoutTableSortColumnsProxyModel.
     * \param table QgsLayoutItemAttributeTable the model is attached to
     * \param filterType filter for columns, controls whether sorted or unsorted columns are shown
     * \param parent optional parent
     */
    QgsLayoutTableSortColumnsProxyModel( QgsLayoutItemAttributeTable *table, ColumnFilterType filterType, QObject *parent SIP_TRANSFERTHIS = nullptr );

    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    /**
     * Returns the QgsLayoutTableColumn corresponding to a row in the proxy model.
     * \see columnFromIndex()
     */
    QgsLayoutTableColumn *columnFromRow( int row );

    /**
     * Returns the QgsLayoutTableColumn corresponding to an index in the proxy model.
     * \see columnFromRow()
     * \see columnFromSourceIndex()
     */
    QgsLayoutTableColumn *columnFromIndex( const QModelIndex &index ) const;

    /**
     * Returns the QgsLayoutTableColumn corresponding to an index from the source
     * QgsLayoutItemAttributeTableColumnModel model.
     * \see columnFromRow()
     * \see columnFromIndex()
     */
    QgsLayoutTableColumn *columnFromSourceIndex( const QModelIndex &sourceIndex ) const;

    /**
     * Invalidates the current filter used by the proxy model
     */
    void resetFilter();

  protected:
    bool filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const override;

  private:
    QgsLayoutItemAttributeTable *mTable = nullptr;
    ColumnFilterType mFilterType;

    /**
     * Returns a list of QgsLayoutTableColumn without a set sort rank
     */
    QList<QgsLayoutTableColumn *> columnsWithoutSortRank() const;

};

// QgsLayoutColumnAlignmentDelegate

//! A delegate for showing column alignment as a combo box
class QgsLayoutColumnAlignmentDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsLayoutColumnAlignmentDelegate( QObject *parent = nullptr );
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

};


// QgsLayoutColumnSourceDelegate

//! A delegate for showing column attribute source as a QgsFieldExpressionWidget
class QgsLayoutColumnSourceDelegate : public QItemDelegate, private QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    QgsLayoutColumnSourceDelegate( QgsVectorLayer *vlayer, QObject *parent = nullptr, const QgsLayoutObject *layoutObject = nullptr );
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
  public slots:
    void commitAndCloseEditor();
  private:
    QgsVectorLayer *mVectorLayer = nullptr;
    const QgsLayoutObject *mLayoutObject = nullptr;
    QgsExpressionContext createExpressionContext() const override;
};

// QgsLayoutColumnWidthDelegate

//! A delegate for showing column width as a spin box
class QgsLayoutColumnWidthDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsLayoutColumnWidthDelegate( QObject *parent = nullptr );
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

};


// QgsLayoutColumnSortOrderDelegate

//! A delegate for showing column sort order as a combo box
class QgsLayoutColumnSortOrderDelegate : public QItemDelegate
{
    Q_OBJECT

  public:
    explicit QgsLayoutColumnSortOrderDelegate( QObject *parent = nullptr );
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;
    void updateEditorGeometry( QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

};


// QgsLayoutAttributeSelectionDialog

//! A dialog to select what attributes to display (in the table item), set the column properties and specify a sort order
class QgsLayoutAttributeSelectionDialog: public QDialog, private Ui::QgsLayoutAttributeSelectionDialogBase
{
    Q_OBJECT
  public:
    QgsLayoutAttributeSelectionDialog( QgsLayoutItemAttributeTable *table, QgsVectorLayer *vLayer, QWidget *parent = nullptr, Qt::WindowFlags f = nullptr );

    ~QgsLayoutAttributeSelectionDialog() override;

  private slots:
    void mRemoveColumnPushButton_clicked();
    void mAddColumnPushButton_clicked();
    void mColumnUpPushButton_clicked();
    void mColumnDownPushButton_clicked();
    void mResetColumnsPushButton_clicked();
    void mClearColumnsPushButton_clicked();
    void mAddSortColumnPushButton_clicked();
    void mRemoveSortColumnPushButton_clicked();
    void mSortColumnUpPushButton_clicked();
    void mSortColumnDownPushButton_clicked();

  private:
    QgsLayoutItemAttributeTable *mTable = nullptr;

    const QgsVectorLayer *mVectorLayer = nullptr;

    QgsLayoutAttributeTableColumnModel *mColumnModel = nullptr;

    QgsLayoutTableSortColumnsProxyModel *mSortedProxyModel = nullptr;

    QgsLayoutTableSortColumnsProxyModel *mAvailableSortProxyModel = nullptr;

    QgsLayoutColumnAlignmentDelegate *mColumnAlignmentDelegate = nullptr;
    QgsLayoutColumnSourceDelegate *mColumnSourceDelegate = nullptr;
    QgsLayoutColumnSortOrderDelegate *mColumnSortOrderDelegate = nullptr;
    QgsLayoutColumnWidthDelegate *mColumnWidthDelegate = nullptr;

};

#endif // QGSLAYOUTATTRIBUTESELECTIONDIALOG_H
