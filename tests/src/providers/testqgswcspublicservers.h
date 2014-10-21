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
      QString offender; // server or empty == qgis
      QStringList versions; // version regex
      QStringList coverages; // coverage regex
      QString description; // problem description
      Issue( const QString & d ) : description( d ) {}
    };
    struct Server
    {
      Server() {}
      Server( const QString & u ) : url( u ) {}
      QString url; // URL
      QString description; // notes
      QList<TestQgsWcsPublicServers::Issue> issues;
      // additional params to be set on URI, e.g. IgnoreGetMapUrl
      QMap<QString, QString> params;
    };

    enum OffenderType
    {
      NoOffender      = 0,
      ServerOffender  = 1,
      QGisOffender    = 1 << 1
    };

    TestQgsWcsPublicServers( const QString & cacheDirPath, int maxCoverages, const QString & server = QString(), const QString & coverage = QString(), const QString &version = QString(), bool force = false );

    ~TestQgsWcsPublicServers();

    void init();
    void test();
    void report();
  private:
    QString cells( QStringList theValues, QString theClass = QString(), int colspan = 1, int rowspan = 1 );
    QString row( QStringList theValues, QString theClass = QString() );
    QString error( QString theMessage );
    void writeReport( QString theReport );

    QMap<QString, QString> readLog( QString theFileName );

    Server getServer( const QString & url );

    QList<Issue> issues( const QString & url, const QString & coverage, const QString &version );
    QStringList issueDescriptions( const QString & url, const QString & coverage, const QString &version );

    int issueOffender( const QString & url, const QString & coverage, const QString &version );

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

    int mTimeout;
    int mOrigTimeout;
};
