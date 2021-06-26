/***************************************************************************
    qgsrangeconfigdlg.h
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

#ifndef QGSRANGECONFIGDLG_H
#define QGSRANGECONFIGDLG_H

#include "ui_qgsrangeconfigdlgbase.h"
#include "qgseditorconfigwidget.h"
#include "qgis_gui.h"

SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsRangeConfigDlg
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsRangeConfigDlg : public QgsEditorConfigWidget, private Ui::QgsRangeConfigDlgBase
{
    Q_OBJECT

  public:
    explicit QgsRangeConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent );
    QVariantMap config() override;
    void setConfig( const QVariantMap &config ) override;

  protected slots:
    void rangeWidgetChanged( int index );
};

#endif // QGSRANGECONFIGDLG_H
