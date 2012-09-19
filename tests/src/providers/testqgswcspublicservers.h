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
    // Known problem
    struct Issue
    {
      QStringList versions; // version regex
      QStringList coverages; // coverage regex
      QString description; // problem description
      Issue( const QString & d ) : description( d ) {}
    };
    struct Server
    {
      Server( ) {}
      Server( const QString & u ) : url( u ) {}
      QString url; // URL
      QList<TestQgsWcsPublicServers::Issue> issues;
    };


    TestQgsWcsPublicServers( const QString & cacheDirPath, int maxCoverages, const QString & server = QString(), const QString & coverage = QString(), const QString &version = QString(), bool force = false );

    void init();
    void test();
    void report();
  private:
    QString row( QStringList theValues, QString theClass = QString() );
    QString error( QString theMessage );
    void writeReport( QString theReport );

    QMap<QString, QString> readLog( QString theFileName );

    QStringList issueDescriptions( const QString & url, const QString & coverage, const QString &version );

    QString mCacheDirPath;
    QDir mCacheDir;

    // Max coverages to test per server/version
    int mMaxCoverages;

    QString mServer;
    QString mCoverage;
    QString mVersion;

    // Force cached
    bool mForce;

    QString mReport;
    QStringList mHead;

    QList<TestQgsWcsPublicServers::Server> mServers;
};
