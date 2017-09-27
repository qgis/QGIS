/***************************************************************************
                         testqgscomposertable.cpp
                         ----------------------
    begin                : July 2014
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
#include "qgscomposermap.h"
#include "qgscomposertexttable.h"
#include "qgsproject.h"
#include "qgsmapsettings.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsfeature.h"
#include "qgssymbol.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsproperty.h"

#include <QObject>
#include "qgstest.h"

class TestQgsComposerDD : public QObject
{
    Q_OBJECT

  public:
    TestQgsComposerDD() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void ddEvaluate(); //test setting/evaluating data defined value

  private:
    QgsComposition *mComposition = nullptr;
    QgsVectorLayer *mVectorLayer = nullptr;
    QgsComposerMap *mAtlasMap = nullptr;
    QgsAtlasComposition *mAtlas = nullptr;
    QString mReport;
};

void TestQgsComposerDD::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create maplayers from testdata and add to layer registry
  QFileInfo vectorFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/france_parts.shp" );
  mVectorLayer = new QgsVectorLayer( vectorFileInfo.filePath(),
                                     vectorFileInfo.completeBaseName(),
                                     QStringLiteral( "ogr" ) );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  mVectorLayer->setSimplifyMethod( simplifyMethod );

  //create composition with composer map

  // select epsg:2154
  QgsCoordinateReferenceSystem crs;
  crs.createFromSrid( 2154 );
  mComposition = new QgsComposition( QgsProject::instance() );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  // fix the renderer, fill with green
  QgsStringMap props;
  props.insert( QStringLiteral( "color" ), QStringLiteral( "0,127,0" ) );
  QgsFillSymbol *fillSymbol = QgsFillSymbol::createSimple( props );
  QgsSingleSymbolRenderer *renderer = new QgsSingleSymbolRenderer( fillSymbol );
  mVectorLayer->setRenderer( renderer );

  // the atlas map
  mAtlasMap = new QgsComposerMap( mComposition, 20, 20, 130, 130 );
  mAtlasMap->setFrameEnabled( true );
  mComposition->addComposerMap( mAtlasMap );

  mAtlas = &mComposition->atlasComposition();
  mAtlas->setCoverageLayer( mVectorLayer );
  mAtlas->setEnabled( true );
  mComposition->setAtlasMode( QgsComposition::ExportAtlas );

  mReport = QStringLiteral( "<h1>Composer Data Defined Tests</h1>\n" );

}

void TestQgsComposerDD::cleanupTestCase()
{
  delete mComposition;
  delete mVectorLayer;

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

void TestQgsComposerDD::init()
{
}

void TestQgsComposerDD::cleanup()
{
}

void TestQgsComposerDD::ddEvaluate()
{
  //set a data defined property
  mAtlasMap->dataDefinedProperties().setProperty( QgsComposerItem::PositionY, QgsProperty::fromExpression( QStringLiteral( "20+30" ) ) );
  //evaluate property
  mAtlasMap->refreshDataDefinedProperty( QgsComposerItem::PositionY );
  QCOMPARE( mAtlasMap->pos().y(), 50.0 );
  mAtlasMap->dataDefinedProperties().setProperty( QgsComposerItem::PositionY, QgsProperty() );
}

QGSTEST_MAIN( TestQgsComposerDD )
#include "testqgscomposerdd.moc"
