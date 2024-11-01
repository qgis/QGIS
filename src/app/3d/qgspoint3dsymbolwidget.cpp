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
#include "moc_qgspoint3dsymbolwidget.cpp"
#include <QFileDialog>
#include <QMessageBox>
#include "qgslayoututils.h"

#include "qgspoint3dsymbol.h"
#include "qgssymbolbutton.h"
#include "qgsmarkersymbol.h"
#include "qgsabstractmaterialsettings.h"
#include "qgsvector3d.h"

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

  cboShape->addItem( tr( "Sphere" ), QVariant::fromValue( Qgis::Point3DShape::Sphere ) );
  cboShape->addItem( tr( "Cylinder" ), QVariant::fromValue( Qgis::Point3DShape::Cylinder ) );
  cboShape->addItem( tr( "Cube" ), QVariant::fromValue( Qgis::Point3DShape::Cube ) );
  cboShape->addItem( tr( "Cone" ), QVariant::fromValue( Qgis::Point3DShape::Cone ) );
  cboShape->addItem( tr( "Plane" ), QVariant::fromValue( Qgis::Point3DShape::Plane ) );
  cboShape->addItem( tr( "Torus" ), QVariant::fromValue( Qgis::Point3DShape::Torus ) );
  cboShape->addItem( tr( "3D Model" ), QVariant::fromValue( Qgis::Point3DShape::Model ) );
  cboShape->addItem( tr( "Billboard" ), QVariant::fromValue( Qgis::Point3DShape::Billboard ) );

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
  connect( btnChangeSymbol, static_cast<void ( QgsSymbolButton::* )()>( &QgsSymbolButton::changed ), this, &QgsPoint3DSymbolWidget::changed );

  // Sync between billboard height and TZ
  connect( spinBillboardHeight, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), spinTZ, &QDoubleSpinBox::setValue );
  connect( spinTZ, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), spinBillboardHeight, &QDoubleSpinBox::setValue );
}

Qgs3DSymbolWidget *QgsPoint3DSymbolWidget::create( QgsVectorLayer * )
{
  return new QgsPoint3DSymbolWidget();
}

void QgsPoint3DSymbolWidget::setSymbol( const QgsAbstract3DSymbol *symbol, QgsVectorLayer *layer )
{
  const QgsPoint3DSymbol *pointSymbol = dynamic_cast<const QgsPoint3DSymbol *>( symbol );
  if ( !pointSymbol )
    return;

  cboAltClamping->setCurrentIndex( static_cast<int>( pointSymbol->altitudeClamping() ) );

  cboShape->setCurrentIndex( cboShape->findData( QVariant::fromValue( pointSymbol->shape() ) ) );
  QgsMaterialSettingsRenderingTechnique technique = QgsMaterialSettingsRenderingTechnique::InstancedPoints;
  bool forceNullMaterial = false;
  switch ( pointSymbol->shape() )
  {
    case Qgis::Point3DShape::Sphere:
      spinRadius->setValue( pointSymbol->shapeProperty( QStringLiteral( "radius" ) ).toDouble() );
      break;
    case Qgis::Point3DShape::Cylinder:
      spinRadius->setValue( pointSymbol->shapeProperty( QStringLiteral( "radius" ) ).toDouble() );
      spinLength->setValue( pointSymbol->shapeProperty( QStringLiteral( "length" ) ).toDouble() );
      break;
    case Qgis::Point3DShape::Cube:
      spinSize->setValue( pointSymbol->shapeProperty( QStringLiteral( "size" ) ).toDouble() );
      break;
    case Qgis::Point3DShape::Cone:
      spinTopRadius->setValue( pointSymbol->shapeProperty( QStringLiteral( "topRadius" ) ).toDouble() );
      spinBottomRadius->setValue( pointSymbol->shapeProperty( QStringLiteral( "bottomRadius" ) ).toDouble() );
      spinLength->setValue( pointSymbol->shapeProperty( QStringLiteral( "length" ) ).toDouble() );
      break;
    case Qgis::Point3DShape::Plane:
      spinSize->setValue( pointSymbol->shapeProperty( QStringLiteral( "size" ) ).toDouble() );
      break;
    case Qgis::Point3DShape::Torus:
      spinRadius->setValue( pointSymbol->shapeProperty( QStringLiteral( "radius" ) ).toDouble() );
      spinMinorRadius->setValue( pointSymbol->shapeProperty( QStringLiteral( "minorRadius" ) ).toDouble() );
      break;
    case Qgis::Point3DShape::Model:
    {
      lineEditModel->setSource( pointSymbol->shapeProperty( QStringLiteral( "model" ) ).toString() );
      // "overwriteMaterial" is a legacy setting indicating that non-null material should be used
      forceNullMaterial = ( pointSymbol->shapeProperties().contains( QStringLiteral( "overwriteMaterial" ) ) && !pointSymbol->shapeProperties().value( QStringLiteral( "overwriteMaterial" ) ).toBool() )
                          || !pointSymbol->materialSettings()
                          || pointSymbol->materialSettings()->type() == QLatin1String( "null" );
      technique = QgsMaterialSettingsRenderingTechnique::TrianglesFromModel;
      break;
    }
    case Qgis::Point3DShape::Billboard:
      if ( pointSymbol->billboardSymbol() )
      {
        btnChangeSymbol->setSymbol( pointSymbol->billboardSymbol()->clone() );
      }
      technique = QgsMaterialSettingsRenderingTechnique::Points;
      break;
    case Qgis::Point3DShape::ExtrudedText:
      break;
  }

  widgetMaterial->setSettings( pointSymbol->materialSettings(), layer );
  widgetMaterial->setTechnique( technique );

  if ( forceNullMaterial )
  {
    widgetMaterial->setType( QStringLiteral( "null" ) );
  }

  // decompose the transform matrix
  // assuming the last row has values [0 0 0 1]
  // see https://math.stackexchange.com/questions/237369/given-this-transformation-matrix-how-do-i-decompose-it-into-translation-rotati
  // A point on the 2D plane (x', y') is transformed to (x, -z) in the 3D world.
  // The formula from stackexchange need to be changed to take into account the 3D representation.
  QMatrix4x4 m = pointSymbol->transform();
  float *md = m.data(); // returns data in column-major order
  const float sx = QVector3D( md[0], md[1], md[2] ).length();
  const float sz = QVector3D( md[4], md[5], md[6] ).length();
  const float sy = QVector3D( md[8], md[9], md[10] ).length();
  float rd[9] = {
    md[0] / sx,
    md[4] / sy,
    md[8] / sz,
    md[1] / sx,
    md[5] / sy,
    md[9] / sz,
    md[2] / sx,
    md[6] / sy,
    md[10] / sz,
  };
  const QMatrix3x3 rot3x3( rd ); // takes data in row-major order
  const QVector3D rot = QQuaternion::fromRotationMatrix( rot3x3 ).toEulerAngles();
  const QgsVector3D translationMapCoords( md[12], -md[14], md[13] );

  spinBillboardHeight->setValue( md[13] );
  spinTX->setValue( translationMapCoords.x() );
  spinTY->setValue( translationMapCoords.y() );
  spinTZ->setValue( translationMapCoords.z() );
  spinSX->setValue( sx );
  spinSY->setValue( sy );
  spinSZ->setValue( sz );
  spinRX->setValue( QgsLayoutUtils::normalizedAngle( rot.x() ) );
  spinRY->setValue( QgsLayoutUtils::normalizedAngle( 360.0 - rot.z() ) );
  spinRZ->setValue( QgsLayoutUtils::normalizedAngle( rot.y() ) );
}

QgsAbstract3DSymbol *QgsPoint3DSymbolWidget::symbol()
{
  QVariantMap vm;
  std::unique_ptr<QgsPoint3DSymbol> sym = std::make_unique<QgsPoint3DSymbol>();
  sym->setBillboardSymbol( static_cast<QgsMarkerSymbol *>( QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ) ) );
  switch ( cboShape->currentData().value<Qgis::Point3DShape>() )
  {
    case Qgis::Point3DShape::Sphere:
      vm[QStringLiteral( "radius" )] = spinRadius->value();
      break;
    case Qgis::Point3DShape::Cylinder:
      vm[QStringLiteral( "radius" )] = spinRadius->value();
      vm[QStringLiteral( "length" )] = spinLength->value();
      break;
    case Qgis::Point3DShape::Cube:
      vm[QStringLiteral( "size" )] = spinSize->value();
      break;
    case Qgis::Point3DShape::Cone:
      vm[QStringLiteral( "topRadius" )] = spinTopRadius->value();
      vm[QStringLiteral( "bottomRadius" )] = spinBottomRadius->value();
      vm[QStringLiteral( "length" )] = spinLength->value();
      break;
    case Qgis::Point3DShape::Plane:
      vm[QStringLiteral( "size" )] = spinSize->value();
      break;
    case Qgis::Point3DShape::Torus:
      vm[QStringLiteral( "radius" )] = spinRadius->value();
      vm[QStringLiteral( "minorRadius" )] = spinMinorRadius->value();
      break;
    case Qgis::Point3DShape::Model:
      vm[QStringLiteral( "model" )] = lineEditModel->source();
      break;
    case Qgis::Point3DShape::Billboard:
      sym->setBillboardSymbol( btnChangeSymbol->clonedSymbol<QgsMarkerSymbol>() );
      break;
    case Qgis::Point3DShape::ExtrudedText:
      break;
  }

  // A point on the 2D plane (x', y') is transformed to (x, -z) in the 3D world.
  // The rotation, scale and translation values need to be converted in the 3D world.
  const QgsVector3D translationWorldCoords( spinTX->value(), spinTZ->value(), -spinTY->value() );
  const QQuaternion rot( QQuaternion::fromEulerAngles( static_cast<float>( spinRX->value() ), static_cast<float>( spinRZ->value() ), static_cast<float>( 360.0 - spinRY->value() ) ) );
  const QVector3D sca( static_cast<float>( spinSX->value() ), static_cast<float>( spinSZ->value() ), static_cast<float>( spinSY->value() ) );
  const QVector3D tra( static_cast<float>( translationWorldCoords.x() ), static_cast<float>( translationWorldCoords.y() ), static_cast<float>( translationWorldCoords.z() ) );

  QMatrix4x4 tr;
  tr.translate( tra );
  tr.scale( sca );
  tr.rotate( rot );

  sym->setAltitudeClamping( static_cast<Qgis::AltitudeClamping>( cboAltClamping->currentIndex() ) );
  sym->setShape( cboShape->itemData( cboShape->currentIndex() ).value<Qgis::Point3DShape>() );
  sym->setShapeProperties( vm );
  sym->setMaterialSettings( widgetMaterial->settings() );
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
  switch ( cboShape->currentData().value<Qgis::Point3DShape>() )
  {
    case Qgis::Point3DShape::Sphere:
      activeWidgets << labelRadius << spinRadius;
      break;
    case Qgis::Point3DShape::Cylinder:
      activeWidgets << labelRadius << spinRadius << labelLength << spinLength;
      break;
    case Qgis::Point3DShape::Cube:
      activeWidgets << labelSize << spinSize;
      break;
    case Qgis::Point3DShape::Cone:
      activeWidgets << labelTopRadius << spinTopRadius << labelBottomRadius << spinBottomRadius << labelLength << spinLength;
      break;
    case Qgis::Point3DShape::Plane:
      activeWidgets << labelSize << spinSize;
      break;
    case Qgis::Point3DShape::Torus:
      activeWidgets << labelRadius << spinRadius << labelMinorRadius << spinMinorRadius;
      break;
    case Qgis::Point3DShape::Model:
      activeWidgets << labelModel << lineEditModel;
      technique = QgsMaterialSettingsRenderingTechnique::TrianglesFromModel;
      break;
    case Qgis::Point3DShape::Billboard:
      activeWidgets << labelBillboardHeight << spinBillboardHeight << labelBillboardSymbol << btnChangeSymbol;
      // Always hide material and transformationwidget for billboard
      materialsGroupBox->hide();
      transformationWidget->hide();
      technique = QgsMaterialSettingsRenderingTechnique::Points;
      break;
    case Qgis::Point3DShape::ExtrudedText:
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
