/***************************************************************************
    testqgswmscapabilities.cpp
    ---------------------
    begin                : May 2016
    copyright            : (C) 2016 by Patrick Valsecchi
    email                : patrick dot valsecchi at camptocamp dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QFile>
#include <QObject>
#include <qgsrasterinterface.h>
#include "qgstest.h"
#include <qgswmscapabilities.h>
#include <qgsapplication.h>

/**
 * \ingroup UnitTests
 * This is a unit test for the WMS capabilities parser.
 */
class TestQgsWmsCapabilities: public QObject
{
    Q_OBJECT
  private slots:

    void initTestCase()
    {
      // init QGIS's paths - true means that all path will be inited from prefix
      QgsApplication::init();
      QgsApplication::initQgis();
    }

    //runs after all tests
    void cleanupTestCase()
    {
      QgsApplication::exitQgis();
    }


    void read()
    {
      QgsWmsCapabilities capabilities;

      QFile file( QStringLiteral( TEST_DATA_DIR ) + "/provider/GetCapabilities.xml" );
      QVERIFY( file.open( QIODevice::ReadOnly | QIODevice::Text ) );
      const QByteArray content = file.readAll();
      QVERIFY( content.size() > 0 );
      const QgsWmsParserSettings config;

      QVERIFY( capabilities.parseResponse( content, config ) );
      QCOMPARE( capabilities.supportedLayers().size(), 5 );
      QCOMPARE( capabilities.supportedLayers()[0].name, QString( "agri_zones" ) );
      QCOMPARE( capabilities.supportedLayers()[1].name, QString( "buildings" ) );
      QCOMPARE( capabilities.supportedLayers()[2].name, QString( "land_surveing_parcels" ) );
      QCOMPARE( capabilities.supportedLayers()[3].name, QString( "cadastre" ) );
      QCOMPARE( capabilities.supportedLayers()[4].name, QString( "test" ) );

      // make sure the default style is not seen twice in the child layers
      QCOMPARE( capabilities.supportedLayers()[3].style.size(), 1 );
      QCOMPARE( capabilities.supportedLayers()[3].style[0].name, QString( "default" ) );
      QCOMPARE( capabilities.supportedLayers()[1].style.size(), 1 );
      QCOMPARE( capabilities.supportedLayers()[1].style[0].name, QString( "default" ) );
      QCOMPARE( capabilities.supportedLayers()[2].style.size(), 1 );
      QCOMPARE( capabilities.supportedLayers()[2].style[0].name, QString( "default" ) );

      // check it can read 2 styles for a layer and that the legend URL is OK
      QCOMPARE( capabilities.supportedLayers()[0].style.size(), 2 );
      QCOMPARE( capabilities.supportedLayers()[0].style[0].name, QString( "yt_style" ) );
      QCOMPARE( capabilities.supportedLayers()[0].style[0].legendUrl.size(), 1 );
      QCOMPARE( capabilities.supportedLayers()[0].style[0].legendUrl[0].onlineResource.xlinkHref,
                QString( "http://www.example.com/yt.png" ) );
      QCOMPARE( capabilities.supportedLayers()[0].style[1].name, QString( "fb_style" ) );
      QCOMPARE( capabilities.supportedLayers()[0].style[1].legendUrl.size(), 1 );
      QCOMPARE( capabilities.supportedLayers()[0].style[1].legendUrl[0].onlineResource.xlinkHref,
                QString( "http://www.example.com/fb.png" ) );

      QCOMPARE( capabilities.supportedLayers()[0].crs, QStringList() << QStringLiteral( "EPSG:2056" ) );
    }

    void guessCrs()
    {
      QgsWmsCapabilities capabilities;

      QFile file( QStringLiteral( TEST_DATA_DIR ) + "/provider/GetCapabilities2.xml" );
      QVERIFY( file.open( QIODevice::ReadOnly | QIODevice::Text ) );
      const QByteArray content = file.readAll();
      QVERIFY( content.size() > 0 );
      const QgsWmsParserSettings config;

      QVERIFY( capabilities.parseResponse( content, config ) );
      QCOMPARE( capabilities.supportedLayers().size(), 5 );

      QCOMPARE( capabilities.supportedLayers().at( 0 ).preferredAvailableCrs(), QStringLiteral( "EPSG:3857" ) );

    }

    void wmstSettings()
    {
      QgsWmsSettings settings = QgsWmsSettings();

      QMap<QString, QString> map = { { "2020-02-13T12:00:00Z", "yyyy-MM-ddThh:mm:ssZ" },
        { "2020-02-13", "yyyy-MM-dd" }
      };
      QMapIterator<QString, QString> iterator( map );

      while ( iterator.hasNext() )
      {
        iterator.next();
        QDateTime date = settings.parseWmstDateTimes( iterator.key() );
        QCOMPARE( date.toString( iterator.value() ), iterator.key() );
      }

      QList<QString> resolutionList =
      {
        "P1D", "P1Y", "PT5M", "P1DT1H",
        "P1Y1DT3S", "P1MT1M", "PT23H3M", "P26DT23H3M", "PT30S"
      };

      for ( const QString &resolutionText : resolutionList )
      {
        QgsTimeDuration resolution = settings.parseWmstResolution( resolutionText );
        QCOMPARE( resolution.toString(), resolutionText );
      }

      QgsWmstDimensionExtent extent = settings.parseTemporalExtent( QStringLiteral( "2020-01-02T00:00:00.000Z/2020-01-09T00:00:00.000Z/P1D" ) );
      settings.setTimeDimensionExtent( extent );

      QDateTime start = QDateTime( QDate( 2020, 1, 2 ), QTime( 0, 0, 0 ), Qt::UTC );
      QDateTime end = QDateTime( QDate( 2020, 1, 9 ), QTime( 0, 0, 0 ), Qt::UTC );

      QgsTimeDuration res;
      res.days = 1;
      QgsTimeDuration extentResolution = extent.datesResolutionList.at( 0 ).resolution;

      QCOMPARE( extent.datesResolutionList.at( 0 ).dates.dateTimes.at( 0 ), start );
      QCOMPARE( extent.datesResolutionList.at( 0 ).dates.dateTimes.at( 1 ), end );

      QCOMPARE( extentResolution.toString(), res.toString() );

      QDateTime firstClosest = settings.findLeastClosestDateTime( QDateTime( QDate( 2020, 1, 3 ), QTime( 16, 0, 0 ), Qt::UTC ) );
      QDateTime firstExpected = QDateTime( QDate( 2020, 1, 3 ), QTime( 0, 0, 0 ), Qt::UTC );

      QCOMPARE( firstClosest, firstExpected );

      QDateTime secondClosest = settings.findLeastClosestDateTime( QDateTime( QDate( 2020, 1, 3 ), QTime( 0, 0, 0 ), Qt::UTC ) );
      QDateTime secondExpected = QDateTime( QDate( 2020, 1, 3 ), QTime( 0, 0, 0 ), Qt::UTC );

      QCOMPARE( secondClosest, secondExpected );

      QgsWmstDimensionExtent secondExtent = settings.parseTemporalExtent( QStringLiteral( "2020-01-02T00:00:00.000Z/2020-01-04T00:00:00.000Z/PT4H" ) );
      settings.setTimeDimensionExtent( secondExtent );

      QDateTime thirdClosest = settings.findLeastClosestDateTime( QDateTime( QDate( 2020, 1, 2 ), QTime( 5, 0, 0 ), Qt::UTC ) );
      QDateTime thirdExpected = QDateTime( QDate( 2020, 1, 2 ), QTime( 4, 0, 0 ), Qt::UTC );

      QCOMPARE( thirdClosest, thirdExpected );

      QDateTime fourthClosest = settings.findLeastClosestDateTime( QDateTime( QDate( 2020, 1, 2 ), QTime( 3, 0, 0 ), Qt::UTC ) );
      QDateTime fourthExpected = QDateTime( QDate( 2020, 1, 2 ), QTime( 0, 0, 0 ), Qt::UTC );

      QCOMPARE( fourthClosest, fourthExpected );

      QDateTime fifthClosest = settings.findLeastClosestDateTime( QDateTime( QDate( 2020, 1, 4 ), QTime( 0, 0, 0 ), Qt::UTC ) );
      QDateTime fifthExpected = QDateTime( QDate( 2020, 1, 4 ), QTime( 0, 0, 0 ), Qt::UTC );

      QCOMPARE( fifthClosest, fifthExpected );

      QDateTime outOfBoundsClosest = settings.findLeastClosestDateTime( QDateTime( QDate( 2020, 1, 5 ), QTime( 0, 0, 0 ), Qt::UTC ) );
      QDateTime outofBoundsExpected = QDateTime( QDate( 2020, 1, 5 ), QTime( 0, 0, 0 ), Qt::UTC );

      QCOMPARE( outOfBoundsClosest, outofBoundsExpected );
    }

    void wmst11extent()
    {
      // test parsing WMS1.1 temporal extent
      const QString layer = R"""(<Layer queryable="0" opaque="0" cascaded="0">
                            <Name>danger_index</Name>
                            <Title>danger_index</Title>
                            <SRS>EPSG:4326</SRS>
                            <LatLonBoundingBox minx="-180" miny="-90" maxx="180" maxy="90" />
                            <BoundingBox SRS="EPSG:4326"
                                        minx="-180" miny="-90" maxx="180" maxy="90" />
                            <Dimension name="time" units="ISO8601"/>
                            <Extent name="time" default="2019-01-01" nearestValue="0">2018-01-01/2019-12-31</Extent>
                        </Layer>)""";

      QDomDocument doc;
      doc.setContent( layer );
      QgsWmsCapabilities cap;
      QgsWmsLayerProperty prop;
      cap.parseLayer( doc.documentElement(), prop );

      QCOMPARE( prop.name, QStringLiteral( "danger_index" ) );
      QCOMPARE( prop.dimensions.size(), 1 );
      QCOMPARE( prop.dimensions.at( 0 ).name, QStringLiteral( "time" ) );
      QCOMPARE( prop.dimensions.at( 0 ).defaultValue, QStringLiteral( "2019-01-01" ) );
      QCOMPARE( prop.dimensions.at( 0 ).extent, QStringLiteral( "2018-01-01/2019-12-31" ) );
      QCOMPARE( prop.dimensions.at( 0 ).units, QStringLiteral( "ISO8601" ) );
    }

    void wmstListOfTimeExtents()
    {
      // test parsing a fixed list of time extents
      QgsWmsSettings settings;

      QgsWmstDimensionExtent extent = settings.parseTemporalExtent( QStringLiteral( "1932-01-01T00:00:00Z, 1947-01-01T00:00:00Z, 1950-01-01T00:00:00Z, 1959-01-01T00:00:00Z, 1960-01-01T00:00:00Z, 1967-01-01T00:00:00Z, 1972-01-01T00:00:00Z, 1974-01-01T00:00:00Z" ) );
      settings.setTimeDimensionExtent( extent );

      QCOMPARE( extent.datesResolutionList.size(), 8 );
      QCOMPARE( extent.datesResolutionList.at( 0 ).dates.dateTimes.size(), 1 );
      QCOMPARE( extent.datesResolutionList.at( 0 ).dates.dateTimes.at( 0 ), QDateTime( QDate( 1932, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );
      QCOMPARE( extent.datesResolutionList.at( 1 ).dates.dateTimes.size(), 1 );
      QCOMPARE( extent.datesResolutionList.at( 1 ).dates.dateTimes.at( 0 ), QDateTime( QDate( 1947, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );
      QCOMPARE( extent.datesResolutionList.at( 2 ).dates.dateTimes.size(), 1 );
      QCOMPARE( extent.datesResolutionList.at( 2 ).dates.dateTimes.at( 0 ), QDateTime( QDate( 1950, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );
      QCOMPARE( extent.datesResolutionList.at( 3 ).dates.dateTimes.size(), 1 );
      QCOMPARE( extent.datesResolutionList.at( 3 ).dates.dateTimes.at( 0 ), QDateTime( QDate( 1959, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );
      QCOMPARE( extent.datesResolutionList.at( 4 ).dates.dateTimes.size(), 1 );
      QCOMPARE( extent.datesResolutionList.at( 4 ).dates.dateTimes.at( 0 ), QDateTime( QDate( 1960, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );
      QCOMPARE( extent.datesResolutionList.at( 5 ).dates.dateTimes.size(), 1 );
      QCOMPARE( extent.datesResolutionList.at( 5 ).dates.dateTimes.at( 0 ), QDateTime( QDate( 1967, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );
      QCOMPARE( extent.datesResolutionList.at( 6 ).dates.dateTimes.size(), 1 );
      QCOMPARE( extent.datesResolutionList.at( 6 ).dates.dateTimes.at( 0 ), QDateTime( QDate( 1972, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );
      QCOMPARE( extent.datesResolutionList.at( 7 ).dates.dateTimes.size(), 1 );
      QCOMPARE( extent.datesResolutionList.at( 7 ).dates.dateTimes.at( 0 ), QDateTime( QDate( 1974, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );

      QCOMPARE( settings.findLeastClosestDateTime( QDateTime( QDate( 1930, 1, 2 ), QTime( 0, 0, 0 ), Qt::UTC ) ),  QDateTime( QDate( 1930, 1, 2 ), QTime( 0, 0, 0 ), Qt::UTC ) );
      QCOMPARE( settings.findLeastClosestDateTime( QDateTime( QDate( 1932, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) ),  QDateTime( QDate( 1932, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );
      QCOMPARE( settings.findLeastClosestDateTime( QDateTime( QDate( 1932, 1, 2 ), QTime( 0, 0, 0 ), Qt::UTC ) ),  QDateTime( QDate( 1932, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );
      QCOMPARE( settings.findLeastClosestDateTime( QDateTime( QDate( 1933, 1, 2 ), QTime( 0, 0, 0 ), Qt::UTC ) ),  QDateTime( QDate( 1932, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );
      QCOMPARE( settings.findLeastClosestDateTime( QDateTime( QDate( 1947, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) ),  QDateTime( QDate( 1947, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );
      QCOMPARE( settings.findLeastClosestDateTime( QDateTime( QDate( 1949, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) ),  QDateTime( QDate( 1947, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );
      QCOMPARE( settings.findLeastClosestDateTime( QDateTime( QDate( 1950, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) ),  QDateTime( QDate( 1950, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );
      QCOMPARE( settings.findLeastClosestDateTime( QDateTime( QDate( 1950, 1, 2 ), QTime( 0, 0, 0 ), Qt::UTC ) ),  QDateTime( QDate( 1950, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );
      QCOMPARE( settings.findLeastClosestDateTime( QDateTime( QDate( 1973, 12, 31 ), QTime( 0, 0, 0 ), Qt::UTC ) ),  QDateTime( QDate( 1972, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );
      QCOMPARE( settings.findLeastClosestDateTime( QDateTime( QDate( 1974, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) ),  QDateTime( QDate( 1974, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );
      QCOMPARE( settings.findLeastClosestDateTime( QDateTime( QDate( 1974, 1, 2 ), QTime( 0, 0, 0 ), Qt::UTC ) ),  QDateTime( QDate( 1974, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );
      QCOMPARE( settings.findLeastClosestDateTime( QDateTime( QDate( 2000, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) ),  QDateTime( QDate( 1974, 1, 1 ), QTime( 0, 0, 0 ), Qt::UTC ) );
    }

    void wmsTemporalDimension_data()
    {
      QTest::addColumn<QString>( "dimension" );
      QTest::addColumn<QString>( "extent" );

      QTest::newRow( "single instant" ) << R"""(<Dimension name="time" units="ISO8601">
                                           2020-01-01
                                           </Dimension>)"""
                                        << "2020-01-01";

      QTest::newRow( "interval" ) << R"""(<Dimension name="time" units="ISO8601">
                                     2020-01-01/2020-12-31/P1M
                                     </Dimension>)"""
                                  << "2020-01-01/2020-12-31/P1M";

      QTest::newRow( "list" )     << R"""(<Dimension name="time" units="ISO8601">
                                     2020-01-01,2020-06-31,2020-12-31
                                     </Dimension>)"""
                                  << "2020-01-01,2020-06-31,2020-12-31";

      QTest::newRow( "continuous" ) << R"""(<Dimension name="time" units="ISO8601">
                                     2020-01-01/2020-06-31
                                     </Dimension>)"""
                                    << "2020-01-01/2020-06-31";

      QTest::newRow( "interval with internal newline characters" )
          << R"""(<Dimension name="time" units="ISO8601">
             2020-01-01/2020-06-31/P1M,
             2020-07-01/2020-12-31/P1D
             </Dimension>)"""
          << "2020-01-01/2020-06-31/P1M, 2020-07-01/2020-12-31/P1D";
    }

    void wmsTemporalDimension()
    {
      QFETCH( QString, dimension );
      QFETCH( QString, extent );

      QDomDocument doc;
      doc.setContent( dimension );
      QgsWmsCapabilities cap;
      QgsWmsDimensionProperty dimensionProperty;

      cap.parseDimension( doc.documentElement(), dimensionProperty );

      QCOMPARE( dimensionProperty.extent, extent );
    }

    void wmsLayerProperty_data()
    {
      QTest::addColumn<QString>( "firstLayer" );
      QTest::addColumn<QString>( "secondLayer" );
      QTest::addColumn<bool>( "result" );

      QTest::newRow( "equal properties" ) << R"""(<Layer queryable="0" opaque="0" cascaded="0">
                                             <Name>Test</Name>
                                             <Title>Test</Title>
                                             <Abstract>Test</Abstract>
                                             <SRS>EPSG:4326</SRS>
                                             <LatLonBoundingBox minx="-180" miny="-90" maxx="180" maxy="90" />
                                             <BoundingBox SRS="EPSG:4326"
                                                         minx="-180" miny="-90" maxx="180" maxy="90" />
                                             <Dimension name="time" units="ISO8601">
                                             2020-01-01
                                             </Dimension>
                                             </Layer>)"""
                                          << R"""(<Layer queryable="0" opaque="0" cascaded="0">
                                             <Name>Test</Name>
                                             <Title>Test</Title>
                                             <Abstract>Test</Abstract>
                                             <SRS>EPSG:4326</SRS>
                                             <LatLonBoundingBox minx="-180" miny="-90" maxx="180" maxy="90" />
                                             <BoundingBox SRS="EPSG:4326"
                                                         minx="-180" miny="-90" maxx="180" maxy="90" />
                                             <Dimension name="time" units="ISO8601">
                                             2020-01-01
                                             </Dimension>
                                             </Layer>)"""
                                          << true;

      QTest::newRow( "different names" ) << R"""(<Layer queryable="0" opaque="0" cascaded="0">
                                            <Name>Test</Name>
                                            <Title>Test</Title>
                                            <Abstract>Test</Abstract>
                                            <SRS>EPSG:4326</SRS>
                                            <LatLonBoundingBox minx="-180" miny="-90" maxx="180" maxy="90" />
                                            <BoundingBox SRS="EPSG:4326"
                                                        minx="-180" miny="-90" maxx="180" maxy="90" />
                                            <Dimension name="time" units="ISO8601">
                                            2020-01-01
                                            </Dimension>
                                            </Layer>)"""
                                         << R"""(<Layer queryable="0" opaque="0" cascaded="0">
                                            <Name>Test2</Name>
                                            <Title>Test</Title>
                                            <Abstract>Test</Abstract>
                                            <SRS>EPSG:4326</SRS>
                                            <LatLonBoundingBox minx="-180" miny="-90" maxx="180" maxy="90" />
                                            <BoundingBox SRS="EPSG:4326"
                                                        minx="-180" miny="-90" maxx="180" maxy="90" />
                                            <Dimension name="time" units="ISO8601">
                                            2020-01-01
                                            </Dimension>
                                            </Layer>)"""
                                         << false;

      QTest::newRow( "different titles" ) << R"""(<Layer queryable="0" opaque="0" cascaded="0">
                                             <Name>Test</Name>
                                             <Title>Test</Title>
                                             <SRS>EPSG:4326</SRS>
                                             <LatLonBoundingBox minx="-180" miny="-90" maxx="180" maxy="90" />
                                             <BoundingBox SRS="EPSG:4326"
                                                         minx="-180" miny="-90" maxx="180" maxy="90" />
                                             <Dimension name="time" units="ISO8601">
                                             2020-01-01
                                             </Dimension>
                                             </Layer>)"""
                                          << R"""(<Layer queryable="0" opaque="0" cascaded="0">
                                             <Name>Test</Name>
                                             <Title>Test2</Title>
                                             <SRS>EPSG:4326</SRS>
                                             <LatLonBoundingBox minx="-180" miny="-90" maxx="180" maxy="90" />
                                             <BoundingBox SRS="EPSG:4326"
                                                         minx="-180" miny="-90" maxx="180" maxy="90" />
                                             <Dimension name="time" units="ISO8601">
                                             2020-01-01
                                             </Dimension>
                                             </Layer>)"""
                                          << false;

      QTest::newRow( "different abstract" ) << R"""(<Layer queryable="0" opaque="0" cascaded="0">
                                             <Name>Test</Name>
                                             <Title>Test</Title>
                                             <Abstract>Test</Abstract>
                                             <SRS>EPSG:4326</SRS>
                                             <LatLonBoundingBox minx="-180" miny="-90" maxx="180" maxy="90" />
                                             <BoundingBox SRS="EPSG:4326"
                                                         minx="-180" miny="-90" maxx="180" maxy="90" />
                                             <Dimension name="time" units="ISO8601">
                                             2020-01-01
                                             </Dimension>
                                             </Layer>)"""
                                            << R"""(<Layer queryable="0" opaque="0" cascaded="0">
                                             <Name>Test</Name>
                                             <Title>Test2</Title>
                                             <Abstract>Test2</Abstract>
                                             <SRS>EPSG:4326</SRS>
                                             <LatLonBoundingBox minx="-180" miny="-90" maxx="180" maxy="90" />
                                             <BoundingBox SRS="EPSG:4326"
                                                         minx="-180" miny="-90" maxx="180" maxy="90" />
                                             <Dimension name="time" units="ISO8601">
                                             2020-01-01
                                             </Dimension>
                                             </Layer>)"""
                                            << false;

      QTest::newRow( "different dimension extent" ) << R"""(<Layer queryable="0" opaque="0" cascaded="0">
                                                       <Name>Test</Name>
                                                       <Title>Test</Title>
                                                       <Abstract>Test</Abstract>
                                                       <SRS>EPSG:4326</SRS>
                                                       <LatLonBoundingBox minx="-180" miny="-90" maxx="180" maxy="90" />
                                                       <BoundingBox SRS="EPSG:4326"
                                                                   minx="-180" miny="-90" maxx="180" maxy="90" />
                                                       <Dimension name="time" units="ISO8601">
                                                       2020-01-01
                                                       </Dimension>
                                                       </Layer>)"""
          << R"""(<Layer queryable="0" opaque="0" cascaded="0">
                                                       <Name>Test</Name>
                                                       <Title>Test</Title>
                                                       <Abstract>Test</Abstract>
                                                       <SRS>EPSG:4326</SRS>
                                                       <LatLonBoundingBox minx="-180" miny="-90" maxx="180" maxy="90" />
                                                       <BoundingBox SRS="EPSG:4326"
                                                                   minx="-180" miny="-90" maxx="180" maxy="90" />
                                                       <Dimension name="time" units="ISO8601">
                                                       2020-01-01/2020-12-31/P1M
                                                       </Dimension>
                                                       </Layer>)"""
          << false;
    }

    void wmsLayerProperty()
    {
      QFETCH( QString, firstLayer );
      QFETCH( QString, secondLayer );
      QFETCH( bool, result );

      QDomDocument doc;
      doc.setContent( firstLayer );

      QDomDocument doc2;
      doc2.setContent( secondLayer );

      QgsWmsCapabilities cap;
      QgsWmsLayerProperty firstLayerProp;
      QgsWmsLayerProperty secondLayerProp;

      cap.parseLayer( doc.documentElement(), firstLayerProp );
      cap.parseLayer( doc2.documentElement(), secondLayerProp );

      QCOMPARE( firstLayerProp.equal( secondLayerProp ), result );

    }

    void wmsIdentifyFormat_data()
    {
      QTest::addColumn<QByteArray>( "format" );
      QTest::addColumn<int>( "capability" );

      QTest::newRow( "text/plain" ) << QByteArray( "text/plain" ) << static_cast<int>( Qgis::RasterInterfaceCapability::IdentifyText );
      QTest::newRow( "text/xml" ) << QByteArray( "text/xml" ) << static_cast<int>( Qgis::RasterInterfaceCapability::IdentifyFeature );
      QTest::newRow( "text/html" ) << QByteArray( "text/html" ) << static_cast<int>( Qgis::RasterInterfaceCapability::IdentifyHtml );
      QTest::newRow( "application/json" ) << QByteArray( "application/json" ) << static_cast<int>( Qgis::RasterInterfaceCapability::IdentifyFeature );
      QTest::newRow( "application/geojson" ) << QByteArray( "application/geojson" ) << static_cast<int>( Qgis::RasterInterfaceCapability::IdentifyFeature );
      QTest::newRow( "application/geo+json" ) << QByteArray( "application/geo+json" ) << static_cast<int>( Qgis::RasterInterfaceCapability::IdentifyFeature );
      QTest::newRow( "application/vnd.ogc.gml" ) << QByteArray( "application/vnd.ogc.gml" ) << static_cast<int>( Qgis::RasterInterfaceCapability::IdentifyFeature );
      QTest::newRow( "application/vnd.esri.wms_featureinfo_xml" ) << QByteArray( "application/vnd.esri.wms_featureinfo_xml" ) << static_cast<int>( Qgis::RasterInterfaceCapability::NoCapabilities );
    }

    void wmsIdentifyFormat()
    {
      QFETCH( QByteArray, format );
      QFETCH( int, capability );

      QgsWmsCapabilities capabilities;

      QFile file( QStringLiteral( TEST_DATA_DIR ) + "/provider/GetCapabilities3.xml" );
      QVERIFY( file.open( QIODevice::ReadOnly | QIODevice::Text ) );
      const QByteArray content = file.readAll().replace( "@INFO_FORMAT@", format );

      QVERIFY( content.size() > 0 );
      const QgsWmsParserSettings config;

      QVERIFY( capabilities.parseResponse( content, config ) );

      // check info formats
      QVERIFY( capabilities.identifyCapabilities() == capability );
    }

    void wmtsTimeDimensionParsing()
    {
      QgsWmsCapabilities capabilities;
      const QgsWmsParserSettings config;

      const QByteArray configData( R"""(<Capabilities>
      <ows:ServiceIdentification>
          <ows:Title>TERN Landscapes Tile Services</ows:Title>
          <ows:ServiceType>OGC WMTS</ows:ServiceType>
          <ows:ServiceTypeVersion>1.0.0</ows:ServiceTypeVersion>
      </ows:ServiceIdentification>
      <ows:ServiceProvider>
          <ows:ServiceContact>
              <ows:ContactInfo/>
          </ows:ServiceContact>
      </ows:ServiceProvider>
      <Contents>
          <Layer>
              <ows:Title>ETa Scaled</ows:Title>
              <ows:Abstract>Mapcache serving CMRSET ETa</ows:Abstract>
              <ows:WGS84BoundingBox>
                  <ows:LowerCorner>109.999000 -45.081000</ows:LowerCorner>
                  <ows:UpperCorner>155.005000 -9.978000</ows:UpperCorner>
              </ows:WGS84BoundingBox>
              <ows:Identifier>ETaScaled</ows:Identifier>
              <Style isDefault="true">
                  <ows:Identifier>default</ows:Identifier>
              </Style>
              <Format>image/png</Format>
              <Dimension>
                  <ows:Identifier>time</ows:Identifier>
                  <Default>current</Default>
                  <Value>2005-08-01T00:00:00Z</Value>
                  <Value>2016-03-01T00:00:00Z</Value>
                  <Value>2005-07-01T00:00:00Z</Value>
                  <Value>2009-02-01T00:00:00Z</Value>
              </Dimension>
              <TileMatrixSetLink>
                  <TileMatrixSet>WGS84</TileMatrixSet>
                  <TileMatrixSetLimits>
                      <TileMatrixLimits>
                          <TileMatrix>WGS84:0</TileMatrix>
                          <MinTileRow>0</MinTileRow>
                          <MaxTileRow>0</MaxTileRow>
                          <MinTileCol>0</MinTileCol>
                          <MaxTileCol>1</MaxTileCol>
                      </TileMatrixLimits>
                  </TileMatrixSetLimits>
              </TileMatrixSetLink>
              <TileMatrixSetLink>
                  <TileMatrixSet>g</TileMatrixSet>
              </TileMatrixSetLink>
              <ResourceURL format="image/png" resourceType="tile" template="https:\/\/landscapes-mapserver.tern.org.au/mapcache/wmts/1.0.0/ETaScaled/default/{time}/{TileMatrixSet}/{TileMatrix}/{TileRow}/{TileCol}.png"/>
          </Layer>
          <TileMatrixSet>
              <ows:Identifier>g</ows:Identifier>
              <ows:BoundingBox crs="urn:ogc:def:crs:EPSG:6.3:900913">
                  <ows:LowerCorner>-20037508.342789 -20037508.342789</ows:LowerCorner>
                  <ows:UpperCorner>20037508.342789 20037508.342789</ows:UpperCorner>
              </ows:BoundingBox>
              <ows:SupportedCRS>urn:ogc:def:crs:EPSG:6.3:900913</ows:SupportedCRS>
              <WellKnownScaleSet>urn:ogc:def:wkss:OGC:1.0:GoogleMapsCompatible</WellKnownScaleSet>
              <TileMatrix>
                  <ows:Identifier>0</ows:Identifier>
                  <ScaleDenominator>559082264.02871787548065185547</ScaleDenominator>
                  <TopLeftCorner>-20037508.342789 20037508.342789</TopLeftCorner>
                  <TileWidth>256</TileWidth>
                  <TileHeight>256</TileHeight>
                  <MatrixWidth>1</MatrixWidth>
                  <MatrixHeight>1</MatrixHeight>
              </TileMatrix>
          </TileMatrixSet>
      </Contents>
</Capabilities>)""" );

      QVERIFY( capabilities.parseResponse( configData, config ) );

      QCOMPARE( capabilities.supportedTileLayers().size(), 1 );
      const QgsWmtsTileLayer tileLayer = capabilities.supportedTileLayers().at( 0 );
      QCOMPARE( tileLayer.title, QStringLiteral( "ETa Scaled" ) );
      QCOMPARE( tileLayer.timeDimensionIdentifier, QStringLiteral( "time" ) );

      QCOMPARE( tileLayer.allTimeRanges, QList< QgsDateTimeRange >(
      {
        QgsDateTimeRange( QDateTime( QDate( 2005, 8, 1 ), QTime( 0, 0, 0 ) ), QDateTime( QDate( 2005, 8, 1 ), QTime( 0, 0, 0 ) ) ),
        QgsDateTimeRange( QDateTime( QDate( 2016, 3, 1 ), QTime( 0, 0, 0 ) ), QDateTime( QDate( 2016, 3, 1 ), QTime( 0, 0, 0 ) ) ),
        QgsDateTimeRange( QDateTime( QDate( 2005, 7, 1 ), QTime( 0, 0, 0 ) ), QDateTime( QDate( 2005, 7, 1 ), QTime( 0, 0, 0 ) ) ),
        QgsDateTimeRange( QDateTime( QDate( 2009, 2, 1 ), QTime( 0, 0, 0 ) ), QDateTime( QDate( 2009, 2, 1 ), QTime( 0, 0, 0 ) ) ),
      } ) );
      QCOMPARE( tileLayer.temporalExtent, QgsDateTimeRange( QDateTime( QDate( 2005, 7, 1 ), QTime( 0, 0, 0 ) ), QDateTime( QDate( 2016, 3, 1 ), QTime( 0, 0, 0 ) ) ) );
      QCOMPARE( tileLayer.temporalInterval, QgsInterval( 1, Qgis::TemporalUnit::IrregularStep ) );
      QCOMPARE( tileLayer.temporalCapabilityFlags, Qgis::RasterTemporalCapabilityFlag::RequestedTimesMustExactlyMatchAllAvailableTemporalRanges );
      QCOMPARE( tileLayer.defaultTimeDimensionValue, QStringLiteral( "current" ) );
    }

    void wmtsTimeDimensionValue_data()
    {
      QTest::addColumn<QString>( "value" );
      QTest::addColumn<QgsDateTimeRange>( "range" );
      QTest::addColumn<int>( "format" );

      QTest::newRow( "YYYYMMDD" ) << QString( "20210103" ) << QgsDateTimeRange( QDateTime( QDate( 2021, 1, 3 ), QTime( 0, 0, 0 ) ), QDateTime( QDate( 2021, 1, 3 ), QTime( 23, 59, 59, 999 ) ) ) << static_cast< int >( QgsWmtsTileLayer::WmtsTimeFormat::yyyyMMdd );
      QTest::newRow( "YYYY-MM-DD" ) << QString( "2021-01-03" ) << QgsDateTimeRange( QDateTime( QDate( 2021, 1, 3 ), QTime( 0, 0, 0 ) ), QDateTime( QDate( 2021, 1, 3 ), QTime( 23, 59, 59, 999 ) ) ) << static_cast< int >( QgsWmtsTileLayer::WmtsTimeFormat::yyyy_MM_dd );
      QTest::newRow( "YYYY" ) << QString( "2021" ) << QgsDateTimeRange( QDateTime( QDate( 2021, 1, 1 ), QTime( 0, 0, 0 ) ), QDateTime( QDate( 2021, 12, 31 ), QTime( 23, 59, 59, 999 ) ) ) << static_cast< int >( QgsWmtsTileLayer::WmtsTimeFormat::yyyy );
      QTest::newRow( "YYYY-MM-DDTHH:mm:ss.SSSZ" ) << QString( "2018-03-01T16:23:44Z" ) << QgsDateTimeRange( QDateTime( QDate( 2018, 3, 1 ), QTime( 16, 23, 44 ) ), QDateTime( QDate( 2018, 3, 1 ), QTime( 16, 23, 44 ) ) ) << static_cast< int >( QgsWmtsTileLayer::WmtsTimeFormat::yyyyMMddThhmmssZ );
    }

    void wmtsTimeDimensionValue()
    {
      QFETCH( QString, value );
      QFETCH( QgsDateTimeRange, range );
      QFETCH( int, format );

      QgsWmtsTileLayer::WmtsTimeFormat resFormat;
      QgsDateTimeRange res = QgsWmsSettings::parseWmtsTimeValue( value, resFormat );
      QCOMPARE( res.begin(), range.begin() );
      QCOMPARE( res.end(), range.end() );
      QCOMPARE( static_cast< int >( resFormat ), format );
    }

};

QGSTEST_MAIN( TestQgsWmsCapabilities )
#include "testqgswmscapabilities.moc"
