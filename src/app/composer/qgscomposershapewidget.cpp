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
#include <QColorDialog>

QgsComposerShapeWidget::QgsComposerShapeWidget( QgsComposerShape* composerShape ): QWidget( 0 ), mComposerShape( composerShape )
{
  setupUi( this );

  //add widget for general composer item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, composerShape );
  mainLayout->addWidget( itemPropertiesWidget );

  blockAllSignals( true );

  //shape types
  mShapeComboBox->addItem( tr( "Ellipse" ) );
  mShapeComboBox->addItem( tr( "Rectangle" ) );
  mShapeComboBox->addItem( tr( "Triangle" ) );

  setGuiElementValues();

  blockAllSignals( false );

  if ( mComposerShape )
  {
    connect( mComposerShape, SIGNAL( itemChanged() ), this, SLOT( setGuiElementValues() ) );
  }
}

QgsComposerShapeWidget::~QgsComposerShapeWidget()
{

}

void QgsComposerShapeWidget::blockAllSignals( bool block )
{
  mShapeComboBox->blockSignals( block );
  mRotationSpinBox->blockSignals( block );
  mCornerRadiusSpinBox->blockSignals( block );
}

void QgsComposerShapeWidget::setGuiElementValues()
{
  if ( !mComposerShape )
  {
    return;
  }

  blockAllSignals( true );

  mRotationSpinBox->setValue( mComposerShape->rotation() );
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

void QgsComposerShapeWidget::on_mRotationSpinBox_valueChanged( int val )
{
  if ( mComposerShape )
  {
    mComposerShape->beginCommand( tr( "Shape rotation changed" ), QgsComposerMergeCommand::ShapeRotation );
    mComposerShape->setRotation( val );
    mComposerShape->update();
    mComposerShape->endCommand();
  }
}

void QgsComposerShapeWidget::on_mCornerRadiusSpinBox_valueChanged( double val )
{
  if ( mComposerShape )
  {
    mComposerShape->beginCommand( tr( "Shape radius changed" ), QgsComposerMergeCommand::ShapeRotation );
    mComposerShape->setCornerRadius( val );
    mComposerShape->update();
    mComposerShape->endCommand();
  }
}

void QgsComposerShapeWidget::on_mShapeComboBox_currentIndexChanged( const QString& text )
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

void QgsComposerShapeWidget::toggleRadiusSpin( const QString& shapeText )
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



