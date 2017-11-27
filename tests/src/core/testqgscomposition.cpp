/***************************************************************************
                         testqgscomposition.cpp
                         ----------------------
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

#include "qgsapplication.h"
#include "qgscomposition.h"
#include "qgscomposerattributetablev2.h"
#include "qgscomposerlabel.h"
#include "qgscomposershape.h"
#include "qgscomposerarrow.h"
#include "qgscomposerhtml.h"
#include "qgscomposerframe.h"
#include "qgscomposermap.h"
#include "qgsmapsettings.h"
#include "qgsmultirenderchecker.h"
#include "qgsfillsymbollayer.h"
#include "qgsproject.h"
#include "qgscomposerlegend.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgslayertreegroup.h"
#include "qgslayertreelayer.h"
#include "qgslayertree.h"
#include "qgslayoutitemlegend.h"

#include <QObject>
#include "qgstest.h"

class TestQgsComposition : public QObject
{
    Q_OBJECT

  public:
    TestQgsComposition();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void itemsOnPage(); //test fetching matching items on a set page
    void shouldExportPage(); //test the shouldExportPage method
    void pageIsEmpty(); //test the pageIsEmpty method
    void customProperties();
    void writeRetrieveCustomProperties();
    void bounds();
    void resizeToContents();
    void resizeToContentsMargin();
    void resizeToContentsMultiPage();
    void georeference();
    void variablesEdited();
    void itemVariablesFunction();
    void referenceMap();
    void legendRestoredFromTemplate();
    void legendRestoredFromTemplateAutoUpdate();
    void attributeTableRestoredFromTemplate();
    void mapLayersRestoredFromTemplate();
    void mapLayersStyleOverrideRestoredFromTemplate();
    void atlasLayerRestoredFromTemplate();

  private:
    QgsComposition *mComposition = nullptr;
    QString mReport;

};

TestQgsComposition::TestQgsComposition() = default;

void TestQgsComposition::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create composition
  mComposition = new QgsComposition( QgsProject::instance() );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
  mComposition->setNumPages( 3 );

  mReport = QStringLiteral( "<h1>Composition Tests</h1>\n" );

}

void TestQgsComposition::cleanupTestCase()
{
  delete mComposition;

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

void TestQgsComposition::init()
{
}

void TestQgsComposition::cleanup()
{
}

void TestQgsComposition::itemsOnPage()
{
  //add some items to the composition
  QgsComposerLabel *label1 = new QgsComposerLabel( mComposition );
  mComposition->addComposerLabel( label1 );
  label1->setItemPosition( 10, 10, 50, 50, QgsComposerItem::UpperLeft, false, 1 );
  QgsComposerLabel *label2 = new QgsComposerLabel( mComposition );
  mComposition->addComposerLabel( label2 );
  label2->setItemPosition( 10, 10, 50, 50, QgsComposerItem::UpperLeft, false, 1 );
  QgsComposerLabel *label3 = new QgsComposerLabel( mComposition );
  mComposition->addComposerLabel( label3 );
  label3->setItemPosition( 10, 10, 50, 50, QgsComposerItem::UpperLeft, false, 2 );
  QgsComposerShape *shape1 = new QgsComposerShape( mComposition );
  mComposition->addComposerShape( shape1 );
  shape1->setItemPosition( 10, 10, 50, 50, QgsComposerItem::UpperLeft, false, 1 );
  QgsComposerShape *shape2 = new QgsComposerShape( mComposition );
  mComposition->addComposerShape( shape2 );
  shape2->setItemPosition( 10, 10, 50, 50, QgsComposerItem::UpperLeft, false, 2 );
  QgsComposerArrow *arrow1 = new QgsComposerArrow( mComposition );
  mComposition->addComposerArrow( arrow1 );
  arrow1->setItemPosition( 10, 10, 50, 50, QgsComposerItem::UpperLeft, false, 3 );
  QgsComposerArrow *arrow2 = new QgsComposerArrow( mComposition );
  mComposition->addComposerArrow( arrow2 );
  arrow2->setItemPosition( 10, 10, 50, 50, QgsComposerItem::UpperLeft, false, 3 );

  //fetch items - remember that these numbers include the paper item!
  QList<QgsComposerItem *> items;
  mComposition->composerItemsOnPage( items, 0 );
  //should be 4 items on page 1
  QCOMPARE( items.length(), 4 );
  mComposition->composerItemsOnPage( items, 1 );
  //should be 3 items on page 2
  QCOMPARE( items.length(), 3 );
  mComposition->composerItemsOnPage( items, 2 );
  //should be 3 items on page 3
  QCOMPARE( items.length(), 3 );

  //check fetching specific item types
  QList<QgsComposerLabel *> labels;
  mComposition->composerItemsOnPage( labels, 0 );
  //should be 2 labels on page 1
  QCOMPARE( labels.length(), 2 );
  mComposition->composerItemsOnPage( labels, 1 );
  //should be 1 label on page 2
  QCOMPARE( labels.length(), 1 );
  mComposition->composerItemsOnPage( labels, 2 );
  //should be no label on page 3
  QCOMPARE( labels.length(), 0 );

  QList<QgsComposerShape *> shapes;
  mComposition->composerItemsOnPage( shapes, 0 );
  //should be 1 shapes on page 1
  QCOMPARE( shapes.length(), 1 );
  mComposition->composerItemsOnPage( shapes, 1 );
  //should be 1 shapes on page 2
  QCOMPARE( shapes.length(), 1 );
  mComposition->composerItemsOnPage( shapes, 2 );
  //should be no shapes on page 3
  QCOMPARE( shapes.length(), 0 );

  QList<QgsComposerArrow *> arrows;
  mComposition->composerItemsOnPage( arrows, 0 );
  //should be no arrows on page 1
  QCOMPARE( arrows.length(), 0 );
  mComposition->composerItemsOnPage( arrows, 1 );
  //should be no arrows on page 2
  QCOMPARE( arrows.length(), 0 );
  mComposition->composerItemsOnPage( arrows, 2 );
  //should be 2 arrows on page 3
  QCOMPARE( arrows.length(), 2 );

  mComposition->removeComposerItem( label1 );
  mComposition->removeComposerItem( label2 );
  mComposition->removeComposerItem( label3 );
  mComposition->removeComposerItem( shape1 );
  mComposition->removeComposerItem( shape2 );
  mComposition->removeComposerItem( arrow1 );
  mComposition->removeComposerItem( arrow2 );

  //check again with removed items
  mComposition->composerItemsOnPage( labels, 0 );
  QCOMPARE( labels.length(), 0 );
  mComposition->composerItemsOnPage( labels, 1 );
  QCOMPARE( labels.length(), 0 );
  mComposition->composerItemsOnPage( labels, 2 );
  QCOMPARE( labels.length(), 0 );
}

void TestQgsComposition::shouldExportPage()
{
  mComposition->setPaperSize( 297, 200 );
  mComposition->setNumPages( 2 );

  QgsComposerHtml *htmlItem = new QgsComposerHtml( mComposition, false );
  //frame on page 1
  QgsComposerFrame *frame1 = new QgsComposerFrame( mComposition, htmlItem, 0, 0, 100, 100 );
  //frame on page 2
  QgsComposerFrame *frame2 = new QgsComposerFrame( mComposition, htmlItem, 0, 320, 100, 100 );
  frame2->setHidePageIfEmpty( true );
  htmlItem->addFrame( frame1 );
  htmlItem->addFrame( frame2 );
  htmlItem->setContentMode( QgsComposerHtml::ManualHtml );
  //short content, so frame 2 should be empty
  htmlItem->setHtml( QStringLiteral( "<p><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( mComposition->shouldExportPage( 1 ), true );
  QCOMPARE( mComposition->shouldExportPage( 2 ), false );

  //long content, so frame 2 should not be empty
  htmlItem->setHtml( QStringLiteral( "<p style=\"height: 10000px\"><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( mComposition->shouldExportPage( 1 ), true );
  QCOMPARE( mComposition->shouldExportPage( 2 ), true );

  //...and back again...
  htmlItem->setHtml( QStringLiteral( "<p><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( mComposition->shouldExportPage( 1 ), true );
  QCOMPARE( mComposition->shouldExportPage( 2 ), false );

  mComposition->removeMultiFrame( htmlItem );
  delete htmlItem;
}

void TestQgsComposition::pageIsEmpty()
{
  //add some items to the composition
  QgsComposerLabel *label1 = new QgsComposerLabel( mComposition );
  mComposition->addComposerLabel( label1 );
  label1->setItemPosition( 10, 10, 50, 50, QgsComposerItem::UpperLeft, false, 1 );
  QgsComposerLabel *label2 = new QgsComposerLabel( mComposition );
  mComposition->addComposerLabel( label2 );
  label2->setItemPosition( 10, 10, 50, 50, QgsComposerItem::UpperLeft, false, 1 );
  QgsComposerLabel *label3 = new QgsComposerLabel( mComposition );
  mComposition->addComposerLabel( label3 );
  label3->setItemPosition( 10, 10, 50, 50, QgsComposerItem::UpperLeft, false, 3 );

  //only page 2 should be empty
  QCOMPARE( mComposition->pageIsEmpty( 1 ), false );
  QCOMPARE( mComposition->pageIsEmpty( 2 ), true );
  QCOMPARE( mComposition->pageIsEmpty( 3 ), false );

  //remove the items
  mComposition->removeComposerItem( label1 );
  mComposition->removeComposerItem( label2 );
  mComposition->removeComposerItem( label3 );

  //expect everything to be empty now
  QCOMPARE( mComposition->pageIsEmpty( 1 ), true );
  QCOMPARE( mComposition->pageIsEmpty( 2 ), true );
  QCOMPARE( mComposition->pageIsEmpty( 3 ), true );
}


void TestQgsComposition::customProperties()
{
  QgsComposition *composition = new QgsComposition( QgsProject::instance() );

  QCOMPARE( composition->customProperty( "noprop", "defaultval" ).toString(), QString( "defaultval" ) );
  QVERIFY( composition->customProperties().isEmpty() );
  composition->setCustomProperty( QStringLiteral( "testprop" ), "testval" );
  QCOMPARE( composition->customProperty( "testprop", "defaultval" ).toString(), QString( "testval" ) );
  QCOMPARE( composition->customProperties().length(), 1 );
  QCOMPARE( composition->customProperties().at( 0 ), QString( "testprop" ) );

  //test no crash
  composition->removeCustomProperty( QStringLiteral( "badprop" ) );

  composition->removeCustomProperty( QStringLiteral( "testprop" ) );
  QVERIFY( composition->customProperties().isEmpty() );
  QCOMPARE( composition->customProperty( "noprop", "defaultval" ).toString(), QString( "defaultval" ) );

  composition->setCustomProperty( QStringLiteral( "testprop1" ), "testval1" );
  composition->setCustomProperty( QStringLiteral( "testprop2" ), "testval2" );
  QStringList keys = composition->customProperties();
  QCOMPARE( keys.length(), 2 );
  QVERIFY( keys.contains( "testprop1" ) );
  QVERIFY( keys.contains( "testprop2" ) );

  delete composition;
}

void TestQgsComposition::writeRetrieveCustomProperties()
{
  QgsComposition *composition = new QgsComposition( QgsProject::instance() );
  composition->setCustomProperty( QStringLiteral( "testprop" ), "testval" );
  composition->setCustomProperty( QStringLiteral( "testprop2" ), 5 );

  //test writing composition with custom properties
  QDomImplementation DomImplementation;
  QDomDocumentType documentType =
    DomImplementation.createDocumentType(
      QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument doc( documentType );
  QDomElement rootNode = doc.createElement( QStringLiteral( "qgis" ) );
  QVERIFY( composition->writeXml( rootNode, doc ) );

  //check if composition node was written
  QDomNodeList evalNodeList = rootNode.elementsByTagName( QStringLiteral( "Composition" ) );
  QCOMPARE( evalNodeList.count(), 1 );
  QDomElement compositionElem = evalNodeList.at( 0 ).toElement();

  //test reading node containing custom properties
  QgsComposition *readComposition = new QgsComposition( QgsProject::instance() );
  QVERIFY( readComposition->readXml( compositionElem, doc ) );

  //test retrieved custom properties
  QCOMPARE( readComposition->customProperties().length(), 2 );
  QVERIFY( readComposition->customProperties().contains( QString( "testprop" ) ) );
  QVERIFY( readComposition->customProperties().contains( QString( "testprop2" ) ) );
  QCOMPARE( readComposition->customProperty( "testprop" ).toString(), QString( "testval" ) );
  QCOMPARE( readComposition->customProperty( "testprop2" ).toInt(), 5 );

  delete composition;
  delete readComposition;
}

void TestQgsComposition::bounds()
{
  //add some items to a composition
  QgsComposition *composition = new QgsComposition( QgsProject::instance() );
  QgsComposerShape *shape1 = new QgsComposerShape( composition );
  shape1->setShapeType( QgsComposerShape::Rectangle );
  composition->addComposerShape( shape1 );
  shape1->setItemPosition( 90, 50, 90, 50, QgsComposerItem::UpperLeft, false, 1 );
  shape1->setItemRotation( 45 );
  QgsComposerShape *shape2 = new QgsComposerShape( composition );
  shape2->setShapeType( QgsComposerShape::Rectangle );
  composition->addComposerShape( shape2 );
  shape2->setItemPosition( 100, 150, 110, 50, QgsComposerItem::UpperLeft, false, 1 );
  QgsComposerShape *shape3 = new QgsComposerShape( composition );
  shape3->setShapeType( QgsComposerShape::Rectangle );
  composition->addComposerShape( shape3 );
  shape3->setItemPosition( 210, 30, 50, 100, QgsComposerItem::UpperLeft, false, 2 );
  QgsComposerShape *shape4 = new QgsComposerShape( composition );
  shape4->setShapeType( QgsComposerShape::Rectangle );
  composition->addComposerShape( shape4 );
  shape4->setItemPosition( 10, 120, 50, 30, QgsComposerItem::UpperLeft, false, 2 );
  shape4->setVisibility( false );

  //check bounds
  QRectF compositionBounds = composition->compositionBounds( false );
  QGSCOMPARENEAR( compositionBounds.height(), 372.15, 0.01 );
  QGSCOMPARENEAR( compositionBounds.width(), 301.00, 0.01 );
  QGSCOMPARENEAR( compositionBounds.left(), -2, 0.01 );
  QGSCOMPARENEAR( compositionBounds.top(), -2, 0.01 );

  QRectF compositionBoundsNoPage = composition->compositionBounds( true );
  QGSCOMPARENEAR( compositionBoundsNoPage.height(), 320.36, 0.01 );
  QGSCOMPARENEAR( compositionBoundsNoPage.width(), 250.30, 0.01 );
  QGSCOMPARENEAR( compositionBoundsNoPage.left(), 9.85, 0.01 );
  QGSCOMPARENEAR( compositionBoundsNoPage.top(), 49.79, 0.01 );

  QRectF page1Bounds = composition->pageItemBounds( 0, true );
  QGSCOMPARENEAR( page1Bounds.height(), 150.36, 0.01 );
  QGSCOMPARENEAR( page1Bounds.width(), 155.72, 0.01 );
  QGSCOMPARENEAR( page1Bounds.left(), 54.43, 0.01 );
  QGSCOMPARENEAR( page1Bounds.top(), 49.79, 0.01 );

  QRectF page2Bounds = composition->pageItemBounds( 1, true );
  QGSCOMPARENEAR( page2Bounds.height(), 100.30, 0.01 );
  QGSCOMPARENEAR( page2Bounds.width(), 50.30, 0.01 );
  QGSCOMPARENEAR( page2Bounds.left(), 209.85, 0.01 );
  QGSCOMPARENEAR( page2Bounds.top(), 249.85, 0.01 );

  QRectF page2BoundsWithHidden = composition->pageItemBounds( 1, false );
  QGSCOMPARENEAR( page2BoundsWithHidden.height(), 120.30, 0.01 );
  QGSCOMPARENEAR( page2BoundsWithHidden.width(), 250.30, 0.01 );
  QGSCOMPARENEAR( page2BoundsWithHidden.left(), 9.85, 0.01 );
  QGSCOMPARENEAR( page2BoundsWithHidden.top(), 249.85, 0.01 );

  delete composition;
}


void TestQgsComposition::resizeToContents()
{
  //add some items to a composition
  QgsComposition *composition = new QgsComposition( QgsProject::instance() );
  QgsSimpleFillSymbolLayer *simpleFill = new QgsSimpleFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( Qt::yellow );
  simpleFill->setStrokeColor( Qt::transparent );
  composition->setPageStyleSymbol( fillSymbol );
  delete fillSymbol;

  QgsComposerShape *shape1 = new QgsComposerShape( composition );
  shape1->setBackgroundColor( QColor::fromRgb( 255, 150, 100 ) );
  shape1->setShapeType( QgsComposerShape::Rectangle );
  composition->addComposerShape( shape1 );
  shape1->setItemPosition( 90, 50, 90, 50, QgsComposerItem::UpperLeft, false, 1 );
  shape1->setItemRotation( 45 );
  QgsComposerShape *shape2 = new QgsComposerShape( composition );
  shape2->setShapeType( QgsComposerShape::Rectangle );
  shape2->setBackgroundColor( QColor::fromRgb( 255, 150, 100 ) );
  composition->addComposerShape( shape2 );
  shape2->setItemPosition( 100, 150, 110, 50, QgsComposerItem::UpperLeft, false, 1 );
  QgsComposerShape *shape3 = new QgsComposerShape( composition );
  shape3->setShapeType( QgsComposerShape::Rectangle );
  shape3->setBackgroundColor( QColor::fromRgb( 255, 150, 100 ) );
  composition->addComposerShape( shape3 );
  shape3->setItemPosition( 210, 30, 50, 100, QgsComposerItem::UpperLeft, false, 1 );

  //resize to contents, no margin
  composition->resizePageToContents();

  QgsCompositionChecker checker( QStringLiteral( "composition_bounds" ), composition );
  checker.setSize( QSize( 774, 641 ) );
  checker.setControlPathPrefix( QStringLiteral( "composition" ) );
  QVERIFY( checker.testComposition( mReport ) );

  delete composition;
}

void TestQgsComposition::resizeToContentsMargin()
{
  //resize to contents, with margin

  QgsComposition *composition = new QgsComposition( QgsProject::instance() );
  QgsSimpleFillSymbolLayer *simpleFill = new QgsSimpleFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( Qt::yellow );
  simpleFill->setStrokeColor( Qt::transparent );
  composition->setPageStyleSymbol( fillSymbol );
  delete fillSymbol;

  QgsComposerShape *shape1 = new QgsComposerShape( composition );
  shape1->setBackgroundColor( QColor::fromRgb( 255, 150, 100 ) );
  shape1->setShapeType( QgsComposerShape::Rectangle );
  composition->addComposerShape( shape1 );
  shape1->setItemPosition( 90, 50, 90, 50, QgsComposerItem::UpperLeft, false, 1 );
  shape1->setItemRotation( 45 );
  QgsComposerShape *shape2 = new QgsComposerShape( composition );
  shape2->setShapeType( QgsComposerShape::Rectangle );
  shape2->setBackgroundColor( QColor::fromRgb( 255, 150, 100 ) );
  composition->addComposerShape( shape2 );
  shape2->setItemPosition( 100, 150, 110, 50, QgsComposerItem::UpperLeft, false, 1 );
  QgsComposerShape *shape3 = new QgsComposerShape( composition );
  shape3->setShapeType( QgsComposerShape::Rectangle );
  shape3->setBackgroundColor( QColor::fromRgb( 255, 150, 100 ) );
  composition->addComposerShape( shape3 );
  shape3->setItemPosition( 210, 30, 50, 100, QgsComposerItem::UpperLeft, false, 1 );

  //resize to contents, with margin
  composition->resizePageToContents( 30, 20, 50, 40 );

  QgsCompositionChecker checker( QStringLiteral( "composition_bounds_margin" ), composition );
  checker.setSize( QSize( 1000, 942 ) );
  checker.setControlPathPrefix( QStringLiteral( "composition" ) );
  QVERIFY( checker.testComposition( mReport ) );

  delete composition;
}

void TestQgsComposition::resizeToContentsMultiPage()
{
  //resize to contents with multi-page composition, should result in a single page

  QgsComposition *composition = new QgsComposition( QgsProject::instance() );
  QgsSimpleFillSymbolLayer *simpleFill = new QgsSimpleFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, simpleFill );
  simpleFill->setColor( Qt::yellow );
  simpleFill->setStrokeColor( Qt::transparent );
  composition->setPageStyleSymbol( fillSymbol );
  delete fillSymbol;

  composition->setNumPages( 3 );

  QgsComposerShape *shape1 = new QgsComposerShape( composition );
  shape1->setBackgroundColor( QColor::fromRgb( 255, 150, 100 ) );
  shape1->setShapeType( QgsComposerShape::Rectangle );
  composition->addComposerShape( shape1 );
  shape1->setItemPosition( 90, 50, 90, 50, QgsComposerItem::UpperLeft, false, 1 );
  shape1->setItemRotation( 45 );
  QgsComposerShape *shape2 = new QgsComposerShape( composition );
  shape2->setShapeType( QgsComposerShape::Rectangle );
  shape2->setBackgroundColor( QColor::fromRgb( 255, 150, 100 ) );
  composition->addComposerShape( shape2 );
  shape2->setItemPosition( 100, 150, 110, 50, QgsComposerItem::UpperLeft, false, 2 );
  QgsComposerShape *shape3 = new QgsComposerShape( composition );
  shape3->setShapeType( QgsComposerShape::Rectangle );
  shape3->setBackgroundColor( QColor::fromRgb( 255, 150, 100 ) );
  composition->addComposerShape( shape3 );
  shape3->setItemPosition( 210, 30, 50, 100, QgsComposerItem::UpperLeft, false, 3 );

  //resize to contents, no margin
  composition->resizePageToContents();

  QCOMPARE( composition->numPages(), 1 );

  QgsCompositionChecker checker( QStringLiteral( "composition_bounds_multipage" ), composition );
  checker.setSize( QSize( 394, 996 ) );
  checker.setControlPathPrefix( QStringLiteral( "composition" ) );
  QVERIFY( checker.testComposition( mReport ) );

  delete composition;
}

void TestQgsComposition::georeference()
{
  QgsRectangle extent( 2000, 2800, 2500, 2900 );
  QgsComposition *composition = new QgsComposition( QgsProject::instance() );

  // no map
  double *t = composition->computeGeoTransform( nullptr );
  QVERIFY( !t );

  QgsComposerMap *map = new QgsComposerMap( composition );
  map->setNewExtent( extent );
  map->setSceneRect( QRectF( 30, 60, 200, 100 ) );
  composition->addComposerMap( map );

  t = composition->computeGeoTransform( map );
  QGSCOMPARENEAR( t[0], 1925.0, 1.0 );
  QGSCOMPARENEAR( t[1], 0.211719, 0.0001 );
  QGSCOMPARENEAR( t[2], 0.0, 4 * DBL_EPSILON );
  QGSCOMPARENEAR( t[3], 3200, 1 );
  QGSCOMPARENEAR( t[4], 0.0, 4 * DBL_EPSILON );
  QGSCOMPARENEAR( t[5], -0.211694, 0.0001 );
  delete[] t;

  // don't specify map
  composition->setReferenceMap( map );
  t = composition->computeGeoTransform();
  QGSCOMPARENEAR( t[0], 1925.0, 1.0 );
  QGSCOMPARENEAR( t[1], 0.211719, 0.0001 );
  QGSCOMPARENEAR( t[2], 0.0, 4 * DBL_EPSILON );
  QGSCOMPARENEAR( t[3], 3200, 1 );
  QGSCOMPARENEAR( t[4], 0.0, 4 * DBL_EPSILON );
  QGSCOMPARENEAR( t[5], -0.211694, 0.0001 );
  delete[] t;

  // specify extent
  t = composition->computeGeoTransform( map, QRectF( 70, 100, 50, 60 ) );
  QGSCOMPARENEAR( t[0], 2100.0, 1.0 );
  QGSCOMPARENEAR( t[1], 0.211864, 0.0001 );
  QGSCOMPARENEAR( t[2], 0.0, 4 * DBL_EPSILON );
  QGSCOMPARENEAR( t[3], 2950, 1 );
  QGSCOMPARENEAR( t[4], 0.0, 4 * DBL_EPSILON );
  QGSCOMPARENEAR( t[5], -0.211864, 0.0001 );
  delete[] t;

  // specify dpi
  t = composition->computeGeoTransform( map, QRectF(), 75 );
  QGSCOMPARENEAR( t[0], 1925.0, 1 );
  QGSCOMPARENEAR( t[1], 0.847603, 0.0001 );
  QGSCOMPARENEAR( t[2], 0.0, 4 * DBL_EPSILON );
  QGSCOMPARENEAR( t[3], 3200.0, 1 );
  QGSCOMPARENEAR( t[4], 0.0, 4 * DBL_EPSILON );
  QGSCOMPARENEAR( t[5], -0.846774, 0.0001 );
  delete[] t;

  // rotation
  map->setMapRotation( 45 );
  t = composition->computeGeoTransform( map );
  QGSCOMPARENEAR( t[0], 1825.7, 1 );
  QGSCOMPARENEAR( t[1], 0.149708, 0.0001 );
  QGSCOMPARENEAR( t[2], 0.149708, 0.0001 );
  QGSCOMPARENEAR( t[3], 2889.64, 1 );
  QGSCOMPARENEAR( t[4], 0.14969, 0.0001 );
  QGSCOMPARENEAR( t[5], -0.14969, 0.0001 );
  delete[] t;

  delete composition;
}

void TestQgsComposition::variablesEdited()
{
  QgsComposition c( QgsProject::instance() );
  QSignalSpy spyVariablesChanged( &c, SIGNAL( variablesChanged() ) );

  c.setCustomProperty( QStringLiteral( "not a variable" ), "1" );
  QVERIFY( spyVariablesChanged.count() == 0 );
  c.setCustomProperty( QStringLiteral( "variableNames" ), "1" );
  QVERIFY( spyVariablesChanged.count() == 1 );
  c.setCustomProperty( QStringLiteral( "variableValues" ), "1" );
  QVERIFY( spyVariablesChanged.count() == 2 );
}

void TestQgsComposition::itemVariablesFunction()
{
  QgsRectangle extent( 2000, 2800, 2500, 2900 );
  QgsComposition *composition = new QgsComposition( QgsProject::instance() );

  QgsExpression e( QStringLiteral( "map_get( item_variables( 'map_id' ), 'map_scale' )" ) );
  // no map
  QgsExpressionContext c = composition->createExpressionContext();
  QVariant r = e.evaluate( &c );
  QVERIFY( !r.isValid() );

  QgsComposerMap *map = new QgsComposerMap( composition );
  map->setNewExtent( extent );
  map->setSceneRect( QRectF( 30, 60, 200, 100 ) );
  map->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  composition->addComposerMap( map );
  map->setId( QStringLiteral( "map_id" ) );

  c = composition->createExpressionContext();
  r = e.evaluate( &c );
  QGSCOMPARENEAR( r.toDouble(), 1.38916e+08, 100 );

  QgsExpression e2( QStringLiteral( "map_get( item_variables( 'map_id' ), 'map_crs' )" ) );
  r = e2.evaluate( &c );
  QCOMPARE( r.toString(), QString( "EPSG:4326" ) );

  QgsExpression e3( QStringLiteral( "map_get( item_variables( 'map_id' ), 'map_crs_definition' )" ) );
  r = e3.evaluate( &c );
  QCOMPARE( r.toString(), QString( "+proj=longlat +datum=WGS84 +no_defs" ) );

  QgsExpression e4( QStringLiteral( "map_get( item_variables( 'map_id' ), 'map_units' )" ) );
  r = e4.evaluate( &c );
  QCOMPARE( r.toString(), QString( "degrees" ) );

  delete composition;
}

void TestQgsComposition::referenceMap()
{
  QgsRectangle extent( 2000, 2800, 2500, 2900 );
  QgsComposition *composition = new QgsComposition( QgsProject::instance() );

  // no maps
  QVERIFY( !composition->referenceMap() );

  QgsComposerMap *map = new QgsComposerMap( composition );
  map->setNewExtent( extent );
  map->setSceneRect( QRectF( 30, 60, 200, 100 ) );
  composition->addComposerMap( map );

  QCOMPARE( composition->referenceMap(), map );

  // add a larger map
  QgsComposerMap *map2 = new QgsComposerMap( composition );
  map2->setNewExtent( extent );
  map2->setSceneRect( QRectF( 30, 60, 250, 150 ) );
  composition->addComposerMap( map2 );

  QCOMPARE( composition->referenceMap(), map2 );

  // explicitly set reference map
  composition->setReferenceMap( map );
  QCOMPARE( composition->referenceMap(), map );

  delete composition;
}

void TestQgsComposition::legendRestoredFromTemplate()
{
  // load a layer

  QFileInfo vectorFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/points.shp" );
  QgsVectorLayer *layer = new QgsVectorLayer( vectorFileInfo.filePath(),
      vectorFileInfo.completeBaseName(),
      QStringLiteral( "ogr" ) );
  QgsProject p;
  p.addMapLayer( layer );

  // create composition
  QgsComposition c( &p );
  // add a legend
  QgsComposerLegend *legend = new QgsComposerLegend( &c );
  c.addComposerLegend( legend );
  legend->setAutoUpdateModel( false );

  QgsLegendModel *model = legend->model();
  QgsLayerTreeNode *node = model->rootGroup()->children().at( 0 );
  // make sure we've got right node
  QgsLayerTreeLayer *layerNode = dynamic_cast< QgsLayerTreeLayer * >( node );
  QVERIFY( layerNode );
  QCOMPARE( layerNode->layer(), layer );

  // got it!
  layerNode->setCustomProperty( QStringLiteral( "legend/title-label" ), QStringLiteral( "new title!" ) );
  // make sure new title stuck
  QCOMPARE( model->data( model->node2index( layerNode ), Qt::DisplayRole ).toString(), QString( "new title!" ) );

  // save composition to template
  QDomDocument doc;
  QDomElement composerElem = doc.createElement( QStringLiteral( "Composer" ) );
  doc.appendChild( composerElem );
  c.writeXml( composerElem, doc );
  c.atlasComposition().writeXml( composerElem, doc );


  // make a new composition from template
  QgsComposition c2( &p );
  QVERIFY( c2.loadFromTemplate( doc ) );
  // get legend from new composition
  QList< QgsComposerLegend * > legends2;
  c2.composerItems( legends2 );
  QgsComposerLegend *legend2 = legends2.at( 0 );
  QVERIFY( legend2 );

  QgsLegendModel *model2 = legend2->model();
  QgsLayerTreeNode *node2 = model2->rootGroup()->children().at( 0 );
  QgsLayerTreeLayer *layerNode2 = dynamic_cast< QgsLayerTreeLayer * >( node2 );
  QVERIFY( layerNode2 );
  QCOMPARE( layerNode2->layer(), layer );
  QCOMPARE( model2->data( model->node2index( layerNode2 ), Qt::DisplayRole ).toString(), QString( "new title!" ) );

  QString oldId = layer->id();
  // new test
  // remove existing layer
  p.removeMapLayer( layer );

  // reload it, with a new id
  QgsVectorLayer *layer2 = new QgsVectorLayer( vectorFileInfo.filePath(),
      vectorFileInfo.completeBaseName(),
      QStringLiteral( "ogr" ) );
  p.addMapLayer( layer2 );
  QVERIFY( oldId != layer2->id() );

  // load composition from template
  QgsComposition c3( &p );
  QVERIFY( c3.loadFromTemplate( doc ) );
  // get legend from new composition
  QList< QgsComposerLegend * > legends3;
  c3.composerItems( legends3 );
  QgsComposerLegend *legend3 = legends3.at( 0 );
  QVERIFY( legend3 );

  //make sure customisation remains intact
  QgsLegendModel *model3 = legend3->model();
  QgsLayerTreeNode *node3 = model3->rootGroup()->children().at( 0 );
  QgsLayerTreeLayer *layerNode3 = dynamic_cast< QgsLayerTreeLayer * >( node3 );
  QVERIFY( layerNode3 );
  QCOMPARE( layerNode3->layer(), layer2 );
  QCOMPARE( model3->data( model->node2index( layerNode3 ), Qt::DisplayRole ).toString(), QString( "new title!" ) );
}

void TestQgsComposition::legendRestoredFromTemplateAutoUpdate()
{
  // load a layer

  QFileInfo vectorFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/points.shp" );
  QgsVectorLayer *layer = new QgsVectorLayer( vectorFileInfo.filePath(),
      vectorFileInfo.completeBaseName(),
      QStringLiteral( "ogr" ) );
  QgsProject p;
  p.addMapLayer( layer );

  // create composition
  QgsComposition c( &p );
  // add a legend
  QgsComposerLegend *legend = new QgsComposerLegend( &c );
  c.addComposerLegend( legend );
  legend->setAutoUpdateModel( true );

  QgsLegendModel *model = legend->model();
  QgsLayerTreeNode *node = model->rootGroup()->children().at( 0 );
  // make sure we've got right node
  QgsLayerTreeLayer *layerNode = dynamic_cast< QgsLayerTreeLayer * >( node );
  QVERIFY( layerNode );
  QCOMPARE( layerNode->layer(), layer );
  QCOMPARE( model->data( model->node2index( layerNode ), Qt::DisplayRole ).toString(), QString( "points" ) );

  // save composition to template
  QDomDocument doc;
  QDomElement composerElem = doc.createElement( QStringLiteral( "Composer" ) );
  doc.appendChild( composerElem );
  c.writeXml( composerElem, doc );
  c.atlasComposition().writeXml( composerElem, doc );

  //new project
  QgsVectorLayer *layer2 = new QgsVectorLayer( vectorFileInfo.filePath(),
      vectorFileInfo.completeBaseName(),
      QStringLiteral( "ogr" ) );
  QgsProject p2;
  p2.addMapLayer( layer2 );

  // make a new composition from template
  QgsComposition c2( &p2 );
  QVERIFY( c2.loadFromTemplate( doc ) );
  // get legend from new composition
  QList< QgsComposerLegend * > legends2;
  c2.composerItems( legends2 );
  QgsComposerLegend *legend2 = legends2.at( 0 );
  QVERIFY( legend2 );

  QgsLegendModel *model2 = legend2->model();
  QgsLayerTreeNode *node2 = model2->rootGroup()->children().at( 0 );
  QgsLayerTreeLayer *layerNode2 = dynamic_cast< QgsLayerTreeLayer * >( node2 );
  QVERIFY( layerNode2 );
  QCOMPARE( layerNode2->layer(), layer2 );
  QCOMPARE( model2->data( model->node2index( layerNode2 ), Qt::DisplayRole ).toString(), QString( "points" ) );
}

void TestQgsComposition::attributeTableRestoredFromTemplate()
{
  // load some layers
  QFileInfo vectorFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/points.shp" );
  QgsVectorLayer *layer = new QgsVectorLayer( vectorFileInfo.filePath(),
      vectorFileInfo.completeBaseName(),
      QStringLiteral( "ogr" ) );
  QgsVectorLayer *layer2 = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "memory" ), QStringLiteral( "memory" ) );
  QgsProject p;
  p.addMapLayer( layer2 );
  p.addMapLayer( layer );

  // create composition
  QgsComposition c( &p );
  // add an attribute table
  QgsComposerAttributeTableV2 *table = new QgsComposerAttributeTableV2( &c, false );
  c.addMultiFrame( table );
  table->setVectorLayer( layer );
  QgsComposerFrame *frame = new QgsComposerFrame( &c, table, 1, 1, 10, 10 );
  c.addComposerTableFrame( table, frame );
  table->addFrame( frame );

  // save composition to template
  QDomDocument doc;
  QDomElement composerElem = doc.createElement( QStringLiteral( "Composer" ) );
  doc.appendChild( composerElem );
  c.writeXml( composerElem, doc );
  c.atlasComposition().writeXml( composerElem, doc );

  // new project
  QgsProject p2;
  QgsVectorLayer *layer3 = new QgsVectorLayer( vectorFileInfo.filePath(),
      vectorFileInfo.completeBaseName(),
      QStringLiteral( "ogr" ) );
  QgsVectorLayer *layer4 = new QgsVectorLayer( QStringLiteral( "Point" ), QStringLiteral( "memory" ), QStringLiteral( "memory" ) );
  p2.addMapLayer( layer4 );
  p2.addMapLayer( layer3 );

  // make a new composition from template
  QgsComposition c2( &p2 );
  QVERIFY( c2.loadFromTemplate( doc ) );
  // get table from new composition
  QList< QgsComposerFrame * > frames2;
  c2.composerItems( frames2 );
  QgsComposerAttributeTableV2 *table2 = static_cast< QgsComposerAttributeTableV2 *>( frames2.at( 0 )->multiFrame() );
  QVERIFY( table2 );

  QCOMPARE( table2->vectorLayer(), layer3 );
}

void TestQgsComposition::mapLayersRestoredFromTemplate()
{
  // load some layers
  QFileInfo vectorFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/points.shp" );
  QgsVectorLayer *layer = new QgsVectorLayer( vectorFileInfo.filePath(),
      vectorFileInfo.completeBaseName(),
      QStringLiteral( "ogr" ) );
  QFileInfo vectorFileInfo2( QStringLiteral( TEST_DATA_DIR ) + "/polys.shp" );
  QgsVectorLayer *layer2 = new QgsVectorLayer( vectorFileInfo2.filePath(),
      vectorFileInfo2.completeBaseName(),
      QStringLiteral( "ogr" ) );
  QFileInfo rasterFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" );
  QgsRasterLayer *rl = new QgsRasterLayer( rasterFileInfo.filePath(),
      rasterFileInfo.completeBaseName() );

  QgsProject p;
  p.addMapLayer( layer2 );
  p.addMapLayer( layer );
  p.addMapLayer( rl );

  // create composition
  QgsComposition c( &p );
  // add a map
  QgsComposerMap *map = new QgsComposerMap( &c, 1, 1, 10, 10 );
  c.addComposerMap( map );
  map->setLayers( QList<QgsMapLayer *>() << layer << layer2 << rl );

  // save composition to template
  QDomDocument doc;
  QDomElement composerElem = doc.createElement( QStringLiteral( "Composer" ) );
  doc.appendChild( composerElem );
  c.writeXml( composerElem, doc );
  c.atlasComposition().writeXml( composerElem, doc );

  // new project
  QgsProject p2;
  QgsVectorLayer *layer3 = new QgsVectorLayer( vectorFileInfo.filePath(),
      vectorFileInfo.completeBaseName(),
      QStringLiteral( "ogr" ) );
  QgsVectorLayer *layer4 = new QgsVectorLayer( vectorFileInfo2.filePath(),
      vectorFileInfo2.completeBaseName(),
      QStringLiteral( "ogr" ) );
  QgsRasterLayer *rl5 = new QgsRasterLayer( rasterFileInfo.filePath(),
      rasterFileInfo.completeBaseName() );
  p2.addMapLayer( layer4 );
  p2.addMapLayer( layer3 );
  p2.addMapLayer( rl5 );

  // make a new composition from template
  QgsComposition c2( &p2 );
  QVERIFY( c2.loadFromTemplate( doc ) );
  // get map from new composition
  QList< QgsComposerMap * > maps;
  c2.composerItems( maps );
  QgsComposerMap *map2 = static_cast< QgsComposerMap *>( maps.at( 0 ) );
  QVERIFY( map2 );

  QCOMPARE( map2->layers(), QList<QgsMapLayer *>() << layer3 << layer4 << rl5 );
}

void TestQgsComposition::mapLayersStyleOverrideRestoredFromTemplate()
{
  // load some layers
  QFileInfo vectorFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/points.shp" );
  QgsVectorLayer *layer = new QgsVectorLayer( vectorFileInfo.filePath(),
      vectorFileInfo.completeBaseName(),
      QStringLiteral( "ogr" ) );
  QFileInfo vectorFileInfo2( QStringLiteral( TEST_DATA_DIR ) + "/polys.shp" );
  QgsVectorLayer *layer2 = new QgsVectorLayer( vectorFileInfo2.filePath(),
      vectorFileInfo2.completeBaseName(),
      QStringLiteral( "ogr" ) );
  QgsProject p;
  p.addMapLayer( layer2 );
  p.addMapLayer( layer );

  // create composition
  QgsComposition c( &p );
  // add a map
  QgsComposerMap *map = new QgsComposerMap( &c, 1, 1, 10, 10 );
  c.addComposerMap( map );
  map->setKeepLayerStyles( true );
  QgsStringMap styles;
  // just close your eyes and pretend these are real styles
  styles.insert( layer->id(), QStringLiteral( "<b>xxxxx</b>" ) );
  styles.insert( layer2->id(), QStringLiteral( "<blink>yyyyy</blink>" ) );
  map->setLayerStyleOverrides( styles );

  // save composition to template
  QDomDocument doc;
  QDomElement composerElem = doc.createElement( QStringLiteral( "Composer" ) );
  doc.appendChild( composerElem );
  c.writeXml( composerElem, doc );
  c.atlasComposition().writeXml( composerElem, doc );

  // new project
  QgsProject p2;
  QgsVectorLayer *layer3 = new QgsVectorLayer( vectorFileInfo.filePath(),
      vectorFileInfo.completeBaseName(),
      QStringLiteral( "ogr" ) );
  QgsVectorLayer *layer4 = new QgsVectorLayer( vectorFileInfo2.filePath(),
      vectorFileInfo2.completeBaseName(),
      QStringLiteral( "ogr" ) );
  p2.addMapLayer( layer4 );
  p2.addMapLayer( layer3 );

  // make a new composition from template
  QgsComposition c2( &p2 );
  QVERIFY( c2.loadFromTemplate( doc ) );
  // get map from new composition
  QList< QgsComposerMap * > maps;
  c2.composerItems( maps );
  QgsComposerMap *map2 = static_cast< QgsComposerMap *>( maps.at( 0 ) );
  QVERIFY( map2 );
  QVERIFY( map2->keepLayerStyles() );

  QgsStringMap restoredStyles = map2->layerStyleOverrides();
  QVERIFY( restoredStyles.contains( layer3->id() ) );
  QCOMPARE( restoredStyles.value( layer3->id() ).trimmed(), QStringLiteral( "<b>xxxxx</b>" ) );
  QVERIFY( restoredStyles.contains( layer4->id() ) );
  QCOMPARE( restoredStyles.value( layer4->id() ).trimmed(), QStringLiteral( "<blink>yyyyy</blink>" ) );
}

void TestQgsComposition::atlasLayerRestoredFromTemplate()
{
  // load some layers
  QFileInfo vectorFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/points.shp" );
  QgsVectorLayer *layer = new QgsVectorLayer( vectorFileInfo.filePath(),
      vectorFileInfo.completeBaseName(),
      QStringLiteral( "ogr" ) );
  QgsProject p;
  p.addMapLayer( layer );

  // create composition
  QgsComposition c( &p );
  // set atlas layer
  c.atlasComposition().setEnabled( true );
  c.atlasComposition().setCoverageLayer( layer );

  // save composition to template
  QDomDocument doc;
  QDomElement composerElem = doc.createElement( QStringLiteral( "Composer" ) );
  doc.appendChild( composerElem );
  c.writeXml( composerElem, doc );
  c.atlasComposition().writeXml( composerElem, doc );

  // new project
  QgsProject p2;
  QgsVectorLayer *layer2 = new QgsVectorLayer( vectorFileInfo.filePath(),
      vectorFileInfo.completeBaseName(),
      QStringLiteral( "ogr" ) );
  p2.addMapLayer( layer2 );

  // make a new composition from template
  QgsComposition c2( &p2 );
  QVERIFY( c2.loadFromTemplate( doc ) );
  // check atlas layer
  QCOMPARE( c2.atlasComposition().coverageLayer(), layer2 );
}

QGSTEST_MAIN( TestQgsComposition )
#include "testqgscomposition.moc"
