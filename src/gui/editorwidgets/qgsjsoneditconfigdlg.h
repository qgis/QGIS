/***************************************************************************
    qgsjsoneditconfigdlg.h
     --------------------------------------
    Date                 : 3.5.2021
    Copyright            : (C) 2021 Damiano Lombardi
    Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSJSONEDITCONFIGDLG_H
#define QGSJSONEDITCONFIGDLG_H

#include "ui_qgsjsoneditconfigdlg.h"

#include "qgseditorconfigwidget.h"
#include "qgis_gui.h"

SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsJsonEditConfigDlg
 * \brief The QgsJsonEditConfigDlg is a configuration widget for JSON edit widget.
 * \since QGIS 3.20
 */
class GUI_EXPORT QgsJsonEditConfigDlg : public QgsEditorConfigWidget, private Ui::QgsJsonEditConfigDlg
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsJsonEditWidget.
     *
     * \param vl       The layer for which the configuration dialog will be created
     * \param fieldIdx The index of the field on the layer for which this dialog will be created
     * \param parent   A parent widget
     */
    explicit QgsJsonEditConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent = nullptr );

    QVariantMap config() override;

    void setConfig( const QVariantMap &config ) override;
};

#endif // QGSJSONEDITCONFIGDLG_H
