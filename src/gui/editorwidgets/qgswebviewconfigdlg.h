/***************************************************************************
    qgswebviewwidgetconfigdlgbase.h
     --------------------------------------
    Date                 : 11.1.2014
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

#ifndef QGSWEBVIEWWIDGETCONFIGDLGBASE_H
#define QGSWEBVIEWWIDGETCONFIGDLGBASE_H

#include "ui_qgswebviewconfigdlgbase.h"

#include "qgseditorconfigwidget.h"

class GUI_EXPORT QgsWebViewWidgetConfigDlg : public QgsEditorConfigWidget, private Ui::QgsWebViewWidgetConfigDlgBase
{
    Q_OBJECT

  public:
    explicit QgsWebViewWidgetConfigDlg( QgsVectorLayer* vl, int fieldIdx, QWidget *parent );

    // QgsEditorConfigWidget interface
  public:
    QgsEditorWidgetConfig config() override;
    void setConfig( const QgsEditorWidgetConfig& config ) override;
};

#endif // QGSWEBVIEWWIDGETCONFIGDLGBASE_H
