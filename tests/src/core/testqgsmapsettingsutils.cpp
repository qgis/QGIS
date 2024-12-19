/***************************************************************************
                         testqgsmapsettingsutils.cpp
                         -----------------------
    begin                : May 2017
    copyright            : (C) 2017 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"

#include "qgsmapsettings.h"
#include "qgsmapsettingsutils.h"
#include "qgsvectorlayer.h"

#include <QString>

class TestQgsMapSettingsUtils : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase() {} // will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after each testfunction was executed.

    void createWorldFileContent(); //test world file content function
    void containsAdvancedEffects(); //test contains advanced effects function

  private:

    QgsMapSettings mMapSettings;

};

void TestQgsMapSettingsUtils::initTestCase()
{
  mMapSettings.setExtent( QgsRectangle( 0, 0, 1, 1 ) );
  mMapSettings.setOutputSize( QSize( 1, 1 ) );
}

void TestQgsMapSettingsUtils::createWorldFileContent()
{
  double a, b, c, d, e, f;
  QgsMapSettingsUtils::worldFileParameters( mMapSettings, a, b, c, d, e, f );
  QCOMPARE( a, 1.0 );
  QCOMPARE( b, 0.0 );
  QCOMPARE( c, 0.5 );
  QCOMPARE( d, 0.0 );
  QCOMPARE( e, -1.0 );
  QCOMPARE( f, 0.5 );

  QCOMPARE( QgsMapSettingsUtils::worldFileContent( mMapSettings ), QString( "1\r\n0\r\n0\r\n-1\r\n0.5\r\n0.5\r\n" ) );

  mMapSettings.setRotation( 45 );
  QCOMPARE( QgsMapSettingsUtils::worldFileContent( mMapSettings ), QString( "0.70710678118654757\r\n0.70710678118654746\r\n0.70710678118654746\r\n-0.70710678118654757\r\n0.5\r\n0.49999999999999994\r\n" ) );

  mMapSettings.setRotation( 145 );
  QCOMPARE( QgsMapSettingsUtils::worldFileContent( mMapSettings ), QString( "-0.81915204428899191\r\n0.57357643635104594\r\n0.57357643635104594\r\n0.81915204428899191\r\n0.5\r\n0.49999999999999994\r\n" ) );

  mMapSettings.setRotation( 0 );
  mMapSettings.setDevicePixelRatio( 2.0 );
  QgsMapSettingsUtils::worldFileParameters( mMapSettings, a, b, c, d, e, f );
  QCOMPARE( a, 0.5 );
  QCOMPARE( b, 0.0 );
  QCOMPARE( c, 0.5 );
  QCOMPARE( d, 0.0 );
  QCOMPARE( e, -0.5 );
  QCOMPARE( f, 0.5 );
}

void TestQgsMapSettingsUtils::containsAdvancedEffects()
{
  QgsMapSettings mapSettings = mMapSettings;

  std::unique_ptr< QgsVectorLayer > layer( new QgsVectorLayer( QStringLiteral( "Point?field=col1:real" ), QStringLiteral( "layer" ), QStringLiteral( "memory" ) ) );
  layer->setBlendMode( QPainter::CompositionMode_Multiply );

  QList<QgsMapLayer *> layers;
  layers << layer.get();
  mapSettings.setLayers( layers );

  QCOMPARE( QgsMapSettingsUtils::containsAdvancedEffects( mapSettings ).size(), 1 );
  QCOMPARE( QgsMapSettingsUtils::containsAdvancedEffects( mapSettings, QgsMapSettingsUtils::EffectsCheckFlag::IgnoreGeoPdfSupportedEffects ).size(), 0 );

  // set the layer scale-based visibility so it falls outside of the map settings scale
  layer->setScaleBasedVisibility( true );
  layer->setMaximumScale( 10 );

  QCOMPARE( QgsMapSettingsUtils::containsAdvancedEffects( mapSettings ).size(), 0 );

  layer->setScaleBasedVisibility( false );
  layer->setBlendMode( QPainter::CompositionMode_SourceOver );
  QCOMPARE( QgsMapSettingsUtils::containsAdvancedEffects( mapSettings ).size(), 0 );
  QCOMPARE( QgsMapSettingsUtils::containsAdvancedEffects( mapSettings, QgsMapSettingsUtils::EffectsCheckFlag::IgnoreGeoPdfSupportedEffects ).size(), 0 );

  // changing an individual layer's opacity is OK -- we don't want to force the whole map to be rasterized just because of this setting!
  // (just let that one layer get exported as raster instead)
  layer->setOpacity( 0.5 );
  QCOMPARE( QgsMapSettingsUtils::containsAdvancedEffects( mapSettings ).size(), 0 );
  QCOMPARE( QgsMapSettingsUtils::containsAdvancedEffects( mapSettings, QgsMapSettingsUtils::EffectsCheckFlag::IgnoreGeoPdfSupportedEffects ).size(), 0 );
}

QGSTEST_MAIN( TestQgsMapSettingsUtils )
#include "testqgsmapsettingsutils.moc"
