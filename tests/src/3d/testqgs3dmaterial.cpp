/***************************************************************************
                         TestQgs3DMaterial.cpp
                         -----------------------
    begin                : October 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaterial.h"
#include "qgsmaterialregistry.h"
#include "qgsphongmaterialsettings.h"
#include "qgsgoochmaterialsettings.h"
#include "qgssimplelinematerialsettings.h"

#include <QObject>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QTechnique>

#include "qgstest.h"
#include "qgs3d.h"


class TestQgs3dMaterial : public QgsTest
{
    Q_OBJECT

  public:
    TestQgs3dMaterial()
      : QgsTest( QStringLiteral( "3D Material Tests" ), QStringLiteral( "3d" ) )
    {}

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void colorDataDefinedPhong();
    void colorDataDefinedGooch();
    void clipping();

  private:
    void setColorProperty( const QgsProperty &property, QgsAbstractMaterialSettings::Property propertyType, QgsPropertyCollection &collection, QgsAbstractMaterialSettings &materialSettings );
};

void TestQgs3dMaterial::initTestCase()
{
  QgsApplication::init(); // init paths for CRS lookup
  QgsApplication::initQgis();
}

void TestQgs3dMaterial::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgs3dMaterial::init()
{
}

void TestQgs3dMaterial::cleanup()
{
}

void TestQgs3dMaterial::setColorProperty( const QgsProperty &property, QgsAbstractMaterialSettings::Property propertyType, QgsPropertyCollection &collection, QgsAbstractMaterialSettings &materialSettings )
{
  collection.setProperty( propertyType, property );
  materialSettings.setDataDefinedProperties( collection );
}


void TestQgs3dMaterial::colorDataDefinedPhong()
{
  const QgsExpressionContext expressionContext;
  QgsPhongMaterialSettings phongSettings;
  phongSettings.setDiffuse( Qt::black );
  phongSettings.setAmbient( Qt::black );
  phongSettings.setSpecular( Qt::black );
  QgsPropertyCollection propertyCollection;

  QByteArray colorByteArrayAllBlack;
  colorByteArrayAllBlack.fill( 0x00, 9 );

  QByteArray colorByteArray_1; // Diffuse : red ; Ambient : black ; Specular : black
  colorByteArray_1.fill( 0x00, 9 );
  colorByteArray_1[0] = 0xff;

  QByteArray colorByteArray_2; // Diffuse : red ; Ambient : blue ; Specular : black
  colorByteArray_2.fill( 0x00, 9 );
  colorByteArray_2[0] = 0xff;
  colorByteArray_2[5] = 0xff;


  QByteArray colorByteArray_3; // Diffuse : red ; Ambient : blue ; Specular : yellow
  colorByteArray_3.fill( 0x00, 9 );
  colorByteArray_3[0] = 0xff;
  colorByteArray_3[5] = 0xff;
  colorByteArray_3[6] = 0xff;
  colorByteArray_3[7] = 0xff;


  QgsProperty redProperty;
  redProperty.setExpressionString( QStringLiteral( "'red'" ) );
  redProperty.setActive( false );
  QgsProperty blueProperty;
  blueProperty.setExpressionString( QStringLiteral( "'blue'" ) );
  blueProperty.setActive( false );
  QgsProperty yellowProperty;
  yellowProperty.setExpressionString( QStringLiteral( "'yellow'" ) );
  yellowProperty.setActive( false );

  setColorProperty( redProperty, QgsAbstractMaterialSettings::Property::Diffuse, propertyCollection, phongSettings );
  QCOMPARE( phongSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArrayAllBlack );

  redProperty.setActive( true );
  setColorProperty( redProperty, QgsAbstractMaterialSettings::Property::Diffuse, propertyCollection, phongSettings );
  QCOMPARE( phongSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_1 );

  setColorProperty( blueProperty, QgsAbstractMaterialSettings::Property::Ambient, propertyCollection, phongSettings );
  QCOMPARE( phongSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_1 );

  blueProperty.setActive( true );
  setColorProperty( blueProperty, QgsAbstractMaterialSettings::Property::Ambient, propertyCollection, phongSettings );
  QCOMPARE( phongSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_2 );

  setColorProperty( yellowProperty, QgsAbstractMaterialSettings::Property::Specular, propertyCollection, phongSettings );
  QCOMPARE( phongSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_2 );

  yellowProperty.setActive( true );
  setColorProperty( yellowProperty, QgsAbstractMaterialSettings::Property::Specular, propertyCollection, phongSettings );
  QCOMPARE( phongSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_3 );
}

void TestQgs3dMaterial::colorDataDefinedGooch()
{
  const QgsExpressionContext expressionContext;

  QgsGoochMaterialSettings goochSettings;
  goochSettings.setDiffuse( Qt::black );
  goochSettings.setWarm( Qt::black );
  goochSettings.setCool( Qt::black );
  goochSettings.setSpecular( Qt::black );
  QgsPropertyCollection propertyCollection;

  QByteArray colorByteArrayAllBlack;
  colorByteArrayAllBlack.fill( 0x00, 12 );

  QByteArray colorByteArray_1; // Diffuse : red ; Warm : black ; Ambient : black ; Specular : black
  colorByteArray_1.fill( 0x00, 12 );
  colorByteArray_1[0] = 0xff;

  QByteArray colorByteArray_2; // Diffuse : red ; Warm : blue ; Ambient : black ; Specular : black
  colorByteArray_2.fill( 0x00, 12 );
  colorByteArray_2[0] = 0xff;
  colorByteArray_2[5] = 0xff;

  QByteArray colorByteArray_3; // Diffuse : red ; Warm : blue ; Ambient : yellow ; Specular : black
  colorByteArray_3.fill( 0x00, 12 );
  colorByteArray_3[0] = 0xff;
  colorByteArray_3[5] = 0xff;
  colorByteArray_3[6] = 0xff;
  colorByteArray_3[7] = 0xff;


  QByteArray colorByteArray_4; // Diffuse : red ; Warm : blue ; Ambient : yellow ; Specular : white
  colorByteArray_4.fill( 0x00, 12 );
  colorByteArray_4[0] = 0xff;
  colorByteArray_4[5] = 0xff;
  colorByteArray_4[6] = 0xff;
  colorByteArray_4[7] = 0xff;
  colorByteArray_4[9] = 0xff;
  colorByteArray_4[10] = 0xff;
  colorByteArray_4[11] = 0xff;

  QgsProperty redProperty;
  redProperty.setExpressionString( QStringLiteral( "'red'" ) );
  redProperty.setActive( false );
  QgsProperty blueProperty;
  blueProperty.setExpressionString( QStringLiteral( "'blue'" ) );
  blueProperty.setActive( false );
  QgsProperty yellowProperty;
  yellowProperty.setExpressionString( QStringLiteral( "'yellow'" ) );
  yellowProperty.setActive( false );
  QgsProperty whiteProperty;
  whiteProperty.setExpressionString( QStringLiteral( "'white'" ) );
  whiteProperty.setActive( false );

  setColorProperty( redProperty, QgsAbstractMaterialSettings::Property::Diffuse, propertyCollection, goochSettings );
  QCOMPARE( goochSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArrayAllBlack );

  redProperty.setActive( true );
  setColorProperty( redProperty, QgsAbstractMaterialSettings::Property::Diffuse, propertyCollection, goochSettings );
  QCOMPARE( goochSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_1 );

  setColorProperty( blueProperty, QgsAbstractMaterialSettings::Property::Warm, propertyCollection, goochSettings );
  QCOMPARE( goochSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_1 );

  blueProperty.setActive( true );
  setColorProperty( blueProperty, QgsAbstractMaterialSettings::Property::Warm, propertyCollection, goochSettings );
  QCOMPARE( goochSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_2 );

  setColorProperty( yellowProperty, QgsAbstractMaterialSettings::Property::Cool, propertyCollection, goochSettings );
  QCOMPARE( goochSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_2 );

  yellowProperty.setActive( true );
  setColorProperty( yellowProperty, QgsAbstractMaterialSettings::Property::Cool, propertyCollection, goochSettings );
  QCOMPARE( goochSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_3 );

  setColorProperty( whiteProperty, QgsAbstractMaterialSettings::Property::Specular, propertyCollection, goochSettings );
  QCOMPARE( goochSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_3 );

  whiteProperty.setActive( true );
  setColorProperty( whiteProperty, QgsAbstractMaterialSettings::Property::Specular, propertyCollection, goochSettings );
  QCOMPARE( goochSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_4 );
}

void TestQgs3dMaterial::clipping()
{
  const QString defineClippingStr = QStringLiteral( "#define %1" ).arg( QgsMaterial::CLIP_PLANE_DEFINE );
  const QList<QVector4D> clipPlanesEquations = QList<QVector4D>()
                                               << QVector4D( 0.866025, -0.5, 0, 150.0 )
                                               << QVector4D( -0.866025, 0.5, 0, 150.0 )
                                               << QVector4D( 0.5, 0.866025, 0, 305.0 )
                                               << QVector4D( -0.5, -0.866025, 0, 205.0 );

  auto findParameters = []( const Qt3DRender::QEffect *effect, bool &arrayFound, bool &maxFound ) -> void {
    arrayFound = false;
    maxFound = false;
    for ( Qt3DRender::QParameter *parameter : effect->parameters() )
    {
      const QString parameterName = parameter->name();
      if ( parameterName == QgsMaterial::CLIP_PLANE_ARRAY_PARAMETER_NAME )
      {
        arrayFound = true;
      }
      else if ( parameterName == QgsMaterial::CLIP_PLANE_MAX_PLANE_PARAMETER_NAME )
      {
        maxFound = true;
      }
    }
  };

  auto getShaderCode = []( const Qt3DRender::QEffect *effect ) -> QList<QByteArray> {
    Qt3DRender::QTechnique *technique = effect->techniques()[0];
    Qt3DRender::QRenderPass *renderPass = technique->renderPasses()[0];
    Qt3DRender::QShaderProgram *shaderProgram = renderPass->shaderProgram();
    QByteArray geomCode = shaderProgram->geometryShaderCode();
    QByteArray vertexCode = shaderProgram->vertexShaderCode();
    return QList<QByteArray> { geomCode, vertexCode };
  };

  // test phong material
  // It does not contain any geometry shader
  const QgsPhongMaterialSettings phongMaterialSettings;
  const QgsMaterialContext phongMaterialContext;
  QgsMaterial *phongMaterial = phongMaterialSettings.toMaterial( QgsMaterialSettingsRenderingTechnique::Triangles, phongMaterialContext );
  QVERIFY( phongMaterial );
  Qt3DRender::QEffect *phongMaterialEffect = phongMaterial->effect();
  QVERIFY( phongMaterialEffect );

  // by default clipping is disabled;
  // check that it does not contain any clipping parameter
  bool phongArrayParameterFound = true;
  bool phongMaxParameterFound = true;
  findParameters( phongMaterialEffect, phongArrayParameterFound, phongMaxParameterFound );
  QVERIFY( !phongArrayParameterFound );
  QVERIFY( !phongMaxParameterFound );

  QList<QByteArray> phongShaderCode = getShaderCode( phongMaterialEffect );
  QByteArray phongDefaultGeomCode = phongShaderCode[0];
  QByteArray phongDefaultVertexCode = phongShaderCode[1];
  QVERIFY( phongDefaultGeomCode.isEmpty() );
  QVERIFY( !phongDefaultVertexCode.isEmpty() );
  QVERIFY( !QString( phongDefaultGeomCode ).contains( defineClippingStr ) );
  QVERIFY( !QString( phongDefaultVertexCode ).contains( defineClippingStr ) );

  // Enable clipping
  phongMaterial->enableClipping( clipPlanesEquations );
  findParameters( phongMaterialEffect, phongArrayParameterFound, phongMaxParameterFound );
  QVERIFY( phongArrayParameterFound );
  QVERIFY( phongMaxParameterFound );

  phongShaderCode = getShaderCode( phongMaterialEffect );
  QVERIFY( phongShaderCode[0].isEmpty() );
  QVERIFY( phongShaderCode[1] != phongDefaultVertexCode );
  QVERIFY( !QString( phongShaderCode[0] ).contains( defineClippingStr ) );
  QVERIFY( QString( phongShaderCode[1] ).contains( defineClippingStr ) );

  // Disable clipping
  phongMaterial->disableClipping();
  findParameters( phongMaterialEffect, phongArrayParameterFound, phongMaxParameterFound );
  QVERIFY( !phongArrayParameterFound );
  QVERIFY( !phongMaxParameterFound );

  phongShaderCode = getShaderCode( phongMaterialEffect );
  QCOMPARE( phongShaderCode[0], phongDefaultGeomCode );
  QCOMPARE( phongShaderCode[1], phongDefaultVertexCode );
  QVERIFY( !QString( phongShaderCode[0] ).contains( defineClippingStr ) );
  QVERIFY( !QString( phongShaderCode[1] ).contains( defineClippingStr ) );


  // test line material
  // It contains a geometry shader
  const QgsSimpleLineMaterialSettings lineMaterialSettings;
  const QgsMaterialContext lineMaterialContext;
  QgsMaterial *lineMaterial = lineMaterialSettings.toMaterial( QgsMaterialSettingsRenderingTechnique::Lines, lineMaterialContext );
  QVERIFY( lineMaterial );
  Qt3DRender::QEffect *lineMaterialEffect = lineMaterial->effect();
  QVERIFY( lineMaterialEffect );

  // by default clipping is disabled;
  // check that it does not contain any clipping parameter
  bool lineArrayParameterFound = true;
  bool lineMaxParameterFound = true;
  findParameters( lineMaterialEffect, lineArrayParameterFound, lineMaxParameterFound );
  QVERIFY( !lineArrayParameterFound );
  QVERIFY( !lineMaxParameterFound );

  QList<QByteArray> lineShaderCode = getShaderCode( lineMaterialEffect );
  QByteArray lineDefaultGeomCode = lineShaderCode[0];
  QByteArray lineDefaultVertexCode = lineShaderCode[1];
  QVERIFY( !lineDefaultGeomCode.isEmpty() );
  QVERIFY( !lineDefaultVertexCode.isEmpty() );
  QVERIFY( !QString( lineDefaultGeomCode ).contains( defineClippingStr ) );
  QVERIFY( !QString( lineDefaultVertexCode ).contains( defineClippingStr ) );

  // Enable clipping
  lineMaterial->enableClipping( clipPlanesEquations );
  findParameters( lineMaterialEffect, lineArrayParameterFound, lineMaxParameterFound );
  QVERIFY( lineArrayParameterFound );
  QVERIFY( lineMaxParameterFound );

  lineShaderCode = getShaderCode( lineMaterialEffect );
  QVERIFY( lineShaderCode[0] != lineDefaultVertexCode );
  QVERIFY( lineShaderCode[1] != lineDefaultVertexCode );
  QVERIFY( QString( lineShaderCode[0] ).contains( defineClippingStr ) );
  QVERIFY( QString( lineShaderCode[1] ).contains( defineClippingStr ) );

  // Disable clipping
  lineMaterial->disableClipping();
  findParameters( lineMaterialEffect, lineArrayParameterFound, lineMaxParameterFound );
  QVERIFY( !lineArrayParameterFound );
  QVERIFY( !lineMaxParameterFound );

  lineShaderCode = getShaderCode( lineMaterialEffect );
  QCOMPARE( lineShaderCode[0], lineDefaultGeomCode );
  QCOMPARE( lineShaderCode[1], lineDefaultVertexCode );
  QVERIFY( !QString( lineShaderCode[0] ).contains( defineClippingStr ) );
  QVERIFY( !QString( lineShaderCode[1] ).contains( defineClippingStr ) );
}

QGSTEST_MAIN( TestQgs3dMaterial )
#include "testqgs3dmaterial.moc"
