/***************************************************************************
                          qgsdamengnewconnection.h  -  description
                             -------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDAMENGNEWCONNECTION_H
#define QGSDAMENGNEWCONNECTION_H

#include "./ui_include/ui_qgsdamengnewconnectionbase.h"
#include "qgsguiutils.h"
#include "qgshelp.h"

/**
 * \class QgsDamengNewConnection
 * \brief Dialog to allow the user to configure and save connection
 * information for a Dameng database
 */
class QgsDamengNewConnection : public QDialog, private Ui::QgsDamengNewConnectionBase
{
    Q_OBJECT
  public:
    //! Constructor
    QgsDamengNewConnection( QWidget *parent = nullptr, const QString &connName = QString(), Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    //! Tests the connection using the parameters supplied
    void testConnection();
  public slots:
    void accept() override;
    void btnConnect_clicked();
  private slots:
    //! Updates state of the OK button depending of the filled fields
    void updateOkButtonState();
  private:
    QString mOriginalConnName; //store initial name to delete entry in case of rename
    void showHelp();

};

#endif //  QGSDAMENGNEWCONNECTIONBASE_H
