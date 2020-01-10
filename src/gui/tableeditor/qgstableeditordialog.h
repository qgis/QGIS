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

class GUI_EXPORT QgsTableEditorDialog : public QMainWindow, private Ui::QgsTableEditorBase
{
    Q_OBJECT
  public:
    QgsTableEditorDialog( QWidget *parent = nullptr );
    void setTableData( const QgsTableContents &contents );
    QgsTableContents tableData() const;

  signals:

    void tableChanged();

  private:
    QgsTableEditorWidget *mTableWidget = nullptr;

    QgsMessageBar *mMessageBar = nullptr;
    QgsDockWidget *mPropertiesDock = nullptr;
    QgsPanelWidgetStack *mPropertiesStack = nullptr;
    QgsTableEditorFormattingWidget *mFormattingWidget = nullptr;

    bool mBlockSignals = false;
};

#endif // QGSTABLEEDITORSHEETWIDGET_H
