/***************************************************************************
    qgsdummyconfigdlg.h
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

#ifndef QGSDUMMYCONFIGDLG_H
#define QGSDUMMYCONFIGDLG_H

#include "ui_qgsdummyconfigdlgbase.h"
#include "qgseditorconfigwidget.h"

class GUI_EXPORT QgsDummyConfigDlg : public QgsEditorConfigWidget, private Ui::QgsDummyConfigDlgBase
{
    Q_OBJECT

  public:
    explicit QgsDummyConfigDlg( QgsVectorLayer* vl, int fieldIdx, QWidget* parent, const QString& description );

    // QgsEditorConfigWidget interface
  public:
    virtual QgsEditorWidgetConfig config() override;
    virtual void setConfig( const QgsEditorWidgetConfig& config ) override;
};

#endif // QGSDUMMYCONFIGDLG_H
