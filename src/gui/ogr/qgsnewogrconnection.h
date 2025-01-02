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
 * \ingroup gui
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

    //! Tests the connection using the parameters supplied
    void testConnection();

    /**
     * Show the help
     * \deprecated QGIS 3.40. Will be made private with QGIS 4.
     */
    Q_DECL_DEPRECATED void showHelp() SIP_DEPRECATED;

  public slots:
    void accept() override;

  private slots:
    void btnConnect_clicked();
    //! Updates state of the OK button depending of the filled fields
    void updateOkButtonState();

  private:
    QString mOriginalConnName;
};

#endif //  QGSNEWOGRCONNECTIONBASE_H
