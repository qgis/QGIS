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

    void attributeTableRestoredFromTemplate();


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

QGSTEST_MAIN( TestQgsComposition )
#include "testqgscomposition.moc"
