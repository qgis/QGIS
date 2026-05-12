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
#include "qgsabstract3dsymbol.h"
#include "qgsfillsymbol.h"
#include "qgsfillsymbollayer.h"
#include "qgsgoochmaterialsettings.h"
#include "qgsline3dsymbol.h"
#include "qgslinesymbol.h"
#include "qgslinesymbollayer.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgsmetalroughmaterialsettings.h"
#include "qgsnullmaterialsettings.h"
#include "qgsphongmaterialsettings.h"
#include "qgsphongtexturedmaterialsettings.h"
#include "qgspoint3dsymbol.h"
#include "qgspolygon3dsymbol.h"
#include "qgsrendercontext.h"
#include "qgssimplelinematerialsettings.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

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
      : QgsTest( u"3D Symbol Utils"_s, u"3d"_s )
    {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testvectorSymbolAverageColor();
    void testVectorSymbolPreviewIcon();
    void testSetVectorSymbolBaseColor();
    void testCopyVectorSymbolMaterial();
    void testCreate3DSymbolFrom2D();
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

void TestQgs3DSymbolUtils::testVectorSymbolPreviewIcon()
{
  const QgsScreenProperties screenProps;
  const int padding = 1;

  // point symbol
  auto sphere3DSymbol = std::make_unique<QgsPoint3DSymbol>();
  QgsPhongMaterialSettings phongSettings;
  phongSettings.setAmbient( QColor( 20, 50, 20 ) );
  phongSettings.setDiffuse( QColor( 80, 150, 80 ) );
  phongSettings.setSpecular( QColor( 100, 150, 100 ) );
  phongSettings.setShininess( 40.0 );
  phongSettings.setOpacity( 0.4 );
  sphere3DSymbol->setMaterialSettings( phongSettings.clone() );
  sphere3DSymbol->setShape( Qgis::Point3DShape::Sphere );
  QVariantMap vmSphere;
  vmSphere[u"radius"_s] = 15.0f;
  sphere3DSymbol->setShapeProperties( vmSphere );

  QSize iconSize( 64, 64 );
  const QIcon pointIcon = Qgs3DSymbolUtils::vectorSymbolPreviewIcon( sphere3DSymbol.get(), iconSize, screenProps, padding );
  QGSVERIFYIMAGECHECK( "icon_point", "icon_point", pointIcon.pixmap( iconSize ).toImage(), QString(), 0, QSize( 0, 0 ), 0 );

  // line symbol
  auto lineSymbol = std::make_unique<QgsLine3DSymbol>();
  lineSymbol->setRenderAsSimpleLines( true );
  QgsSimpleLineMaterialSettings lineMatSettings;
  lineMatSettings.setAmbient( Qt::red );
  lineSymbol->setMaterialSettings( lineMatSettings.clone() );
  lineSymbol->setWidth( 6.0f );

  iconSize = QSize( 64, 24 );
  const QIcon lineIcon = Qgs3DSymbolUtils::vectorSymbolPreviewIcon( lineSymbol.get(), iconSize, screenProps, padding );
  QGSVERIFYIMAGECHECK( "icon_line", "icon_line", lineIcon.pixmap( iconSize ).toImage(), QString(), 0, QSize( 0, 0 ), 0 );

  // polygon symbol
  auto polygonSymbol = std::make_unique<QgsPolygon3DSymbol>();
  QgsGoochMaterialSettings goochSettings;
  goochSettings.setWarm( QColor( 255, 220, 80 ) );
  goochSettings.setCool( QColor( 50, 80, 180 ) );
  goochSettings.setDiffuse( QColor( 120, 120, 100 ) );
  goochSettings.setSpecular( QColor( 255, 255, 180 ) );
  goochSettings.setShininess( 80.0 );
  goochSettings.setAlpha( 0.5 );
  goochSettings.setBeta( 0.5 );
  polygonSymbol->setMaterialSettings( goochSettings.clone() );
  polygonSymbol->setEdgesEnabled( true );
  polygonSymbol->setEdgeColor( Qt::blue );
  polygonSymbol->setEdgeWidth( 1.0f );

  iconSize = QSize( 64, 64 );
  const QIcon polygonIcon = Qgs3DSymbolUtils::vectorSymbolPreviewIcon( polygonSymbol.get(), iconSize, screenProps, padding );
  QGSVERIFYIMAGECHECK( "icon_polygon", "icon_polygon", polygonIcon.pixmap( iconSize ).toImage(), QString(), 0, QSize( 0, 0 ), 0 );
}

void TestQgs3DSymbolUtils::testSetVectorSymbolBaseColor()
{
  const QColor baseColor( 181, 140, 99 );

  // point symbol
  auto sphere3DSymbol = std::make_unique<QgsPoint3DSymbol>();
  QgsPhongMaterialSettings phongSettings;
  phongSettings.setAmbient( QColor( 20, 50, 20 ) );
  phongSettings.setDiffuse( QColor( 80, 150, 80 ) );
  phongSettings.setSpecular( QColor( 100, 150, 100 ) );
  phongSettings.setShininess( 40.0 );
  phongSettings.setOpacity( 0.4 );
  sphere3DSymbol->setMaterialSettings( phongSettings.clone() );

  QVERIFY( Qgs3DSymbolUtils::setVectorSymbolBaseColor( sphere3DSymbol.get(), baseColor ) );
  QgsPhongMaterialSettings *newPhongSettings = dynamic_cast<QgsPhongMaterialSettings *>( sphere3DSymbol->materialSettings() );
  QVERIFY( newPhongSettings );
  QCOMPARE( phongSettings.shininess(), 40.0f );
  QCOMPARE( phongSettings.opacity(), 0.4 );
  QCOMPARE( newPhongSettings->ambient().red(), 36 );
  QCOMPARE( newPhongSettings->ambient().green(), 28 );
  QCOMPARE( newPhongSettings->ambient().blue(), 20 );
  QCOMPARE( newPhongSettings->diffuse().red(), 174 );
  QCOMPARE( newPhongSettings->diffuse().green(), 134 );
  QCOMPARE( newPhongSettings->diffuse().blue(), 95 );
  QCOMPARE( newPhongSettings->specular().red(), 10 );
  QCOMPARE( newPhongSettings->specular().green(), 10 );
  QCOMPARE( newPhongSettings->specular().blue(), 10 );

  // line symbol
  auto lineSymbol = std::make_unique<QgsLine3DSymbol>();
  lineSymbol->setRenderAsSimpleLines( true );
  QgsSimpleLineMaterialSettings lineMatSettings;
  lineMatSettings.setAmbient( Qt::red );
  lineSymbol->setMaterialSettings( lineMatSettings.clone() );

  QVERIFY( Qgs3DSymbolUtils::setVectorSymbolBaseColor( lineSymbol.get(), baseColor ) );
  QgsSimpleLineMaterialSettings *newLineMatSettings = dynamic_cast<QgsSimpleLineMaterialSettings *>( lineSymbol->materialSettings() );
  QVERIFY( newLineMatSettings );
  QCOMPARE( newLineMatSettings->ambient().red(), baseColor.red() );
  QCOMPARE( newLineMatSettings->ambient().green(), baseColor.green() );
  QCOMPARE( newLineMatSettings->ambient().blue(), baseColor.blue() );

  // polygon symbol
  auto polygonSymbol = std::make_unique<QgsPolygon3DSymbol>();
  QgsGoochMaterialSettings goochSettings;
  goochSettings.setWarm( QColor( 255, 220, 80 ) );
  goochSettings.setCool( QColor( 50, 80, 180 ) );
  goochSettings.setDiffuse( QColor( 120, 120, 100 ) );
  goochSettings.setSpecular( QColor( 255, 255, 180 ) );
  goochSettings.setShininess( 80.0 );
  goochSettings.setAlpha( 0.5 );
  goochSettings.setBeta( 0.5 );
  polygonSymbol->setMaterialSettings( goochSettings.clone() );

  QVERIFY( Qgs3DSymbolUtils::setVectorSymbolBaseColor( polygonSymbol.get(), baseColor ) );
  QgsGoochMaterialSettings *newGoochSettings = dynamic_cast<QgsGoochMaterialSettings *>( polygonSymbol->materialSettings() );
  QVERIFY( newGoochSettings );
  QCOMPARE( goochSettings.shininess(), 80.0f );
  QCOMPARE( goochSettings.alpha(), 0.5 );
  QCOMPARE( goochSettings.beta(), 0.5 );
  QCOMPARE( newGoochSettings->warm().red(), 218 );
  QCOMPARE( newGoochSettings->warm().green(), 198 );
  QCOMPARE( newGoochSettings->warm().blue(), 50 );
  QCOMPARE( newGoochSettings->cool().red(), 91 );
  QCOMPARE( newGoochSettings->cool().green(), 70 );
  QCOMPARE( newGoochSettings->cool().blue(), 177 );
  QCOMPARE( newGoochSettings->diffuse().red(), 154 );
  QCOMPARE( newGoochSettings->diffuse().green(), 134 );
  QCOMPARE( newGoochSettings->diffuse().blue(), 113 );
  QCOMPARE( newGoochSettings->specular().red(), 102 );
  QCOMPARE( newGoochSettings->specular().green(), 102 );
  QCOMPARE( newGoochSettings->specular().blue(), 102 );
}

void TestQgs3DSymbolUtils::testCopyVectorSymbolMaterial()
{
  auto sphere3DSymbol = std::make_unique<QgsPoint3DSymbol>();
  QgsPhongMaterialSettings phongSettings;
  phongSettings.setAmbient( QColor( 20, 50, 20 ) );
  phongSettings.setDiffuse( QColor( 80, 150, 80 ) );
  phongSettings.setSpecular( QColor( 100, 150, 100 ) );
  phongSettings.setShininess( 40.0 );
  phongSettings.setOpacity( 0.4 );
  sphere3DSymbol->setMaterialSettings( phongSettings.clone() );

  auto sphere3DSymbol2 = std::make_unique<QgsPoint3DSymbol>();
  QgsGoochMaterialSettings goochSettings;
  goochSettings.setWarm( QColor( 255, 220, 80 ) );
  goochSettings.setCool( QColor( 50, 80, 180 ) );
  goochSettings.setDiffuse( QColor( 120, 120, 100 ) );
  goochSettings.setSpecular( QColor( 255, 255, 180 ) );
  goochSettings.setShininess( 80.0 );
  goochSettings.setAlpha( 0.5 );
  goochSettings.setBeta( 0.5 );
  sphere3DSymbol2->setMaterialSettings( goochSettings.clone() );

  QVERIFY( Qgs3DSymbolUtils::copyVectorSymbolMaterial( sphere3DSymbol.get(), sphere3DSymbol2.get() ) );
  const QgsAbstractMaterialSettings *newSettings = sphere3DSymbol2->materialSettings();
  QCOMPARE( newSettings->type(), "phong"_L1 );
  const QgsPhongMaterialSettings *newPhongSettings = dynamic_cast<const QgsPhongMaterialSettings *>( newSettings );
  QVERIFY( newPhongSettings );
  QCOMPARE( newPhongSettings->ambient().red(), phongSettings.ambient().red() );
  QCOMPARE( newPhongSettings->ambient().green(), phongSettings.ambient().green() );
  QCOMPARE( newPhongSettings->ambient().blue(), phongSettings.ambient().blue() );
  QCOMPARE( newPhongSettings->diffuse().red(), phongSettings.diffuse().red() );
  QCOMPARE( newPhongSettings->diffuse().green(), phongSettings.diffuse().green() );
  QCOMPARE( newPhongSettings->diffuse().blue(), phongSettings.diffuse().blue() );
  QCOMPARE( newPhongSettings->specular().red(), phongSettings.specular().red() );
  QCOMPARE( newPhongSettings->specular().green(), phongSettings.specular().green() );
  QCOMPARE( newPhongSettings->specular().blue(), phongSettings.specular().blue() );
  QCOMPARE( newPhongSettings->opacity(), phongSettings.opacity() );
  QCOMPARE( newPhongSettings->shininess(), phongSettings.shininess() );
}

void TestQgs3DSymbolUtils::testCreate3DSymbolFrom2D()
{
  auto layerPolygon = std::make_unique<QgsVectorLayer>( u"Polygon?crs=EPSG:4326"_s, u"test_polygon"_s, u"memory"_s );
  QVERIFY( layerPolygon->isValid() );
  QCOMPARE( layerPolygon->wkbType(), Qgis::WkbType::Polygon );

  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( QSize( 400, 400 ) );
  mapSettings.setOutputDpi( 96 );
  const QgsRenderContext context = QgsRenderContext::fromMapSettings( mapSettings );

  // null 2d symbol
  std::unique_ptr<QgsAbstract3DSymbol> symbol3D = Qgs3DSymbolUtils::create3DSymbolFrom2D( layerPolygon.get(), nullptr, context );
  QVERIFY( !symbol3D );

  // polygon
  {
    auto fillSymbol2D = std::make_unique<QgsFillSymbol>();
    auto fillSymbolLayer = std::make_unique<QgsSimpleFillSymbolLayer>();
    fillSymbolLayer->setFillColor( QColor( 0, 255, 255 ) );
    fillSymbol2D->changeSymbolLayer( 0, fillSymbolLayer.release() );

    auto symbol3D = Qgs3DSymbolUtils::create3DSymbolFrom2D( layerPolygon.get(), fillSymbol2D.get(), context );

    QVERIFY( symbol3D );
    QCOMPARE( symbol3D->type(), u"polygon"_s );
    const QgsPolygon3DSymbol *polygonSymbol = dynamic_cast<const QgsPolygon3DSymbol *>( symbol3D.get() );
    QVERIFY( polygonSymbol );

    const QgsAbstractMaterialSettings *material = polygonSymbol->materialSettings();
    QVERIFY( material );
    QCOMPARE( material->averageColor().red(), 0 );
    QCOMPARE( material->averageColor().green(), 255 );
    QCOMPARE( material->averageColor().blue(), 255 );
  }

  // point
  {
    auto layerPoint = std::make_unique<QgsVectorLayer>( u"Point?crs=EPSG:4326"_s, u"test_point"_s, u"memory"_s );
    QVERIFY( layerPoint->isValid() );
    QCOMPARE( layerPoint->wkbType(), Qgis::WkbType::Point );

    auto markerSymbol = std::make_unique<QgsMarkerSymbol>();
    auto markerSymbolLayer = std::make_unique<QgsSimpleMarkerSymbolLayer>( Qgis::MarkerShape::Circle, 4.0, 0.0, Qgis::ScaleMethod::ScaleDiameter, QColor( 120, 0, 0 ), QColor() );
    markerSymbol->changeSymbolLayer( 0, markerSymbolLayer.release() );
    markerSymbol->setOpacity( 0.5 );
    markerSymbol->setSizeUnit( Qgis::RenderUnit::Pixels );

    std::unique_ptr<QgsAbstract3DSymbol> symbol3D = Qgs3DSymbolUtils::create3DSymbolFrom2D( layerPoint.get(), markerSymbol.get(), context );

    QVERIFY( symbol3D );
    QCOMPARE( symbol3D->type(), u"point"_s );
    QgsPoint3DSymbol *point3DSymbol = dynamic_cast<QgsPoint3DSymbol *>( symbol3D.get() );
    QVERIFY( point3DSymbol );
    QCOMPARE( point3DSymbol->shape(), Qgis::Point3DShape::Sphere );

    QVariantMap props = point3DSymbol->shapeProperties();
    QVERIFY( props.contains( "radius" ) );
    QCOMPARE( props.value( "radius" ).toDouble(), 2.0 );

    QgsAbstractMaterialSettings *material = point3DSymbol->materialSettings();
    QVERIFY( material );
    QCOMPARE( material->type(), u"metalrough"_s );
    const QgsMetalRoughMaterialSettings *metalMaterial = dynamic_cast<const QgsMetalRoughMaterialSettings *>( material );
    QVERIFY( metalMaterial );
    QCOMPARE( metalMaterial->opacity(), 0.5 );
    QCOMPARE( metalMaterial->averageColor().red(), 120 );
    QCOMPARE( metalMaterial->averageColor().green(), 0 );
    QCOMPARE( metalMaterial->averageColor().blue(), 0 );
  }

  // line
  {
    auto layerLine = std::make_unique<QgsVectorLayer>( u"LineString?crs=EPSG:4326"_s, u"test_linestring"_s, u"memory"_s );
    QVERIFY( layerLine->isValid() );
    QCOMPARE( layerLine->wkbType(), Qgis::WkbType::LineString );

    auto lineSymbolLayer = std::make_unique<QgsSimpleLineSymbolLayer>();
    lineSymbolLayer->setWidth( 5.0 );
    lineSymbolLayer->setWidthUnit( Qgis::RenderUnit::Pixels );
    auto symbol2D = std::make_unique<QgsLineSymbol>();
    symbol2D->changeSymbolLayer( 0, lineSymbolLayer.release() );

    auto symbol3D = Qgs3DSymbolUtils::create3DSymbolFrom2D( layerLine.get(), symbol2D.get(), context );
    QVERIFY( symbol3D );
    QCOMPARE( symbol3D->type(), u"line"_s );

    const QgsLine3DSymbol *lineSymbol3D = dynamic_cast<const QgsLine3DSymbol *>( symbol3D.get() );
    QVERIFY( lineSymbol3D );
    QCOMPARE( static_cast<double>( lineSymbol3D->width() ), 5.0 );

    QgsAbstractMaterialSettings *material = lineSymbol3D->materialSettings();
    QVERIFY( material );
    QCOMPARE( material->type(), u"metalrough"_s );
    const QgsMetalRoughMaterialSettings *metalMaterial = dynamic_cast<const QgsMetalRoughMaterialSettings *>( material );
    QVERIFY( metalMaterial );
    QCOMPARE( metalMaterial->opacity(), 1.0 );
    QCOMPARE( metalMaterial->averageColor().red(), 35 );
    QCOMPARE( metalMaterial->averageColor().green(), 35 );
    QCOMPARE( metalMaterial->averageColor().blue(), 35 );
  }
}

QGSTEST_MAIN( TestQgs3DSymbolUtils )
#include "testqgs3dsymbolutils.moc"
