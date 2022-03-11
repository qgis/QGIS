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

#include <QObject>
#include <QString>
#include <QVector>
#include <QByteArray>

#include "qgstest.h"
#include "qgsapplication.h"
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
    case QgsPointCloudAttribute::UChar:
    {
      const unsigned char val = ( unsigned char )( value );
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

    case QgsPointCloudAttribute::Int32:
    {
      qint32 val = qint32( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ), sizeof( qint32 ) );
      break;
    }
    case QgsPointCloudAttribute::UInt32:
    {
      quint32 val = quint32( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ), sizeof( quint32 ) );
      break;
    }

    case QgsPointCloudAttribute::Int64:
    {
      qint64 val = qint64( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ), sizeof( qint64 ) );
      break;
    }
    case QgsPointCloudAttribute::UInt64:
    {
      quint64 val = quint64( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ), sizeof( quint64 ) );
      break;
    }

    case QgsPointCloudAttribute::Float:
    {
      float val = float( value );
      memcpy( s + position, reinterpret_cast< char * >( &val ),  sizeof( float ) );
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
    void testEvaluating_data();
    void testEvaluating();
    void testBlockResize();

  private:
    QString mTestDataDir;
    QVector<QVariantMap> mPoints;
    QgsPointCloudBlock *mBlock = nullptr;;
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

  const QVector< QgsPointCloudAttribute > attributesVector = attributes.attributes();
  for ( const auto &point : points )
  {
    for ( const auto &attribute : std::as_const( attributesVector ) )
    {
      if ( attribute.name().compare( QLatin1String( "X" ), Qt::CaseInsensitive ) == 0 )
        _storeToStream<qint32>( dataBuffer, dataOffset, attribute.type(), std::round( ( point[ attribute.name() ].toDouble() - offset.x() ) / scale.x() ) );
      else if ( attribute.name().compare( QLatin1String( "Y" ), Qt::CaseInsensitive ) == 0 )
        _storeToStream<qint32>( dataBuffer, dataOffset, attribute.type(), std::round( ( point[ attribute.name() ].toDouble() - offset.y() ) / scale.y() ) );
      else if ( attribute.name().compare( QLatin1String( "Z" ), Qt::CaseInsensitive ) == 0 )
        _storeToStream<qint32>( dataBuffer, dataOffset, attribute.type(), std::round( ( point[ attribute.name() ].toDouble() - offset.z() ) / scale.z() ) );
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
  QVariantMap p0, p1, p2, p3, p4;
  p0[ QLatin1String( "X" ) ] = 1001.1;
  p0[ QLatin1String( "Y" ) ] = 2002.2;
  p0[ QLatin1String( "Z" ) ] = 3003.3;
  p0[ QLatin1String( "Classification" ) ] = 1;
  p0[ QLatin1String( "Intensity" ) ] = 2;
  p0[ QLatin1String( "ReturnNumber" ) ] = 3;
  p0[ QLatin1String( "NumberOfReturns" ) ] = 4;
  p0[ QLatin1String( "ScanDirectionFlag" ) ] = 5;
  p0[ QLatin1String( "EdgeOfFlightLine" ) ] = 6;
  p0[ QLatin1String( "ScanAngleRank" ) ] = 7;
  p0[ QLatin1String( "UserData" ) ] = 8;
  p0[ QLatin1String( "PointSourceId" ) ] = 9;
  p0[ QLatin1String( "GpsTime" ) ] = 10;
  p0[ QLatin1String( "Red" ) ] = 11;
  p0[ QLatin1String( "Green" ) ] = 12;
  p0[ QLatin1String( "Blue" ) ] = 13;

  p1[ QLatin1String( "X" ) ] = 1002.2;
  p1[ QLatin1String( "Y" ) ] = 2003.3;
  p1[ QLatin1String( "Z" ) ] = 3004.4;
  p1[ QLatin1String( "Classification" ) ] = 2;
  p1[ QLatin1String( "Intensity" ) ] = 3;
  p1[ QLatin1String( "ReturnNumber" ) ] = 4;
  p1[ QLatin1String( "NumberOfReturns" ) ] = 5;
  p1[ QLatin1String( "ScanDirectionFlag" ) ] = 6;
  p1[ QLatin1String( "EdgeOfFlightLine" ) ] = 7;
  p1[ QLatin1String( "ScanAngleRank" ) ] = 8;
  p1[ QLatin1String( "UserData" ) ] = 9;
  p1[ QLatin1String( "PointSourceId" ) ] = 10;
  p1[ QLatin1String( "GpsTime" ) ] = 11;
  p1[ QLatin1String( "Red" ) ] = 12;
  p1[ QLatin1String( "Green" ) ] = 13;
  p1[ QLatin1String( "Blue" ) ] = 14;

  p2[ QLatin1String( "X" ) ] = 1003.3;
  p2[ QLatin1String( "Y" ) ] = 2004.4;
  p2[ QLatin1String( "Z" ) ] = 3005.5;
  p2[ QLatin1String( "Classification" ) ] = 3;
  p2[ QLatin1String( "Intensity" ) ] = 4;
  p2[ QLatin1String( "ReturnNumber" ) ] = 5;
  p2[ QLatin1String( "NumberOfReturns" ) ] = 6;
  p2[ QLatin1String( "ScanDirectionFlag" ) ] = 7;
  p2[ QLatin1String( "EdgeOfFlightLine" ) ] = 8;
  p2[ QLatin1String( "ScanAngleRank" ) ] = 9;
  p2[ QLatin1String( "UserData" ) ] = 10;
  p2[ QLatin1String( "PointSourceId" ) ] = 11;
  p2[ QLatin1String( "GpsTime" ) ] = 12;
  p2[ QLatin1String( "Red" ) ] = 13;
  p2[ QLatin1String( "Green" ) ] = 14;
  p2[ QLatin1String( "Blue" ) ] = 15;

  p3[ QLatin1String( "X" ) ] = 1004.4;
  p3[ QLatin1String( "Y" ) ] = 2005.5;
  p3[ QLatin1String( "Z" ) ] = 3006.6;
  p3[ QLatin1String( "Classification" ) ] = 4;
  p3[ QLatin1String( "Intensity" ) ] = 4;
  p3[ QLatin1String( "ReturnNumber" ) ] = 6;
  p3[ QLatin1String( "NumberOfReturns" ) ] = 6;
  p3[ QLatin1String( "ScanDirectionFlag" ) ] = 7;
  p3[ QLatin1String( "EdgeOfFlightLine" ) ] = 8;
  p3[ QLatin1String( "ScanAngleRank" ) ] = 9;
  p3[ QLatin1String( "UserData" ) ] = 10;
  p3[ QLatin1String( "PointSourceId" ) ] = 11;
  p3[ QLatin1String( "GpsTime" ) ] = 12;
  p3[ QLatin1String( "Red" ) ] = 13;
  p3[ QLatin1String( "Green" ) ] = 14;
  p3[ QLatin1String( "Blue" ) ] = 15;

  p4[ QLatin1String( "X" ) ] = 1005.5;
  p4[ QLatin1String( "Y" ) ] = 2006.6;
  p4[ QLatin1String( "Z" ) ] = 3007.7;
  p4[ QLatin1String( "Classification" ) ] = 1;
  p4[ QLatin1String( "Intensity" ) ] = 4;
  p4[ QLatin1String( "ReturnNumber" ) ] = 7;
  p4[ QLatin1String( "NumberOfReturns" ) ] = 7;
  p4[ QLatin1String( "ScanDirectionFlag" ) ] = 7;
  p4[ QLatin1String( "EdgeOfFlightLine" ) ] = 8;
  p4[ QLatin1String( "ScanAngleRank" ) ] = 9;
  p4[ QLatin1String( "UserData" ) ] = 10;
  p4[ QLatin1String( "PointSourceId" ) ] = 11;
  p4[ QLatin1String( "GpsTime" ) ] = 12;
  p4[ QLatin1String( "Red" ) ] = 13;
  p4[ QLatin1String( "Green" ) ] = 14;
  p4[ QLatin1String( "Blue" ) ] = 15;
  mPoints << p0 << p1 << p2 << p3 << p4;

  // Also define scale and offset for x/y/z in the block
  QgsVector3D scale( 0.01, 0.01, 0.01 );
  QgsVector3D offset( 1000, 2000, 3000 );

  mBlock = createPointCloudBlock( mPoints, scale, offset, attributes );
}

void TestQgsPointCloudExpression::cleanupTestCase()
{
  delete mBlock;
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
  const QgsVector3D scale = mBlock->scale();
  const QgsVector3D offset = mBlock->offset();

  // Check that the block has the correct data
  QVariantMap map;
  int i = 0;
  for ( const auto &p : mPoints )
  {
    map = QgsPointCloudAttribute::getAttributeMap( mBlock->data(),
          i * mBlock->attributes().pointRecordSize(),
          mBlock->attributes() );

    QCOMPARE( map[ "X" ].toDouble() * scale.x() + offset.x(), p[ "X" ] );
    QCOMPARE( map[ "Y" ].toDouble() * scale.y() + offset.y(), p[ "Y" ] );
    QCOMPARE( map[ "Z" ].toDouble() * scale.z() + offset.z(), p[ "Z" ] );
    QCOMPARE( map[ "Classification" ], p[ "Classification" ] );
    QCOMPARE( map[ "Intensity" ], p[ "Intensity" ] );
    QCOMPARE( map[ "ReturnNumber" ], p[ "ReturnNumber" ] );
    QCOMPARE( map[ "NumberOfReturns" ], p[ "NumberOfReturns" ] );
    QCOMPARE( map[ "ScanDirectionFlag" ], p[ "ScanDirectionFlag" ] );
    QCOMPARE( map[ "EdgeOfFlightLine" ], p[ "EdgeOfFlightLine" ] );
    QCOMPARE( map[ "ScanAngleRank" ], p[ "ScanAngleRank" ] );
    QCOMPARE( map[ "UserData" ], p[ "UserData" ] );
    QCOMPARE( map[ "PointSourceId" ], p[ "PointSourceId" ] );
    QCOMPARE( map[ "GpsTime" ], p[ "GpsTime" ] );
    QCOMPARE( map[ "Red" ], p[ "Red" ] );
    QCOMPARE( map[ "Green" ], p[ "Green" ] );
    QCOMPARE( map[ "Blue" ], p[ "Blue" ] );
    ++i;
  }
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
  QTest::newRow( "unknown attribute" ) << "Jujufication" << true;
  QTest::newRow( "unknown quoted attribute" ) << "\"Jujufication\"" << true;
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

  QgsPointCloudExpression exp( string );

  if ( exp.hasParserError() )
    qDebug() << "Parser error: " << exp.parserErrorString();
  else
    qDebug() << "Parsed string: " << exp.expression();

  QCOMPARE( !exp.hasParserError(), valid );
}

void TestQgsPointCloudExpression::testEvaluating_data()
{
  QTest::addColumn<QString>( "string" );
  QTest::addColumn<int>( "point_n" );
  QTest::addColumn<bool>( "valid" );

  QTest::newRow( "geom eq" ) << "X = 1001.1" << 0 << true;
  QTest::newRow( "geom eq" ) << "X = 1001.1" << 1 << false;
  QTest::newRow( "geom eq" ) << "X = 1001.1" << 2 << false;
  QTest::newRow( "geom eq" ) << "X = 1001.1" << 3 << false;
  QTest::newRow( "geom eq" ) << "X = 1001.1" << 4 << false;

  QTest::newRow( "single attribute eq" ) << "Classification = 1" << 0 << true;
  QTest::newRow( "single attribute eq" ) << "Classification = 1" << 1 << false;
  QTest::newRow( "single attribute eq" ) << "Classification = 1" << 2 << false;
  QTest::newRow( "single attribute eq" ) << "Classification = 1" << 3 << false;
  QTest::newRow( "single attribute eq" ) << "Classification = 1" << 4 << true;

  QTest::newRow( "single attribute lt" ) << "Z < 3004.5" << 0 << true;
  QTest::newRow( "single attribute lt" ) << "Z < 3004.5" << 1 << true;
  QTest::newRow( "single attribute lt" ) << "Z < 3004.5" << 2 << false;
  QTest::newRow( "single attribute lt" ) << "Z < 3004.5" << 3 << false;
  QTest::newRow( "single attribute lt" ) << "Z < 3004.5" << 4 << false;

  QTest::newRow( "single attribute gt" ) << "Z > 3004.5" << 0 << false;
  QTest::newRow( "single attribute gt" ) << "Z > 3004.5" << 1 << false;
  QTest::newRow( "single attribute gt" ) << "Z > 3004.5" << 2 << true;
  QTest::newRow( "single attribute gt" ) << "Z > 3004.5" << 3 << true;
  QTest::newRow( "single attribute gt" ) << "Z > 3004.5" << 4 << true;

  QTest::newRow( "single attribute lteq" ) << "Y <= 2005" << 0 << true;
  QTest::newRow( "single attribute lteq" ) << "Y <= 2005" << 1 << true;
  QTest::newRow( "single attribute lteq" ) << "Y <= 2005" << 2 << true;
  QTest::newRow( "single attribute lteq" ) << "Y <= 2005" << 3 << false;
  QTest::newRow( "single attribute lteq" ) << "Y <= 2005" << 4 << false;

  QTest::newRow( "single attribute gteq" ) << "Y >= 2005" << 0 << false;
  QTest::newRow( "single attribute gteq" ) << "Y >= 2005" << 1 << false;
  QTest::newRow( "single attribute gteq" ) << "Y >= 2005" << 2 << false;
  QTest::newRow( "single attribute gteq" ) << "Y >= 2005" << 3 << true;
  QTest::newRow( "single attribute gteq" ) << "Y >= 2005" << 4 << true;

  QTest::newRow( "single attribute neq" ) << "Classification != 1" << 0 << false;
  QTest::newRow( "single attribute neq" ) << "Classification != 1" << 1 << true;
  QTest::newRow( "single attribute neq" ) << "Classification != 1" << 2 << true;
  QTest::newRow( "single attribute neq" ) << "Classification != 1" << 3 << true;
  QTest::newRow( "single attribute neq" ) << "Classification != 1" << 4 << false;

  QTest::newRow( "single attribute in()" ) << "Classification in (1, 3, 5)" << 0 << true;
  QTest::newRow( "single attribute in()" ) << "Classification in (1, 3, 5)" << 1 << false;
  QTest::newRow( "single attribute in()" ) << "Classification in (1, 3, 5)" << 2 << true;
  QTest::newRow( "single attribute in()" ) << "Classification in (1, 3, 5)" << 3 << false;
  QTest::newRow( "single attribute in()" ) << "Classification in (1, 3, 5)" << 4 << true;

  QTest::newRow( "single attribute not in()" ) << "Classification not in (1, 3, 5)" << 0 << false;
  QTest::newRow( "single attribute not in()" ) << "Classification not in (1, 3, 5)" << 1 << true;
  QTest::newRow( "single attribute not in()" ) << "Classification not in (1, 3, 5)" << 2 << false;
  QTest::newRow( "single attribute not in()" ) << "Classification not in (1, 3, 5)" << 3 << true;
  QTest::newRow( "single attribute not in()" ) << "Classification not in (1, 3, 5)" << 4 << false;

  QTest::newRow( "single attribute arithmetic" ) << "Z > ( 2000 + 20^3 ) * 0.4 / 2 + 1005" << 0 << false;
  QTest::newRow( "single attribute arithmetic" ) << "Z > ( 2000 + 20^3 ) * 0.4 / 2 + 1005" << 1 << false;
  QTest::newRow( "single attribute arithmetic" ) << "Z > ( 2000 + 20^3 ) * 0.4 / 2 + 1005" << 2 << true;
  QTest::newRow( "single attribute arithmetic" ) << "Z > ( 2000 + 20^3 ) * 0.4 / 2 + 1005" << 3 << true;
  QTest::newRow( "single attribute arithmetic" ) << "Z > ( 2000 + 20^3 ) * 0.4 / 2 + 1005" << 4 << true;

  QTest::newRow( "multiple attributes AND" ) << "Classification = 1 and X < 1002" << 0 << true;
  QTest::newRow( "multiple attributes AND" ) << "Classification = 1 and X < 1002" << 1 << false;
  QTest::newRow( "multiple attributes AND" ) << "Classification = 1 and X < 1002" << 2 << false;
  QTest::newRow( "multiple attributes AND" ) << "Classification = 1 and X < 1002" << 3 << false;
  QTest::newRow( "multiple attributes AND" ) << "Classification = 1 and X < 1002" << 4 << false;

  QTest::newRow( "multiple attributes OR" ) << "Classification = 1 or ReturnNumber = 4" << 0 << true;
  QTest::newRow( "multiple attributes OR" ) << "Classification = 1 or ReturnNumber = 4" << 1 << true;
  QTest::newRow( "multiple attributes OR" ) << "Classification = 1 or ReturnNumber = 4" << 2 << false;
  QTest::newRow( "multiple attributes OR" ) << "Classification = 1 or ReturnNumber = 4" << 3 << false;
  QTest::newRow( "multiple attributes OR" ) << "Classification = 1 or ReturnNumber = 4" << 4 << true;

  QTest::newRow( "multiple attributes NOT" ) << "not Classification = 1" << 0 << false;
  QTest::newRow( "multiple attributes NOT" ) << "not Classification = 1" << 1 << true;
  QTest::newRow( "multiple attributes NOT" ) << "not Classification = 1" << 2 << true;
  QTest::newRow( "multiple attributes NOT" ) << "not Classification = 1" << 3 << true;
  QTest::newRow( "multiple attributes NOT" ) << "not Classification = 1" << 4 << false;

  QTest::newRow( "multiple attributes AND/OR/NOT" ) << "Classification = 1 and X < 1005 or Z = 3004.4" << 0 << true;
  QTest::newRow( "multiple attributes AND/OR/NOT" ) << "Classification = 1 and X < 1005 or Z = 3004.4" << 1 << true;
  QTest::newRow( "multiple attributes AND/OR/NOT" ) << "Classification = 1 and X < 1005 or Z = 3004.4" << 2 << false;
  QTest::newRow( "multiple attributes AND/OR/NOT" ) << "Classification = 1 and X < 1005 or Z = 3004.4" << 3 << false;
  QTest::newRow( "multiple attributes AND/OR/NOT" ) << "Classification = 1 and X < 1005 or Z = 3004.4" << 4 << false;

  QTest::newRow( "multiple attributes compared" ) << "ReturnNumber = NumberOfReturns" << 0 << false;
  QTest::newRow( "multiple attributes compared" ) << "ReturnNumber = NumberOfReturns" << 1 << false;
  QTest::newRow( "multiple attributes compared" ) << "ReturnNumber = NumberOfReturns" << 2 << false;
  QTest::newRow( "multiple attributes compared" ) << "ReturnNumber = NumberOfReturns" << 3 << true;
  QTest::newRow( "multiple attributes compared" ) << "ReturnNumber = NumberOfReturns" << 4 << true;

  QTest::newRow( "multiple attributes compared arithmetic" ) << "ReturnNumber = NumberOfReturns -1" << 0 << true;
  QTest::newRow( "multiple attributes compared arithmetic" ) << "ReturnNumber = NumberOfReturns -1" << 1 << true;
  QTest::newRow( "multiple attributes compared arithmetic" ) << "ReturnNumber = NumberOfReturns -1" << 2 << true;
  QTest::newRow( "multiple attributes compared arithmetic" ) << "ReturnNumber = NumberOfReturns -1" << 3 << false;
  QTest::newRow( "multiple attributes compared arithmetic" ) << "ReturnNumber = NumberOfReturns -1" << 4 << false;

}

void TestQgsPointCloudExpression::testEvaluating()
{
  QFETCH( QString, string );
  QFETCH( int, point_n );
  QFETCH( bool, valid );

  QgsPointCloudExpression exp( string );
  exp.prepare( mBlock );

  QVERIFY( valid ? exp.evaluate( point_n ) != 0. : exp.evaluate( point_n ) == 0. );
}

void TestQgsPointCloudExpression::testBlockResize()
{
  int pointCount = mBlock->pointCount();
  const char *ptr = mBlock->data();
  const int recordSize = mBlock->attributes().pointRecordSize();
  QByteArray data( ptr, pointCount * recordSize );

  // can enlarge, data is unchanged
  mBlock->setPointCount( pointCount + 1 );
  QCOMPARE( mBlock->pointCount(), pointCount + 1 );
  QCOMPARE( QByteArray( ptr, pointCount * recordSize ), data );

  // when shrunk, data is unchanged
  mBlock->setPointCount( pointCount - 1 );
  QCOMPARE( mBlock->pointCount(), pointCount - 1 );
  data.resize( ( pointCount - 1 ) * recordSize );
  QCOMPARE( QByteArray( ptr, mBlock->pointCount() * recordSize ), data );

  // cannot go negative, nothing changes
  mBlock->setPointCount( -1 );
  QCOMPARE( mBlock->pointCount(), pointCount - 1 );
  QCOMPARE( QByteArray( ptr, mBlock->pointCount() * recordSize ), data );

  // can be empty
  mBlock->setPointCount( 0 );
  QCOMPARE( mBlock->pointCount(), 0 );
  data.resize( 0 );
  QCOMPARE( QByteArray( ptr, mBlock->pointCount() * recordSize ), data );
}

QGSTEST_MAIN( TestQgsPointCloudExpression )
#include "testqgspointcloudexpression.moc"
