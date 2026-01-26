/***************************************************************************
                         testqgsquerybuilder.cpp
                         ---------------------------
    begin                : June 2021
    copyright            : (C) 2021 by Julien Cabieces
    email                : julien.cabieces@oslandia.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsquerybuilder.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

class TestQgsQueryBuilder : public QObject
{
    Q_OBJECT

  public:
    TestQgsQueryBuilder() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void testFillValues();

  private:
    QStringList getModelItemDisplayStrings( QStandardItemModel *model );
};

void TestQgsQueryBuilder::initTestCase() // will be called before the first testfunction is executed.
{
}

void TestQgsQueryBuilder::cleanupTestCase()
{
}

void TestQgsQueryBuilder::init()
{
}

void TestQgsQueryBuilder::cleanup()
{
}

QStringList TestQgsQueryBuilder::getModelItemDisplayStrings( QStandardItemModel *model )
{
  QStringList result;
  for ( int row = 0; row < model->rowCount(); row++ )
  {
    result << model->item( row )->text();
  }

  result.sort();

  return result;
}


void TestQgsQueryBuilder::testFillValues()
{
  QgsVectorLayer vl( u"Point?field=intarray:int[]&field=strarray:string[]&field=intf:int"_s, u"test"_s, u"memory"_s );

  QgsFeature feat1( vl.fields() );
  feat1.setAttribute( u"intarray"_s, QVariantList() << 1 );
  feat1.setAttribute( u"strarray"_s, QVariantList() << u"testA"_s );
  feat1.setAttribute( u"intf"_s, 0 );
  vl.dataProvider()->addFeature( feat1 );

  QgsFeature feat2( vl.fields() );
  feat2.setAttribute( u"intarray"_s, QVariantList() << 2 << 3 );
  feat2.setAttribute( u"strarray"_s, QVariantList() << QVariant() );
  feat2.setAttribute( u"intf"_s, 42 );
  vl.dataProvider()->addFeature( feat2 );

  QgsFeature feat3( vl.fields() );
  feat3.setAttribute( u"intarray"_s, QVariantList() << QVariant() );
  feat3.setAttribute( u"strarray"_s, QVariantList() << u"testB"_s << u"testC"_s );
  feat3.setAttribute( u"intf"_s, 42 );
  vl.dataProvider()->addFeature( feat3 );

  QgsFeature feat4( vl.fields() );
  feat4.setAttribute( u"intarray"_s, QVariantList() << 4 << 5 << 6 );
  feat4.setAttribute( u"strarray"_s, QVariantList() << QVariant() );
  feat4.setAttribute( u"intf"_s, QVariant() );
  vl.dataProvider()->addFeature( feat4 );

  QgsQueryBuilder queryBuilder( &vl );

  queryBuilder.fillValues( "intarray", 10 );
  QCOMPARE( getModelItemDisplayStrings( queryBuilder.mModelValues ), QStringList() << "1" << "2, 3" << "4, 5, 6" << "NULL" );

  queryBuilder.fillValues( "strarray", 10 );
  QCOMPARE( getModelItemDisplayStrings( queryBuilder.mModelValues ), QStringList() << "NULL" << "testA" << "testB, testC" );

  queryBuilder.fillValues( "intf", 10 );
  QCOMPARE( getModelItemDisplayStrings( queryBuilder.mModelValues ), QStringList() << "0" << "42" << "NULL" );
}

QGSTEST_MAIN( TestQgsQueryBuilder )
#include "testqgsquerybuilder.moc"
