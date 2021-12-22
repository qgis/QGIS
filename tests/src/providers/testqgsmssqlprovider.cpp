/***************************************************************************
  testqgsmssqlprovider.cpp
 ---------------------
 begin                : 16.3.2021
 copyright            : (C) 2021 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgstest.h"

#include <QSqlError>
#include <QRandomGenerator>
#include <qtconcurrentrun.h>

//qgis includes...
#include "qgis.h"
#include "qgsapplication.h"
#include "qgsmssqltransaction.h"
#include "qgsmssqlconnection.h"
#include "qgsvectorlayer.h"
//#include "qgsmssqldatabase.h"
#include "qgsproject.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the gdal provider
 */
class TestQgsMssqlProvider : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {}// will be called before each testfunction is executed.
    void cleanup() {}// will be called after every testfunction.

    void openLayer();

    void projectTransaction();

    void transactionTwoLayers();
    void transactionUndoRedo();

  private:

    QString mDbConn;

    QStringList mSomeDataWktGeom;
    QStringList mSomeDataPolyWktGeom;
    QList<QVariantList> mSomeDataAttributes;

};

//runs before all tests
void TestQgsMssqlProvider::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mDbConn = qEnvironmentVariable( "QGIS_MSSQLTEST_DB",
                                  "service='testsqlserver' user=sa password='<YourStrong!Passw0rd>' " );

  mSomeDataWktGeom << QStringLiteral( "Point (-70.33199999999999363 66.32999999999999829)" )
                   << QStringLiteral( "Point (-68.20000000000000284 70.79999999999999716)" )
                   << QString()
                   << QStringLiteral( "Point (-65.31999999999999318 78.29999999999999716)" )
                   << QStringLiteral( "Point (-71.12300000000000466 78.23000000000000398)" );

  QVariantList varList;
  varList << 1ll << 100 << "Orange" <<  "oranGe" <<  "1" << QDateTime( QDate( 2020, 05, 03 ), QTime( 12, 13, 14 ) ) << QDate( 2020, 05, 03 ) << QTime( 12, 13, 14 ) ;
  mSomeDataAttributes << varList;
  varList.clear();
  varList << 2ll << 200 << "Apple" <<  "Apple" <<  "2" << QDateTime( QDate( 2020, 05, 04 ), QTime( 12, 14, 14 ) ) << QDate( 2020, 05, 04 ) << QTime( 12, 14, 14 ) ;
  mSomeDataAttributes << varList;
  varList.clear();
  varList << 3ll << 300 << "Pear" <<  "PEaR" <<  "3" << QDateTime() << QDate() << QTime();
  mSomeDataAttributes << varList;
  varList.clear();
  varList << 4ll << 400 << "Honey" <<  "Honey" <<  "4" << QDateTime( QDate( 2021, 05, 04 ), QTime( 13, 13, 14 ) ) << QDate( 2021, 05, 04 ) << QTime( 13, 13, 14 ) ;
  mSomeDataAttributes << varList;
  varList.clear();
  varList << 5ll << -200 << "" <<  "NuLl" <<  "5" << QDateTime( QDate( 2020, 05, 04 ), QTime( 12, 13, 14 ) ) << QDate( 2020, 05, 02 ) << QTime( 12, 13, 1 ) ;
  mSomeDataAttributes << varList;


  mSomeDataPolyWktGeom << QStringLiteral( "Polygon ((-69 81.40000000000000568, -69 80.20000000000000284, -73.70000000000000284 80.20000000000000284, -73.70000000000000284 76.29999999999999716, -74.90000000000000568 76.29999999999999716, -74.90000000000000568 81.40000000000000568, -69 81.40000000000000568))" )
                       << QStringLiteral( "Polygon ((-67.59999999999999432 81.20000000000000284, -66.29999999999999716 81.20000000000000284, -66.29999999999999716 76.90000000000000568, -67.59999999999999432 76.90000000000000568, -67.59999999999999432 81.20000000000000284))" )
                       << QStringLiteral( "Polygon ((-68.40000000000000568 75.79999999999999716, -67.5 72.59999999999999432, -68.59999999999999432 73.70000000000000284, -70.20000000000000284 72.90000000000000568, -68.40000000000000568 75.79999999999999716))" )
                       << QString();
}


//runs after all tests
void TestQgsMssqlProvider::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMssqlProvider::openLayer()
{
  QString uri( mDbConn + QStringLiteral( " key = \"pk\" srid=4326 type=POINT schema=\"qgis_test\" table=\"someData\" (geom) sql=" ) );

  QgsVectorLayer vl( uri, QStringLiteral( "point_layer" ), QStringLiteral( "mssql" ) );

  QVERIFY( vl.isValid() );
  QCOMPARE( vl.featureCount(), 5 );

  QVERIFY( vl.isValid() );
}


void TestQgsMssqlProvider::projectTransaction()
{
  QString uriPoint( mDbConn + QStringLiteral( " key = \"pk\" srid=4326 type=POINT schema=\"qgis_test\" table=\"someData\" (geom) sql=" ) );
  QString uriPolygon( mDbConn + QStringLiteral( " key = \"pk\" srid=4326 type=POLYGON schema=\"qgis_test\" table=\"some_poly_data\" (geom) sql=" ) );

  QgsProject project;

  QgsVectorLayer *vectorLayerPoint = new QgsVectorLayer( uriPoint, QStringLiteral( "point_layer" ), QStringLiteral( "mssql" ) );
  QgsVectorLayer *vectorLayerPoly = new QgsVectorLayer( uriPolygon, QStringLiteral( "poly_layer" ), QStringLiteral( "mssql" ) );
  project.addMapLayer( vectorLayerPoint );
  project.addMapLayer( vectorLayerPoly );

  project.setTransactionMode( Qgis::TransactionMode::AutomaticGroups );

  QVERIFY( vectorLayerPoint->startEditing() );
  QVERIFY( vectorLayerPoint->isEditable() );
  QVERIFY( vectorLayerPoly->isEditable() );

  // test geometries and attributes of exisintg layer
  QgsFeatureIterator featIt = vectorLayerPoint->getFeatures();
  QgsFeature feat;
  QList<QVariantList> attributes;
  QStringList geoms;
  while ( featIt.nextFeature( feat ) )
  {
    attributes.append( feat.attributes().toList() );
    geoms.append( feat.geometry().asWkt() );
  }
  QCOMPARE( attributes.size(), mSomeDataAttributes.size() );
  QVERIFY( attributes == mSomeDataAttributes );
  QVERIFY( geoms == mSomeDataWktGeom );

  featIt = vectorLayerPoly->getFeatures();
  geoms.clear();
  while ( featIt.nextFeature( feat ) )
    geoms.append( feat.geometry().asWkt() );

  QVERIFY( geoms == mSomeDataPolyWktGeom );

  // add a point to the layer point
  feat = QgsFeature();
  feat.setFields( vectorLayerPoint->fields(), true );
  feat.setAttribute( 0, 10 );
  feat.setGeometry( QgsGeometry( new QgsPoint( -71, 67 ) ) );

  QStringList newGeoms = mSomeDataWktGeom;
  newGeoms << QStringLiteral( "Point (-71 67)" );
  QList<QVariantList> newAttributes = mSomeDataAttributes;
  QVariantList attrList;
  attrList << 10ll << 0 << "" << "" << "" << QDateTime() << QDate() << QTime();
  newAttributes.append( attrList );

  vectorLayerPoint->addFeature( feat );

  QCOMPARE( vectorLayerPoint->featureCount(), 6 );

  featIt = vectorLayerPoint->getFeatures();
  attributes.clear();
  geoms.clear();
  while ( featIt.nextFeature( feat ) )
  {
    attributes.append( feat.attributes().toList() );
    geoms.append( feat.geometry().asWkt() );
  }
  QCOMPARE( attributes.size(), newAttributes.size() );
  QVERIFY( attributes == newAttributes );
  QVERIFY( geoms == newGeoms );

  vectorLayerPoint->rollBack();

  QCOMPARE( vectorLayerPoint->featureCount(), 5 );
}

void TestQgsMssqlProvider::transactionTwoLayers()
{
  QString uriPoint( mDbConn + QStringLiteral( " key = \"pk\" srid=4326 type=POINT schema=\"qgis_test\" table=\"someData\" (geom) sql=" ) );
  QgsVectorLayer *vectorLayerPoint1 = new QgsVectorLayer( uriPoint, QStringLiteral( "point_layer_1" ), QStringLiteral( "mssql" ) );
  QgsVectorLayer *vectorLayerPoint2 = new QgsVectorLayer( uriPoint, QStringLiteral( "point_layer_2" ), QStringLiteral( "mssql" ) );

  QgsProject project;
  project.addMapLayer( vectorLayerPoint1 );
  project.addMapLayer( vectorLayerPoint2 );

  // point to be added to the point layer
  QgsFeature feat;
  feat.setFields( vectorLayerPoint1->fields(), true );
  feat.setAttribute( 0, 123 );
  feat.setGeometry( QgsGeometry( new QgsPoint( -123, 123 ) ) );

  // 1. without transaction, try to add a feature to the first layer -> the other layer is not affected

  QVERIFY( vectorLayerPoint1->startEditing() );

  vectorLayerPoint1->addFeature( feat );

  QCOMPARE( vectorLayerPoint1->featureCount(), 6 );
  QCOMPARE( vectorLayerPoint2->featureCount(), 5 );

  vectorLayerPoint1->rollBack();

  // 2. with transaction, try to add a feature to the first layer -> both layers are affected

  project.setTransactionMode( Qgis::TransactionMode::AutomaticGroups );

  QVERIFY( vectorLayerPoint1->startEditing() );

  vectorLayerPoint1->addFeature( feat );

  QCOMPARE( vectorLayerPoint1->featureCount(), 6 );
  QCOMPARE( vectorLayerPoint2->featureCount(), 6 );

  vectorLayerPoint1->rollBack();

  QCOMPARE( vectorLayerPoint1->featureCount(), 5 );
  QCOMPARE( vectorLayerPoint2->featureCount(), 5 );
}

void TestQgsMssqlProvider::transactionUndoRedo()
{
  QString uriPoint( mDbConn + QStringLiteral( " key = \"pk\" srid=4326 type=POINT schema=\"qgis_test\" table=\"someData\" (geom) sql=" ) );
  QgsVectorLayer *vectorLayerPoint1 = new QgsVectorLayer( uriPoint, QStringLiteral( "point_layer_1" ), QStringLiteral( "mssql" ) );
  //QgsVectorLayer *vectorLayerPoint2 = new QgsVectorLayer( uriPoint, QStringLiteral( "point_layer_2" ), QStringLiteral( "mssql" ) );

  QgsProject project;
  project.addMapLayer( vectorLayerPoint1 );
  //project.addMapLayer( vectorLayerPoint2 );

  // point to be added to the point layer
  QgsFeature feat1;
  feat1.setFields( vectorLayerPoint1->fields(), true );
  feat1.setAttribute( 0, 111 );
  feat1.setGeometry( QgsGeometry( new QgsPoint( -111, 111 ) ) );

  QgsFeature feat2;
  feat2.setFields( vectorLayerPoint1->fields(), true );
  feat2.setAttribute( 0, 222 );
  feat2.setGeometry( QgsGeometry( new QgsPoint( -222, 222 ) ) );

  // 1. without transaction

  project.setTransactionMode( Qgis::TransactionMode::AutomaticGroups );

  QVERIFY( vectorLayerPoint1->startEditing() );

  QCOMPARE( vectorLayerPoint1->undoStack()->count(), 0 );

  vectorLayerPoint1->addFeature( feat1 );

  vectorLayerPoint1->addFeature( feat2 );

  QCOMPARE( vectorLayerPoint1->undoStack()->count(), 2 );

  QCOMPARE( vectorLayerPoint1->featureCount(), 7 );
  vectorLayerPoint1->undoStack()->undo();
  QCOMPARE( vectorLayerPoint1->featureCount(), 6 );
  vectorLayerPoint1->undoStack()->undo();
  QCOMPARE( vectorLayerPoint1->featureCount(), 5 );
  vectorLayerPoint1->undoStack()->redo();
  QCOMPARE( vectorLayerPoint1->featureCount(), 6 );
  vectorLayerPoint1->undoStack()->redo();
  QCOMPARE( vectorLayerPoint1->featureCount(), 7 );

  vectorLayerPoint1->rollBack();

  // 2. with transaction, try to add a feature to the first layer -> both layers are affected

}

QGSTEST_MAIN( TestQgsMssqlProvider )
#include "testqgsmssqlprovider.moc"
