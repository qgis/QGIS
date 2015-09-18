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
#include "qgscomposermap.h"
#include "qgscomposertexttable.h"
#include "qgscomposerattributetable.h"
#include "qgsmapsettings.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsfeature.h"

#include <QObject>
#include <QtTest/QtTest>

class TestQgsComposerTable : public QObject
{
    Q_OBJECT

  public:
    TestQgsComposerTable()
        : mComposition( 0 )
        , mComposerMap( 0 )
        , mComposerTextTable( 0 )
        , mMapSettings( 0 )
        , mVectorLayer( 0 )
        , mComposerAttributeTable( 0 )
    {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void textTableHeadings(); //test setting/retrieving text table headers
    void textTableRows(); //test adding and retrieving text table rows
    void attributeTableHeadings(); //test retrieving attribute table headers
    void attributeTableRows(); //test retrieving attribute table rows
    void attributeTableFilterFeatures(); //test filtering attribute table rows
    void attributeTableSetAttributes(); //test subset of attributes in table
    void attributeTableSetAliasOnSubset(); //test setting alias for attribute table with subset of attributes
    void attributeTableAlias(); //test setting alias for attribute column
    void attributeTableGetAlias(); //test getting alias map for attribute table
    void attributeTableVisibleOnly(); //test displaying only visible attributes
    void attributeTableSort(); //test sorting of attribute table
    void attributeTableGetAttributes(); //test getting subset of attributes in table

  private:
    QgsComposition* mComposition;
    QgsComposerMap* mComposerMap;
    QgsComposerTextTable* mComposerTextTable;
    QgsMapSettings *mMapSettings;
    QgsVectorLayer* mVectorLayer;
    QgsComposerAttributeTable* mComposerAttributeTable;

    //compares rows in mComposerAttributeTable to expected rows
    void compareTable( QList<QStringList> &expectedRows );
};

void TestQgsComposerTable::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mMapSettings = new QgsMapSettings();

  //create maplayers from testdata and add to layer registry
  QFileInfo vectorFileInfo( QString( TEST_DATA_DIR ) + "/" +  "points.shp" );
  mVectorLayer = new QgsVectorLayer( vectorFileInfo.filePath(),
                                     vectorFileInfo.completeBaseName(),
                                     "ogr" );

  //create composition with composer map
  mMapSettings->setLayers( QStringList() << mVectorLayer->id() );
  mMapSettings->setCrsTransformEnabled( false );
  mComposition = new QgsComposition( *mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  mComposerTextTable = new QgsComposerTextTable( mComposition );
  mComposition->addItem( mComposerTextTable );

  mComposerAttributeTable = new QgsComposerAttributeTable( mComposition );
  mComposition->addComposerTable( mComposerAttributeTable );
  mComposerAttributeTable->setVectorLayer( mVectorLayer );
  mComposerAttributeTable->setDisplayOnlyVisibleFeatures( false );
  mComposerAttributeTable->setMaximumNumberOfFeatures( 10 );
}

void TestQgsComposerTable::cleanupTestCase()
{
  delete mComposerMap;
  delete mComposition;
  delete mMapSettings;

  QgsApplication::exitQgis();
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
  headers << "1" << "2";
  mComposerTextTable->setHeaderLabels( headers );
  //call this twice, to test that headers are overwritten and not appended
  headers.clear();
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

void TestQgsComposerTable::textTableRows()
{
  //test adding and retrieving text table rows

  //add some rows to the table
  QList<QStringList> rows;
  QStringList row;
  row << "a1" << "b1" << "c1";
  rows.append( row );
  row.clear();
  row << "a2" << "b2" << "c2";
  rows.append( row );
  row.clear();
  row << "a3" << "b3" << "c3";
  rows.append( row );
  QList<QStringList>::const_iterator rowIt = rows.constBegin();
  for ( ; rowIt != rows.constEnd(); ++rowIt )
  {
    mComposerTextTable->addRow( *rowIt );
  }

  //now retrieve rows and check
  QList<QgsAttributeMap> evaluatedRows;
  bool result = mComposerTextTable->getFeatureAttributes( evaluatedRows );
  QCOMPARE( result, true );

  QList<QgsAttributeMap>::const_iterator resultIt = evaluatedRows.constBegin();
  int rowNumber = 0;
  int colNumber = 0;
  for ( ; resultIt != evaluatedRows.constEnd(); ++resultIt )
  {
    colNumber = 0;
    QgsAttributeMap::const_iterator cellIt = ( *resultIt ).constBegin();
    for ( ; cellIt != ( *resultIt ).constEnd(); ++cellIt )
    {
      QCOMPARE(( *cellIt ).toString(), rows.at( rowNumber ).at( colNumber ) );
      colNumber++;
    }
    rowNumber++;
  }
}

void TestQgsComposerTable::attributeTableHeadings()
{
  //test retrieving attribute table headers
  QStringList expectedHeaders;
  expectedHeaders << "Class" << "Heading" << "Importance" << "Pilots" << "Cabin Crew" << "Staff";

  //get header labels and compare
  QMap<int, QString> headerMap = mComposerAttributeTable->headerLabels();
  QMap<int, QString>::const_iterator headerIt = headerMap.constBegin();
  QString expected;
  QString evaluated;
  for ( ; headerIt != headerMap.constEnd(); ++headerIt )
  {
    evaluated = headerIt.value();
    expected = expectedHeaders.at( headerIt.key() );
    QCOMPARE( evaluated, expected );
  }
}

void TestQgsComposerTable::compareTable( QList<QStringList> &expectedRows )
{
  //retrieve rows and check
  QList<QgsAttributeMap> evaluatedRows;
  bool result = mComposerAttributeTable->getFeatureAttributes( evaluatedRows );
  QCOMPARE( result, true );

  QList<QgsAttributeMap>::const_iterator resultIt = evaluatedRows.constBegin();
  int rowNumber = 0;
  int colNumber = 0;

  //check that number of rows matches expected
  QCOMPARE( evaluatedRows.count(), expectedRows.count() );

  for ( ; resultIt != evaluatedRows.constEnd(); ++resultIt )
  {
    colNumber = 0;
    QgsAttributeMap::const_iterator cellIt = ( *resultIt ).constBegin();
    for ( ; cellIt != ( *resultIt ).constEnd(); ++cellIt )
    {
      QCOMPARE(( *cellIt ).toString(), expectedRows.at( rowNumber ).at( colNumber ) );
      colNumber++;
    }
    //also check that number of columns matches expected
    QCOMPARE(( *resultIt ).count(), expectedRows.at( rowNumber ).count() );

    rowNumber++;
  }
}

void TestQgsComposerTable::attributeTableRows()
{
  //test retrieving attribute table rows

  QList<QStringList> expectedRows;
  QStringList row;
  row << "Jet" << "90" << "3" << "2" << "0" << "2";
  expectedRows.append( row );
  row.clear();
  row << "Biplane" << "0" << "1" << "3" << "3" << "6";
  expectedRows.append( row );
  row.clear();
  row << "Jet" << "85" << "3" << "1" << "1" << "2";
  expectedRows.append( row );

  //retrieve rows and check
  mComposerAttributeTable->setMaximumNumberOfFeatures( 3 );
  compareTable( expectedRows );
}

void TestQgsComposerTable::attributeTableFilterFeatures()
{
  //test filtering attribute table rows
  mComposerAttributeTable->setMaximumNumberOfFeatures( 10 );
  mComposerAttributeTable->setFeatureFilter( QString( "\"Class\"='B52'" ) );
  mComposerAttributeTable->setFilterFeatures( true );

  QList<QStringList> expectedRows;
  QStringList row;
  row << "B52" << "0" << "10" << "2" << "1" << "3";
  expectedRows.append( row );
  row.clear();
  row << "B52" << "12" << "10" << "1" << "1" << "2";
  expectedRows.append( row );
  row.clear();
  row << "B52" << "34" << "10" << "2" << "1" << "3";
  expectedRows.append( row );
  row.clear();
  row << "B52" << "80" << "10" << "2" << "1" << "3";
  expectedRows.append( row );

  //retrieve rows and check
  compareTable( expectedRows );

  mComposerAttributeTable->setFilterFeatures( false );
}

void TestQgsComposerTable::attributeTableSetAttributes()
{
  //test subset of attributes in table
  QSet<int> attributes;
  attributes << 0 << 3 << 4;
  mComposerAttributeTable->setDisplayAttributes( attributes );
  mComposerAttributeTable->setMaximumNumberOfFeatures( 3 );

  //check headers
  QStringList expectedHeaders;
  expectedHeaders << "Class" << "Pilots" << "Cabin Crew";

  //get header labels and compare
  QMap<int, QString> headerMap = mComposerAttributeTable->headerLabels();
  QMap<int, QString>::const_iterator headerIt = headerMap.constBegin();
  QString expected;
  QString evaluated;
  for ( ; headerIt != headerMap.constEnd(); ++headerIt )
  {
    evaluated = headerIt.value();
    expected = expectedHeaders.at( headerIt.key() );
    QCOMPARE( evaluated, expected );
  }

  QList<QStringList> expectedRows;
  QStringList row;
  row << "Jet" << "2" << "0";
  expectedRows.append( row );
  row.clear();
  row << "Biplane" << "3" << "3";
  expectedRows.append( row );
  row.clear();
  row << "Jet" << "1" << "1";
  expectedRows.append( row );

  //retrieve rows and check
  compareTable( expectedRows );

  attributes.clear();
  mComposerAttributeTable->setDisplayAttributes( attributes );
}

void TestQgsComposerTable::attributeTableSetAliasOnSubset()
{
  //test setting alias for attribute table with subset of attributes
  QStringList expectedHeaders;
  expectedHeaders << "1Heading" << "2Pilots" << "3Cabin Crew";

  QSet<int> attributes;
  attributes << 1 << 3 << 4;
  mComposerAttributeTable->setDisplayAttributes( attributes );
  QMap<int, QString> aliases;
  aliases.insert( 1, QString( "1Heading" ) );
  aliases.insert( 3, QString( "2Pilots" ) );
  aliases.insert( 4, QString( "3Cabin Crew" ) );
  Q_NOWARN_DEPRECATED_PUSH
  mComposerAttributeTable->setFieldAliasMap( aliases );
  Q_NOWARN_DEPRECATED_POP

  //get header labels and compare
  QMap<int, QString> headerMap = mComposerAttributeTable->headerLabels();
  QMap<int, QString>::const_iterator headerIt = headerMap.constBegin();
  QString expected;
  QString evaluated;
  for ( ; headerIt != headerMap.constEnd(); ++headerIt )
  {
    evaluated = headerIt.value();
    expected = expectedHeaders.at( headerIt.key() );
    QCOMPARE( evaluated, expected );
  }
  attributes.clear();
  aliases.clear();
  mComposerAttributeTable->setDisplayAttributes( attributes );
  Q_NOWARN_DEPRECATED_PUSH
  mComposerAttributeTable->setFieldAliasMap( aliases );
  Q_NOWARN_DEPRECATED_POP
}

void TestQgsComposerTable::attributeTableGetAttributes()
{
  //test getting subset of attributes in table
  QSet<int> attributes;
  attributes << 0 << 3 << 4;
  mComposerAttributeTable->setDisplayAttributes( attributes );

  Q_NOWARN_DEPRECATED_PUSH
  QSet<int> evaluated = mComposerAttributeTable->displayAttributes();
  Q_NOWARN_DEPRECATED_POP

  QCOMPARE( evaluated, attributes );

  attributes.clear();
  mComposerAttributeTable->setDisplayAttributes( attributes );
}

void TestQgsComposerTable::attributeTableAlias()
{
  //test setting alias for attribute column
  QMap<int, QString> fieldAliasMap;

  fieldAliasMap.insert( 0, QString( "alias 0" ) );
  fieldAliasMap.insert( 3, QString( "alias 3" ) );
  Q_NOWARN_DEPRECATED_PUSH
  mComposerAttributeTable->setFieldAliasMap( fieldAliasMap );
  Q_NOWARN_DEPRECATED_POP

  QStringList expectedHeaders;
  expectedHeaders << "alias 0" << "Heading" << "Importance" << "alias 3" << "Cabin Crew" << "Staff";

  //get header labels and compare
  QMap<int, QString> headerMap = mComposerAttributeTable->headerLabels();
  QMap<int, QString>::const_iterator headerIt = headerMap.constBegin();
  QString expected;
  QString evaluated;
  for ( ; headerIt != headerMap.constEnd(); ++headerIt )
  {
    evaluated = headerIt.value();
    expected = expectedHeaders.at( headerIt.key() );
    QCOMPARE( evaluated, expected );
  }

  fieldAliasMap.clear();
  Q_NOWARN_DEPRECATED_PUSH
  mComposerAttributeTable->setFieldAliasMap( fieldAliasMap );
  Q_NOWARN_DEPRECATED_POP
}

void TestQgsComposerTable::attributeTableGetAlias()
{
  QSet<int> attributes;
  attributes << 1 << 3 << 4;
  mComposerAttributeTable->setDisplayAttributes( attributes );

  //test getting alias map
  QMap<int, QString> fieldAliasMap;

  fieldAliasMap.insert( 1, QString( "alias 1" ) );
  fieldAliasMap.insert( 2, QString( "alias 2" ) );
  Q_NOWARN_DEPRECATED_PUSH
  mComposerAttributeTable->setFieldAliasMap( fieldAliasMap );
  Q_NOWARN_DEPRECATED_POP

  QMap<int, QString> expectedAliases;
  expectedAliases.insert( 1, QString( "alias 1" ) );
  expectedAliases.insert( 3, QString( "Pilots" ) );
  expectedAliases.insert( 4, QString( "Cabin Crew" ) );

  //get header labels and compare
  Q_NOWARN_DEPRECATED_PUSH
  QMap<int, QString> aliasMap = mComposerAttributeTable->fieldAliasMap();
  Q_NOWARN_DEPRECATED_POP
  QMap<int, QString>::const_iterator aliasIt = aliasMap.constBegin();
  QString expected;
  QString evaluated;
  for ( ; aliasIt != aliasMap.constEnd(); ++aliasIt )
  {
    evaluated = aliasIt.value();
    expected = expectedAliases.value( aliasIt.key() );
    QCOMPARE( evaluated, expected );
  }

  fieldAliasMap.clear();
  Q_NOWARN_DEPRECATED_PUSH
  mComposerAttributeTable->setFieldAliasMap( fieldAliasMap );
  Q_NOWARN_DEPRECATED_POP
  attributes.clear();
  mComposerAttributeTable->setDisplayAttributes( attributes );
}

void TestQgsComposerTable::attributeTableSort()
{
  //test sorting of attribute table
  QList< QPair<int, bool> > sort;
  sort.append( qMakePair( 0, true ) );
  sort.append( qMakePair( 1, false ) );
  sort.append( qMakePair( 3, true ) );
  Q_NOWARN_DEPRECATED_PUSH
  mComposerAttributeTable->setSortAttributes( sort );
  Q_NOWARN_DEPRECATED_POP
  mComposerAttributeTable->setMaximumNumberOfFeatures( 5 );

  QList<QStringList> expectedRows;
  QStringList row;
  row << "Biplane" << "0" << "1" << "3" << "3" << "6";
  expectedRows.append( row );
  row.clear();
  row << "Jet" << "95" << "3" << "1" << "1" << "2";
  expectedRows.append( row );
  row.clear();
  row << "Jet" << "90" << "3" << "2" << "0" << "2";
  expectedRows.append( row );
  row.clear();
  row << "Jet" << "90" << "3" << "1" << "0" << "1";
  expectedRows.append( row );
  row.clear();
  row << "Jet" << "85" << "3" << "1" << "1" << "2";
  expectedRows.append( row );

  //retrieve rows and check
  compareTable( expectedRows );

  sort.clear();
  Q_NOWARN_DEPRECATED_PUSH
  mComposerAttributeTable->setSortAttributes( sort );
  Q_NOWARN_DEPRECATED_POP
}

void TestQgsComposerTable::attributeTableVisibleOnly()
{
  //test displaying only visible attributes

  mComposerMap = new QgsComposerMap( mComposition, 20, 20, 200, 100 );
  mComposerMap->setFrameEnabled( true );
  mComposition->addComposerMap( mComposerMap );
  mComposerMap->setNewExtent( QgsRectangle( -131.767, 30.558, -110.743, 41.070 ) );

  mComposerAttributeTable->setComposerMap( mComposerMap );
  mComposerAttributeTable->setDisplayOnlyVisibleFeatures( true );

  QList<QStringList> expectedRows;
  QStringList row;
  row << "Jet" << "90" << "3" << "2" << "0" << "2";
  expectedRows.append( row );
  row.clear();
  row << "Biplane" << "240" << "1" << "3" << "2" << "5";
  expectedRows.append( row );
  row.clear();
  row << "Jet" << "180" << "3" << "1" << "0" << "1";
  expectedRows.append( row );

  //retrieve rows and check
  compareTable( expectedRows );

  mComposerAttributeTable->setDisplayOnlyVisibleFeatures( false );
  mComposerAttributeTable->setComposerMap( 0 );
  mComposition->removeItem( mComposerMap );
}

QTEST_MAIN( TestQgsComposerTable )
#include "testqgscomposertable.moc"
