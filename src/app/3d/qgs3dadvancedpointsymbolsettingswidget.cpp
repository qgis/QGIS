/***************************************************************************
    qgs3dadvancedpointsymbolsettingswidget.cpp
    ---------------------
    begin                : July 2026
    copyright            : (C) 2026 by Jean Felder
    email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dadvancedpointsymbolsettingswidget.h"

#include "qgs3dutils.h"
#include "qgslayoututils.h"
#include "qgspoint3dsymbol.h"
#include "qgspolygon3dsymbol.h"
#include "qgsvectorlayer.h"

#include <QDialogButtonBox>
#include <QQuaternion>
#include <QString>

#include "moc_qgs3dadvancedpointsymbolsettingswidget.cpp"

using namespace Qt::StringLiterals;

Qgs3DAdvancedPointSymbolSettingsWidget::Qgs3DAdvancedPointSymbolSettingsWidget( const QgsAbstract3DSymbol *symbol, QgsVectorLayer *vLayer, Qgis::MaterialRenderingTechnique renderingTechnique, QWidget *parent )
  : Qgs3DAdvancedSymbolSettingsWidget( symbol, parent )
{
  setupUi( this );

  bool forceNullMaterial = false;

  const QgsPoint3DSymbol *pointSymbol = qgis::down_cast<const QgsPoint3DSymbol *>( symbol );
  // The material depends on the shape
  forceNullMaterial = ( pointSymbol->shapeProperties().contains( u"overwriteMaterial"_s ) && !pointSymbol->shapeProperties().value( u"overwriteMaterial"_s ).toBool() )
                      || !pointSymbol->materialSettings()
                      || pointSymbol->materialSettings()->type() == "null"_L1;

  // Transformation group
  // Always hide material and transformation widgets for billboard
  const bool isBillboard = pointSymbol->shape() == Qgis::Point3DShape::Billboard;
  mGroupTransformation->setVisible( !isBillboard );
  mGroupShading->setVisible( !isBillboard );

  if ( !isBillboard )
  {
    QVector3D translation, scale;
    QQuaternion rotation;
    Qgs3DUtils::decomposeTransformMatrix( pointSymbol->transform(), translation, rotation, scale );

    const QVector3D eulerAngles = rotation.toEulerAngles();

    mSpinTX->setClearValue( 0.0 );
    mSpinTY->setClearValue( 0.0 );
    mSpinTZ->setClearValue( 0.0 );
    mSpinSX->setClearValue( 1.0 );
    mSpinSY->setClearValue( 1.0 );
    mSpinSZ->setClearValue( 1.0 );
    mSpinRX->setClearValue( 0.0 );
    mSpinRY->setClearValue( 0.0 );
    mSpinRZ->setClearValue( 0.0 );
    mSpinTX->setValue( translation.x() );
    mSpinTY->setValue( translation.y() );
    mSpinTZ->setValue( translation.z() );
    mSpinSX->setValue( scale.x() );
    mSpinSY->setValue( scale.y() );
    mSpinSZ->setValue( scale.z() );
    mSpinRX->setValue( QgsLayoutUtils::normalizedAngle( eulerAngles.x() ) );
    mSpinRY->setValue( QgsLayoutUtils::normalizedAngle( eulerAngles.y() ) );
    mSpinRZ->setValue( QgsLayoutUtils::normalizedAngle( eulerAngles.z() ) );

    mButtonDDScaleX->init( static_cast< int >( QgsAbstract3DSymbol::Property::ScaleX ), pointSymbol->dataDefinedProperties(), QgsAbstract3DSymbol::propertyDefinitions(), vLayer, true );
    mButtonDDScaleY->init( static_cast< int >( QgsAbstract3DSymbol::Property::ScaleY ), pointSymbol->dataDefinedProperties(), QgsAbstract3DSymbol::propertyDefinitions(), vLayer, true );
    mButtonDDScaleZ->init( static_cast< int >( QgsAbstract3DSymbol::Property::ScaleZ ), pointSymbol->dataDefinedProperties(), QgsAbstract3DSymbol::propertyDefinitions(), vLayer, true );

    mButtonDDTranslationX->init( static_cast< int >( QgsAbstract3DSymbol::Property::TranslationX ), pointSymbol->dataDefinedProperties(), QgsAbstract3DSymbol::propertyDefinitions(), vLayer, true );
    mButtonDDTranslationY->init( static_cast< int >( QgsAbstract3DSymbol::Property::TranslationY ), pointSymbol->dataDefinedProperties(), QgsAbstract3DSymbol::propertyDefinitions(), vLayer, true );
    mButtonDDTranslationZ->init( static_cast< int >( QgsAbstract3DSymbol::Property::TranslationZ ), pointSymbol->dataDefinedProperties(), QgsAbstract3DSymbol::propertyDefinitions(), vLayer, true );

    mButtonDDRotationX->init( static_cast< int >( QgsAbstract3DSymbol::Property::RotationX ), pointSymbol->dataDefinedProperties(), QgsAbstract3DSymbol::propertyDefinitions(), vLayer, true );
    mButtonDDRotationY->init( static_cast< int >( QgsAbstract3DSymbol::Property::RotationY ), pointSymbol->dataDefinedProperties(), QgsAbstract3DSymbol::propertyDefinitions(), vLayer, true );
    mButtonDDRotationZ->init( static_cast< int >( QgsAbstract3DSymbol::Property::RotationZ ), pointSymbol->dataDefinedProperties(), QgsAbstract3DSymbol::propertyDefinitions(), vLayer, true );

    const QList<QDoubleSpinBox *> spinWidgets = { mSpinTX, mSpinTY, mSpinTZ, mSpinSX, mSpinSY, mSpinSZ, mSpinRX, mSpinRY, mSpinRZ };
    for ( QDoubleSpinBox *spinBox : spinWidgets )
    {
      connect( spinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &Qgs3DAdvancedPointSymbolSettingsWidget::widgetChanged );
    }

    connect( mButtonDDScaleX, &QgsPropertyOverrideButton::changed, this, &Qgs3DAdvancedPointSymbolSettingsWidget::widgetChanged );
    connect( mButtonDDScaleY, &QgsPropertyOverrideButton::changed, this, &Qgs3DAdvancedPointSymbolSettingsWidget::widgetChanged );
    connect( mButtonDDScaleZ, &QgsPropertyOverrideButton::changed, this, &Qgs3DAdvancedPointSymbolSettingsWidget::widgetChanged );
    connect( mButtonDDTranslationX, &QgsPropertyOverrideButton::changed, this, &Qgs3DAdvancedPointSymbolSettingsWidget::widgetChanged );
    connect( mButtonDDTranslationY, &QgsPropertyOverrideButton::changed, this, &Qgs3DAdvancedPointSymbolSettingsWidget::widgetChanged );
    connect( mButtonDDTranslationZ, &QgsPropertyOverrideButton::changed, this, &Qgs3DAdvancedPointSymbolSettingsWidget::widgetChanged );
    connect( mButtonDDRotationX, &QgsPropertyOverrideButton::changed, this, &Qgs3DAdvancedPointSymbolSettingsWidget::widgetChanged );
    connect( mButtonDDRotationY, &QgsPropertyOverrideButton::changed, this, &Qgs3DAdvancedPointSymbolSettingsWidget::widgetChanged );
    connect( mButtonDDRotationZ, &QgsPropertyOverrideButton::changed, this, &Qgs3DAdvancedPointSymbolSettingsWidget::widgetChanged );
  }

  // material widget
  mWidgetMaterial->setSettings( symbol->materialSettings(), vLayer );
  mWidgetMaterial->setDockMode( dockMode() );
  mWidgetMaterial->setTechnique( renderingTechnique );
  mWidgetMaterial->setStyle( QgsMaterialSettingsWidget::WidgetStyle::Full );
  mWidgetMaterial->setFilterByTechnique( true );
  if ( forceNullMaterial )
  {
    mWidgetMaterial->setType( u"null"_s );
  }

  connect( mWidgetMaterial, &QgsMaterialWidget::changed, this, &Qgs3DAdvancedPointSymbolSettingsWidget::widgetChanged );
}

std::unique_ptr<QgsAbstract3DSymbol> Qgs3DAdvancedPointSymbolSettingsWidget::symbol() const
{
  std::unique_ptr<QgsAbstract3DSymbol> symbol( mBaseSymbol->clone() );
  symbol->setMaterialSettings( mWidgetMaterial->settings().release() );

  QgsPoint3DSymbol *pointSymbol = qgis::down_cast<QgsPoint3DSymbol *>( symbol.get() );
  const QQuaternion rotation( QQuaternion::fromEulerAngles( static_cast<float>( mSpinRX->value() ), static_cast<float>( mSpinRY->value() ), static_cast<float>( mSpinRZ->value() ) ) );
  const QVector3D scale( static_cast<float>( mSpinSX->value() ), static_cast<float>( mSpinSY->value() ), static_cast<float>( mSpinSZ->value() ) );
  const QVector3D translation( static_cast<float>( mSpinTX->value() ), static_cast<float>( mSpinTY->value() ), static_cast<float>( mSpinTZ->value() ) );

  QMatrix4x4 transform;
  transform.translate( translation );
  transform.scale( scale );
  transform.rotate( rotation );
  pointSymbol->setTransform( transform );

  QgsPropertyCollection ddp;
  ddp.setProperty( QgsAbstract3DSymbol::Property::ScaleX, mButtonDDScaleX->toProperty() );
  ddp.setProperty( QgsAbstract3DSymbol::Property::ScaleY, mButtonDDScaleY->toProperty() );
  ddp.setProperty( QgsAbstract3DSymbol::Property::ScaleZ, mButtonDDScaleZ->toProperty() );
  ddp.setProperty( QgsAbstract3DSymbol::Property::TranslationX, mButtonDDTranslationX->toProperty() );
  ddp.setProperty( QgsAbstract3DSymbol::Property::TranslationY, mButtonDDTranslationY->toProperty() );
  ddp.setProperty( QgsAbstract3DSymbol::Property::TranslationZ, mButtonDDTranslationZ->toProperty() );
  ddp.setProperty( QgsAbstract3DSymbol::Property::RotationX, mButtonDDRotationX->toProperty() );
  ddp.setProperty( QgsAbstract3DSymbol::Property::RotationY, mButtonDDRotationY->toProperty() );
  ddp.setProperty( QgsAbstract3DSymbol::Property::RotationZ, mButtonDDRotationZ->toProperty() );
  pointSymbol->setDataDefinedProperties( ddp );


  return symbol;
}
