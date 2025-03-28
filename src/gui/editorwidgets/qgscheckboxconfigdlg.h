/***************************************************************************
    qgscheckboxconfigdlg.h
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

#ifndef QGSCHECKBOXCONFIGDLG_H
#define QGSCHECKBOXCONFIGDLG_H

#include "ui_qgscheckboxconfigdlgbase.h"

#include "qgseditorconfigwidget.h"
#include "qgis_gui.h"

SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsCheckBoxConfigDlg
 * \brief Configuration widget for checkbox widgets.
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsCheckBoxConfigDlg : public QgsEditorConfigWidget, private Ui::QgsCheckBoxConfigDlgBase
{
    Q_OBJECT

  public:
    explicit QgsCheckBoxConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent = nullptr );

    // QgsEditorConfigWidget interface
  public:
    QVariantMap config() override;
    void setConfig( const QVariantMap &config ) override;
};

#endif // QGSCHECKBOXCONFIGDLG_H
