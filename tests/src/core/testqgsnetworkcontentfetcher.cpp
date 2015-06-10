/***************************************************************************
                         testqgsnetworkcontentfetcher.cpp
                         -----------------------
    begin                : July 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnetworkcontentfetcher.h"
#include "qgsapplication.h"
#include <QObject>
#include <QtTest/QtTest>
#include <QNetworkReply>

class TestQgsNetworkContentFetcher : public QObject
{
    Q_OBJECT
  public:
    TestQgsNetworkContentFetcher()
        : mLoaded( false )
    {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void fetchEmptyUrl(); //test fetching blank url
    void fetchBadUrl(); //test fetching bad url
    void fetchEncodedContent(); //test fetching url content encoded as utf-8

    void contentLoaded();

  private:
    bool mLoaded;
};

void TestQgsNetworkContentFetcher::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsNetworkContentFetcher::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsNetworkContentFetcher::init()
{
}

void TestQgsNetworkContentFetcher::cleanup()
{
}

void TestQgsNetworkContentFetcher::fetchEmptyUrl()
{
  QgsNetworkContentFetcher fetcher;
  //test fetching from a blank url
  mLoaded = false;
  fetcher.fetchContent( QUrl() );
  connect( &fetcher, SIGNAL( finished() ), this, SLOT( contentLoaded() ) );
  while ( !mLoaded )
  {
    qApp->processEvents();
  }

  QVERIFY( fetcher.reply()->error() != QNetworkReply::NoError );
}

void TestQgsNetworkContentFetcher::fetchBadUrl()
{
  QgsNetworkContentFetcher fetcher;
  //test fetching from a bad url
  mLoaded = false;
  fetcher.fetchContent( QUrl( "http://x" ) );
  connect( &fetcher, SIGNAL( finished() ), this, SLOT( contentLoaded() ) );
  while ( !mLoaded )
  {
    qApp->processEvents();
  }
  QVERIFY( fetcher.reply()->error() != QNetworkReply::NoError );
}


void TestQgsNetworkContentFetcher::fetchEncodedContent()
{
  QgsNetworkContentFetcher fetcher;
  //test fetching encoded content as string
  mLoaded = false;
  fetcher.fetchContent( QUrl::fromLocalFile( QString( TEST_DATA_DIR ) + QDir::separator() +  "encoded_html.html" ) );
  connect( &fetcher, SIGNAL( finished() ), this, SLOT( contentLoaded() ) );
  while ( !mLoaded )
  {
    qApp->processEvents();
  }
  QVERIFY( fetcher.reply()->error() == QNetworkReply::NoError );

  //test retrieved content and check for correct detection of encoding
  QString mFetchedHtml = fetcher.contentAsString();
  QVERIFY( mFetchedHtml.contains( QChar( 6040 ) ) );
}

void TestQgsNetworkContentFetcher::contentLoaded()
{
  mLoaded = true;
}

QTEST_MAIN( TestQgsNetworkContentFetcher )
#include "testqgsnetworkcontentfetcher.moc"
