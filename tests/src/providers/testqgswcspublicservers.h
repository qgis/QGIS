/***************************************************************************
     testqgswcspublicservers.h
     --------------------------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Radim Blazek
    Email                : radim dot blazek at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QApplication>
#include <QObject>
#include <QString>
#include <QStringList>

#include <qgsapplication.h>
#include <qgsdatasourceuri.h>
#include <qgsproviderregistry.h>
#include <qgsrasterdataprovider.h>
#include <qgsrasterlayer.h>

/** 
 * This class tries to get samples of coverages from public WCS servers,
 * cache results and write report. 
 */
class TestQgsWcsPublicServers: public QObject
{
    Q_OBJECT;
  public:
    void init();
    void test();
    void report();
  private:
    QString row( QStringList theValues, QString theClass = QString() );
    QString error( QString theMessage );
    void writeReport( QString theReport );

    QMap<QString,QString> readLog ( QString theFileName );

    QDir mCacheDir;
    QString mReport;
    QStringList mHead;
};
