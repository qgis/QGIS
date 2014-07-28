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
#include <QObject>
#include <QtTest>
#include <QNetworkReply>

class TestQgsNetworkContentFetcher: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void fetchEmptyUrl(); //test fetching blank url
    void fetchBadUrl(); //test fetching bad url
    void fetchUrlContent(); //test fetching url content

    void contentLoaded();

  private:

    bool mLoaded;

};

void TestQgsNetworkContentFetcher::initTestCase()
{

}

void TestQgsNetworkContentFetcher::cleanupTestCase()
{

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

void TestQgsNetworkContentFetcher::fetchUrlContent()
{
  QgsNetworkContentFetcher fetcher;
  //test fetching content from the QGIS homepage
  mLoaded = false;
  fetcher.fetchContent( QUrl( "http://www.qgis.org/en/site/" ) );
  connect( &fetcher, SIGNAL( finished() ), this, SLOT( contentLoaded() ) );
  while ( !mLoaded )
  {
    qApp->processEvents();
  }
  QVERIFY( fetcher.reply()->error() == QNetworkReply::NoError );

  //test retrieved content
  QString mFetchedHtml = fetcher.contentAsString();
  QVERIFY( mFetchedHtml.contains( QString( "QGIS" ) ) );
}

void TestQgsNetworkContentFetcher::contentLoaded()
{
  mLoaded = true;
}

QTEST_MAIN( TestQgsNetworkContentFetcher )
#include "moc_testqgsnetworkcontentfetcher.cxx"
