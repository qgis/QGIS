/***************************************************************************
     testqgspallabeling.cpp
     ----------------------
    Date                 : May 2015
    Copyright            : (C) 2015 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsfontutils.h"
#include "qgsmaprenderersequentialjob.h"
#include "qgsmapsettings.h"
#include "qgsnullsymbolrenderer.h"
#include "qgspallabeling.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerlabeling.h"

#include <QMimeData>
#include <QObject>
#include <QString>
#include <QStringList>

class TestQgsPalLabeling : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsPalLabeling()
      : QgsTest( u"PAL labeling Tests"_s, u"pallabeling"_s ) {}

  private slots:
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void wrapChar();        //test wrapping text lines
    void graphemes();       //test splitting strings to graphemes
    void testGeometryGenerator();
    void testLabelSettingsToFromMime();
};

void TestQgsPalLabeling::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsPalLabeling::wrapChar()
{
  QCOMPARE( QgsPalLabeling::splitToLines( "nolines", QString() ), QStringList() << "nolines" );
  QCOMPARE( QgsPalLabeling::splitToLines( "new line\nonly", QString() ), QStringList() << "new line" << "only" );
  QCOMPARE( QgsPalLabeling::splitToLines( "new line\nonly", QString( "\n" ) ), QStringList() << "new line" << "only" );
  QCOMPARE( QgsPalLabeling::splitToLines( "mixed new line\nand char", QString( " " ) ), QStringList() << "mixed" << "new" << "line" << "and" << "char" );
  QCOMPARE( QgsPalLabeling::splitToLines( "no matching chars", QString( "#" ) ), QStringList() << "no matching chars" );
  QCOMPARE( QgsPalLabeling::splitToLines( "no\nmatching\nchars", QString( "#" ) ), QStringList() << "no" << "matching" << "chars" );

  // with auto wrap
  QCOMPARE( QgsPalLabeling::splitToLines( "with auto wrap", QString(), 12, true ), QStringList() << "with auto" << "wrap" );
  QCOMPARE( QgsPalLabeling::splitToLines( "with auto wrap", QString(), 6, false ), QStringList() << "with auto" << "wrap" );

  // manual wrap character should take precedence
  QCOMPARE( QgsPalLabeling::splitToLines( u"with auto-wrap and manual-wrap"_s, u"-"_s, 12, true ), QStringList() << "with auto" << "wrap and" << "manual" << "wrap" );
  QCOMPARE( QgsPalLabeling::splitToLines( u"with automatic-wrap and manual-wrap"_s, u"-"_s, 12, true ), QStringList() << "with" << "automatic" << "wrap and" << "manual" << "wrap" );
  QCOMPARE( QgsPalLabeling::splitToLines( u"with automatic-wrap and manual-wrap"_s, u"-"_s, 6, true ), QStringList() << "with" << "automatic" << "wrap" << "and" << "manual" << "wrap" );
  QCOMPARE( QgsPalLabeling::splitToLines( u"with auto-wrap and manual-wrap"_s, u"-"_s, 12, false ), QStringList() << "with auto" << "wrap and manual" << "wrap" );
  QCOMPARE( QgsPalLabeling::splitToLines( u"with auto-wrap and manual-wrap"_s, u"-"_s, 6, false ), QStringList() << "with auto" << "wrap and" << "manual" << "wrap" );
}

void TestQgsPalLabeling::graphemes()
{
  QCOMPARE( QgsPalLabeling::splitToGraphemes( QString() ), QStringList() );
  QCOMPARE( QgsPalLabeling::splitToGraphemes( "abcd" ), QStringList() << "a" << "b" << "c" << "d" );
  QCOMPARE( QgsPalLabeling::splitToGraphemes( "ab cd" ), QStringList() << "a" << "b" << " " << "c" << "d" );
  QCOMPARE( QgsPalLabeling::splitToGraphemes( "ab cd " ), QStringList() << "a" << "b" << " " << "c" << "d" << " " );

  //note - have to use this method to build up unicode QStrings to avoid issues with Windows
  //builds and invalid codepages
  QString str1;
  str1 += QChar( 0x179F );
  str1 += QChar( 0x17D2 );
  str1 += QChar( 0x178F );
  str1 += QChar( 0x17D2 );
  str1 += QChar( 0x179A );
  str1 += QChar( 0x17B8 );
  str1 += QChar( 0x179B );
  str1 += QChar( 0x17D2 );
  QString expected1Pt1;
  expected1Pt1 += QChar( 0x179F );
  expected1Pt1 += QChar( 0x17D2 );
  expected1Pt1 += QChar( 0x178F );
  expected1Pt1 += QChar( 0x17D2 );
  expected1Pt1 += QChar( 0x179A );
  expected1Pt1 += QChar( 0x17B8 );
  QString expected1Pt2;
  expected1Pt2 += QChar( 0x179B );
  expected1Pt2 += QChar( 0x17D2 );

  QCOMPARE( QgsPalLabeling::splitToGraphemes( str1 ), QStringList() << expected1Pt1 << expected1Pt2 );

  QString str2;
  str2 += QChar( 0x1780 );
  str2 += QChar( 0x17D2 );
  str2 += QChar( 0x179A );
  str2 += QChar( 0x17BB );
  str2 += QChar( 0x1798 );
  str2 += QChar( 0x17A2 );
  str2 += QChar( 0x1784 );
  str2 += QChar( 0x17D2 );
  str2 += QChar( 0x1782 );
  str2 += QChar( 0x1780 );
  str2 += QChar( 0x17B6 );
  str2 += QChar( 0x179A );
  str2 += QChar( 0x179F );
  str2 += QChar( 0x17B7 );
  str2 += QChar( 0x1791 );
  str2 += QChar( 0x17D2 );
  str2 += QChar( 0x1792 );
  str2 += QChar( 0x17B7 );
  str2 += QChar( 0x1798 );
  str2 += QChar( 0x1793 );
  str2 += QChar( 0x17BB );
  str2 += QChar( 0x179F );
  str2 += QChar( 0x17D2 );
  str2 += QChar( 0x179F );

  QString expected2Pt1;
  expected2Pt1 += QChar( 0x1780 );
  expected2Pt1 += QChar( 0x17D2 );
  expected2Pt1 += QChar( 0x179A );
  expected2Pt1 += QChar( 0x17BB );
  QString expected2Pt2;
  expected2Pt2 += QChar( 0x1798 );
  QString expected2Pt3;
  expected2Pt3 += QChar( 0x17A2 );
  QString expected2Pt4;
  expected2Pt4 += QChar( 0x1784 );
  expected2Pt4 += QChar( 0x17D2 );
  expected2Pt4 += QChar( 0x1782 );
  QString expected2Pt5;
  expected2Pt5 += QChar( 0x1780 );
  expected2Pt5 += QChar( 0x17B6 );
  QString expected2Pt6;
  expected2Pt6 += QChar( 0x179A );
  QString expected2Pt7;
  expected2Pt7 += QChar( 0x179F );
  expected2Pt7 += QChar( 0x17B7 );
  QString expected2Pt8;
  expected2Pt8 += QChar( 0x1791 );
  expected2Pt8 += QChar( 0x17D2 );
  expected2Pt8 += QChar( 0x1792 );
  expected2Pt8 += QChar( 0x17B7 );
  QString expected2Pt9;
  expected2Pt9 += QChar( 0x1798 );
  QString expected2Pt10;
  expected2Pt10 += QChar( 0x1793 );
  expected2Pt10 += QChar( 0x17BB );
  QString expected2Pt11;
  expected2Pt11 += QChar( 0x179F );
  expected2Pt11 += QChar( 0x17D2 );
  expected2Pt11 += QChar( 0x179F );

  QCOMPARE( QgsPalLabeling::splitToGraphemes( str2 ), QStringList() << expected2Pt1 << expected2Pt2 << expected2Pt3 << expected2Pt4 << expected2Pt5 << expected2Pt6 << expected2Pt7 << expected2Pt8 << expected2Pt9 << expected2Pt10 << expected2Pt11 );
}

void TestQgsPalLabeling::testGeometryGenerator()
{
  // test that no labels are drawn outside of the specified label boundary
  QgsPalLayerSettings settings;

  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ).family() );
  format.setSize( 12 );
  format.setNamedStyle( u"Bold"_s );
  format.setColor( QColor( 0, 0, 0 ) );
  settings.setFormat( format );

  settings.fieldName = u"'X'"_s;
  settings.isExpression = true;

  settings.placement = Qgis::LabelPlacement::OverPoint;
  settings.geometryGeneratorEnabled = true;
  settings.geometryGeneratorType = Qgis::GeometryType::Point;
  settings.geometryGenerator = "translate($geometry, 1, 0)";

  auto vl2 = std::make_unique<QgsVectorLayer>( u"Point?crs=epsg:4326&field=id:integer"_s, u"vl"_s, u"memory"_s );

  vl2->setRenderer( new QgsNullSymbolRenderer() );

  // DEBUG HINT:
  // Labels should be rendered with an offset of their original geometry. To debug, enable rendering the geometry itself.
  // vl2->setRenderer( new QgsSingleSymbolRenderer( new QgsMarkerSymbol() ) );


  QgsFeature f( vl2->fields(), 1 );

  for ( int x = 0; x < 17; x += 3 )
  {
    for ( int y = 0; y < 10; y += 3 )
    {
      f.setGeometry( std::make_unique<QgsPoint>( x, y ) );
      vl2->dataProvider()->addFeature( f );
    }
  }

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  vl2->setLabelsEnabled( true );

  // make a fake render context
  const QSize size( 640, 480 );
  QgsMapSettings mapSettings;
  QgsCoordinateReferenceSystem tgtCrs;
  tgtCrs.createFromString( u"EPSG:4326"_s );
  mapSettings.setDestinationCrs( tgtCrs );

  mapSettings.setOutputSize( size );
  mapSettings.setExtent( vl2->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << vl2.get() );
  mapSettings.setOutputDpi( 96 );

  QgsMapRendererSequentialJob job( mapSettings );
  job.start();
  job.waitForFinished();

  QImage img = job.renderedImage();
  QGSVERIFYIMAGECHECK( "geometry_generator_translated", "expected_geometry_generator_translated", img, "expected_geometry_generator_translated", 20, QSize(), 2 );

  // with rotation
  mapSettings.setRotation( 45 );
  QgsMapRendererSequentialJob job2( mapSettings );
  job2.start();
  job2.waitForFinished();

  img = job2.renderedImage();
  QGSVERIFYIMAGECHECK( "rotated_geometry_generator_translated", "expected_rotated_geometry_generator_translated", img, "expected_rotated_geometry_generator_translated", 20, QSize(), 2 );
}

void TestQgsPalLabeling::testLabelSettingsToFromMime()
{
  QgsPalLayerSettings settings;

  settings.fieldName = u"'X'"_s;
  settings.isExpression = true;
  settings.placement = Qgis::LabelPlacement::OverPoint;

  const QMimeData *md = settings.toMimeData();

  bool ok = false;
  QgsPalLayerSettings from_mime = QgsPalLayerSettings::fromMimeData( nullptr, &ok );
  QVERIFY( !ok );
  from_mime = QgsPalLayerSettings::fromMimeData( md, &ok );
  QVERIFY( ok );
  QCOMPARE( from_mime.fieldName, settings.fieldName );
  QCOMPARE( from_mime.isExpression, settings.isExpression );
  QCOMPARE( from_mime.placement, settings.placement );
}

QGSTEST_MAIN( TestQgsPalLabeling )
#include "testqgspallabeling.moc"
