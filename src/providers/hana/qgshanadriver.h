/***************************************************************************
   qgshanadriver.h
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maksim Rylov
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
    QgsHanaDriver();
    ~QgsHanaDriver();

  public:
    odbc::ConnectionRef createConnection();
    QString getDriver() const;

    static QgsHanaDriver *instance();
    static void cleanupInstance();

  protected:
    Q_DISABLE_COPY( QgsHanaDriver )

  private:
    odbc::EnvironmentRef mEnv;
    QString mDriver;

    static QgsHanaDriver *sInstance;
};

#endif  // QGSHANADRIVER_H
