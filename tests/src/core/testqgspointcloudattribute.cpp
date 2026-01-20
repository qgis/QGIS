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
#include <cpl_conv.h>
#include <cpl_string.h>
#include <ogr_api.h>

#include "qgsapplication.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsogrutils.h"
#include "qgspoint.h"
#include "qgspointcloudattribute.h"
#include "qgstest.h"

#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>

class TestQgsPointCloudAttribute : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
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
  const QgsPointCloudAttribute attribute( u"name"_s, QgsPointCloudAttribute::DataType::Float );
  QCOMPARE( attribute.name(), u"name"_s );
  QCOMPARE( attribute.type(), QgsPointCloudAttribute::DataType::Float );
  QCOMPARE( attribute.size(), 4 );
}

void TestQgsPointCloudAttribute::testAttributeDisplayType()
{
  QCOMPARE( QgsPointCloudAttribute( u"x"_s, QgsPointCloudAttribute::DataType::Char ).displayType(), u"Character"_s );
  QCOMPARE( QgsPointCloudAttribute( u"x"_s, QgsPointCloudAttribute::DataType::Short ).displayType(), u"Short"_s );
  QCOMPARE( QgsPointCloudAttribute( u"x"_s, QgsPointCloudAttribute::DataType::UShort ).displayType(), u"Unsigned Short"_s );
  QCOMPARE( QgsPointCloudAttribute( u"x"_s, QgsPointCloudAttribute::DataType::Int32 ).displayType(), u"Integer"_s );
  QCOMPARE( QgsPointCloudAttribute( u"x"_s, QgsPointCloudAttribute::DataType::Float ).displayType(), u"Float"_s );
  QCOMPARE( QgsPointCloudAttribute( u"x"_s, QgsPointCloudAttribute::DataType::Double ).displayType(), u"Double"_s );
}

void TestQgsPointCloudAttribute::testVariantType()
{
  QCOMPARE( QgsPointCloudAttribute( u"x"_s, QgsPointCloudAttribute::DataType::Char ).variantType(), QMetaType::Type::Int );
  QCOMPARE( QgsPointCloudAttribute( u"x"_s, QgsPointCloudAttribute::DataType::Short ).variantType(), QMetaType::Type::Int );
  QCOMPARE( QgsPointCloudAttribute( u"x"_s, QgsPointCloudAttribute::DataType::UShort ).variantType(), QMetaType::Type::Int );
  QCOMPARE( QgsPointCloudAttribute( u"x"_s, QgsPointCloudAttribute::DataType::Int32 ).variantType(), QMetaType::Type::Int );
  QCOMPARE( QgsPointCloudAttribute( u"x"_s, QgsPointCloudAttribute::DataType::Float ).variantType(), QMetaType::Type::Double );
  QCOMPARE( QgsPointCloudAttribute( u"x"_s, QgsPointCloudAttribute::DataType::Double ).variantType(), QMetaType::Type::Double );
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
  QVERIFY( !collection.find( u"test"_s, offset ) );

  collection.push_back( QgsPointCloudAttribute( u"at1"_s, QgsPointCloudAttribute::DataType::Float ) );
  QCOMPARE( collection.attributes().size(), 1 );
  QCOMPARE( collection.count(), 1 );
  QCOMPARE( collection.at( 0 ).name(), u"at1"_s );
  QCOMPARE( collection.attributes().at( 0 ).name(), u"at1"_s );
  QCOMPARE( collection.pointRecordSize(), 4 );
  QVERIFY( !collection.find( u"test"_s, offset ) );
  QCOMPARE( collection.find( u"at1"_s, offset )->name(), u"at1"_s );
  QCOMPARE( collection.indexOf( "test"_L1 ), -1 );
  QCOMPARE( collection.indexOf( "at1"_L1 ), 0 );
  QCOMPARE( offset, 0 );

  collection.push_back( QgsPointCloudAttribute( u"at2"_s, QgsPointCloudAttribute::DataType::Short ) );
  QCOMPARE( collection.attributes().size(), 2 );
  QCOMPARE( collection.count(), 2 );
  QCOMPARE( collection.at( 0 ).name(), u"at1"_s );
  QCOMPARE( collection.at( 1 ).name(), u"at2"_s );
  QCOMPARE( collection.attributes().at( 0 ).name(), u"at1"_s );
  QCOMPARE( collection.attributes().at( 1 ).name(), u"at2"_s );
  QCOMPARE( collection.pointRecordSize(), 6 );
  QVERIFY( !collection.find( u"test"_s, offset ) );
  QCOMPARE( collection.find( u"at1"_s, offset )->name(), u"at1"_s );
  QCOMPARE( offset, 0 );
  QCOMPARE( collection.find( u"at2"_s, offset )->name(), u"at2"_s );
  QCOMPARE( offset, 4 );
  QCOMPARE( collection.indexOf( "test"_L1 ), -1 );
  QCOMPARE( collection.indexOf( "at1"_L1 ), 0 );
  QCOMPARE( collection.indexOf( "at2"_L1 ), 1 );

  collection.push_back( QgsPointCloudAttribute( u"at3"_s, QgsPointCloudAttribute::DataType::Double ) );
  QCOMPARE( collection.attributes().size(), 3 );
  QCOMPARE( collection.count(), 3 );
  QCOMPARE( collection.attributes().at( 0 ).name(), u"at1"_s );
  QCOMPARE( collection.attributes().at( 1 ).name(), u"at2"_s );
  QCOMPARE( collection.attributes().at( 2 ).name(), u"at3"_s );
  QCOMPARE( collection.pointRecordSize(), 14 );
  QVERIFY( !collection.find( u"test"_s, offset ) );
  QCOMPARE( collection.find( u"at1"_s, offset )->name(), u"at1"_s );
  QCOMPARE( offset, 0 );
  QCOMPARE( collection.find( u"at2"_s, offset )->name(), u"at2"_s );
  QCOMPARE( offset, 4 );
  QCOMPARE( collection.find( u"at3"_s, offset )->name(), u"at3"_s );
  QCOMPARE( offset, 6 );

  // populate from other attributes
  const QgsPointCloudAttributeCollection collection2( QVector<QgsPointCloudAttribute>() << QgsPointCloudAttribute( u"at1"_s, QgsPointCloudAttribute::DataType::Float ) << QgsPointCloudAttribute( u"at2"_s, QgsPointCloudAttribute::DataType::Short ) << QgsPointCloudAttribute( u"at3"_s, QgsPointCloudAttribute::DataType::Double ) );
  QCOMPARE( collection2.attributes().size(), 3 );
  QCOMPARE( collection2.count(), 3 );
  QCOMPARE( collection2.attributes().at( 0 ).name(), u"at1"_s );
  QCOMPARE( collection2.attributes().at( 1 ).name(), u"at2"_s );
  QCOMPARE( collection2.attributes().at( 2 ).name(), u"at3"_s );
  QCOMPARE( collection2.pointRecordSize(), 14 );
  QVERIFY( !collection2.find( u"test"_s, offset ) );
  QCOMPARE( collection2.find( u"at1"_s, offset )->name(), u"at1"_s );
  QCOMPARE( offset, 0 );
  QCOMPARE( collection2.find( u"at2"_s, offset )->name(), u"at2"_s );
  QCOMPARE( offset, 4 );
  QCOMPARE( collection2.find( u"at3"_s, offset )->name(), u"at3"_s );
  QCOMPARE( offset, 6 );
}

void TestQgsPointCloudAttribute::testCollectionFindCaseInsensitive()
{
  int offset = 0;
  const QgsPointCloudAttributeCollection collection( QVector<QgsPointCloudAttribute>() << QgsPointCloudAttribute( u"at1"_s, QgsPointCloudAttribute::DataType::Float ) << QgsPointCloudAttribute( u"at2"_s, QgsPointCloudAttribute::DataType::Short ) << QgsPointCloudAttribute( u"AT3"_s, QgsPointCloudAttribute::DataType::Double ) );
  QCOMPARE( collection.attributes().size(), 3 );
  QCOMPARE( collection.count(), 3 );
  QCOMPARE( collection.attributes().at( 0 ).name(), u"at1"_s );
  QCOMPARE( collection.attributes().at( 1 ).name(), u"at2"_s );
  QCOMPARE( collection.attributes().at( 2 ).name(), u"AT3"_s );
  QCOMPARE( collection.pointRecordSize(), 14 );
  QVERIFY( !collection.find( u"test"_s, offset ) );
  QCOMPARE( collection.find( u"At1"_s, offset )->name(), u"at1"_s );
  QCOMPARE( offset, 0 );
  QCOMPARE( collection.find( u"aT2"_s, offset )->name(), u"at2"_s );
  QCOMPARE( offset, 4 );
  QCOMPARE( collection.find( u"aT3"_s, offset )->name(), u"AT3"_s );
  QCOMPARE( offset, 6 );
}

void TestQgsPointCloudAttribute::testCollevtionExtend()
{
  int offset = 0;
  QgsPointCloudAttributeCollection collection( QVector<QgsPointCloudAttribute>() << QgsPointCloudAttribute( u"at1"_s, QgsPointCloudAttribute::DataType::Float ) << QgsPointCloudAttribute( u"at2"_s, QgsPointCloudAttribute::DataType::Short ) << QgsPointCloudAttribute( u"at3"_s, QgsPointCloudAttribute::DataType::Double ) );
  const QgsPointCloudAttributeCollection collection2( QVector<QgsPointCloudAttribute>() << QgsPointCloudAttribute( u"at1"_s, QgsPointCloudAttribute::DataType::Float ) << QgsPointCloudAttribute( u"at2"_s, QgsPointCloudAttribute::DataType::Short ) << QgsPointCloudAttribute( u"at3"_s, QgsPointCloudAttribute::DataType::Double ) << QgsPointCloudAttribute( u"at4"_s, QgsPointCloudAttribute::DataType::Float ) << QgsPointCloudAttribute( u"at5"_s, QgsPointCloudAttribute::DataType::Short ) );

  collection.extend( collection2, QSet<QString>() );
  QCOMPARE( collection.attributes().size(), 3 );
  QCOMPARE( collection.count(), 3 );
  QCOMPARE( collection.attributes().at( 0 ).name(), u"at1"_s );
  QCOMPARE( collection.attributes().at( 1 ).name(), u"at2"_s );
  QCOMPARE( collection.attributes().at( 2 ).name(), u"at3"_s );
  QCOMPARE( collection.pointRecordSize(), 14 );
  QVERIFY( !collection.find( u"test"_s, offset ) );
  QCOMPARE( collection.find( u"at1"_s, offset )->name(), u"at1"_s );
  QCOMPARE( offset, 0 );
  QCOMPARE( collection.find( u"at2"_s, offset )->name(), u"at2"_s );
  QCOMPARE( offset, 4 );
  QCOMPARE( collection.find( u"at3"_s, offset )->name(), u"at3"_s );
  QCOMPARE( offset, 6 );

  collection.extend( collection2, QSet<QString>() << u"at4"_s );
  QCOMPARE( collection.attributes().size(), 4 );
  QCOMPARE( collection.count(), 4 );
  QVERIFY( !collection.find( u"at5"_s, offset ) );
  QCOMPARE( collection.find( u"at1"_s, offset )->name(), u"at1"_s );
  QCOMPARE( offset, 0 );
  QCOMPARE( collection.find( u"at2"_s, offset )->name(), u"at2"_s );
  QCOMPARE( offset, 4 );
  QCOMPARE( collection.find( u"at3"_s, offset )->name(), u"at3"_s );
  QCOMPARE( offset, 6 );
  QCOMPARE( collection.find( u"at4"_s, offset )->name(), u"at4"_s );
  QCOMPARE( offset, 14 );

  collection.extend( collection2, QSet<QString>() << u"at4"_s << u"at5"_s << u"at6"_s );
  QCOMPARE( collection.attributes().size(), 5 );
  QCOMPARE( collection.count(), 5 );
  QVERIFY( !collection.find( u"at6"_s, offset ) );
  QCOMPARE( collection.find( u"at1"_s, offset )->name(), u"at1"_s );
  QCOMPARE( offset, 0 );
  QCOMPARE( collection.find( u"at2"_s, offset )->name(), u"at2"_s );
  QCOMPARE( offset, 4 );
  QCOMPARE( collection.find( u"at3"_s, offset )->name(), u"at3"_s );
  QCOMPARE( offset, 6 );
  QCOMPARE( collection.find( u"at4"_s, offset )->name(), u"at4"_s );
  QCOMPARE( collection.find( u"at5"_s, offset )->name(), u"at5"_s );
}

void TestQgsPointCloudAttribute::testToFields()
{
  QgsFields fields = QgsPointCloudAttributeCollection().toFields();
  QCOMPARE( fields.size(), 0 );

  const QgsPointCloudAttributeCollection collection( QVector<QgsPointCloudAttribute>() << QgsPointCloudAttribute( u"at1"_s, QgsPointCloudAttribute::DataType::Float ) << QgsPointCloudAttribute( u"at2"_s, QgsPointCloudAttribute::DataType::Short ) << QgsPointCloudAttribute( u"at3"_s, QgsPointCloudAttribute::DataType::Double ) );
  fields = collection.toFields();
  QCOMPARE( fields.size(), 3 );

  QCOMPARE( fields.at( 0 ).name(), u"at1"_s );
  QCOMPARE( fields.at( 0 ).type(), QMetaType::Type::Double );
  QCOMPARE( fields.at( 1 ).name(), u"at2"_s );
  QCOMPARE( fields.at( 1 ).type(), QMetaType::Type::Int );
  QCOMPARE( fields.at( 2 ).name(), u"at3"_s );
  QCOMPARE( fields.at( 2 ).type(), QMetaType::Type::Double );
}

QGSTEST_MAIN( TestQgsPointCloudAttribute )
#include "testqgspointcloudattribute.moc"
