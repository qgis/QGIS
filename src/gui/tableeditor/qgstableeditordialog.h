/***************************************************************************
    qgstableeditordialog.h
    ---------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSTABLEEDITORDIALOG_H
#define QGSTABLEEDITORDIALOG_H

#include "qgis_gui.h"
#include "qgstablecell.h"
#include "ui_qgstableeditorbase.h"
#include <QMainWindow>

class QgsTableEditorWidget;
class QgsMessageBar;
class QgsDockWidget;
class QgsPanelWidgetStack;
class QgsTableEditorFormattingWidget;
class QgsExpressionContextGenerator;

/**
 * \ingroup gui
 * \class QgsTableEditorDialog
 *
 * A reusable window for editing simple spreadsheet-style tables.
 *
 * Table content is retrieved and set using the QgsTableContents class. The editor
 * has support for table foreground and background colors, and numeric formats.
 *
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsTableEditorDialog : public QMainWindow, private Ui::QgsTableEditorBase
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsTableEditorDialog with the specified \a parent widget.
     */
    QgsTableEditorDialog( QWidget *parent = nullptr );

    /**
     * Sets the \a contents to show in the editor widget.
     *
     * \see tableContents()
     */
    void setTableContents( const QgsTableContents &contents );

    /**
     * Parses the clipboard text into contents to show in the editor widget.
     * \returns TRUE if the clipboard was successfully parsed
     *
     * \see tableContents()
     */

    bool setTableContentsFromClipboard();

    /**
     * Returns the current contents of the editor widget table.
     *
     * \see setTableContents()
     */
    QgsTableContents tableContents() const;

    /**
     * Returns the configured row height for the specified \a row, or 0 if an automatic height
     * should be used for the row.
     *
     * \see setTableRowHeight()
     */
    double tableRowHeight( int row );

    /**
     * Returns the configured column width for the specified \a column, or 0 if an automatic width
     * should be used for the column.
     *
     * \see setTableColumnWidth()
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
     * Returns TRUE if the table includes a header row.
     *
     * \see setIncludeTableHeader()
     */
    bool includeTableHeader() const;

    /**
     * Sets whether the table includes a header row.
     *
     * \see includeTableHeader()
     */
    void setIncludeTableHeader( bool included );

    /**
     * Returns the table header values.
     *
     * \see setTableHeaders()
     */
    QVariantList tableHeaders() const;

    /**
     * Sets the table \a headers.
     *
     * \see tableHeaders()
     */
    void setTableHeaders( const QVariantList &headers );

    /**
     * Register an expression context generator class that will be used to retrieve
     * an expression context for the editor when required.
     * \since QGIS 3.16
     */
    void registerExpressionContextGenerator( QgsExpressionContextGenerator *generator );

  signals:

    /**
     * Emitted whenever the table contents are changed.
     */
    void tableChanged();

    /**
     * Emitted whenever the "include table header" setting is changed.
     */
    void includeHeaderChanged( bool included );

  private:
    QgsTableEditorWidget *mTableWidget = nullptr;
    QgsMessageBar *mMessageBar = nullptr;
    QgsDockWidget *mPropertiesDock = nullptr;
    QgsPanelWidgetStack *mPropertiesStack = nullptr;
    QgsTableEditorFormattingWidget *mFormattingWidget = nullptr;
    bool mBlockSignals = false;

    void updateActionNamesFromSelection();
};

#endif // QGSTABLEEDITORSHEETWIDGET_H
