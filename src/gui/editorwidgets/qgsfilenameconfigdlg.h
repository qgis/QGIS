/***************************************************************************
    qgsfilenameconfigdlg.h
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

#ifndef QGSFILENAMECONFIGDLG_H
#define QGSFILENAMECONFIGDLG_H

#include "ui_qgsfilenameconfigdlg.h"

#include "qgseditorconfigwidget.h"
//#include "qgsfilenamewidgetwrapper.h"

/** \class QgsFileNameConfigDlg
 * \note not available in Python bindings
 */

class GUI_EXPORT QgsFileNameConfigDlg : public QgsEditorConfigWidget, private Ui::QgsFileNameConfigDlg
{
    Q_OBJECT

  public:
    explicit QgsFileNameConfigDlg( QgsVectorLayer* vl, int fieldIdx, QWidget *parent = 0 );

    // QgsEditorConfigWidget interface
  public:
    QgsEditorWidgetConfig config() override;
    void setConfig( const QgsEditorWidgetConfig& config ) override;

  private slots:
    void chooseDefaultPath();
    void enableRelativeDefault();
    void enableRelative( bool state );
};

#endif // QGSFILENAMECONFIGDLG_H
