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

#include "qgs3dutils.h"
#include "qgsabstractmaterialsettings.h"
#include "qgslayoututils.h"
#include "qgsmarkersymbol.h"
#include "qgspoint3dsymbol.h"
#include "qgssymbolbutton.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QString>

#include "moc_qgspoint3dsymbolwidget.cpp"

using namespace Qt::StringLiterals;

QString resolveAxisConflict( const QString &axisWithPossibleConflict, const QString &fixedAxis, bool isUpFixed )
{
  // mapping of original axis which clashes to suggested value, respecting right hand rule
  static const QMap<QString, QString> rightHandRulesUpFixed = {
    { u"x"_s, u"z"_s },
    { u"-x"_s, u"-z"_s },
    { u"y"_s, u"x"_s },
    { u"-y"_s, u"-x"_s },
    { u"z"_s, u"y"_s },
    { u"-z"_s, u"-y"_s },
  };
  static const QMap<QString, QString> rightHandRulesForwardFixed = {
    { u"x"_s, u"y"_s },
    { u"-x"_s, u"-y"_s },
    { u"y"_s, u"z"_s },
    { u"-y"_s, u"-z"_s },
    { u"z"_s, u"x"_s },
    { u"-z"_s, u"-x"_s },
  };

  if ( fixedAxis.last( 1 ) == axisWithPossibleConflict.last( 1 ) )
  {
    return isUpFixed ? rightHandRulesUpFixed.value( axisWithPossibleConflict ) : rightHandRulesForwardFixed.value( axisWithPossibleConflict );
  }
  return QString();
}


QgsPoint3DSymbolWidget::QgsPoint3DSymbolWidget( QWidget *parent )
  : Qgs3DSymbolWidget( parent )
{
  setupUi( this );

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

  for ( QComboBox *combo : { mComboModelUpAxis, mComboModelForwardAxis } )
  {
    combo->addItem( tr( "X" ), u"x"_s );
    combo->addItem( tr( "Y" ), u"y"_s );
    combo->addItem( tr( "Z" ), u"z"_s );
    combo->addItem( tr( "-X" ), u"-x"_s );
    combo->addItem( tr( "-Y" ), u"-y"_s );
    combo->addItem( tr( "-Z" ), u"-z"_s );
  }
  mComboModelUpAxis->setCurrentIndex( mComboModelUpAxis->findData( "z" ) );
  mComboModelForwardAxis->setCurrentIndex( mComboModelForwardAxis->findData( "y" ) );

  btnChangeSymbol->setSymbolType( Qgis::SymbolType::Marker );
  btnChangeSymbol->setDialogTitle( tr( "Billboard symbol" ) );

  QgsPoint3DSymbol defaultSymbol;
  setSymbol( &defaultSymbol, nullptr );
  onShapeChanged();

  connect( cboAltClamping, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPoint3DSymbolWidget::changed );
  connect( cboShape, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPoint3DSymbolWidget::onShapeChanged );
  QList<QDoubleSpinBox *> spinWidgets;
  spinWidgets << spinRadius << spinTopRadius << spinBottomRadius << spinMinorRadius << spinSize << spinLength << spinBillboardHeight;
  const auto constSpinWidgets = spinWidgets;
  for ( QDoubleSpinBox *spinBox : constSpinWidgets )
    connect( spinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPoint3DSymbolWidget::changed );
  connect( lineEditModel, &QgsAbstractFileContentSourceLineEdit::sourceChanged, this, &QgsPoint3DSymbolWidget::changed );
  connect( widgetMaterial, &QgsMaterialWidget::changed, this, &QgsPoint3DSymbolWidget::changed );
  connect( widgetMaterial, &QgsMaterialWidget::showPanel, this, &QgsPoint3DSymbolWidget::openPanel );
  connect( btnChangeSymbol, static_cast<void ( QgsSymbolButton::* )()>( &QgsSymbolButton::changed ), this, &QgsPoint3DSymbolWidget::changed );

  // Sync between billboard height and TZ
  connect( spinBillboardHeight, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsPoint3DSymbolWidget::onBillboardHeightChanged );

  connect( mComboModelUpAxis, qOverload< int >( &QComboBox::currentIndexChanged ), this, [this] {
    // ensure up axis is different to forward axis
    const QString upAxis = mComboModelUpAxis->currentData().toString();
    const QString forwardAxis = mComboModelForwardAxis->currentData().toString();
    const QString resolvedAxisConflict = resolveAxisConflict( forwardAxis, upAxis, true );
    if ( !resolvedAxisConflict.isEmpty() )
    {
      whileBlocking( mComboModelForwardAxis )->setCurrentIndex( mComboModelForwardAxis->findData( resolvedAxisConflict ) );
    }

    emit changed();
  } );
  connect( mComboModelForwardAxis, qOverload< int >( &QComboBox::currentIndexChanged ), this, [this] {
    // ensure up axis is different to forward axis
    const QString upAxis = mComboModelUpAxis->currentData().toString();
    const QString forwardAxis = mComboModelForwardAxis->currentData().toString();
    const QString resolvedAxisConflict = resolveAxisConflict( upAxis, forwardAxis, false );
    if ( !resolvedAxisConflict.isEmpty() )
    {
      whileBlocking( mComboModelUpAxis )->setCurrentIndex( mComboModelUpAxis->findData( resolvedAxisConflict ) );
    }
    emit changed();
  } );

  widgetMaterial->setDockMode( dockMode() );
  widgetMaterial->setStyle( QgsMaterialSettingsWidget::WidgetStyle::Compact );
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
  mRenderingTechnique = Qgis::MaterialRenderingTechnique::InstancedPoints;
  bool forceNullMaterial = false;
  switch ( pointSymbol->shape() )
  {
    case Qgis::Point3DShape::Sphere:
      spinRadius->setValue( pointSymbol->shapeProperty( u"radius"_s ).toDouble() );
      break;
    case Qgis::Point3DShape::Cylinder:
      spinRadius->setValue( pointSymbol->shapeProperty( u"radius"_s ).toDouble() );
      spinLength->setValue( pointSymbol->shapeProperty( u"length"_s ).toDouble() );
      break;
    case Qgis::Point3DShape::Cube:
      spinSize->setValue( pointSymbol->shapeProperty( u"size"_s ).toDouble() );
      break;
    case Qgis::Point3DShape::Cone:
      spinTopRadius->setValue( pointSymbol->shapeProperty( u"topRadius"_s ).toDouble() );
      spinBottomRadius->setValue( pointSymbol->shapeProperty( u"bottomRadius"_s ).toDouble() );
      spinLength->setValue( pointSymbol->shapeProperty( u"length"_s ).toDouble() );
      break;
    case Qgis::Point3DShape::Plane:
      spinSize->setValue( pointSymbol->shapeProperty( u"size"_s ).toDouble() );
      break;
    case Qgis::Point3DShape::Torus:
      spinRadius->setValue( pointSymbol->shapeProperty( u"radius"_s ).toDouble() );
      spinMinorRadius->setValue( pointSymbol->shapeProperty( u"minorRadius"_s ).toDouble() );
      break;
    case Qgis::Point3DShape::Model:
    {
      lineEditModel->setSource( pointSymbol->shapeProperty( u"model"_s ).toString() );
      // "overwriteMaterial" is a legacy setting indicating that non-null material should be used
      forceNullMaterial = ( pointSymbol->shapeProperties().contains( u"overwriteMaterial"_s ) && !pointSymbol->shapeProperties().value( u"overwriteMaterial"_s ).toBool() )
                          || !pointSymbol->materialSettings()
                          || pointSymbol->materialSettings()->type() == "null"_L1;
      mRenderingTechnique = Qgis::MaterialRenderingTechnique::TrianglesFromModel;

      whileBlocking( mComboModelUpAxis )->setCurrentIndex( mComboModelUpAxis->findData( pointSymbol->shapeProperty( u"upAxis"_s ).toString() ) );
      whileBlocking( mComboModelForwardAxis )->setCurrentIndex( mComboModelForwardAxis->findData( pointSymbol->shapeProperty( u"forwardAxis"_s ).toString() ) );
      break;
    }
    case Qgis::Point3DShape::Billboard:
      if ( pointSymbol->billboardSymbol() )
      {
        btnChangeSymbol->setSymbol( pointSymbol->billboardSymbol()->clone() );
      }
      mRenderingTechnique = Qgis::MaterialRenderingTechnique::Billboards;
      break;
    case Qgis::Point3DShape::ExtrudedText:
      break;
  }

  widgetMaterial->setSettings( pointSymbol->materialSettings(), layer );
  widgetMaterial->setTechnique( mRenderingTechnique );
  widgetMaterial->setFilterByTechnique( true );
  emit renderingTechniqueChanged();

  if ( forceNullMaterial )
  {
    widgetMaterial->setType( u"null"_s );
  }

  mTransform = pointSymbol->transform();
  QVector3D translation, scale;
  QQuaternion rotation;
  Qgs3DUtils::decomposeTransformMatrix( mTransform, translation, rotation, scale );
  spinBillboardHeight->setValue( translation.z() );
}

QgsAbstract3DSymbol *QgsPoint3DSymbolWidget::symbol()
{
  QVariantMap vm;
  auto sym = std::make_unique<QgsPoint3DSymbol>();
  sym->setBillboardSymbol( static_cast<QgsMarkerSymbol *>( QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ) ) );
  switch ( cboShape->currentData().value<Qgis::Point3DShape>() )
  {
    case Qgis::Point3DShape::Sphere:
      vm[u"radius"_s] = spinRadius->value();
      break;
    case Qgis::Point3DShape::Cylinder:
      vm[u"radius"_s] = spinRadius->value();
      vm[u"length"_s] = spinLength->value();
      break;
    case Qgis::Point3DShape::Cube:
      vm[u"size"_s] = spinSize->value();
      break;
    case Qgis::Point3DShape::Cone:
      vm[u"topRadius"_s] = spinTopRadius->value();
      vm[u"bottomRadius"_s] = spinBottomRadius->value();
      vm[u"length"_s] = spinLength->value();
      break;
    case Qgis::Point3DShape::Plane:
      vm[u"size"_s] = spinSize->value();
      break;
    case Qgis::Point3DShape::Torus:
      vm[u"radius"_s] = spinRadius->value();
      vm[u"minorRadius"_s] = spinMinorRadius->value();
      break;
    case Qgis::Point3DShape::Model:
      vm[u"model"_s] = lineEditModel->source();
      vm[u"upAxis"_s] = mComboModelUpAxis->currentData().toString();
      vm[u"forwardAxis"_s] = mComboModelForwardAxis->currentData().toString();
      break;
    case Qgis::Point3DShape::Billboard:
      sym->setBillboardSymbol( btnChangeSymbol->clonedSymbol<QgsMarkerSymbol>() );
      break;
    case Qgis::Point3DShape::ExtrudedText:
      break;
  }

  sym->setAltitudeClamping( static_cast<Qgis::AltitudeClamping>( cboAltClamping->currentIndex() ) );
  sym->setShape( cboShape->itemData( cboShape->currentIndex() ).value<Qgis::Point3DShape>() );
  sym->setShapeProperties( vm );
  sym->setMaterialSettings( widgetMaterial->settings().release() );
  sym->setTransform( mTransform );

  return sym.release();
}

QString QgsPoint3DSymbolWidget::symbolType() const
{
  return u"point"_s;
}

Qgis::MaterialRenderingTechnique QgsPoint3DSymbolWidget::renderingTechnique() const
{
  return mRenderingTechnique;
}

void QgsPoint3DSymbolWidget::setDockMode( bool dockMode )
{
  widgetMaterial->setDockMode( dockMode );
  Qgs3DSymbolWidget::setDockMode( dockMode );
}

void QgsPoint3DSymbolWidget::onShapeChanged()
{
  QList<QWidget *> allWidgets;
  allWidgets
    << labelSize
    << spinSize
    << labelRadius
    << spinRadius
    << labelMinorRadius
    << spinMinorRadius
    << labelTopRadius
    << spinTopRadius
    << labelBottomRadius
    << spinBottomRadius
    << labelLength
    << spinLength
    << labelModel
    << lineEditModel
    << labelBillboardHeight
    << spinBillboardHeight
    << labelBillboardSymbol
    << btnChangeSymbol
    << mComboModelForwardAxis
    << mComboModelUpAxis
    << labelUpAxis
    << labelForwardAxis;

  materialsGroupBox->show();
  QList<QWidget *> activeWidgets;
  mRenderingTechnique = Qgis::MaterialRenderingTechnique::InstancedPoints;
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
      activeWidgets << labelModel << lineEditModel << mComboModelForwardAxis << mComboModelUpAxis << labelUpAxis << labelForwardAxis;
      mRenderingTechnique = Qgis::MaterialRenderingTechnique::TrianglesFromModel;
      break;
    case Qgis::Point3DShape::Billboard:
      activeWidgets << labelBillboardHeight << spinBillboardHeight << labelBillboardSymbol << btnChangeSymbol;
      // Always hide material for billboard
      materialsGroupBox->hide();
      mRenderingTechnique = Qgis::MaterialRenderingTechnique::Billboards;
      break;
    case Qgis::Point3DShape::ExtrudedText:
      break;
  }

  widgetMaterial->setTechnique( mRenderingTechnique );
  widgetMaterial->setFilterByTechnique( true );
  emit renderingTechniqueChanged();

  if ( cboShape->currentIndex() == 6 )
  {
    // going from different shape -> model resets the material to the null type
    widgetMaterial->setType( u"null"_s );
  }

  const auto constAllWidgets = allWidgets;
  for ( QWidget *w : constAllWidgets )
  {
    w->setVisible( activeWidgets.contains( w ) );
  }

  emit changed();
}


void QgsPoint3DSymbolWidget::onBillboardHeightChanged()
{
  QVector3D translation, scale;
  QQuaternion rotation;

  Qgs3DUtils::decomposeTransformMatrix( mTransform, translation, rotation, scale );
  translation.setZ( spinBillboardHeight->value() );

  mTransform.setToIdentity();
  mTransform.translate( translation );
  mTransform.rotate( rotation );
  mTransform.scale( scale );

  emit changed();
}
