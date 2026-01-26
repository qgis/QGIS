/***************************************************************************
    testqgsgdalguiutils.cpp
     --------------------------------------
    Date                 : 12.11.2024
    Copyright            : (C) 2024 Abdullaev Ruslan
    Email                : caboose7 at yandex dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgsgdalguiutils.h"
#include "qgstest.h"

class TestQgsGdalGuiUtils : public QObject
{
    Q_OBJECT

  public:
    TestQgsGdalGuiUtils();

  private slots:
    void initTestCase();
    void cleanupTestCase();

    void checkUriBuilding();
};

TestQgsGdalGuiUtils::TestQgsGdalGuiUtils()
  : QObject()
{
}

void TestQgsGdalGuiUtils::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsGdalGuiUtils::checkUriBuilding()
{
  // Test vsicurl: no protocol -> should default to http
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "vsicurl"_L1, "www.test.com"_L1, QString(), QString(), QString(), false );
    QString expected( "/vsicurl/http://www.test.com" );
    QCOMPARE( actual, expected );
  }

  // Test vsicurl: with http already specified
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "vsicurl"_L1, "http://www.test.com"_L1, QString(), QString(), QString(), false );
    // Already has protocol, just add prefix
    QString expected( "/vsicurl/http://www.test.com" );
    QCOMPARE( actual, expected );
  }

  // Test vsicurl: with https
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "vsicurl"_L1, "https://data.test.com/path"_L1, QString(), QString(), QString(), false );
    QString expected( "/vsicurl/https://data.test.com/path" );
    QCOMPARE( actual, expected );
  }

  // Test vsicurl: with ftp
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "vsicurl"_L1, "ftp://files.test.com"_L1, QString(), QString(), QString(), false );
    QString expected( "/vsicurl/ftp://files.test.com" );
    QCOMPARE( actual, expected );
  }

  // Test vsicurl: prefix already included
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "vsicurl"_L1, "/vsicurl/http://already.prefixed.com"_L1, QString(), QString(), QString(), false );
    // No change expected
    QString expected( "/vsicurl/http://already.prefixed.com" );
    QCOMPARE( actual, expected );
  }

  // Test vsis3: no prefix yet
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "vsis3"_L1, "bucket/key"_L1, QString(), QString(), QString(), false );
    QString expected( "/vsis3/bucket/key" );
    QCOMPARE( actual, expected );
  }

  // Test vsis3: prefix already included
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "vsis3"_L1, "/vsis3/mydata"_L1, QString(), QString(), QString(), false );
    QString expected( "/vsis3/mydata" );
    QCOMPARE( actual, expected );
  }

  // Test vsigs
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "vsigs"_L1, "gs://mybucket/data"_L1, QString(), QString(), QString(), false );
    // Even if gs:// is specified, code only prepends /vsigs/ if not present
    QString expected( "/vsigs/gs://mybucket/data" );
    QCOMPARE( actual, expected );
  }

  // Test vsiaz
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "vsiaz"_L1, "mycontainer/myfile"_L1, QString(), QString(), QString(), false );
    QString expected( "/vsiaz/mycontainer/myfile" );
    QCOMPARE( actual, expected );
  }

  // Test vsiadls
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "vsiadls"_L1, "adls_path"_L1, QString(), QString(), QString(), false );
    QString expected( "/vsiadls/adls_path" );
    QCOMPARE( actual, expected );
  }

  // Test vsioss
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "vsioss"_L1, "my_oss_bucket/file"_L1, QString(), QString(), QString(), false );
    QString expected( "/vsioss/my_oss_bucket/file" );
    QCOMPARE( actual, expected );
  }

  // Test vsiswift
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "vsiswift"_L1, "swift_container/object"_L1, QString(), QString(), QString(), false );
    QString expected( "/vsiswift/swift_container/object" );
    QCOMPARE( actual, expected );
  }

  // Test vsihdfs
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "vsihdfs"_L1, "hdfs_path"_L1, QString(), QString(), QString(), false );
    QString expected( "/vsihdfs/hdfs_path" );
    QCOMPARE( actual, expected );
  }

  // Test GeoJSON
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "GeoJSON"_L1, "file.json"_L1, QString(), QString(), QString(), false );
    // No prefix changes for GeoJSON
    QString expected( "file.json" );
    QCOMPARE( actual, expected );
  }

  // Test GeoJSONSeq
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "GeoJSONSeq"_L1, "seq_file.json"_L1, QString(), QString(), QString(), false );
    // No prefix changes for GeoJSONSeq
    QString expected( "seq_file.json" );
    QCOMPARE( actual, expected );
  }

  // Test CouchDB: no prefix yet
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "CouchDB"_L1, "localhost:5984/db"_L1, QString(), QString(), QString(), false );
    QString expected( "couchdb:localhost:5984/db" );
    QCOMPARE( actual, expected );
  }

  // Test CouchDB: prefix already included
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "CouchDB"_L1, "couchdb:localhost:5984/db"_L1, QString(), QString(), QString(), false );
    QString expected( "couchdb:localhost:5984/db" );
    QCOMPARE( actual, expected );
  }

  // Test DODS/OPeNDAP: no prefix
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "DODS/OPeNDAP"_L1, "http://opendap.data"_L1, QString(), QString(), QString(), false );
    QString expected( "DODS:http://opendap.data" );
    QCOMPARE( actual, expected );
  }

  // Test DODS/OPeNDAP: prefix already included
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "DODS/OPeNDAP"_L1, "DODS:http://opendap.data"_L1, QString(), QString(), QString(), false );
    QString expected( "DODS:http://opendap.data" );
    QCOMPARE( actual, expected );
  }

  // Test WFS3: no prefix
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "WFS3"_L1, "https://mywfs3.org"_L1, QString(), QString(), QString(), false );
    QString expected( "WFS3:https://mywfs3.org" );
    QCOMPARE( actual, expected );
  }

  // Test WFS3: prefix already
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "WFS3"_L1, "WFS3:https://mywfs3.org"_L1, QString(), QString(), QString(), false );
    QString expected( "WFS3:https://mywfs3.org" );
    QCOMPARE( actual, expected );
  }

  // Test with configId and expandAuthConfig = false
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "vsicurl"_L1, "mydata.com"_L1, "myconfig"_L1, QString(), QString(), false );
    // Should prepend prefix and protocol
    QString expected( "/vsicurl/http://mydata.com authcfg='myconfig'" );
    QCOMPARE( actual, expected );
  }

  // Test with configId and no username/password, expandAuthConfig = true
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "CouchDB"_L1, "mydb"_L1, "ab34567"_L1, QString(), QString(), true );
    // Prefix is added, then authcfg updated. If no real auth expansion, it remains as is.
    // Actual code tries to expand, if fails, it returns with authcfg.
    QString expected( "couchdb:mydb" );
    QCOMPARE( actual, expected );
  }

  // Test with username/password and no configId
  {
    QString actual = QgsGdalGuiUtils::createProtocolURI( "vsicurl"_L1, "https://securedata.com"_L1, QString(), "user"_L1, "pass"_L1, false );
    // Insert credentials into the URI after "://"
    QString expected( "/vsicurl/https://user:pass@securedata.com" );
    QCOMPARE( actual, expected );
  }
}

void TestQgsGdalGuiUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

QGSTEST_MAIN( TestQgsGdalGuiUtils )
#include "testqgsgdalguiutils.moc"
