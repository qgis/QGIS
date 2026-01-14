/***************************************************************************
     testqgs3dsymbolutils.cpp
     ----------------------
    Date                 : January 2026
    Copyright            : (C) 2026 by Jean Felder
    Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3d.h"
#include "qgs3dsymbolutils.h"
#include "qgsgoochmaterialsettings.h"
#include "qgsline3dsymbol.h"
#include "qgsmetalroughmaterialsettings.h"
#include "qgsnullmaterialsettings.h"
#include "qgsphongmaterialsettings.h"
#include "qgsphongtexturedmaterialsettings.h"
#include "qgssimplelinematerialsettings.h"
#include "qgstest.h"

#include <QString>

using namespace Qt::StringLiterals;

/**
 * \ingroup UnitTests
 * This is a unit test for Qgs3DSymbolUtils
 */
class TestQgs3DSymbolUtils : public QgsTest
{
    Q_OBJECT
  public:
    TestQgs3DSymbolUtils()
      : QgsTest( u"3D Symbol Utils"_s, u"3d"_s ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testvectorSymbolAverageColor();
};

//runs before all tests
void TestQgs3DSymbolUtils::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
  Qgs3D::initialize();
}

//runs after all tests
void TestQgs3DSymbolUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgs3DSymbolUtils::testvectorSymbolAverageColor()
{
  // phong material
  // Matte green
  auto lineSymbol = std::make_unique<QgsLine3DSymbol>();
  QgsPhongMaterialSettings phongSettings;
  phongSettings.setAmbient( QColor( 20, 50, 20 ) );
  phongSettings.setDiffuse( QColor( 80, 150, 80 ) );
  phongSettings.setSpecular( QColor( 100, 150, 100 ) );
  phongSettings.setShininess( 40.0 );

  lineSymbol->setMaterialSettings( phongSettings.clone() );
  const QColor averageColorP = Qgs3DSymbolUtils::vectorSymbolAverageColor( lineSymbol.get() );
  QCOMPARE( averageColorP.red(), 70 );
  QCOMPARE( averageColorP.green(), 140 );
  QCOMPARE( averageColorP.blue(), 70 );
  QCOMPARE( averageColorP.alpha(), 255 );

  // phong textured
  // red
  QgsPhongTexturedMaterialSettings phongTexturedSettings;
  phongTexturedSettings.setAmbient( QColor( 230, 20, 20 ) );
  phongTexturedSettings.setSpecular( QColor( 255, 220, 220 ) );
  phongTexturedSettings.setDiffuseTexturePath( testDataPath( "/3d/texture_red.png" ) );
  QFileInfo textureInfo( phongTexturedSettings.diffuseTexturePath() );
  QVERIFY( textureInfo.exists() );
  phongTexturedSettings.setShininess( 150.0 );
  phongTexturedSettings.setOpacity( 1.0 );

  lineSymbol->setMaterialSettings( phongTexturedSettings.clone() );
  const QColor averageColorPT = Qgs3DSymbolUtils::vectorSymbolAverageColor( lineSymbol.get() );
  QCOMPARE( averageColorPT.red(), 90 );
  QCOMPARE( averageColorPT.green(), 14 );
  QCOMPARE( averageColorPT.blue(), 14 );
  QCOMPARE( averageColorPT.alpha(), 255 );

  // gooch
  // warm beige-gold
  QgsGoochMaterialSettings goochSettings;
  goochSettings.setWarm( QColor( 255, 220, 80 ) );
  goochSettings.setCool( QColor( 50, 80, 180 ) );
  goochSettings.setDiffuse( QColor( 120, 120, 100 ) );
  goochSettings.setSpecular( QColor( 255, 255, 180 ) );
  goochSettings.setShininess( 80.0 );
  goochSettings.setAlpha( 0.5 );
  goochSettings.setBeta( 0.5 );

  lineSymbol->setMaterialSettings( goochSettings.clone() );
  const QColor averageColorG = Qgs3DSymbolUtils::vectorSymbolAverageColor( lineSymbol.get() );
  QCOMPARE( averageColorG.red(), 221 );
  QCOMPARE( averageColorG.green(), 219 );
  QCOMPARE( averageColorG.blue(), 180 );
  QCOMPARE( averageColorG.alpha(), 255 );

  // metal
  // reddish copper
  QgsMetalRoughMaterialSettings metalSettings;
  const QColor metalColor( 220, 140, 80 );
  metalSettings.setBaseColor( metalColor );
  lineSymbol->setMaterialSettings( metalSettings.clone() );
  const QColor averageColorM = Qgs3DSymbolUtils::vectorSymbolAverageColor( lineSymbol.get() );

  QCOMPARE( averageColorM.red(), metalColor.red() );
  QCOMPARE( averageColorM.green(), metalColor.green() );
  QCOMPARE( averageColorM.blue(), metalColor.blue() );
  QCOMPARE( averageColorM.alpha(), metalColor.alpha() );

  // simple line
  // green
  QgsSimpleLineMaterialSettings simpleLineSettings;
  const QColor simpleLineColor( 3, 230, 40, 33 );
  simpleLineSettings.setAmbient( simpleLineColor );
  lineSymbol->setMaterialSettings( simpleLineSettings.clone() );
  const QColor averageColorSL = Qgs3DSymbolUtils::vectorSymbolAverageColor( lineSymbol.get() );

  QCOMPARE( averageColorSL.red(), simpleLineColor.red() );
  QCOMPARE( averageColorSL.green(), simpleLineColor.green() );
  QCOMPARE( averageColorSL.blue(), simpleLineColor.blue() );
  QCOMPARE( averageColorSL.alpha(), simpleLineColor.alpha() );

  // null material
  // fallback black color
  QgsNullMaterialSettings nullSettings;
  lineSymbol->setMaterialSettings( nullSettings.clone() );
  const QColor averageColorN = Qgs3DSymbolUtils::vectorSymbolAverageColor( lineSymbol.get() );

  QCOMPARE( averageColorN.red(), 0 );
  QCOMPARE( averageColorN.green(), 0 );
  QCOMPARE( averageColorN.blue(), 0 );
  QCOMPARE( averageColorN.alpha(), 255 );
}

QGSTEST_MAIN( TestQgs3DSymbolUtils )
#include "testqgs3dsymbolutils.moc"
