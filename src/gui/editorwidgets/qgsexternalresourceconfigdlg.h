/***************************************************************************
    qgsexternalresourceconfigdlg.h
     --------------------------------------
    Date                 : 2015-11-26
    Copyright            : (C) 2015 Médéric Ribreux
    Email                : mederic dot ribreux at medspx dot fr
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXTERNALRESOURCECONFIGDLG_H
#define QGSEXTERNALRESOURCECONFIGDLG_H

#include "ui_qgsexternalresourceconfigdlg.h"

#include "qgseditorconfigwidget.h"
#include "qgis_gui.h"

SIP_NO_FILE

/**
 * \ingroup gui
 * \class QgsExternalResourceConfigDlg
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsExternalResourceConfigDlg : public QgsEditorConfigWidget, private Ui::QgsExternalResourceConfigDlg
{
    Q_OBJECT

  public:

    //! Constructor for QgsExternalResourceConfigDlg
    explicit QgsExternalResourceConfigDlg( QgsVectorLayer *vl, int fieldIdx, QWidget *parent = nullptr );

    // QgsEditorConfigWidget interface
  public:
    QVariantMap config() override;
    void setConfig( const QVariantMap &config ) override;

  private slots:
    //! Choose a base directory for rootPath
    void chooseDefaultPath();

    //! Modify RelativeDefault according to mRootPath content
    void enableRelativeDefault();
};

#endif // QGSEXTERNALRESOURCECONFIGDLG_H
