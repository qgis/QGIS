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

#include "qgsapplication.h"
#include "qgscomposerhtml.h"
#include "qgscomposerframe.h"
#include "qgscomposition.h"
#include "qgscompositionchecker.h"
#include "qgsfontutils.h"
#include <QObject>
#include <QtTest/QtTest>

class TestQgsComposerHtml: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void sourceMode(); //test if rendering manual HTML works
    void userStylesheets(); //test if user stylesheets work
    void evalExpressions(); //test if rendering with expressions works
    void evalExpressionsOff(); //test if rendering with expressions disabled works
    void table(); //test if rendering a HTML url works
    void tableMultiFrame(); //tests multiframe capabilities of composer html
    void htmlMultiFrameSmartBreak(); //tests smart page breaks in html multi frame
  private:
    QgsComposition* mComposition;
    QgsMapSettings mMapSettings;
    QString mReport;
    QFont mTestFont;
};

void TestQgsComposerHtml::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mComposition = new QgsComposition( mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  mReport = "<h1>Composer HTML Tests</h1>\n";

  QgsFontUtils::loadStandardTestFonts( QStringList() << "Oblique" );
  mTestFont = QgsFontUtils::getStandardTestFont( "Oblique " );
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
  QgsApplication::exitQgis();
}

void TestQgsComposerHtml::init()
{

}

void TestQgsComposerHtml::cleanup()
{

}

void TestQgsComposerHtml::sourceMode()
{
  QgsComposerHtml* htmlItem = new QgsComposerHtml( mComposition, false );
  QgsComposerFrame* htmlFrame = new QgsComposerFrame( mComposition, htmlItem, 0, 0, 100, 200 );
  htmlFrame->setFrameEnabled( true );
  htmlItem->addFrame( htmlFrame );
  htmlItem->setContentMode( QgsComposerHtml::ManualHtml );
  htmlItem->setHtml( QString( "<body style=\"margin: 10px;\"><div style=\"width: 100px; height: 50px; background-color: red;\"></div></body>" ) );
  htmlItem->loadHtml();

  QgsCompositionChecker checker( "composerhtml_manual", mComposition );
  bool result = checker.testComposition( mReport, 0, 100 );
  mComposition->removeMultiFrame( htmlItem );
  delete htmlItem;
  QVERIFY( result );
}

void TestQgsComposerHtml::userStylesheets()
{
  QgsComposerHtml* htmlItem = new QgsComposerHtml( mComposition, false );
  QgsComposerFrame* htmlFrame = new QgsComposerFrame( mComposition, htmlItem, 0, 0, 100, 200 );
  htmlFrame->setFrameEnabled( true );
  htmlItem->addFrame( htmlFrame );
  htmlItem->setContentMode( QgsComposerHtml::ManualHtml );
  htmlItem->setHtml( QString( "<body style=\"margin: 10px;\"><div style=\"width: 100px; height: 50px; background-color: red;\"></div></body>" ) );

  //set user stylesheet
  htmlItem->setUserStylesheet( QString( "div { background-color: green !important; }" ) );
  //setting user stylesheet enabled automatically loads html
  htmlItem->setUserStylesheetEnabled( true );

  QgsCompositionChecker checker( "composerhtml_userstylesheet", mComposition );
  bool result = checker.testComposition( mReport, 0, 100 );
  mComposition->removeMultiFrame( htmlItem );
  delete htmlItem;
  QVERIFY( result );
}

void TestQgsComposerHtml::evalExpressions()
{
  QgsComposerHtml* htmlItem = new QgsComposerHtml( mComposition, false );
  QgsComposerFrame* htmlFrame = new QgsComposerFrame( mComposition, htmlItem, 0, 0, 100, 200 );
  htmlFrame->setFrameEnabled( true );
  htmlItem->addFrame( htmlFrame );
  htmlItem->setContentMode( QgsComposerHtml::ManualHtml );
  htmlItem->setEvaluateExpressions( true );
  htmlItem->setHtml( QString( "<body style=\"margin: 10px;\"><div style=\"width: [% 10 * 10 %]px; height: [% 30 + 20 %]px; background-color: [% 'yel' || 'low' %];\"></div></body>" ) );

  htmlItem->loadHtml();

  QgsCompositionChecker checker( "composerhtml_expressions_enabled", mComposition );
  bool result = checker.testComposition( mReport );
  mComposition->removeMultiFrame( htmlItem );
  delete htmlItem;
  QVERIFY( result );
}

void TestQgsComposerHtml::evalExpressionsOff()
{
  QgsComposerHtml* htmlItem = new QgsComposerHtml( mComposition, false );
  QgsComposerFrame* htmlFrame = new QgsComposerFrame( mComposition, htmlItem, 0, 0, 100, 200 );
  htmlFrame->setFrameEnabled( true );
  htmlItem->addFrame( htmlFrame );
  htmlItem->setContentMode( QgsComposerHtml::ManualHtml );
  htmlItem->setEvaluateExpressions( false );
  htmlItem->setHtml( QString( "<body style=\"margin: 10px;\"><div style=\"width: [% 10 * 10 %]px; height: [% 30 + 20 %]px; background-color: [% 'yel' || 'low' %];\"></div></body>" ) );
  htmlItem->loadHtml();

  QgsCompositionChecker checker( "composerhtml_expressions_disabled", mComposition );
  bool result = checker.testComposition( mReport );
  mComposition->removeMultiFrame( htmlItem );
  delete htmlItem;
  QVERIFY( result );
}

void TestQgsComposerHtml::table()
{
  QgsComposerHtml* htmlItem = new QgsComposerHtml( mComposition, false );
  QgsComposerFrame* htmlFrame = new QgsComposerFrame( mComposition, htmlItem, 0, 0, 100, 200 );
  htmlFrame->setFrameEnabled( true );
  htmlItem->addFrame( htmlFrame );
  htmlItem->setUrl( QUrl( QString( "file:///%1" ).arg( QString( TEST_DATA_DIR ) + QDir::separator() +  "test_html.html" ) ) );

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
  htmlItem->setUrl( QUrl( QString( "file:///%1" ).arg( QString( TEST_DATA_DIR ) + QDir::separator() +  "test_html.html" ) ) );
  htmlItem->frame( 0 )->setFrameEnabled( true );
  QgsCompositionChecker checker1( "composerhtml_multiframe1", mComposition );
  bool result = checker1.testComposition( mReport );

  //page2
  QgsCompositionChecker checker2( "composerhtml_multiframe2", mComposition );
  result = checker2.testComposition( mReport, 1 ) && result;

  mComposition->removeMultiFrame( htmlItem );
  delete htmlItem;
  QVERIFY( result );
}

void TestQgsComposerHtml::htmlMultiFrameSmartBreak()
{
  QgsComposerHtml* htmlItem = new QgsComposerHtml( mComposition, false );
  QgsComposerFrame* htmlFrame = new QgsComposerFrame( mComposition, htmlItem, 10, 10, 100, 52 );
  htmlItem->addFrame( htmlFrame );
  htmlItem->setResizeMode( QgsComposerMultiFrame::RepeatUntilFinished );
  htmlItem->setUseSmartBreaks( true );

  //page1
  htmlItem->setUrl( QUrl( QString( "file:///%1" ).arg( QString( TEST_DATA_DIR ) + QDir::separator() +  "test_html.html" ) ) );
  htmlItem->frame( 0 )->setFrameEnabled( true );
  QgsCompositionChecker checker1( "composerhtml_smartbreaks1", mComposition );
  bool result = checker1.testComposition( mReport, 0, 200 );

  //page2
  QgsCompositionChecker checker2( "composerhtml_smartbreaks2", mComposition );
  result = checker2.testComposition( mReport, 1, 200 ) && result;

  mComposition->removeMultiFrame( htmlItem );
  delete htmlItem;
  QVERIFY( result );
}


QTEST_MAIN( TestQgsComposerHtml )
#include "testqgscomposerhtml.moc"
