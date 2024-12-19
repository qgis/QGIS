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


#include "qgstest.h"

#include <qgsvectorlayer.h>
#include <qgsquerybuilder.h>

class TestQgsQueryBuilder : public QObject
{
    Q_OBJECT

  public:
    TestQgsQueryBuilder() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
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
  QgsVectorLayer vl( QStringLiteral( "Point?field=intarray:int[]&field=strarray:string[]&field=intf:int" ), QStringLiteral( "test" ), QStringLiteral( "memory" ) );

  QgsFeature feat1( vl.fields() );
  feat1.setAttribute( QStringLiteral( "intarray" ), QVariantList() << 1 );
  feat1.setAttribute( QStringLiteral( "strarray" ), QVariantList() << QStringLiteral( "testA" ) );
  feat1.setAttribute( QStringLiteral( "intf" ), 0 );
  vl.dataProvider()->addFeature( feat1 );

  QgsFeature feat2( vl.fields() );
  feat2.setAttribute( QStringLiteral( "intarray" ), QVariantList() << 2 << 3 );
  feat2.setAttribute( QStringLiteral( "strarray" ), QVariantList() << QVariant() );
  feat2.setAttribute( QStringLiteral( "intf" ), 42 );
  vl.dataProvider()->addFeature( feat2 );

  QgsFeature feat3( vl.fields() );
  feat3.setAttribute( QStringLiteral( "intarray" ), QVariantList() << QVariant() );
  feat3.setAttribute( QStringLiteral( "strarray" ), QVariantList() << QStringLiteral( "testB" ) << QStringLiteral( "testC" ) );
  feat3.setAttribute( QStringLiteral( "intf" ), 42 );
  vl.dataProvider()->addFeature( feat3 );

  QgsFeature feat4( vl.fields() );
  feat4.setAttribute( QStringLiteral( "intarray" ), QVariantList() << 4 << 5 << 6 );
  feat4.setAttribute( QStringLiteral( "strarray" ), QVariantList() << QVariant() );
  feat4.setAttribute( QStringLiteral( "intf" ), QVariant() );
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
