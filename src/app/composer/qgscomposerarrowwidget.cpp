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
#include "qgssymbolv2selectordialog.h"
#include "qgsstylev2.h"
#include "qgssymbolv2.h"
#include <QColorDialog>
#include <QFileDialog>
#include <QFileInfo>

QgsComposerArrowWidget::QgsComposerArrowWidget( QgsComposerArrow* arrow ): QgsComposerItemBaseWidget( 0, arrow ), mArrow( arrow )
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
  mainLayout->addWidget( itemPropertiesWidget );

  mArrowHeadOutlineColorButton->setColorDialogTitle( tr( "Select arrow head outline color" ) );
  mArrowHeadOutlineColorButton->setAllowAlpha( true );
  mArrowHeadOutlineColorButton->setContext( "composer" );
  mArrowHeadOutlineColorButton->setNoColorString( tr( "Transparent outline" ) );
  mArrowHeadOutlineColorButton->setShowNoColor( true );
  mArrowHeadFillColorButton->setColorDialogTitle( tr( "Select arrow head fill color" ) );
  mArrowHeadFillColorButton->setAllowAlpha( true );
  mArrowHeadFillColorButton->setContext( "composer" );
  mArrowHeadFillColorButton->setNoColorString( tr( "Transparent fill" ) );
  mArrowHeadFillColorButton->setShowNoColor( true );

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

  mArrow->beginCommand( tr( "Arrow head outline width" ), QgsComposerMergeCommand::ArrowOutlineWidth );
  mArrow->setArrowHeadOutlineWidth( d );
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

void QgsComposerArrowWidget::on_mArrowHeadFillColorButton_colorChanged( const QColor& newColor )
{
  if ( !mArrow )
  {
    return;
  }

  mArrow->beginCommand( tr( "Arrow head fill color" ) );
  mArrow->setArrowHeadFillColor( newColor );
  mArrow->update();
  mArrow->endCommand();
}

void QgsComposerArrowWidget::on_mArrowHeadOutlineColorButton_colorChanged( const QColor &newColor )
{
  if ( !mArrow )
  {
    return;
  }

  mArrow->beginCommand( tr( "Arrow head outline color" ) );
  mArrow->setArrowHeadOutlineColor( newColor );
  mArrow->update();
  mArrow->endCommand();
}

void QgsComposerArrowWidget::blockAllSignals( bool block )
{
  mLineStyleButton->blockSignals( block );
  mArrowHeadFillColorButton->blockSignals( block );
  mArrowHeadOutlineColorButton->blockSignals( block );
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
  mArrowHeadFillColorButton->setColor( mArrow->arrowHeadFillColor() );
  mArrowHeadOutlineColorButton->setColor( mArrow->arrowHeadOutlineColor() );
  mOutlineWidthSpinBox->setValue( mArrow->arrowHeadOutlineWidth() );
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

  updateLineSymbolMarker();

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

void QgsComposerArrowWidget::on_mStartMarkerLineEdit_textChanged( const QString & text )
{
  if ( mArrow )
  {
    mArrow->beginCommand( tr( "Arrow start marker" ) );
    QFileInfo fi( text );
    if ( fi.exists() && fi.isFile() )
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

void QgsComposerArrowWidget::on_mEndMarkerLineEdit_textChanged( const QString & text )
{
  if ( mArrow )
  {
    mArrow->beginCommand( tr( "Arrow end marker" ) );
    QFileInfo fi( text );
    if ( fi.exists() && fi.isFile() )
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
  QSettings s;
  QString openDir;

  if ( !mStartMarkerLineEdit->text().isEmpty() )
  {
    QFileInfo fi( mStartMarkerLineEdit->text() );
    openDir = fi.dir().absolutePath();
  }

  if ( openDir.isEmpty() )
  {
    openDir = s.value( "/UI/lastComposerMarkerDir", "" ).toString();
  }

  QString svgFileName = QFileDialog::getOpenFileName( this, tr( "Start marker svg file" ), openDir );
  if ( !svgFileName.isNull() )
  {
    QFileInfo fileInfo( svgFileName );
    s.setValue( "/UI/lastComposerMarkerDir", fileInfo.absolutePath() );
    mArrow->beginCommand( tr( "Arrow start marker" ) );
    mStartMarkerLineEdit->setText( svgFileName );
    mArrow->endCommand();
  }
}

void QgsComposerArrowWidget::on_mEndMarkerToolButton_clicked()
{
  QSettings s;
  QString openDir;

  if ( !mEndMarkerLineEdit->text().isEmpty() )
  {
    QFileInfo fi( mEndMarkerLineEdit->text() );
    openDir = fi.dir().absolutePath();
  }

  if ( openDir.isEmpty() )
  {
    openDir = s.value( "/UI/lastComposerMarkerDir", "" ).toString();
  }

  QString svgFileName = QFileDialog::getOpenFileName( this, tr( "End marker svg file" ), openDir );
  if ( !svgFileName.isNull() )
  {
    QFileInfo fileInfo( svgFileName );
    s.setValue( "/UI/lastComposerMarkerDir", fileInfo.absolutePath() );
    mArrow->beginCommand( tr( "Arrow end marker" ) );
    mEndMarkerLineEdit->setText( svgFileName );
    mArrow->endCommand();
  }
}

void QgsComposerArrowWidget::on_mLineStyleButton_clicked()
{
  if ( !mArrow )
  {
    return;
  }

  QgsLineSymbolV2* newSymbol = dynamic_cast<QgsLineSymbolV2*>( mArrow->lineSymbol()->clone() );
  QgsSymbolV2SelectorDialog d( newSymbol, QgsStyleV2::defaultStyle(), 0, this );
  d.setExpressionContext( mArrow->createExpressionContext() );

  if ( d.exec() == QDialog::Accepted )
  {
    mArrow->beginCommand( tr( "Arrow line style changed" ) );
    mArrow->setLineSymbol( newSymbol );
    updateLineSymbolMarker();
    mArrow->endCommand();
    mArrow->update();
  }
  else
  {
    delete newSymbol;
  }
}

void QgsComposerArrowWidget::updateLineSymbolMarker()
{
  if ( !mArrow )
  {
    return;
  }

  QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mArrow->lineSymbol(), mLineStyleButton->iconSize() );
  mLineStyleButton->setIcon( icon );
}
