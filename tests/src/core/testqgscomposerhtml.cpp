/***************************************************************************
                         testqgscomposerhtml.cpp
                         -----------------------
    begin                : August 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
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

class TestQgsComposerHtml: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void table(); //test if rendering of the composition with composr map is correct
    void tableMultiFrame(); //tests multiframe capabilities of composer html
    void htmlMultiFrameSmarkBreak(); //tests smart page breaks in html multi frame
  private:
    QgsComposition* mComposition;
    QgsMapSettings mMapSettings;
    QString mReport;
};

void TestQgsComposerHtml::initTestCase()
{
  mComposition = new QgsComposition( mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  mReport = "<h1>Composer HTML Tests</h1>\n";
}

void TestQgsComposerHtml::cleanupTestCase()
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

void TestQgsComposerHtml::init()
{

}

void TestQgsComposerHtml::cleanup()
{

}

void TestQgsComposerHtml::table()
{
  QgsComposerHtml* htmlItem = new QgsComposerHtml( mComposition, false );
  QgsComposerFrame* htmlFrame = new QgsComposerFrame( mComposition, htmlItem, 0, 0, 100, 200 );
  htmlFrame->setFrameEnabled( true );
  htmlItem->addFrame( htmlFrame );
  htmlItem->setUrl( QUrl( QString( "file:///%1" ).arg( QString( TEST_DATA_DIR ) + QDir::separator() +  "html_table.html" ) ) );

  QgsCompositionChecker checker( "composerhtml_table", mComposition );
  bool result = checker.testComposition( mReport );
  mComposition->removeMultiFrame( htmlItem );
  delete htmlItem;
  QVERIFY( result );
}

void TestQgsComposerHtml::tableMultiFrame()
{
  QgsComposerHtml* htmlItem = new QgsComposerHtml( mComposition, false );
  QgsComposerFrame* htmlFrame = new QgsComposerFrame( mComposition, htmlItem, 10, 10, 100, 50 );
  htmlItem->addFrame( htmlFrame );
  htmlItem->setResizeMode( QgsComposerMultiFrame::RepeatUntilFinished );
  htmlItem->setUseSmartBreaks( false );

  //page1
  htmlItem->setUrl( QUrl( QString( "file:///%1" ).arg( QString( TEST_DATA_DIR ) + QDir::separator() +  "html_table.html" ) ) );
  htmlItem->frame( 0 )->setFrameEnabled( true );
  QgsCompositionChecker checker1( "composerhtml_multiframe1", mComposition );
  bool result = checker1.testComposition( mReport );

  //page2
  QgsCompositionChecker checker2( "composerhtml_multiframe2", mComposition );
  result = checker2.testComposition( mReport, 1 ) && result;
  //page 3
  QgsCompositionChecker checker3( "composerhtml_multiframe3", mComposition );
  result = checker3.testComposition( mReport, 2 ) && result;

  mComposition->removeMultiFrame( htmlItem );
  delete htmlItem;
  QVERIFY( result );
}

void TestQgsComposerHtml::htmlMultiFrameSmarkBreak()
{
  QgsComposerHtml* htmlItem = new QgsComposerHtml( mComposition, false );
  QgsComposerFrame* htmlFrame = new QgsComposerFrame( mComposition, htmlItem, 10, 10, 100, 50 );
  htmlItem->addFrame( htmlFrame );
  htmlItem->setResizeMode( QgsComposerMultiFrame::RepeatUntilFinished );
  htmlItem->setUseSmartBreaks( true );

  //page1
  htmlItem->setUrl( QUrl( QString( "file:///%1" ).arg( QString( TEST_DATA_DIR ) + QDir::separator() +  "html_table.html" ) ) );
  htmlItem->frame( 0 )->setFrameEnabled( true );
  QgsCompositionChecker checker1( "composerhtml_smartbreaks1", mComposition );
  bool result = checker1.testComposition( mReport );

  //page2
  QgsCompositionChecker checker2( "composerhtml_smartbreaks2", mComposition );
  result = checker2.testComposition( mReport, 1 ) && result;
  //page 3
  QgsCompositionChecker checker3( "composerhtml_smartbreaks3", mComposition );
  result = checker3.testComposition( mReport, 2 ) && result;

  mComposition->removeMultiFrame( htmlItem );
  delete htmlItem;
  QVERIFY( result );
}


QTEST_MAIN( TestQgsComposerHtml )
#include "moc_testqgscomposerhtml.cxx"
