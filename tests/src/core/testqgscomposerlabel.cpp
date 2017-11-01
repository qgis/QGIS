/***************************************************************************
                         testqgscomposerlabel.cpp
                         ----------------------
    begin                : Sept 2012
    copyright            : (C) 2012 by Hugo Mercier
    email                : hugo dot mercier at oslandia dot com
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
#include "qgscomposition.h"
#include "qgscomposerlabel.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsmultirenderchecker.h"
#include "qgsfontutils.h"
#include "qgsproject.h"

#include <QObject>
#include "qgstest.h"

class TestQgsComposerLabel : public QObject
{
    Q_OBJECT

  public:
    TestQgsComposerLabel() = default;

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
    QgsComposition *mComposition = nullptr;
    QgsComposerLabel *mComposerLabel = nullptr;
    QgsVectorLayer *mVectorLayer = nullptr;
    QString mReport;
};

void TestQgsComposerLabel::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create maplayers from testdata and add to layer registry
  QFileInfo vectorFileInfo( QStringLiteral( TEST_DATA_DIR ) + '/' +  "france_parts.shp" );
  mVectorLayer = new QgsVectorLayer( vectorFileInfo.filePath(),
                                     vectorFileInfo.completeBaseName(),
                                     QStringLiteral( "ogr" ) );

  //create composition with composer map
  mComposition = new QgsComposition( QgsProject::instance() );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
  mComposition->atlasComposition().setCoverageLayer( mVectorLayer );

  mComposerLabel = new QgsComposerLabel( mComposition );
  mComposition->addComposerLabel( mComposerLabel );

  qWarning() << "composer label font: " << mComposerLabel->font().toString() << " exactMatch:" << mComposerLabel->font().exactMatch();
}

void TestQgsComposerLabel::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  delete mComposition;
  delete mVectorLayer;

  QgsApplication::exitQgis();
}

void TestQgsComposerLabel::init()
{
  mComposerLabel = new QgsComposerLabel( mComposition );
  mComposition->addComposerLabel( mComposerLabel );
}

void TestQgsComposerLabel::cleanup()
{
  mComposition->removeItem( mComposerLabel );
  delete mComposerLabel;
  mComposerLabel = 0;
}

void TestQgsComposerLabel::evaluation()
{
  {
    // $CURRENT_DATE evaluation
    QString expected = "__" + QDate::currentDate().toString() + "__";
    mComposerLabel->setText( QStringLiteral( "__$CURRENT_DATE__" ) );
    QString evaluated = mComposerLabel->displayText();
    QCOMPARE( evaluated, expected );
  }
  {
    // $CURRENT_DATE() evaluation
    QDateTime now = QDateTime::currentDateTime();
    QString expected = "__" + now.toString( QStringLiteral( "dd" ) ) + "(ok)__";
    mComposerLabel->setText( QStringLiteral( "__$CURRENT_DATE(dd)(ok)__" ) );
    QString evaluated = mComposerLabel->displayText();
    QCOMPARE( evaluated, expected );
  }
  {
    // $CURRENT_DATE() evaluation (inside an expression)
    QDate now = QDate::currentDate();
    int dd = now.day();

    QString expected = "__" + QStringLiteral( "%1" ).arg( dd + 1 ) + "(ok)__";
    mComposerLabel->setText( QStringLiteral( "__[%$CURRENT_DATE(dd) + 1%](ok)__" ) );
    QString evaluated = mComposerLabel->displayText();
    QCOMPARE( evaluated, expected );
  }
  {
    // expression evaluation (without feature)
    QString expected = QStringLiteral( "__[NAME_1]42__" );
    mComposerLabel->setText( QStringLiteral( "__[%\"NAME_1\"%][%21*2%]__" ) );
    QString evaluated = mComposerLabel->displayText();
    QCOMPARE( evaluated, expected );
  }
}

void TestQgsComposerLabel::feature_evaluation()
{
  mComposition->atlasComposition().setEnabled( true );
  mComposition->setAtlasMode( QgsComposition::ExportAtlas );
  mComposition->atlasComposition().updateFeatures();
  mComposition->atlasComposition().prepareForFeature( 0 );
  {
    // evaluation with a feature
    mComposerLabel->setText( QStringLiteral( "[%\"NAME_1\"||'_ok'%]" ) );
    QString evaluated = mComposerLabel->displayText();
    QString expected = QStringLiteral( "Basse-Normandie_ok" );
    QCOMPARE( evaluated, expected );
  }
  mComposition->atlasComposition().prepareForFeature( 1 );
  {
    // evaluation with a feature
    mComposerLabel->setText( QStringLiteral( "[%\"NAME_1\"||'_ok'%]" ) );
    QString evaluated = mComposerLabel->displayText();
    QString expected = QStringLiteral( "Bretagne_ok" );
    QCOMPARE( evaluated, expected );
  }
  mComposition->atlasComposition().setEnabled( false );
}

void TestQgsComposerLabel::page_evaluation()
{
  mComposition->setNumPages( 2 );
  {
    mComposerLabel->setText( QStringLiteral( "[%@layout_page||'/'||@layout_numpages%]" ) );
    QString evaluated = mComposerLabel->displayText();
    QString expected = QStringLiteral( "1/2" );
    QCOMPARE( evaluated, expected );

    // move to the second page and re-evaluate
    mComposerLabel->setItemPosition( 0, 320 );
    QCOMPARE( mComposerLabel->displayText(), QString( "2/2" ) );
  }
}

void TestQgsComposerLabel::marginMethods()
{
  QgsComposerLabel label( mComposition );
  //test setting margins separately
  label.setMarginX( 3.0 );
  label.setMarginY( 4.0 );
  QCOMPARE( label.marginX(), 3.0 );
  QCOMPARE( label.marginY(), 4.0 );
  //test setting margins together
  label.setMargin( 5.0 );
  QCOMPARE( label.marginX(), 5.0 );
  QCOMPARE( label.marginY(), 5.0 );

  //test reading label margins from pre 2.7 projects
  QDomDocument labelDoc;
  QString labelXml;
  labelXml = QStringLiteral( "<ComposerLabel margin=\"9\"><ComposerItem></ComposerItem></ComposerLabel" );
  labelDoc.setContent( labelXml );
  QgsComposerLabel label2( mComposition );
  label2.readXml( labelDoc.firstChildElement(), labelDoc );
  QCOMPARE( label2.marginX(), 9.0 );
  QCOMPARE( label2.marginY(), 9.0 );

  //test reading label margins from >=2.7 projects
  labelXml = QStringLiteral( "<ComposerLabel marginX=\"11\" marginY=\"12\"><ComposerItem></ComposerItem></ComposerLabel" );
  labelDoc.setContent( labelXml );
  QgsComposerLabel label3( mComposition );
  label3.readXml( labelDoc.firstChildElement(), labelDoc );
  QCOMPARE( label3.marginX(), 11.0 );
  QCOMPARE( label3.marginY(), 12.0 );
}

void TestQgsComposerLabel::render()
{
  mComposerLabel->setText( QStringLiteral( "test label" ) );
  mComposerLabel->setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ), 48 ) );
  mComposerLabel->setPos( 70, 70 );
  mComposerLabel->adjustSizeToText();

  QgsCompositionChecker checker( QStringLiteral( "composerlabel_render" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_label" ) );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );
}

void TestQgsComposerLabel::renderAsHtml()
{
  mComposerLabel->setFontColor( QColor( 200, 40, 60 ) );
  mComposerLabel->setText( QStringLiteral( "test <i>html</i>" ) );
  mComposerLabel->setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ), 48 ) );
  mComposerLabel->setPos( 70, 70 );
  mComposerLabel->adjustSizeToText();
  mComposerLabel->setHtmlState( 1 );
  mComposerLabel->update();

  QgsCompositionChecker checker( QStringLiteral( "composerlabel_renderhtml" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_label" ) );
  QVERIFY( checker.testComposition( mReport, 0, 10 ) );
}

void TestQgsComposerLabel::renderAsHtmlRelative()
{
  QgsProject::instance()->setFileName( QStringLiteral( TEST_DATA_DIR ) +  QDir::separator() + "test.qgs" );
  mComposerLabel->setFontColor( QColor( 200, 40, 60 ) );
  mComposerLabel->setText( QStringLiteral( "test <img src=\"small_sample_image.png\" />" ) );
  mComposerLabel->setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ), 48 ) );
  mComposerLabel->setPos( 70, 70 );
  mComposerLabel->adjustSizeToText();
  mComposerLabel->setHtmlState( 1 );
  mComposerLabel->update();

  QgsCompositionChecker checker( QStringLiteral( "composerlabel_renderhtmlrelative" ), mComposition );
  checker.setControlPathPrefix( QStringLiteral( "composer_label" ) );
  QVERIFY( checker.testComposition( mReport, 0, 0 ) );
}

QGSTEST_MAIN( TestQgsComposerLabel )
#include "testqgscomposerlabel.moc"
