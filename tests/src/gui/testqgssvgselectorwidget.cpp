/***************************************************************************
    testqgssvgselectorwidget.cpp
    ---------------------
    begin                : 2024/09/25
    copyright            : (C) 2024 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QPixmapCache>
#include <QTemporaryFile>

#include "qgstest.h"
#include "qgssvgselectorwidget.h"

class TestQgsSvgSelectorWidget : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsSvgSelectorWidget()
      : QgsTest( QStringLiteral( "SVG Selector Widget Tests" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void testPixmapCache();
};

void TestQgsSvgSelectorWidget::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsSvgSelectorWidget::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsSvgSelectorWidget::init()
{
}

void TestQgsSvgSelectorWidget::cleanup()
{
}

void TestQgsSvgSelectorWidget::testPixmapCache()
{
  // We want to check that QPixmapCache is correctly used
  // So we force a cache image different from the one we set to the model to check that this is
  // the cache image returned, not the model svg one

  const QPixmap cachePixmap( QStringLiteral( "%1/rgb256x256.png" ).arg( TEST_DATA_DIR ) );
  const QString sampleSvg = QStringLiteral( "%1/sample_svg.svg" ).arg( TEST_DATA_DIR );
  QPixmapCache::insert( sampleSvg, cachePixmap );

  QgsSvgSelectorListModel model( nullptr );
  model.addSvgs( QStringList() << sampleSvg );

  const QPixmap pixmap = model.data( model.index( 0, 0 ), Qt::DecorationRole ).value<QPixmap>();
  QVERIFY( !pixmap.isNull() );

  QCOMPARE( pixmap, cachePixmap );
}


QGSTEST_MAIN( TestQgsSvgSelectorWidget )
#include "testqgssvgselectorwidget.moc"
