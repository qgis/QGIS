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
#include <memory>

class QgsNumericFormat;

/**
 * \ingroup gui
 * \class QgsTableEditorFormattingWidget
 *
 * A reusable widget for formatting the contents of a QgsTableCell.
 *
 * The editor has support for table foreground and background colors, and numeric formats.
 *
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsTableEditorFormattingWidget : public QgsPanelWidget, private Ui::QgsTableEditorFormattingWidgetBase
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
     */
    QgsNumericFormat *numericFormat() SIP_TRANSFERBACK;

    /**
     * Sets the cell foreground \a color to show in the widget.
     *
     * \see foregroundColorChanged()
     * \see setBackgroundColor()
     */
    void setForegroundColor( const QColor &color );

    /**
     * Sets the cell background \a color to show in the widget.
     *
     * \see backgroundColorChanged()
     * \see setForegroundColor()
     */
    void setBackgroundColor( const QColor &color );

  signals:

    /**
     * Emitted whenever the cell foreground \a color is changed in the widget.
     *
     * \see setForegroundColor()
     */
    void foregroundColorChanged( const QColor &color );

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

  private:

    std::unique_ptr< QgsNumericFormat > mNumericFormat;
    bool mBlockSignals = false;

};

#endif // QGSTABLEEDITORFORMATTINGWIDGET_H
