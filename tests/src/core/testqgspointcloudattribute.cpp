/***************************************************************************
     testqgspointcloudattribute.cpp
     -------------------
    Date                 : November 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QSettings>

#include <ogr_api.h>
#include "cpl_conv.h"
#include "cpl_string.h"

#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsogrutils.h"
#include "qgsapplication.h"
#include "qgspoint.h"
#include "qgspointcloudattribute.h"

class TestQgsPointCloudAttribute: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void testAttribute();
    void testAttributeDisplayType();
    void testVariantType();
    void testIsNumeric();
    void testCollection();
    void testCollectionFindCaseInsensitive();
    void testCollevtionExtend();
    void testToFields();

  private:

    QString mTestDataDir;
};

void TestQgsPointCloudAttribute::initTestCase()
{
  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::registerOgrDrivers();
}

void TestQgsPointCloudAttribute::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsPointCloudAttribute::init()
{

}

void TestQgsPointCloudAttribute::cleanup()
{

}

void TestQgsPointCloudAttribute::testAttribute()
{
  // basic tests
  const QgsPointCloudAttribute attribute( QStringLiteral( "name" ), QgsPointCloudAttribute::DataType::Float );
  QCOMPARE( attribute.name(), QStringLiteral( "name" ) );
  QCOMPARE( attribute.type(), QgsPointCloudAttribute::DataType::Float );
  QCOMPARE( attribute.size(), 4 );
}

void TestQgsPointCloudAttribute::testAttributeDisplayType()
{
  QCOMPARE( QgsPointCloudAttribute( QStringLiteral( "x" ), QgsPointCloudAttribute::DataType::Char ).displayType(), QStringLiteral( "Character" ) );
  QCOMPARE( QgsPointCloudAttribute( QStringLiteral( "x" ), QgsPointCloudAttribute::DataType::Short ).displayType(), QStringLiteral( "Short" ) );
  QCOMPARE( QgsPointCloudAttribute( QStringLiteral( "x" ), QgsPointCloudAttribute::DataType::UShort ).displayType(), QStringLiteral( "Unsigned Short" ) );
  QCOMPARE( QgsPointCloudAttribute( QStringLiteral( "x" ), QgsPointCloudAttribute::DataType::Int32 ).displayType(), QStringLiteral( "Integer" ) );
  QCOMPARE( QgsPointCloudAttribute( QStringLiteral( "x" ), QgsPointCloudAttribute::DataType::Float ).displayType(), QStringLiteral( "Float" ) );
  QCOMPARE( QgsPointCloudAttribute( QStringLiteral( "x" ), QgsPointCloudAttribute::DataType::Double ).displayType(), QStringLiteral( "Double" ) );
}

void TestQgsPointCloudAttribute::testVariantType()
{
  QCOMPARE( QgsPointCloudAttribute( QStringLiteral( "x" ), QgsPointCloudAttribute::DataType::Char ).variantType(), QVariant::Int );
  QCOMPARE( QgsPointCloudAttribute( QStringLiteral( "x" ), QgsPointCloudAttribute::DataType::Short ).variantType(), QVariant::Int );
  QCOMPARE( QgsPointCloudAttribute( QStringLiteral( "x" ), QgsPointCloudAttribute::DataType::UShort ).variantType(), QVariant::Int );
  QCOMPARE( QgsPointCloudAttribute( QStringLiteral( "x" ), QgsPointCloudAttribute::DataType::Int32 ).variantType(), QVariant::Int );
  QCOMPARE( QgsPointCloudAttribute( QStringLiteral( "x" ), QgsPointCloudAttribute::DataType::Float ).variantType(), QVariant::Double );
  QCOMPARE( QgsPointCloudAttribute( QStringLiteral( "x" ), QgsPointCloudAttribute::DataType::Double ).variantType(), QVariant::Double );
}

void TestQgsPointCloudAttribute::testIsNumeric()
{
  QVERIFY( !QgsPointCloudAttribute::isNumeric( QgsPointCloudAttribute::DataType::Char ) );
  QVERIFY( QgsPointCloudAttribute::isNumeric( QgsPointCloudAttribute::DataType::Short ) );
  QVERIFY( QgsPointCloudAttribute::isNumeric( QgsPointCloudAttribute::DataType::UShort ) );
  QVERIFY( QgsPointCloudAttribute::isNumeric( QgsPointCloudAttribute::DataType::Int32 ) );
  QVERIFY( QgsPointCloudAttribute::isNumeric( QgsPointCloudAttribute::DataType::Float ) );
  QVERIFY( QgsPointCloudAttribute::isNumeric( QgsPointCloudAttribute::DataType::Double ) );
}

void TestQgsPointCloudAttribute::testCollection()
{
  // test collections
  QgsPointCloudAttributeCollection collection;
  QVERIFY( collection.attributes().empty() );
  QCOMPARE( collection.count(), 0 );
  QCOMPARE( collection.pointRecordSize(), 0 );
  int offset = 0;
  QVERIFY( !collection.find( QStringLiteral( "test" ), offset ) );

  collection.push_back( QgsPointCloudAttribute( QStringLiteral( "at1" ), QgsPointCloudAttribute::DataType::Float ) );
  QCOMPARE( collection.attributes().size(), 1 );
  QCOMPARE( collection.count(), 1 );
  QCOMPARE( collection.at( 0 ).name(), QStringLiteral( "at1" ) );
  QCOMPARE( collection.attributes().at( 0 ).name(), QStringLiteral( "at1" ) );
  QCOMPARE( collection.pointRecordSize(), 4 );
  QVERIFY( !collection.find( QStringLiteral( "test" ), offset ) );
  QCOMPARE( collection.find( QStringLiteral( "at1" ), offset )->name(), QStringLiteral( "at1" ) );
  QCOMPARE( collection.indexOf( QLatin1String( "test" ) ), -1 );
  QCOMPARE( collection.indexOf( QLatin1String( "at1" ) ), 0 );
  QCOMPARE( offset, 0 );

  collection.push_back( QgsPointCloudAttribute( QStringLiteral( "at2" ), QgsPointCloudAttribute::DataType::Short ) );
  QCOMPARE( collection.attributes().size(), 2 );
  QCOMPARE( collection.count(), 2 );
  QCOMPARE( collection.at( 0 ).name(), QStringLiteral( "at1" ) );
  QCOMPARE( collection.at( 1 ).name(), QStringLiteral( "at2" ) );
  QCOMPARE( collection.attributes().at( 0 ).name(), QStringLiteral( "at1" ) );
  QCOMPARE( collection.attributes().at( 1 ).name(), QStringLiteral( "at2" ) );
  QCOMPARE( collection.pointRecordSize(), 6 );
  QVERIFY( !collection.find( QStringLiteral( "test" ), offset ) );
  QCOMPARE( collection.find( QStringLiteral( "at1" ), offset )->name(), QStringLiteral( "at1" ) );
  QCOMPARE( offset, 0 );
  QCOMPARE( collection.find( QStringLiteral( "at2" ), offset )->name(), QStringLiteral( "at2" ) );
  QCOMPARE( offset, 4 );
  QCOMPARE( collection.indexOf( QLatin1String( "test" ) ), -1 );
  QCOMPARE( collection.indexOf( QLatin1String( "at1" ) ), 0 );
  QCOMPARE( collection.indexOf( QLatin1String( "at2" ) ), 1 );

  collection.push_back( QgsPointCloudAttribute( QStringLiteral( "at3" ), QgsPointCloudAttribute::DataType::Double ) );
  QCOMPARE( collection.attributes().size(), 3 );
  QCOMPARE( collection.count(), 3 );
  QCOMPARE( collection.attributes().at( 0 ).name(), QStringLiteral( "at1" ) );
  QCOMPARE( collection.attributes().at( 1 ).name(), QStringLiteral( "at2" ) );
  QCOMPARE( collection.attributes().at( 2 ).name(), QStringLiteral( "at3" ) );
  QCOMPARE( collection.pointRecordSize(), 14 );
  QVERIFY( !collection.find( QStringLiteral( "test" ), offset ) );
  QCOMPARE( collection.find( QStringLiteral( "at1" ), offset )->name(), QStringLiteral( "at1" ) );
  QCOMPARE( offset, 0 );
  QCOMPARE( collection.find( QStringLiteral( "at2" ), offset )->name(), QStringLiteral( "at2" ) );
  QCOMPARE( offset, 4 );
  QCOMPARE( collection.find( QStringLiteral( "at3" ), offset )->name(), QStringLiteral( "at3" ) );
  QCOMPARE( offset, 6 );

  // populate from other attributes
  const QgsPointCloudAttributeCollection collection2( QVector< QgsPointCloudAttribute >()
      << QgsPointCloudAttribute( QStringLiteral( "at1" ), QgsPointCloudAttribute::DataType::Float )
      << QgsPointCloudAttribute( QStringLiteral( "at2" ), QgsPointCloudAttribute::DataType::Short )
      << QgsPointCloudAttribute( QStringLiteral( "at3" ), QgsPointCloudAttribute::DataType::Double ) );
  QCOMPARE( collection2.attributes().size(), 3 );
  QCOMPARE( collection2.count(), 3 );
  QCOMPARE( collection2.attributes().at( 0 ).name(), QStringLiteral( "at1" ) );
  QCOMPARE( collection2.attributes().at( 1 ).name(), QStringLiteral( "at2" ) );
  QCOMPARE( collection2.attributes().at( 2 ).name(), QStringLiteral( "at3" ) );
  QCOMPARE( collection2.pointRecordSize(), 14 );
  QVERIFY( !collection2.find( QStringLiteral( "test" ), offset ) );
  QCOMPARE( collection2.find( QStringLiteral( "at1" ), offset )->name(), QStringLiteral( "at1" ) );
  QCOMPARE( offset, 0 );
  QCOMPARE( collection2.find( QStringLiteral( "at2" ), offset )->name(), QStringLiteral( "at2" ) );
  QCOMPARE( offset, 4 );
  QCOMPARE( collection2.find( QStringLiteral( "at3" ), offset )->name(), QStringLiteral( "at3" ) );
  QCOMPARE( offset, 6 );
}

void TestQgsPointCloudAttribute::testCollectionFindCaseInsensitive()
{
  int offset = 0;
  const QgsPointCloudAttributeCollection collection( QVector< QgsPointCloudAttribute >()
      << QgsPointCloudAttribute( QStringLiteral( "at1" ), QgsPointCloudAttribute::DataType::Float )
      << QgsPointCloudAttribute( QStringLiteral( "at2" ), QgsPointCloudAttribute::DataType::Short )
      << QgsPointCloudAttribute( QStringLiteral( "AT3" ), QgsPointCloudAttribute::DataType::Double ) );
  QCOMPARE( collection.attributes().size(), 3 );
  QCOMPARE( collection.count(), 3 );
  QCOMPARE( collection.attributes().at( 0 ).name(), QStringLiteral( "at1" ) );
  QCOMPARE( collection.attributes().at( 1 ).name(), QStringLiteral( "at2" ) );
  QCOMPARE( collection.attributes().at( 2 ).name(), QStringLiteral( "AT3" ) );
  QCOMPARE( collection.pointRecordSize(), 14 );
  QVERIFY( !collection.find( QStringLiteral( "test" ), offset ) );
  QCOMPARE( collection.find( QStringLiteral( "At1" ), offset )->name(), QStringLiteral( "at1" ) );
  QCOMPARE( offset, 0 );
  QCOMPARE( collection.find( QStringLiteral( "aT2" ), offset )->name(), QStringLiteral( "at2" ) );
  QCOMPARE( offset, 4 );
  QCOMPARE( collection.find( QStringLiteral( "aT3" ), offset )->name(), QStringLiteral( "AT3" ) );
  QCOMPARE( offset, 6 );
}

void TestQgsPointCloudAttribute::testCollevtionExtend()
{
  int offset = 0;
  QgsPointCloudAttributeCollection collection( QVector< QgsPointCloudAttribute >()
      << QgsPointCloudAttribute( QStringLiteral( "at1" ), QgsPointCloudAttribute::DataType::Float )
      << QgsPointCloudAttribute( QStringLiteral( "at2" ), QgsPointCloudAttribute::DataType::Short )
      << QgsPointCloudAttribute( QStringLiteral( "at3" ), QgsPointCloudAttribute::DataType::Double ) );
  const QgsPointCloudAttributeCollection collection2( QVector< QgsPointCloudAttribute >()
      << QgsPointCloudAttribute( QStringLiteral( "at1" ), QgsPointCloudAttribute::DataType::Float )
      << QgsPointCloudAttribute( QStringLiteral( "at2" ), QgsPointCloudAttribute::DataType::Short )
      << QgsPointCloudAttribute( QStringLiteral( "at3" ), QgsPointCloudAttribute::DataType::Double )
      << QgsPointCloudAttribute( QStringLiteral( "at4" ), QgsPointCloudAttribute::DataType::Float )
      << QgsPointCloudAttribute( QStringLiteral( "at5" ), QgsPointCloudAttribute::DataType::Short ) );

  collection.extend( collection2, QSet<QString>() );
  QCOMPARE( collection.attributes().size(), 3 );
  QCOMPARE( collection.count(), 3 );
  QCOMPARE( collection.attributes().at( 0 ).name(), QStringLiteral( "at1" ) );
  QCOMPARE( collection.attributes().at( 1 ).name(), QStringLiteral( "at2" ) );
  QCOMPARE( collection.attributes().at( 2 ).name(), QStringLiteral( "at3" ) );
  QCOMPARE( collection.pointRecordSize(), 14 );
  QVERIFY( !collection.find( QStringLiteral( "test" ), offset ) );
  QCOMPARE( collection.find( QStringLiteral( "at1" ), offset )->name(), QStringLiteral( "at1" ) );
  QCOMPARE( offset, 0 );
  QCOMPARE( collection.find( QStringLiteral( "at2" ), offset )->name(), QStringLiteral( "at2" ) );
  QCOMPARE( offset, 4 );
  QCOMPARE( collection.find( QStringLiteral( "at3" ), offset )->name(), QStringLiteral( "at3" ) );
  QCOMPARE( offset, 6 );

  collection.extend( collection2, QSet<QString>() << QStringLiteral( "at4" ) );
  QCOMPARE( collection.attributes().size(), 4 );
  QCOMPARE( collection.count(), 4 );
  QVERIFY( !collection.find( QStringLiteral( "at5" ), offset ) );
  QCOMPARE( collection.find( QStringLiteral( "at1" ), offset )->name(), QStringLiteral( "at1" ) );
  QCOMPARE( offset, 0 );
  QCOMPARE( collection.find( QStringLiteral( "at2" ), offset )->name(), QStringLiteral( "at2" ) );
  QCOMPARE( offset, 4 );
  QCOMPARE( collection.find( QStringLiteral( "at3" ), offset )->name(), QStringLiteral( "at3" ) );
  QCOMPARE( offset, 6 );
  QCOMPARE( collection.find( QStringLiteral( "at4" ), offset )->name(), QStringLiteral( "at4" ) );
  QCOMPARE( offset, 14 );

  collection.extend( collection2, QSet<QString>() << QStringLiteral( "at4" ) << QStringLiteral( "at5" ) << QStringLiteral( "at6" ) );
  QCOMPARE( collection.attributes().size(), 5 );
  QCOMPARE( collection.count(), 5 );
  QVERIFY( !collection.find( QStringLiteral( "at6" ), offset ) );
  QCOMPARE( collection.find( QStringLiteral( "at1" ), offset )->name(), QStringLiteral( "at1" ) );
  QCOMPARE( offset, 0 );
  QCOMPARE( collection.find( QStringLiteral( "at2" ), offset )->name(), QStringLiteral( "at2" ) );
  QCOMPARE( offset, 4 );
  QCOMPARE( collection.find( QStringLiteral( "at3" ), offset )->name(), QStringLiteral( "at3" ) );
  QCOMPARE( offset, 6 );
  QCOMPARE( collection.find( QStringLiteral( "at4" ), offset )->name(), QStringLiteral( "at4" ) );
  QCOMPARE( collection.find( QStringLiteral( "at5" ), offset )->name(), QStringLiteral( "at5" ) );
}

void TestQgsPointCloudAttribute::testToFields()
{
  QgsFields fields = QgsPointCloudAttributeCollection().toFields();
  QCOMPARE( fields.size(), 0 );

  const QgsPointCloudAttributeCollection collection( QVector< QgsPointCloudAttribute >()
      << QgsPointCloudAttribute( QStringLiteral( "at1" ), QgsPointCloudAttribute::DataType::Float )
      << QgsPointCloudAttribute( QStringLiteral( "at2" ), QgsPointCloudAttribute::DataType::Short )
      << QgsPointCloudAttribute( QStringLiteral( "at3" ), QgsPointCloudAttribute::DataType::Double ) );
  fields = collection.toFields();
  QCOMPARE( fields.size(), 3 );

  QCOMPARE( fields.at( 0 ).name(), QStringLiteral( "at1" ) );
  QCOMPARE( fields.at( 0 ).type(), QVariant::Double );
  QCOMPARE( fields.at( 1 ).name(), QStringLiteral( "at2" ) );
  QCOMPARE( fields.at( 1 ).type(), QVariant::Int );
  QCOMPARE( fields.at( 2 ).name(), QStringLiteral( "at3" ) );
  QCOMPARE( fields.at( 2 ).type(), QVariant::Double );
}

QGSTEST_MAIN( TestQgsPointCloudAttribute )
#include "testqgspointcloudattribute.moc"
