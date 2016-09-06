#include <QtTest/QtTest>
#include <QObject>

#include <qgspostgresconn.h>

class TestQgsPostgresConn: public QObject
{
    Q_OBJECT
  private slots:
    void quotedValueHstore()
    {
      QVariantMap map;
      map["1"] = "2";
      map["a"] = "b \"c' \\x";

      const QString actual = QgsPostgresConn::quotedValue( map );
      QCOMPARE( actual, QString( "E'\"1\"=>\"2\",\"a\"=>\"b \\\\\"c\\' \\\\\\\\x\"'::hstore" ) );
    }

    void quotedValueString()
    {
      QCOMPARE( QgsPostgresConn::quotedValue( "b" ), QString( "'b'" ) );
      QCOMPARE( QgsPostgresConn::quotedValue( "b's" ), QString( "'b''s'" ) );
      QCOMPARE( QgsPostgresConn::quotedValue( "b \"c' \\x" ), QString( "E'b \"c'' \\\\x'" ) );
    }

    void quotedValueStringArray()
    {
      QStringList list;
      list << "a" << "b \"c' \\x";
      const QString actual = QgsPostgresConn::quotedValue( list );
      QCOMPARE( actual, QString( "E'{\"a\",\"b \\\\\"c\\' \\\\\\\\x\"}'" ) );
    }

    void quotedValueIntArray()
    {
      QVariantList list;
      list << 1 << -5;
      const QString actual = QgsPostgresConn::quotedValue( list );
      QCOMPARE( actual, QString( "E'{\"1\",\"-5\"}'" ) );
    }

};

QTEST_MAIN( TestQgsPostgresConn )
#include "testqgspostgresconn.moc"
