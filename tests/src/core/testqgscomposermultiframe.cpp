/***************************************************************************
                         testqgscomposermultiframe.cpp
                         -----------------------
    begin                : September 2014
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

#include "qgscomposerhtml.h"
#include "qgscomposerframe.h"
#include "qgscomposition.h"
#include "qgscompositionchecker.h"
#include <QObject>
#include <QtTest>

class TestQgsComposerMultiFrame: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void frameIsEmpty(); //test if frame is empty works

  private:
    QgsComposition* mComposition;
    QgsMapSettings mMapSettings;
    QString mReport;
};

void TestQgsComposerMultiFrame::initTestCase()
{
  mComposition = new QgsComposition( mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  mReport = "<h1>Composer MultiFrame Tests</h1>\n";
}

void TestQgsComposerMultiFrame::cleanupTestCase()
{
  delete mComposition;

  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsComposerMultiFrame::init()
{

}

void TestQgsComposerMultiFrame::cleanup()
{

}

void TestQgsComposerMultiFrame::frameIsEmpty()
{
  QgsComposerHtml* htmlItem = new QgsComposerHtml( mComposition, false );
  QgsComposerFrame* frame1 = new QgsComposerFrame( mComposition, htmlItem, 0, 0, 100, 200 );
  QgsComposerFrame* frame2 = new QgsComposerFrame( mComposition, htmlItem, 0, 0, 100, 200 );
  htmlItem->addFrame( frame1 );
  htmlItem->addFrame( frame2 );
  htmlItem->setContentMode( QgsComposerHtml::ManualHtml );
  //short content, so frame 2 should be empty
  htmlItem->setHtml( QString( "<p><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( frame1->isEmpty(), false );
  QCOMPARE( frame2->isEmpty(), true );

  //long content, so frame 2 should not be empty
  htmlItem->setHtml( QString( "<p style=\"height: 10000px\"><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( frame1->isEmpty(), false );
  QCOMPARE( frame2->isEmpty(), false );

  //..and back again..
  htmlItem->setHtml( QString( "<p><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( frame1->isEmpty(), false );
  QCOMPARE( frame2->isEmpty(), true );

  mComposition->removeMultiFrame( htmlItem );
  delete htmlItem;
}

QTEST_MAIN( TestQgsComposerMultiFrame )
#include "moc_testqgscomposermultiframe.cxx"
