/***************************************************************************
    testqgsfilefiledownloader.cpp
     --------------------------------------
    Date                 : 11.8.2016
    Copyright            : (C) 2016 Alessandro Pasotti
    Email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"
#include <QObject>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QUrl>
#include <QEventLoop>
#include <QTimer>

#include <qgsapplication.h>
#include <qgsfiledownloader.h>

class TestQgsFileDownloader: public QObject
{
    Q_OBJECT
  public:
    TestQgsFileDownloader() = default;

  public slots:
    //! Called when the download has completed successfully
    void downloadCompleted()
    {
      mCompleted = true;
    }
    //! Called when the download exits
    void downloadExited()
    {
      mExited = true;
    }
    //! Called when the download was canceled by the user
    void downloadCanceled()
    {
      mCanceled = true;
    }
    //! Called when an error makes the download fail
    void downloadError( QStringList errorMessages )
    {
      mError = true;
      errorMessages.sort();
      mErrorMessage = errorMessages.join( QLatin1Char( ';' ) );
    }
    //! Called when data ready to be processed
    void downloadProgress( qint64 bytesReceived, qint64 bytesTotal )
    {
      Q_UNUSED( bytesReceived );
      Q_UNUSED( bytesTotal );
      mProgress = true;
    }

  private slots:
    void initTestCase(); // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup(); // will be called after every testfunction.

    void testValidDownload();
    void testInValidDownload();
    void testInvalidFile();
    void testCanceledDownload();
    void testInvalidUrl();
    void testBlankUrl();
    void testLacksWritePermissionsError();
#ifndef QT_NO_SSL
    void testSslError_data();
    void testSslError();
#endif

  private:
    void makeCall( QUrl url, QString fileName, bool cancel = false );
    QTemporaryFile *mTempFile = nullptr;
    QString mErrorMessage;
    bool mCanceled =  false ;
    bool mProgress =  false ;
    bool mError =  false ;
    bool mCompleted =  false ;
    bool mExited =  false ;
    QgsFileDownloader *mFileDownloader = nullptr;
};

void TestQgsFileDownloader::makeCall( QUrl url, QString fileName, bool cancel )
{
  QEventLoop loop;

  mFileDownloader = new QgsFileDownloader( url, fileName );
  connect( mFileDownloader, &QgsFileDownloader::downloadCompleted, this, &TestQgsFileDownloader::downloadCompleted );
  connect( mFileDownloader, &QgsFileDownloader::downloadCanceled, this, &TestQgsFileDownloader::downloadCanceled );
  connect( mFileDownloader, &QgsFileDownloader::downloadExited, this, &TestQgsFileDownloader::downloadExited );
  connect( mFileDownloader, &QgsFileDownloader::downloadError, this, &TestQgsFileDownloader::downloadError );
  connect( mFileDownloader, &QgsFileDownloader::downloadProgress, this, &TestQgsFileDownloader::downloadProgress );

  connect( mFileDownloader, &QgsFileDownloader::downloadExited, &loop, &QEventLoop::quit );

  if ( cancel )
    QTimer::singleShot( 1000, mFileDownloader, &QgsFileDownloader::cancelDownload );

  loop.exec();

}

void TestQgsFileDownloader::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

}

void TestQgsFileDownloader::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsFileDownloader::init()
{
  mErrorMessage.clear();
  mCanceled = false;
  mProgress = false;
  mError = false;
  mCompleted = false;
  mExited = false;
  mTempFile = new QTemporaryFile();
  QVERIFY( mTempFile->open() );
  mTempFile->close();
}



void TestQgsFileDownloader::cleanup()
{
  delete mTempFile;
}

void TestQgsFileDownloader::testValidDownload()
{
  QVERIFY( ! mTempFile->fileName().isEmpty() );
  makeCall( QUrl( QStringLiteral( "http://www.qgis.org" ) ), mTempFile->fileName() );
  QVERIFY( mExited );
  QVERIFY( mCompleted );
  QVERIFY( mProgress );
  QVERIFY( !mError );
  QVERIFY( !mCanceled );
  QVERIFY( mTempFile->size() > 0 );
}

void TestQgsFileDownloader::testInValidDownload()
{
  QVERIFY( ! mTempFile->fileName().isEmpty() );
  makeCall( QUrl( QStringLiteral( "http://www.doesnotexistofthatimsure.qgis" ) ), mTempFile->fileName() );
  QVERIFY( mExited );
  QVERIFY( !mCompleted );
  QVERIFY( mError );
  QVERIFY( !mCanceled );
  QVERIFY( mTempFile->size() == 0 );
  QCOMPARE( mErrorMessage, QString( "Download failed: Host www.doesnotexistofthatimsure.qgis not found" ) );
}

void TestQgsFileDownloader::testCanceledDownload()
{
  QVERIFY( ! mTempFile->fileName().isEmpty() );
  makeCall( QUrl( QStringLiteral( "https://github.com/qgis/QGIS/archive/master.zip" ) ), mTempFile->fileName(), true );
  QVERIFY( mExited );
  QVERIFY( !mCompleted );
  QVERIFY( !mError );
  QVERIFY( mProgress );
  QVERIFY( mCanceled );
  QVERIFY( !mTempFile->exists() );
}

void TestQgsFileDownloader::testInvalidFile()
{
  makeCall( QUrl( QStringLiteral( "https://github.com/qgis/QGIS/archive/master.zip" ) ), QString() );
  QVERIFY( mExited );
  QVERIFY( !mCompleted );
  QVERIFY( mError );
  QVERIFY( !mCanceled );
  QCOMPARE( mErrorMessage, QString( "No output filename specified" ) );
}

void TestQgsFileDownloader::testInvalidUrl()
{
  QVERIFY( ! mTempFile->fileName().isEmpty() );
  makeCall( QUrl( QStringLiteral( "xyz://www" ) ), mTempFile->fileName() );
  QVERIFY( mExited );
  QVERIFY( !mCompleted );
  QVERIFY( mError );
  QVERIFY( !mCanceled );
  QCOMPARE( mErrorMessage, QString( "Download failed: Protocol \"xyz\" is unknown" ) );
}

void TestQgsFileDownloader::testBlankUrl()
{
  QVERIFY( ! mTempFile->fileName().isEmpty() );
  makeCall( QUrl( QString() ), mTempFile->fileName() );
  QVERIFY( mExited );
  QVERIFY( !mCompleted );
  QVERIFY( mError );
  QVERIFY( !mCanceled );
  QCOMPARE( mErrorMessage, QString( "Download failed: Protocol \"\" is unknown" ) );
}

#ifndef QT_NO_SSL
void TestQgsFileDownloader::testSslError_data()
{
  QTest::addColumn<QString>( "url" );
  QTest::addColumn<QString>( "result" );

  QTest::newRow( "expired" ) << "https://expired.badssl.com/"
                             << "SSL Errors: ;The certificate has expired";
  QTest::newRow( "self-signed" ) << "https://self-signed.badssl.com/"
                                 << "SSL Errors: ;The certificate is self-signed, and untrusted";
  QTest::newRow( "untrusted-root" ) << "https://untrusted-root.badssl.com/"
                                    << "No certificates could be verified;SSL Errors: ;The issuer certificate of a locally looked up certificate could not be found";
}

void TestQgsFileDownloader::testSslError()
{
  QFETCH( QString, url );
  QFETCH( QString, result );
  QVERIFY( ! mTempFile->fileName().isEmpty() );
  makeCall( QUrl( url ), mTempFile->fileName() );
  QCOMPARE( mErrorMessage, result );
  QVERIFY( !mCompleted );
  QVERIFY( mError );
  QVERIFY( !mCanceled );
}

void TestQgsFileDownloader::testLacksWritePermissionsError()
{
  const QTemporaryDir dir;
  QFile tmpDir( dir.path( ) );
  tmpDir.setPermissions( tmpDir.permissions() & ~( QFile::Permission::WriteGroup |  QFile::Permission::WriteUser | QFile::Permission::WriteOther | QFile::Permission::WriteOwner ) );
  QVERIFY( ! tmpDir.isWritable() );
  const QString fileName( dir.path() + '/' + QStringLiteral( "tmp.bin" ) );
  makeCall( QUrl( QStringLiteral( "http://www.qgis.org" ) ), fileName );
  QVERIFY( mExited );
  QVERIFY( !mCompleted );
  QVERIFY( mError );
  QVERIFY( !mCanceled );
  QVERIFY( ! QFileInfo::exists( fileName ) );
}


#endif


QGSTEST_MAIN( TestQgsFileDownloader )
#include "testqgsfiledownloader.moc"


