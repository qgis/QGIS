/***************************************************************************
                              qgsgeonodenewconnection.cpp
                              -------------------
    begin                : Feb 2017
    copyright            : (C) 2017 by Muhammad Yarjuna Rohmat, Ismail Sunni
    email                : rohmat at kartoza dot com, ismail at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMessageBox>
#include "qgsgeonodenewconnection.h"
#include "qgsgeonodeconnection.h"
#include "qgsgeonoderequest.h"

QgsGeoNodeNewConnection::QgsGeoNodeNewConnection( QWidget *parent, const QString &connName, Qt::WindowFlags fl )
  : QgsNewHttpConnection( parent, 0, QgsGeoNodeConnectionUtils::pathGeoNodeConnection(), connName, QgsNewHttpConnection::FlagShowTestConnection, fl )
{
  setWindowTitle( tr( "Create a New GeoNode Connection" ) );

  connect( testConnectButton(), &QPushButton::clicked, this, &QgsGeoNodeNewConnection::testConnection );
}

void QgsGeoNodeNewConnection::testConnection()
{
  QApplication::setOverrideCursor( Qt::BusyCursor );
  QgsGeoNodeRequest geonodeRequest( url(), true );

  QList<QgsGeoNodeRequest::ServiceLayerDetail> layers = geonodeRequest.fetchLayersBlocking();
  QApplication::restoreOverrideCursor();

  if ( !layers.empty() )
  {
    QMessageBox::information( this,
                              tr( "Test connection" ),
                              tr( "\nConnection to %1 was successful, \n\n%1 is a valid geonode instance.\n\n" ).arg( url() ) );
  }
  else
  {
    QMessageBox::information( this,
                              tr( "Test connection" ),
                              tr( "\nConnection failed, \n\nplease check whether %1 is a valid geonode instance.\n\n" ).arg( url() ) );
  }
}

bool QgsGeoNodeNewConnection::validate()
{
  if ( !url().contains( "://" ) )
  {
    QMessageBox::warning(
      this,
      tr( "Invalid URL" ),
      tr( "Your URL doesn't contain a protocol (e.g. http or https). Please add the protocol." ) );
    return false;
  }
  return QgsNewHttpConnection::validate();
}
