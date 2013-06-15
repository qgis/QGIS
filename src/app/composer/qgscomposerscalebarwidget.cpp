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
#include "qgscomposition.h"
#include <QColorDialog>
#include <QFontDialog>
#include <QWidget>

QgsComposerScaleBarWidget::QgsComposerScaleBarWidget( QgsComposerScaleBar* scaleBar ): QWidget(), mComposerScaleBar( scaleBar )
{
  setupUi( this );
  connectUpdateSignal();

  //add widget for general composer item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, scaleBar );
  mainLayout->addWidget( itemPropertiesWidget );

  blockMemberSignals( true );

  //style combo box
  mStyleComboBox->insertItem( 0, tr( "Single Box" ) );
  mStyleComboBox->insertItem( 1, tr( "Double Box" ) );
  mStyleComboBox->insertItem( 2, tr( "Line Ticks Middle" ) );
  mStyleComboBox->insertItem( 3, tr( "Line Ticks Down" ) );
  mStyleComboBox->insertItem( 4, tr( "Line Ticks Up" ) );
  mStyleComboBox->insertItem( 5, tr( "Numeric" ) );

  //alignment combo box
  mAlignmentComboBox->insertItem( 0, tr( "Left" ) );
  mAlignmentComboBox->insertItem( 1, tr( "Middle" ) );
  mAlignmentComboBox->insertItem( 2, tr( "Right" ) );

  //units combo box
  mUnitsComboBox->insertItem( 0, tr( "Map units" ), 0 );
  mUnitsComboBox->insertItem( 1, tr( "Meters" ), 1 );
  mUnitsComboBox->insertItem( 2, tr( "Feet" ), 2 );

  blockMemberSignals( false );
  setGuiElements(); //set the GUI elements to the state of scaleBar
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

  if ( saveCurrentComboText.isEmpty() && mComposerScaleBar->composerMap() )
  {
    //combo box was not initialised before
    mMapComboBox->setCurrentIndex( mMapComboBox->findText( tr( "Map %1" ).arg( mComposerScaleBar->composerMap()->id() ) ) );
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
  mComposerScaleBar->beginCommand( tr( "Scalebar map changed" ) );
  disconnectUpdateSignal();
  mComposerScaleBar->setComposerMap( composerMap );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::setGuiElements()
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  blockMemberSignals( true );
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
  toggleStyleSpecificControls( style );

  //alignment
  mAlignmentComboBox->setCurrentIndex(( int )( mComposerScaleBar->alignment() ) );

  //units
  mUnitsComboBox->setCurrentIndex( mUnitsComboBox->findData(( int )mComposerScaleBar->units() ) );

  blockMemberSignals( false );
}

//slots

void QgsComposerScaleBarWidget::on_mLineWidthSpinBox_valueChanged( double d )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar line width" ), QgsComposerMergeCommand::ScaleBarLineWidth );
  disconnectUpdateSignal();
  QPen newPen( mComposerScaleBar->pen().color() );
  newPen.setWidthF( d );
  mComposerScaleBar->setPen( newPen );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::on_mSegmentSizeSpinBox_valueChanged( double d )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar segment size" ), QgsComposerMergeCommand::ScaleBarSegmentSize );
  disconnectUpdateSignal();
  mComposerScaleBar->setNumUnitsPerSegment( d );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::on_mSegmentsLeftSpinBox_valueChanged( int i )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar segments left" ), QgsComposerMergeCommand::ScaleBarSegmentsLeft );
  disconnectUpdateSignal();
  mComposerScaleBar->setNumSegmentsLeft( i );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::on_mNumberOfSegmentsSpinBox_valueChanged( int i )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Number of scalebar segments changed" ), QgsComposerMergeCommand::ScaleBarNSegments );
  disconnectUpdateSignal();
  mComposerScaleBar->setNumSegments( i );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::on_mHeightSpinBox_valueChanged( int i )
{
  if ( !mComposerScaleBar )
  {
    return;
  }
  mComposerScaleBar->beginCommand( tr( "Scalebar height changed" ), QgsComposerMergeCommand::ScaleBarHeight );
  disconnectUpdateSignal();
  mComposerScaleBar->setHeight( i );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::on_mFontButton_clicked()
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  bool dialogAccepted;
  QFont oldFont = mComposerScaleBar->font();
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
  // Native Mac dialog works only for Qt Carbon
  QFont newFont = QFontDialog::getFont( &dialogAccepted, oldFont, 0, QString(), QFontDialog::DontUseNativeDialog );
#else
  QFont newFont = QFontDialog::getFont( &dialogAccepted, oldFont, 0 );
#endif
  if ( dialogAccepted )
  {
    mComposerScaleBar->beginCommand( tr( "Scalebar font changed" ) );
    disconnectUpdateSignal();
    mComposerScaleBar->setFont( newFont );
    connectUpdateSignal();
    mComposerScaleBar->endCommand();
  }
  mComposerScaleBar->update();
}

void QgsComposerScaleBarWidget::on_mFontColorPushButton_clicked()
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  QColor oldColor = mComposerScaleBar->fontColor();
  QColor newColor = QColorDialog::getColor( oldColor, 0 );

  if ( !newColor.isValid() ) //user canceled the dialog
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar font color changed" ) );
  disconnectUpdateSignal();
  mComposerScaleBar->setFontColor( newColor );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
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

  mComposerScaleBar->beginCommand( tr( "Scalebar color changed" ) );
  disconnectUpdateSignal();
  QBrush newBrush( newColor );
  mComposerScaleBar->setBrush( newBrush );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::on_mStrokeColorPushButton_clicked()
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  QColor oldColor = mComposerScaleBar->pen().color();
  QColor newColor = QColorDialog::getColor( oldColor, 0 );

  if ( !newColor.isValid() ) //user canceled the dialog
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar stroke color changed" ) );
  disconnectUpdateSignal();
  QPen newPen = mComposerScaleBar->pen();
  newPen.setColor( newColor );
  mComposerScaleBar->setPen( newPen );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::on_mUnitLabelLineEdit_textChanged( const QString& text )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar unit text" ), QgsComposerMergeCommand::ScaleBarUnitText );
  disconnectUpdateSignal();
  mComposerScaleBar->setUnitLabeling( text );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::on_mMapUnitsPerBarUnitSpinBox_valueChanged( double d )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar map units per segment" ), QgsComposerMergeCommand::ScaleBarMapUnitsSegment );
  disconnectUpdateSignal();
  mComposerScaleBar->setNumMapUnitsPerScaleBarUnit( d );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::on_mStyleComboBox_currentIndexChanged( const QString& text )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar style changed" ) );
  disconnectUpdateSignal();
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

  //disable or enable controls which apply to specific scale bar styles
  toggleStyleSpecificControls( untranslatedStyleName );

  mComposerScaleBar->setStyle( untranslatedStyleName );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::toggleStyleSpecificControls( const QString& style )
{
  if ( style == "Numeric" )
  {
    //Disable controls which don't apply to numeric scale bars
    mGroupBoxUnits->setEnabled( false );
    mGroupBoxUnits->setCollapsed( true );
    mGroupBoxSegments->setEnabled( false );
    mGroupBoxSegments->setCollapsed( true );
    mLabelBarSpaceSpinBox->setEnabled( false );
    mLineWidthSpinBox->setEnabled( false );
    mColorPushButton->setEnabled( false );
    mStrokeColorPushButton->setEnabled( false );
  }
  else
  {
    //Enable all controls
    mGroupBoxUnits->setEnabled( true );
    mGroupBoxSegments->setEnabled( true );
    mLabelBarSpaceSpinBox->setEnabled( true );
    mLineWidthSpinBox->setEnabled( true );
    mColorPushButton->setEnabled( true );
    mStrokeColorPushButton->setEnabled( true );
  }
}

void QgsComposerScaleBarWidget::on_mLabelBarSpaceSpinBox_valueChanged( double d )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar label bar space" ), QgsComposerMergeCommand::ScaleBarLabelBarSize );
  disconnectUpdateSignal();
  mComposerScaleBar->setLabelBarSpace( d );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::on_mBoxSizeSpinBox_valueChanged( double d )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar box content space" ), QgsComposerMergeCommand::ScaleBarBoxContentSpace );
  disconnectUpdateSignal();
  mComposerScaleBar->setBoxContentSpace( d );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::on_mAlignmentComboBox_currentIndexChanged( int index )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar alignment" ) );
  disconnectUpdateSignal();
  mComposerScaleBar->setAlignment(( QgsComposerScaleBar::Alignment ) index );
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::on_mUnitsComboBox_currentIndexChanged( int index )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  QVariant unitData = mUnitsComboBox->itemData( index );
  if ( unitData.type() == QVariant::Invalid )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar unit changed" ) );
  disconnectUpdateSignal();
  mComposerScaleBar->setUnits(( QgsComposerScaleBar::ScaleBarUnits )unitData.toInt() );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
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
  mAlignmentComboBox->blockSignals( block );
  mUnitsComboBox->blockSignals( block );
}

void QgsComposerScaleBarWidget::connectUpdateSignal()
{
  if ( mComposerScaleBar )
  {
    QObject::connect( mComposerScaleBar, SIGNAL( itemChanged() ), this, SLOT( setGuiElements() ) );
  }
}

void QgsComposerScaleBarWidget::disconnectUpdateSignal()
{
  if ( mComposerScaleBar )
  {
    QObject::disconnect( mComposerScaleBar, SIGNAL( itemChanged() ), this, SLOT( setGuiElements() ) );
  }
}
