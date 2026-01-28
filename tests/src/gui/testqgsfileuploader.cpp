/***************************************************************************
    testqgsfilefiledownloader.cpp
     --------------------------------------
    Date                 : August 2025
    Copyright            : (C) 2025 Valentin Buira
    Email                : valentin dot buira at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsapplication.h"
#include "qgsfileuploader.h"
#include "qgstest.h"

#include <QObject>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTimer>
#include <QUrl>

class TestQgsFileUploader : public QObject
{
    Q_OBJECT
  public:
    TestQgsFileUploader() = default;

  public slots:
    //! Called when the download has completed successfully
    void uploadCompleted()
    {
      mCompleted = true;
    }
    //! Called when the download was canceled by the user
    void uploadCanceled()
    {
      mCanceled = true;
    }
    //! Called when an error makes the download fail
    void uploadError( QStringList errorMessages )
    {
      mError = true;
      errorMessages.sort();
      mErrorMessage = errorMessages.join( ';'_L1 );
    }
    //! Called when data ready to be processed
    void uploadProgress( qint64 bytesSent, qint64 bytesTotal )
    {
      Q_UNUSED( bytesSent );
      Q_UNUSED( bytesTotal );
      mProgress = true;
    }

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void testInvalidHttpMethod();
    void testBlankUrl();
#ifndef QT_NO_SSL
    void testSslError_data();
    void testSslError();
#endif

  private:
    void makeCall( QUrl url, QString fileName, bool cancel = false );
    QTemporaryFile *mTempFile = nullptr;
    QString mErrorMessage;
    bool mCanceled = false;
    bool mProgress = false;
    bool mError = false;
    bool mCompleted = false;
    QgsFileUploader *mFileUploader = nullptr;
};

void TestQgsFileUploader::makeCall( QUrl url, QString fileName, bool cancel )
{
  mFileUploader = new QgsFileUploader( fileName, url );
  connect( mFileUploader, &QgsFileUploader::uploadCompleted, this, &TestQgsFileUploader::uploadCompleted );
  connect( mFileUploader, &QgsFileUploader::uploadCanceled, this, &TestQgsFileUploader::uploadCanceled );
  connect( mFileUploader, &QgsFileUploader::uploadError, this, &TestQgsFileUploader::uploadError );
  connect( mFileUploader, &QgsFileUploader::uploadProgress, this, &TestQgsFileUploader::uploadProgress );

  if ( cancel )
    QTimer::singleShot( 1000, mFileUploader, &QgsFileUploader::cancelUpload );

  mFileUploader->startUpload();
}

void TestQgsFileUploader::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsFileUploader::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsFileUploader::init()
{
  mErrorMessage.clear();
  mCanceled = false;
  mProgress = false;
  mError = false;
  mCompleted = false;
  mTempFile = new QTemporaryFile();
  QVERIFY( mTempFile->open() );
  mTempFile->close();
}


void TestQgsFileUploader::cleanup()
{
  delete mTempFile;
}

void TestQgsFileUploader::testInvalidHttpMethod()
{
  QVERIFY( !mTempFile->fileName().isEmpty() );
  makeCall( QUrl( u"http://example.com"_s ), mTempFile->fileName() );
  QVERIFY( !mCompleted );
  QVERIFY( !mProgress );
  QVERIFY( mError );
  QVERIFY( !mCanceled );
  QCOMPARE( mErrorMessage.left( 62 ), u"Upload failed: Error transferring http://example.com - server "_s );
}

void TestQgsFileUploader::testBlankUrl()
{
  QVERIFY( !mTempFile->fileName().isEmpty() );
  makeCall( QUrl( QString() ), mTempFile->fileName() );
  QVERIFY( !mCompleted );
  QVERIFY( mError );
  QVERIFY( !mCanceled );
  QCOMPARE( mErrorMessage, QString( "Upload failed: Protocol \"\" is unknown" ) );
}

#ifndef QT_NO_SSL
void TestQgsFileUploader::testSslError_data()
{
  QTest::addColumn<QString>( "url" );
  QTest::addColumn<QString>( "result" );

  QTest::newRow( "expired" ) << "https://expired.badssl.com/"
                             << "Upload failed: SSL handshake failed: The certificate has expired";
  QTest::newRow( "self-signed" ) << "https://self-signed.badssl.com/"
                                 << "Upload failed: SSL handshake failed: The certificate is self-signed, and untrusted";
}

void TestQgsFileUploader::testSslError()
{
  QFETCH( QString, url );
  QFETCH( QString, result );
  QVERIFY( !mTempFile->fileName().isEmpty() );
  makeCall( QUrl( url ), mTempFile->fileName() );
  QCOMPARE( mErrorMessage, result );
  QVERIFY( !mCompleted );
  QVERIFY( mError );
  QVERIFY( !mCanceled );
}

#endif


QGSTEST_MAIN( TestQgsFileUploader )
#include "testqgsfileuploader.moc"
