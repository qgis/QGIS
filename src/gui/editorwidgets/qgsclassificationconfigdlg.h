/***************************************************************************
    qgsclassificationconfigdlg.h
     --------------------------------------
    Date                 : 7.8.2025
    Copyright            : (C) 2014 Moritz Hackenberg
    Email                : hackenberg at dev-gis dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCLASSIFICATIONCONFIGDLG_H
#define QGSCLASSIFICATIONCONFIGDLG_H

#include "ui_qgsclassificationconfigdlgbase.h"

#include "qgseditorconfigwidget.h"
#include "qgis_gui.h"

SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsClassificationConfigDlg
 * \brief Configuration widget for classification widgets.
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsClassificationConfigDlg : public QgsEditorConfigWidget, private Ui::QgsClassificationWidget
{
    Q_OBJECT

  public:
    explicit QgsClassificationConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent = nullptr );

    // QgsEditorConfigWidget interface
  public:
    QVariantMap config() override;
    void setConfig( const QVariantMap &config ) override;
};

#endif // QGSCLASSIFICATIONCONFIGDLG_H
