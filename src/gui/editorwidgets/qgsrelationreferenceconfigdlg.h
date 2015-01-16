/***************************************************************************
    qgsrelationreferenceconfigdlgbase.h
     --------------------------------------
    Date                 : 21.4.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRELATIONREFERENCECONFIGDLGBASE_H
#define QGSRELATIONREFERENCECONFIGDLGBASE_H

#include "ui_qgsrelationreferenceconfigdlgbase.h"
#include "qgseditorconfigwidget.h"

class GUI_EXPORT QgsRelationReferenceConfigDlg : public QgsEditorConfigWidget, private Ui::QgsRelReferenceConfigDlgBase
{
    Q_OBJECT

  public:
    explicit QgsRelationReferenceConfigDlg( QgsVectorLayer* vl, int fieldIdx, QWidget* parent );
    virtual QgsEditorWidgetConfig config() override;
    virtual void setConfig( const QgsEditorWidgetConfig& config ) override;

  private slots:
    void relationChanged( int idx );
};

#endif // QGSRELATIONREFERENCECONFIGDLGBASE_H
