/***************************************************************************
    qgsrangeconfigdlg.h
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

#ifndef QGSRANGECONFIGDLG_H
#define QGSRANGECONFIGDLG_H

#include "ui_qgsrangeconfigdlgbase.h"
#include "qgseditorconfigwidget.h"

class GUI_EXPORT QgsRangeConfigDlg : public QgsEditorConfigWidget, private Ui::QgsRangeConfigDlgBase
{
    Q_OBJECT

  public:
    explicit QgsRangeConfigDlg( QgsVectorLayer* vl, int fieldIdx, QWidget* parent );
    virtual QgsEditorWidgetConfig config();
    virtual void setConfig( const QgsEditorWidgetConfig& config );

  protected slots:
    void rangeWidgetChanged( int index );
};

#endif // QGSRANGECONFIGDLG_H
