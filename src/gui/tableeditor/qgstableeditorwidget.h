// This file is part of CppSheets.
//
// Copyright 2018 Patrick Flynn <patrick_dev2000@outlook.com>
//
// CppSheets is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CppSheets is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CppSheets. If not, see <https://www.gnu.org/licenses/>.

#ifndef QGSTABLEEDITORWIDGET_H
#define QGSTABLEEDITORWIDGET_H

#include "qgis_gui.h"
#include "qgstablecell.h"
#include <QTableWidget>
#include <QStyledItemDelegate>


#ifndef SIP_RUN
///@cond PRIVATE
///
class QgsTableEditorDelegate : public QStyledItemDelegate
{
    Q_OBJECT
  public:
    QgsTableEditorDelegate( QObject *parent );
  protected:
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem & /*option*/, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;

};

///@endcond

#endif

/**
 * \ingroup gui
 * \class QgsTableEditorWidget
 *
 * A reusable widget for editing simple spreadsheet-style tables.
 *
 * Table content is retrieved and set using the QgsTableContents class. The editor
 * has support for table foreground and background colors, and numeric formats.
 *
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsTableEditorWidget : public QTableWidget
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsTableEditorWidget with the specified \a parent widget.
     */
    QgsTableEditorWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsTableEditorWidget() override;

    /**
     * Sets the \a contents to show in the editor widget.
     *
     * \see tableContents()
     */
    void setTableContents( const QgsTableContents &contents );

    /**
     * Returns the current contents of the editor widget table.
     *
     * \see setTableContents()
     */
    QgsTableContents tableContents() const;

    /**
     * Sets the numeric \a format to use for the currently selected cells.
     *
     * Ownership of \a format is transferred to the widget.
     *
     * \see selectionNumericFormat()
     */
    void setSelectionNumericFormat( QgsNumericFormat *format SIP_TRANSFER );

    /**
     * Returns the numeric format used for the currently selected cells, or
     * NULLPTR if the selection has no numeric format set.
     *
     * If the selected cells have a mix of different formats then NULLPTR
     * will be returned.
     *
     * \see setSelectionNumericFormat()
     * \see hasMixedSelectionNumericFormat()
     */
    QgsNumericFormat *selectionNumericFormat();

    /**
     * Returns TRUE if the current selection has a mix of numeric formats.
     *
     * \see selectionNumericFormat()
     */
    bool hasMixedSelectionNumericFormat();

    /**
     * Returns the foreground color for the currently selected cells.
     *
     * If the selected cells have a mix of different foreground colors then an
     * invalid color will be returned.
     *
     * \see setSelectionForegroundColor()
     * \see selectionBackgroundColor()
     */
    QColor selectionForegroundColor();

    /**
     * Returns the background color for the currently selected cells.
     *
     * If the selected cells have a mix of different background colors then an
     * invalid color will be returned.
     *
     * \see setSelectionBackgroundColor()
     * \see selectionForegroundColor()
     */
    QColor selectionBackgroundColor();

    /**
     * Returns the height (in millimeters) of the rows associated with the current selection,
     * or 0 if an automatic row height is desired, or -1 if the selection has mixed row heights.
     *
     * \see setSelectionRowHeight()
     */
    double selectionRowHeight();

    /**
     * Returns the width (in millimeters) of the columns associated with the current selection,
     * or 0 if an automatic column width is desired.
     *
     * \see setSelectionColumnWidth()
     */
    double selectionColumnWidth();

    /**
     * Returns the configured row height for the specified \a row, or 0 if an automatic height
     * should be used for the row.
     */
    double tableRowHeight( int row );

    /**
     * Returns the configured column width for the specified \a column, or 0 if an automatic width
     * should be used for the column.
     */
    double tableColumnWidth( int column );

    /**
     * Sets the configured row \a height for the specified \a row. Set \a height to 0
     * if an automatic height should be used for the row.
     *
     * This should be called after a call to setTableContents().
     *
     * \see tableRowHeight()
     */
    void setTableRowHeight( int row, double height );

    /**
     * Sets the configured column \a width for the specified \a column. Set \a width to 0
     * if an automatic width should be used for the column.
     *
     * This should be called after a call to setTableContents().
     *
     * \see tableColumnWidth()
     */
    void setTableColumnWidth( int column, double width );

    /**
     * Returns a list of the rows associated with the current table selected cells.
     *
     * \see columnsAssociatedWithSelection()
     */
    QList<int> rowsAssociatedWithSelection();

    /**
     * Returns a list of the columns associated with the current table selected cells.
     *
     * \see rowsAssociatedWithSelection()
     */
    QList<int> columnsAssociatedWithSelection();

  public slots:

    /**
     * Inserts new rows below the current selection.
     *
     * \see insertRowsAbove()
     */
    void insertRowsBelow();

    /**
     * Inserts new rows above the current selection.
     *
     * \see insertRowsBelow()
     */
    void insertRowsAbove();

    /**
     * Inserts new columns before the current selection.
     *
     * \see insertColumnsAfter()
     */
    void insertColumnsBefore();

    /**
     * Inserts new columns after the current selection.
     *
     * \see insertColumnsBefore()
     */
    void insertColumnsAfter();

    /**
     * Deletes all rows associated with the current selected cells.
     *
     * \see deleteColumns()
     */
    void deleteRows();

    /**
     * Deletes all columns associated with the current selected cells.
     *
     * \see deleteRows()
     */
    void deleteColumns();

    /**
     * Expands out the selection to include whole rows associated with the
     * current selected cells.
     * \see expandColumnSelection()
     */
    void expandRowSelection();

    /**
     * Expands out the selection to include whole columns associated with the
     * current selected cells.
     * \see expandRowSelection()
     */
    void expandColumnSelection();

    /**
     * Clears the contents of the currently selected cells.
     */
    void clearSelectedCells();

    /**
     * Sets the foreground color for the currently selected cells.
     *
     * \see selectionForegroundColor()
     * \see setSelectionBackgroundColor()
     */
    void setSelectionForegroundColor( const QColor &color );

    /**
     * Sets the background color for the currently selected cells.
     *
     * \see selectionBackgroundColor()
     * \see setSelectionForegroundColor()
     */
    void setSelectionBackgroundColor( const QColor &color );

    /**
     * Sets the row \a height (in millimeters) for the currently selected rows, or 0 for automatic row height.
     *
     * \see setSelectionColumnWidth()
     */
    void setSelectionRowHeight( double height );

    /**
     * Sets the column \a width (in millimeters) for the currently selected columns, or 0 for automatic column width.
     *
     * \see setSelectionRowHeight()
     */
    void setSelectionColumnWidth( double height );

  protected:
    void keyPressEvent( QKeyEvent *event ) override;

  signals:

    /**
     * Emitted whenever the table contents are changed.
     */
    void tableChanged();

    /**
     * Emitted whenever the active (or selected) cell changes in the widget.
     */
    void activeCellChanged();

  private:

    //! Custom roles
    enum Roles
    {
      PresetBackgroundColorRole = Qt::UserRole + 1,
      RowHeight,
      ColumnWidth
    };

    void updateHeaders();

    bool collectConsecutiveRowRange( const QModelIndexList &list, int &minRow, int &maxRow ) const;
    bool collectConsecutiveColumnRange( const QModelIndexList &list, int &minColumn, int &maxColumn ) const;
    QList< int > collectUniqueRows( const QModelIndexList &list ) const;
    QList< int > collectUniqueColumns( const QModelIndexList &list ) const;

    int mBlockSignals = 0;
    QHash< QTableWidgetItem *, QgsNumericFormat * > mNumericFormats;
    QMenu *mHeaderMenu = nullptr;

};

#endif // QGSTABLEEDITORWIDGET_H
