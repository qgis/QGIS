/***************************************************************************
                      testqgspointcloudexpression.cpp
                      -------------------------------
    begin                : January 2022
    copyright            : (C) 2022 Stefanos Natsis
    email                : uclaros at gmail dot com
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
#include <QVector>

#include <ogr_api.h>
#include "cpl_conv.h"
#include "cpl_string.h"

#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsogrutils.h"
#include "qgsapplication.h"
#include "qgspoint.h"
#include "qgspointcloudattribute.h"
#include "qgspointcloudblock.h"
#include "qgspointcloudexpression.h"



template <typename T>
bool _storeToStream( char *s, size_t position, QgsPointCloudAttribute::DataType type, T value )
{
  switch ( type )
  {
    case QgsPointCloudAttribute::Char:
    {
      const char val = char( value );
      s[position] = val;
      break;
    }
    case QgsPointCloudAttribute::Short:
    {
      short val = short( value );
      memcpy( s + position, reinterpret_cast<char * >( &val ), sizeof( short ) );
      break;
    }

    case QgsPointCloudAttribute::UShort:
    {
      unsigned short val = static_cast< unsigned short>( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ), sizeof( unsigned short ) );
      break;
    }

    case QgsPointCloudAttribute::Float:
    {
      float val = float( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ),  sizeof( float ) );
      break;
    }
    case QgsPointCloudAttribute::Int32:
    {
      qint32 val = qint32( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ), sizeof( qint32 ) );
      break;
    }
    case QgsPointCloudAttribute::Double:
    {
      double val = double( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ), sizeof( double ) );
      break;
    }
  }

  return true;
}



class TestQgsPointCloudExpression: public QObject
{
    Q_OBJECT

  private slots:
    QgsPointCloudBlock *createPointCloudBlock( const QVector<QVariantMap> &points, const QgsVector3D &scale, const QgsVector3D &offset, const QgsPointCloudAttributeCollection &attributes );
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void testCreateBlock();
    void testParsing_data();
    void testParsing();

  private:
    QString mTestDataDir;
};

QgsPointCloudBlock *TestQgsPointCloudExpression::createPointCloudBlock( const QVector<QVariantMap> &points, const QgsVector3D &scale, const QgsVector3D &offset, const QgsPointCloudAttributeCollection &attributes )
{
  const int pointRecordSize = attributes.pointRecordSize();
  QByteArray data;
  data.resize( points.size() * pointRecordSize );
  data.fill( 0 );
  char *dataBuffer = data.data();
  int dataOffset = 0;
  int count = 0;

  QVector< QLatin1String > allAttributes;
  allAttributes << QLatin1String( "X" )
                << QLatin1String( "Y" )
                << QLatin1String( "Z" )
                << QLatin1String( "Classification" )
                << QLatin1String( "Intensity" )
                << QLatin1String( "ReturnNumber" )
                << QLatin1String( "NumberOfReturns" )
                << QLatin1String( "ScanDirectionFlag" )
                << QLatin1String( "EdgeOfFlightLine" )
                << QLatin1String( "ScanAngleRank" )
                << QLatin1String( "UserData" )
                << QLatin1String( "PointSourceId" )
                << QLatin1String( "GpsTime" )
                << QLatin1String( "Red" )
                << QLatin1String( "Green" )
                << QLatin1String( "Blue" );

  const QVector< QgsPointCloudAttribute > attributesVector = attributes.attributes();
  for ( const auto &point : points )
  {
    for ( const auto &attribute : std::as_const( attributesVector ) )
    {
      if ( attribute.name().compare( QLatin1String( "X" ), Qt::CaseInsensitive ) == 0 )
        _storeToStream<qint32>( dataBuffer, dataOffset, attribute.type(), ( point[ attribute.name() ].toDouble() - offset.x() ) / scale.x() );
      else if ( attribute.name().compare( QLatin1String( "Y" ), Qt::CaseInsensitive ) == 0 )
        _storeToStream<qint32>( dataBuffer, dataOffset, attribute.type(), ( point[ attribute.name() ].toDouble() - offset.y() ) / scale.y() );
      else if ( attribute.name().compare( QLatin1String( "Z" ), Qt::CaseInsensitive ) == 0 )
        _storeToStream<qint32>( dataBuffer, dataOffset, attribute.type(), ( point[ attribute.name() ].toDouble() - offset.z() ) / scale.z() );
      else if ( attribute.name().compare( QLatin1String( "Classification" ), Qt::CaseInsensitive ) == 0 )
        _storeToStream<unsigned char>( dataBuffer, dataOffset, attribute.type(), point[ attribute.name() ].toInt() );
      else if ( attribute.name().compare( QLatin1String( "Intensity" ), Qt::CaseInsensitive ) == 0 )
        _storeToStream<unsigned short>( dataBuffer, dataOffset, attribute.type(), point[ attribute.name() ].toInt() );
      else if ( attribute.name().compare( QLatin1String( "ReturnNumber" ), Qt::CaseInsensitive ) == 0 )
        _storeToStream<unsigned char>( dataBuffer, dataOffset, attribute.type(), point[ attribute.name() ].toInt() );
      else if ( attribute.name().compare( QLatin1String( "NumberOfReturns" ), Qt::CaseInsensitive ) == 0 )
        _storeToStream<unsigned char>( dataBuffer, dataOffset, attribute.type(), point[ attribute.name() ].toInt() );
      else if ( attribute.name().compare( QLatin1String( "ScanDirectionFlag" ), Qt::CaseInsensitive ) == 0 )
        _storeToStream<unsigned char>( dataBuffer, dataOffset, attribute.type(), point[ attribute.name() ].toInt() );
      else if ( attribute.name().compare( QLatin1String( "EdgeOfFlightLine" ), Qt::CaseInsensitive ) == 0 )
        _storeToStream<unsigned char>( dataBuffer, dataOffset, attribute.type(), point[ attribute.name() ].toInt() );
      else if ( attribute.name().compare( QLatin1String( "ScanAngleRank" ), Qt::CaseInsensitive ) == 0 )
        _storeToStream<char>( dataBuffer, dataOffset, attribute.type(), point[ attribute.name() ].toFloat() );
      else if ( attribute.name().compare( QLatin1String( "UserData" ), Qt::CaseInsensitive ) == 0 )
        _storeToStream<unsigned char>( dataBuffer, dataOffset, attribute.type(), point[ attribute.name() ].toInt() );
      else if ( attribute.name().compare( QLatin1String( "PointSourceId" ), Qt::CaseInsensitive ) == 0 )
        _storeToStream<unsigned short>( dataBuffer, dataOffset, attribute.type(), point[ attribute.name() ].toUInt() );
      else if ( attribute.name().compare( QLatin1String( "GpsTime" ), Qt::CaseInsensitive ) == 0 )
        _storeToStream<double>( dataBuffer, dataOffset, attribute.type(), point[ attribute.name() ].toDouble() );
      else if ( attribute.name().compare( QLatin1String( "Red" ), Qt::CaseInsensitive ) == 0 )
        _storeToStream<unsigned short>( dataBuffer, dataOffset, attribute.type(), point[ attribute.name() ].toUInt() );
      else if ( attribute.name().compare( QLatin1String( "Green" ), Qt::CaseInsensitive ) == 0 )
        _storeToStream<unsigned short>( dataBuffer, dataOffset, attribute.type(), point[ attribute.name() ].toUInt() );
      else if ( attribute.name().compare( QLatin1String( "Blue" ), Qt::CaseInsensitive ) == 0 )
        _storeToStream<unsigned short>( dataBuffer, dataOffset, attribute.type(), point[ attribute.name() ].toUInt() );
      else
        _storeToStream<unsigned short>( dataBuffer, dataOffset, attribute.type(), point[ attribute.name() ].toUInt() );
      dataOffset += attribute.size();
    }
    ++count;
  }
  QgsPointCloudBlock *block = new QgsPointCloudBlock( count, attributes, data, scale, offset );
  return block;
}



void TestQgsPointCloudExpression::initTestCase()
{
  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::registerOgrDrivers();
}

void TestQgsPointCloudExpression::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsPointCloudExpression::init()
{

}

void TestQgsPointCloudExpression::cleanup()
{

}

void TestQgsPointCloudExpression::testCreateBlock()
{
  // Create all attributes
  QgsPointCloudAttributeCollection attributes;
  attributes.push_back( QgsPointCloudAttribute( QLatin1String( "X" ), QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( QLatin1String( "Y" ), QgsPointCloudAttribute::Int32 ) );
  attributes.push_back( QgsPointCloudAttribute( QLatin1String( "Z" ), QgsPointCloudAttribute::Double ) );
  attributes.push_back( QgsPointCloudAttribute( QLatin1String( "Classification" ), QgsPointCloudAttribute::Char ) );
  attributes.push_back( QgsPointCloudAttribute( QLatin1String( "Intensity" ), QgsPointCloudAttribute::UShort ) );
  attributes.push_back( QgsPointCloudAttribute( QLatin1String( "ReturnNumber" ), QgsPointCloudAttribute::Char ) );
  attributes.push_back( QgsPointCloudAttribute( QLatin1String( "NumberOfReturns" ), QgsPointCloudAttribute::Char ) );
  attributes.push_back( QgsPointCloudAttribute( QLatin1String( "ScanDirectionFlag" ), QgsPointCloudAttribute::Char ) );
  attributes.push_back( QgsPointCloudAttribute( QLatin1String( "EdgeOfFlightLine" ), QgsPointCloudAttribute::Char ) );
  attributes.push_back( QgsPointCloudAttribute( QLatin1String( "ScanAngleRank" ), QgsPointCloudAttribute::Float ) );
  attributes.push_back( QgsPointCloudAttribute( QLatin1String( "UserData" ), QgsPointCloudAttribute::Char ) );
  attributes.push_back( QgsPointCloudAttribute( QLatin1String( "PointSourceId" ), QgsPointCloudAttribute::UShort ) );
  attributes.push_back( QgsPointCloudAttribute( QLatin1String( "GpsTime" ), QgsPointCloudAttribute::Double ) );
  attributes.push_back( QgsPointCloudAttribute( QLatin1String( "Red" ), QgsPointCloudAttribute::UShort ) );
  attributes.push_back( QgsPointCloudAttribute( QLatin1String( "Green" ), QgsPointCloudAttribute::UShort ) );
  attributes.push_back( QgsPointCloudAttribute( QLatin1String( "Blue" ), QgsPointCloudAttribute::UShort ) );

  // Generate some points with predefined data
  QVariantMap point;
  point[ QLatin1String( "X" ) ] = 1001.1;
  point[ QLatin1String( "Y" ) ] = 2002.2;
  point[ QLatin1String( "Z" ) ] = 3003.3;
  point[ QLatin1String( "Classification" ) ] = 1;
  point[ QLatin1String( "Intensity" ) ] = 2;
  point[ QLatin1String( "ReturnNumber" ) ] = 3;
  point[ QLatin1String( "NumberOfReturns" ) ] = 4;
  point[ QLatin1String( "ScanDirectionFlag" ) ] = 5;
  point[ QLatin1String( "EdgeOfFlightLine" ) ] = 6;
  point[ QLatin1String( "ScanAngleRank" ) ] = 7;
  point[ QLatin1String( "UserData" ) ] = 8;
  point[ QLatin1String( "PointSourceId" ) ] = 9;
  point[ QLatin1String( "GpsTime" ) ] = 10;
  point[ QLatin1String( "Red" ) ] = 11;
  point[ QLatin1String( "Green" ) ] = 12;
  point[ QLatin1String( "Blue" ) ] = 13;

  QVector<QVariantMap> points;
  points << point << point << point;

  // Also define scale and offset for x/y/z in the block
  QgsVector3D scale( 0.01, 0.01, 0.01 );
  QgsVector3D offset( 1000, 2000, 3000 );

  QgsPointCloudBlock *block = createPointCloudBlock( points, scale, offset, attributes );

  // Check that the block has the correct data
  QVariantMap map = QgsPointCloudAttribute::getAttributeMap( block->data(), 0, attributes );

  QCOMPARE( map[ "X" ].toDouble() * scale.x() + offset.x(), point[ "X" ] );
  QCOMPARE( map[ "Y" ].toDouble() * scale.y() + offset.y(), point[ "Y" ] );
  QCOMPARE( map[ "Z" ].toDouble() * scale.z() + offset.z(), point[ "Z" ] );
  QCOMPARE( map[ "Classification" ], point[ "Classification" ] );
  QCOMPARE( map[ "Intensity" ], point[ "Intensity" ] );
  QCOMPARE( map[ "ReturnNumber" ], point[ "ReturnNumber" ] );
  QCOMPARE( map[ "NumberOfReturns" ], point[ "NumberOfReturns" ] );
  QCOMPARE( map[ "ScanDirectionFlag" ], point[ "ScanDirectionFlag" ] );
  QCOMPARE( map[ "EdgeOfFlightLine" ], point[ "EdgeOfFlightLine" ] );
  QCOMPARE( map[ "ScanAngleRank" ], point[ "ScanAngleRank" ] );
  QCOMPARE( map[ "UserData" ], point[ "UserData" ] );
  QCOMPARE( map[ "PointSourceId" ], point[ "PointSourceId" ] );
  QCOMPARE( map[ "GpsTime" ], point[ "GpsTime" ] );
  QCOMPARE( map[ "Red" ], point[ "Red" ] );
  QCOMPARE( map[ "Green" ], point[ "Green" ] );
  QCOMPARE( map[ "Blue" ], point[ "Blue" ] );

  delete block;
}

void TestQgsPointCloudExpression::testParsing_data()
{
  QTest::addColumn<QString>( "string" );
  QTest::addColumn<bool>( "valid" );

  // invalid strings
  QTest::newRow( "empty string" ) << "" << false;
  QTest::newRow( "invalid character" ) << "@" << false;
  QTest::newRow( "missing operator" ) << "my col" << false;
  QTest::newRow( "string literal" ) << "'test'" << false;
  QTest::newRow( "null" ) << "NULL" << false;
  QTest::newRow( "invalid binary operator" ) << "1+" << false;
  QTest::newRow( "invalid function not known no args" ) << "watwat()" << false;
  QTest::newRow( "invalid function not known" ) << "coz(1)" << false;
  QTest::newRow( "invalid operator IN" ) << "n in p" << false;
  QTest::newRow( "empty node list" ) << "1 in ()" << false;

  // valid strings
  QTest::newRow( "int literal" ) << "1" << true;
  QTest::newRow( "float literal" ) << "1.23" << true;
  QTest::newRow( "attribute reference" ) << "Classification" << true;
  QTest::newRow( "quoted attribute" ) << "\"Classification\"" << true;
  QTest::newRow( "unary minus" ) << "-(-3)" << true;
  QTest::newRow( "operator IN" ) << "n in (a,b,c)" << true;
  QTest::newRow( "pow" ) << "2 ^ 8" << true;
  QTest::newRow( "arithmetic" ) << "1+2*3" << true;
  QTest::newRow( "logic" ) << "be or not be" << true;

}

void TestQgsPointCloudExpression::testParsing()
{
  QFETCH( QString, string );
  QFETCH( bool, valid );

  QgsPointcloudExpression exp( string );

  if ( exp.hasParserError() )
    qDebug() << "Parser error: " << exp.parserErrorString();
  else
    qDebug() << "Parsed string: " << exp.expression();

  QCOMPARE( !exp.hasParserError(), valid );
}


QGSTEST_MAIN( TestQgsPointCloudExpression )
#include "testqgspointcloudexpression.moc"
