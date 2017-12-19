/***************************************************************************
                         TestQgsCompositionConverter.cpp
                         -----------------
    begin                : December 2017
    copyright            : (C) 2017 by Alessandro Pasotti
    email                : elpaso at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDebug>

#include "qgstest.h"
#include "qgslayout.h"
#include "qgscompositionconverter.h"
#include "qgsproject.h"
#include "qgsreadwritecontext.h"
#include "qgslayoutexporter.h"
#include "qgsmultirenderchecker.h"
#include "qgssettings.h"


#include "qgslayoutpagecollection.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutitemshape.h"
#include "qgslayoutitempicture.h"
#include "qgslayoutitempolygon.h"
#include "qgslayoutitempolyline.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitemscalebar.h"
#include "qgslayoutitemlegend.h"


class TestQgsCompositionConverter: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    /**
     * Test import label from a composer template
     */
    void importComposerTemplateLabel();

    /**
     * Test import shape from a composer template
     */
    void importComposerTemplateShape();

    /**
     * Test import multiple ements from a composer template
     */
    void importComposerTemplate();

    /**
     * Test import pictures from a composer template
     */
    void importComposerTemplatePicture();

    /**
     * Test import polygon from a composer template
     */
    void importComposerTemplatePolygon();

    /**
     * Test import polyline from a composer template
     */
    void importComposerTemplatePolyline();

    /**
     * Test import arrow from a composer template
     */
    void importComposerTemplateArrow();

    /**
     * Test import map from a composer template
     */
    void importComposerTemplateMap();

    /**
     * Test import legend from a composer template
     */
    void importComposerTemplateLegend();

    /**
     * Test import scalebar from a composer template
     */
    void importComposerTemplateScaleBar();


  private:

    void checkRenderedImage( QgsLayout *layout, const QString testName, const int pageNumber = 0 );

    QDomElement loadComposition( const QString name );

    QString mReport;

};

void TestQgsCompositionConverter::initTestCase()
{
  mReport = QStringLiteral( "<h1>Layout Tests</h1>\n" );
  QgsSettings settings;
  settings.setValue( QStringLiteral( "svg/searchPathsForSVG" ), QStringLiteral( TEST_DATA_DIR ) ) ;
}

void TestQgsCompositionConverter::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsCompositionConverter::init()
{

}

void TestQgsCompositionConverter::cleanup()
{

}


void TestQgsCompositionConverter::importComposerTemplateLabel()
{
  QDomElement docElem( loadComposition( "2x_template_label.qpt" ) );
  QgsReadWriteContext context;
  QgsProject project;
  QgsLayout *layout = QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project );

  QVERIFY( layout );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );

  QList<QgsLayoutItemLabel *> items;
  layout->layoutItems<QgsLayoutItemLabel>( items );
  QVERIFY( items.size() > 0 );

  //exportLayout( layout, QTest::currentTestFunction() );

  // Check the label
  const QgsLayoutItemLabel *label = items.at( 0 );
  QVERIFY( label );
  QCOMPARE( label->text(), QStringLiteral( "QGIS" ) );
  QCOMPARE( label->pos().x(), 55.5333 );
  QCOMPARE( label->pos().y(), 35.3929 );
  QCOMPARE( label->sizeWithUnits().width(), 10.875 );
  QCOMPARE( label->sizeWithUnits().height(), 6.0 );
  QCOMPARE( label->referencePoint(), QgsLayoutItem::ReferencePoint::LowerRight );
  QCOMPARE( label->frameStrokeColor(), QColor( 251, 0, 0, 255 ) );
  QCOMPARE( label->frameStrokeWidth().length(), 0.2 );
  QCOMPARE( ( int )label->rotation(), 4 );

  checkRenderedImage( layout, QTest::currentTestFunction(), 0 );

  qDeleteAll( items );
}

void TestQgsCompositionConverter::importComposerTemplateShape()
{
  QDomElement docElem( loadComposition( "2x_template_shape.qpt" ) );
  QgsReadWriteContext context;
  QgsProject project;
  QgsLayout *layout = QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project );

  QVERIFY( layout );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );

  QList<QgsLayoutItemShape *> items;
  layout->layoutItems<QgsLayoutItemShape>( items );
  QVERIFY( items.size() > 0 );

  //exportLayout( layout, QTest::currentTestFunction() );

  // Check the shape
  const QgsLayoutItemShape *shape = items.at( 0 );
  QCOMPARE( shape->pos().x(), 261.132 );
  QCOMPARE( shape->pos().y(), 83.1791 );
  QCOMPARE( shape->sizeWithUnits().width(), 12.0988 );
  QCOMPARE( shape->sizeWithUnits().height(), 33.2716 );
  QCOMPARE( shape->referencePoint(), QgsLayoutItem::ReferencePoint::MiddleRight );
  QCOMPARE( shape->frameStrokeColor(), QColor( 0, 0, 0, 255 ) );
  QCOMPARE( shape->frameStrokeWidth().length(), 0.3 );
  QCOMPARE( shape->backgroundColor(), QColor( 255, 255, 255, 255 ) );
  QCOMPARE( ( int )shape->rotation(), 0 );
  QCOMPARE( shape->hasFrame(), false );
  QCOMPARE( shape->hasBackground(), false );

  checkRenderedImage( layout, QTest::currentTestFunction(), 0 );

  qDeleteAll( items );
}

void TestQgsCompositionConverter::importComposerTemplatePicture()
{
  QDomElement docElem( loadComposition( "2x_template_pictures.qpt" ) );
  QVERIFY( !docElem.isNull() );
  QgsProject project;
  QgsLayout *layout = QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project );
  QVERIFY( layout );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );

  QList<QgsLayoutItemPicture *> items;
  layout->layoutItems<QgsLayoutItemPicture>( items );
  QCOMPARE( items.size(), 1 );
  QVERIFY( QFile( items.at( 0 )->picturePath() ).exists() );

  QgsLayoutItemPicture *item = items.at( 0 );
  QCOMPARE( item->mPictureHeight, 18.1796 );
  QCOMPARE( item->mPictureWidth, 18.1796 );
  QCOMPARE( item->sizeWithUnits().width(), 25.7099 );
  QCOMPARE( item->sizeWithUnits().height(), 30.7511 );
  QCOMPARE( item->pos().x(), 207.192 );
  QCOMPARE( item->pos().y(), 12.6029 );
  QVERIFY( item->isVisible() );

  checkRenderedImage( layout, QTest::currentTestFunction(), 0 );

  qDeleteAll( items );

}

void TestQgsCompositionConverter::importComposerTemplatePolygon()
{
  QDomElement docElem( loadComposition( "2x_template_polygon.qpt" ) );
  QVERIFY( !docElem.isNull() );
  QgsProject project;
  QgsLayout *layout = QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project );
  QVERIFY( layout );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );

  QList<QgsLayoutItemPolygon *> items;
  layout->layoutItems<QgsLayoutItemPolygon>( items );
  QCOMPARE( items.size(), 1 );

  QgsLayoutItemPolygon *item = items.at( 0 );
  QVERIFY( item->isVisible() );
  QCOMPARE( item->nodes().count(), 7 );

  checkRenderedImage( layout, QTest::currentTestFunction(), 0 );

  qDeleteAll( items );

}

void TestQgsCompositionConverter::importComposerTemplatePolyline()
{
  QDomElement docElem( loadComposition( "2x_template_polyline.qpt" ) );
  QVERIFY( !docElem.isNull() );
  QgsProject project;
  QgsLayout *layout = QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project );
  QVERIFY( layout );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );

  QList<QgsLayoutItemPolyline *> items;
  layout->layoutItems<QgsLayoutItemPolyline>( items );
  QCOMPARE( items.size(), 1 );

  QgsLayoutItemPolyline *item = items.at( 0 );
  QVERIFY( item->isVisible() );
  QCOMPARE( item->nodes().count(), 4 );
  QCOMPARE( item->startMarker(), QgsLayoutItemPolyline::MarkerMode::NoMarker );
  QCOMPARE( item->endMarker(), QgsLayoutItemPolyline::MarkerMode::NoMarker );
  //QCOMPARE( item->nodes().at(0), QPointF( 266.622, 371.215) );
  //QCOMPARE( item->nodes().at(1), QPointF( 261.581, 296.606) );

  checkRenderedImage( layout, QTest::currentTestFunction(), 0 );

  qDeleteAll( items );

}

void TestQgsCompositionConverter::importComposerTemplateArrow()
{
  QDomElement docElem( loadComposition( "2x_template_arrow.qpt" ) );
  QVERIFY( !docElem.isNull() );
  QgsProject project;
  QgsLayout *layout = QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project );
  QVERIFY( layout );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );

  QList<QgsLayoutItemPolyline *> items;
  layout->layoutItems<QgsLayoutItemPolyline>( items );
  QCOMPARE( items.size(), 1 );

  QgsLayoutItemPolyline *item = items.at( 0 );
  QVERIFY( item->isVisible() );
  QCOMPARE( item->nodes().count(), 2 );
  QCOMPARE( item->startMarker(), QgsLayoutItemPolyline::MarkerMode::NoMarker );
  QCOMPARE( item->endMarker(), QgsLayoutItemPolyline::MarkerMode::ArrowHead );

  checkRenderedImage( layout, QTest::currentTestFunction(), 0 );

  qDeleteAll( items );

}


void TestQgsCompositionConverter::importComposerTemplateMap()
{
  QDomElement docElem( loadComposition( "2x_template_map_overview.qpt" ) );
  QVERIFY( !docElem.isNull() );
  QgsProject project;
  QgsLayout *layout = QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project );
  QVERIFY( layout );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );

  QList<QgsLayoutItemMap *> items;
  layout->layoutItems<QgsLayoutItemMap>( items );
  QCOMPARE( items.size(), 2 );

  QgsLayoutItemMap *item = items.at( 0 );
  QVERIFY( item->isVisible() );

  checkRenderedImage( layout, QTest::currentTestFunction(), 0 );

  qDeleteAll( items );

}

void TestQgsCompositionConverter::importComposerTemplateLegend()
{
  QDomElement docElem( loadComposition( "2x_template_legend.qpt" ) );
  QVERIFY( !docElem.isNull() );
  QgsProject project;
  QgsLayout *layout = QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project );
  QVERIFY( layout );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );

  QList<QgsLayoutItemLegend *> items;
  layout->layoutItems<QgsLayoutItemLegend>( items );
  QCOMPARE( items.size(), 1 );

  QgsLayoutItemLegend *item = items.at( 0 );
  QVERIFY( item->isVisible() );

  checkRenderedImage( layout, QTest::currentTestFunction(), 0 );

  qDeleteAll( items );

}

void TestQgsCompositionConverter::importComposerTemplateScaleBar()
{
  QDomElement docElem( loadComposition( "2x_template_scalebar.qpt" ) );
  QVERIFY( !docElem.isNull() );
  QgsProject project;
  QgsLayout *layout = QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project );
  QVERIFY( layout );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );

  QList<QgsLayoutItemScaleBar *> items;
  layout->layoutItems<QgsLayoutItemScaleBar>( items );
  QCOMPARE( items.size(), 1 );

  QgsLayoutItemScaleBar *item = items.at( 0 );
  QVERIFY( item->isVisible() );

  checkRenderedImage( layout, QTest::currentTestFunction(), 0 );

  qDeleteAll( items );

}

void TestQgsCompositionConverter::importComposerTemplate()
{
  QDomElement docElem( loadComposition( "2x_template.qpt" ) );
  QgsProject project;
  QgsLayout *layout = QgsCompositionConverter::createLayoutFromCompositionXml( docElem, &project );

  QVERIFY( layout );
  QCOMPARE( layout->pageCollection()->pageCount(), 2 );

  checkRenderedImage( layout, QTest::currentTestFunction(), 0 );
  checkRenderedImage( layout, QTest::currentTestFunction(), 1 );

  delete layout;
}

void TestQgsCompositionConverter::checkRenderedImage( QgsLayout *layout, const QString testName, const int pageNumber )
{
  QgsLayoutChecker checker( testName + "_" + QString::number( pageNumber ), layout );
  QSize size( layout->pageCollection()->page( pageNumber )->sizeWithUnits().width() * 3.77, layout->pageCollection()->page( pageNumber )->sizeWithUnits().height() * 3.77 );
  checker.setSize( size );
  checker.setControlPathPrefix( QStringLiteral( "compositionconverter" ) );
  QVERIFY( checker.testLayout( mReport, pageNumber ) );
}


QDomElement TestQgsCompositionConverter::loadComposition( const QString name )
{
  QString templatePath( QStringLiteral( TEST_DATA_DIR ) + "/layouts/" + name );
  QDomDocument doc( "mydocument" );
  QFile file( templatePath );
  file.open( QIODevice::ReadOnly );
  doc.setContent( &file );
  file.close();
  QDomNodeList nodes( doc.elementsByTagName( QStringLiteral( "Composition" ) ) );
  if ( nodes.length() > 0 )
    return nodes.at( 0 ).toElement();
  else
  {
    QDomElement elem;
    return elem;
  }
}


QGSTEST_MAIN( TestQgsCompositionConverter )
#include "testqgscompositionconverter.moc"
