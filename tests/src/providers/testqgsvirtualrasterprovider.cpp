/***************************************************************************
    testqgsvirtualrasterprovider.cpp
    --------------------------------------
   Date                 : June 2021
   Copyright            : (C) 2021 by Francesco Bursi
   Email                : francesco.bursi@hotmail.it
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <limits>

#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QTemporaryDir>

//qgis includes...
#include <qgis.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsrasterdataprovider.h>
#include "qgsmaplayer.h"
#include "qgsrasterlayer.h"
#include <qgsrectangle.h>
#include "qgsproject.h"
#include "qgsrastercalculator.h"
#include "qgsrastercalcnode.h"

#include "qgsvirtualrasterprovider.h"

#include <QUrl>
#include <QUrlQuery>
/**
* \ingroup UnitTests
* This is a unit test for the virtualraster provider
*/


class TestQgsVirtualRasterProvider : public QObject
{
   Q_OBJECT

 private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {}// will be called before each testfunction is executed.
    void cleanup() {}// will be called after every testfunction.

    void validLayer();
    void testv();
    void testRaster();
    void testUrlDecoding();
    void testUrlDecodingMinimal();
    void testUriProviderDecoding();
    void testUriEncoding();

private:
    QString mTestDataDir;
    QString mReport;
    QgsRasterLayer *mdemRasterLayer = nullptr;

};

//runs before all tests
void TestQgsVirtualRasterProvider::initTestCase()
{
    // init QGIS's paths - true means that all path will be inited from prefix
    QgsApplication::init();
    QgsApplication::initQgis();

    mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt
    mReport = QStringLiteral( "<h1>Virtual Raster Provider Tests</h1>\n" );

    QString demFileName = mTestDataDir + "raster/dem.tif";
    QFileInfo demRasterFileInfo( demFileName );
    mdemRasterLayer = new QgsRasterLayer( demRasterFileInfo.filePath(),
                                          demRasterFileInfo.completeBaseName() );

    QgsProject::instance()->addMapLayers(
      QList<QgsMapLayer *>() << mdemRasterLayer );
}

void TestQgsVirtualRasterProvider::validLayer()
{
  QgsRasterLayer::LayerOptions options;

  std::unique_ptr< QgsRasterLayer > layer = std::make_unique< QgsRasterLayer >(
        mTestDataDir + QStringLiteral( "raster/dem.tif" ),
        QStringLiteral( "layer" ),
        QStringLiteral( "virtualrasterprovider" ),
        options
      );

  QVERIFY( layer->isValid() );

}

//runs after all tests
void TestQgsVirtualRasterProvider::cleanupTestCase()
{
  QgsApplication::exitQgis();
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsVirtualRasterProvider::testv()
{
    if ( mdemRasterLayer->extent().xMaximum()== 45.8117014376000000 )
    {
        QgsDebugMsg("testv is starting");
    }
/*
    QgsRasterCalculatorEntry entry1;
    entry1.bandNumber = 1;
    entry1.raster = mdemRasterLayer;
    entry1.ref = QStringLiteral( "dem@1" );

    QVector<QgsRasterCalculatorEntry> entries;
    entries << entry1;

    //QgsCoordinateReferenceSystem crs( QStringLiteral( "EPSG:32633" ) );
    QgsCoordinateReferenceSystem mOutputCrs( QStringLiteral( "EPSG:4326" ) );
    QgsRectangle extent(18.6662979442000001,45.7767014376000034,18.7035979441999984,45.8117014376000000);
*/


    QgsRectangle extent(18.6662979442000001,45.7767014376000034,18.7035979441999984,45.8117014376000000);
    QString demFileName = "/home/franc/dev/cpp/QGIS/tests/testdata/raster/dem.tif";
    QgsDataProvider *provider = QgsProviderRegistry::instance()->createProvider( QStringLiteral( "virtualrasterprovider" ), demFileName, QgsDataProvider::ProviderOptions() );
    QgsRasterDataProvider *rp = dynamic_cast< QgsRasterDataProvider * >( provider );
    QVERIFY( rp );
    QVERIFY( rp->isValid() );
    if ( rp )
      {
        std::unique_ptr<QgsRasterBlock> block( rp->block( 1, extent, 373, 350 ) );

        qDebug() << "VALUE BLOCK at row 0, col 0: " << block->value( 0, 0 );
        qDebug() << "VALUE BLOCK at  row 350, col 373: " << block->value(349,372);
        qDebug() << "bandCount result: " << rp->bandCount();
        QVERIFY( block );
        QCOMPARE( block->width(),  373 );
        QCOMPARE( block->height(), 350 );

        QCOMPARE( block->value( 0, 0 ), 292.86041259765625 );

        QCOMPARE( rp->bandCount(), 1 );
      }
    delete provider;

}

void TestQgsVirtualRasterProvider::testRaster()
{

    QgsRasterLayer *r = new QgsRasterLayer("/home/franc/dev/cpp/QGIS/tests/testdata/raster/dem.tif","dem","virtualrasterprovider");
    QgsDebugMsg("NAME of LAYER: "+r->name());

    double sampledValue= r->dataProvider()->sample(QgsPointXY(18.67714, 45.79202),1);

    qDebug() <<"VALUE IS "<< sampledValue;

    delete r;

}

void TestQgsVirtualRasterProvider::testUrlDecoding()
{
    //only to check how qurl and qurlquery works
    QUrl url("?crs=EPSG:4326&extent=18.6662979442000001,45.7767014376000034,18.7035979441999984,45.8117014376000000&width=373&height=350&formula=\"dem@1\" + 200&dem:uri=path/to/file&dem:provider=gdal&landsat:uri=path/to/landsat&landsat:provider=gdal");
    //FROM HERE
    QUrlQuery query(url.query());
    QVariantMap components;

    QSet<QString> rLayerName;
    for ( const auto &item : query.queryItems() )
    {
        if ( item.first.indexOf(':') > 0 )
        {
            rLayerName.insert( item.first.mid(0, item.first.indexOf(':')) );
        }
        else
        {
            components.insert( item.first, item.second );
        }
    }

    QVariantMap rLayers;
/*
    foreach (const QString &value, rLayerName)
    {

        //QVariantMap rLayer;

        //rLayer.insert( "name" ,  value );
        //rLayer.insert( "uri" , query.queryItemValue( value % QStringLiteral(":uri") ) );
        //rLayer.insert( "provider" , query.queryItemValue( value % QStringLiteral(":provider") ) );

       //rLayers.insert(QStringLiteral("rLayer")%QStringLiteral("@")%value,rLayer);

        QStringList rLayer;
        rLayer << value;
        rLayer << query.queryItemValue( value % QStringLiteral(":uri") );
        rLayer << query.queryItemValue( value % QStringLiteral(":provider") );

        rLayers.insert(QStringLiteral("rLayer")%QStringLiteral("@")%value,rLayer);

    }
 */


    QSet<QString>::iterator i;
    for (i = rLayerName.begin(); i != rLayerName.end(); ++i)
    //foreach (const QString &value, rLayerName)
    {
        QStringList rLayer;
        rLayer << (*i);
        rLayer << query.queryItemValue( (*i) % QStringLiteral(":uri") );
        rLayer << query.queryItemValue( (*i) % QStringLiteral(":provider") );

        rLayers.insert(QStringLiteral("rLayer")%QStringLiteral("@")%(*i),rLayer);
    }

    components.insert( QStringLiteral("rLayers"), rLayers );
    //TO HERE
    //qDebug() << components << endl;
    //qDebug() << components.value("rLayers")<< endl;

    /*
    qDebug() << components.value("rLayers").toMap()<< endl;
    qDebug() << components.value("rLayers").toMap().value("rLayer@dem") << endl;
    qDebug() << components.value("rLayers").toMap().value("rLayer@dem").toMap().value("name") << endl;
    */

    //qDebug() <<components.value("width").toString().toInt() << endl;
    //qDebug() <<components.value("extent").toString() << endl;

}

void TestQgsVirtualRasterProvider::testUrlDecodingMinimal()
{
    QUrl url("?crs=EPSG:4326&extent=POLYGON((18.6662979442000001 45.77670143760000343, 18.70359794419999844 45.77670143760000343, 18.70359794419999844 45.81170143760000002, 18.6662979442000001 45.81170143760000002, 18.6662979442000001 45.77670143760000343))&width=373&height=350&formula=\"dem@1\" + 200&dem:uri=path/to/file&dem:provider=gdal&landsat:uri=path/to/landsat&landsat:provider=gdal");
    QUrlQuery query(url.query());
    QVariantMap components;

    for ( const auto &item : query.queryItems() )
    {
        components.insert( item.first, item.second );
    }

    /*
    qDebug() << components << endl;

    qDebug() <<"--------------------------------------------------------------------------------------------------";

    QString uri1 = QStringLiteral("?crs=EPSG:4326&extent=POLYGON((18.6662979442000001 45.77670143760000343, 18.70359794419999844 45.77670143760000343, 18.70359794419999844 45.81170143760000002, 18.6662979442000001 45.81170143760000002, 18.6662979442000001 45.77670143760000343))&width=373&height=350&formula=\"dem@1\" + 200&dem:uri=path/to/file&dem:provider=gdal&landsat:uri=path/to/landsat&landsat:provider=gdal");
    QUrl url1 = QUrl::fromEncoded( uri1.toUtf8() );
    const QUrlQuery query1( url1 );

    //crs
    if ( query1.hasQueryItem( QStringLiteral( "crs" ) ) )
    {
        QgsCoordinateReferenceSystem mCrs;
        mCrs.createFromString( query1.queryItemValue( QStringLiteral( "crs" ) ) );

        qDebug() << "mCrs.authid: " << mCrs.authid();
        //qDebug() << query1.queryItemValue( QStringLiteral( "crs" ) );
    }

    //width and height (the same wth different key)
    if ( query1.hasQueryItem( QStringLiteral( "width" ) ) )
    {
        int mWidth;
        mWidth = query1.queryItemValue( QStringLiteral( "width" ) ).toInt();
        qDebug() << "mWidth: " << mWidth;
    }

    //formula
    if ( query1.hasQueryItem( QStringLiteral( "formula" ) ) )
    {
        QString mFormulaString;
        mFormulaString = query1.queryItemValue( QStringLiteral( "formula" ) );
        qDebug() << "mFormula: " << mFormulaString;
    }

    //extent
    if ( query1.hasQueryItem( QStringLiteral( "extent" ) ) )
    {
        QgsRectangle mExtent;
        mExtent = QgsRectangle::fromWkt ( query1.queryItemValue( QStringLiteral( "extent" ) ) );
        qDebug() << "mExtent: " << mExtent.toString();
        QCOMPARE( mExtent.toString() , QStringLiteral("18.6662979442000001,45.7767014376000034 : 18.7035979441999984,45.8117014376000000") );
    }

    //rasterlayer

    QSet<QString> rLayerName;
    for ( const auto &item : query.queryItems() )
    {
        if ( (item.first.indexOf(':') == -1) )
        {
            continue;
        }
        else
        {
            rLayerName.insert( item.first.mid(0, item.first.indexOf(':')) );
        }
    }
    qDebug() << "rLayerName " << rLayerName;

    QVector<QStringList> mRasterLayers;

    QSet<QString>::iterator i;
    for (i = rLayerName.begin(); i != rLayerName.end(); ++i)
    //foreach (const QString &value, rLayerName)
    {
        QStringList rLayerEl;
        rLayerEl << (*i);
        rLayerEl << query.queryItemValue( (*i) % QStringLiteral(":uri") );
        rLayerEl << query.queryItemValue( (*i) % QStringLiteral(":provider") );


        //mRasterLayers.append( QgsRasterLayer( rLayerEl.at(1) , rLayerEl.at(0) , rLayerEl.at(2)) );
        mRasterLayers.append(rLayerEl);
    }
    qDebug() << mRasterLayers;

    */
}

void TestQgsVirtualRasterProvider::testUriProviderDecoding()
{
    QgsRasterDataProvider::DecodedUriParameters decodedParams = QgsVirtualRasterProvider::decodeVirtualRasterProviderUri(QStringLiteral("?crs=EPSG:4326&extent=18.6662979442000001,45.7767014376000034,18.7035979441999984,45.8117014376000000&width=373&height=350&formula=\"dem@1\" + 200&dem:uri=path/to/file&dem:provider=gdal&landsat:uri=path/to/landsat&landsat:provider=gdal"));


    QCOMPARE( decodedParams.width , 373);
    QCOMPARE( decodedParams.height , 350);
    QCOMPARE( decodedParams.extent , QgsRectangle(18.6662979442000001,45.7767014376000034,18.7035979441999984,45.8117014376000000) );
    QCOMPARE( decodedParams.crs , QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
    QCOMPARE( decodedParams.formula , QStringLiteral( "\"dem@1\" + 200" ) );


    QCOMPARE( decodedParams.rInputLayers.at(1).provider , QStringLiteral( "gdal" ) );
    QCOMPARE( decodedParams.rInputLayers.at(0).provider , QStringLiteral( "gdal" ) );

    qDebug() << endl;
    qDebug() << QStringLiteral("Raster layer: name, uri, provider");
    for (int i = 0; i < decodedParams.rInputLayers.size() ; ++i)
    {
        qDebug() << decodedParams.rInputLayers.at(i).name << " " <<
                    decodedParams.rInputLayers.at(i).uri  << " " <<
                    decodedParams.rInputLayers.at(i).provider;
    }
    qDebug() << endl;
}

void TestQgsVirtualRasterProvider::testUriEncoding()
{

    QgsRasterDataProvider::DecodedUriParameters decodedParams = QgsVirtualRasterProvider::decodeVirtualRasterProviderUri(QStringLiteral("?crs=EPSG:4326&extent=18.6662979442000001,45.7767014376000034,18.7035979441999984,45.8117014376000000&width=373&height=350&formula=\"dem@1\" + 200&dem:uri=path/to/file&dem:provider=gdal&landsat:uri=path/to/landsat&landsat:provider=gdal"));
    qDebug() << QgsVirtualRasterProvider::encodeVirtualRasterProviderUri( decodedParams ) << endl;

    /*
    QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "virtualrasterprovider" ) );
    QVERIFY( metadata );

    const QVariantMap parts = metadata->decodeUri( QStringLiteral("?crs=EPSG:4326&extent=POLYGON((18.6662979442000001 45.77670143760000343, 18.70359794419999844 45.77670143760000343, 18.70359794419999844 45.81170143760000002, 18.6662979442000001 45.81170143760000002, 18.6662979442000001 45.77670143760000343))&width=373&height=350&formula=\"dem@1\" + 200&dem:uri=path/to/file&dem:provider=gdal&landsat:uri=path/to/landsat&landsat:provider=gdal") );

    //qDebug() << parts << endl;
    qDebug() << endl;
    qDebug() << QgsVirtualRasterProvider::encodeVirtualRasterProviderUri( parts ) << endl;
*/
}

QGSTEST_MAIN( TestQgsVirtualRasterProvider )
#include "testqgsvirtualrasterprovider.moc"
