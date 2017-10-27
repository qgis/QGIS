/***************************************************************************
                         qgscomposershapewidget.cpp
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

#include "qgscomposershapewidget.h"
#include "qgscomposershape.h"
#include "qgscomposeritemwidget.h"
#include "qgscomposition.h"
#include "qgsstyle.h"
#include "qgssymbolselectordialog.h"
#include "qgssymbollayerutils.h"
#include <QColorDialog>

QgsComposerShapeWidget::QgsComposerShapeWidget( QgsComposerShape *composerShape ): QgsComposerItemBaseWidget( nullptr, composerShape ), mComposerShape( composerShape )
{
  setupUi( this );
  connect( mShapeComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsComposerShapeWidget::mShapeComboBox_currentIndexChanged );
  connect( mCornerRadiusSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerShapeWidget::mCornerRadiusSpinBox_valueChanged );
  setPanelTitle( tr( "Shape properties" ) );

  //add widget for general composer item properties
  QgsComposerItemWidget *itemPropertiesWidget = new QgsComposerItemWidget( this, composerShape );

  //shapes don't use background or frame, since the symbol style is set through a QgsSymbolSelectorWidget
  itemPropertiesWidget->showBackgroundGroup( false );
  itemPropertiesWidget->showFrameGroup( false );

  mainLayout->addWidget( itemPropertiesWidget );

  blockAllSignals( true );

  //shape types
  mShapeComboBox->addItem( tr( "Ellipse" ) );
  mShapeComboBox->addItem( tr( "Rectangle" ) );
  mShapeComboBox->addItem( tr( "Triangle" ) );

  mShapeStyleButton->setSymbolType( QgsSymbol::Fill );

  setGuiElementValues();

  blockAllSignals( false );

  if ( mComposerShape )
  {
    connect( mComposerShape, &QgsComposerObject::itemChanged, this, &QgsComposerShapeWidget::setGuiElementValues );
    mShapeStyleButton->registerExpressionContextGenerator( mComposerShape );
  }
  connect( mShapeStyleButton, &QgsSymbolButton::changed, this, &QgsComposerShapeWidget::symbolChanged );
  mShapeStyleButton->setLayer( atlasCoverageLayer() );
}

void QgsComposerShapeWidget::blockAllSignals( bool block )
{
  mShapeComboBox->blockSignals( block );
  mCornerRadiusSpinBox->blockSignals( block );
  mShapeStyleButton->blockSignals( block );
}

void QgsComposerShapeWidget::setGuiElementValues()
{
  if ( !mComposerShape )
  {
    return;
  }

  blockAllSignals( true );

  mShapeStyleButton->setSymbol( mComposerShape->shapeStyleSymbol()->clone() );

  mCornerRadiusSpinBox->setValue( mComposerShape->cornerRadius() );
  if ( mComposerShape->shapeType() == QgsComposerShape::Ellipse )
  {
    mShapeComboBox->setCurrentIndex( mShapeComboBox->findText( tr( "Ellipse" ) ) );
    mCornerRadiusSpinBox->setEnabled( false );
  }
  else if ( mComposerShape->shapeType() == QgsComposerShape::Rectangle )
  {
    mShapeComboBox->setCurrentIndex( mShapeComboBox->findText( tr( "Rectangle" ) ) );
    mCornerRadiusSpinBox->setEnabled( true );
  }
  else if ( mComposerShape->shapeType() == QgsComposerShape::Triangle )
  {
    mShapeComboBox->setCurrentIndex( mShapeComboBox->findText( tr( "Triangle" ) ) );
    mCornerRadiusSpinBox->setEnabled( false );
  }

  blockAllSignals( false );
}

void QgsComposerShapeWidget::symbolChanged()
{
  if ( !mComposerShape )
  {
    return;
  }

  mComposerShape->beginCommand( tr( "Shape style changed" ) );
  mComposerShape->setShapeStyleSymbol( mShapeStyleButton->clonedSymbol<QgsFillSymbol>() );
  mComposerShape->endCommand();
}

void QgsComposerShapeWidget::mCornerRadiusSpinBox_valueChanged( double val )
{
  if ( mComposerShape )
  {
    mComposerShape->beginCommand( tr( "Shape radius changed" ), QgsComposerMergeCommand::ShapeCornerRadius );
    mComposerShape->setCornerRadius( val );
    mComposerShape->update();
    mComposerShape->endCommand();
  }
}

void QgsComposerShapeWidget::mShapeComboBox_currentIndexChanged( const QString &text )
{
  if ( !mComposerShape )
  {
    return;
  }

  mComposerShape->beginCommand( tr( "Shape type changed" ) );
  if ( text == tr( "Ellipse" ) )
  {
    mComposerShape->setShapeType( QgsComposerShape::Ellipse );
  }
  else if ( text == tr( "Rectangle" ) )
  {
    mComposerShape->setShapeType( QgsComposerShape::Rectangle );
  }
  else if ( text == tr( "Triangle" ) )
  {
    mComposerShape->setShapeType( QgsComposerShape::Triangle );
  }
  toggleRadiusSpin( text );
  mComposerShape->update();
  mComposerShape->endCommand();
}

void QgsComposerShapeWidget::toggleRadiusSpin( const QString &shapeText )
{
  if ( shapeText == tr( "Rectangle" ) )
  {
    mCornerRadiusSpinBox->setEnabled( true );
  }
  else
  {
    mCornerRadiusSpinBox->setEnabled( false );
  }
}
