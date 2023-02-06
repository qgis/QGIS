/***************************************************************************
   qgshananewconnection.h
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSHANANEWCONNECTION_H
#define QGSHANANEWCONNECTION_H

#include "qgsguiutils.h"
#include "qgshanasettings.h"
#include "qgshelp.h"
#include "ui_qgshananewconnectionbase.h"

/**
 * \class QgsHanaNewConnection
 * \brief Dialog to allow the user to configure and save connection
 * information for an SAP HANA database
 */
class QgsHanaNewConnection : public QDialog, private Ui::QgsHanaNewConnectionBase
{
    Q_OBJECT
  public:
    //! Constructor
    QgsHanaNewConnection(
      QWidget *parent = nullptr,
      const QString &connName = QString(),
      Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    void resizeEvent( QResizeEvent *ev ) override;

    //! Tests the connection using the parameters supplied
    void testConnection();

  public slots:
    void accept() override;

  private:
    QgsHanaConnectionType getCurrentConnectionType() const;
    QString getDatabaseName() const;
    void resizeWidgets();

    void btnConnect_clicked();
    void cmbConnectionType_changed( int index );
    void cmbIdentifierType_changed( int index );
    void rbtnSingleContainer_clicked();
    void rbtnMultipleContainers_clicked();
    void rbtnTenantDatabase_clicked();
    void rbtnSystemDatabase_clicked();
    void chkEnableSSL_clicked();
    void chkEnableProxy_clicked();
    void chkValidateCertificate_clicked();
    void readSettingsFromControls( QgsHanaSettings &settings );
    void updateControlsFromSettings( const QgsHanaSettings &settings );
    void showHelp();

  private:
    QString mOriginalConnName;
};

#endif //  QGSHANANEWCONNECTION_H
