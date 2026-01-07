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

#include "qgsdatadefinedsizelegend.h"
#include "qgsfontutils.h"
#include "qgsmarkersymbol.h"
#include "qgstest.h"

static QgsRenderContext _createRenderContext( double mupp, double dpi, double scale )
{
  QgsRenderContext context;
  context.setScaleFactor( dpi / 25.4 );
  context.setRendererScale( scale );
  context.setMapToPixel( QgsMapToPixel( mupp ) );
  context.setFlag( Qgis::RenderContextFlag::Antialiasing, true );
  return context;
}


/**
 * \ingroup UnitTests
 * This is a unit test for legend rendering when using data-defined size of markers.
 */
class TestQgsDataDefinedSizeLegend : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsDataDefinedSizeLegend()
      : QgsTest( u"Data Defined Size Legend Tests"_s, u"data_defined_size_legend"_s ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testBasic();
    void testCrowded();
};

void TestQgsDataDefinedSizeLegend::initTestCase()
{
  // Runs once before any tests are run
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
}

void TestQgsDataDefinedSizeLegend::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsDataDefinedSizeLegend::testBasic()
{
  QgsDataDefinedSizeLegend settings;
  settings.setLegendType( QgsDataDefinedSizeLegend::LegendCollapsed );
  settings.setFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) );

  QList<QgsDataDefinedSizeLegend::SizeClass> classes;
  classes << QgsDataDefinedSizeLegend::SizeClass( 3., u"3"_s );
  classes << QgsDataDefinedSizeLegend::SizeClass( 15., u"15"_s );
  classes << QgsDataDefinedSizeLegend::SizeClass( 30., u"30"_s );
  settings.setClasses( classes );

  QVariantMap props;
  props[u"name"_s] = u"circle"_s;
  props[u"color"_s] = u"200,255,200"_s;
  props[u"outline_color"_s] = u"0,255,0"_s;

  settings.setSymbol( QgsMarkerSymbol::createSimple( props ).release() ); // takes ownership

  QgsRenderContext context( _createRenderContext( 100, 96, 100 ) );

  const QImage imgBottom = settings.collapsedLegendImage( context, Qt::white, 1 );
  QGSVERIFYIMAGECHECK( "basic_bottom", "basic_bottom", imgBottom, QString(), 500, QSize( 3, 3 ) );

  settings.setVerticalAlignment( QgsDataDefinedSizeLegend::AlignCenter );

  const QImage imgCenter = settings.collapsedLegendImage( context, Qt::white, 1 );
  QGSVERIFYIMAGECHECK( "basic_center", "basic_center", imgCenter, QString(), 500, QSize( 3, 3 ) );
}

void TestQgsDataDefinedSizeLegend::testCrowded()
{
  QgsDataDefinedSizeLegend settings;
  settings.setLegendType( QgsDataDefinedSizeLegend::LegendCollapsed );
  settings.setFont( QgsFontUtils::getStandardTestFont( u"Bold"_s ) );

  QList<QgsDataDefinedSizeLegend::SizeClass> classes;
  classes << QgsDataDefinedSizeLegend::SizeClass( 2., u"2"_s );
  classes << QgsDataDefinedSizeLegend::SizeClass( 5., u"5"_s );
  classes << QgsDataDefinedSizeLegend::SizeClass( 10., u"10"_s );
  classes << QgsDataDefinedSizeLegend::SizeClass( 12., u"12"_s );
  classes << QgsDataDefinedSizeLegend::SizeClass( 15., u"15"_s );
  classes << QgsDataDefinedSizeLegend::SizeClass( 18., u"18"_s );
  settings.setClasses( classes );

  QVariantMap props;
  props[u"name"_s] = u"circle"_s;
  props[u"color"_s] = u"200,255,200"_s;
  props[u"outline_color"_s] = u"0,255,0"_s;

  settings.setSymbol( QgsMarkerSymbol::createSimple( props ).release() ); // takes ownership

  QgsRenderContext context( _createRenderContext( 100, 96, 100 ) );

  const QImage img = settings.collapsedLegendImage( context, Qt::white, 1 );
  QGSVERIFYIMAGECHECK( "crowded", "crowded", img, QString(), 500, QSize( 3, 3 ) );
}

QGSTEST_MAIN( TestQgsDataDefinedSizeLegend )
#include "testqgsdatadefinedsizelegend.moc"
