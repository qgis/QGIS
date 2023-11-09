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
#include "qgsproperty.h"
#include <QTableWidget>
#include <QPlainTextEdit>
#include <QStyledItemDelegate>


#ifndef SIP_RUN
///@cond PRIVATE

class QgsTableEditorTextEdit : public QPlainTextEdit
{
    Q_OBJECT
  public:
    QgsTableEditorTextEdit( QWidget *parent );

    /**
     * Sets whether the editor is an a "weak" editor mode, where any
     * cursor key presses will be ignored by the editor and deferred to the table instead.
     */
    void setWeakEditorMode( bool weakEditorMode );

    void setWidgetOwnsGeometry( bool value )
    {
      mWidgetOwnsGeometry = value;
    }

  public slots:

    void resizeToContents();

  protected:
    void changeEvent( QEvent *e ) override;

    void keyPressEvent( QKeyEvent *e ) override;

  private:

    void updateMinimumSize();

    bool mWeakEditorMode = false;
    int mOriginalWidth = -1;
    int mOriginalHeight = -1;
    bool mWidgetOwnsGeometry = false;

};

class QgsTableEditorDelegate : public QStyledItemDelegate
{
    Q_OBJECT
  public:
    QgsTableEditorDelegate( QObject *parent );

    /**
     * Sets whether the editor is an a "weak" editor mode, where any
     * cursor key presses will be ignored by the editor and deferred to the table instead.
     */
    void setWeakEditorMode( bool weakEditorMode );

  signals:

    void updateNumericFormatForIndex( const QModelIndex &index ) const;

  protected:
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem & /*option*/, const QModelIndex &index ) const override;
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;

  private:

    bool mWeakEditorMode = false;
    mutable QModelIndex mLastIndex;
};

///@endcond

#endif

/**
 * \ingroup gui
 * \class QgsTableEditorWidget
 *
 * \brief A reusable widget for editing simple spreadsheet-style tables.
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
     *
     * \deprecated use selectionTextFormat() instead.
     */
    Q_DECL_DEPRECATED QColor selectionForegroundColor() SIP_DEPRECATED;

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
     * Returns the horizontal alignment for the currently selected cells.
     *
     * If the returned value contains both horizontal and vertical alignment flags, then
     * the selected cells have a mix of different horizontal alignments.
     *
     * \see selectionVerticalAlignment()
     */
    Qt::Alignment selectionHorizontalAlignment();

    /**
     * Returns the horizontal alignment for the currently selected cells.
     *
     * If the returned value contains both horizontal and vertical alignment flags, then
     * the selected cells have a mix of different vertical alignments.
     *
     * \see selectionVerticalAlignment()
     */
    Qt::Alignment selectionVerticalAlignment();

    /**
     * Returns the QgsProperty used for the contents of the currently selected cells.
     *
     * If the returned value is a default constructed QgsProperty, then the selected
     * cells have a mix of different properties.
     *
     * \since QGIS 3.16
     */
    QgsProperty selectionCellProperty();

    /**
     * Returns the text format for the currently selected cells.
     *
     * Returns an invalid QgsTextFormat if the selection has mixed text format.
     *
     * \since QGIS 3.16
     */
    QgsTextFormat selectionTextFormat();

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

    /**
     * Returns the table header values.
     *
     * \see setTableHeaders()
     */
    QVariantList tableHeaders() const;

    /**
     * Returns TRUE if any header cells are selected.
     */
    bool isHeaderCellSelected();

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
     *
     * \deprecated Use setSelectionTextFormat() instead.
     */
    Q_DECL_DEPRECATED void setSelectionForegroundColor( const QColor &color ) SIP_DEPRECATED;

    /**
     * Sets the background color for the currently selected cells.
     *
     * \see selectionBackgroundColor()
     * \see setSelectionForegroundColor()
     */
    void setSelectionBackgroundColor( const QColor &color );

    /**
     * Sets the horizontal alignment for the currently selected cells.
     *
     * \see selectionHorizontalAlignment()
     * \see setSelectionVerticalAlignment()
     *
     * \since QGIS 3.16
     */
    void setSelectionHorizontalAlignment( Qt::Alignment alignment );

    /**
     * Sets the vertical alignment for the currently selected cells.
     *
     * \see selectionVerticalAlignment()
     * \see setSelectionHorizontalAlignment()
     *
     * \since QGIS 3.16
     */
    void setSelectionVerticalAlignment( Qt::Alignment alignment );

    /**
     * Sets the cell contents QgsProperty for the currently selected cells.
     *
     * \since QGIS 3.16
     */
    void setSelectionCellProperty( const QgsProperty &property );

    /**
     * Sets the text \a format for the selected cells.
     *
     * \since QGIS 3.16
     */
    void setSelectionTextFormat( const QgsTextFormat &format );

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

    /**
     * Sets whether the table includes a header row.
     */
    void setIncludeTableHeader( bool included );

    /**
     * Sets the table \a headers.
     *
     * \see tableHeaders()
     */
    void setTableHeaders( const QVariantList &headers );

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

  private slots:

    void updateNumericFormatForIndex( const QModelIndex &index );

  private:

    //! Custom roles
    enum Roles
    {
      PresetBackgroundColorRole = Qt::UserRole + 1,
      RowHeight,
      ColumnWidth,
      CellContent,
      TextFormat,
      HorizontalAlignment,
      VerticalAlignment,
      CellProperty,
    };

    void updateHeaders();

    bool collectConsecutiveRowRange( const QModelIndexList &list, int &minRow, int &maxRow ) const;
    bool collectConsecutiveColumnRange( const QModelIndexList &list, int &minColumn, int &maxColumn ) const;
    QList< int > collectUniqueRows( const QModelIndexList &list ) const;
    QList< int > collectUniqueColumns( const QModelIndexList &list ) const;

    int mBlockSignals = 0;
    QHash< QTableWidgetItem *, QgsNumericFormat * > mNumericFormats;
    QMenu *mHeaderMenu = nullptr;
    bool mIncludeHeader = false;
    bool mFirstSet = true;

    friend class QgsTableEditorDelegate;

};

#endif // QGSTABLEEDITORWIDGET_H
