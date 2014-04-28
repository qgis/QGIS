/***************************************************************************
                         testqgscomposertable.cpp
                         ----------------------
    begin                : April 2014
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
#include "qgscomposertexttable.h"
#include "qgscomposerattributetable.h"
#include "qgsmapsettings.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

#include <QObject>
#include <QtTest>

class TestQgsComposerTable: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void textTableHeadings(); //test setting/retrieving text table headers

  private:
    QgsComposition* mComposition;
    QgsComposerTextTable* mComposerTextTable;
    QgsMapSettings mMapSettings;
    QgsVectorLayer* mVectorLayer;
};

void TestQgsComposerTable::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create maplayers from testdata and add to layer registry
  QFileInfo vectorFileInfo( QString( TEST_DATA_DIR ) + QDir::separator() +  "france_parts.shp" );
  mVectorLayer = new QgsVectorLayer( vectorFileInfo.filePath(),
                                     vectorFileInfo.completeBaseName(),
                                     "ogr" );

  //create composition with composer map
  mMapSettings.setLayers( QStringList() << mVectorLayer->id() );
  mMapSettings.setCrsTransformEnabled( false );
  mComposition = new QgsComposition( mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  mComposerTextTable = new QgsComposerTextTable( mComposition );
  mComposition->addItem( mComposerTextTable );

}

void TestQgsComposerTable::cleanupTestCase()
{
  delete mComposition;
  delete mVectorLayer;
}

void TestQgsComposerTable::init()
{
}

void TestQgsComposerTable::cleanup()
{
}

void TestQgsComposerTable::textTableHeadings()
{
  //test setting/retrieving text table headers
  QStringList headers;
  headers << "a" << "b" << "c";
  mComposerTextTable->setHeaderLabels( headers );

  //get header labels and compare
  QMap<int, QString> headerMap = mComposerTextTable->headerLabels();
  QMap<int, QString>::const_iterator headerIt = headerMap.constBegin();
  int col = 0;
  QString expected;
  QString evaluated;
  for ( ; headerIt != headerMap.constEnd(); ++headerIt )
  {
    col = headerIt.key();
    evaluated = headerIt.value();
    expected = headers.at( col );
    QCOMPARE( evaluated, expected );
  }
}

QTEST_MAIN( TestQgsComposerTable )
#include "moc_testqgscomposertable.cxx"
