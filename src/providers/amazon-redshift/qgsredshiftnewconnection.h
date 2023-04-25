/***************************************************************************
   qgsredshiftnewconnection.h
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSREDSHIFTNEWCONNECTION_H
#define QGSREDSHIFTNEWCONNECTION_H
#include "qgsguiutils.h"
#include "qgshelp.h"
#include "ui_qgsredshiftnewconnectionbase.h"

/**
 * \class QgsRedshiftNewConnection
 * \brief Dialog to allow the user to configure and save connection
 * information for a Redshift cluster
 */
class QgsRedshiftNewConnection : public QDialog, private Ui::QgsRedshiftNewConnectionBase
{
    Q_OBJECT
  public:
    //! Constructor
    QgsRedshiftNewConnection( QWidget *parent = nullptr, const QString &connName = QString(),
                              Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    //! Tests the connection using the parameters supplied
    void testConnection();
  public slots:
    void accept() override;
    void btnConnect_clicked();
  private slots:
    //! Updates state of the OK button depending of the filled fields
    void updateOkButtonState();

  private:
    //! Stores the initial name of the entry to delete in case of rename. QString mOriginalConnName;
    QString mOriginalConnName;
    void showHelp();
};

#endif //  QGSREDSHIFTNEWCONNECTION_H
