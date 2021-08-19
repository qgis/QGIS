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
#include "qgsscalebarrendererregistry.h"
#include "qgslayout.h"
#include "qgsguiutils.h"
#include "qgsvectorlayer.h"
#include "qgsnumericformatselectorwidget.h"
#include "qgslayoutundostack.h"
#include "qgsfillsymbol.h"
#include "qgslinesymbol.h"

#include <QColorDialog>
#include <QFontDialog>
#include <QWidget>

QgsLayoutScaleBarWidget::QgsLayoutScaleBarWidget( QgsLayoutItemScaleBar *scaleBar )
  : QgsLayoutItemBaseWidget( nullptr, scaleBar )
  , mScalebar( scaleBar )
{
  setupUi( this );

  mNumberOfSubdivisionsSpinBox->setClearValue( 1 );

  connect( mHeightSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutScaleBarWidget::mHeightSpinBox_valueChanged );
  connect( mSegmentSizeSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutScaleBarWidget::mSegmentSizeSpinBox_valueChanged );
  connect( mSegmentsLeftSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLayoutScaleBarWidget::mSegmentsLeftSpinBox_valueChanged );
  connect( mNumberOfSegmentsSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLayoutScaleBarWidget::mNumberOfSegmentsSpinBox_valueChanged );
  connect( mNumberOfSubdivisionsSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLayoutScaleBarWidget::mNumberOfSubdivisionsSpinBox_valueChanged );
  connect( mSubdivisionsHeightSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutScaleBarWidget::mSubdivisionsHeightSpinBox_valueChanged );
  connect( mUnitLabelLineEdit, &QLineEdit::textChanged, this, &QgsLayoutScaleBarWidget::mUnitLabelLineEdit_textChanged );
  connect( mMapUnitsPerBarUnitSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutScaleBarWidget::mMapUnitsPerBarUnitSpinBox_valueChanged );
  connect( mStyleComboBox, &QComboBox::currentTextChanged, this, &QgsLayoutScaleBarWidget::mStyleComboBox_currentIndexChanged );
  connect( mLabelBarSpaceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutScaleBarWidget::mLabelBarSpaceSpinBox_valueChanged );
  connect( mBoxSizeSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutScaleBarWidget::mBoxSizeSpinBox_valueChanged );
  connect( mAlignmentComboBox, &QgsAlignmentComboBox::changed, this, &QgsLayoutScaleBarWidget::alignmentChanged );
  connect( mLabelVerticalPlacementComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutScaleBarWidget::mLabelVerticalPlacementComboBox_currentIndexChanged );
  connect( mLabelHorizontalPlacementComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutScaleBarWidget::mLabelHorizontalPlacementComboBox_currentIndexChanged );
  connect( mUnitsComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutScaleBarWidget::mUnitsComboBox_currentIndexChanged );
  connect( mMinWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutScaleBarWidget::mMinWidthSpinBox_valueChanged );
  connect( mMaxWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutScaleBarWidget::mMaxWidthSpinBox_valueChanged );
  connect( mNumberFormatPushButton, &QPushButton::clicked, this, &QgsLayoutScaleBarWidget::changeNumberFormat );
  setPanelTitle( tr( "Scalebar Properties" ) );

  mFontButton->registerExpressionContextGenerator( this );

  connectUpdateSignal();

  //add widget for general composer item properties
  mItemPropertiesWidget = new QgsLayoutItemPropertiesWidget( this, scaleBar );
  mainLayout->addWidget( mItemPropertiesWidget );

  mSegmentSizeRadioGroup.addButton( mFixedSizeRadio );
  mSegmentSizeRadioGroup.addButton( mFitWidthRadio );
  connect( &mSegmentSizeRadioGroup, static_cast < void ( QButtonGroup::* )( QAbstractButton * ) > ( &QButtonGroup::buttonClicked ), this, &QgsLayoutScaleBarWidget::segmentSizeRadioChanged );

  blockMemberSignals( true );

  //style combo box
  const QStringList renderers = QgsApplication::scaleBarRendererRegistry()->sortedRendererList();
  for ( const QString &renderer : renderers )
  {
    mStyleComboBox->addItem( QgsApplication::scaleBarRendererRegistry()->visibleName( renderer ), renderer );
  }

  //label vertical/horizontal placement combo box
  mLabelVerticalPlacementComboBox->addItem( tr( "Above Segments" ), static_cast< int >( QgsScaleBarSettings::LabelAboveSegment ) );
  mLabelVerticalPlacementComboBox->addItem( tr( "Below Segments" ), static_cast< int >( QgsScaleBarSettings::LabelBelowSegment ) );
  mLabelHorizontalPlacementComboBox->addItem( tr( "Centered at Segment Edge" ), static_cast< int >( QgsScaleBarSettings::LabelCenteredEdge ) );
  mLabelHorizontalPlacementComboBox->addItem( tr( "Centered at Center of Segment" ), static_cast< int >( QgsScaleBarSettings::LabelCenteredSegment ) );

  //alignment combo box
  mAlignmentComboBox->setAvailableAlignments( Qt::AlignLeft | Qt::AlignHCenter | Qt::AlignRight );

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

  mLineStyleButton->setSymbolType( Qgis::SymbolType::Line );
  connect( mLineStyleButton, &QgsSymbolButton::changed, this, &QgsLayoutScaleBarWidget::lineSymbolChanged );

  mDivisionStyleButton->setSymbolType( Qgis::SymbolType::Line );
  connect( mDivisionStyleButton, &QgsSymbolButton::changed, this, &QgsLayoutScaleBarWidget::divisionSymbolChanged );

  mSubdivisionStyleButton->setSymbolType( Qgis::SymbolType::Line );
  connect( mSubdivisionStyleButton, &QgsSymbolButton::changed, this, &QgsLayoutScaleBarWidget::subdivisionSymbolChanged );

  mFillSymbol1Button->setSymbolType( Qgis::SymbolType::Fill );
  connect( mFillSymbol1Button, &QgsSymbolButton::changed, this, &QgsLayoutScaleBarWidget::fillSymbol1Changed );

  mFillSymbol2Button->setSymbolType( Qgis::SymbolType::Fill );
  connect( mFillSymbol2Button, &QgsSymbolButton::changed, this, &QgsLayoutScaleBarWidget::fillSymbol2Changed );

  mFontButton->setDialogTitle( tr( "Scalebar Font" ) );
  mFontButton->setMode( QgsFontButton::ModeTextRenderer );

  if ( mScalebar )
  {
    QgsLayout *scaleBarLayout = mScalebar->layout();
    if ( scaleBarLayout )
    {
      mMapItemComboBox->setCurrentLayout( scaleBarLayout );
      mMapItemComboBox->setItemType( QgsLayoutItemRegistry::LayoutMap );
    }
  }

  connect( mMapItemComboBox, &QgsLayoutItemComboBox::itemChanged, this, &QgsLayoutScaleBarWidget::mapChanged );

  blockMemberSignals( false );
  setGuiElements(); //set the GUI elements to the state of scaleBar

  mLineStyleButton->registerExpressionContextGenerator( mScalebar );
  mLineStyleButton->setLayer( coverageLayer() );
  mDivisionStyleButton->registerExpressionContextGenerator( mScalebar );
  mDivisionStyleButton->setLayer( coverageLayer() );
  mSubdivisionStyleButton->registerExpressionContextGenerator( mScalebar );
  mSubdivisionStyleButton->setLayer( coverageLayer() );
  mFillSymbol1Button->registerExpressionContextGenerator( mScalebar );
  mFillSymbol1Button->setLayer( coverageLayer() );
  mFillSymbol2Button->registerExpressionContextGenerator( mScalebar );
  mFillSymbol2Button->setLayer( coverageLayer() );

  connect( mFontButton, &QgsFontButton::changed, this, &QgsLayoutScaleBarWidget::textFormatChanged );
  mFontButton->setLayer( coverageLayer() );
  if ( mScalebar->layout() )
  {
    connect( &mScalebar->layout()->reportContext(), &QgsLayoutReportContext::layerChanged, mFontButton, &QgsFontButton::setLayer );
    connect( &mScalebar->layout()->reportContext(), &QgsLayoutReportContext::layerChanged, mLineStyleButton, &QgsSymbolButton::setLayer );
    connect( &mScalebar->layout()->reportContext(), &QgsLayoutReportContext::layerChanged, mDivisionStyleButton, &QgsSymbolButton::setLayer );
    connect( &mScalebar->layout()->reportContext(), &QgsLayoutReportContext::layerChanged, mSubdivisionStyleButton, &QgsSymbolButton::setLayer );
    connect( &mScalebar->layout()->reportContext(), &QgsLayoutReportContext::layerChanged, mFillSymbol1Button, &QgsSymbolButton::setLayer );
    connect( &mScalebar->layout()->reportContext(), &QgsLayoutReportContext::layerChanged, mFillSymbol2Button, &QgsSymbolButton::setLayer );
  }
}

void QgsLayoutScaleBarWidget::setMasterLayout( QgsMasterLayoutInterface *masterLayout )
{
  if ( mItemPropertiesWidget )
    mItemPropertiesWidget->setMasterLayout( masterLayout );
}

QgsExpressionContext QgsLayoutScaleBarWidget::createExpressionContext() const
{
  QgsExpressionContext context = mScalebar->createExpressionContext();
  QgsExpressionContextScope *scaleScope = new QgsExpressionContextScope( QStringLiteral( "scalebar_text" ) );
  scaleScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "scale_value" ), 0, true, false ) );
  context.appendScope( scaleScope );
  context.setHighlightedVariables( QStringList() << QStringLiteral( "scale_value" ) );
  return context;
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
    mFillSymbol1Button->registerExpressionContextGenerator( mScalebar );
    mFillSymbol2Button->registerExpressionContextGenerator( mScalebar );
    mLineStyleButton->registerExpressionContextGenerator( mScalebar );
    mDivisionStyleButton->registerExpressionContextGenerator( mScalebar );
    mSubdivisionStyleButton->registerExpressionContextGenerator( mScalebar );
  }

  setGuiElements();

  return true;
}

void QgsLayoutScaleBarWidget::lineSymbolChanged()
{
  if ( !mScalebar )
    return;

  mScalebar->layout()->undoStack()->beginCommand( mScalebar, tr( "Change Scalebar Line Style" ), QgsLayoutItem::UndoShapeStyle );
  mScalebar->setLineSymbol( mLineStyleButton->clonedSymbol<QgsLineSymbol>() );
  mScalebar->update();
  mScalebar->layout()->undoStack()->endCommand();
}

void QgsLayoutScaleBarWidget::divisionSymbolChanged()
{
  if ( !mScalebar )
    return;

  mScalebar->layout()->undoStack()->beginCommand( mScalebar, tr( "Change Scalebar Division Style" ), QgsLayoutItem::UndoShapeStyle );
  mScalebar->setDivisionLineSymbol( mDivisionStyleButton->clonedSymbol<QgsLineSymbol>() );
  mScalebar->update();
  mScalebar->layout()->undoStack()->endCommand();
}

void QgsLayoutScaleBarWidget::subdivisionSymbolChanged()
{
  if ( !mScalebar )
    return;

  mScalebar->layout()->undoStack()->beginCommand( mScalebar, tr( "Change Scalebar Subdivision Style" ), QgsLayoutItem::UndoShapeStyle );
  mScalebar->setSubdivisionLineSymbol( mSubdivisionStyleButton->clonedSymbol<QgsLineSymbol>() );
  mScalebar->update();
  mScalebar->layout()->undoStack()->endCommand();
}

void QgsLayoutScaleBarWidget::fillSymbol1Changed()
{
  if ( !mScalebar )
    return;

  mScalebar->layout()->undoStack()->beginCommand( mScalebar, tr( "Change Scalebar Fill Style" ), QgsLayoutItem::UndoShapeStyle );
  mScalebar->setFillSymbol( mFillSymbol1Button->clonedSymbol<QgsFillSymbol>() );
  mScalebar->update();
  mScalebar->layout()->undoStack()->endCommand();
}

void QgsLayoutScaleBarWidget::fillSymbol2Changed()
{
  if ( !mScalebar )
    return;

  mScalebar->layout()->undoStack()->beginCommand( mScalebar, tr( "Change Scalebar Fill Style" ), QgsLayoutItem::UndoShapeStyle );
  mScalebar->setAlternateFillSymbol( mFillSymbol2Button->clonedSymbol<QgsFillSymbol>() );
  mScalebar->update();
  mScalebar->layout()->undoStack()->endCommand();
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
  mHeightSpinBox->setValue( mScalebar->height() );
  mNumberOfSubdivisionsSpinBox->setValue( mScalebar->numberOfSubdivisions() );
  mSubdivisionsHeightSpinBox->setValue( mScalebar->subdivisionsHeight() );
  mMapUnitsPerBarUnitSpinBox->setValue( mScalebar->mapUnitsPerScaleBarUnit() );
  mLabelBarSpaceSpinBox->setValue( mScalebar->labelBarSpace() );
  mBoxSizeSpinBox->setValue( mScalebar->boxContentSpace() );
  mUnitLabelLineEdit->setText( mScalebar->unitLabel() );
  mFontButton->setTextFormat( mScalebar->textFormat() );

  whileBlocking( mLineStyleButton )->setSymbol( mScalebar->lineSymbol()->clone() );
  whileBlocking( mDivisionStyleButton )->setSymbol( mScalebar->divisionLineSymbol()->clone() );
  whileBlocking( mSubdivisionStyleButton )->setSymbol( mScalebar->subdivisionLineSymbol()->clone() );
  whileBlocking( mFillSymbol1Button )->setSymbol( mScalebar->fillSymbol()->clone() );
  whileBlocking( mFillSymbol2Button )->setSymbol( mScalebar->alternateFillSymbol()->clone() );

  //map combo box
  mMapItemComboBox->setItem( mScalebar->linkedMap() );

  //style...
  const QString style = mScalebar->style();
  mStyleComboBox->setCurrentIndex( mStyleComboBox->findData( style ) );
  toggleStyleSpecificControls( style );

  //label vertical/horizontal placement
  mLabelVerticalPlacementComboBox->setCurrentIndex( mLabelVerticalPlacementComboBox->findData( static_cast< int >( mScalebar->labelVerticalPlacement() ) ) );
  mLabelHorizontalPlacementComboBox->setCurrentIndex( mLabelHorizontalPlacementComboBox->findData( static_cast< int >( mScalebar->labelHorizontalPlacement() ) ) );

  //alignment

  Qt::Alignment a = Qt::AlignLeft;
  switch ( mScalebar->alignment() )
  {
    case QgsScaleBarSettings::AlignLeft:
      a = Qt::AlignLeft;
      break;
    case QgsScaleBarSettings::AlignRight:
      a = Qt::AlignRight;
      break;
    case QgsScaleBarSettings::AlignMiddle:
      a = Qt::AlignHCenter;
      break;
  }
  mAlignmentComboBox->setCurrentAlignment( a );

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
  blockMemberSignals( false );
}

//slots

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

void QgsLayoutScaleBarWidget::mNumberOfSubdivisionsSpinBox_valueChanged( int i )
{
  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Subdivisions" ), QgsLayoutItem::UndoScaleBarSubdivisions );
  disconnectUpdateSignal();
  mScalebar->setNumberOfSubdivisions( i );
  mScalebar->update();
  connectUpdateSignal();
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::mSubdivisionsHeightSpinBox_valueChanged( double d )
{
  if ( !mScalebar )
  {
    return;
  }
  mScalebar->beginCommand( tr( "Set Subdivisions Height" ), QgsLayoutItem::UndoScaleBarSubdivisionsHeight );
  disconnectUpdateSignal();
  mScalebar->setSubdivisionsHeight( d );
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

void QgsLayoutScaleBarWidget::changeNumberFormat()
{
  if ( !mScalebar )
  {
    return;
  }

  QgsNumericFormatSelectorWidget *widget = new QgsNumericFormatSelectorWidget( this );
  widget->setPanelTitle( tr( "Number Format" ) );
  widget->setFormat( mScalebar->numericFormat() );
  connect( widget, &QgsNumericFormatSelectorWidget::changed, this, [ = ]
  {
    mScalebar->beginCommand( tr( "Set Scalebar Number Format" ) );
    disconnectUpdateSignal();
    mScalebar->setNumericFormat( widget->format() );
    connectUpdateSignal();
    mScalebar->endCommand();
    mScalebar->update();
  } );
  openPanel( widget );
  return;
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

void QgsLayoutScaleBarWidget::mStyleComboBox_currentIndexChanged( const QString & )
{
  if ( !mScalebar )
  {
    return;
  }

  const QString rendererId = mStyleComboBox->currentData().toString();
  if ( rendererId == mScalebar->style() )
    return;

  mScalebar->beginCommand( tr( "Set Scalebar Style" ) );
  disconnectUpdateSignal();

  bool defaultsApplied = false;
  const std::unique_ptr< QgsScaleBarRenderer > renderer( QgsApplication::scaleBarRendererRegistry()->renderer( rendererId ) );
  if ( renderer )
    defaultsApplied = mScalebar->applyDefaultRendererSettings( renderer.get() );

  //disable or enable controls which apply to specific scale bar styles
  toggleStyleSpecificControls( rendererId );

  mScalebar->setStyle( rendererId );
  mScalebar->update();
  connectUpdateSignal();
  mScalebar->endCommand();

  if ( defaultsApplied )
    setGuiElements();
}

void QgsLayoutScaleBarWidget::toggleStyleSpecificControls( const QString &style )
{
  std::unique_ptr< QgsScaleBarRenderer > renderer( QgsApplication::scaleBarRendererRegistry()->renderer( style ) );

  //Selectively enable controls which apply to the scale bar style
  mUnitsComboBox->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagRespectsUnits : true );
  mUnitsLabel->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagRespectsUnits : true );
  mMapUnitsPerBarUnitSpinBox->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagRespectsMapUnitsPerScaleBarUnit : true );
  mMapUnitsPerBarUnitLabel->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagRespectsMapUnitsPerScaleBarUnit : true );
  mUnitLabelLineEdit->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesUnitLabel : true );
  mUnitLabelLabel->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesUnitLabel : true );
  mSubdivisionsLabel->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesSubdivisions : true );
  mNumberOfSubdivisionsSpinBox->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesSubdivisions : true );
  mSubdivisionsHeightLabel->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesSubdivisionsHeight : true );
  mSubdivisionsHeightSpinBox->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesSubdivisionsHeight : true );
  mGroupBoxSegments->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesSegments : true );
  if ( !mGroupBoxUnits->isEnabled() )
    mGroupBoxSegments->setCollapsed( true );
  mLabelBarSpaceSpinBox->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesLabelBarSpace : true );
  mLabelBarSpaceLabel->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesLabelBarSpace : true );
  mLabelVerticalPlacementComboBox->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesLabelVerticalPlacement : true );
  mLabelVerticalPlacementLabel->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesLabelVerticalPlacement : true );
  mLabelHorizontalPlacementComboBox->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesLabelHorizontalPlacement : true );
  mLabelHorizontalPlacementLabel->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesLabelHorizontalPlacement : true );
  mAlignmentComboBox->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesAlignment : true );
  mAlignmentLabel->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesAlignment : true );
  mFillSymbol1Button->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesFillSymbol : true );
  mFillSymbol1Label->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesFillSymbol : true );
  mFillSymbol2Button->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesAlternateFillSymbol : true );
  mFillSymbol2Label->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesAlternateFillSymbol : true );
  mLineStyleButton->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesLineSymbol : true );
  mLineStyleLabel->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesLineSymbol : true );
  mDivisionStyleButton->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesDivisionSymbol : true );
  mDivisionStyleLabel->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesDivisionSymbol : true );
  mSubdivisionStyleButton->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesSubdivisionSymbol : true );
  mSubdivisionStyleLabel->setEnabled( renderer ? renderer->flags() & QgsScaleBarRenderer::Flag::FlagUsesSubdivisionSymbol : true );
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

void QgsLayoutScaleBarWidget::mLabelVerticalPlacementComboBox_currentIndexChanged( int index )
{
  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Label Vertical Placement" ) );
  disconnectUpdateSignal();
  mScalebar->setLabelVerticalPlacement( static_cast<QgsScaleBarSettings::LabelVerticalPlacement>( mLabelVerticalPlacementComboBox->itemData( index ).toInt() ) );
  mScalebar->update();
  connectUpdateSignal();
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::mLabelHorizontalPlacementComboBox_currentIndexChanged( int index )
{
  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Label Horizontal Placement" ) );
  disconnectUpdateSignal();
  mScalebar->setLabelHorizontalPlacement( static_cast<QgsScaleBarSettings::LabelHorizontalPlacement>( mLabelHorizontalPlacementComboBox->itemData( index ).toInt() ) );
  mScalebar->update();
  connectUpdateSignal();
  mScalebar->endCommand();
}

void QgsLayoutScaleBarWidget::alignmentChanged()
{
  if ( !mScalebar )
  {
    return;
  }

  mScalebar->beginCommand( tr( "Set Scalebar Alignment" ) );
  disconnectUpdateSignal();

  const QgsScaleBarSettings::Alignment a = mAlignmentComboBox->currentAlignment() & Qt::AlignLeft ? QgsScaleBarSettings::AlignLeft
      : mAlignmentComboBox->currentAlignment() & Qt::AlignRight ? QgsScaleBarSettings::AlignRight : QgsScaleBarSettings::AlignMiddle;
  mScalebar->setAlignment( a );
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

  const QVariant unitData = mUnitsComboBox->itemData( index );
  if ( unitData.type() == QVariant::Invalid )
  {
    return;
  }

  disconnectUpdateSignal();
  mScalebar->beginCommand( tr( "Set Scalebar Units" ) );
  mScalebar->applyDefaultSize( static_cast<  QgsUnitTypes::DistanceUnit >( unitData.toInt() ) );
  mScalebar->update();

  mNumberOfSegmentsSpinBox->setValue( mScalebar->numberOfSegments() );
  mSegmentsLeftSpinBox->setValue( mScalebar->numberOfSegmentsLeft() );
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
  mNumberOfSubdivisionsSpinBox->blockSignals( block );
  mSubdivisionsHeightSpinBox->blockSignals( block );
  mStyleComboBox->blockSignals( block );
  mUnitLabelLineEdit->blockSignals( block );
  mMapUnitsPerBarUnitSpinBox->blockSignals( block );
  mHeightSpinBox->blockSignals( block );
  mLineStyleButton->blockSignals( block );
  mDivisionStyleButton->blockSignals( block );
  mSubdivisionStyleButton->blockSignals( block );
  mLabelBarSpaceSpinBox->blockSignals( block );
  mBoxSizeSpinBox->blockSignals( block );
  mLabelVerticalPlacementComboBox->blockSignals( block );
  mLabelHorizontalPlacementComboBox->blockSignals( block );
  mAlignmentComboBox->blockSignals( block );
  mUnitsComboBox->blockSignals( block );
  mFillSymbol1Button->blockSignals( block );
  mFillSymbol2Button->blockSignals( block );
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

void QgsLayoutScaleBarWidget::segmentSizeRadioChanged( QAbstractButton *radio )
{
  const bool fixedSizeMode = radio == mFixedSizeRadio;
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
