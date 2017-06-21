/***************************************************************************
     testqgsdatadefinedsizelegend.cpp
     --------------------------------------
    Date                 : June 2017
    Copyright            : (C) 2017 by Martin Dobias
    Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include "qgsdatadefinedsizelegend.h"
#include "qgsfontutils.h"
#include "qgsrenderchecker.h"
#include "qgssymbol.h"


static QString _fileNameForTest( const QString &testName )
{
  return QDir::tempPath() + '/' + testName + ".png";
}

static bool _verifyImage( const QString &testName, QString &report )
{
  QgsRenderChecker checker;
  checker.setControlPathPrefix( QStringLiteral( "data_defined_size_legend" ) );
  checker.setControlName( "expected_" + testName );
  checker.setRenderedImage( _fileNameForTest( testName ) );
  checker.setSizeTolerance( 3, 3 );
  bool equal = checker.compareImages( testName, 500 );
  report += checker.report();
  return equal;
}

static QgsRenderContext _createRenderContext( double mupp, double dpi, double scale )
{
  QgsRenderContext context;
  context.setScaleFactor( dpi / 25.4 );
  context.setRendererScale( scale );
  context.setMapToPixel( QgsMapToPixel( mupp ) );
  return context;
}


/** \ingroup UnitTests
 * This is a unit test for legend rendering when using data-defined size of markers.
 */
class TestQgsDataDefinedSizeLegend : public QObject
{
    Q_OBJECT

  public:

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void testBasic();
    void testCrowded();

  private:
    QString mReport;
};

void TestQgsDataDefinedSizeLegend::initTestCase()
{
  // Runs once before any tests are run
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  mReport += QLatin1String( "<h1>Data Defined Size Legend Tests</h1>\n" );
}

void TestQgsDataDefinedSizeLegend::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  QgsApplication::exitQgis();
}

void TestQgsDataDefinedSizeLegend::testBasic()
{
  QgsDataDefinedSizeLegend settings;
  settings.setLegendType( QgsDataDefinedSizeLegend::LegendCollapsed );
  settings.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );

  QList<QgsDataDefinedSizeLegend::SizeClass> classes;
  classes << QgsDataDefinedSizeLegend::SizeClass( 3.,  QString( "3" ) );
  classes << QgsDataDefinedSizeLegend::SizeClass( 15., QString( "15" ) );
  classes << QgsDataDefinedSizeLegend::SizeClass( 30., QString( "30" ) );
  settings.setClasses( classes );

  QgsStringMap props;
  props["name"] = "circle";
  props["color"] = "200,255,200";
  props["outline_color"] = "0,255,0";

  settings.setSymbol( QgsMarkerSymbol::createSimple( props ) );  // takes ownership

  QgsRenderContext context( _createRenderContext( 100, 96, 100 ) );

  QImage imgBottom = settings.collapsedLegendImage( context, 1 );
  imgBottom.save( _fileNameForTest( "basic_bottom" ) );
  QVERIFY( _verifyImage( "basic_bottom", mReport ) );

  settings.setVerticalAlignment( QgsDataDefinedSizeLegend::AlignCenter );

  QImage imgCenter = settings.collapsedLegendImage( context, 1 );
  imgCenter.save( _fileNameForTest( "basic_center" ) );
  QVERIFY( _verifyImage( "basic_center", mReport ) );
}

void TestQgsDataDefinedSizeLegend::testCrowded()
{
  QgsDataDefinedSizeLegend settings;
  settings.setLegendType( QgsDataDefinedSizeLegend::LegendCollapsed );
  settings.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );

  QList<QgsDataDefinedSizeLegend::SizeClass> classes;
  classes << QgsDataDefinedSizeLegend::SizeClass( 2.,  QString( "2" ) );
  classes << QgsDataDefinedSizeLegend::SizeClass( 5.,  QString( "5" ) );
  classes << QgsDataDefinedSizeLegend::SizeClass( 10., QString( "10" ) );
  classes << QgsDataDefinedSizeLegend::SizeClass( 12., QString( "12" ) );
  classes << QgsDataDefinedSizeLegend::SizeClass( 15., QString( "15" ) );
  classes << QgsDataDefinedSizeLegend::SizeClass( 18., QString( "18" ) );
  settings.setClasses( classes );

  QgsStringMap props;
  props["name"] = "circle";
  props["color"] = "200,255,200";
  props["outline_color"] = "0,255,0";

  settings.setSymbol( QgsMarkerSymbol::createSimple( props ) );  // takes ownership

  QgsRenderContext context( _createRenderContext( 100, 96, 100 ) );

  QImage img = settings.collapsedLegendImage( context, 1 );
  img.save( _fileNameForTest( "crowded" ) );

  QVERIFY( _verifyImage( "crowded", mReport ) );
}

QGSTEST_MAIN( TestQgsDataDefinedSizeLegend )
#include "testqgsdatadefinedsizelegend.moc"
