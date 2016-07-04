/***************************************************************************
    qgsexternalresourceconfigdlg.h
     --------------------------------------
    Date                 : 2015-11-26
    Copyright            : (C) 2015 Médéric Ribreux
    Email                : mederic dot ribreux at medspx dot fr
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXTERNALRESOURCECONFIGDLG_H
#define QGSEXTERNALRESOURCECONFIGDLG_H

#include "ui_qgsexternalresourceconfigdlg.h"

#include "qgseditorconfigwidget.h"

/** \ingroup gui
 * \class QgsExternalResourceConfigDlg
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsExternalResourceConfigDlg : public QgsEditorConfigWidget, private Ui::QgsExternalResourceConfigDlg
{
    Q_OBJECT

  public:
    explicit QgsExternalResourceConfigDlg( QgsVectorLayer* vl, int fieldIdx, QWidget *parent = 0 );

    // QgsEditorConfigWidget interface
  public:
    QgsEditorWidgetConfig config() override;
    void setConfig( const QgsEditorWidgetConfig& config ) override;

  private slots:
    //! Choose a base directory for rootPath
    void chooseDefaultPath();

    //! Modify RelativeDefault according to mRootPath content
    void enableRelativeDefault();

    //! Dynamic activation of RelativeGroupBox
    void enableRelative( bool state );
};

#endif // QGSEXTERNALRESOURCECONFIGDLG_H
