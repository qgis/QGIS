/***************************************************************************
    qgstexteditconfigdlg.h
     --------------------------------------
    Date                 : 8.5.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTEXTEDITCONFIGDLG_H
#define QGSTEXTEDITCONFIGDLG_H

#include "ui_qgstexteditconfigdlg.h"

#include "qgseditorconfigwidget.h"

class GUI_EXPORT QgsTextEditConfigDlg : public QgsEditorConfigWidget, private Ui::QgsTextEditConfigDlg
{
    Q_OBJECT

  public:
    explicit QgsTextEditConfigDlg( QgsVectorLayer* vl, int fieldIdx, QWidget *parent = 0 );

    // QgsEditorConfigWidget interface
  public:
    QgsEditorWidgetConfig config();
    void setConfig( const QgsEditorWidgetConfig& config );
};

#endif // QGSTEXTEDITCONFIGDLG_H
