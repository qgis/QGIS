/***************************************************************************
                         qgscomposerarrowwidget.cpp
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

#include "qgscomposerarrowwidget.h"
#include "qgscomposerarrow.h"
#include "qgscomposeritemwidget.h"
#include <QColorDialog>
#include <QFileDialog>
#include <QFileInfo>

QgsComposerArrowWidget::QgsComposerArrowWidget( QgsComposerArrow* arrow ): QWidget( 0 ), mArrow( arrow )
{
  setupUi( this );
  mRadioButtonGroup = new QButtonGroup( this );
  mRadioButtonGroup->addButton( mDefaultMarkerRadioButton );
  mRadioButtonGroup->addButton( mNoMarkerRadioButton );
  mRadioButtonGroup->addButton( mSvgMarkerRadioButton );
  mRadioButtonGroup->setExclusive( true );

  //disable the svg related gui elements by default
  on_mSvgMarkerRadioButton_toggled( false );

  //add widget for general composer item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, mArrow );
  toolBox->addItem( itemPropertiesWidget, tr( "General options" ) );

  setGuiElementValues();

  if ( arrow )
  {
    connect( arrow, SIGNAL( itemChanged() ), this, SLOT( setGuiElementValues() ) );
  }
}

QgsComposerArrowWidget::~QgsComposerArrowWidget()
{

}

void QgsComposerArrowWidget::on_mOutlineWidthSpinBox_valueChanged( double d )
{
  if ( !mArrow )
  {
    return;
  }

  mArrow->beginCommand( tr( "Arrow outline width" ), QgsComposerMergeCommand::ArrowOutlineWidth );
  mArrow->setOutlineWidth( d );
  mArrow->update();
  mArrow->endCommand();
}

void QgsComposerArrowWidget::on_mArrowHeadWidthSpinBox_valueChanged( double d )
{
  if ( !mArrow )
  {
    return;
  }

  mArrow->beginCommand( tr( "Arrowhead width" ), QgsComposerMergeCommand::ArrowHeadWidth );
  mArrow->setArrowHeadWidth( d );
  mArrow->update();
  mArrow->endCommand();
}

void QgsComposerArrowWidget::on_mArrowColorButton_clicked()
{
  if ( !mArrow )
  {
    return;
  }

#if QT_VERSION >= 0x040500
  QColor newColor = QColorDialog::getColor( mArrow->arrowColor(), 0, tr( "Arrow color" ), QColorDialog::ShowAlphaChannel );
#else
  QColor newColor = QColorDialog::getColor( mArrow->arrowColor() );
#endif
  if ( newColor.isValid() )
  {
    mArrow->beginCommand( tr( "Arrow color changed" ) );
    mArrow->setArrowColor( newColor );
    mArrow->update();
    mArrow->endCommand();
  }
}

void QgsComposerArrowWidget::blockAllSignals( bool block )
{
  mArrowColorButton->blockSignals( block );
  mOutlineWidthSpinBox->blockSignals( block );
  mArrowHeadWidthSpinBox->blockSignals( block );
  mDefaultMarkerRadioButton->blockSignals( block );
  mNoMarkerRadioButton->blockSignals( block );
  mSvgMarkerRadioButton->blockSignals( block );
  mStartMarkerLineEdit->blockSignals( block );
  mStartMarkerToolButton->blockSignals( block );
  mEndMarkerLineEdit->blockSignals( block );
  mEndMarkerToolButton->blockSignals( block );
}

void QgsComposerArrowWidget::setGuiElementValues()
{
  if ( !mArrow )
  {
    return;
  }

  blockAllSignals( true );
  mOutlineWidthSpinBox->setValue( mArrow->outlineWidth() );
  mArrowHeadWidthSpinBox->setValue( mArrow->arrowHeadWidth() );

  QgsComposerArrow::MarkerMode mode = mArrow->markerMode();
  if ( mode == QgsComposerArrow::DefaultMarker )
  {
    mDefaultMarkerRadioButton->setChecked( true );
  }
  else if ( mode == QgsComposerArrow::NoMarker )
  {
    mNoMarkerRadioButton->setChecked( true );
  }
  else //svg marker
  {
    mSvgMarkerRadioButton->setChecked( true );
    enableSvgInputElements( true );
  }
  mStartMarkerLineEdit->setText( mArrow->startMarker() );
  mEndMarkerLineEdit->setText( mArrow->endMarker() );
  blockAllSignals( false );
}

void QgsComposerArrowWidget::enableSvgInputElements( bool enable )
{
  mStartMarkerLineEdit->setEnabled( enable );
  mStartMarkerToolButton->setEnabled( enable );
  mEndMarkerLineEdit->setEnabled( enable );
  mEndMarkerToolButton->setEnabled( enable );
}

void QgsComposerArrowWidget::on_mDefaultMarkerRadioButton_toggled( bool toggled )
{
  if ( mArrow && toggled )
  {
    mArrow->beginCommand( tr( "Arrow marker changed" ) );
    mArrow->setMarkerMode( QgsComposerArrow::DefaultMarker );
    mArrow->update();
    mArrow->endCommand();
  }
}

void QgsComposerArrowWidget::on_mNoMarkerRadioButton_toggled( bool toggled )
{
  if ( mArrow && toggled )
  {
    mArrow->beginCommand( tr( "Arrow marker changed" ) );
    mArrow->setMarkerMode( QgsComposerArrow::NoMarker );
    mArrow->update();
    mArrow->endCommand();
  }
}

void QgsComposerArrowWidget::on_mSvgMarkerRadioButton_toggled( bool toggled )
{
  enableSvgInputElements( toggled );
  if ( mArrow && toggled )
  {
    mArrow->beginCommand( tr( "Arrow marker changed" ) );
    mArrow->setMarkerMode( QgsComposerArrow::SVGMarker );
    mArrow->update();
    mArrow->endCommand();
  }
}

void QgsComposerArrowWidget::on_mStartMarkerLineEdit_editingFinished( const QString & text )
{
  if ( mArrow )
  {
    mArrow->beginCommand( tr( "Arrow start marker" ) );
    QFileInfo fi( text );
    if ( fi.exists() )
    {
      mArrow->setStartMarker( text );
    }
    else
    {
      mArrow->setStartMarker( "" );
    }
    mArrow->update();
    mArrow->endCommand();
  }
}

void QgsComposerArrowWidget::on_mEndMarkerLineEdit_editingFinished( const QString & text )
{
  if ( mArrow )
  {
    mArrow->beginCommand( tr( "Arrow start marker" ) );
    QFileInfo fi( text );
    if ( fi.exists() )
    {
      mArrow->setEndMarker( text );
    }
    else
    {
      mArrow->setEndMarker( "" );
    }
    mArrow->update();
    mArrow->endCommand();
  }
}

void QgsComposerArrowWidget::on_mStartMarkerToolButton_clicked()
{
  QFileInfo fi( mStartMarkerLineEdit->text() );
  QString svgFileName = QFileDialog::getOpenFileName( 0, tr( "Start marker svg file" ), fi.dir().absolutePath() );
  if ( !svgFileName.isNull() )
  {
    mArrow->beginCommand( tr( "Arrow start marker" ) );
    mStartMarkerLineEdit->setText( svgFileName );
    mArrow->endCommand();
  }
}

void QgsComposerArrowWidget::on_mEndMarkerToolButton_clicked()
{
  QFileInfo fi( mEndMarkerLineEdit->text() );
  QString svgFileName = QFileDialog::getOpenFileName( 0, tr( "End marker svg file" ), fi.dir().absolutePath() );
  if ( !svgFileName.isNull() )
  {
    mArrow->beginCommand( tr( "Arrow end marker" ) );
    mEndMarkerLineEdit ->setText( svgFileName );
    mArrow->endCommand();
  }
}
