/***************************************************************************
    oracleconnectgui.cpp
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
/* $Id$ */

#include "qgsoracleconnect_ui.h"

// Qt Includes
#include <QSettings>
#include <QMessageBox>

QgsOracleConnect::QgsOracleConnect( QWidget* parent,
                                    const QString& connName,
                                    Qt::WFlags fl ) : QDialog( parent, fl )
{
  setupUi( this );

  if ( ! connName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters

    QSettings settings;

    QString key = "/Oracle/connections/" + connName;

    txtDatabase->setText( settings.value( key + "/database" ).toString() );
    txtUsername->setText( settings.value( key + "/username" ).toString() );

    if ( settings.value( key + "/savepass" ).toString() == "true" )
    {
      txtPassword->setText( settings.value( key + "/password" ).toString() );
      chkStorePassword->setChecked( true );
    }
    txtName->setText( connName );
  }
}

QgsOracleConnect::~QgsOracleConnect()
{
}

void QgsOracleConnect::on_buttonBox_rejected()
{
  // cancel button mapped to context help - changed this to close the
  // dialog instead. If context help is to be added, a Help button
  // is required. - gsherman
  // helpInfo();
  reject();
}

void QgsOracleConnect::on_buttonBox_accepted()
{
  saveConnection();
}

void QgsOracleConnect::saveConnection()
{
  QSettings settings;

  QString baseKey = "/Oracle/connections/";

  settings.setValue( baseKey + "selected", txtName->text() );
  baseKey += txtName->text();
  settings.setValue( baseKey + "/database", txtDatabase->text() );
  settings.setValue( baseKey + "/username", txtUsername->text() );
  settings.setValue( baseKey + "/password", txtPassword->text() );
  settings.setValue( baseKey + "/savepass", chkStorePassword->isChecked() ? "true" : "false" );
  settings.setValue( baseKey + "/subdtset", "GEOR:" +
                     txtUsername->text() + "/" +
                     txtPassword->text() + "@" +
                     txtDatabase->text() );

  accept();
}

void QgsOracleConnect::helpInfo()
{
  //  QgsContextHelp::run( context_id );
}

