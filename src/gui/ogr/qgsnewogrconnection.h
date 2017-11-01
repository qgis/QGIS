/***************************************************************************
                          qgsnewogrconnection.h  -  description
                             -------------------
    begin                : Mon Jan 2 2009
    copyright            : (C) 2009 by Godofredo Contreras Nava
    email                : frdcn at hotmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNEWOGRCONNECTION_H
#define QGSNEWOGRCONNECTION_H

#define SIP_NO_FILE

#include "ui_qgsnewogrconnectionbase.h"
#include "qgsguiutils.h"
#include "qgis_gui.h"


/**
 * \class QgsNewOgrConnection
 * \brief Dialog to allow the user to define, test and save connection
 * information for OGR databases
 * \note not available in python bindings
 */
class GUI_EXPORT QgsNewOgrConnection : public QDialog, private Ui::QgsNewOgrConnectionBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsNewOgrConnection( QWidget *parent = nullptr, const QString &connType = QString(), const QString &connName = QString(), Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    ~QgsNewOgrConnection();
    //! Tests the connection using the parameters supplied
    void testConnection();
    void showHelp();
  public slots:
    void accept() override;
    void btnConnect_clicked();

  private:
    QString mOriginalConnName;
};

#endif //  QGSNEWOGRCONNECTIONBASE_H
