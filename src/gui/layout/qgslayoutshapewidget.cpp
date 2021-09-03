/***************************************************************************
                         qgslayoutshapewidget.cpp
                         --------------------------
    begin                : November 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco@hugis.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutshapewidget.h"
#include "qgsstyle.h"
#include "qgslayoutitemshape.h"
#include "qgslayout.h"
#include "qgslayoutundostack.h"
#include "qgsvectorlayer.h"
#include "qgsfillsymbol.h"

QgsLayoutShapeWidget::QgsLayoutShapeWidget( QgsLayoutItemShape *shape )
  : QgsLayoutItemBaseWidget( nullptr, shape )
  , mShape( shape )
{
  Q_ASSERT( mShape );

  setupUi( this );
  connect( mShapeComboBox, &QComboBox::currentTextChanged, this, &QgsLayoutShapeWidget::mShapeComboBox_currentIndexChanged );
  connect( mCornerRadiusSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutShapeWidget::mCornerRadiusSpinBox_valueChanged );
  setPanelTitle( tr( "Shape Properties" ) );

  //add widget for general composer item properties
  mItemPropertiesWidget = new QgsLayoutItemPropertiesWidget( this, shape );
  //shapes don't use background or frame, since the symbol style is set through a QgsSymbolSelectorWidget
  mItemPropertiesWidget->showBackgroundGroup( false );
  mItemPropertiesWidget->showFrameGroup( false );
  mainLayout->addWidget( mItemPropertiesWidget );

  blockAllSignals( true );

  //shape types
  mShapeComboBox->addItem( tr( "Rectangle" ),  QgsLayoutItemShape::Rectangle );
  mShapeComboBox->addItem( tr( "Ellipse" ), QgsLayoutItemShape::Ellipse );
  mShapeComboBox->addItem( tr( "Triangle" ), QgsLayoutItemShape::Triangle );

  mShapeStyleButton->setSymbolType( Qgis::SymbolType::Fill );
  mRadiusUnitsComboBox->linkToWidget( mCornerRadiusSpinBox );
  mRadiusUnitsComboBox->setConverter( &mShape->layout()->renderContext().measurementConverter() );

  setGuiElementValues();

  blockAllSignals( false );

  connect( mShape, &QgsLayoutObject::changed, this, &QgsLayoutShapeWidget::setGuiElementValues );
  mShapeStyleButton->registerExpressionContextGenerator( mShape );

  connect( mShapeStyleButton, &QgsSymbolButton::changed, this, &QgsLayoutShapeWidget::symbolChanged );
  connect( mRadiusUnitsComboBox, &QgsLayoutUnitsComboBox::changed, this, &QgsLayoutShapeWidget::radiusUnitsChanged );

  mShapeStyleButton->setLayer( coverageLayer() );
  if ( mShape->layout() )
  {
    connect( &mShape->layout()->reportContext(), &QgsLayoutReportContext::layerChanged, mShapeStyleButton, &QgsSymbolButton::setLayer );
  }
}

void QgsLayoutShapeWidget::setMasterLayout( QgsMasterLayoutInterface *masterLayout )
{
  if ( mItemPropertiesWidget )
    mItemPropertiesWidget->setMasterLayout( masterLayout );
}

bool QgsLayoutShapeWidget::setNewItem( QgsLayoutItem *item )
{
  if ( item->type() != QgsLayoutItemRegistry::LayoutShape )
    return false;

  if ( mShape )
  {
    disconnect( mShape, &QgsLayoutObject::changed, this, &QgsLayoutShapeWidget::setGuiElementValues );
  }

  mShape = qobject_cast< QgsLayoutItemShape * >( item );
  mItemPropertiesWidget->setItem( mShape );

  if ( mShape )
  {
    connect( mShape, &QgsLayoutObject::changed, this, &QgsLayoutShapeWidget::setGuiElementValues );
    mShapeStyleButton->registerExpressionContextGenerator( mShape );
  }

  setGuiElementValues();

  return true;
}

void QgsLayoutShapeWidget::blockAllSignals( bool block )
{
  mShapeComboBox->blockSignals( block );
  mCornerRadiusSpinBox->blockSignals( block );
  mRadiusUnitsComboBox->blockSignals( block );
  mShapeStyleButton->blockSignals( block );
}

void QgsLayoutShapeWidget::setGuiElementValues()
{
  if ( !mShape )
  {
    return;
  }

  blockAllSignals( true );

  mShapeStyleButton->setSymbol( mShape->symbol()->clone() );

  mCornerRadiusSpinBox->setValue( mShape->cornerRadius().length() );
  mRadiusUnitsComboBox->setUnit( mShape->cornerRadius().units() );

  mShapeComboBox->setCurrentIndex( mShapeComboBox->findData( mShape->shapeType() ) );
  toggleRadiusSpin( mShape->shapeType() );

  blockAllSignals( false );
}

void QgsLayoutShapeWidget::symbolChanged()
{
  if ( !mShape )
    return;

  mShape->layout()->undoStack()->beginCommand( mShape, tr( "Change Shape Style" ), QgsLayoutItem::UndoShapeStyle );
  mShape->setSymbol( mShapeStyleButton->clonedSymbol<QgsFillSymbol>() );
  mShape->layout()->undoStack()->endCommand();
}

void QgsLayoutShapeWidget::mCornerRadiusSpinBox_valueChanged( double val )
{
  if ( !mShape )
    return;

  mShape->layout()->undoStack()->beginCommand( mShape, tr( "Change Shape Radius" ), QgsLayoutItem::UndoShapeCornerRadius );
  mShape->setCornerRadius( QgsLayoutMeasurement( val, mRadiusUnitsComboBox->unit() ) );
  mShape->layout()->undoStack()->endCommand();
  mShape->update();
}

void QgsLayoutShapeWidget::radiusUnitsChanged()
{
  if ( !mShape )
    return;

  mShape->layout()->undoStack()->beginCommand( mShape, tr( "Change Shape Radius" ), QgsLayoutItem::UndoShapeCornerRadius );
  mShape->setCornerRadius( QgsLayoutMeasurement( mCornerRadiusSpinBox->value(), mRadiusUnitsComboBox->unit() ) );
  mShape->layout()->undoStack()->endCommand();
  mShape->update();
}

void QgsLayoutShapeWidget::mShapeComboBox_currentIndexChanged( const QString & )
{
  if ( !mShape )
  {
    return;
  }

  mShape->layout()->undoStack()->beginCommand( mShape, tr( "Change Shape Type" ) );
  const QgsLayoutItemShape::Shape shape = static_cast< QgsLayoutItemShape::Shape >( mShapeComboBox->currentData().toInt() );
  mShape->setShapeType( shape );
  toggleRadiusSpin( shape );
  mShape->update();
  mShape->layout()->undoStack()->endCommand();
}

void QgsLayoutShapeWidget::toggleRadiusSpin( QgsLayoutItemShape::Shape shape )
{
  switch ( shape )
  {
    case QgsLayoutItemShape::Ellipse:
    case QgsLayoutItemShape::Triangle:
    {
      mCornerRadiusSpinBox->setEnabled( false );
      break;
    }
    case QgsLayoutItemShape::Rectangle:
    {
      mCornerRadiusSpinBox->setEnabled( true );
      break;
    }
  }
}
