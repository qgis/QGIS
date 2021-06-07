/***************************************************************************
    qgsuniquevaluesconfigdlg.h
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

#ifndef QGSUNIQUEVALUESCONFIGDLG_H
#define QGSUNIQUEVALUESCONFIGDLG_H

#include "ui_qgsuniquevaluesconfigdlgbase.h"

#include "qgseditorconfigwidget.h"
#include "qgis_gui.h"

SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsUniqueValuesConfigDlg
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsUniqueValuesConfigDlg : public QgsEditorConfigWidget, private Ui::QgsUniqueValuesConfigDlgBase
{
    Q_OBJECT

  public:
    explicit QgsUniqueValuesConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent = nullptr );

    // QgsEditorConfigWidget interface
  public:
    QVariantMap config() override;
    void setConfig( const QVariantMap &config ) override;
};

#endif // QGSUNIQUEVALUESCONFIGDLG_H
