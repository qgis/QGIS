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


#include "qgslayoutitemlabel.h"
#include "qgslayoutitemshape.h"

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

  private:

    void exportLayout( QgsLayout *layout, const QString testName );

    QString mReport;

};

void TestQgsCompositionConverter::initTestCase()
{
  mReport = QStringLiteral( "<h1>Layout Tests</h1>\n" );
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
  QString templatePath( QStringLiteral( TEST_DATA_DIR ) + "/layouts/2x_template_label.qpt" );
  QDomDocument doc( "mydocument" );
  QFile file( templatePath );
  QVERIFY( file.open( QIODevice::ReadOnly ) );
  if ( !doc.setContent( &file ) )
  {
    file.close();
    return;
  }
  file.close();

  QDomNodeList nodes( doc.elementsByTagName( QStringLiteral( "Composition" ) ) );
  QVERIFY( nodes.length() > 0 );
  QDomElement docElem = nodes.at( 0 ).toElement();

  QgsReadWriteContext context;
  QgsLayout *layout = QgsCompositionConverter::createLayoutFromCompositionXml( docElem, context );

  QVERIFY( layout );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );

  QList<QgsLayoutItemLabel *> items;
  layout->layoutItems<QgsLayoutItemLabel>( items );
  QVERIFY( items.size() > 0 );

  exportLayout( layout, QStringLiteral( "ComposerTemplateLabel" ) );

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

  /*
  QgsCompositionChecker checker( QStringLiteral( "ComposerTemplateLabel" ), composition );
  checker.setSize( QSize( 774, 641 ) );
  checker.setControlPathPrefix( QStringLiteral( "compositionconverter" ) );
  QVERIFY( checker.testComposition( mReport ) );
  */

  qDeleteAll( items );
}

void TestQgsCompositionConverter::importComposerTemplateShape()
{
  QString templatePath( QStringLiteral( TEST_DATA_DIR ) + "/layouts/2x_template_shape.qpt" );
  QDomDocument doc( "mydocument" );
  QFile file( templatePath );
  QVERIFY( file.open( QIODevice::ReadOnly ) );
  if ( !doc.setContent( &file ) )
  {
    file.close();
    return;
  }
  file.close();

  QDomNodeList nodes( doc.elementsByTagName( QStringLiteral( "Composition" ) ) );
  QVERIFY( nodes.length() > 0 );
  QDomElement docElem = nodes.at( 0 ).toElement();

  QgsReadWriteContext context;
  QgsLayout *layout = QgsCompositionConverter::createLayoutFromCompositionXml( docElem, context );

  QVERIFY( layout );
  QCOMPARE( layout->pageCollection()->pageCount(), 1 );

  QList<QgsLayoutItemShape *> items;
  layout->layoutItems<QgsLayoutItemShape>( items );
  QVERIFY( items.size() > 0 );

  exportLayout( layout, QString( "ComposerTemplateShape" ) );

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

  qDeleteAll( items );
}

void TestQgsCompositionConverter::importComposerTemplate()
{
  QString templatePath( QStringLiteral( TEST_DATA_DIR ) + "/layouts/2x_template.qpt" );
  QDomDocument doc( "mydocument" );
  QFile file( templatePath );
  QVERIFY( file.open( QIODevice::ReadOnly ) );
  if ( !doc.setContent( &file ) )
  {
    file.close();
    return;
  }
  file.close();

  QDomNodeList nodes( doc.elementsByTagName( QStringLiteral( "Composition" ) ) );
  QVERIFY( nodes.length() > 0 );
  QDomElement docElem = nodes.at( 0 ).toElement();

  QgsReadWriteContext context;
  QgsLayout *layout = QgsCompositionConverter::createLayoutFromCompositionXml( docElem, context );

  QVERIFY( layout );
  QCOMPARE( layout->pageCollection()->pageCount(), 2 );


  exportLayout( layout,  QString( "ComposerTemplate" ) );
  delete layout;
}

void TestQgsCompositionConverter::exportLayout( QgsLayout *layout, const QString testName )
{
  // Save the template for inspection
  QTemporaryFile tmpTemplate( QString( "%1_converted-XXXXXX.qpt" ).arg( testName ) );
  tmpTemplate.setAutoRemove( false );
  tmpTemplate.open();
  tmpTemplate.close();
  QgsReadWriteContext context;
  layout->saveAsTemplate( tmpTemplate.fileName(), context );
  qDebug() << tmpTemplate.fileName();

  for ( int i = 0; i < layout->pageCollection()->pageCount(); ++i )
  {
    QgsLayoutItemPage *page = layout->pageCollection()->pages().at( i );
    QSize size;
    QgsLayoutSize pageSize = page->sizeWithUnits();
    size.setHeight( pageSize.height() * 3.77 );
    size.setWidth( pageSize.width() * 3.77 );

    QImage outputImage( size, QImage::Format_RGB32 );
    outputImage.setDotsPerMeterX( 96 / 25.4 * 1000 );
    outputImage.setDotsPerMeterY( 96 / 25.4 * 1000 );
    QPainter p( &outputImage );
    layout->exporter().renderPage( &p, i );
    p.end();

    QString renderedFilePath = tmpTemplate.fileName() + QString( "_%1_.png" ).arg( i );
    outputImage.save( renderedFilePath, "PNG" );
  }
}


QGSTEST_MAIN( TestQgsCompositionConverter )
#include "testqgscompositionconverter.moc"
