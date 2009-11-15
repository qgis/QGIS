/***************************************************************************
                            qgscomposerscalebarwidget.cpp
                            -----------------------------
    begin                : 11 June 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerscalebarwidget.h"
#include "qgscomposeritemwidget.h"
#include "qgscomposermap.h"
#include "qgscomposerscalebar.h"
#include <QColorDialog>
#include <QFontDialog>
#include <QWidget>

QgsComposerScaleBarWidget::QgsComposerScaleBarWidget( QgsComposerScaleBar* scaleBar ): QWidget(), mComposerScaleBar( scaleBar )
{
  setupUi( this );

  //add widget for general composer item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, scaleBar );
  gridLayout->addWidget( itemPropertiesWidget );

  blockMemberSignals( true );
  mStyleComboBox->insertItem( 0, tr( "Single Box" ) );
  mStyleComboBox->insertItem( 1, tr( "Double Box" ) );
  mStyleComboBox->insertItem( 2, tr( "Line Ticks Middle" ) );
  mStyleComboBox->insertItem( 3, tr( "Line Ticks Down" ) );
  mStyleComboBox->insertItem( 4, tr( "Line Ticks Up" ) );
  mStyleComboBox->insertItem( 5, tr( "Numeric" ) );
  setGuiElements(); //set the GUI elements to the state of scaleBar
  blockMemberSignals( false );
}

QgsComposerScaleBarWidget::~QgsComposerScaleBarWidget()
{

}

void QgsComposerScaleBarWidget::refreshMapComboBox()
{
  //save the current entry in case it is still present after refresh
  QString saveCurrentComboText = mMapComboBox->currentText();

  mMapComboBox->clear();

  if ( mComposerScaleBar )
  {
    //insert available maps into mMapComboBox
    const QgsComposition* scaleBarComposition = mComposerScaleBar->composition();
    if ( scaleBarComposition )
    {
      QList<const QgsComposerMap*> availableMaps = scaleBarComposition->composerMapItems();
      QList<const QgsComposerMap*>::const_iterator mapItemIt = availableMaps.constBegin();
      for ( ; mapItemIt != availableMaps.constEnd(); ++mapItemIt )
      {
        mMapComboBox->addItem( tr( "Map %1" ).arg(( *mapItemIt )->id() ) );
      }
    }
  }

  if ( mMapComboBox->findText( saveCurrentComboText ) == -1 )
  {
    //the former entry is no longer present. Inform the scalebar about the changed composer map
    on_mMapComboBox_activated( mMapComboBox->currentText() );
  }
  else
  {
    //the former entry is still present. Make it the current entry again
    mMapComboBox->setCurrentIndex( mMapComboBox->findText( saveCurrentComboText ) );
  }
}

void QgsComposerScaleBarWidget::showEvent( QShowEvent * event )
{
  refreshMapComboBox();
  QWidget::showEvent( event );
}

void QgsComposerScaleBarWidget::on_mMapComboBox_activated( const QString& text )
{
  if ( !mComposerScaleBar || text.isEmpty() )
  {
    return;
  }

  const QgsComposition* comp = mComposerScaleBar->composition();
  if ( !comp )
  {
    return;
  }

  //extract id
  int id;
  bool conversionOk;
  QStringList textSplit = text.split( " " );
  if ( textSplit.size() < 1 )
  {
    return;
  }

  QString idString = textSplit.at( textSplit.size() - 1 );
  id = idString.toInt( &conversionOk );

  if ( !conversionOk )
  {
    return;
  }

  //get QgsComposerMap object from composition
  const QgsComposerMap* composerMap = comp->getComposerMapById( id );
  if ( !composerMap )
  {
    return;
  }

  //set it to scale bar
  mComposerScaleBar->setComposerMap( composerMap );

  //update scale bar
  mComposerScaleBar->update();
}

void QgsComposerScaleBarWidget::setGuiElements()
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mNumberOfSegmentsSpinBox->setValue( mComposerScaleBar->numSegments() );
  mSegmentsLeftSpinBox->setValue( mComposerScaleBar->numSegmentsLeft() );
  mSegmentSizeSpinBox->setValue( mComposerScaleBar->numUnitsPerSegment() );
  mLineWidthSpinBox->setValue( mComposerScaleBar->pen().widthF() );
  mHeightSpinBox->setValue( mComposerScaleBar->height() );
  mMapUnitsPerBarUnitSpinBox->setValue( mComposerScaleBar->numMapUnitsPerScaleBarUnit() );
  mLabelBarSpaceSpinBox->setValue( mComposerScaleBar->labelBarSpace() );
  mBoxSizeSpinBox->setValue( mComposerScaleBar->boxContentSpace() );
  mUnitLabelLineEdit->setText( mComposerScaleBar->unitLabeling() );

  //map combo box
  if ( mComposerScaleBar->composerMap() )
  {
    QString mapText = tr( "Map %1" ).arg( mComposerScaleBar->composerMap()->id() );
    int itemId = mMapComboBox->findText( mapText );
    if ( itemId >= 0 )
    {
      mMapComboBox->setCurrentIndex( itemId );
    }
  }

  //style...
  QString style = mComposerScaleBar->style();
  mStyleComboBox->setCurrentIndex( mStyleComboBox->findText( tr( style.toLocal8Bit().data() ) ) );
}

//slots

void QgsComposerScaleBarWidget::on_mLineWidthSpinBox_valueChanged( double d )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  QPen newPen( QColor( 0, 0, 0 ) );
  newPen.setWidthF( d );
  mComposerScaleBar->setPen( newPen );
  mComposerScaleBar->update();
}

void QgsComposerScaleBarWidget::on_mSegmentSizeSpinBox_valueChanged( double d )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->setNumUnitsPerSegment( d );
  mComposerScaleBar->update();
}

void QgsComposerScaleBarWidget::on_mSegmentsLeftSpinBox_valueChanged( int i )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->setNumSegmentsLeft( i );
  mComposerScaleBar->update();
}

void QgsComposerScaleBarWidget::on_mNumberOfSegmentsSpinBox_valueChanged( int i )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->setNumSegments( i );
  mComposerScaleBar->update();
}

void QgsComposerScaleBarWidget::on_mHeightSpinBox_valueChanged( int i )
{
  if ( !mComposerScaleBar )
  {
    return;
  }
  mComposerScaleBar->setHeight( i );
  mComposerScaleBar->update();
}

void QgsComposerScaleBarWidget::on_mFontButton_clicked()
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  bool dialogAccepted;
  QFont oldFont = mComposerScaleBar->font();
  QFont newFont = QFontDialog::getFont( &dialogAccepted, oldFont, 0 );
  if ( dialogAccepted )
  {
    mComposerScaleBar->setFont( newFont );
  }
  mComposerScaleBar->update();
}

void QgsComposerScaleBarWidget::on_mColorPushButton_clicked()
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  QColor oldColor = mComposerScaleBar->brush().color();
  QColor newColor = QColorDialog::getColor( oldColor, 0 );

  if ( !newColor.isValid() ) //user canceled the dialog
  {
    return;
  }

  QBrush newBrush( newColor );
  mComposerScaleBar->setBrush( newBrush );
  mComposerScaleBar->update();
}

void QgsComposerScaleBarWidget::on_mUnitLabelLineEdit_textChanged( const QString& text )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->setUnitLabeling( text );
  mComposerScaleBar->update();
}

void QgsComposerScaleBarWidget::on_mMapUnitsPerBarUnitSpinBox_valueChanged( double d )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->setNumMapUnitsPerScaleBarUnit( d );
  mComposerScaleBar->update();
}

void QgsComposerScaleBarWidget::on_mStyleComboBox_currentIndexChanged( const QString& text )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  QString untranslatedStyleName;
  if ( text == tr( "Single Box" ) )
  {
    untranslatedStyleName = "Single Box";
  }
  else if ( text == tr( "Double Box" ) )
  {
    untranslatedStyleName = "Double Box";
  }
  else if ( text == tr( "Line Ticks Middle" ) )
  {
    untranslatedStyleName = "Line Ticks Middle";
  }
  else if ( text == tr( "Line Ticks Middle" ) )
  {
    untranslatedStyleName = "Line Ticks Middle";
  }
  else if ( text == tr( "Line Ticks Down" ) )
  {
    untranslatedStyleName = "Line Ticks Down";
  }
  else if ( text == tr( "Line Ticks Up" ) )
  {
    untranslatedStyleName = "Line Ticks Up";
  }
  else if ( text == tr( "Numeric" ) )
  {
    untranslatedStyleName = "Numeric";
  }
  mComposerScaleBar->setStyle( untranslatedStyleName );
  mComposerScaleBar->update();
}

void QgsComposerScaleBarWidget::on_mLabelBarSpaceSpinBox_valueChanged( double d )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->setLabelBarSpace( d );
  mComposerScaleBar->update();
}

void QgsComposerScaleBarWidget::on_mBoxSizeSpinBox_valueChanged( double d )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->setBoxContentSpace( d );
  mComposerScaleBar->update();
}

void QgsComposerScaleBarWidget::blockMemberSignals( bool block )
{
  mSegmentSizeSpinBox->blockSignals( block );
  mNumberOfSegmentsSpinBox->blockSignals( block );
  mSegmentsLeftSpinBox->blockSignals( block );
  mStyleComboBox->blockSignals( block );
  mUnitLabelLineEdit->blockSignals( block );
  mMapUnitsPerBarUnitSpinBox->blockSignals( block );
  mMapComboBox->blockSignals( block );
  mHeightSpinBox->blockSignals( block );
  mLineWidthSpinBox->blockSignals( block );
  mLabelBarSpaceSpinBox->blockSignals( block );
  mBoxSizeSpinBox->blockSignals( block );
}
