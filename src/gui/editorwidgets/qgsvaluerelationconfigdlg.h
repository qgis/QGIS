/***************************************************************************
    qgsvaluerelationconfigdlg.h
     --------------------------------------
    Date                 : 5.1.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVALUERELATIONCONFIGDLG_H
#define QGSVALUERELATIONCONFIGDLG_H

#include "ui_qgsvaluerelationconfigdlgbase.h"

#include "qgseditorconfigwidget.h"

class GUI_EXPORT QgsValueRelationConfigDlg : public QgsEditorConfigWidget, private Ui::QgsValueRelationConfigDlgBase
{
    Q_OBJECT

  public:
    explicit QgsValueRelationConfigDlg( QgsVectorLayer* vl, int fieldIdx, QWidget *parent = 0 );

  public slots:
    void editExpression();

    // QgsEditorConfigWidget interface
  public:
    QgsEditorWidgetConfig config() override;
    void setConfig( const QgsEditorWidgetConfig& config ) override;
};

#endif // QGSVALUERELATIONCONFIGDLG_H
