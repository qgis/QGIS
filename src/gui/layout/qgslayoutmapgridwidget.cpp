/***************************************************************************
                         qgslayoutmapgridwidget.cpp
                         ----------------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutmapgridwidget.h"
#include "qgssymbolselectordialog.h"
#include "qgssymbol.h"
#include "qgslayoutitemmap.h"
#include "qgsproject.h"
#include "qgssymbollayerutils.h"
#include "qgsstyle.h"
#include "qgsprojectionselectiondialog.h"
#include "qgslayout.h"
#include "qgsmapsettings.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsvectorlayer.h"
#include "qgsprojectviewsettings.h"
#include "qgstextformatwidget.h"
#include "qgsguiutils.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"

QgsLayoutMapGridWidget::QgsLayoutMapGridWidget( QgsLayoutItemMapGrid *mapGrid, QgsLayoutItemMap *map )
  : QgsLayoutItemBaseWidget( nullptr, mapGrid )
  , mMap( map )
  , mMapGrid( mapGrid )
{
  setupUi( this );

  mFrameStyleComboBox->addItem( tr( "No Frame" ), QgsLayoutItemMapGrid::NoFrame );
  mFrameStyleComboBox->addItem( tr( "Zebra" ), QgsLayoutItemMapGrid::Zebra );
  mFrameStyleComboBox->addItem( tr( "Zebra (Nautical)" ), QgsLayoutItemMapGrid::ZebraNautical );
  mFrameStyleComboBox->addItem( tr( "Interior Ticks" ), QgsLayoutItemMapGrid::InteriorTicks );
  mFrameStyleComboBox->addItem( tr( "Exterior Ticks" ), QgsLayoutItemMapGrid::ExteriorTicks );
  mFrameStyleComboBox->addItem( tr( "Interior and Exterior Ticks" ), QgsLayoutItemMapGrid::InteriorExteriorTicks );
  mFrameStyleComboBox->addItem( tr( "Line Border" ), QgsLayoutItemMapGrid::LineBorder );
  mFrameStyleComboBox->addItem( tr( "Line Border (Nautical)" ), QgsLayoutItemMapGrid::LineBorderNautical );

  mRotatedTicksLengthModeComboBox->addItem( tr( "Orthogonal" ), QgsLayoutItemMapGrid::OrthogonalTicks );
  mRotatedTicksLengthModeComboBox->addItem( tr( "Fixed Length" ), QgsLayoutItemMapGrid::NormalizedTicks );
  mRotatedAnnotationsLengthModeComboBox->addItem( tr( "Orthogonal" ), QgsLayoutItemMapGrid::OrthogonalTicks );
  mRotatedAnnotationsLengthModeComboBox->addItem( tr( "Fixed Length" ), QgsLayoutItemMapGrid::NormalizedTicks );

  mGridFrameMarginSpinBox->setShowClearButton( true );
  mGridFrameMarginSpinBox->setClearValue( 0 );

  mDistanceToMapFrameSpinBox->setShowClearButton( true );
  mDistanceToMapFrameSpinBox->setClearValue( 0 );

  connect( mIntervalXSpinBox, &QgsDoubleSpinBox::editingFinished, this, &QgsLayoutMapGridWidget::mIntervalXSpinBox_editingFinished );
  connect( mIntervalYSpinBox, &QgsDoubleSpinBox::editingFinished, this, &QgsLayoutMapGridWidget::mIntervalYSpinBox_editingFinished );
  connect( mOffsetXSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mOffsetXSpinBox_valueChanged );
  connect( mOffsetYSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mOffsetYSpinBox_valueChanged );
  connect( mCrossWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mCrossWidthSpinBox_valueChanged );
  connect( mFrameWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mFrameWidthSpinBox_valueChanged );
  connect( mGridFrameMarginSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mGridFrameMarginSpinBox_valueChanged );
  connect( mFrameStyleComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mFrameStyleComboBox_currentIndexChanged );
  connect( mRotatedTicksGroupBox, &QGroupBox::toggled, this, &QgsLayoutMapGridWidget::mRotatedTicksGroupBox_toggled );
  connect( mRotatedTicksLengthModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mRotatedTicksLengthModeComboBox_currentIndexChanged );
  connect( mRotatedTicksThresholdSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mRotatedTicksThresholdSpinBox_valueChanged );
  connect( mRotatedTicksMarginToCornerSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mRotatedTicksMarginToCornerSpinBox_valueChanged );
  connect( mRotatedAnnotationsGroupBox, &QGroupBox::toggled, this, &QgsLayoutMapGridWidget::mRotatedAnnotationsGroupBox_toggled );
  connect( mRotatedAnnotationsLengthModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mRotatedAnnotationsLengthModeComboBox_currentIndexChanged );
  connect( mRotatedAnnotationsThresholdSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mRotatedAnnotationsThresholdSpinBox_valueChanged );
  connect( mRotatedAnnotationsMarginToCornerSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mRotatedAnnotationsMarginToCornerSpinBox_valueChanged );
  connect( mGridFramePenSizeSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mGridFramePenSizeSpinBox_valueChanged );
  connect( mGridFramePenColorButton, &QgsColorButton::colorChanged, this, &QgsLayoutMapGridWidget::mGridFramePenColorButton_colorChanged );
  connect( mGridFrameFill1ColorButton, &QgsColorButton::colorChanged, this, &QgsLayoutMapGridWidget::mGridFrameFill1ColorButton_colorChanged );
  connect( mGridFrameFill2ColorButton, &QgsColorButton::colorChanged, this, &QgsLayoutMapGridWidget::mGridFrameFill2ColorButton_colorChanged );
  connect( mGridTypeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mGridTypeComboBox_currentIndexChanged );
  connect( mMapGridUnitComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::intervalUnitChanged );
  connect( mGridBlendComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mGridBlendComboBox_currentIndexChanged );
  connect( mCheckGridLeftSide, &QCheckBox::toggled, this, &QgsLayoutMapGridWidget::mCheckGridLeftSide_toggled );
  connect( mCheckGridRightSide, &QCheckBox::toggled, this, &QgsLayoutMapGridWidget::mCheckGridRightSide_toggled );
  connect( mCheckGridTopSide, &QCheckBox::toggled, this, &QgsLayoutMapGridWidget::mCheckGridTopSide_toggled );
  connect( mCheckGridBottomSide, &QCheckBox::toggled, this, &QgsLayoutMapGridWidget::mCheckGridBottomSide_toggled );
  connect( mFrameDivisionsLeftComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mFrameDivisionsLeftComboBox_currentIndexChanged );
  connect( mFrameDivisionsRightComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mFrameDivisionsRightComboBox_currentIndexChanged );
  connect( mFrameDivisionsTopComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mFrameDivisionsTopComboBox_currentIndexChanged );
  connect( mFrameDivisionsBottomComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mFrameDivisionsBottomComboBox_currentIndexChanged );
  connect( mDrawAnnotationGroupBox, &QgsCollapsibleGroupBoxBasic::toggled, this, &QgsLayoutMapGridWidget::mDrawAnnotationGroupBox_toggled );
  connect( mAnnotationFormatButton, &QToolButton::clicked, this, &QgsLayoutMapGridWidget::mAnnotationFormatButton_clicked );
  connect( mAnnotationDisplayLeftComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationDisplayLeftComboBox_currentIndexChanged );
  connect( mAnnotationDisplayRightComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationDisplayRightComboBox_currentIndexChanged );
  connect( mAnnotationDisplayTopComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationDisplayTopComboBox_currentIndexChanged );
  connect( mAnnotationDisplayBottomComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationDisplayBottomComboBox_currentIndexChanged );
  connect( mAnnotationPositionLeftComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationPositionLeftComboBox_currentIndexChanged );
  connect( mAnnotationPositionRightComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationPositionRightComboBox_currentIndexChanged );
  connect( mAnnotationPositionTopComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationPositionTopComboBox_currentIndexChanged );
  connect( mAnnotationPositionBottomComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationPositionBottomComboBox_currentIndexChanged );
  connect( mAnnotationDirectionComboBoxLeft, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationDirectionComboBoxLeft_currentIndexChanged );
  connect( mAnnotationDirectionComboBoxRight, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationDirectionComboBoxRight_currentIndexChanged );
  connect( mAnnotationDirectionComboBoxTop, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationDirectionComboBoxTop_currentIndexChanged );
  connect( mAnnotationDirectionComboBoxBottom, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationDirectionComboBoxBottom_currentIndexChanged );
  connect( mAnnotationFormatComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationFormatComboBox_currentIndexChanged );
  connect( mCoordinatePrecisionSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mCoordinatePrecisionSpinBox_valueChanged );
  connect( mDistanceToMapFrameSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mDistanceToMapFrameSpinBox_valueChanged );
  connect( mMinWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::minIntervalChanged );
  connect( mMaxWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::maxIntervalChanged );
  connect( mEnabledCheckBox, &QCheckBox::toggled, this, &QgsLayoutMapGridWidget::gridEnabledToggled );
  setPanelTitle( tr( "Map Grid Properties" ) );

  mMapGridCrsSelector->setOptionVisible( QgsProjectionSelectionWidget::CrsNotSet, true );
  mMapGridCrsSelector->setNotSetText( tr( "Use Map CRS" ) );
  mMapGridCrsSelector->setDialogTitle( tr( "Grid CRS" ) );

  connect( mMapGridCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsLayoutMapGridWidget::mapGridCrsChanged );

  blockAllSignals( true );

  mMapGridUnitComboBox->addItem( tr( "Map Units" ), QgsLayoutItemMapGrid::MapUnit );
  mMapGridUnitComboBox->addItem( tr( "Fit Segment Width" ), QgsLayoutItemMapGrid::DynamicPageSizeBased );
  mMapGridUnitComboBox->addItem( tr( "Millimeters" ), QgsLayoutItemMapGrid::MM );
  mMapGridUnitComboBox->addItem( tr( "Centimeters" ), QgsLayoutItemMapGrid::CM );

  mGridTypeComboBox->insertItem( 0, tr( "Solid" ), QgsLayoutItemMapGrid::Solid );
  mGridTypeComboBox->insertItem( 1, tr( "Cross" ), QgsLayoutItemMapGrid::Cross );
  mGridTypeComboBox->insertItem( 2, tr( "Markers" ), QgsLayoutItemMapGrid::Markers );
  mGridTypeComboBox->insertItem( 3, tr( "Frame and annotations only" ), QgsLayoutItemMapGrid::FrameAnnotationsOnly );

  insertFrameDisplayEntries( mFrameDivisionsLeftComboBox );
  insertFrameDisplayEntries( mFrameDivisionsRightComboBox );
  insertFrameDisplayEntries( mFrameDivisionsTopComboBox );
  insertFrameDisplayEntries( mFrameDivisionsBottomComboBox );

  insertAnnotationDisplayEntries( mAnnotationDisplayLeftComboBox );
  insertAnnotationDisplayEntries( mAnnotationDisplayRightComboBox );
  insertAnnotationDisplayEntries( mAnnotationDisplayTopComboBox );
  insertAnnotationDisplayEntries( mAnnotationDisplayBottomComboBox );

  insertAnnotationPositionEntries( mAnnotationPositionLeftComboBox );
  insertAnnotationPositionEntries( mAnnotationPositionRightComboBox );
  insertAnnotationPositionEntries( mAnnotationPositionTopComboBox );
  insertAnnotationPositionEntries( mAnnotationPositionBottomComboBox );

  insertAnnotationDirectionEntries( mAnnotationDirectionComboBoxLeft );
  insertAnnotationDirectionEntries( mAnnotationDirectionComboBoxRight );
  insertAnnotationDirectionEntries( mAnnotationDirectionComboBoxTop );
  insertAnnotationDirectionEntries( mAnnotationDirectionComboBoxBottom );

  mGridFramePenColorButton->setColorDialogTitle( tr( "Select Grid Frame Color" ) );
  mGridFramePenColorButton->setAllowOpacity( true );
  mGridFramePenColorButton->setContext( QStringLiteral( "composer" ) );
  mGridFramePenColorButton->setNoColorString( tr( "Transparent Frame" ) );
  mGridFramePenColorButton->setShowNoColor( true );

  mGridFrameFill1ColorButton->setColorDialogTitle( tr( "Select Grid Frame Fill Color" ) );
  mGridFrameFill1ColorButton->setAllowOpacity( true );
  mGridFrameFill1ColorButton->setContext( QStringLiteral( "composer" ) );
  mGridFrameFill1ColorButton->setNoColorString( tr( "Transparent Fill" ) );
  mGridFrameFill1ColorButton->setShowNoColor( true );

  mGridFrameFill2ColorButton->setColorDialogTitle( tr( "Select Grid Frame Fill Color" ) );
  mGridFrameFill2ColorButton->setAllowOpacity( true );
  mGridFrameFill2ColorButton->setContext( QStringLiteral( "composer" ) );
  mGridFrameFill2ColorButton->setNoColorString( tr( "Transparent Fill" ) );
  mGridFrameFill2ColorButton->setShowNoColor( true );

  mGridLineStyleButton->setSymbolType( Qgis::SymbolType::Line );
  mGridMarkerStyleButton->setSymbolType( Qgis::SymbolType::Marker );

  //set initial state of frame style controls
  toggleFrameControls( false, false, false, false );

  registerDataDefinedButton( mEnabledDDBtn, QgsLayoutObject::MapGridEnabled );
  registerDataDefinedButton( mIntervalXDDBtn, QgsLayoutObject::MapGridIntervalX );
  registerDataDefinedButton( mIntervalYDDBtn, QgsLayoutObject::MapGridIntervalY );
  registerDataDefinedButton( mOffsetXDDBtn, QgsLayoutObject::MapGridOffsetX );
  registerDataDefinedButton( mOffsetYDDBtn, QgsLayoutObject::MapGridOffsetY );
  registerDataDefinedButton( mFrameSizeDDBtn, QgsLayoutObject::MapGridFrameSize );
  registerDataDefinedButton( mFrameMarginDDBtn, QgsLayoutObject::MapGridFrameMargin );
  registerDataDefinedButton( mLabelDistDDBtn, QgsLayoutObject::MapGridLabelDistance );
  registerDataDefinedButton( mCrossWidthDDBtn, QgsLayoutObject::MapGridCrossSize );
  registerDataDefinedButton( mFrameLineThicknessDDBtn, QgsLayoutObject::MapGridFrameLineThickness );
  registerDataDefinedButton( mAnnotationDisplayLeftDDBtn, QgsLayoutObject::MapGridAnnotationDisplayLeft );
  registerDataDefinedButton( mAnnotationDisplayRightDDBtn, QgsLayoutObject::MapGridAnnotationDisplayRight );
  registerDataDefinedButton( mAnnotationDisplayTopDDBtn, QgsLayoutObject::MapGridAnnotationDisplayTop );
  registerDataDefinedButton( mAnnotationDisplayBottomDDBtn, QgsLayoutObject::MapGridAnnotationDisplayBottom );
  registerDataDefinedButton( mFrameDivisionsLeftDDBtn, QgsLayoutObject::MapGridFrameDivisionsLeft );
  registerDataDefinedButton( mFrameDivisionsRightDDBtn, QgsLayoutObject::MapGridFrameDivisionsRight );
  registerDataDefinedButton( mFrameDivisionsTopDDBtn, QgsLayoutObject::MapGridFrameDivisionsTop );
  registerDataDefinedButton( mFrameDivisionsBottomDDBtn, QgsLayoutObject::MapGridFrameDivisionsBottom );

  // call to initially populate mAnnotationFormatComboBox
  onCrsChanged();

  updateGuiElements();

  blockAllSignals( false );
  connect( mAnnotationFontButton, &QgsFontButton::changed, this, &QgsLayoutMapGridWidget::annotationTextFormatChanged );
  connect( mGridLineStyleButton, &QgsSymbolButton::changed, this, &QgsLayoutMapGridWidget::lineSymbolChanged );
  connect( mGridMarkerStyleButton, &QgsSymbolButton::changed, this, &QgsLayoutMapGridWidget::markerSymbolChanged );

  mGridLineStyleButton->registerExpressionContextGenerator( mMapGrid );
  mGridLineStyleButton->setLayer( coverageLayer() );
  mGridMarkerStyleButton->registerExpressionContextGenerator( mMapGrid );
  mGridMarkerStyleButton->setLayer( coverageLayer() );
  if ( mMap->layout() )
  {
    connect( &mMap->layout()->reportContext(), &QgsLayoutReportContext::layerChanged, mGridLineStyleButton, &QgsSymbolButton::setLayer );
    connect( &mMap->layout()->reportContext(), &QgsLayoutReportContext::layerChanged, mGridMarkerStyleButton, &QgsSymbolButton::setLayer );
    connect( &mMap->layout()->reportContext(), &QgsLayoutReportContext::layerChanged, mAnnotationFontButton, &QgsFontButton::setLayer );
  }
  mAnnotationFontButton->setLayer( coverageLayer() );
  mAnnotationFontButton->registerExpressionContextGenerator( mMapGrid );

  connect( mMapGrid, &QgsLayoutItemMapGrid::crsChanged, this, &QgsLayoutMapGridWidget::onCrsChanged );
}

void QgsLayoutMapGridWidget::populateDataDefinedButtons()
{
  updateDataDefinedButton( mEnabledDDBtn );
  updateDataDefinedButton( mIntervalXDDBtn );
  updateDataDefinedButton( mIntervalYDDBtn );
  updateDataDefinedButton( mOffsetXDDBtn );
  updateDataDefinedButton( mOffsetYDDBtn );
  updateDataDefinedButton( mFrameSizeDDBtn );
  updateDataDefinedButton( mFrameMarginDDBtn );
  updateDataDefinedButton( mLabelDistDDBtn );
  updateDataDefinedButton( mCrossWidthDDBtn );
  updateDataDefinedButton( mFrameLineThicknessDDBtn );
  updateDataDefinedButton( mAnnotationDisplayLeftDDBtn );
  updateDataDefinedButton( mAnnotationDisplayRightDDBtn );
  updateDataDefinedButton( mAnnotationDisplayTopDDBtn );
  updateDataDefinedButton( mAnnotationDisplayBottomDDBtn );
  updateDataDefinedButton( mFrameDivisionsLeftDDBtn );
  updateDataDefinedButton( mFrameDivisionsRightDDBtn );
  updateDataDefinedButton( mFrameDivisionsTopDDBtn );
  updateDataDefinedButton( mFrameDivisionsBottomDDBtn );
}

void QgsLayoutMapGridWidget::setGuiElementValues()
{
  updateGuiElements();
}

void QgsLayoutMapGridWidget::updateGuiElements()
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  blockAllSignals( true );
  populateDataDefinedButtons();
  setGridItems();
  blockAllSignals( false );
}

void QgsLayoutMapGridWidget::blockAllSignals( bool block )
{
  //grid
  mEnabledCheckBox->blockSignals( block );
  mGridTypeComboBox->blockSignals( block );
  mIntervalXSpinBox->blockSignals( block );
  mIntervalYSpinBox->blockSignals( block );
  mOffsetXSpinBox->blockSignals( block );
  mOffsetYSpinBox->blockSignals( block );
  mCrossWidthSpinBox->blockSignals( block );
  mFrameStyleComboBox->blockSignals( block );
  mFrameWidthSpinBox->blockSignals( block );
  mGridFrameMarginSpinBox->blockSignals( block );
  mGridLineStyleButton->blockSignals( block );
  mMapGridUnitComboBox->blockSignals( block );
  mGridFramePenSizeSpinBox->blockSignals( block );
  mGridFramePenColorButton->blockSignals( block );
  mGridFrameFill1ColorButton->blockSignals( block );
  mGridFrameFill2ColorButton->blockSignals( block );
  mGridBlendComboBox->blockSignals( block );
  mCheckGridLeftSide->blockSignals( block );
  mCheckGridRightSide->blockSignals( block );
  mCheckGridTopSide->blockSignals( block );
  mCheckGridBottomSide->blockSignals( block );
  mFrameDivisionsLeftComboBox->blockSignals( block );
  mFrameDivisionsRightComboBox->blockSignals( block );
  mFrameDivisionsTopComboBox->blockSignals( block );
  mFrameDivisionsBottomComboBox->blockSignals( block );
  mGridMarkerStyleButton->blockSignals( block );

  //grid annotation
  mDrawAnnotationGroupBox->blockSignals( block );
  mAnnotationFormatComboBox->blockSignals( block );
  mAnnotationDisplayLeftComboBox->blockSignals( block );
  mAnnotationPositionLeftComboBox->blockSignals( block );
  mAnnotationDirectionComboBoxLeft->blockSignals( block );
  mAnnotationDisplayRightComboBox->blockSignals( block );
  mAnnotationPositionRightComboBox->blockSignals( block );
  mAnnotationDirectionComboBoxRight->blockSignals( block );
  mAnnotationDisplayTopComboBox->blockSignals( block );
  mAnnotationPositionTopComboBox->blockSignals( block );
  mAnnotationDirectionComboBoxTop->blockSignals( block );
  mAnnotationDisplayBottomComboBox->blockSignals( block );
  mAnnotationPositionBottomComboBox->blockSignals( block );
  mAnnotationDirectionComboBoxBottom->blockSignals( block );
  mDistanceToMapFrameSpinBox->blockSignals( block );
  mCoordinatePrecisionSpinBox->blockSignals( block );
  mAnnotationFontButton->blockSignals( block );
  mMinWidthSpinBox->blockSignals( block );
  mMaxWidthSpinBox->blockSignals( block );
}

void QgsLayoutMapGridWidget::handleChangedFrameDisplay( QgsLayoutItemMapGrid::BorderSide border, const QgsLayoutItemMapGrid::DisplayMode mode )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Frame Divisions" ) );
  mMapGrid->setFrameDivisions( mode, border );
  mMap->endCommand();
  mMap->updateBoundingRect();
}

void QgsLayoutMapGridWidget::handleChangedAnnotationDisplay( QgsLayoutItemMapGrid::BorderSide border, const QgsLayoutItemMapGrid::DisplayMode mode )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Annotation Format" ) );
  mMapGrid->setAnnotationDisplay( mode, border );
  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::toggleFrameControls( bool frameEnabled, bool frameFillEnabled, bool frameSizeEnabled, bool rotationEnabled )
{
  //set status of frame controls
  mFrameWidthSpinBox->setEnabled( frameSizeEnabled );
  mGridFrameMarginSpinBox->setEnabled( frameEnabled );
  mGridFramePenSizeSpinBox->setEnabled( frameEnabled );
  mGridFramePenColorButton->setEnabled( frameEnabled );
  mGridFrameFill1ColorButton->setEnabled( frameFillEnabled );
  mGridFrameFill2ColorButton->setEnabled( frameFillEnabled );
  mFrameWidthLabel->setEnabled( frameSizeEnabled );
  mFrameMarginLabel->setEnabled( frameEnabled );
  mFramePenLabel->setEnabled( frameEnabled );
  mFrameFillLabel->setEnabled( frameFillEnabled );
  mCheckGridLeftSide->setEnabled( frameEnabled );
  mCheckGridRightSide->setEnabled( frameEnabled );
  mCheckGridTopSide->setEnabled( frameEnabled );
  mCheckGridBottomSide->setEnabled( frameEnabled );
  mFrameDivisionsLeftComboBox->setEnabled( frameEnabled );
  mFrameDivisionsRightComboBox->setEnabled( frameEnabled );
  mFrameDivisionsTopComboBox->setEnabled( frameEnabled );
  mFrameDivisionsBottomComboBox->setEnabled( frameEnabled );
  mLeftDivisionsLabel->setEnabled( frameEnabled );
  mRightDivisionsLabel->setEnabled( frameEnabled );
  mTopDivisionsLabel->setEnabled( frameEnabled );
  mBottomDivisionsLabel->setEnabled( frameEnabled );
  mRotatedTicksGroupBox->setEnabled( rotationEnabled );
}

void QgsLayoutMapGridWidget::insertAnnotationPositionEntries( QComboBox *c )
{
  c->insertItem( 0, tr( "Inside Frame" ), QgsLayoutItemMapGrid::InsideMapFrame );
  c->insertItem( 1, tr( "Outside Frame" ), QgsLayoutItemMapGrid::OutsideMapFrame );
}

void QgsLayoutMapGridWidget::insertAnnotationDirectionEntries( QComboBox *c )
{
  c->addItem( tr( "Horizontal" ), QgsLayoutItemMapGrid::Horizontal );
  c->addItem( tr( "Vertical Ascending" ), QgsLayoutItemMapGrid::Vertical );
  c->addItem( tr( "Vertical Descending" ), QgsLayoutItemMapGrid::VerticalDescending );
  c->addItem( tr( "Boundary Direction" ), QgsLayoutItemMapGrid::BoundaryDirection );
  // c->addItem( tr( "Parallel to Tick" ), QgsLayoutItemMapGrid::ParallelToTick );
  c->addItem( tr( "Above Tick" ), QgsLayoutItemMapGrid::AboveTick );
  c->addItem( tr( "On Tick" ), QgsLayoutItemMapGrid::OnTick );
  c->addItem( tr( "Under Tick" ), QgsLayoutItemMapGrid::UnderTick );
}

void QgsLayoutMapGridWidget::initFrameDisplayBox( QComboBox *c, QgsLayoutItemMapGrid::DisplayMode display )
{
  if ( !c )
  {
    return;
  }
  c->setCurrentIndex( c->findData( display ) );
}

void QgsLayoutMapGridWidget::handleChangedAnnotationPosition( QgsLayoutItemMapGrid::BorderSide border, const QgsLayoutItemMapGrid::AnnotationPosition position )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Annotation Position" ) );
  mMapGrid->setAnnotationPosition( position, border );

  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::handleChangedAnnotationDirection( QgsLayoutItemMapGrid::BorderSide border, QgsLayoutItemMapGrid::AnnotationDirection direction )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Annotation Direction" ) );
  mMapGrid->setAnnotationDirection( direction, border );
  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::insertFrameDisplayEntries( QComboBox *c )
{
  c->addItem( tr( "All" ), QgsLayoutItemMapGrid::ShowAll );
  c->addItem( tr( "Latitude/Y Only" ), QgsLayoutItemMapGrid::LatitudeOnly );
  c->addItem( tr( "Longitude/X Only" ), QgsLayoutItemMapGrid::LongitudeOnly );
}

void QgsLayoutMapGridWidget::insertAnnotationDisplayEntries( QComboBox *c )
{
  c->insertItem( 0, tr( "Show All" ), QgsLayoutItemMapGrid::ShowAll );
  c->insertItem( 1, tr( "Show Latitude/Y Only" ), QgsLayoutItemMapGrid::LatitudeOnly );
  c->insertItem( 2, tr( "Show Longitude/X Only" ), QgsLayoutItemMapGrid::LongitudeOnly );
  c->insertItem( 3, tr( "Disabled" ), QgsLayoutItemMapGrid::HideAll );
}

void QgsLayoutMapGridWidget::initAnnotationPositionBox( QComboBox *c, QgsLayoutItemMapGrid::AnnotationPosition pos )
{
  if ( !c )
  {
    return;
  }

  if ( pos == QgsLayoutItemMapGrid::InsideMapFrame )
  {
    c->setCurrentIndex( c->findText( tr( "Inside Frame" ) ) );
  }
  else
  {
    c->setCurrentIndex( c->findText( tr( "Outside Frame" ) ) );
  }
}

void QgsLayoutMapGridWidget::initAnnotationDirectionBox( QComboBox *c, QgsLayoutItemMapGrid::AnnotationDirection dir )
{
  if ( !c )
  {
    return;
  }
  c->setCurrentIndex( c->findData( dir ) );
}

bool QgsLayoutMapGridWidget::hasPredefinedScales() const
{
  // first look at project's scales
  const QVector< double > scales = QgsProject::instance()->viewSettings()->mapScales();
  const bool hasProjectScales( QgsProject::instance()->viewSettings()->useProjectScales() );
  if ( !hasProjectScales || scales.isEmpty() )
  {
    // default to global map tool scales
    const QgsSettings settings;
    const QString scalesStr( settings.value( QStringLiteral( "Map/scales" ), Qgis::defaultProjectScales() ).toString() );
    QStringList myScalesList = scalesStr.split( ',' );
    return !myScalesList.isEmpty() && !myScalesList[0].isEmpty();
  }
  return true;
}

void QgsLayoutMapGridWidget::setGridItems()
{
  if ( !mMapGrid )
  {
    return;
  }

  mGridMarkerStyleButton->registerExpressionContextGenerator( mMapGrid );
  mGridLineStyleButton->registerExpressionContextGenerator( mMapGrid );
  mAnnotationFontButton->registerExpressionContextGenerator( mMapGrid );

  mEnabledCheckBox->setChecked( mMapGrid->enabled() );
  mIntervalXSpinBox->setValue( mMapGrid->intervalX() );
  mIntervalYSpinBox->setValue( mMapGrid->intervalY() );
  mOffsetXSpinBox->setValue( mMapGrid->offsetX() );
  mOffsetYSpinBox->setValue( mMapGrid->offsetY() );
  mCrossWidthSpinBox->setValue( mMapGrid->crossLength() );
  mFrameWidthSpinBox->setValue( mMapGrid->frameWidth() );
  mGridFrameMarginSpinBox->setValue( mMapGrid->frameMargin() );
  mGridFramePenSizeSpinBox->setValue( mMapGrid->framePenSize() );
  mGridFramePenColorButton->setColor( mMapGrid->framePenColor() );
  mGridFrameFill1ColorButton->setColor( mMapGrid->frameFillColor1() );
  mGridFrameFill2ColorButton->setColor( mMapGrid->frameFillColor2() );

  const QgsLayoutItemMapGrid::GridStyle gridStyle = mMapGrid->style();
  mGridTypeComboBox->setCurrentIndex( mGridTypeComboBox->findData( gridStyle ) );
  switch ( gridStyle )
  {
    case QgsLayoutItemMapGrid::Cross:
      mCrossWidthSpinBox->setVisible( true );
      mCrossWidthDDBtn->setVisible( true );
      mCrossWidthLabel->setVisible( true );
      mGridLineStyleButton->setVisible( true );
      mLineStyleLabel->setVisible( true );
      mGridMarkerStyleButton->setVisible( false );
      mMarkerStyleLabel->setVisible( false );
      mGridBlendComboBox->setVisible( true );
      mGridBlendLabel->setVisible( true );
      break;
    case QgsLayoutItemMapGrid::Markers:
      mCrossWidthSpinBox->setVisible( false );
      mCrossWidthDDBtn->setVisible( false );
      mCrossWidthLabel->setVisible( false );
      mGridLineStyleButton->setVisible( false );
      mLineStyleLabel->setVisible( false );
      mGridMarkerStyleButton->setVisible( true );
      mMarkerStyleFrame->setVisible( true );
      mMarkerStyleLabel->setVisible( true );
      mGridBlendComboBox->setVisible( true );
      mGridBlendLabel->setVisible( true );
      break;
    case QgsLayoutItemMapGrid::Solid:
      mCrossWidthSpinBox->setVisible( false );
      mCrossWidthDDBtn->setVisible( false );
      mCrossWidthLabel->setVisible( false );
      mGridLineStyleButton->setVisible( true );
      mLineStyleLabel->setVisible( true );
      mGridMarkerStyleButton->setVisible( false );
      mMarkerStyleFrame->setVisible( false );
      mMarkerStyleLabel->setVisible( false );
      mGridBlendComboBox->setVisible( true );
      mGridBlendLabel->setVisible( true );
      break;
    case QgsLayoutItemMapGrid::FrameAnnotationsOnly:
      mCrossWidthSpinBox->setVisible( false );
      mCrossWidthDDBtn->setVisible( false );
      mCrossWidthLabel->setVisible( false );
      mGridLineStyleButton->setVisible( false );
      mLineStyleLabel->setVisible( false );
      mGridMarkerStyleButton->setVisible( false );
      mMarkerStyleFrame->setVisible( false );
      mMarkerStyleLabel->setVisible( false );
      mGridBlendComboBox->setVisible( false );
      mGridBlendLabel->setVisible( false );
      break;
  }

  //grid frame
  mFrameWidthSpinBox->setValue( mMapGrid->frameWidth() );
  mGridFrameMarginSpinBox->setValue( mMapGrid->frameMargin() );
  const QgsLayoutItemMapGrid::FrameStyle gridFrameStyle = mMapGrid->frameStyle();
  mFrameStyleComboBox->setCurrentIndex( mFrameStyleComboBox->findData( gridFrameStyle ) );
  switch ( gridFrameStyle )
  {
    case QgsLayoutItemMapGrid::Zebra:
    case QgsLayoutItemMapGrid::ZebraNautical:
      toggleFrameControls( true, true, true, false );
      break;
    case QgsLayoutItemMapGrid::InteriorTicks:
    case QgsLayoutItemMapGrid::ExteriorTicks:
    case QgsLayoutItemMapGrid::InteriorExteriorTicks:
      toggleFrameControls( true, false, true, true );
      break;
    case QgsLayoutItemMapGrid::LineBorder:
    case QgsLayoutItemMapGrid::LineBorderNautical:
      toggleFrameControls( true, false, false, false );
      break;
    case QgsLayoutItemMapGrid::NoFrame:
      toggleFrameControls( false, false, false, false );
      break;
  }

  mCheckGridLeftSide->setChecked( mMapGrid->testFrameSideFlag( QgsLayoutItemMapGrid::FrameLeft ) );
  mCheckGridRightSide->setChecked( mMapGrid->testFrameSideFlag( QgsLayoutItemMapGrid::FrameRight ) );
  mCheckGridTopSide->setChecked( mMapGrid->testFrameSideFlag( QgsLayoutItemMapGrid::FrameTop ) );
  mCheckGridBottomSide->setChecked( mMapGrid->testFrameSideFlag( QgsLayoutItemMapGrid::FrameBottom ) );

  mRotatedTicksGroupBox->setChecked( mMapGrid->rotatedTicksEnabled() );
  mRotatedTicksLengthModeComboBox->setCurrentIndex( mRotatedTicksLengthModeComboBox->findData( mMapGrid->rotatedTicksLengthMode() ) );
  mRotatedTicksThresholdSpinBox->setValue( mMapGrid->rotatedTicksMinimumAngle() );
  mRotatedTicksMarginToCornerSpinBox->setValue( mMapGrid->rotatedTicksMarginToCorner() );

  mRotatedAnnotationsGroupBox->setChecked( mMapGrid->rotatedAnnotationsEnabled() );
  mRotatedAnnotationsLengthModeComboBox->setCurrentIndex( mRotatedAnnotationsLengthModeComboBox->findData( mMapGrid->rotatedAnnotationsLengthMode() ) );
  mRotatedAnnotationsThresholdSpinBox->setValue( mMapGrid->rotatedAnnotationsMinimumAngle() );
  mRotatedAnnotationsMarginToCornerSpinBox->setValue( mMapGrid->rotatedAnnotationsMarginToCorner() );

  initFrameDisplayBox( mFrameDivisionsLeftComboBox, mMapGrid->frameDivisions( QgsLayoutItemMapGrid::Left ) );
  initFrameDisplayBox( mFrameDivisionsRightComboBox, mMapGrid->frameDivisions( QgsLayoutItemMapGrid::Right ) );
  initFrameDisplayBox( mFrameDivisionsTopComboBox, mMapGrid->frameDivisions( QgsLayoutItemMapGrid::Top ) );
  initFrameDisplayBox( mFrameDivisionsBottomComboBox, mMapGrid->frameDivisions( QgsLayoutItemMapGrid::Bottom ) );

  //line style
  mGridLineStyleButton->setSymbol( mMapGrid->lineSymbol()->clone() );
  //marker style
  mGridMarkerStyleButton->setSymbol( mMapGrid->markerSymbol()->clone() );

  mGridBlendComboBox->setBlendMode( mMapGrid->blendMode() );

  mDrawAnnotationGroupBox->setChecked( mMapGrid->annotationEnabled() );
  mAnnotationDisplayLeftComboBox->setCurrentIndex( mAnnotationDisplayLeftComboBox->findData( mMapGrid->annotationDisplay( QgsLayoutItemMapGrid::Left ) ) );
  mAnnotationDisplayRightComboBox->setCurrentIndex( mAnnotationDisplayRightComboBox->findData( mMapGrid->annotationDisplay( QgsLayoutItemMapGrid::Right ) ) );
  mAnnotationDisplayTopComboBox->setCurrentIndex( mAnnotationDisplayTopComboBox->findData( mMapGrid->annotationDisplay( QgsLayoutItemMapGrid::Top ) ) );
  mAnnotationDisplayBottomComboBox->setCurrentIndex( mAnnotationDisplayBottomComboBox->findData( mMapGrid->annotationDisplay( QgsLayoutItemMapGrid::Bottom ) ) );

  mAnnotationPositionLeftComboBox->setCurrentIndex( mAnnotationPositionLeftComboBox->findData( mMapGrid->annotationPosition( QgsLayoutItemMapGrid::Left ) ) );
  mAnnotationPositionRightComboBox->setCurrentIndex( mAnnotationPositionRightComboBox->findData( mMapGrid->annotationPosition( QgsLayoutItemMapGrid::Right ) ) );
  mAnnotationPositionTopComboBox->setCurrentIndex( mAnnotationPositionTopComboBox->findData( mMapGrid->annotationPosition( QgsLayoutItemMapGrid::Top ) ) );
  mAnnotationPositionBottomComboBox->setCurrentIndex( mAnnotationPositionBottomComboBox->findData( mMapGrid->annotationPosition( QgsLayoutItemMapGrid::Bottom ) ) );

  initAnnotationDirectionBox( mAnnotationDirectionComboBoxLeft, mMapGrid->annotationDirection( QgsLayoutItemMapGrid::Left ) );
  initAnnotationDirectionBox( mAnnotationDirectionComboBoxRight, mMapGrid->annotationDirection( QgsLayoutItemMapGrid::Right ) );
  initAnnotationDirectionBox( mAnnotationDirectionComboBoxTop, mMapGrid->annotationDirection( QgsLayoutItemMapGrid::Top ) );
  initAnnotationDirectionBox( mAnnotationDirectionComboBoxBottom, mMapGrid->annotationDirection( QgsLayoutItemMapGrid::Bottom ) );

  mAnnotationFontButton->setDialogTitle( tr( "Grid Annotation Font" ) );
  mAnnotationFontButton->setMode( QgsFontButton::ModeTextRenderer );
  mAnnotationFontButton->setTextFormat( mMapGrid->annotationTextFormat() );

  mAnnotationFormatComboBox->setCurrentIndex( mAnnotationFormatComboBox->findData( mMapGrid->annotationFormat() ) );
  mAnnotationFormatButton->setEnabled( mMapGrid->annotationFormat() == QgsLayoutItemMapGrid::CustomFormat );
  mDistanceToMapFrameSpinBox->setValue( mMapGrid->annotationFrameDistance() );
  mCoordinatePrecisionSpinBox->setValue( mMapGrid->annotationPrecision() );

  //Unit
  mMapGridUnitComboBox->setCurrentIndex( mMapGridUnitComboBox->findData( mMapGrid->units() ) );
  switch ( mMapGrid->units() )
  {
    case QgsLayoutItemMapGrid::MapUnit:
    case QgsLayoutItemMapGrid::MM:
    case QgsLayoutItemMapGrid::CM:
      mIntervalStackedWidget->setCurrentIndex( 0 );
      break;

    case QgsLayoutItemMapGrid::DynamicPageSizeBased:
      mIntervalStackedWidget->setCurrentIndex( 1 );
      break;
  }
  mMinWidthSpinBox->setValue( mMapGrid->minimumIntervalWidth() );
  mMaxWidthSpinBox->setValue( mMapGrid->maximumIntervalWidth() );

  whileBlocking( mMapGridCrsSelector )->setCrs( mMapGrid->crs() );
}

void QgsLayoutMapGridWidget::mIntervalXSpinBox_editingFinished()
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Grid Interval" ) );
  mMapGrid->setIntervalX( mIntervalXSpinBox->value() );
  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mIntervalYSpinBox_editingFinished()
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Grid Interval" ) );
  mMapGrid->setIntervalY( mIntervalYSpinBox->value() );
  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mOffsetXSpinBox_valueChanged( double value )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Grid Offset" ) );
  mMapGrid->setOffsetX( value );
  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mOffsetYSpinBox_valueChanged( double value )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Grid Offset" ) );
  mMapGrid->setOffsetY( value );
  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mCrossWidthSpinBox_valueChanged( double val )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Cross Width" ) );
  mMapGrid->setCrossLength( val );
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mFrameWidthSpinBox_valueChanged( double val )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Frame Width" ) );
  mMapGrid->setFrameWidth( val );
  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mGridFrameMarginSpinBox_valueChanged( double val )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Grid Frame Margin" ) );
  mMapGrid->setFrameMargin( val );
  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mCheckGridLeftSide_toggled( bool checked )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Frame Left" ) );
  mMapGrid->setFrameSideFlag( QgsLayoutItemMapGrid::FrameLeft, checked );
  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mCheckGridRightSide_toggled( bool checked )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Frame Right" ) );
  mMapGrid->setFrameSideFlag( QgsLayoutItemMapGrid::FrameRight, checked );
  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mCheckGridTopSide_toggled( bool checked )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Frame Top" ) );
  mMapGrid->setFrameSideFlag( QgsLayoutItemMapGrid::FrameTop, checked );
  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mCheckGridBottomSide_toggled( bool checked )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Frame Bottom" ) );
  mMapGrid->setFrameSideFlag( QgsLayoutItemMapGrid::FrameBottom, checked );
  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mFrameDivisionsLeftComboBox_currentIndexChanged( int index )
{
  handleChangedFrameDisplay( QgsLayoutItemMapGrid::Left, static_cast< QgsLayoutItemMapGrid::DisplayMode >( mFrameDivisionsLeftComboBox->itemData( index ).toInt() ) );
}

void QgsLayoutMapGridWidget::mFrameDivisionsRightComboBox_currentIndexChanged( int index )
{
  handleChangedFrameDisplay( QgsLayoutItemMapGrid::Right, static_cast< QgsLayoutItemMapGrid::DisplayMode >( mFrameDivisionsRightComboBox->itemData( index ).toInt() ) );
}

void QgsLayoutMapGridWidget::mFrameDivisionsTopComboBox_currentIndexChanged( int index )
{
  handleChangedFrameDisplay( QgsLayoutItemMapGrid::Top, static_cast< QgsLayoutItemMapGrid::DisplayMode >( mFrameDivisionsTopComboBox->itemData( index ).toInt() ) );
}

void QgsLayoutMapGridWidget::mFrameDivisionsBottomComboBox_currentIndexChanged( int index )
{
  handleChangedFrameDisplay( QgsLayoutItemMapGrid::Bottom, static_cast< QgsLayoutItemMapGrid::DisplayMode >( mFrameDivisionsBottomComboBox->itemData( index ).toInt() ) );
}

void QgsLayoutMapGridWidget::mGridFramePenSizeSpinBox_valueChanged( double d )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Frame Thickness" ) );
  mMapGrid->setFramePenSize( d );
  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mGridFramePenColorButton_colorChanged( const QColor &newColor )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Frame Color" ), QgsLayoutItem::UndoGridFramePenColor );
  mMapGrid->setFramePenColor( newColor );
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mGridFrameFill1ColorButton_colorChanged( const QColor &newColor )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Frame Fill Color" ), QgsLayoutItem::UndoMapGridFrameFill1Color );
  mMapGrid->setFrameFillColor1( newColor );
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mGridFrameFill2ColorButton_colorChanged( const QColor &newColor )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Frame Fill Color" ), QgsLayoutItem::UndoMapGridFrameFill2Color );
  mMapGrid->setFrameFillColor2( newColor );
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mFrameStyleComboBox_currentIndexChanged( int )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  const QgsLayoutItemMapGrid::FrameStyle style = static_cast< QgsLayoutItemMapGrid::FrameStyle >( mFrameStyleComboBox->currentData().toInt() );
  mMap->beginCommand( tr( "Change Frame Style" ) );
  mMapGrid->setFrameStyle( style );
  switch ( style )
  {
    case QgsLayoutItemMapGrid::Zebra:
    case QgsLayoutItemMapGrid::ZebraNautical:
      toggleFrameControls( true, true, true, false );
      break;
    case QgsLayoutItemMapGrid::InteriorTicks:
    case QgsLayoutItemMapGrid::ExteriorTicks:
    case QgsLayoutItemMapGrid::InteriorExteriorTicks:
      toggleFrameControls( true, false, true, true );
      break;
    case QgsLayoutItemMapGrid::LineBorder:
    case QgsLayoutItemMapGrid::LineBorderNautical:
      toggleFrameControls( true, false, false, false );
      break;
    case QgsLayoutItemMapGrid::NoFrame:
      toggleFrameControls( false, false, false, false );
      break;
  }
  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mRotatedTicksGroupBox_toggled( bool state )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Tick Rotation Enabled" ) );
  mMapGrid->setRotatedTicksEnabled( state );
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mRotatedTicksLengthModeComboBox_currentIndexChanged( int )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  const QgsLayoutItemMapGrid::TickLengthMode mode = static_cast< QgsLayoutItemMapGrid::TickLengthMode >( mRotatedTicksLengthModeComboBox->currentData().toInt() );
  mMap->beginCommand( tr( "Change Tick Length Mode" ) );
  mMapGrid->setRotatedTicksLengthMode( mode );
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mRotatedTicksThresholdSpinBox_valueChanged( double val )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Rotated Ticks Threshold" ) );
  mMapGrid->setRotatedTicksMinimumAngle( val );
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mRotatedTicksMarginToCornerSpinBox_valueChanged( double val )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Rotated Ticks Margin to Corner" ) );
  mMapGrid->setRotatedTicksMarginToCorner( val );
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mRotatedAnnotationsGroupBox_toggled( bool state )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Annotation Rotation Enabled" ) );
  mMapGrid->setRotatedAnnotationsEnabled( state );
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mRotatedAnnotationsLengthModeComboBox_currentIndexChanged( int )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  const QgsLayoutItemMapGrid::TickLengthMode mode = static_cast< QgsLayoutItemMapGrid::TickLengthMode >( mRotatedAnnotationsLengthModeComboBox->currentData().toInt() );
  mMap->beginCommand( tr( "Change Annotation Length Mode" ) );
  mMapGrid->setRotatedAnnotationsLengthMode( mode );
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mRotatedAnnotationsThresholdSpinBox_valueChanged( double val )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Rotated Annotations Threshold" ) );
  mMapGrid->setRotatedAnnotationsMinimumAngle( val );
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mRotatedAnnotationsMarginToCornerSpinBox_valueChanged( double val )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Rotated Annotations Margin to Corner" ) );
  mMapGrid->setRotatedAnnotationsMarginToCorner( val );
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::intervalUnitChanged( int )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  const QgsLayoutItemMapGrid::GridUnit unit = static_cast< QgsLayoutItemMapGrid::GridUnit >( mMapGridUnitComboBox->currentData().toInt() );
  switch ( unit )
  {
    case QgsLayoutItemMapGrid::MapUnit:
    case QgsLayoutItemMapGrid::MM:
    case QgsLayoutItemMapGrid::CM:
      mIntervalStackedWidget->setCurrentIndex( 0 );
      break;

    case QgsLayoutItemMapGrid::DynamicPageSizeBased:
      mIntervalStackedWidget->setCurrentIndex( 1 );
      break;
  }

  mMap->beginCommand( tr( "Change Grid Unit" ) );
  mMapGrid->setUnits( unit );
  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::minIntervalChanged( double interval )
{
  mMap->beginCommand( tr( "Change Grid Interval Range" ), QgsLayoutItem::UndoMapGridIntervalRange );
  mMapGrid->setMinimumIntervalWidth( interval );
  mMap->endCommand();
  mMap->updateBoundingRect();
  mMap->update();
}

void QgsLayoutMapGridWidget::maxIntervalChanged( double interval )
{
  mMap->beginCommand( tr( "Change Grid Interval Range" ), QgsLayoutItem::UndoMapGridIntervalRange );
  mMapGrid->setMaximumIntervalWidth( interval );
  mMap->endCommand();
  mMap->updateBoundingRect();
  mMap->update();
}

void QgsLayoutMapGridWidget::annotationTextFormatChanged()
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Annotation Font" ) );
  mMapGrid->setAnnotationTextFormat( mAnnotationFontButton->textFormat() );
  mMap->endCommand();
  mMap->updateBoundingRect();
  mMap->update();
}

void QgsLayoutMapGridWidget::onCrsChanged()
{
  mBlockAnnotationFormatUpdates++;
  const QgsLayoutItemMapGrid::AnnotationFormat prevFormat = static_cast< QgsLayoutItemMapGrid::AnnotationFormat  >( mAnnotationFormatComboBox->currentData().toInt() );

  mAnnotationFormatComboBox->clear();
  mAnnotationFormatComboBox->addItem( tr( "Decimal" ), QgsLayoutItemMapGrid::Decimal );
  mAnnotationFormatComboBox->addItem( tr( "Decimal with Suffix" ), QgsLayoutItemMapGrid::DecimalWithSuffix );

  // only show degree based options for geographic CRSes
  const QgsCoordinateReferenceSystem crs = mMapGrid->crs().isValid() ? mMapGrid->crs() : mMap->crs();
  switch ( crs.mapUnits() )
  {
    case QgsUnitTypes::DistanceMeters:
    case QgsUnitTypes::DistanceKilometers:
    case QgsUnitTypes::DistanceFeet:
    case QgsUnitTypes::DistanceNauticalMiles:
    case QgsUnitTypes::DistanceYards:
    case QgsUnitTypes::DistanceMiles:
    case QgsUnitTypes::DistanceCentimeters:
    case QgsUnitTypes::DistanceMillimeters:
      break;

    case QgsUnitTypes::DistanceDegrees:
    case QgsUnitTypes::DistanceUnknownUnit:
      mAnnotationFormatComboBox->addItem( tr( "Degree, Minute" ), QgsLayoutItemMapGrid::DegreeMinuteNoSuffix );
      mAnnotationFormatComboBox->addItem( tr( "Degree, Minute with Suffix" ), QgsLayoutItemMapGrid::DegreeMinute );
      mAnnotationFormatComboBox->addItem( tr( "Degree, Minute Aligned" ), QgsLayoutItemMapGrid::DegreeMinutePadded );
      mAnnotationFormatComboBox->addItem( tr( "Degree, Minute, Second" ), QgsLayoutItemMapGrid::DegreeMinuteSecondNoSuffix );
      mAnnotationFormatComboBox->addItem( tr( "Degree, Minute, Second with Suffix" ), QgsLayoutItemMapGrid::DegreeMinuteSecond );
      mAnnotationFormatComboBox->addItem( tr( "Degree, Minute, Second Aligned" ), QgsLayoutItemMapGrid::DegreeMinuteSecondPadded );
      break;
  }
  mAnnotationFormatComboBox->addItem( tr( "Custom" ), QgsLayoutItemMapGrid::CustomFormat );

  const int prevIndex = mAnnotationFormatComboBox->findData( prevFormat );
  if ( prevIndex >= 0 )
    mAnnotationFormatComboBox->setCurrentIndex( prevIndex );
  else
    mAnnotationFormatComboBox->setCurrentIndex( 0 );
  mBlockAnnotationFormatUpdates--;

  const QgsLayoutItemMapGrid::AnnotationFormat newFormat = static_cast< QgsLayoutItemMapGrid::AnnotationFormat  >( mAnnotationFormatComboBox->currentData().toInt() );
  if ( newFormat != prevFormat )
  {
    mAnnotationFormatComboBox_currentIndexChanged( mAnnotationFormatComboBox->currentIndex() );
  }
}

void QgsLayoutMapGridWidget::mGridBlendComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index )
  if ( mMapGrid )
  {
    mMap->beginCommand( tr( "Change Grid Blend Mode" ) );
    mMapGrid->setBlendMode( mGridBlendComboBox->blendMode() );
    mMap->update();
    mMap->endCommand();
  }

}

void QgsLayoutMapGridWidget::mGridTypeComboBox_currentIndexChanged( int )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Grid Type" ) );
  switch ( static_cast< QgsLayoutItemMapGrid::GridStyle >( mGridTypeComboBox->currentData().toInt() ) )
  {
    case QgsLayoutItemMapGrid::Cross:
      mMapGrid->setStyle( QgsLayoutItemMapGrid::Cross );
      mCrossWidthSpinBox->setVisible( true );
      mCrossWidthDDBtn->setVisible( true );
      mCrossWidthLabel->setVisible( true );
      mGridLineStyleButton->setVisible( true );
      mLineStyleLabel->setVisible( true );
      mGridMarkerStyleButton->setVisible( false );
      mMarkerStyleFrame->setVisible( false );
      mMarkerStyleFrame->setVisible( false );
      mMarkerStyleLabel->setVisible( false );
      mGridBlendComboBox->setVisible( true );
      mGridBlendLabel->setVisible( true );
      break;

    case QgsLayoutItemMapGrid::Markers:
      mMapGrid->setStyle( QgsLayoutItemMapGrid::Markers );
      mCrossWidthSpinBox->setVisible( false );
      mCrossWidthDDBtn->setVisible( false );
      mCrossWidthLabel->setVisible( false );
      mGridLineStyleButton->setVisible( false );
      mLineStyleLabel->setVisible( false );
      mGridMarkerStyleButton->setVisible( true );
      mMarkerStyleFrame->setVisible( true );
      mMarkerStyleLabel->setVisible( true );
      mGridBlendComboBox->setVisible( true );
      mGridBlendLabel->setVisible( true );
      break;

    case QgsLayoutItemMapGrid::Solid:
      mMapGrid->setStyle( QgsLayoutItemMapGrid::Solid );
      mCrossWidthSpinBox->setVisible( false );
      mCrossWidthDDBtn->setVisible( false );
      mCrossWidthLabel->setVisible( false );
      mGridLineStyleButton->setVisible( true );
      mLineStyleLabel->setVisible( true );
      mGridMarkerStyleButton->setVisible( false );
      mMarkerStyleFrame->setVisible( false );
      mMarkerStyleLabel->setVisible( false );
      mGridBlendComboBox->setVisible( true );
      mGridBlendLabel->setVisible( true );
      break;

    case QgsLayoutItemMapGrid::FrameAnnotationsOnly:
      mMapGrid->setStyle( QgsLayoutItemMapGrid::FrameAnnotationsOnly );
      mCrossWidthSpinBox->setVisible( false );
      mCrossWidthDDBtn->setVisible( false );
      mCrossWidthLabel->setVisible( false );
      mGridLineStyleButton->setVisible( false );
      mLineStyleLabel->setVisible( false );
      mGridMarkerStyleButton->setVisible( false );
      mMarkerStyleFrame->setVisible( false );
      mMarkerStyleLabel->setVisible( false );
      mGridBlendComboBox->setVisible( false );
      mGridBlendLabel->setVisible( false );
      break;
  }

  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mapGridCrsChanged( const QgsCoordinateReferenceSystem &crs )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  if ( mMapGrid->crs() == crs )
    return;

  mMap->beginCommand( tr( "Change Grid CRS" ) );
  mMapGrid->setCrs( crs );
  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mDrawAnnotationGroupBox_toggled( bool state )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Toggle Annotations" ) );
  mMapGrid->setAnnotationEnabled( state );
  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mAnnotationFormatButton_clicked()
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  QgsExpressionContext expressionContext = mMapGrid->createExpressionContext();
  expressionContext.setHighlightedFunctions( QStringList() << QStringLiteral( "to_dms" ) << QStringLiteral( "to_dm" ) );

  QgsExpressionBuilderDialog exprDlg( coverageLayer(), mMapGrid->annotationExpression(), this, QStringLiteral( "generic" ), expressionContext );
  exprDlg.setWindowTitle( tr( "Expression Based Annotation" ) );

  if ( exprDlg.exec() == QDialog::Accepted )
  {
    const QString expression = exprDlg.expressionText();
    mMap->beginCommand( tr( "Change Annotation Format" ) );
    mMapGrid->setAnnotationExpression( expression );
    mMap->updateBoundingRect();
    mMap->update();
    mMap->endCommand();
  }
}

void QgsLayoutMapGridWidget::mAnnotationDisplayLeftComboBox_currentIndexChanged( int )
{
  handleChangedAnnotationDisplay( QgsLayoutItemMapGrid::Left, static_cast< QgsLayoutItemMapGrid::DisplayMode >( mAnnotationDisplayLeftComboBox->currentData().toInt() ) );
}

void QgsLayoutMapGridWidget::mAnnotationDisplayRightComboBox_currentIndexChanged( int )
{
  handleChangedAnnotationDisplay( QgsLayoutItemMapGrid::Right, static_cast< QgsLayoutItemMapGrid::DisplayMode >( mAnnotationDisplayRightComboBox->currentData().toInt() ) );
}

void QgsLayoutMapGridWidget::mAnnotationDisplayTopComboBox_currentIndexChanged( int )
{
  handleChangedAnnotationDisplay( QgsLayoutItemMapGrid::Top, static_cast< QgsLayoutItemMapGrid::DisplayMode >( mAnnotationDisplayTopComboBox->currentData().toInt() ) );
}

void QgsLayoutMapGridWidget::mAnnotationDisplayBottomComboBox_currentIndexChanged( int )
{
  handleChangedAnnotationDisplay( QgsLayoutItemMapGrid::Bottom, static_cast< QgsLayoutItemMapGrid::DisplayMode >( mAnnotationDisplayBottomComboBox->currentData().toInt() ) );
}

void QgsLayoutMapGridWidget::mAnnotationPositionLeftComboBox_currentIndexChanged( int )
{
  handleChangedAnnotationPosition( QgsLayoutItemMapGrid::Left, static_cast< QgsLayoutItemMapGrid::AnnotationPosition >( mAnnotationPositionLeftComboBox->currentData().toInt() ) );
}

void QgsLayoutMapGridWidget::mAnnotationPositionRightComboBox_currentIndexChanged( int )
{
  handleChangedAnnotationPosition( QgsLayoutItemMapGrid::Right, static_cast< QgsLayoutItemMapGrid::AnnotationPosition >( mAnnotationPositionRightComboBox->currentData().toInt() ) );
}

void QgsLayoutMapGridWidget::mAnnotationPositionTopComboBox_currentIndexChanged( int )
{
  handleChangedAnnotationPosition( QgsLayoutItemMapGrid::Top, static_cast< QgsLayoutItemMapGrid::AnnotationPosition >( mAnnotationPositionTopComboBox->currentData().toInt() ) );
}

void QgsLayoutMapGridWidget::mAnnotationPositionBottomComboBox_currentIndexChanged( int )
{
  handleChangedAnnotationPosition( QgsLayoutItemMapGrid::Bottom, static_cast< QgsLayoutItemMapGrid::AnnotationPosition >( mAnnotationPositionBottomComboBox->currentData().toInt() ) );
}

void QgsLayoutMapGridWidget::mAnnotationDirectionComboBoxLeft_currentIndexChanged( int index )
{
  handleChangedAnnotationDirection( QgsLayoutItemMapGrid::Left, static_cast< QgsLayoutItemMapGrid::AnnotationDirection >( mAnnotationDirectionComboBoxLeft->itemData( index ).toInt() ) );
}

void QgsLayoutMapGridWidget::mAnnotationDirectionComboBoxRight_currentIndexChanged( int index )
{
  handleChangedAnnotationDirection( QgsLayoutItemMapGrid::Right, static_cast< QgsLayoutItemMapGrid::AnnotationDirection >( mAnnotationDirectionComboBoxRight->itemData( index ).toInt() ) );
}

void QgsLayoutMapGridWidget::mAnnotationDirectionComboBoxTop_currentIndexChanged( int index )
{
  handleChangedAnnotationDirection( QgsLayoutItemMapGrid::Top, static_cast< QgsLayoutItemMapGrid::AnnotationDirection >( mAnnotationDirectionComboBoxTop->itemData( index ).toInt() ) );
}

void QgsLayoutMapGridWidget::mAnnotationDirectionComboBoxBottom_currentIndexChanged( int index )
{
  handleChangedAnnotationDirection( QgsLayoutItemMapGrid::Bottom, static_cast< QgsLayoutItemMapGrid::AnnotationDirection >( mAnnotationDirectionComboBoxBottom->itemData( index ).toInt() ) );
}

void QgsLayoutMapGridWidget::mDistanceToMapFrameSpinBox_valueChanged( double d )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Annotation Distance" ), QgsLayoutItem::UndoMapAnnotationDistance );
  mMapGrid->setAnnotationFrameDistance( d );
  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::lineSymbolChanged()
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Grid Line Style" ), QgsLayoutItem::UndoMapGridLineSymbol );
  mMapGrid->setLineSymbol( mGridLineStyleButton->clonedSymbol<QgsLineSymbol>() );
  mMap->endCommand();
  mMap->update();
}

void QgsLayoutMapGridWidget::markerSymbolChanged()
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Grid Marker Style" ), QgsLayoutItem::UndoMapGridMarkerSymbol );
  mMapGrid->setMarkerSymbol( mGridMarkerStyleButton->clonedSymbol<QgsMarkerSymbol>() );
  mMap->endCommand();
  mMap->update();
}

void QgsLayoutMapGridWidget::gridEnabledToggled( bool active )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Toggle Grid Display" ) );
  mMapGrid->setEnabled( active );
  mMap->endCommand();
  mMap->updateBoundingRect();
  mMap->update();
}

void QgsLayoutMapGridWidget::mAnnotationFormatComboBox_currentIndexChanged( int index )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }
  if ( mBlockAnnotationFormatUpdates )
    return;

  mMap->beginCommand( tr( "Change Annotation Format" ) );

  mMapGrid->setAnnotationFormat( static_cast< QgsLayoutItemMapGrid::AnnotationFormat >( mAnnotationFormatComboBox->itemData( index ).toInt() ) );
  mAnnotationFormatButton->setEnabled( mMapGrid->annotationFormat() == QgsLayoutItemMapGrid::CustomFormat );

  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mCoordinatePrecisionSpinBox_valueChanged( int value )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }
  mMap->beginCommand( tr( "Change Annotation Precision" ) );
  mMapGrid->setAnnotationPrecision( value );
  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}
