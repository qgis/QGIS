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
  QgsComposerHtml* htmlItem = new QgsComposerHtml( mComposition, 0, 0, 100, 200 );
  htmlItem->setUrl( QUrl( QString( "file:///%1" ).arg( QString( TEST_DATA_DIR ) + QDir::separator() +  "html_table.html" ) ) );
  QgsCompositionChecker checker( "Composer html table", mComposition, QString( QString( TEST_DATA_DIR ) + QDir::separator() +
                                 "control_images" + QDir::separator() + "expected_composerhtml" + QDir::separator() + "composerhtml_table.png" ) );
  QVERIFY( checker.testComposition() );
}


QTEST_MAIN( TestQgsComposerHtml )
#include "moc_testqgscomposerhtml.cxx"
