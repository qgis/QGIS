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
#include "qgsmultirenderchecker.h"
#include "qgsfontutils.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectordataprovider.h"
#include "qgsproject.h"
#include <QObject>
#include <QtTest/QtTest>

class TestQgsComposerHtml : public QObject
{
    Q_OBJECT

  public:
    TestQgsComposerHtml();

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
    void javascriptSetFeature(); //test that JavaScript setFeature() function is correctly called

  private:
    QgsComposition *mComposition;
    QgsMapSettings *mMapSettings;
    QString mReport;
    QFont mTestFont;
};

TestQgsComposerHtml::TestQgsComposerHtml()
    : mComposition( 0 )
    , mMapSettings( 0 )
{

}

void TestQgsComposerHtml::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mMapSettings = new QgsMapSettings();
  mComposition = new QgsComposition( *mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  mReport = "<h1>Composer HTML Tests</h1>\n";

  QgsFontUtils::loadStandardTestFonts( QStringList() << "Oblique" );
  mTestFont = QgsFontUtils::getStandardTestFont( "Oblique " );
}

void TestQgsComposerHtml::cleanupTestCase()
{
  delete mComposition;
  delete mMapSettings;

  QString myReportFile = QDir::tempPath() + "/qgistest.html";
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
  checker.setControlPathPrefix( "composer_html" );
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
  checker.setControlPathPrefix( "composer_html" );
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
  checker.setControlPathPrefix( "composer_html" );
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
  checker.setControlPathPrefix( "composer_html" );
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
  htmlItem->setUrl( QUrl( QString( "file:///%1/test_html.html" ).arg( TEST_DATA_DIR ) ) );

  QgsCompositionChecker checker( "composerhtml_table", mComposition );
  checker.setControlPathPrefix( "composer_html" );
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
  htmlItem->setUrl( QUrl( QString( "file:///%1/test_html.html" ).arg( TEST_DATA_DIR ) ) );
  htmlItem->frame( 0 )->setFrameEnabled( true );
  QgsCompositionChecker checker1( "composerhtml_multiframe1", mComposition );
  checker1.setControlPathPrefix( "composer_html" );
  bool result = checker1.testComposition( mReport );

  //page2
  QgsCompositionChecker checker2( "composerhtml_multiframe2", mComposition );
  checker2.setControlPathPrefix( "composer_html" );
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
  htmlItem->setUrl( QUrl( QString( "file:///%1/test_html.html" ).arg( TEST_DATA_DIR ) ) );
  htmlItem->frame( 0 )->setFrameEnabled( true );
  QgsCompositionChecker checker1( "composerhtml_smartbreaks1", mComposition );
  checker1.setControlPathPrefix( "composer_html" );
  bool result = checker1.testComposition( mReport, 0, 200 );

  //page2
  QgsCompositionChecker checker2( "composerhtml_smartbreaks2", mComposition );
  checker2.setControlPathPrefix( "composer_html" );
  result = checker2.testComposition( mReport, 1, 200 ) && result;

  mComposition->removeMultiFrame( htmlItem );
  delete htmlItem;
  QVERIFY( result );
}

void TestQgsComposerHtml::javascriptSetFeature()
{
  //test that JavaScript setFeature() function is correctly called

  // first need to setup some layers with a relation

  //parent layer
  QgsVectorLayer* parentLayer = new QgsVectorLayer( "Point?field=fldtxt:string&field=fldint:integer&field=foreignkey:integer", "parent", "memory" );
  QgsVectorDataProvider* pr = parentLayer->dataProvider();
  QgsFeature pf1;
  pf1.setFields( parentLayer->fields() );
  pf1.setAttributes( QgsAttributes() << "test1" << 67 <<  123 );
  QgsFeature pf2;
  pf2.setFields( parentLayer->fields() );
  pf2.setAttributes( QgsAttributes() << "test2" << 68 << 124 );
  QVERIFY( pr->addFeatures( QgsFeatureList() << pf1 << pf2 ) );

  // child layer
  QgsVectorLayer* childLayer = new QgsVectorLayer( "Point?field=x:string&field=y:integer&field=z:integer", "referencedlayer", "memory" );
  pr = childLayer->dataProvider();
  QgsFeature f1;
  f1.setFields( childLayer->fields() );
  f1.setAttributes( QgsAttributes() << "foo" << 123 << 321 );
  QgsFeature f2;
  f2.setFields( childLayer->fields() );
  f2.setAttributes( QgsAttributes() << "bar" <<  123 <<  654 );
  QgsFeature f3;
  f3.setFields( childLayer->fields() );
  f3.setAttributes( QgsAttributes() << "foobar" << 124 <<  554 );
  QVERIFY( pr->addFeatures( QgsFeatureList() << f1 <<  f2 <<  f3 ) );

  QgsMapLayerRegistry::instance()->addMapLayers( QList<QgsMapLayer *>() << childLayer << parentLayer );

  //atlas
  mComposition->atlasComposition().setCoverageLayer( parentLayer );
  mComposition->atlasComposition().setEnabled( true );

  QgsRelation rel;
  rel.setRelationId( "rel1" );
  rel.setRelationName( "relation one" );
  rel.setReferencingLayer( childLayer->id() );
  rel.setReferencedLayer( parentLayer->id() );
  rel.addFieldPair( "y", "foreignkey" );
  QgsProject::instance()->relationManager()->addRelation( rel );

  QgsComposerHtml* htmlItem = new QgsComposerHtml( mComposition, false );
  QgsComposerFrame* htmlFrame = new QgsComposerFrame( mComposition, htmlItem, 0, 0, 100, 200 );
  htmlFrame->setFrameEnabled( true );
  htmlItem->addFrame( htmlFrame );
  htmlItem->setContentMode( QgsComposerHtml::ManualHtml );
  htmlItem->setEvaluateExpressions( true );
  // hopefully arial bold 40px is big enough to avoid cross-platform rendering issues
  htmlItem->setHtml( QString( "<body style=\"margin: 10px; font-family: Arial; font-weight: bold; font-size: 40px;\">"
                              "<div id=\"dest\"></div><script>setFeature=function(feature){"
                              "document.getElementById('dest').innerHTML = feature.properties.foreignkey + ',' +"
                              "  feature.properties['relation one'][0].z + ',' + feature.properties['relation one'][1].z;}"
                              "</script></body>" ) );

  mComposition->setAtlasMode( QgsComposition::ExportAtlas );
  QVERIFY( mComposition->atlasComposition().beginRender() );
  QVERIFY( mComposition->atlasComposition().prepareForFeature( 0 ) );

  htmlItem->loadHtml();

  QgsCompositionChecker checker( "composerhtml_setfeature", mComposition );
  checker.setControlPathPrefix( "composer_html" );
  bool result = checker.testComposition( mReport );
  mComposition->removeMultiFrame( htmlItem );
  delete htmlItem;
  QVERIFY( result );

  QgsMapLayerRegistry::instance()->removeMapLayers( QList<QgsMapLayer *>() << childLayer << parentLayer );
}


QTEST_MAIN( TestQgsComposerHtml )
#include "testqgscomposerhtml.moc"
