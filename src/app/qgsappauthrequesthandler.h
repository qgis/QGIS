/***************************************************************************
    qgsappauthrequesthandler.h
    -------------------------
    begin                : January 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAPPAUTHREQUESTHANDLER_H
#define QGSAPPAUTHREQUESTHANDLER_H

#include "qgsnetworkaccessmanager.h"

class QgsAppAuthRequestHandler : public QgsNetworkAuthenticationHandler
{
  public:
    void handleAuthRequest( QNetworkReply *reply, QAuthenticator *auth ) override;
    void handleAuthRequestOpenBrowser( const QUrl &url ) override;
    void handleAuthRequestCloseBrowser() override;
};


#endif // QGSAPPAUTHREQUESTHANDLER_H
