/***************************************************************************
                            qgslayoutscalebarwidget.cpp
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

#include "qgslayoutscalebarwidget.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitemscalebar.h"
#include "qgslayout.h"
#include "qgsguiutils.h"

#include <QColorDialog>
#include <QFontDialog>
#include <QWidget>

QgsLayoutScaleBarWidget::QgsLayoutScaleBarWidget( QgsLayoutItemScaleBar *scaleBar )
  : QgsLayoutItemBaseWidget( nullptr, scaleBar )
  , mScalebar( scaleBar )
{
  setupUi( this );
  connect( mHeightSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutScaleBarWidget::mHeightSpinBox_valueChanged );
  connect( mLineWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutScaleBarWidget::mLineWidthSpinBox_valueChanged );
  connect( mSegmentSizeSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutScaleBarWidget::mSegmentSizeSpinBox_valueChanged );
  connect( mSegmentsLeftSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLayoutScaleBarWidget::mSegmentsLeftSpinBox_valueChanged );
  connect( mNumberOfSegmentsSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLayoutScaleBarWidget::mNumberOfSegmentsSpinBox_valueChanged );
  connect( mUnitLabelLineEdit, &QLineEdit::textChanged, this, &QgsLayoutScaleBarWidget::mUnitLabelLineEdit_textChanged );
  connect( mMapUnitsPerBarUnitSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutScaleBarWidget::mMapUnitsPerBarUnitSpinBox_valueChanged );
  connect( mFillColorButton, &QgsColorButton::colorChanged, this, &QgsLayoutScaleBarWidget::mFillColorButton_colorChanged );
  connect( mFillColor2Button, &QgsColorButton::colorChanged, this, &QgsLayoutScaleBarWidget::mFillColor2Button_colorChanged );
  connect( mStrokeColorButton, &QgsColorButton::colorChanged, this, &QgsLayoutScaleBarWidget::mStrokeColorButton_colorChanged );
  connect( mStyleComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutScaleBarWidget::mStyleComboBox_currentIndexChanged );
  connect( mLabelBarSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutScaleBarWidget::mLabelBarSpaceSpinBox_valueChanged );
  connect( mBoxSizeSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutScaleBarWidget::mBoxSizeSpinBox_valueChanged );
  connect( mAlignmentComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutScaleBarWidget::mAlignmentComboBox_currentIndexChanged );
  connect( mUnitsComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutScaleBarWidget::mUnitsComboBox_currentIndexChanged );
  connect( mLineJoinStyleCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutScaleBarWidget::mLineJoinStyleCombo_currentIndexChanged );
  connect( mLineCapStyleCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutScaleBarWidget::mLineCapStyleCombo_currentIndexChanged );
  connect( mMinWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutScaleBarWidget::mMinWidthSpinBox_valueChanged );
  connect( mMaxWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutScaleBarWidget::mMaxWidthSpinBox_valueChanged );
  setPanelTitle( tr( "Scalebar Properties" ) );

  mFontButton->setMode( QgsFontButton::ModeQFont );

  connectUpdateSignal();

  //add widget for general composer item properties
  mItemPropertiesWidget = new QgsLayoutItemPropertiesWidget( this, scaleBar );
  mainLayout->addWidget( mItemPropertiesWidget );

  mSegmentSizeRadioGroup.addButton( mFixedSizeRadio );
  mSegmentSizeRadioGroup.addButton( mFitWidthRadio );
  connect( &mSegmentSizeRadioGroup, static_cast < void ( QButtonGroup::* )( QAbstractButton * ) > ( &QButtonGroup::buttonClicked ), this, &QgsLayoutScaleBarWidget::segmentSizeRadioChanged );

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
  mUnitsComboBox->addItem( tr( "Map units" ), QgsUnitTypes::DistanceUnknownUnit );
  mUnitsComboBox->addItem( tr( "Meters" ), QgsUnitTypes::DistanceMeters );
  mUnitsComboBox->addItem( tr( "Kilometers" ), QgsUnitTypes::DistanceKilometers );
  mUnitsComboBox->addItem( tr( "Feet" ), QgsUnitTypes::DistanceFeet );
  mUnitsComboBox->addItem( tr( "Yards" ), QgsUnitTypes::DistanceYards );
  mUnitsComboBox->addItem( tr( "Miles" ), QgsUnitTypes::DistanceMiles );
  mUnitsComboBox->addItem( tr( "Nautical Miles" ), QgsUnitTypes::DistanceNauticalMiles );
  mUnitsComboBox->addItem( tr( "Centimeters" ), QgsUnitTypes::DistanceCentimeters );
  mUnitsComboBox->addItem( tr( "Millimeters" ), QgsUnitTypes::DistanceMillimeters );

  mFillColorButton->setColorDialogTitle( tr( "Select Fill Color" ) );
  mFillColorButton->setAllowOpacity( true );
  mFillColorButton->setContext( QStringLiteral( "composer" ) );
  mFillColorButton->setNoColorString( tr( "Transparent Fill" ) );
  mFillColorButton->setShowNoColor( true );

  mFillColor2Button->setColorDialogTitle( tr( "Select Alternate Fill Color" ) );
  mFillColor2Button->setAllowOpacity( true );
  mFillColor2Button->setContext( QStringLiteral( "composer" ) );
  mFillColor2Button->setNoColorString( tr( "Transparent Fill" ) );
  mFillColor2Button->setShowNoColor( true );

  mFontButton->setDialogTitle( tr( "Scalebar Font" ) );
  mFontButton->setMode( QgsFontButton::ModeTextRenderer );

  mStrokeColorButton->setColorDialogTitle( tr( "Select Line Color" ) );
  mStrokeColorButton->setAllowOpacity( true );
  mStrokeColorButton->setContext( QStringLiteral( "composer" ) );
  mStrokeColorButton->setNoColorString( tr( "Transparent Line" ) );
  mStrokeColorButton->setShowNoColor( true );

  mFillColorDDBtn->setFlags( QgsPropertyOverrideButton::FlagDisableCheckedWidgetOnlyWhenProjectColorSet );
  mFillColorDDBtn->registerEnabledWidget( mFillColorButton, false );
  mFillColor2DDBtn->setFlags( QgsPropertyOverrideButton::FlagDisableCheckedWidgetOnlyWhenProjectColorSet );
  mFillColor2DDBtn->registerEnabledWidget( mFillColor2Button, false );
  mLineColorDDBtn->setFlags( QgsPropertyOverrideButton::FlagDisableCheckedWidgetOnlyWhenProjectColorSet );
  mLineColorDDBtn->registerEnabledWidget( mStrokeColorButton, false );

  if ( mScalebar )
  {
    mFillColorDDBtn->registerExpressionContextGenerator( mScalebar );
    mFillColor2DDBtn->registerExpressionContextGenerator( mScalebar );
    mLineColorDDBtn->registerExpressionContextGenerator( mScalebar );
    mLineWidthDDBtn->registerExpressionContextGenerator( mScalebar );
    QgsLayout *scaleBarLayout = mScalebar->layout();
    if ( scaleBarLayout )
    {
      mMapItemComboBox->setCurrentLayout( scaleBarLayout );
      mMapItemComboBox->setItemType( QgsLayoutItemRegistry::LayoutMap );
    }
  }

  connect( mMapItemComboBox, &QgsLayoutItemComboBox::itemChanged, this, &QgsLayoutScaleBarWidget::mapChanged );

  registerDataDefinedButton( mFillColorDDBtn, QgsLayoutObject::ScalebarFillColor );
  registerDataDefinedButton( mFillColor2DDBtn, QgsLayoutObject::ScalebarFillColor2 );
  registerDataDefinedButton( mLineColorDDBtn, QgsLayoutObject::ScalebarLineColor );
  registerDataDefinedButton( mLineWidthDDBtn, QgsLayoutObject::ScalebarLineWidth );

  blockMemberSignals( false );
  setGuiElements(); //set the GUI elements to the state of scaleBar

  connect( mFontButton, &QgsFontButton::changed, this, &QgsLayoutScaleBarWidget::textFormatChanged );
}

bool QgsLayoutScaleBarWidget::setNewItem( QgsLayoutItem *item )
{
  if ( item->type() != QgsLayoutItemRegistry::LayoutScaleBar )
    return false;

  disconnectUpdateSignal();

  mScalebar = qobject_cast< QgsLayoutItemScaleBar * >( item );
  mItemPropertiesWidget->setItem( mScalebar );

  if ( mScalebar )
  {
    connectUpdateSignal();
    mFillColorDDBtn->registerExpressionContextGenerator( mScalebar );
    mFillColor2DDBtn->registerExpressionContextGenerator( mScalebar );
    mLineColorDDBtn->registerExpressionContextGenerator( mScalebar );
    mLineWidthDDBtn->registerExpressionContextGenerator( mScalebar );
  }

  setGuiElements();

  return true;
}

void QgsLayoutScaleBarWidget::setGuiElements()
{
  if ( !mScalebar )
  {
    return;
  }

  blockMemberSignals( true );
  mNumberOfSegmentsSpinBox->setValue( mScalebar->numberOfSegments() );
  mSegmentsLeftSpinBox->setValue( mScalebar->numberOfSegmentsLeft() );
  mSegmentSizeSpinBox->setValue( mScalebar->unitsPerSegment() );
  mLineWidthSpinBox->setValue( mScalebar->lineWidth() );
  mHeightSpinBox->setValue( mScalebar->height() );
  mMapUnitsPerBarUnitSpinBox->setValue( mScalebar->mapUnitsPerScaleBarUnit() );
  mLabelBarSpaceSpinBox->setValue( mScalebar->labelBarSpace() );
  mBoxSizeSpinBox->setValue( mScalebar->boxContentSpace() );
  mUnitLabelLineEdit->setText( mScalebar->unitLabel() );
  mLineJoinStyleCombo->setPenJoinStyle( mScalebar->lineJoinStyle() );
  mLineCapStyleCombo->setPenCapStyle( mScalebar->lineCapStyle() );
  mFillColorButton->setColor( mScalebar->fillColor() );
  mFillColor2Button->setColor( mScalebar->fillColor2() );
  mStrokeColorButton->setColor( mScalebar->lineColor() );
  mFontButton->setTextFormat( mScalebar->textFormat() );

  //map combo box
  mMapItemComboBox->setItem( mScalebar->linkedMap() );

  //style...
  QString style = mScalebar->style();
  mStyleComboBox->setCurrentIndex( mStyleComboBox->findText( tr( style.toLocal8Bit().data() ) ) );
  toggleStyleSpecificControls( style );

  //alignment
  mAlignmentComboBox->setCurrentIndex( static_cast< int >( mScalebar->alignment() ) );

  //units
  mUnitsComboBox->setCurrentIndex( mUnitsComboBox->findData( static_cast< int >( mScalebar->units() ) ) );

  if ( mScalebar->segmentSizeMode() == QgsScaleBarSettings::SegmentSizeFixed )
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
  mMinWidthSpinBox->setValue( mScalebar->minimumBarWidth() );
  mMaxWidthSpinBox->setValue( mScalebar->maximumBarWidth() );
  updateDataDefinedButton( mFillColorDDBtn );
  updateDataDefinedButton( mFillColor2DDBtn );
  updateDataDefinedButton( mLineColorDDBtn );
  updateDataDefinedButton( mLineWidthDDBtn );
  blockMemberSignals( false );
}

//slots

void QgsLayoutScaleBarWidget::mLineWidthSpinBox_valueChanged( double d )
{
  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Line Width" ), QgsLayoutItem::UndoScaleBarLineWidth );
  disconnectUpdateSignal();
  mScalebar->setLineWidth( d );
  mScalebar->update();
  connectUpdateSignal();
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::mSegmentSizeSpinBox_valueChanged( double d )
{
  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Segment Size" ), QgsLayoutItem::UndoScaleBarSegmentSize );
  disconnectUpdateSignal();
  mScalebar->setUnitsPerSegment( d );
  mScalebar->update();
  connectUpdateSignal();
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::mSegmentsLeftSpinBox_valueChanged( int i )
{
  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Segments" ), QgsLayoutItem::UndoScaleBarSegmentsLeft );
  disconnectUpdateSignal();
  mScalebar->setNumberOfSegmentsLeft( i );
  mScalebar->update();
  connectUpdateSignal();
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::mNumberOfSegmentsSpinBox_valueChanged( int i )
{
  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Segments" ), QgsLayoutItem::UndoScaleBarSegments );
  disconnectUpdateSignal();
  mScalebar->setNumberOfSegments( i );
  mScalebar->update();
  connectUpdateSignal();
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::mHeightSpinBox_valueChanged( double d )
{
  if ( !mScalebar )
  {
    return;
  }
  mScalebar->beginCommand( tr( "Set Scalebar Height" ), QgsLayoutItem::UndoScaleBarHeight );
  disconnectUpdateSignal();
  mScalebar->setHeight( d );
  mScalebar->update();
  connectUpdateSignal();
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::textFormatChanged()
{
  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Font" ) );
  disconnectUpdateSignal();
  mScalebar->setTextFormat( mFontButton->textFormat() );
  connectUpdateSignal();
  mScalebar->endCommand();
  mScalebar->update();
}

void QgsLayoutScaleBarWidget::mFillColorButton_colorChanged( const QColor &newColor )
{
  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Fill Color" ), QgsLayoutItem::UndoScaleBarFillColor );
  disconnectUpdateSignal();
  mScalebar->setFillColor( newColor );
  mScalebar->update();
  connectUpdateSignal();
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::mFillColor2Button_colorChanged( const QColor &newColor )
{
  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Fill Color" ), QgsLayoutItem::UndoScaleBarFillColor2 );
  disconnectUpdateSignal();
  mScalebar->setFillColor2( newColor );
  mScalebar->update();
  connectUpdateSignal();
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::mStrokeColorButton_colorChanged( const QColor &newColor )
{
  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Stroke Color" ), QgsLayoutItem::UndoScaleBarStrokeColor );
  disconnectUpdateSignal();
  mScalebar->setLineColor( newColor );
  mScalebar->update();
  connectUpdateSignal();
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::mUnitLabelLineEdit_textChanged( const QString &text )
{
  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Unit Text" ), QgsLayoutItem::UndoScaleBarUnitText );
  disconnectUpdateSignal();
  mScalebar->setUnitLabel( text );
  mScalebar->update();
  connectUpdateSignal();
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::mMapUnitsPerBarUnitSpinBox_valueChanged( double d )
{
  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Map Units per Segment" ), QgsLayoutItem::UndoScaleBarMapUnitsSegment );
  disconnectUpdateSignal();
  mScalebar->setMapUnitsPerScaleBarUnit( d );
  mScalebar->update();
  connectUpdateSignal();
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::mStyleComboBox_currentIndexChanged( const QString &text )
{
  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Style" ) );
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

  mScalebar->setStyle( untranslatedStyleName );
  mScalebar->update();
  connectUpdateSignal();
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::toggleStyleSpecificControls( const QString &style )
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
    mAlignmentComboBox->setEnabled( true );
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
    mAlignmentComboBox->setEnabled( false );
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

void QgsLayoutScaleBarWidget::mLabelBarSpaceSpinBox_valueChanged( double d )
{
  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Label Space" ), QgsLayoutItem::UndoScaleBarLabelBarSize );
  disconnectUpdateSignal();
  mScalebar->setLabelBarSpace( d );
  mScalebar->update();
  connectUpdateSignal();
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::mBoxSizeSpinBox_valueChanged( double d )
{
  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Box Space" ), QgsLayoutItem::UndoScaleBarBoxContentSpace );
  disconnectUpdateSignal();
  mScalebar->setBoxContentSpace( d );
  mScalebar->update();
  connectUpdateSignal();
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::mAlignmentComboBox_currentIndexChanged( int index )
{
  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Alignment" ) );
  disconnectUpdateSignal();
  mScalebar->setAlignment( static_cast< QgsScaleBarSettings::Alignment >( index ) );
  mScalebar->update();
  connectUpdateSignal();
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::mUnitsComboBox_currentIndexChanged( int index )
{
  if ( !mScalebar )
  {
    return;
  }

  QVariant unitData = mUnitsComboBox->itemData( index );
  if ( unitData.type() == QVariant::Invalid )
  {
    return;
  }

  disconnectUpdateSignal();
  mScalebar->beginCommand( tr( "Set Scalebar Units" ) );
  mScalebar->applyDefaultSize( static_cast<  QgsUnitTypes::DistanceUnit >( unitData.toInt() ) );
  mScalebar->update();

  mUnitLabelLineEdit->setText( mScalebar->unitLabel() );
  mSegmentSizeSpinBox->setValue( mScalebar->unitsPerSegment() );
  mMapUnitsPerBarUnitSpinBox->setValue( mScalebar->mapUnitsPerScaleBarUnit() );

  connectUpdateSignal();
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::blockMemberSignals( bool block )
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
  mFillColorButton->blockSignals( block );
  mFillColor2Button->blockSignals( block );
  mStrokeColorButton->blockSignals( block );
  mSegmentSizeRadioGroup.blockSignals( block );
  mMapItemComboBox->blockSignals( block );
  mFontButton->blockSignals( block );
  mMinWidthSpinBox->blockSignals( block );
  mMaxWidthSpinBox->blockSignals( block );
}

void QgsLayoutScaleBarWidget::connectUpdateSignal()
{
  if ( mScalebar )
  {
    connect( mScalebar, &QgsLayoutObject::changed, this, &QgsLayoutScaleBarWidget::setGuiElements );
  }
}

void QgsLayoutScaleBarWidget::disconnectUpdateSignal()
{
  if ( mScalebar )
  {
    disconnect( mScalebar, &QgsLayoutObject::changed, this, &QgsLayoutScaleBarWidget::setGuiElements );
  }
}

void QgsLayoutScaleBarWidget::mLineJoinStyleCombo_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Join Style" ) );
  mScalebar->setLineJoinStyle( mLineJoinStyleCombo->penJoinStyle() );
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::mLineCapStyleCombo_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Cap Style" ) );
  mScalebar->setLineCapStyle( mLineCapStyleCombo->penCapStyle() );
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::segmentSizeRadioChanged( QAbstractButton *radio )
{
  bool fixedSizeMode = radio == mFixedSizeRadio;
  mMinWidthSpinBox->setEnabled( !fixedSizeMode );
  mMaxWidthSpinBox->setEnabled( !fixedSizeMode );
  mSegmentSizeSpinBox->setEnabled( fixedSizeMode );

  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Size Mode" ), QgsLayoutItem::UndoScaleBarSegmentSize );
  disconnectUpdateSignal();
  if ( mFixedSizeRadio->isChecked() )
  {
    mScalebar->setSegmentSizeMode( QgsScaleBarSettings::SegmentSizeFixed );
    mScalebar->setUnitsPerSegment( mSegmentSizeSpinBox->value() );
  }
  else /*if(mFitWidthRadio->isChecked())*/
  {
    mScalebar->setSegmentSizeMode( QgsScaleBarSettings::SegmentSizeFitWidth );
  }
  mScalebar->update();
  connectUpdateSignal();
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::mapChanged( QgsLayoutItem *item )
{
  QgsLayoutItemMap *map = qobject_cast< QgsLayoutItemMap * >( item );
  if ( !map )
  {
    return;
  }

  //set it to scale bar
  mScalebar->beginCommand( tr( "Set Scalebar Map" ) );
  disconnectUpdateSignal();
  mScalebar->setLinkedMap( map );
  mScalebar->update();
  connectUpdateSignal();
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::mMinWidthSpinBox_valueChanged( double )
{
  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Size Mode" ), QgsLayoutItem::UndoScaleBarSegmentSize );
  disconnectUpdateSignal();
  mScalebar->setMinimumBarWidth( mMinWidthSpinBox->value() );
  mScalebar->update();
  connectUpdateSignal();
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::mMaxWidthSpinBox_valueChanged( double )
{
  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Size Mode" ), QgsLayoutItem::UndoScaleBarSegmentSize );
  disconnectUpdateSignal();
  mScalebar->setMaximumBarWidth( mMaxWidthSpinBox->value() );
  mScalebar->update();
  connectUpdateSignal();
  mScalebar->endCommand();
}
