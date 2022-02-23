/***************************************************************************
  qgspoint3dsymbolwidget.cpp
  --------------------------------------
  Date                 : July 2017
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

#include "qgspoint3dsymbolwidget.h"
#include <QFileDialog>
#include <QMessageBox>
#include "qgslayoututils.h"
#include "qgsreadwritecontext.h"
#include "qgssettings.h"

#include "qgspoint3dsymbol.h"
#include "qgssymbolbutton.h"
#include "qgssymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgsphongmaterialsettings.h"
#include "qgsmarkersymbol.h"

QgsPoint3DSymbolWidget::QgsPoint3DSymbolWidget( QWidget *parent )
  : Qgs3DSymbolWidget( parent )
{
  setupUi( this );

  spinTX->setClearValue( 0.0 );
  spinTY->setClearValue( 0.0 );
  spinTZ->setClearValue( 0.0 );
  spinSX->setClearValue( 1.0 );
  spinSY->setClearValue( 1.0 );
  spinSZ->setClearValue( 1.0 );
  spinRX->setClearValue( 0.0 );
  spinRY->setClearValue( 0.0 );
  spinRZ->setClearValue( 0.0 );
  spinRadius->setClearValue( 10.0 );
  spinMinorRadius->setClearValue( 5.0 );
  spinTopRadius->setClearValue( 0.0 );
  spinBottomRadius->setClearValue( 10.0 );
  spinSize->setClearValue( 10.0 );
  spinLength->setClearValue( 10.0 );
  spinTopRadius->setClearValue( 0.0 );
  spinBillboardHeight->setClearValue( 0.0 );

  cboShape->addItem( tr( "Sphere" ), QgsPoint3DSymbol::Sphere );
  cboShape->addItem( tr( "Cylinder" ), QgsPoint3DSymbol::Cylinder );
  cboShape->addItem( tr( "Cube" ), QgsPoint3DSymbol::Cube );
  cboShape->addItem( tr( "Cone" ), QgsPoint3DSymbol::Cone );
  cboShape->addItem( tr( "Plane" ), QgsPoint3DSymbol::Plane );
  cboShape->addItem( tr( "Torus" ), QgsPoint3DSymbol::Torus );
  cboShape->addItem( tr( "3D Model" ), QgsPoint3DSymbol::Model );
  cboShape->addItem( tr( "Billboard" ), QgsPoint3DSymbol::Billboard );

  btnChangeSymbol->setSymbolType( Qgis::SymbolType::Marker );
  btnChangeSymbol->setDialogTitle( tr( "Billboard symbol" ) );

  QgsPoint3DSymbol defaultSymbol;
  setSymbol( &defaultSymbol, nullptr );
  onShapeChanged();

  connect( cboAltClamping, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPoint3DSymbolWidget::changed );
  connect( cboShape, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPoint3DSymbolWidget::onShapeChanged );
  QList<QDoubleSpinBox *> spinWidgets;
  spinWidgets << spinRadius << spinTopRadius << spinBottomRadius << spinMinorRadius << spinSize << spinLength << spinBillboardHeight;
  spinWidgets << spinTX << spinTY << spinTZ << spinSX << spinSY << spinSZ << spinRX << spinRY << spinRZ;
  const auto constSpinWidgets = spinWidgets;
  for ( QDoubleSpinBox *spinBox : constSpinWidgets )
    connect( spinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPoint3DSymbolWidget::changed );
  connect( lineEditModel, &QgsAbstractFileContentSourceLineEdit::sourceChanged, this, &QgsPoint3DSymbolWidget::changed );
  connect( widgetMaterial, &QgsMaterialWidget::changed, this, &QgsPoint3DSymbolWidget::changed );
  connect( btnChangeSymbol, static_cast<void ( QgsSymbolButton::* )( )>( &QgsSymbolButton::changed ), this, &QgsPoint3DSymbolWidget::changed );

  // Sync between billboard height and TY
  connect( spinBillboardHeight, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), spinTY,  &QDoubleSpinBox::setValue );
  connect( spinTY, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), spinBillboardHeight,  &QDoubleSpinBox::setValue );
}

Qgs3DSymbolWidget *QgsPoint3DSymbolWidget::create( QgsVectorLayer * )
{
  return new QgsPoint3DSymbolWidget();
}

void QgsPoint3DSymbolWidget::setSymbol( const QgsAbstract3DSymbol *symbol, QgsVectorLayer *layer )
{
  const QgsPoint3DSymbol *pointSymbol = dynamic_cast< const QgsPoint3DSymbol *>( symbol );
  if ( !pointSymbol )
    return;

  cboAltClamping->setCurrentIndex( static_cast<int>( pointSymbol->altitudeClamping() ) );

  QVariantMap vm = pointSymbol->shapeProperties();
  const int index = cboShape->findData( pointSymbol->shape() );
  cboShape->setCurrentIndex( index != -1 ? index : 1 );  // use cylinder by default if shape is not set
  QgsMaterialSettingsRenderingTechnique technique = QgsMaterialSettingsRenderingTechnique::InstancedPoints;
  bool forceNullMaterial = false;
  switch ( cboShape->currentIndex() )
  {
    case 0:  // sphere
      spinRadius->setValue( vm[QStringLiteral( "radius" )].toDouble() );
      break;
    case 1:  // cylinder
      spinRadius->setValue( vm[QStringLiteral( "radius" )].toDouble() );
      spinLength->setValue( vm[QStringLiteral( "length" )].toDouble() );
      break;
    case 2:  // cube
      spinSize->setValue( vm[QStringLiteral( "size" )].toDouble() );
      break;
    case 3:  // cone
      spinTopRadius->setValue( vm[QStringLiteral( "topRadius" )].toDouble() );
      spinBottomRadius->setValue( vm[QStringLiteral( "bottomRadius" )].toDouble() );
      spinLength->setValue( vm[QStringLiteral( "length" )].toDouble() );
      break;
    case 4:  // plane
      spinSize->setValue( vm[QStringLiteral( "size" )].toDouble() );
      break;
    case 5:  // torus
      spinRadius->setValue( vm[QStringLiteral( "radius" )].toDouble() );
      spinMinorRadius->setValue( vm[QStringLiteral( "minorRadius" )].toDouble() );
      break;
    case 6:  // 3d model
    {
      lineEditModel->setSource( vm[QStringLiteral( "model" )].toString() );
      // "overwriteMaterial" is a legacy setting indicating that non-null material should be used
      forceNullMaterial = ( vm.contains( QStringLiteral( "overwriteMaterial" ) ) && !vm[QStringLiteral( "overwriteMaterial" )].toBool() )
                          || !pointSymbol->material()
                          || pointSymbol->material()->type() == QLatin1String( "null" );
      technique = QgsMaterialSettingsRenderingTechnique::TrianglesFromModel;
      break;
    }
    case 7:  // billboard
      if ( pointSymbol->billboardSymbol() )
      {
        btnChangeSymbol->setSymbol( pointSymbol->billboardSymbol()->clone() );
      }
      technique = QgsMaterialSettingsRenderingTechnique::Points;
      break;
  }

  widgetMaterial->setSettings( pointSymbol->material(), layer );
  widgetMaterial->setTechnique( technique );

  if ( forceNullMaterial )
  {
    widgetMaterial->setType( QStringLiteral( "null" ) );
  }

  // decompose the transform matrix
  // assuming the last row has values [0 0 0 1]
  // see https://math.stackexchange.com/questions/237369/given-this-transformation-matrix-how-do-i-decompose-it-into-translation-rotati
  QMatrix4x4 m = pointSymbol->transform();
  float *md = m.data();  // returns data in column-major order
  const float sx = QVector3D( md[0], md[1], md[2] ).length();
  const float sy = QVector3D( md[4], md[5], md[6] ).length();
  const float sz = QVector3D( md[8], md[9], md[10] ).length();
  float rd[9] =
  {
    md[0] / sx, md[4] / sy, md[8] / sz,
    md[1] / sx, md[5] / sy, md[9] / sz,
    md[2] / sx, md[6] / sy, md[10] / sz,
  };
  const QMatrix3x3 rot3x3( rd ); // takes data in row-major order
  const QVector3D rot = QQuaternion::fromRotationMatrix( rot3x3 ).toEulerAngles();

  spinBillboardHeight->setValue( md[13] );
  spinTX->setValue( md[12] );
  spinTY->setValue( md[13] );
  spinTZ->setValue( md[14] );
  spinSX->setValue( sx );
  spinSY->setValue( sy );
  spinSZ->setValue( sz );
  spinRX->setValue( QgsLayoutUtils::normalizedAngle( rot.x() ) );
  spinRY->setValue( QgsLayoutUtils::normalizedAngle( rot.y() ) );
  spinRZ->setValue( QgsLayoutUtils::normalizedAngle( rot.z() ) );
}

QgsAbstract3DSymbol *QgsPoint3DSymbolWidget::symbol()
{
  QVariantMap vm;
  std::unique_ptr< QgsPoint3DSymbol > sym = std::make_unique< QgsPoint3DSymbol >();
  sym->setBillboardSymbol( static_cast<QgsMarkerSymbol *>( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) ) );
  switch ( cboShape->currentIndex() )
  {
    case 0:  // sphere
      vm[QStringLiteral( "radius" )] = spinRadius->value();
      break;
    case 1:  // cylinder
      vm[QStringLiteral( "radius" )] = spinRadius->value();
      vm[QStringLiteral( "length" )] = spinLength->value();
      break;
    case 2:  // cube
      vm[QStringLiteral( "size" )] = spinSize->value();
      break;
    case 3:  // cone
      vm[QStringLiteral( "topRadius" )] = spinTopRadius->value();
      vm[QStringLiteral( "bottomRadius" )] = spinBottomRadius->value();
      vm[QStringLiteral( "length" )] = spinLength->value();
      break;
    case 4:  // plane
      vm[QStringLiteral( "size" )] = spinSize->value();
      break;
    case 5:  // torus
      vm[QStringLiteral( "radius" )] = spinRadius->value();
      vm[QStringLiteral( "minorRadius" )] = spinMinorRadius->value();
      break;
    case 6:  // 3d model
      vm[QStringLiteral( "model" )] = lineEditModel->source();
      break;
    case 7:  // billboard
      sym->setBillboardSymbol( btnChangeSymbol->clonedSymbol<QgsMarkerSymbol>() );
      break;
  }

  const QQuaternion rot( QQuaternion::fromEulerAngles( spinRX->value(), spinRY->value(), spinRZ->value() ) );
  const QVector3D sca( spinSX->value(), spinSY->value(), spinSZ->value() );
  const QVector3D tra( spinTX->value(), spinTY->value(), spinTZ->value() );

  QMatrix4x4 tr;
  tr.translate( tra );
  tr.scale( sca );
  tr.rotate( rot );

  sym->setAltitudeClamping( static_cast<Qgis::AltitudeClamping>( cboAltClamping->currentIndex() ) );
  sym->setShape( static_cast<QgsPoint3DSymbol::Shape>( cboShape->itemData( cboShape->currentIndex() ).toInt() ) );
  sym->setShapeProperties( vm );
  sym->setMaterial( widgetMaterial->settings() );
  sym->setTransform( tr );
  return sym.release();
}

QString QgsPoint3DSymbolWidget::symbolType() const
{
  return QStringLiteral( "point" );
}

void QgsPoint3DSymbolWidget::onShapeChanged()
{
  QList<QWidget *> allWidgets;
  allWidgets << labelSize << spinSize
             << labelRadius << spinRadius
             << labelMinorRadius << spinMinorRadius
             << labelTopRadius << spinTopRadius
             << labelBottomRadius << spinBottomRadius
             << labelLength << spinLength
             << labelModel << lineEditModel
             << labelBillboardHeight << spinBillboardHeight << labelBillboardSymbol << btnChangeSymbol;

  materialsGroupBox->show();
  transformationWidget->show();
  QList<QWidget *> activeWidgets;
  QgsMaterialSettingsRenderingTechnique technique = QgsMaterialSettingsRenderingTechnique::InstancedPoints;
  switch ( cboShape->currentIndex() )
  {
    case 0:  // sphere
      activeWidgets << labelRadius << spinRadius;
      break;
    case 1:  // cylinder
      activeWidgets << labelRadius << spinRadius << labelLength << spinLength;
      break;
    case 2:  // cube
      activeWidgets << labelSize << spinSize;
      break;
    case 3:  // cone
      activeWidgets << labelTopRadius << spinTopRadius << labelBottomRadius << spinBottomRadius << labelLength << spinLength;
      break;
    case 4:  // plane
      activeWidgets << labelSize << spinSize;
      break;
    case 5:  // torus
      activeWidgets << labelRadius << spinRadius << labelMinorRadius << spinMinorRadius;
      break;
    case 6:  // 3d model
      activeWidgets << labelModel << lineEditModel;
      technique = QgsMaterialSettingsRenderingTechnique::TrianglesFromModel;
      break;
    case 7:  // billboard
      activeWidgets << labelBillboardHeight << spinBillboardHeight << labelBillboardSymbol << btnChangeSymbol;
      // Always hide material and transformationwidget for billboard
      materialsGroupBox->hide();
      transformationWidget->hide();
      technique = QgsMaterialSettingsRenderingTechnique::Points;
      break;
  }

  widgetMaterial->setTechnique( technique );

  if ( cboShape->currentIndex() == 6 )
  {
    // going from different shape -> model resets the material to the null type
    widgetMaterial->setType( QStringLiteral( "null" ) );
  }

  const auto constAllWidgets = allWidgets;
  for ( QWidget *w : constAllWidgets )
  {
    w->setVisible( activeWidgets.contains( w ) );
  }

  emit changed();
}
