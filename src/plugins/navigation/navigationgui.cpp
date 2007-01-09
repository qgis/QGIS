/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "navigationgui.h"
#include "navigation.h"
#include "gpscore.h"

#include "qgisinterface.h"
#include "qgisgui.h"

#include <QMessageBox>


NavigationGui::NavigationGui(Navigation* plugin)
  : QDialog (plugin->qgis()->getMainWindow(), QgisGui::ModalDialogFlags), mPlugin(plugin)
{
  setupUi(this);
  
  QTcpSocket* socket = plugin->gps()->tcpSocket();
  
  // init GUI with the current state of the socket
  gpsStateChanged(socket->state());

  connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
          this, SLOT(gpsStateChanged(QAbstractSocket::SocketState)));

  connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
          this, SLOT(socketError(QAbstractSocket::SocketError)));
}  

NavigationGui::~NavigationGui()
{
}

void NavigationGui::pbnOK_clicked()
{
  //close the dialog
  done(1);
} 

void NavigationGui::pbnCancel_clicked()
{
 close(1);
}


void NavigationGui::on_pbnStart_clicked()
{
  mPlugin->gps()->start();
  
}

void NavigationGui::on_pbnStop_clicked()
{
  mPlugin->gps()->stop();
}

void NavigationGui::gpsStateChanged(QAbstractSocket::SocketState state)
{
  QString status;
  switch (state)
  {
    case QAbstractSocket::UnconnectedState:
      status = "Not Connected";
      break;
    case QAbstractSocket::HostLookupState:
      status = "Looking up host...";
      break;
    case QAbstractSocket::ConnectingState:
      status = "Connecting...";
      break;
    case QAbstractSocket::ConnectedState:
      status = "Connected!";
      break;
    case QAbstractSocket::ClosingState:
      status = "Closing connection...";
      break;
    default:
      status = "???";
  }
  lblConnectionStatus->setText(status);
  
  bool canStart = (state == QAbstractSocket::UnconnectedState);
    
  pbnStart->setEnabled(canStart);
  pbnStop->setEnabled(!canStart);

}

void NavigationGui::socketError(QAbstractSocket::SocketError error)
{
  // GUI is not updated automatically on error => do it explicitly
  gpsStateChanged(mPlugin->gps()->tcpSocket()->state());
}
