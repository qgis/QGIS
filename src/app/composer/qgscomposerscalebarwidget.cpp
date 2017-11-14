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
#include "qgsguiutils.h"
#include <QColorDialog>
#include <QFontDialog>
#include <QWidget>

QgsComposerScaleBarWidget::QgsComposerScaleBarWidget( QgsComposerScaleBar *scaleBar ): QgsComposerItemBaseWidget( nullptr, scaleBar ), mComposerScaleBar( scaleBar )
{
  setupUi( this );
  connect( mHeightSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerScaleBarWidget::mHeightSpinBox_valueChanged );
  connect( mLineWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerScaleBarWidget::mLineWidthSpinBox_valueChanged );
  connect( mSegmentSizeSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerScaleBarWidget::mSegmentSizeSpinBox_valueChanged );
  connect( mSegmentsLeftSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsComposerScaleBarWidget::mSegmentsLeftSpinBox_valueChanged );
  connect( mNumberOfSegmentsSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsComposerScaleBarWidget::mNumberOfSegmentsSpinBox_valueChanged );
  connect( mUnitLabelLineEdit, &QLineEdit::textChanged, this, &QgsComposerScaleBarWidget::mUnitLabelLineEdit_textChanged );
  connect( mMapUnitsPerBarUnitSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerScaleBarWidget::mMapUnitsPerBarUnitSpinBox_valueChanged );
  connect( mFontColorButton, &QgsColorButton::colorChanged, this, &QgsComposerScaleBarWidget::mFontColorButton_colorChanged );
  connect( mFillColorButton, &QgsColorButton::colorChanged, this, &QgsComposerScaleBarWidget::mFillColorButton_colorChanged );
  connect( mFillColor2Button, &QgsColorButton::colorChanged, this, &QgsComposerScaleBarWidget::mFillColor2Button_colorChanged );
  connect( mStrokeColorButton, &QgsColorButton::colorChanged, this, &QgsComposerScaleBarWidget::mStrokeColorButton_colorChanged );
  connect( mStyleComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsComposerScaleBarWidget::mStyleComboBox_currentIndexChanged );
  connect( mLabelBarSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerScaleBarWidget::mLabelBarSpaceSpinBox_valueChanged );
  connect( mBoxSizeSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerScaleBarWidget::mBoxSizeSpinBox_valueChanged );
  connect( mAlignmentComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerScaleBarWidget::mAlignmentComboBox_currentIndexChanged );
  connect( mUnitsComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerScaleBarWidget::mUnitsComboBox_currentIndexChanged );
  connect( mLineJoinStyleCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerScaleBarWidget::mLineJoinStyleCombo_currentIndexChanged );
  connect( mLineCapStyleCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerScaleBarWidget::mLineCapStyleCombo_currentIndexChanged );
  connect( mMinWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerScaleBarWidget::mMinWidthSpinBox_valueChanged );
  connect( mMaxWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerScaleBarWidget::mMaxWidthSpinBox_valueChanged );
  setPanelTitle( tr( "Scalebar properties" ) );

  mFontButton->setMode( QgsFontButton::ModeQFont );

  connectUpdateSignal();

  //add widget for general composer item properties
  QgsComposerItemWidget *itemPropertiesWidget = new QgsComposerItemWidget( this, scaleBar );
  mainLayout->addWidget( itemPropertiesWidget );

  mSegmentSizeRadioGroup.addButton( mFixedSizeRadio );
  mSegmentSizeRadioGroup.addButton( mFitWidthRadio );
  connect( &mSegmentSizeRadioGroup, static_cast < void ( QButtonGroup::* )( QAbstractButton * ) > ( &QButtonGroup::buttonClicked ), this, &QgsComposerScaleBarWidget::segmentSizeRadioChanged );

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
  mUnitsComboBox->insertItem( 0, tr( "Map units" ), QgsUnitTypes::DistanceUnknownUnit );
  mUnitsComboBox->insertItem( 1, tr( "Meters" ), QgsUnitTypes::DistanceMeters );
  mUnitsComboBox->insertItem( 2, tr( "Feet" ), QgsUnitTypes::DistanceFeet );
  mUnitsComboBox->insertItem( 3, tr( "Nautical Miles" ), QgsUnitTypes::DistanceNauticalMiles );

  mFillColorButton->setColorDialogTitle( tr( "Select Fill Color" ) );
  mFillColorButton->setAllowOpacity( true );
  mFillColorButton->setContext( QStringLiteral( "composer" ) );
  mFillColorButton->setNoColorString( tr( "Transparent Fill" ) );
  mFillColorButton->setShowNoColor( true );

  mFillColor2Button->setColorDialogTitle( tr( "Select Alternate Fill Color" ) );
  mFillColor2Button->setAllowOpacity( true );
  mFillColor2Button->setContext( QStringLiteral( "composer" ) );
  mFillColor2Button->setNoColorString( tr( "Transparent fill" ) );
  mFillColor2Button->setShowNoColor( true );

  mFontColorButton->setColorDialogTitle( tr( "Select Font Color" ) );
  mFontColorButton->setAllowOpacity( true );
  mFontColorButton->setContext( QStringLiteral( "composer" ) );

  mStrokeColorButton->setColorDialogTitle( tr( "Select Line Color" ) );
  mStrokeColorButton->setAllowOpacity( true );
  mStrokeColorButton->setContext( QStringLiteral( "composer" ) );
  mStrokeColorButton->setNoColorString( tr( "Transparent line" ) );
  mStrokeColorButton->setShowNoColor( true );

  QgsComposition *scaleBarComposition = mComposerScaleBar->composition();
  if ( scaleBarComposition )
  {
    mMapItemComboBox->setComposition( scaleBarComposition );
    mMapItemComboBox->setItemType( QgsComposerItem::ComposerMap );
  }

  connect( mMapItemComboBox, &QgsComposerItemComboBox::itemChanged, this, &QgsComposerScaleBarWidget::composerMapChanged );

  registerDataDefinedButton( mFillColorDDBtn, QgsComposerObject::ScalebarFillColor );
  registerDataDefinedButton( mFillColor2DDBtn, QgsComposerObject::ScalebarFillColor2 );
  registerDataDefinedButton( mLineColorDDBtn, QgsComposerObject::ScalebarLineColor );
  registerDataDefinedButton( mLineWidthDDBtn, QgsComposerObject::ScalebarLineWidth );

  blockMemberSignals( false );
  setGuiElements(); //set the GUI elements to the state of scaleBar

  connect( mFontButton, &QgsFontButton::changed, this, &QgsComposerScaleBarWidget::fontChanged );
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
  mLineWidthSpinBox->setValue( mComposerScaleBar->lineWidth() );
  mHeightSpinBox->setValue( mComposerScaleBar->height() );
  mMapUnitsPerBarUnitSpinBox->setValue( mComposerScaleBar->numMapUnitsPerScaleBarUnit() );
  mLabelBarSpaceSpinBox->setValue( mComposerScaleBar->labelBarSpace() );
  mBoxSizeSpinBox->setValue( mComposerScaleBar->boxContentSpace() );
  mUnitLabelLineEdit->setText( mComposerScaleBar->unitLabeling() );
  mLineJoinStyleCombo->setPenJoinStyle( mComposerScaleBar->lineJoinStyle() );
  mLineCapStyleCombo->setPenCapStyle( mComposerScaleBar->lineCapStyle() );
  mFontColorButton->setColor( mComposerScaleBar->fontColor() );
  mFillColorButton->setColor( mComposerScaleBar->fillColor() );
  mFillColor2Button->setColor( mComposerScaleBar->fillColor2() );
  mStrokeColorButton->setColor( mComposerScaleBar->lineColor() );
  mFontButton->setCurrentFont( mComposerScaleBar->font() );

  //map combo box
  mMapItemComboBox->setItem( mComposerScaleBar->composerMap() );

  //style...
  QString style = mComposerScaleBar->style();
  mStyleComboBox->setCurrentIndex( mStyleComboBox->findText( tr( style.toLocal8Bit().data() ) ) );
  toggleStyleSpecificControls( style );

  //alignment
  mAlignmentComboBox->setCurrentIndex( ( int )( mComposerScaleBar->alignment() ) );

  //units
  mUnitsComboBox->setCurrentIndex( mUnitsComboBox->findData( ( int )mComposerScaleBar->units() ) );

  if ( mComposerScaleBar->segmentSizeMode() == QgsScaleBarSettings::SegmentSizeFixed )
  {
    mFixedSizeRadio->setChecked( true );
    mSegmentSizeSpinBox->setEnabled( true );
    mMinWidthSpinBox->setEnabled( false );
    mMaxWidthSpinBox->setEnabled( false );
  }
  else /*if(mComposerScaleBar->segmentSizeMode() == QgsComposerScaleBar::SegmentSizeFitWidth)*/
  {
    mFitWidthRadio->setChecked( true );
    mSegmentSizeSpinBox->setEnabled( false );
    mMinWidthSpinBox->setEnabled( true );
    mMaxWidthSpinBox->setEnabled( true );
  }
  mMinWidthSpinBox->setValue( mComposerScaleBar->minBarWidth() );
  mMaxWidthSpinBox->setValue( mComposerScaleBar->maxBarWidth() );
  updateDataDefinedButton( mFillColorDDBtn );
  updateDataDefinedButton( mFillColor2DDBtn );
  updateDataDefinedButton( mLineColorDDBtn );
  updateDataDefinedButton( mLineWidthDDBtn );
  blockMemberSignals( false );
}

//slots

void QgsComposerScaleBarWidget::mLineWidthSpinBox_valueChanged( double d )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar line width" ), QgsComposerMergeCommand::ScaleBarLineWidth );
  disconnectUpdateSignal();
  mComposerScaleBar->setLineWidth( d );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::mSegmentSizeSpinBox_valueChanged( double d )
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

void QgsComposerScaleBarWidget::mSegmentsLeftSpinBox_valueChanged( int i )
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

void QgsComposerScaleBarWidget::mNumberOfSegmentsSpinBox_valueChanged( int i )
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

void QgsComposerScaleBarWidget::mHeightSpinBox_valueChanged( double d )
{
  if ( !mComposerScaleBar )
  {
    return;
  }
  mComposerScaleBar->beginCommand( tr( "Scalebar height changed" ), QgsComposerMergeCommand::ScaleBarHeight );
  disconnectUpdateSignal();
  mComposerScaleBar->setHeight( d );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::fontChanged()
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar font changed" ) );
  disconnectUpdateSignal();
  mComposerScaleBar->setFont( mFontButton->currentFont() );
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
  mComposerScaleBar->update();
}

void QgsComposerScaleBarWidget::mFontColorButton_colorChanged( const QColor &newColor )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar font color changed" ), QgsComposerMergeCommand::ScaleBarFontColor );
  disconnectUpdateSignal();
  mComposerScaleBar->setFontColor( newColor );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::mFillColorButton_colorChanged( const QColor &newColor )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar color changed" ), QgsComposerMergeCommand::ScaleBarFillColor );
  disconnectUpdateSignal();
  mComposerScaleBar->setFillColor( newColor );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::mFillColor2Button_colorChanged( const QColor &newColor )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar secondary color changed" ), QgsComposerMergeCommand::ScaleBarFill2Color );
  disconnectUpdateSignal();
  mComposerScaleBar->setFillColor2( newColor );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::mStrokeColorButton_colorChanged( const QColor &newColor )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar line color changed" ), QgsComposerMergeCommand::ScaleBarStrokeColor );
  disconnectUpdateSignal();
  mComposerScaleBar->setLineColor( newColor );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::mUnitLabelLineEdit_textChanged( const QString &text )
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

void QgsComposerScaleBarWidget::mMapUnitsPerBarUnitSpinBox_valueChanged( double d )
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

void QgsComposerScaleBarWidget::mStyleComboBox_currentIndexChanged( const QString &text )
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
    untranslatedStyleName = QStringLiteral( "Single Box" );
  }
  else if ( text == tr( "Double Box" ) )
  {
    untranslatedStyleName = QStringLiteral( "Double Box" );
  }
  else if ( text == tr( "Line Ticks Middle" ) )
  {
    untranslatedStyleName = QStringLiteral( "Line Ticks Middle" );
  }
  else if ( text == tr( "Line Ticks Middle" ) )
  {
    untranslatedStyleName = QStringLiteral( "Line Ticks Middle" );
  }
  else if ( text == tr( "Line Ticks Down" ) )
  {
    untranslatedStyleName = QStringLiteral( "Line Ticks Down" );
  }
  else if ( text == tr( "Line Ticks Up" ) )
  {
    untranslatedStyleName = QStringLiteral( "Line Ticks Up" );
  }
  else if ( text == tr( "Numeric" ) )
  {
    untranslatedStyleName = QStringLiteral( "Numeric" );
  }

  //disable or enable controls which apply to specific scale bar styles
  toggleStyleSpecificControls( untranslatedStyleName );

  mComposerScaleBar->setStyle( untranslatedStyleName );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::toggleStyleSpecificControls( const QString &style )
{
  if ( style == QLatin1String( "Numeric" ) )
  {
    //Disable controls which don't apply to numeric scale bars
    mGroupBoxUnits->setEnabled( false );
    mGroupBoxUnits->setCollapsed( true );
    mGroupBoxSegments->setEnabled( false );
    mGroupBoxSegments->setCollapsed( true );
    mLabelBarSpaceSpinBox->setEnabled( false );
    mLineWidthSpinBox->setEnabled( false );
    mFillColorButton->setEnabled( false );
    mFillColor2Button->setEnabled( false );
    mStrokeColorButton->setEnabled( false );
    mLineJoinStyleCombo->setEnabled( false );
    mLineCapStyleCombo->setEnabled( false );
  }
  else
  {
    //Enable controls
    mGroupBoxUnits->setEnabled( true );
    mGroupBoxSegments->setEnabled( true );
    mLabelBarSpaceSpinBox->setEnabled( true );
    mLineWidthSpinBox->setEnabled( true );
    mFillColorButton->setEnabled( true );
    mFillColor2Button->setEnabled( true );
    mStrokeColorButton->setEnabled( true );
    if ( style == QLatin1String( "Single Box" ) || style == QLatin1String( "Double Box" ) )
    {
      mLineJoinStyleCombo->setEnabled( true );
      mLineCapStyleCombo->setEnabled( false );
    }
    else
    {
      mLineJoinStyleCombo->setEnabled( false );
      mLineCapStyleCombo->setEnabled( true );
    }

  }
}

void QgsComposerScaleBarWidget::mLabelBarSpaceSpinBox_valueChanged( double d )
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

void QgsComposerScaleBarWidget::mBoxSizeSpinBox_valueChanged( double d )
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

void QgsComposerScaleBarWidget::mAlignmentComboBox_currentIndexChanged( int index )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar alignment" ) );
  disconnectUpdateSignal();
  mComposerScaleBar->setAlignment( ( QgsScaleBarSettings::Alignment ) index );
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::mUnitsComboBox_currentIndexChanged( int index )
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

  disconnectUpdateSignal();
  mComposerScaleBar->setUnits( ( QgsUnitTypes::DistanceUnit )unitData.toInt() );
  switch ( mUnitsComboBox->currentIndex() )
  {
    case 0:
    {
      mComposerScaleBar->beginCommand( tr( "Scalebar changed to map units" ) );
      mComposerScaleBar->applyDefaultSize( QgsUnitTypes::DistanceUnknownUnit );
      break;
    }
    case 2:
    {
      mComposerScaleBar->beginCommand( tr( "Scalebar changed to feet" ) );
      mComposerScaleBar->applyDefaultSize( QgsUnitTypes::DistanceFeet );
      break;
    }
    case 3:
    {
      mComposerScaleBar->beginCommand( tr( "Scalebar changed to nautical miles" ) );
      mComposerScaleBar->applyDefaultSize( QgsUnitTypes::DistanceNauticalMiles );
      break;
    }
    case 1:
    default:
    {
      mComposerScaleBar->beginCommand( tr( "Scalebar changed to meters" ) );
      mComposerScaleBar->applyDefaultSize( QgsUnitTypes::DistanceMeters );
      break;
    }
  }

  mComposerScaleBar->update();

  mUnitLabelLineEdit->setText( mComposerScaleBar->unitLabeling() );
  mSegmentSizeSpinBox->setValue( mComposerScaleBar->numUnitsPerSegment() );
  mMapUnitsPerBarUnitSpinBox->setValue( mComposerScaleBar->numMapUnitsPerScaleBarUnit() );

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
  mHeightSpinBox->blockSignals( block );
  mLineWidthSpinBox->blockSignals( block );
  mLabelBarSpaceSpinBox->blockSignals( block );
  mBoxSizeSpinBox->blockSignals( block );
  mAlignmentComboBox->blockSignals( block );
  mUnitsComboBox->blockSignals( block );
  mLineJoinStyleCombo->blockSignals( block );
  mLineCapStyleCombo->blockSignals( block );
  mFontColorButton->blockSignals( block );
  mFillColorButton->blockSignals( block );
  mFillColor2Button->blockSignals( block );
  mStrokeColorButton->blockSignals( block );
  mSegmentSizeRadioGroup.blockSignals( block );
  mMapItemComboBox->blockSignals( block );
  mFontButton->blockSignals( block );
  mMinWidthSpinBox->blockSignals( block );
  mMaxWidthSpinBox->blockSignals( block );
}

void QgsComposerScaleBarWidget::connectUpdateSignal()
{
  if ( mComposerScaleBar )
  {
    connect( mComposerScaleBar, &QgsComposerObject::itemChanged, this, &QgsComposerScaleBarWidget::setGuiElements );
  }
}

void QgsComposerScaleBarWidget::disconnectUpdateSignal()
{
  if ( mComposerScaleBar )
  {
    disconnect( mComposerScaleBar, &QgsComposerObject::itemChanged, this, &QgsComposerScaleBarWidget::setGuiElements );
  }
}

void QgsComposerScaleBarWidget::mLineJoinStyleCombo_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar line join style" ) );
  mComposerScaleBar->setLineJoinStyle( mLineJoinStyleCombo->penJoinStyle() );
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::mLineCapStyleCombo_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar line cap style" ) );
  mComposerScaleBar->setLineCapStyle( mLineCapStyleCombo->penCapStyle() );
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::segmentSizeRadioChanged( QAbstractButton *radio )
{
  bool fixedSizeMode = radio == mFixedSizeRadio;
  mMinWidthSpinBox->setEnabled( !fixedSizeMode );
  mMaxWidthSpinBox->setEnabled( !fixedSizeMode );
  mSegmentSizeSpinBox->setEnabled( fixedSizeMode );

  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar segment size mode" ), QgsComposerMergeCommand::ScaleBarSegmentSize );
  disconnectUpdateSignal();
  if ( mFixedSizeRadio->isChecked() )
  {
    mComposerScaleBar->setSegmentSizeMode( QgsScaleBarSettings::SegmentSizeFixed );
    mComposerScaleBar->setNumUnitsPerSegment( mSegmentSizeSpinBox->value() );
  }
  else /*if(mFitWidthRadio->isChecked())*/
  {
    mComposerScaleBar->setSegmentSizeMode( QgsScaleBarSettings::SegmentSizeFitWidth );
  }
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::composerMapChanged( QgsComposerItem *item )
{
  QgsComposerMap *composerMap = dynamic_cast< QgsComposerMap * >( item );
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

void QgsComposerScaleBarWidget::mMinWidthSpinBox_valueChanged( double )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar segment size mode" ), QgsComposerMergeCommand::ScaleBarSegmentSize );
  disconnectUpdateSignal();
  mComposerScaleBar->setMinBarWidth( mMinWidthSpinBox->value() );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}

void QgsComposerScaleBarWidget::mMaxWidthSpinBox_valueChanged( double )
{
  if ( !mComposerScaleBar )
  {
    return;
  }

  mComposerScaleBar->beginCommand( tr( "Scalebar segment size mode" ), QgsComposerMergeCommand::ScaleBarSegmentSize );
  disconnectUpdateSignal();
  mComposerScaleBar->setMaxBarWidth( mMaxWidthSpinBox->value() );
  mComposerScaleBar->update();
  connectUpdateSignal();
  mComposerScaleBar->endCommand();
}
