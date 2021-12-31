/***************************************************************************
                         testqgslayoutlabel.cpp
                         ----------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsapplication.h"
#include "qgslayout.h"
#include "qgslayoutitemlabel.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsmultirenderchecker.h"
#include "qgsfontutils.h"
#include "qgsproject.h"
#include "qgsprintlayout.h"
#include "qgslayoutatlas.h"
#include "qgslayoutpagecollection.h"

#include <QObject>
#include "qgstest.h"

class TestQgsLayoutLabel : public QObject
{
    Q_OBJECT

  public:
    TestQgsLayoutLabel() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    // test simple expression evaluation
    void evaluation();
    // test expression evaluation when a feature is set
    void featureEvaluationUsingAtlas();
    void featureEvaluationUsingContext();
    // test page expressions
    void pageEvaluation();
    void pageSizeEvaluation();
    void marginMethods(); //tests getting/setting margins
    void render();
    void renderAsHtml();
    void renderAsHtmlRelative();
    void labelRotation();

  private:
    QgsVectorLayer *mVectorLayer = nullptr;
    QString mReport;
};

void TestQgsLayoutLabel::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create maplayers from testdata and add to layer registry
  const QFileInfo vectorFileInfo( QStringLiteral( TEST_DATA_DIR ) + '/' +  "france_parts.shp" );
  mVectorLayer = new QgsVectorLayer( vectorFileInfo.filePath(),
                                     vectorFileInfo.completeBaseName(),
                                     QStringLiteral( "ogr" ) );

}

void TestQgsLayoutLabel::cleanupTestCase()
{
  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  delete mVectorLayer;

  QgsApplication::exitQgis();
}

void TestQgsLayoutLabel::init()
{
}

void TestQgsLayoutLabel::cleanup()
{
}

void TestQgsLayoutLabel::evaluation()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  l.reportContext().setLayer( mVectorLayer );

  QgsLayoutItemLabel *label = new QgsLayoutItemLabel( &l );
  label->setMargin( 1 );
  l.addLayoutItem( label );

  {
    // $CURRENT_DATE evaluation
    const QString expected = "__" + QDate::currentDate().toString() + "__";
    label->setText( QStringLiteral( "__$CURRENT_DATE__" ) );
    const QString evaluated = label->currentText();
    QCOMPARE( evaluated, expected );
  }
  {
    // $CURRENT_DATE() evaluation
    const QDateTime now = QDateTime::currentDateTime();
    const QString expected = "__" + now.toString( QStringLiteral( "dd" ) ) + "(ok)__";
    label->setText( QStringLiteral( "__$CURRENT_DATE(dd)(ok)__" ) );
    const QString evaluated = label->currentText();
    QCOMPARE( evaluated, expected );
  }
  {
    // $CURRENT_DATE() evaluation (inside an expression)
    const QDate now = QDate::currentDate();
    const int dd = now.day();

    const QString expected = "__" + QString::number( dd + 1 ) + "(ok)__";
    label->setText( QStringLiteral( "__[%$CURRENT_DATE(dd) + 1%](ok)__" ) );
    const QString evaluated = label->currentText();
    QCOMPARE( evaluated, expected );
  }
  {
    // expression evaluation (without feature)
    const QString expected = QStringLiteral( "__[NAME_1]42__" );
    label->setText( QStringLiteral( "__[%try(\"NAME_1\", '[NAME_1]')%][%21*2%]__" ) );
    const QString evaluated = label->currentText();
    QCOMPARE( evaluated, expected );
  }
}

void TestQgsLayoutLabel::featureEvaluationUsingAtlas()
{
  QgsPrintLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemLabel *label = new QgsLayoutItemLabel( &l );
  label->setMargin( 1 );
  l.addLayoutItem( label );

  l.atlas()->setEnabled( true );
  l.atlas()->setCoverageLayer( mVectorLayer );
  QVERIFY( l.atlas()->beginRender() );
  l.atlas()->seekTo( 0 );
  {
    // evaluation with a feature
    label->setText( QStringLiteral( "[%\"NAME_1\"||'_ok'%]" ) );
    const QString evaluated = label->currentText();
    const QString expected = QStringLiteral( "Basse-Normandie_ok" );
    QCOMPARE( evaluated, expected );
  }
  l.atlas()->seekTo( 1 );
  {
    // evaluation with a feature
    label->setText( QStringLiteral( "[%\"NAME_1\"||'_ok'%]" ) );
    const QString evaluated = label->currentText();
    const QString expected = QStringLiteral( "Bretagne_ok" );
    QCOMPARE( evaluated, expected );
  }
}

void TestQgsLayoutLabel::featureEvaluationUsingContext()
{
  // just using context, no atlas
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemLabel *label = new QgsLayoutItemLabel( &l );
  label->setMargin( 1 );
  l.addLayoutItem( label );

  QgsFeature f;
  QgsFeatureIterator it = mVectorLayer->getFeatures();

  l.reportContext().setLayer( mVectorLayer );
  it.nextFeature( f );
  l.reportContext().setFeature( f );
  {
    // evaluation with a feature
    label->setText( QStringLiteral( "[%\"NAME_1\"||'_ok'%]" ) );
    const QString evaluated = label->currentText();
    const QString expected = QStringLiteral( "Basse-Normandie_ok" );
    QCOMPARE( evaluated, expected );
  }
  it.nextFeature( f );
  l.reportContext().setFeature( f );
  {
    // evaluation with a feature
    label->setText( QStringLiteral( "[%\"NAME_1\"||'_ok'%]" ) );
    const QString evaluated = label->currentText();
    const QString expected = QStringLiteral( "Bretagne_ok" );
    QCOMPARE( evaluated, expected );
  }
}

void TestQgsLayoutLabel::pageEvaluation()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemPage *page2 = new QgsLayoutItemPage( &l );
  page2->setPageSize( "A4", QgsLayoutItemPage::Landscape );
  l.pageCollection()->addPage( page2 );
  l.reportContext().setLayer( mVectorLayer );

  QgsLayoutItemLabel *label = new QgsLayoutItemLabel( &l );
  label->setMargin( 1 );
  l.addLayoutItem( label );

  {
    label->setText( QStringLiteral( "[%@layout_page||'/'||@layout_numpages%]" ) );
    const QString evaluated = label->currentText();
    const QString expected = QStringLiteral( "1/2" );
    QCOMPARE( evaluated, expected );

    // move to the second page and re-evaluate
    label->attemptMove( QgsLayoutPoint( 0, 320 ) );
    QCOMPARE( label->currentText(), QString( "2/2" ) );
  }
}

void TestQgsLayoutLabel::pageSizeEvaluation()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemLabel *label = new QgsLayoutItemLabel( &l );
  label->setMargin( 1 );
  label->setText( QStringLiteral( "[%array_to_string(@layout_pageoffsets)%]" ) );
  l.addLayoutItem( label );

  {
    const QString evaluated = label->currentText();
    const QString expected = QStringLiteral( "0" );
    QCOMPARE( evaluated, expected );
  }

  // add a page and re-evaluate
  QgsLayoutItemPage *page2 = new QgsLayoutItemPage( &l );
  page2->setPageSize( "A4", QgsLayoutItemPage::Landscape );
  l.pageCollection()->addPage( page2 );

  {
    const QString evaluated = label->currentText();
    const QString expected = QStringLiteral( "0,220" );
    QCOMPARE( evaluated, expected );
  }
}

void TestQgsLayoutLabel::marginMethods()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemLabel *label = new QgsLayoutItemLabel( &l );
  label->setMargin( 1 );
  l.addLayoutItem( label );

  //test setting margins separately
  label->setMarginX( 3.0 );
  label->setMarginY( 4.0 );
  QCOMPARE( label->marginX(), 3.0 );
  QCOMPARE( label->marginY(), 4.0 );
  //test setting margins together
  label->setMargin( 5.0 );
  QCOMPARE( label->marginX(), 5.0 );
  QCOMPARE( label->marginY(), 5.0 );
}

void TestQgsLayoutLabel::render()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemLabel *label = new QgsLayoutItemLabel( &l );
  label->setMargin( 1 );
  l.addLayoutItem( label );

  label->setText( QStringLiteral( "test label" ) );
  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );
  format.setSize( 48 );
  format.setSizeUnit( QgsUnitTypes::RenderPoints );
  label->setTextFormat( format );
  label->attemptMove( QgsLayoutPoint( 70, 70 ) );
  label->adjustSizeToText();

  QgsLayoutChecker checker( QStringLiteral( "composerlabel_render" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_label" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
}

void TestQgsLayoutLabel::renderAsHtml()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemLabel *label = new QgsLayoutItemLabel( &l );
  label->setMargin( 1 );
  l.addLayoutItem( label );

  label->setText( QStringLiteral( "test <i>html</i>" ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );
  format.setSize( 48 );
  format.setSizeUnit( QgsUnitTypes::RenderPoints );
  format.setColor( QColor( 200, 40, 60 ) );
  label->setTextFormat( format );

  label->setPos( 70, 70 );
  label->adjustSizeToText();
  label->setMode( QgsLayoutItemLabel::ModeHtml );
  label->update();

  QgsLayoutChecker checker( QStringLiteral( "composerlabel_renderhtml" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_label" ) );
  QVERIFY( checker.testLayout( mReport, 0, 10 ) );
}

void TestQgsLayoutLabel::renderAsHtmlRelative()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  l.renderContext().mIsPreviewRender = false;
  QgsLayoutItemLabel *label = new QgsLayoutItemLabel( &l );
  label->setMargin( 1 );
  l.addLayoutItem( label );

  QgsProject::instance()->setFileName( QStringLiteral( TEST_DATA_DIR ) +  QDir::separator() + "test.qgs" );
  label->setText( QStringLiteral( "test <img src=\"small_sample_image.png\" />" ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );
  format.setSize( 48 );
  format.setSizeUnit( QgsUnitTypes::RenderPoints );
  format.setColor( QColor( 200, 40, 60 ) );
  label->setTextFormat( format );

  label->setPos( 70, 70 );
  label->adjustSizeToText();
  label->setMode( QgsLayoutItemLabel::ModeHtml );
  label->update();

  QgsLayoutChecker checker( QStringLiteral( "composerlabel_renderhtmlrelative" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_label" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
}

void TestQgsLayoutLabel::labelRotation()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemLabel *label = new QgsLayoutItemLabel( &l );
  label->setMargin( 1 );
  l.addLayoutItem( label );
  label->setText( QStringLiteral( "test label" ) );

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );
  format.setSize( 30 );
  format.setSizeUnit( QgsUnitTypes::RenderPoints );
  label->setTextFormat( format );

  label->attemptMove( QgsLayoutPoint( 70, 70 ) );
  label->adjustSizeToText();
  label->setBackgroundColor( QColor::fromRgb( 255, 150, 0 ) );
  label->setBackgroundEnabled( true );
  label->setItemRotation( 135 );

  QgsLayoutChecker checker( QStringLiteral( "layoutrotation_label" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_items" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
}

QGSTEST_MAIN( TestQgsLayoutLabel )
#include "testqgslayoutlabel.moc"
