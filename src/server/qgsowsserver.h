/***************************************************************************
                              qgsowsserver.h
                              --------------
  begin                : March 24, 2014
  copyright            : (C) 2006 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOWSSERVER_H
#define QGSOWSSERVER_H

#include "qgsrequesthandler.h"

class QgsOWSServer
{
  public:
    QgsOWSServer( const QString& configFilePath, const QMap<QString, QString>& parameters, QgsRequestHandler* rh )
        : mParameters( parameters ), mRequestHandler( rh ), mConfigFilePath( configFilePath ) {}
    virtual ~QgsOWSServer() {}

    virtual void executeRequest() = 0;

  private:
    QgsOWSServer() {}

  protected:
    QMap<QString, QString> mParameters;
    QgsRequestHandler* mRequestHandler;
    QString mConfigFilePath;
};

#endif // QGSOWSSERVER_H
