/***************************************************************************
    qgscheckboxconfigdlg.h
     --------------------------------------
    Date                 : 5.1.2014
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

#ifndef QGSCHECKBOXCONFIGDLG_H
#define QGSCHECKBOXCONFIGDLG_H

#include "ui_qgscheckboxconfigdlgbase.h"

#include "qgseditorconfigwidget.h"
#include "qgis_gui.h"

SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsCheckBoxConfigDlg
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
