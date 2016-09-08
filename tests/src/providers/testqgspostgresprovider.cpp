#include <QtTest/QtTest>
#include <QObject>

#include <qgspostgresprovider.h>

class TestQgsPostgresProvider: public QObject
{
    Q_OBJECT
  private slots:
    void decodeHstore()
    {
      const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::Map, "\"1\"=>\"2\", \"a\"=>\"b \\\"c'\"" );
      QCOMPARE( decoded.type(), QVariant::Map );

      QVariantMap expected;
      expected["1"] = "2";
      expected["a"] = "b \"c'";
      qDebug() << "actual: " << decoded;
      QCOMPARE( decoded.toMap(), expected );
    }

    void decodeHstoreNoQuote()
    {
      const QVariant decoded = QgsPostgresProvider::convertValue( QVariant::Map, "1=>2, a=>b" );
      QCOMPARE( decoded.type(), QVariant::Map );

      QVariantMap expected;
      expected["1"] = "2";
      expected["a"] = "b";
      qDebug() << "actual: " << decoded;
      QCOMPARE( decoded.toMap(), expected );
    }
};

QTEST_MAIN( TestQgsPostgresProvider )
#include "testqgspostgresprovider.moc"
