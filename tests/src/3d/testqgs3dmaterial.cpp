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

#include "qgsmaterialregistry.h"
#include "qgsphongmaterialsettings.h"
#include "qgsgoochmaterialsettings.h"

#include <QObject>
#include "qgstest.h"
#include "qgs3d.h"


class TestQgs3dMaterial : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void colorDataDefinedPhong();
    void colorDataDefinedGooch();

  private:
    void setColorProperty( const QgsProperty &property,
                           QgsAbstractMaterialSettings::Property propertyType,
                           QgsPropertyCollection &collection,
                           QgsAbstractMaterialSettings &materialSettings );

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

  setColorProperty( redProperty, QgsAbstractMaterialSettings::Diffuse, propertyCollection, phongSettings );
  QCOMPARE( phongSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArrayAllBlack );

  redProperty.setActive( true );
  setColorProperty( redProperty, QgsAbstractMaterialSettings::Diffuse, propertyCollection, phongSettings );
  QCOMPARE( phongSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_1 );

  setColorProperty( blueProperty, QgsAbstractMaterialSettings::Ambient, propertyCollection, phongSettings );
  QCOMPARE( phongSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_1 );

  blueProperty.setActive( true );
  setColorProperty( blueProperty, QgsAbstractMaterialSettings::Ambient, propertyCollection, phongSettings );
  QCOMPARE( phongSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_2 );

  setColorProperty( yellowProperty, QgsAbstractMaterialSettings::Specular, propertyCollection, phongSettings );
  QCOMPARE( phongSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_2 );

  yellowProperty.setActive( true );
  setColorProperty( yellowProperty, QgsAbstractMaterialSettings::Specular, propertyCollection, phongSettings );
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

  setColorProperty( redProperty, QgsAbstractMaterialSettings::Diffuse, propertyCollection, goochSettings );
  QCOMPARE( goochSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArrayAllBlack );

  redProperty.setActive( true );
  setColorProperty( redProperty, QgsAbstractMaterialSettings::Diffuse, propertyCollection, goochSettings );
  QCOMPARE( goochSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_1 );

  setColorProperty( blueProperty, QgsAbstractMaterialSettings::Warm, propertyCollection, goochSettings );
  QCOMPARE( goochSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_1 );

  blueProperty.setActive( true );
  setColorProperty( blueProperty, QgsAbstractMaterialSettings::Warm, propertyCollection, goochSettings );
  QCOMPARE( goochSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_2 );

  setColorProperty( yellowProperty, QgsAbstractMaterialSettings::Cool, propertyCollection, goochSettings );
  QCOMPARE( goochSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_2 );

  yellowProperty.setActive( true );
  setColorProperty( yellowProperty, QgsAbstractMaterialSettings::Cool, propertyCollection, goochSettings );
  QCOMPARE( goochSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_3 );

  setColorProperty( whiteProperty, QgsAbstractMaterialSettings::Specular, propertyCollection, goochSettings );
  QCOMPARE( goochSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_3 );

  whiteProperty.setActive( true );
  setColorProperty( whiteProperty, QgsAbstractMaterialSettings::Specular, propertyCollection, goochSettings );
  QCOMPARE( goochSettings.dataDefinedVertexColorsAsByte( expressionContext ), colorByteArray_4 );
}

QGSTEST_MAIN( TestQgs3dMaterial )
#include "testqgs3dmaterial.moc"
