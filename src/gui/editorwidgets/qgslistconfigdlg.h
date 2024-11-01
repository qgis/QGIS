/***************************************************************************
    qgstexteditconfigdlg.h
     --------------------------------------
    Date                 : 9.2020
    Copyright            : (C) 2020 Stephen Knox
    Email                : stephenknox73 at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLISTCONFIGDLG_H
#define QGSLISTCONFIGDLG_H

#include "ui_qgslistconfigdlg.h"

#include "qgseditorconfigwidget.h"
#include "qgis_gui.h"

SIP_NO_FILE

/**
 * A configuration dialog for the List Widget class
 * \ingroup gui
 * \class QgsListConfigDlg
 * \note not available in Python bindings
 *
 * \since QGIS 3.16
 */
class GUI_EXPORT QgsListConfigDlg : public QgsEditorConfigWidget, private Ui::QgsListConfigDlg
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsListConfigDlg, with the specified \a vector layer and field index.
     */
    explicit QgsListConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent = nullptr );

    // QgsEditorConfigWidget interface
  public:
    QVariantMap config() override;
    void setConfig( const QVariantMap &config ) override;
};

#endif // QGSLISTCONFIGDLG_H
