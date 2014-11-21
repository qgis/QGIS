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
#include "qgscomposerattributetable.h"
#include "qgsmaplayerregistry.h"
#include "qgsmapsettings.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsfeature.h"
#include "qgssymbolv2.h"
#include "qgssinglesymbolrendererv2.h"
#include "qgsdatadefined.h"

#include <QObject>
#include <QtTest/QtTest>

class TestQgsComposerDD: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void ddEvaluate(); //test setting/evaluating data defined value

  private:
    QgsComposition* mComposition;
    QgsMapSettings mMapSettings;
    QgsVectorLayer* mVectorLayer;
    QgsComposerMap* mAtlasMap;
    QgsAtlasComposition* mAtlas;
    QString mReport;
};

void TestQgsComposerDD::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create maplayers from testdata and add to layer registry
  QFileInfo vectorFileInfo( QString( TEST_DATA_DIR ) + QDir::separator() +  "france_parts.shp" );
  mVectorLayer = new QgsVectorLayer( vectorFileInfo.filePath(),
                                     vectorFileInfo.completeBaseName(),
                                     "ogr" );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  mVectorLayer->setSimplifyMethod( simplifyMethod );

  QgsMapLayerRegistry::instance()->addMapLayers( QList<QgsMapLayer*>() << mVectorLayer );

  //create composition with composer map
  mMapSettings.setLayers( QStringList() << mVectorLayer->id() );
  mMapSettings.setCrsTransformEnabled( true );
  mMapSettings.setMapUnits( QGis::Meters );

  // select epsg:2154
  QgsCoordinateReferenceSystem crs;
  crs.createFromSrid( 2154 );
  mMapSettings.setDestinationCrs( crs );
  mComposition = new QgsComposition( mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  // fix the renderer, fill with green
  QgsStringMap props;
  props.insert( "color", "0,127,0" );
  QgsFillSymbolV2* fillSymbol = QgsFillSymbolV2::createSimple( props );
  QgsSingleSymbolRendererV2* renderer = new QgsSingleSymbolRendererV2( fillSymbol );
  mVectorLayer->setRendererV2( renderer );

  // the atlas map
  mAtlasMap = new QgsComposerMap( mComposition, 20, 20, 130, 130 );
  mAtlasMap->setFrameEnabled( true );
  mComposition->addComposerMap( mAtlasMap );

  mAtlas = &mComposition->atlasComposition();
  mAtlas->setCoverageLayer( mVectorLayer );
  mAtlas->setEnabled( true );
  mComposition->setAtlasMode( QgsComposition::ExportAtlas );

  mReport = "<h1>Composer Data Defined Tests</h1>\n";

}

void TestQgsComposerDD::cleanupTestCase()
{
  delete mComposition;

  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
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
  mAtlasMap->setDataDefinedProperty( QgsComposerItem::PositionY, true, true, QString( "20+30" ), QString() );
  //evaluate property
  mAtlasMap->refreshDataDefinedProperty( QgsComposerItem::PositionY );
  QCOMPARE( mAtlasMap->pos().y(), 50.0 );
  mAtlasMap->setDataDefinedProperty( QgsComposerItem::PositionY, false, false, QString(), QString() );
}

QTEST_MAIN( TestQgsComposerDD )
#include "testqgscomposerdd.moc"

