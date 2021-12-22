/***************************************************************************
    qgstableeditorformattingwidget.h
    --------------------------------
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
#ifndef QGSTABLEEDITORFORMATTINGWIDGET_H
#define QGSTABLEEDITORFORMATTINGWIDGET_H

#include "qgis_gui.h"
#include "ui_qgstableeditorformattingwidgetbase.h"
#include "qgspanelwidget.h"
#include "qgsexpressioncontextgenerator.h"
#include <memory>

#define SIP_NO_FILE

class QgsNumericFormat;
class QgsProperty;

/**
 * \ingroup gui
 * \class QgsTableEditorFormattingWidget
 *
 * \brief A reusable widget for formatting the contents of a QgsTableCell.
 *
 * The editor has support for table foreground and background colors, and numeric formats.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsTableEditorFormattingWidget : public QgsPanelWidget, public QgsExpressionContextGenerator, private Ui::QgsTableEditorFormattingWidgetBase
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsTableEditorFormattingWidget with the specified \a parent widget.
     */
    QgsTableEditorFormattingWidget( QWidget *parent = nullptr );
    ~QgsTableEditorFormattingWidget() override;

    /**
     * Returns the current numeric format shown in the widget, or a NULLPTR
     * if no numeric format is set.
     *
     * The caller takes ownership of the returned object.
     *
     * \see setNumericFormat()
     */
    QgsNumericFormat *numericFormat() SIP_TRANSFERBACK;

    /**
     * Returns the current text format shown in the widget.
     *
     * \see setTextFormat()
     * \since QGIS 3.16
     */
    QgsTextFormat textFormat() const;

    /**
     * Sets the cell background \a color to show in the widget.
     *
     * \see backgroundColorChanged()
     */
    void setBackgroundColor( const QColor &color );

    /**
     * Sets the numeric \a format to show in the widget, or NULLPTR if no numeric format is set.
     *
     * if \a isMixedFormat is TRUE then the widget will be set to indicate a "mixed format" setting.
     *
     * \see numericFormat()
     */
    void setNumericFormat( QgsNumericFormat *format, bool isMixedFormat );

    /**
     * Sets the text \a format to show in the widget.
     *
     * \see textFormat()
     * \since QGIS 3.16
     */
    void setTextFormat( const QgsTextFormat &format );

    /**
     * Sets the row \a height to show in the widget, or 0 for automatic height.
     *
     * \see rowHeightChanged()
     * \see setColumnWidth()
     */
    void setRowHeight( double height );

    /**
     * Sets the column \a width to show in the widget, or 0 for automatic width.
     *
     * \see columnWidthChanged()
     * \see setRowHeight()
     */
    void setColumnWidth( double width );

    /**
     * Sets the horizontal \a alignment to show in the widget.
     *
     * \see horizontalAlignmentChanged()
     * \see setVerticalAlignment()
     *
     * \since QGIS 3.16
     */
    void setHorizontalAlignment( Qt::Alignment alignment );

    /**
     * Sets the vertical \a alignment to show in the widget.
     *
     * \see verticalAlignmentChanged()
     * \see setHorizontalAlignment()
     *
     * \since QGIS 3.16
     */
    void setVerticalAlignment( Qt::Alignment alignment );

    /**
     * Sets the cell content's \a property to show in the widget.
     *
     * \since QGIS 3.16
     */
    void setCellProperty( const QgsProperty &property );

    /**
     * Register an expression context generator class that will be used to retrieve
     * an expression context for the widget when required.
     * \since QGIS 3.16
     */
    void registerExpressionContextGenerator( QgsExpressionContextGenerator *generator );

    QgsExpressionContext createExpressionContext() const override;

  signals:

    /**
     * Emitted whenever the cell background \a color is changed in the widget.
     *
     * \see setBackgroundColor()
     */
    void backgroundColorChanged( const QColor &color );

    /**
     * Emitted whenever the numeric format shown in the widget is changed.
     */
    void numberFormatChanged();

    /**
     * Emitted whenever the text format shown in the widget is changed.
     *
     * \since QGIS 3.16
     */
    void textFormatChanged();

    /**
     * Emitted whenever the row \a height shown in the widget is changed.
     */
    void rowHeightChanged( double height );

    /**
     * Emitted whenever the column \a width shown in the widget is changed.
     */
    void columnWidthChanged( double width );

    /**
     * Emitted when the horizontal \a alignment shown in the widget is changed.
     *
     * \since QGIS 3.16
     */
    void horizontalAlignmentChanged( Qt::Alignment alignment );

    /**
     * Emitted when the vertical \a alignment shown in the widget is changed.
     *
     * \since QGIS 3.16
     */
    void verticalAlignmentChanged( Qt::Alignment alignment );

    /**
     * Emitted when the cell contents \a property shown in the widget is changed.
     *
     * \since QGIS 3.16
     */
    void cellPropertyChanged( const QgsProperty &property );

  private:

    std::unique_ptr< QgsNumericFormat > mNumericFormat;
    int mBlockSignals = 0;
    QgsExpressionContextGenerator *mContextGenerator = nullptr;

};

#endif // QGSTABLEEDITORFORMATTINGWIDGET_H
