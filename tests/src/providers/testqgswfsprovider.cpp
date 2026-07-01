/***************************************************************************
    testqgswfsprovider.cpp
    --------------------------------------
    Date                 : June 2026
    Copyright            : (C) 2026 by Alexander Willner
 ***************************************************************************/

#include "qgstest.h"
#include <QObject>
#include <QStringList>
#include "qgswfsdatasourceuri.h"

using namespace Qt::StringLiterals;

class TestQgsWfsProvider : public QObject
{
    Q_OBJECT

  private slots:

    void testPropertyNameUriParsing()
    {
      const QString uri = QStringLiteral(
        "url='https://example.com/wfs' "
        "typename='ns:layer' "
        "version='2.0.0' "
        "propertyName='schema:id_nemo,schema:geometry,schema:asset_type'"
      );

      QgsWFSDataSourceURI dsUri( uri );
      QVERIFY( dsUri.isValid() );

      QStringList props = dsUri.propertyName();
      QCOMPARE( props.size(), 3 );
      QCOMPARE( props.at( 0 ), QStringLiteral( "schema:id_nemo" ) );
      QCOMPARE( props.at( 1 ), QStringLiteral( "schema:geometry" ) );
      QCOMPARE( props.at( 2 ), QStringLiteral( "schema:asset_type" ) );
    }

    void testPropertyNameEmpty()
    {
      const QString uri = QStringLiteral(
        "url='https://example.com/wfs' "
        "typename='ns:layer' "
        "version='2.0.0'"
      );

      QgsWFSDataSourceURI dsUri( uri );
      QVERIFY( dsUri.propertyName().isEmpty() );
    }

    void testPropertyNameUnknownKey()
    {
      const QString uri = QStringLiteral(
        "url='https://example.com/wfs' "
        "typename='ns:layer' "
        "version='2.0.0' "
        "propertyName='schema:id'"
      );

      QgsWFSDataSourceURI dsUri( uri );
      QSet<QString> unknown = dsUri.unknownParamKeys();
      QVERIFY( !unknown.contains( QStringLiteral( "propertyName" ) ) );
    }

    void testPropertyNameSingleValue()
    {
      const QString uri = QStringLiteral(
        "url='https://example.com/wfs' "
        "typename='ns:layer' "
        "version='2.0.0' "
        "propertyName='schema:id_nemo'"
      );

      QgsWFSDataSourceURI dsUri( uri );
      QStringList props = dsUri.propertyName();
      QCOMPARE( props.size(), 1 );
      QCOMPARE( props.at( 0 ), QStringLiteral( "schema:id_nemo" ) );
    }

    void testPropertyNameVersion1()
    {
      const QString uri = QStringLiteral(
        "url='https://example.com/wfs' "
        "typename='ns:layer' "
        "version='1.1.0' "
        "propertyName='prop1,prop2'"
      );

      QgsWFSDataSourceURI dsUri( uri );
      QStringList props = dsUri.propertyName();
      QCOMPARE( props.size(), 2 );
      QCOMPARE( props.at( 0 ), QStringLiteral( "prop1" ) );
      QCOMPARE( props.at( 1 ), QStringLiteral( "prop2" ) );
    }
};

QGSTEST_MAIN( TestQgsWfsProvider )
#include "testqgswfsprovider.moc"
