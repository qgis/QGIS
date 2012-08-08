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
  private:
    QgsComposition* mComposition;
};

void TestQgsComposerHtml::initTestCase()
{
  mComposition = new QgsComposition( 0 );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
}

void TestQgsComposerHtml::cleanupTestCase()
{
  delete mComposition;
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
  htmlItem->addFrame( htmlFrame );
  htmlItem->setUrl( QUrl( QString( "file:///%1" ).arg( QString( TEST_DATA_DIR ) + QDir::separator() +  "html_table.html" ) ) );
  QgsCompositionChecker checker( "Composer html table", mComposition, QString( QString( TEST_DATA_DIR ) + QDir::separator() +
                                 "control_images" + QDir::separator() + "expected_composerhtml" + QDir::separator() + "composerhtml_table.png" ) );
  bool result = checker.testComposition();
  mComposition->removeMultiFrame( htmlItem );
  delete htmlItem;
  QVERIFY( result );
}

void TestQgsComposerHtml::tableMultiFrame()
{
  QgsComposerHtml* htmlItem = new QgsComposerHtml( mComposition, false );
  QgsComposerFrame* htmlFrame = new QgsComposerFrame( mComposition, htmlItem, 10, 10, 100, 50 );
  htmlItem->addFrame( htmlFrame );
  htmlItem->setResizeMode( QgsComposerMultiFrame::ExtendToNextPage );
  bool result = true;
  //page1
  htmlItem->setUrl( QUrl( QString( "file:///%1" ).arg( QString( TEST_DATA_DIR ) + QDir::separator() +  "html_table.html" ) ) );
  QgsCompositionChecker checker1( "Composer html table", mComposition, QString( QString( TEST_DATA_DIR ) + QDir::separator() +
                                  "control_images" + QDir::separator() + "expected_composerhtml" + QDir::separator() + "composerhtml_table_multiframe1.png" ) );
  if ( !checker1.testComposition( 0 ) )
  {
    result = false;
  }
  //page2
  QgsCompositionChecker checker2( "Composer html table", mComposition, QString( QString( TEST_DATA_DIR ) + QDir::separator() +
                                  "control_images" + QDir::separator() + "expected_composerhtml" + QDir::separator() + "composerhtml_table_multiframe2.png" ) );
  if ( !checker2.testComposition( 1 ) )
  {
    result = false;
  }
  //page 3
  QgsCompositionChecker checker3( "Composer html table", mComposition, QString( QString( TEST_DATA_DIR ) + QDir::separator() +
                                  "control_images" + QDir::separator() + "expected_composerhtml" + QDir::separator() + "composerhtml_table_multiframe3.png" ) );
  if ( !checker3.testComposition( 2 ) )
  {
    result = false;
  }

  mComposition->removeMultiFrame( htmlItem );
  delete htmlItem;
  QVERIFY( result );
}


QTEST_MAIN( TestQgsComposerHtml )
#include "moc_testqgscomposerhtml.cxx"
