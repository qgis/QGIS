/***************************************************************************
    qgstexteditconfigdlg.h
     --------------------------------------
    Date                 : 8.5.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
