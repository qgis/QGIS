/***************************************************************************
    qgsappsslerrorhandler.h
    -------------------------
    begin                : December 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAPPSSLERRORHANDLER_H
#define QGSAPPSSLERRORHANDLER_H

#include "qgsnetworkaccessmanager.h"

class QgsAppSslErrorHandler : public QgsSslErrorHandler
{

  public:

    void handleSslErrors( QNetworkReply *reply, const QList<QSslError> &errors ) override;

};


#endif // QGSAPPSSLERRORHANDLER_H
