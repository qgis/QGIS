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
    odbc::ConnectionRef createConnection();
    const QString &getDriver() const;

    static QgsHanaDriver *instance();

  protected:
    Q_DISABLE_COPY( QgsHanaDriver )

  private:
    odbc::EnvironmentRef mEnv;
    QString mDriver;
};

#endif  // QGSHANADRIVER_H
