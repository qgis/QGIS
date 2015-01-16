/***************************************************************************
    qgsphotoconfigdlg.h
     --------------------------------------
    Date                 : 5.1.2014
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

#ifndef QGSPHOTOCONFIGDLG_H
#define QGSPHOTOCONFIGDLG_H

#include "ui_qgsphotoconfigdlgbase.h"

#include "qgseditorconfigwidget.h"

class GUI_EXPORT QgsPhotoConfigDlg : public QgsEditorConfigWidget, private Ui::QgsPhotoConfigDlgBase
{
    Q_OBJECT

  public:
    explicit QgsPhotoConfigDlg( QgsVectorLayer* vl, int fieldIdx, QWidget *parent = 0 );

    // QgsEditorConfigWidget interface
  public:
    QgsEditorWidgetConfig config() override;
    void setConfig( const QgsEditorWidgetConfig& config ) override;
};

#endif // QGSPHOTOCONFIGDLG_H
