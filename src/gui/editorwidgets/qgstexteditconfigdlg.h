/***************************************************************************
    qgstexteditconfigdlg.h
     --------------------------------------
    Date                 : 8.5.2014
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

#ifndef QGSTEXTEDITCONFIGDLG_H
#define QGSTEXTEDITCONFIGDLG_H

#include "ui_qgstexteditconfigdlg.h"

#include "qgseditorconfigwidget.h"
#include "qgis_gui.h"

SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsTextEditConfigDlg
 * \brief Configuration widget for text edit widgets.
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsTextEditConfigDlg : public QgsEditorConfigWidget, private Ui::QgsTextEditConfigDlg
{
    Q_OBJECT

  public:
    explicit QgsTextEditConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent = nullptr );

    // QgsEditorConfigWidget interface
  public:
    QVariantMap config() override;
    void setConfig( const QVariantMap &config ) override;
};

#endif // QGSTEXTEDITCONFIGDLG_H
