/***************************************************************************
    qgsnewhttpconnection.cpp -  selector for a new HTTP server for WMS, etc.
                             -------------------
    begin                : 3 April 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#include "qgsnewhttpconnection.h"
#include "qgscontexthelp.h"
#include <QSettings>
#include <QMessageBox>
#include <QUrl>

QgsNewHttpConnection::QgsNewHttpConnection(
  QWidget *parent, const QString& baseKey, const QString& connName, Qt::WFlags fl ):
    QDialog( parent, fl ),
    mBaseKey( baseKey ),
    mOriginalConnName( connName )
{
  setupUi( this );

  if ( !connName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters

    QSettings settings;

    QString key = mBaseKey + connName;
    QString credentialsKey = "/Qgis/WMS/" + connName;
    txtName->setText( connName );
    txtUrl->setText( settings.value( key + "/url" ).toString() );
    txtUserName->setText( settings.value( credentialsKey + "/username" ).toString() );
    txtPassword->setText( settings.value( credentialsKey + "/password" ).toString() );

  }
}

QgsNewHttpConnection::~QgsNewHttpConnection()
{
}

void QgsNewHttpConnection::accept()
{
  QSettings settings;
  QString key = mBaseKey + txtName->text();
  QString credentialsKey = "/Qgis/WMS/" + txtName->text();

  // warn if entry was renamed to an existing connection
  if (( mOriginalConnName.isNull() || mOriginalConnName != txtName->text() ) &&
      settings.contains( key + "/url" ) &&
      QMessageBox::question( this,
                             tr( "Save connection" ),
                             tr( "Should the existing connection %1 be overwritten?" ).arg( txtName->text() ),
                             QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  // on rename delete original entry first
  if ( !mOriginalConnName.isNull() && mOriginalConnName != key )
  {
    settings.remove( mBaseKey + mOriginalConnName );
    settings.remove( "/Qgis/WMS/" + mOriginalConnName );
  }

  QUrl url( txtUrl->text().trimmed() );

  QList< QPair<QByteArray, QByteArray> > params = url.encodedQueryItems();
  for ( int i = 0; i < params.size(); i++ )
  {
    if ( params[i].first.toUpper() == "SERVICE" ||
         params[i].first.toUpper() == "REQUEST" ||
         params[i].first.toUpper() == "FORMAT" )
    {
      params.removeAt( i-- );
    }
  }
  url.setEncodedQueryItems( params );

  settings.setValue( key + "/url", url.toString() );
  settings.setValue( credentialsKey + "/username", txtUserName->text() );
  settings.setValue( credentialsKey + "/password", txtPassword->text() );

  QDialog::accept();
}
