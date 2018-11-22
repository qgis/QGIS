/***************************************************************************
                          qgsoraclenewconnection.h  -  description
                             -------------------
    begin                : August 2012
    copyright            : (C) 2012 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSORACLENEWCONNECTION_H
#define QGSORACLENEWCONNECTION_H
#include "ui_qgsoraclenewconnectionbase.h"
#include "qgsguiutils.h"
#include "qgshelp.h"

/**
 * \class QgsOracleNewConnection
 * \brief Dialog to allow the user to configure and save connection
 * information for a Oracle database
 */
class QgsOracleNewConnection : public QDialog, private Ui::QgsOracleNewConnectionBase
{
    Q_OBJECT
  public:
    //! Constructor
    QgsOracleNewConnection( QWidget *parent = nullptr, const QString &connName = QString(), Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    QString originalConnName() const { return mOriginalConnName; }
    QString connName() const { return txtName->text(); }

  public slots:
    void accept() override;

  private slots:
    void testConnection();

  private:
    QString mOriginalConnName; //store initial name to delete entry in case of rename
    void showHelp();
};

#endif //  QGSORACLENEWCONNECTIONBASE_H
