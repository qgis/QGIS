/***************************************************************************
    oracleconnectgui.u
    -------------------
    begin                : Oracle Spatial Plugin
    copyright            : (C) Ivan Lucena
    email                : ivan.lucena@pmldnet.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QgsOracleConnect_H
#define QgsOracleConnect_H

// Qt Designer Includes
#include "ui_qgsoracleconnectbase.h"

// QGIS Includes
#include <qgisgui.h>

class QgsOracleConnect : public QDialog, private Ui::OracleConnectGuiBase
{
    Q_OBJECT

  public:
    QgsOracleConnect( QWidget* parent = nullptr,
                      const QString& connName = QString::null,
                      Qt::WindowFlags fl = QgisGui::ModalDialogFlags );
    ~QgsOracleConnect();

  private:
    void saveConnection();
    void helpInfo();

  public slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
};

#endif /* _ORACLECONNECTGUI_H */
