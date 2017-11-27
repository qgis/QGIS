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
#include "qgssettings.h"

#include "qgspoint3dsymbol.h"

QgsPoint3DSymbolWidget::QgsPoint3DSymbolWidget( QWidget *parent )
  : QWidget( parent )
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

  cboShape->addItem( tr( "Sphere" ), QgsPoint3DSymbol::Sphere );
  cboShape->addItem( tr( "Cylinder" ), QgsPoint3DSymbol::Cylinder );
  cboShape->addItem( tr( "Cube" ), QgsPoint3DSymbol::Cube );
  cboShape->addItem( tr( "Cone" ), QgsPoint3DSymbol::Cone );
  cboShape->addItem( tr( "Plane" ), QgsPoint3DSymbol::Plane );
  cboShape->addItem( tr( "Torus" ), QgsPoint3DSymbol::Torus );
  cboShape->addItem( tr( "3D Model" ), QgsPoint3DSymbol::Model );

  setSymbol( QgsPoint3DSymbol() );
  onShapeChanged();

  connect( cboAltClamping, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPoint3DSymbolWidget::changed );
  connect( cboShape, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPoint3DSymbolWidget::onShapeChanged );
  QList<QDoubleSpinBox *> spinWidgets;
  spinWidgets << spinRadius << spinTopRadius << spinBottomRadius << spinMinorRadius << spinSize << spinLength;
  spinWidgets << spinTX << spinTY << spinTZ << spinSX << spinSY << spinSZ << spinRX << spinRY << spinRZ;
  Q_FOREACH ( QDoubleSpinBox *spinBox, spinWidgets )
    connect( spinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPoint3DSymbolWidget::changed );
  connect( lineEditModel, static_cast<void ( QLineEdit::* )( const QString & )>( &QLineEdit::textChanged ), this, &QgsPoint3DSymbolWidget::changed );
  connect( btnModel, static_cast<void ( QToolButton::* )( bool )>( &QToolButton::clicked ), this, &QgsPoint3DSymbolWidget::onChooseModelClicked );
  connect( cbOverwriteMaterial, static_cast<void ( QCheckBox::* )( int )>( &QCheckBox::stateChanged ), this, &QgsPoint3DSymbolWidget::onOverwriteMaterialChecked );
  connect( widgetMaterial, &QgsPhongMaterialWidget::changed, this, &QgsPoint3DSymbolWidget::changed );
}

void QgsPoint3DSymbolWidget::onChooseModelClicked( bool )
{
  QgsSettings s;
  QString lastDir = s.value( QStringLiteral( "/UI/lastModel3dDir" ), QDir::homePath() ).toString();

  QString filePath = QFileDialog::getOpenFileName( this, tr( "Open 3d Model File" ), lastDir, QStringLiteral( "3D models (*.*)" ) );
  if ( filePath.isEmpty() )
  {
    return;
  }

  //check if file exists
  QFileInfo fileInfo( filePath );
  if ( !fileInfo.exists() || !fileInfo.isReadable() )
  {
    QMessageBox::critical( nullptr, tr( "Invalid file" ), tr( "Error, file does not exist or is not readable" ) );
    return;
  }

  s.setValue( QStringLiteral( "/UI/lastModel3dDir" ), fileInfo.absolutePath() );
  lineEditModel->setText( filePath );
}

void QgsPoint3DSymbolWidget::onOverwriteMaterialChecked( int state )
{
  if ( state == Qt::Checked )
  {
    widgetMaterial->setEnabled( true );
  }
  else
  {
    widgetMaterial->setEnabled( false );
  }
  emit changed();
}

void QgsPoint3DSymbolWidget::setSymbol( const QgsPoint3DSymbol &symbol )
{
  cboAltClamping->setCurrentIndex( ( int ) symbol.altitudeClamping() );

  QVariantMap vm = symbol.shapeProperties();
  int index = cboShape->findData( symbol.shape() );
  cboShape->setCurrentIndex( index != -1 ? index : 1 );  // use cylinder by default if shape is not set

  widgetMaterial->setEnabled( true );
  switch ( cboShape->currentIndex() )
  {
    case 0:  // sphere
      spinRadius->setValue( vm["radius"].toDouble() );
      break;
    case 1:  // cylinder
      spinRadius->setValue( vm["radius"].toDouble() );
      spinLength->setValue( vm["length"].toDouble() );
      break;
    case 2:  // cube
      spinSize->setValue( vm["size"].toDouble() );
      break;
    case 3:  // cone
      spinTopRadius->setValue( vm["topRadius"].toDouble() );
      spinBottomRadius->setValue( vm["bottomRadius"].toDouble() );
      spinLength->setValue( vm["length"].toDouble() );
      break;
    case 4:  // plane
      spinSize->setValue( vm["size"].toDouble() );
      break;
    case 5:  // torus
      spinRadius->setValue( vm["radius"].toDouble() );
      spinMinorRadius->setValue( vm["minorRadius"].toDouble() );
      break;
    case 6:  // 3d model
      lineEditModel->setText( vm["model"].toString() );
      bool overwriteMaterial = vm["overwriteMaterial"].toBool();
      widgetMaterial->setEnabled( overwriteMaterial );
      cbOverwriteMaterial->setChecked( overwriteMaterial );
      break;
  }

  widgetMaterial->setMaterial( symbol.material() );

  // decompose the transform matrix
  // assuming the last row has values [0 0 0 1]
  // see https://math.stackexchange.com/questions/237369/given-this-transformation-matrix-how-do-i-decompose-it-into-translation-rotati
  QMatrix4x4 m = symbol.transform();
  float *md = m.data();  // returns data in column-major order
  float sx = QVector3D( md[0], md[1], md[2] ).length();
  float sy = QVector3D( md[4], md[5], md[6] ).length();
  float sz = QVector3D( md[8], md[9], md[10] ).length();
  float rd[9] =
  {
    md[0] / sx, md[4] / sy, md[8] / sz,
    md[1] / sx, md[5] / sy, md[9] / sz,
    md[2] / sx, md[6] / sy, md[10] / sz,
  };
  QMatrix3x3 rot3x3( rd ); // takes data in row-major order
  QVector3D rot = QQuaternion::fromRotationMatrix( rot3x3 ).toEulerAngles();

  spinTX->setValue( md[12] );
  spinTY->setValue( md[13] );
  spinTZ->setValue( md[14] );
  spinSX->setValue( sx );
  spinSY->setValue( sy );
  spinSZ->setValue( sz );
  spinRX->setValue( rot.x() );
  spinRY->setValue( rot.y() );
  spinRZ->setValue( rot.z() );
}

QgsPoint3DSymbol QgsPoint3DSymbolWidget::symbol() const
{
  QVariantMap vm;
  switch ( cboShape->currentIndex() )
  {
    case 0:  // sphere
      vm["radius"] = spinRadius->value();
      break;
    case 1:  // cylinder
      vm["radius"] = spinRadius->value();
      vm["length"] = spinLength->value();
      break;
    case 2:  // cube
      vm["size"] = spinSize->value();
      break;
    case 3:  // cone
      vm["topRadius"] = spinTopRadius->value();
      vm["bottomRadius"] = spinBottomRadius->value();
      vm["length"] = spinLength->value();
      break;
    case 4:  // plane
      vm["size"] = spinSize->value();
      break;
    case 5:  // torus
      vm["radius"] = spinRadius->value();
      vm["minorRadius"] = spinMinorRadius->value();
      break;
    case 6:  // model
      vm["model"] = lineEditModel->text();
      vm["overwriteMaterial"] = cbOverwriteMaterial->isChecked();
      break;
  }

  QQuaternion rot( QQuaternion::fromEulerAngles( spinRX->value(), spinRY->value(), spinRZ->value() ) );
  QVector3D sca( spinSX->value(), spinSY->value(), spinSZ->value() );
  QVector3D tra( spinTX->value(), spinTY->value(), spinTZ->value() );

  QMatrix4x4 tr;
  tr.translate( tra );
  tr.scale( sca );
  tr.rotate( rot );

  QgsPoint3DSymbol sym;
  sym.setAltitudeClamping( ( AltitudeClamping ) cboAltClamping->currentIndex() );
  sym.setShape( ( QgsPoint3DSymbol::Shape ) cboShape->itemData( cboShape->currentIndex() ).toInt() );
  sym.setShapeProperties( vm );
  sym.setMaterial( widgetMaterial->material() );
  sym.setTransform( tr );
  return sym;
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
             << labelModel << lineEditModel << btnModel << cbOverwriteMaterial;

  QList<QWidget *> activeWidgets;
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
      activeWidgets << labelModel << lineEditModel << btnModel << cbOverwriteMaterial;
      break;
  }

  Q_FOREACH ( QWidget *w, allWidgets )
  {
    w->setVisible( activeWidgets.contains( w ) );
  }

  emit changed();
}
