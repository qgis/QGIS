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

class GUI_EXPORT QgsTableEditorFormattingWidget : public QgsPanelWidget, private Ui::QgsTableEditorFormattingWidgetBase
{
    Q_OBJECT
  public:
    QgsTableEditorFormattingWidget( QWidget *parent = nullptr );
    ~QgsTableEditorFormattingWidget() override;

    QgsNumericFormat *numericFormat() SIP_TRANSFERBACK;

    void setForegroundColor( const QColor &color );
    void setBackgroundColor( const QColor &color );

  signals:

    void foregroundColorChanged( const QColor &color );
    void backgroundColorChanged( const QColor &color );
    void numberFormatChanged();

  private:

    std::unique_ptr< QgsNumericFormat > mNumericFormat;
    bool mBlockSignals = false;

};

#endif // QGSTABLEEDITORFORMATTINGWIDGET_H
