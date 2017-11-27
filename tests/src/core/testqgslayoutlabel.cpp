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
    void feature_evaluation();
    // test page expressions
    void page_evaluation();
    void marginMethods(); //tests getting/setting margins
    void render();
    void renderAsHtml();
    void renderAsHtmlRelative();

  private:
    QgsVectorLayer *mVectorLayer = nullptr;
    QString mReport;
};

void TestQgsLayoutLabel::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create maplayers from testdata and add to layer registry
  QFileInfo vectorFileInfo( QStringLiteral( TEST_DATA_DIR ) + '/' +  "france_parts.shp" );
  mVectorLayer = new QgsVectorLayer( vectorFileInfo.filePath(),
                                     vectorFileInfo.completeBaseName(),
                                     QStringLiteral( "ogr" ) );

}

void TestQgsLayoutLabel::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
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

#if 0 //TODO
  l.atlasComposition().setCoverageLayer( mVectorLayer );
#endif

  QgsLayoutItemLabel *label = new QgsLayoutItemLabel( &l );
  label->setMargin( 1 );
  l.addLayoutItem( label );

  qWarning() << "composer label font: " << label->font().toString() << " exactMatch:" << label->font().exactMatch();


  {
    // $CURRENT_DATE evaluation
    QString expected = "__" + QDate::currentDate().toString() + "__";
    label->setText( QStringLiteral( "__$CURRENT_DATE__" ) );
    QString evaluated = label->currentText();
    QCOMPARE( evaluated, expected );
  }
  {
    // $CURRENT_DATE() evaluation
    QDateTime now = QDateTime::currentDateTime();
    QString expected = "__" + now.toString( QStringLiteral( "dd" ) ) + "(ok)__";
    label->setText( QStringLiteral( "__$CURRENT_DATE(dd)(ok)__" ) );
    QString evaluated = label->currentText();
    QCOMPARE( evaluated, expected );
  }
  {
    // $CURRENT_DATE() evaluation (inside an expression)
    QDate now = QDate::currentDate();
    int dd = now.day();

    QString expected = "__" + QStringLiteral( "%1" ).arg( dd + 1 ) + "(ok)__";
    label->setText( QStringLiteral( "__[%$CURRENT_DATE(dd) + 1%](ok)__" ) );
    QString evaluated = label->currentText();
    QCOMPARE( evaluated, expected );
  }
  {
    // expression evaluation (without feature)
    QString expected = QStringLiteral( "__[NAME_1]42__" );
    label->setText( QStringLiteral( "__[%\"NAME_1\"%][%21*2%]__" ) );
    QString evaluated = label->currentText();
    QCOMPARE( evaluated, expected );
  }
}

void TestQgsLayoutLabel::feature_evaluation()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
#if 0 //TODO
  l.atlasComposition().setCoverageLayer( mVectorLayer );
#endif

  QgsLayoutItemLabel *label = new QgsLayoutItemLabel( &l );
  label->setMargin( 1 );
  l.addLayoutItem( label );

#if 0 //TODO
  l.atlasComposition().setEnabled( true );
  l.setAtlasMode( QgsComposition::ExportAtlas );
  l.atlasComposition().updateFeatures();
  l.atlasComposition().prepareForFeature( 0 );

  {
    // evaluation with a feature
    label->setText( QStringLiteral( "[%\"NAME_1\"||'_ok'%]" ) );
    QString evaluated = label->displayText();
    QString expected = QStringLiteral( "Basse-Normandie_ok" );
    QCOMPARE( evaluated, expected );
  }
  mComposition->atlasComposition().prepareForFeature( 1 );
  {
    // evaluation with a feature
    label->setText( QStringLiteral( "[%\"NAME_1\"||'_ok'%]" ) );
    QString evaluated = label->displayText();
    QString expected = QStringLiteral( "Bretagne_ok" );
    QCOMPARE( evaluated, expected );
  }
  mComposition->atlasComposition().setEnabled( false );
#endif
}

void TestQgsLayoutLabel::page_evaluation()
{
#if 0 //TODO
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  mComposition->atlasComposition().setCoverageLayer( mVectorLayer );

  QgsLayoutItemLabel *label = new QgsLayoutItemLabel( &l );
  label->setMargin( 1 );
  l.addLayoutItem( label );

  mComposition->setNumPages( 2 );
  {
    label->setText( QStringLiteral( "[%@layout_page||'/'||@layout_numpages%]" ) );
    QString evaluated = label->displayText();
    QString expected = QStringLiteral( "1/2" );
    QCOMPARE( evaluated, expected );

    // move to the second page and re-evaluate
    label->setItemPosition( 0, 320 );
    QCOMPARE( label->displayText(), QString( "2/2" ) );
  }
#endif
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
  label->setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ), 48 ) );
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

  label->setFontColor( QColor( 200, 40, 60 ) );
  label->setText( QStringLiteral( "test <i>html</i>" ) );
  label->setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ), 48 ) );
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
  QgsLayoutItemLabel *label = new QgsLayoutItemLabel( &l );
  label->setMargin( 1 );
  l.addLayoutItem( label );

  QgsProject::instance()->setFileName( QStringLiteral( TEST_DATA_DIR ) +  QDir::separator() + "test.qgs" );
  label->setFontColor( QColor( 200, 40, 60 ) );
  label->setText( QStringLiteral( "test <img src=\"small_sample_image.png\" />" ) );
  label->setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ), 48 ) );
  label->setPos( 70, 70 );
  label->adjustSizeToText();
  label->setMode( QgsLayoutItemLabel::ModeHtml );
  label->update();

  QgsLayoutChecker checker( QStringLiteral( "composerlabel_renderhtmlrelative" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_label" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
}

QGSTEST_MAIN( TestQgsLayoutLabel )
#include "testqgslayoutlabel.moc"
