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
  toolBox->addItem( itemPropertiesWidget, tr( "General options" ) );

  blockAllSignals( true );

  //shape types
  mShapeComboBox->addItem( tr( "Ellipse" ) );
  mShapeComboBox->addItem( tr( "Rectangle" ) );
  mShapeComboBox->addItem( tr( "Triangle" ) );

  setGuiElementValues();

  blockAllSignals( false );

  connect( mShapeComboBox, SIGNAL( itemChanged() ), this, SLOT( setGuiElementValues() ) );
}

QgsComposerShapeWidget::~QgsComposerShapeWidget()
{

}

void QgsComposerShapeWidget::blockAllSignals( bool block )
{
  mShapeComboBox->blockSignals( block );
  mOutlineColorButton->blockSignals( block );
  mOutlineWidthSpinBox->blockSignals( block );
  mFillColorButton->blockSignals( block );
  mRotationSpinBox->blockSignals( block );
  mTransparentCheckBox->blockSignals( block );
}

void QgsComposerShapeWidget::setGuiElementValues()
{
  if ( !mComposerShape )
  {
    return;
  }

  blockAllSignals( true );
  mOutlineWidthSpinBox->setValue( mComposerShape->lineWidth() );
  mRotationSpinBox->setValue( mComposerShape->rotation() );
  if ( mComposerShape->shapeType() == QgsComposerShape::Ellipse )
  {
    mShapeComboBox->setCurrentIndex( mShapeComboBox->findText( tr( "Ellipse" ) ) );
  }
  else if ( mComposerShape->shapeType() == QgsComposerShape::Rectangle )
  {
    mShapeComboBox->setCurrentIndex( mShapeComboBox->findText( tr( "Rectangle" ) ) );
  }
  else if ( mComposerShape->shapeType() == QgsComposerShape::Triangle )
  {
    mShapeComboBox->setCurrentIndex( mShapeComboBox->findText( tr( "Triangle" ) ) );
  }

  if ( mComposerShape->transparentFill() )
  {
    mTransparentCheckBox->setCheckState( Qt::Checked );
    mFillColorButton->setEnabled( false );
  }
  else
  {
    mTransparentCheckBox->setCheckState( Qt::Unchecked );
    mFillColorButton->setEnabled( true );
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
  mComposerShape->update();
  mComposerShape->endCommand();
}

void QgsComposerShapeWidget::on_mOutlineColorButton_clicked()
{
  if ( !mComposerShape )
  {
    return;
  }
  QColor existingColor = mComposerShape->outlineColor();
#if QT_VERSION >= 0x040500
  QColor newColor = QColorDialog::getColor( existingColor, 0, tr( "Select outline color" ), QColorDialog::ShowAlphaChannel );
#else
  QColor newColor = QColorDialog::getColor( existingColor );
#endif
  if ( newColor.isValid() )
  {
    mComposerShape->beginCommand( tr( "Shape outline color" ) );
    mComposerShape->setOutlineColor( newColor );
    mComposerShape->update();
    mComposerShape->endCommand();
  }
}

void QgsComposerShapeWidget::on_mOutlineWidthSpinBox_valueChanged( double d )
{
  if ( !mComposerShape )
  {
    return;
  }
  mComposerShape->beginCommand( tr( "Shape outline width" ), QgsComposerMergeCommand::ShapeOutlineWidth );
  mComposerShape->setLineWidth( d );
  mComposerShape->update();
  mComposerShape->endCommand();
}

void QgsComposerShapeWidget::on_mTransparentCheckBox_stateChanged( int state )
{
  if ( !mComposerShape )
  {
    return;
  }

  mComposerShape->beginCommand( tr( "Shape transparency toggled" ) );
  if ( state == Qt::Checked )
  {
    mComposerShape->setTransparentFill( true );
    mFillColorButton->setEnabled( false );
  }
  else
  {
    mComposerShape->setTransparentFill( false );
    mFillColorButton->setEnabled( true );
  }
  mComposerShape->update();
  mComposerShape->endCommand();
}


void QgsComposerShapeWidget::on_mFillColorButton_clicked()
{
  if ( !mComposerShape )
  {
    return;
  }
  QColor existingColor = mComposerShape->fillColor();
#if QT_VERSION >= 0x040500
  QColor newColor = QColorDialog::getColor( existingColor, 0, tr( "Select fill color" ), QColorDialog::ShowAlphaChannel );
#else
  QColor newColor = QColorDialog::getColor( existingColor );
#endif
  if ( newColor.isValid() )
  {
    mComposerShape->beginCommand( tr( "Shape fill color" ) );
    mComposerShape->setFillColor( newColor );
    mComposerShape->update();
    mComposerShape->endCommand();
  }
}
