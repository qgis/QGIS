/***************************************************************************
   qgshanadriver.h
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSHANADRIVER_H
#define QGSHANADRIVER_H

#include <qglobal.h>
#include <QString>
#include "odbc/Forwards.h"

class QgsHanaDriver
{
  private:
    QgsHanaDriver();
    ~QgsHanaDriver();

  public:
    NS_ODBC::ConnectionRef createConnection();
    QStringList dataSources();
    const QString &driver() const;

    static QgsHanaDriver *instance();
    static bool isInstalled( const QString &name );
    static bool isValidPath( const QString &path );

  protected:
    Q_DISABLE_COPY( QgsHanaDriver )

  private:
    NS_ODBC::EnvironmentRef mEnv;
    QString mDriver;
};

#endif  // QGSHANADRIVER_H
