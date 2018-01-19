/***************************************************************************
  testqgsdxfexport.cpp
  --------------------
  Date                 : November 2017
  Copyright            : (C) 2017 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include "qgsapplication.h"
#include "qgsdxfexport.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsfontutils.h"
#include "qgstextrenderer.h"
#include "qgspallabeling.h"
#include "qgslabelingengine.h"
#include "qgsvectorlayerlabeling.h"
#include <QTemporaryFile>

class TestQgsDxfExport : public QObject
{
    Q_OBJECT
  public:
    TestQgsDxfExport() = default;

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void testPoints();
    void testLines();
    void testPolygons();
    void testMtext();
    void testText();

  private:
    QgsVectorLayer *mPointLayer = nullptr;
    QgsVectorLayer *mLineLayer = nullptr;
    QgsVectorLayer *mPolygonLayer = nullptr;

    QString mReport;

    void setDefaultLabelParams( QgsPalLayerSettings &settings );
    QString getTempFileName( const QString &file ) const;

    bool fileContainsText( const QString &path, const QString &text ) const;
};

void TestQgsDxfExport::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  QgsFontUtils::loadStandardTestFonts( QStringList() << QStringLiteral( "Bold" ) );
}

void TestQgsDxfExport::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsDxfExport::init()
{
  QString filename = QStringLiteral( TEST_DATA_DIR ) + "/points.shp";
  mPointLayer = new QgsVectorLayer( filename, QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( mPointLayer->isValid() );
  QgsProject::instance()->addMapLayer( mPointLayer );
  filename = QStringLiteral( TEST_DATA_DIR ) + "/lines.shp";
  mLineLayer = new QgsVectorLayer( filename, QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( mLineLayer->isValid() );
  QgsProject::instance()->addMapLayer( mLineLayer );
  filename = QStringLiteral( TEST_DATA_DIR ) + "/polys.shp";
  mPolygonLayer = new QgsVectorLayer( filename, QStringLiteral( "polygons" ), QStringLiteral( "ogr" ) );
  QVERIFY( mPolygonLayer->isValid() );
  QgsProject::instance()->addMapLayer( mPolygonLayer );
}

void TestQgsDxfExport::cleanup()
{
  QgsProject::instance()->removeMapLayer( mPointLayer->id() );
  mPointLayer = nullptr;
}

void TestQgsDxfExport::testPoints()
{
  QgsDxfExport d;
  d.addLayers( QList< QPair< QgsVectorLayer *, int > >() << qMakePair( mPointLayer, -1 ) );

  QgsMapSettings mapSettings;
  QSize size( 640, 480 );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( mPointLayer->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << mPointLayer );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( mPointLayer->crs() );

  d.setMapSettings( mapSettings );
  d.setSymbologyScale( 1000 );

  QString file = getTempFileName( "point_dxf" );
  QFile dxfFile( file );
  QCOMPARE( d.writeToFile( &dxfFile, QStringLiteral( "CP1252" ) ), 0 );
  dxfFile.close();

  // reload and compare
  std::unique_ptr< QgsVectorLayer > result = qgis::make_unique< QgsVectorLayer >( file, "dxf" );
  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), mPointLayer->featureCount() );
  QCOMPARE( result->wkbType(), QgsWkbTypes::Point );
}

void TestQgsDxfExport::testLines()
{
  QgsDxfExport d;
  d.addLayers( QList< QPair< QgsVectorLayer *, int > >() << qMakePair( mLineLayer, -1 ) );

  QgsMapSettings mapSettings;
  QSize size( 640, 480 );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( mLineLayer->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << mLineLayer );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( mLineLayer->crs() );

  d.setMapSettings( mapSettings );
  d.setSymbologyScale( 1000 );

  QString file = getTempFileName( "line_dxf" );
  QFile dxfFile( file );
  QCOMPARE( d.writeToFile( &dxfFile, QStringLiteral( "CP1252" ) ), 0 );
  dxfFile.close();

  // reload and compare
  std::unique_ptr< QgsVectorLayer > result = qgis::make_unique< QgsVectorLayer >( file, "dxf" );
  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), mLineLayer->featureCount() );
  QCOMPARE( result->wkbType(), QgsWkbTypes::LineString );
}

void TestQgsDxfExport::testPolygons()
{
  QgsDxfExport d;
  d.addLayers( QList< QPair< QgsVectorLayer *, int > >() << qMakePair( mPolygonLayer, -1 ) );

  QgsMapSettings mapSettings;
  QSize size( 640, 480 );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( mPolygonLayer->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << mPolygonLayer );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( mPolygonLayer->crs() );

  d.setMapSettings( mapSettings );
  d.setSymbologyScale( 1000 );

  QString file = getTempFileName( "polygon_dxf" );
  QFile dxfFile( file );
  QCOMPARE( d.writeToFile( &dxfFile, QStringLiteral( "CP1252" ) ), 0 );
  dxfFile.close();

  // reload and compare
  std::unique_ptr< QgsVectorLayer > result = qgis::make_unique< QgsVectorLayer >( file, "dxf" );
  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), 12L );
  QCOMPARE( result->wkbType(), QgsWkbTypes::LineString );
}

void TestQgsDxfExport::testMtext()
{
  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );
  mPointLayer->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  mPointLayer->setLabelsEnabled( true );

  QgsDxfExport d;
  d.addLayers( QList< QPair< QgsVectorLayer *, int > >() << qMakePair( mPointLayer, -1 ) );

  QgsMapSettings mapSettings;
  QSize size( 640, 480 );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( mPointLayer->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << mPointLayer );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( mPointLayer->crs() );

  d.setMapSettings( mapSettings );
  d.setSymbologyScale( 1000 );
  d.setSymbologyExport( QgsDxfExport::FeatureSymbology );

  QString file = getTempFileName( "mtext_dxf" );
  QFile dxfFile( file );
  QCOMPARE( d.writeToFile( &dxfFile, QStringLiteral( "CP1252" ) ), 0 );
  dxfFile.close();


  QVERIFY( fileContainsText( file, "MTEXT\n"
                             "  5\n"
                             "dd\n"
                             "100\n"
                             "AcDbEntity\n"
                             "100\n"
                             "AcDbMText\n"
                             "  8\n"
                             "points\n"
                             "420\n"
                             "**no check**\n"
                             " 10\n"
                             "**no check**\n"
                             " 20\n"
                             "**no check**\n"
                             "  1\n"
                             "\\fQGIS Vera Sans|i0|b1;\\H3.81136;Biplane\n"
                             " 50\n"
                             "0.0\n"
                             " 41\n"
                             "**no check**\n"
                             " 71\n"
                             "     7\n"
                             "  7\n"
                             "STANDARD\n"
                             "  0" ) );
}

void TestQgsDxfExport::testText()
{
  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );
  settings.setFormat( format );
  mPointLayer->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  mPointLayer->setLabelsEnabled( true );

  QgsDxfExport d;
  d.addLayers( QList< QPair< QgsVectorLayer *, int > >() << qMakePair( mPointLayer, -1 ) );

  QgsMapSettings mapSettings;
  QSize size( 640, 480 );
  mapSettings.setOutputSize( size );
  mapSettings.setExtent( mPointLayer->extent() );
  mapSettings.setLayers( QList<QgsMapLayer *>() << mPointLayer );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( mPointLayer->crs() );

  d.setMapSettings( mapSettings );
  d.setSymbologyScale( 1000 );
  d.setSymbologyExport( QgsDxfExport::FeatureSymbology );
  d.setFlags( QgsDxfExport::FlagNoMText );

  QString file = getTempFileName( "text_dxf" );
  QFile dxfFile( file );
  QCOMPARE( d.writeToFile( &dxfFile, QStringLiteral( "CP1252" ) ), 0 );
  dxfFile.close();


  QVERIFY( fileContainsText( file, "TEXT\n"
                             "  5\n"
                             "dd\n"
                             "100\n"
                             "AcDbEntity\n"
                             "100\n"
                             "AcDbText\n"
                             "  8\n"
                             "points\n"
                             "420\n"
                             "**no check**\n"
                             " 10\n"
                             "**no check**\n"
                             " 20\n"
                             "**no check**\n"
                             " 40\n"
                             "**no check**\n"
                             "  1\n"
                             "Biplane\n"
                             " 50\n"
                             "0.0\n"
                             "  7\n"
                             "STANDARD\n"
                             "100\n"
                             "AcDbText" ) );
}

bool TestQgsDxfExport::fileContainsText( const QString &path, const QString &text ) const
{
  QStringList searchLines = text.split( '\n' );
  QFile file( path );
  if ( !file.open( QIODevice::ReadOnly ) )
    return false;
  QTextStream in( &file );
  QString line;
  do
  {
    bool found = true;
    for ( const QString &searchLine : searchLines )
    {
      line = in.readLine();
      if ( searchLine != QLatin1String( "**no check**" ) && line != searchLine )
      {
        found = false;
        break;
      }
      int i = 1;
      i++;
    }
    if ( found )
      return true;
  }
  while ( !line.isNull() );
  return false;
}

QString TestQgsDxfExport::getTempFileName( const QString &file ) const
{
  return QDir::tempPath() + '/' + file + ".dxf";
}


QGSTEST_MAIN( TestQgsDxfExport )
#include "testqgsdxfexport.moc"
