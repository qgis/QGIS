/***************************************************************************
                         testqgslayouthtml.cpp
                         -----------------------
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
#include "qgsfontutils.h"
#include "qgslayout.h"
#include "qgslayoutframe.h"
#include "qgslayoutitemhtml.h"
#include "qgslayoutreportcontext.h"
#include "qgsproject.h"
#include "qgsrelationmanager.h"
#include "qgstest.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

#include <QObject>

class TestQgsLayoutHtml : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsLayoutHtml()
      : QgsTest( u"Layout HTML Tests"_s, u"composer_html"_s ) {}

  private slots:
    void initTestCase();             // will be called before the first testfunction is executed.
    void cleanupTestCase();          // will be called after the last testfunction was executed.
    void sourceMode();               //test if rendering manual HTML works
    void userStylesheets();          //test if user stylesheets work
    void evalExpressions();          //test if rendering with expressions works
    void evalExpressionsOff();       //test if rendering with expressions disabled works
    void table();                    //test if rendering a HTML url works
    void tableMultiFrame();          //tests multiframe capabilities of composer html
    void htmlMultiFrameSmartBreak(); //tests smart page breaks in html multi frame
    void javascriptSetFeature();     //test that JavaScript setFeature() function is correctly called

  private:
    QFont mTestFont;
};

void TestQgsLayoutHtml::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  QgsFontUtils::loadStandardTestFonts( QStringList() << u"Oblique"_s );
  mTestFont = QgsFontUtils::getStandardTestFont( u"Oblique "_s );
}

void TestQgsLayoutHtml::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsLayoutHtml::sourceMode()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemHtml *htmlItem = new QgsLayoutItemHtml( &l );
  QgsLayoutFrame *htmlFrame = new QgsLayoutFrame( &l, htmlItem );
  htmlFrame->attemptSetSceneRect( QRectF( 0, 0, 100, 200 ) );
  htmlFrame->setFrameEnabled( true );
  htmlItem->addFrame( htmlFrame );
  htmlItem->setContentMode( QgsLayoutItemHtml::ManualHtml );
  htmlItem->setHtml( u"<body style=\"margin: 10px;\"><div style=\"width: 100px; height: 50px; background-color: red;\"></div></body>"_s );
  htmlItem->loadHtml();

  QGSVERIFYLAYOUTCHECK( u"composerhtml_manual"_s, &l, 0, 100 );
}

void TestQgsLayoutHtml::userStylesheets()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemHtml *htmlItem = new QgsLayoutItemHtml( &l );
  QgsLayoutFrame *htmlFrame = new QgsLayoutFrame( &l, htmlItem );
  htmlFrame->attemptSetSceneRect( QRectF( 0, 0, 100, 200 ) );
  htmlFrame->setFrameEnabled( true );
  htmlItem->addFrame( htmlFrame );
  htmlItem->setContentMode( QgsLayoutItemHtml::ManualHtml );
  htmlItem->setHtml( u"<body style=\"margin: 10px;\"><div style=\"width: 100px; height: 50px; background-color: red;\"></div></body>"_s );

  //set user stylesheet
  htmlItem->setUserStylesheet( u"div { background-color: green !important; }"_s );
  //setting user stylesheet enabled automatically loads html
  htmlItem->setUserStylesheetEnabled( true );

  QGSVERIFYLAYOUTCHECK( u"composerhtml_userstylesheet"_s, &l, 0, 100 );
}

void TestQgsLayoutHtml::evalExpressions()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemHtml *htmlItem = new QgsLayoutItemHtml( &l );
  QgsLayoutFrame *htmlFrame = new QgsLayoutFrame( &l, htmlItem );
  htmlFrame->attemptSetSceneRect( QRectF( 0, 0, 100, 200 ) );
  htmlFrame->setFrameEnabled( true );
  htmlItem->addFrame( htmlFrame );
  htmlItem->setContentMode( QgsLayoutItemHtml::ManualHtml );
  htmlItem->setEvaluateExpressions( true );
  htmlItem->setHtml( u"<body style=\"margin: 10px;\"><div style=\"width: [% 10 * 10 %]px; height: [% 30 + 20 %]px; background-color: [% 'yel' || 'low' %];\"></div></body>"_s );

  htmlItem->loadHtml();

  QGSVERIFYLAYOUTCHECK( u"composerhtml_expressions_enabled"_s, &l );
}

void TestQgsLayoutHtml::evalExpressionsOff()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemHtml *htmlItem = new QgsLayoutItemHtml( &l );
  QgsLayoutFrame *htmlFrame = new QgsLayoutFrame( &l, htmlItem );
  htmlFrame->attemptSetSceneRect( QRectF( 0, 0, 100, 200 ) );
  htmlFrame->setFrameEnabled( true );
  htmlItem->addFrame( htmlFrame );
  htmlItem->setContentMode( QgsLayoutItemHtml::ManualHtml );
  htmlItem->setEvaluateExpressions( false );
  htmlItem->setHtml( u"<body style=\"margin: 10px;\"><div style=\"width: [% 10 * 10 %]px; height: [% 30 + 20 %]px; background-color: [% 'yel' || 'low' %];\"></div></body>"_s );
  htmlItem->loadHtml();

  QGSVERIFYLAYOUTCHECK( u"composerhtml_expressions_disabled"_s, &l );
}

void TestQgsLayoutHtml::table()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemHtml *htmlItem = new QgsLayoutItemHtml( &l );
  QgsLayoutFrame *htmlFrame = new QgsLayoutFrame( &l, htmlItem );
  htmlFrame->attemptSetSceneRect( QRectF( 0, 0, 100, 200 ) );
  htmlFrame->setFrameEnabled( true );
  htmlItem->addFrame( htmlFrame );
  htmlItem->setUrl( QUrl( u"file:///%1/test_html.html"_s.arg( TEST_DATA_DIR ) ) );

  QGSVERIFYLAYOUTCHECK( u"composerhtml_table"_s, &l );
}

void TestQgsLayoutHtml::tableMultiFrame()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemHtml *htmlItem = new QgsLayoutItemHtml( &l );
  QgsLayoutFrame *htmlFrame = new QgsLayoutFrame( &l, htmlItem );
  htmlFrame->attemptSetSceneRect( QRectF( 10, 10, 100, 50 ) );
  htmlItem->addFrame( htmlFrame );
  htmlItem->setResizeMode( QgsLayoutMultiFrame::RepeatUntilFinished );
  htmlItem->setUseSmartBreaks( false );

  //page1
  htmlItem->setUrl( QUrl( u"file:///%1/test_html.html"_s.arg( TEST_DATA_DIR ) ) );
  htmlItem->frame( 0 )->setFrameEnabled( true );
  QGSVERIFYLAYOUTCHECK( u"composerhtml_multiframe1"_s, &l );

  //page2
  QGSVERIFYLAYOUTCHECK( u"composerhtml_multiframe2"_s, &l, 1 );
}

void TestQgsLayoutHtml::htmlMultiFrameSmartBreak()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemHtml *htmlItem = new QgsLayoutItemHtml( &l );
  QgsLayoutFrame *htmlFrame = new QgsLayoutFrame( &l, htmlItem );
  htmlFrame->attemptSetSceneRect( QRectF( 10, 10, 100, 52 ) );
  htmlItem->addFrame( htmlFrame );
  htmlItem->setResizeMode( QgsLayoutMultiFrame::RepeatUntilFinished );
  htmlItem->setUseSmartBreaks( true );

  //page1
  htmlItem->setUrl( QUrl( u"file:///%1/test_html.html"_s.arg( TEST_DATA_DIR ) ) );
  htmlItem->frame( 0 )->setFrameEnabled( true );
  QGSVERIFYLAYOUTCHECK( u"composerhtml_smartbreaks1"_s, &l, 0, 200 );

  //page2
  QGSVERIFYLAYOUTCHECK( u"composerhtml_smartbreaks2"_s, &l, 1, 200 );
}

void TestQgsLayoutHtml::javascriptSetFeature()
{
  //test that JavaScript setFeature() function is correctly called

  // first need to setup some layers with a relation

  //parent layer
  QgsVectorLayer *parentLayer = new QgsVectorLayer( u"Point?field=fldtxt:string&field=fldint:integer&field=foreignkey:integer"_s, u"parent"_s, u"memory"_s );
  QgsVectorDataProvider *pr = parentLayer->dataProvider();
  QgsFeature pf1;
  pf1.setFields( parentLayer->fields() );
  pf1.setAttributes( QgsAttributes() << "test1" << 67 << 123 );
  QgsFeature pf2;
  pf2.setFields( parentLayer->fields() );
  pf2.setAttributes( QgsAttributes() << "test2" << 68 << 124 );
  QVERIFY( pr->addFeatures( QgsFeatureList() << pf1 << pf2 ) );

  // child layer
  QgsVectorLayer *childLayer = new QgsVectorLayer( u"Point?field=x:string&field=y:integer&field=z:integer"_s, u"referencedlayer"_s, u"memory"_s );
  pr = childLayer->dataProvider();
  QgsFeature f1;
  f1.setFields( childLayer->fields() );
  f1.setAttributes( QgsAttributes() << "foo" << 123 << 321 );
  QgsFeature f2;
  f2.setFields( childLayer->fields() );
  f2.setAttributes( QgsAttributes() << "bar" << 123 << 654 );
  QgsFeature f3;
  f3.setFields( childLayer->fields() );
  f3.setAttributes( QgsAttributes() << "foobar" << 124 << 554 );
  QVERIFY( pr->addFeatures( QgsFeatureList() << f1 << f2 << f3 ) );

  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << childLayer << parentLayer );

  //atlas
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  l.reportContext().setLayer( parentLayer );

  QgsRelation rel;
  rel.setId( u"rel1"_s );
  rel.setName( u"relation one"_s );
  rel.setReferencingLayer( childLayer->id() );
  rel.setReferencedLayer( parentLayer->id() );
  rel.addFieldPair( u"y"_s, u"foreignkey"_s );
  QgsProject::instance()->relationManager()->addRelation( rel );

  QgsLayoutItemHtml *htmlItem = new QgsLayoutItemHtml( &l );
  QgsLayoutFrame *htmlFrame = new QgsLayoutFrame( &l, htmlItem );
  htmlFrame->attemptSetSceneRect( QRectF( 0, 0, 100, 200 ) );

  htmlFrame->setFrameEnabled( true );
  htmlItem->addFrame( htmlFrame );
  htmlItem->setContentMode( QgsLayoutItemHtml::ManualHtml );
  htmlItem->setEvaluateExpressions( true );
  // hopefully arial bold 40px is big enough to avoid cross-platform rendering issues
  htmlItem->setHtml( QString( "<body style=\"margin: 10px; font-family: Arial; font-weight: bold; font-size: 40px;\">"
                              "<div id=\"dest\"></div><script>setFeature=function(feature){"
                              "document.getElementById('dest').innerHTML = feature.properties.foreignkey + ',' +"
                              "  feature.properties['relation one'][0].z + ',' + feature.properties['relation one'][1].z;}"
                              "</script></body>" ) );

  QgsFeature f;
  QgsFeatureIterator it = parentLayer->getFeatures();
  it.nextFeature( f );
  l.reportContext().setFeature( f );

  htmlItem->loadHtml();

  QGSVERIFYLAYOUTCHECK( u"composerhtml_setfeature"_s, &l );

  QgsProject::instance()->removeMapLayers( QList<QgsMapLayer *>() << childLayer << parentLayer );
}


QGSTEST_MAIN( TestQgsLayoutHtml )
#include "testqgslayouthtml.moc"
