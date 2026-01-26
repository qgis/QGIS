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

#include "qgsexpressionbuilderdialog.h"
#include "qgslayout.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutreportcontext.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"
#include "qgsproject.h"
#include "qgsprojectviewsettings.h"
#include "qgssettingsregistrycore.h"
#include "qgssymbol.h"
#include "qgsvectorlayer.h"

#include "moc_qgslayoutmapgridwidget.cpp"

QgsLayoutMapGridWidget::QgsLayoutMapGridWidget( QgsLayoutItemMapGrid *mapGrid, QgsLayoutItemMap *map )
  : QgsLayoutItemBaseWidget( nullptr, mapGrid )
  , mMap( map )
  , mMapGrid( mapGrid )
{
  setupUi( this );

  mFrameStyleComboBox->addItem( tr( "No Frame" ), QVariant::fromValue( Qgis::MapGridFrameStyle::NoFrame ) );
  mFrameStyleComboBox->addItem( tr( "Zebra" ), QVariant::fromValue( Qgis::MapGridFrameStyle::Zebra ) );
  mFrameStyleComboBox->addItem( tr( "Zebra (Nautical)" ), QVariant::fromValue( Qgis::MapGridFrameStyle::ZebraNautical ) );
  mFrameStyleComboBox->addItem( tr( "Interior Ticks" ), QVariant::fromValue( Qgis::MapGridFrameStyle::InteriorTicks ) );
  mFrameStyleComboBox->addItem( tr( "Exterior Ticks" ), QVariant::fromValue( Qgis::MapGridFrameStyle::ExteriorTicks ) );
  mFrameStyleComboBox->addItem( tr( "Interior and Exterior Ticks" ), QVariant::fromValue( Qgis::MapGridFrameStyle::InteriorExteriorTicks ) );
  mFrameStyleComboBox->addItem( tr( "Line Border" ), QVariant::fromValue( Qgis::MapGridFrameStyle::LineBorder ) );
  mFrameStyleComboBox->addItem( tr( "Line Border (Nautical)" ), QVariant::fromValue( Qgis::MapGridFrameStyle::LineBorderNautical ) );

  mRotatedTicksLengthModeComboBox->addItem( tr( "Orthogonal" ), QVariant::fromValue( Qgis::MapGridTickLengthMode::OrthogonalTicks ) );
  mRotatedTicksLengthModeComboBox->addItem( tr( "Fixed Length" ), QVariant::fromValue( Qgis::MapGridTickLengthMode::NormalizedTicks ) );
  mRotatedAnnotationsLengthModeComboBox->addItem( tr( "Orthogonal" ), QVariant::fromValue( Qgis::MapGridTickLengthMode::OrthogonalTicks ) );
  mRotatedAnnotationsLengthModeComboBox->addItem( tr( "Fixed Length" ), QVariant::fromValue( Qgis::MapGridTickLengthMode::NormalizedTicks ) );

  mGridFrameMarginSpinBox->setShowClearButton( true );
  mGridFrameMarginSpinBox->setClearValue( 0 );

  mDistanceToMapFrameSpinBox->setShowClearButton( true );
  mDistanceToMapFrameSpinBox->setClearValue( 0 );

  mOffsetXSpinBox->setShowClearButton( true );
  mOffsetXSpinBox->setClearValue( 0 );
  mOffsetYSpinBox->setShowClearButton( true );
  mOffsetYSpinBox->setClearValue( 0 );

  connect( mIntervalXSpinBox, &QgsDoubleSpinBox::editingFinished, this, &QgsLayoutMapGridWidget::mIntervalXSpinBox_editingFinished );
  connect( mIntervalYSpinBox, &QgsDoubleSpinBox::editingFinished, this, &QgsLayoutMapGridWidget::mIntervalYSpinBox_editingFinished );
  connect( mOffsetXSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mOffsetXSpinBox_valueChanged );
  connect( mOffsetYSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mOffsetYSpinBox_valueChanged );
  connect( mCrossWidthSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mCrossWidthSpinBox_valueChanged );
  connect( mFrameWidthSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mFrameWidthSpinBox_valueChanged );
  connect( mGridFrameMarginSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mGridFrameMarginSpinBox_valueChanged );
  connect( mFrameStyleComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mFrameStyleComboBox_currentIndexChanged );
  connect( mRotatedTicksGroupBox, &QGroupBox::toggled, this, &QgsLayoutMapGridWidget::mRotatedTicksGroupBox_toggled );
  connect( mRotatedTicksLengthModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mRotatedTicksLengthModeComboBox_currentIndexChanged );
  connect( mRotatedTicksThresholdSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mRotatedTicksThresholdSpinBox_valueChanged );
  connect( mRotatedTicksMarginToCornerSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mRotatedTicksMarginToCornerSpinBox_valueChanged );
  connect( mRotatedAnnotationsGroupBox, &QGroupBox::toggled, this, &QgsLayoutMapGridWidget::mRotatedAnnotationsGroupBox_toggled );
  connect( mRotatedAnnotationsLengthModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mRotatedAnnotationsLengthModeComboBox_currentIndexChanged );
  connect( mRotatedAnnotationsThresholdSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mRotatedAnnotationsThresholdSpinBox_valueChanged );
  connect( mRotatedAnnotationsMarginToCornerSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mRotatedAnnotationsMarginToCornerSpinBox_valueChanged );
  connect( mGridFramePenSizeSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mGridFramePenSizeSpinBox_valueChanged );
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
  connect( mCoordinatePrecisionSpinBox, static_cast<void ( QSpinBox::* )( int )>( &QSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mCoordinatePrecisionSpinBox_valueChanged );
  connect( mDistanceToMapFrameSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mDistanceToMapFrameSpinBox_valueChanged );
  connect( mMinWidthSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::minIntervalChanged );
  connect( mMaxWidthSpinBox, static_cast<void ( QDoubleSpinBox::* )( double )>( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::maxIntervalChanged );
  connect( mEnabledCheckBox, &QCheckBox::toggled, this, &QgsLayoutMapGridWidget::gridEnabledToggled );
  setPanelTitle( tr( "Map Grid Properties" ) );

  mMapGridCrsSelector->setOptionVisible( QgsProjectionSelectionWidget::CrsNotSet, true );
  mMapGridCrsSelector->setNotSetText( tr( "Use Map CRS" ) );
  mMapGridCrsSelector->setDialogTitle( tr( "Grid CRS" ) );

  connect( mMapGridCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsLayoutMapGridWidget::mapGridCrsChanged );

  blockAllSignals( true );

  mMapGridUnitComboBox->addItem( tr( "Map Units" ), QVariant::fromValue( Qgis::MapGridUnit::MapUnits ) );
  mMapGridUnitComboBox->addItem( tr( "Fit Segment Width" ), QVariant::fromValue( Qgis::MapGridUnit::DynamicPageSizeBased ) );
  mMapGridUnitComboBox->addItem( tr( "Millimeters" ), QVariant::fromValue( Qgis::MapGridUnit::Millimeters ) );
  mMapGridUnitComboBox->addItem( tr( "Centimeters" ), QVariant::fromValue( Qgis::MapGridUnit::Centimeters ) );

  mGridTypeComboBox->insertItem( 0, tr( "Solid" ), QVariant::fromValue( Qgis::MapGridStyle::Lines ) );
  mGridTypeComboBox->insertItem( 1, tr( "Cross" ), QVariant::fromValue( Qgis::MapGridStyle::LineCrosses ) );
  mGridTypeComboBox->insertItem( 2, tr( "Markers" ), QVariant::fromValue( Qgis::MapGridStyle::Markers ) );
  mGridTypeComboBox->insertItem( 3, tr( "Frame and annotations only" ), QVariant::fromValue( Qgis::MapGridStyle::FrameAndAnnotationsOnly ) );

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
  mGridFramePenColorButton->setContext( u"composer"_s );
  mGridFramePenColorButton->setNoColorString( tr( "Transparent Frame" ) );
  mGridFramePenColorButton->setShowNoColor( true );

  mGridFrameFill1ColorButton->setColorDialogTitle( tr( "Select Grid Frame Fill Color" ) );
  mGridFrameFill1ColorButton->setAllowOpacity( true );
  mGridFrameFill1ColorButton->setContext( u"composer"_s );
  mGridFrameFill1ColorButton->setNoColorString( tr( "Transparent Fill" ) );
  mGridFrameFill1ColorButton->setShowNoColor( true );

  mGridFrameFill2ColorButton->setColorDialogTitle( tr( "Select Grid Frame Fill Color" ) );
  mGridFrameFill2ColorButton->setAllowOpacity( true );
  mGridFrameFill2ColorButton->setContext( u"composer"_s );
  mGridFrameFill2ColorButton->setNoColorString( tr( "Transparent Fill" ) );
  mGridFrameFill2ColorButton->setShowNoColor( true );

  mGridLineStyleButton->setSymbolType( Qgis::SymbolType::Line );
  mGridMarkerStyleButton->setSymbolType( Qgis::SymbolType::Marker );

  //set initial state of frame style controls
  toggleFrameControls( false, false, false, false );

  registerDataDefinedButton( mEnabledDDBtn, QgsLayoutObject::DataDefinedProperty::MapGridEnabled );
  registerDataDefinedButton( mIntervalXDDBtn, QgsLayoutObject::DataDefinedProperty::MapGridIntervalX );
  registerDataDefinedButton( mIntervalYDDBtn, QgsLayoutObject::DataDefinedProperty::MapGridIntervalY );
  registerDataDefinedButton( mOffsetXDDBtn, QgsLayoutObject::DataDefinedProperty::MapGridOffsetX );
  registerDataDefinedButton( mOffsetYDDBtn, QgsLayoutObject::DataDefinedProperty::MapGridOffsetY );
  registerDataDefinedButton( mFrameSizeDDBtn, QgsLayoutObject::DataDefinedProperty::MapGridFrameSize );
  registerDataDefinedButton( mFrameMarginDDBtn, QgsLayoutObject::DataDefinedProperty::MapGridFrameMargin );
  registerDataDefinedButton( mLabelDistDDBtn, QgsLayoutObject::DataDefinedProperty::MapGridLabelDistance );
  registerDataDefinedButton( mCrossWidthDDBtn, QgsLayoutObject::DataDefinedProperty::MapGridCrossSize );
  registerDataDefinedButton( mFrameLineThicknessDDBtn, QgsLayoutObject::DataDefinedProperty::MapGridFrameLineThickness );
  registerDataDefinedButton( mAnnotationDisplayLeftDDBtn, QgsLayoutObject::DataDefinedProperty::MapGridAnnotationDisplayLeft );
  registerDataDefinedButton( mAnnotationDisplayRightDDBtn, QgsLayoutObject::DataDefinedProperty::MapGridAnnotationDisplayRight );
  registerDataDefinedButton( mAnnotationDisplayTopDDBtn, QgsLayoutObject::DataDefinedProperty::MapGridAnnotationDisplayTop );
  registerDataDefinedButton( mAnnotationDisplayBottomDDBtn, QgsLayoutObject::DataDefinedProperty::MapGridAnnotationDisplayBottom );
  registerDataDefinedButton( mFrameDivisionsLeftDDBtn, QgsLayoutObject::DataDefinedProperty::MapGridFrameDivisionsLeft );
  registerDataDefinedButton( mFrameDivisionsRightDDBtn, QgsLayoutObject::DataDefinedProperty::MapGridFrameDivisionsRight );
  registerDataDefinedButton( mFrameDivisionsTopDDBtn, QgsLayoutObject::DataDefinedProperty::MapGridFrameDivisionsTop );
  registerDataDefinedButton( mFrameDivisionsBottomDDBtn, QgsLayoutObject::DataDefinedProperty::MapGridFrameDivisionsBottom );
  registerDataDefinedButton( mDrawAnnotationDDBtn, QgsLayoutObject::DataDefinedProperty::MapGridDrawAnnotation );

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

void QgsLayoutMapGridWidget::handleChangedFrameDisplay( Qgis::MapGridBorderSide border, const Qgis::MapGridComponentVisibility mode )
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

void QgsLayoutMapGridWidget::handleChangedAnnotationDisplay( Qgis::MapGridBorderSide border, const Qgis::MapGridComponentVisibility mode )
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
  c->insertItem( 0, tr( "Inside Frame" ), QVariant::fromValue( Qgis::MapGridAnnotationPosition::InsideMapFrame ) );
  c->insertItem( 1, tr( "Outside Frame" ), QVariant::fromValue( Qgis::MapGridAnnotationPosition::OutsideMapFrame ) );
}

void QgsLayoutMapGridWidget::insertAnnotationDirectionEntries( QComboBox *c )
{
  c->addItem( tr( "Horizontal" ), QVariant::fromValue( Qgis::MapGridAnnotationDirection::Horizontal ) );
  c->addItem( tr( "Vertical Ascending" ), QVariant::fromValue( Qgis::MapGridAnnotationDirection::Vertical ) );
  c->addItem( tr( "Vertical Descending" ), QVariant::fromValue( Qgis::MapGridAnnotationDirection::VerticalDescending ) );
  c->addItem( tr( "Boundary Direction" ), QVariant::fromValue( Qgis::MapGridAnnotationDirection::BoundaryDirection ) );
  // c->addItem( tr( "Parallel to Tick" ), QVariant::fromValue( Qgis::MapGridAnnotationDirection::ParallelToTick ) );
  c->addItem( tr( "Above Tick" ), QVariant::fromValue( Qgis::MapGridAnnotationDirection::AboveTick ) );
  c->addItem( tr( "On Tick" ), QVariant::fromValue( Qgis::MapGridAnnotationDirection::OnTick ) );
  c->addItem( tr( "Under Tick" ), QVariant::fromValue( Qgis::MapGridAnnotationDirection::UnderTick ) );
}

void QgsLayoutMapGridWidget::initFrameDisplayBox( QComboBox *c, Qgis::MapGridComponentVisibility display )
{
  if ( !c )
  {
    return;
  }
  c->setCurrentIndex( c->findData( QVariant::fromValue( display ) ) );
}

void QgsLayoutMapGridWidget::handleChangedAnnotationPosition( Qgis::MapGridBorderSide border, const Qgis::MapGridAnnotationPosition position )
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

void QgsLayoutMapGridWidget::handleChangedAnnotationDirection( Qgis::MapGridBorderSide border, Qgis::MapGridAnnotationDirection direction )
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
  c->addItem( tr( "All" ), QVariant::fromValue( Qgis::MapGridComponentVisibility::ShowAll ) );
  c->addItem( tr( "Latitude/Y Only" ), QVariant::fromValue( Qgis::MapGridComponentVisibility::LatitudeOnly ) );
  c->addItem( tr( "Longitude/X Only" ), QVariant::fromValue( Qgis::MapGridComponentVisibility::LongitudeOnly ) );
}

void QgsLayoutMapGridWidget::insertAnnotationDisplayEntries( QComboBox *c )
{
  c->insertItem( 0, tr( "Show All" ), QVariant::fromValue( Qgis::MapGridComponentVisibility::ShowAll ) );
  c->insertItem( 1, tr( "Show Latitude/Y Only" ), QVariant::fromValue( Qgis::MapGridComponentVisibility::LatitudeOnly ) );
  c->insertItem( 2, tr( "Show Longitude/X Only" ), QVariant::fromValue( Qgis::MapGridComponentVisibility::LongitudeOnly ) );
  c->insertItem( 3, tr( "Disabled" ), QVariant::fromValue( Qgis::MapGridComponentVisibility::HideAll ) );
}

void QgsLayoutMapGridWidget::initAnnotationPositionBox( QComboBox *c, Qgis::MapGridAnnotationPosition pos )
{
  if ( !c )
  {
    return;
  }

  c->setCurrentIndex( c->findData( QVariant::fromValue( pos ) ) );
}

void QgsLayoutMapGridWidget::initAnnotationDirectionBox( QComboBox *c, Qgis::MapGridAnnotationDirection dir )
{
  if ( !c )
  {
    return;
  }
  c->setCurrentIndex( c->findData( QVariant::fromValue( dir ) ) );
}

bool QgsLayoutMapGridWidget::hasPredefinedScales() const
{
  // first look at project's scales
  const QVector<double> scales = QgsProject::instance()->viewSettings()->mapScales();
  const bool hasProjectScales( QgsProject::instance()->viewSettings()->useProjectScales() );
  if ( !hasProjectScales || scales.isEmpty() )
  {
    // default to global map tool scales
    const QStringList scales = QgsSettingsRegistryCore::settingsMapScales->value();
    return !scales.isEmpty() && !scales[0].isEmpty();
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

  const Qgis::MapGridStyle gridStyle = mMapGrid->style();
  mGridTypeComboBox->setCurrentIndex( mGridTypeComboBox->findData( QVariant::fromValue( gridStyle ) ) );
  switch ( gridStyle )
  {
    case Qgis::MapGridStyle::LineCrosses:
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
    case Qgis::MapGridStyle::Markers:
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
    case Qgis::MapGridStyle::Lines:
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
    case Qgis::MapGridStyle::FrameAndAnnotationsOnly:
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
  const Qgis::MapGridFrameStyle gridFrameStyle = mMapGrid->frameStyle();
  mFrameStyleComboBox->setCurrentIndex( mFrameStyleComboBox->findData( QVariant::fromValue( gridFrameStyle ) ) );
  switch ( gridFrameStyle )
  {
    case Qgis::MapGridFrameStyle::Zebra:
    case Qgis::MapGridFrameStyle::ZebraNautical:
      toggleFrameControls( true, true, true, false );
      break;
    case Qgis::MapGridFrameStyle::InteriorTicks:
    case Qgis::MapGridFrameStyle::ExteriorTicks:
    case Qgis::MapGridFrameStyle::InteriorExteriorTicks:
      toggleFrameControls( true, false, true, true );
      break;
    case Qgis::MapGridFrameStyle::LineBorder:
    case Qgis::MapGridFrameStyle::LineBorderNautical:
      toggleFrameControls( true, false, false, false );
      break;
    case Qgis::MapGridFrameStyle::NoFrame:
      toggleFrameControls( false, false, false, false );
      break;
  }

  mCheckGridLeftSide->setChecked( mMapGrid->testFrameSideFlag( Qgis::MapGridFrameSideFlag::Left ) );
  mCheckGridRightSide->setChecked( mMapGrid->testFrameSideFlag( Qgis::MapGridFrameSideFlag::Right ) );
  mCheckGridTopSide->setChecked( mMapGrid->testFrameSideFlag( Qgis::MapGridFrameSideFlag::Top ) );
  mCheckGridBottomSide->setChecked( mMapGrid->testFrameSideFlag( Qgis::MapGridFrameSideFlag::Bottom ) );

  mRotatedTicksGroupBox->setChecked( mMapGrid->rotatedTicksEnabled() );
  mRotatedTicksLengthModeComboBox->setCurrentIndex( mRotatedTicksLengthModeComboBox->findData( QVariant::fromValue( mMapGrid->rotatedTicksLengthMode() ) ) );
  mRotatedTicksThresholdSpinBox->setValue( mMapGrid->rotatedTicksMinimumAngle() );
  mRotatedTicksMarginToCornerSpinBox->setValue( mMapGrid->rotatedTicksMarginToCorner() );

  mRotatedAnnotationsGroupBox->setChecked( mMapGrid->rotatedAnnotationsEnabled() );
  mRotatedAnnotationsLengthModeComboBox->setCurrentIndex( mRotatedAnnotationsLengthModeComboBox->findData( QVariant::fromValue( mMapGrid->rotatedAnnotationsLengthMode() ) ) );
  mRotatedAnnotationsThresholdSpinBox->setValue( mMapGrid->rotatedAnnotationsMinimumAngle() );
  mRotatedAnnotationsMarginToCornerSpinBox->setValue( mMapGrid->rotatedAnnotationsMarginToCorner() );

  initFrameDisplayBox( mFrameDivisionsLeftComboBox, mMapGrid->frameDivisions( Qgis::MapGridBorderSide::Left ) );
  initFrameDisplayBox( mFrameDivisionsRightComboBox, mMapGrid->frameDivisions( Qgis::MapGridBorderSide::Right ) );
  initFrameDisplayBox( mFrameDivisionsTopComboBox, mMapGrid->frameDivisions( Qgis::MapGridBorderSide::Top ) );
  initFrameDisplayBox( mFrameDivisionsBottomComboBox, mMapGrid->frameDivisions( Qgis::MapGridBorderSide::Bottom ) );

  //line style
  mGridLineStyleButton->setSymbol( mMapGrid->lineSymbol()->clone() );
  //marker style
  mGridMarkerStyleButton->setSymbol( mMapGrid->markerSymbol()->clone() );

  mGridBlendComboBox->setBlendMode( mMapGrid->blendMode() );

  mDrawAnnotationGroupBox->setChecked( mMapGrid->annotationEnabled() );
  mAnnotationDisplayLeftComboBox->setCurrentIndex( mAnnotationDisplayLeftComboBox->findData( QVariant::fromValue( mMapGrid->annotationDisplay( Qgis::MapGridBorderSide::Left ) ) ) );
  mAnnotationDisplayRightComboBox->setCurrentIndex( mAnnotationDisplayRightComboBox->findData( QVariant::fromValue( mMapGrid->annotationDisplay( Qgis::MapGridBorderSide::Right ) ) ) );
  mAnnotationDisplayTopComboBox->setCurrentIndex( mAnnotationDisplayTopComboBox->findData( QVariant::fromValue( mMapGrid->annotationDisplay( Qgis::MapGridBorderSide::Top ) ) ) );
  mAnnotationDisplayBottomComboBox->setCurrentIndex( mAnnotationDisplayBottomComboBox->findData( QVariant::fromValue( mMapGrid->annotationDisplay( Qgis::MapGridBorderSide::Bottom ) ) ) );

  mAnnotationPositionLeftComboBox->setCurrentIndex( mAnnotationPositionLeftComboBox->findData( QVariant::fromValue( mMapGrid->annotationPosition( Qgis::MapGridBorderSide::Left ) ) ) );
  mAnnotationPositionRightComboBox->setCurrentIndex( mAnnotationPositionRightComboBox->findData( QVariant::fromValue( mMapGrid->annotationPosition( Qgis::MapGridBorderSide::Right ) ) ) );
  mAnnotationPositionTopComboBox->setCurrentIndex( mAnnotationPositionTopComboBox->findData( QVariant::fromValue( mMapGrid->annotationPosition( Qgis::MapGridBorderSide::Top ) ) ) );
  mAnnotationPositionBottomComboBox->setCurrentIndex( mAnnotationPositionBottomComboBox->findData( QVariant::fromValue( mMapGrid->annotationPosition( Qgis::MapGridBorderSide::Bottom ) ) ) );

  initAnnotationDirectionBox( mAnnotationDirectionComboBoxLeft, mMapGrid->annotationDirection( Qgis::MapGridBorderSide::Left ) );
  initAnnotationDirectionBox( mAnnotationDirectionComboBoxRight, mMapGrid->annotationDirection( Qgis::MapGridBorderSide::Right ) );
  initAnnotationDirectionBox( mAnnotationDirectionComboBoxTop, mMapGrid->annotationDirection( Qgis::MapGridBorderSide::Top ) );
  initAnnotationDirectionBox( mAnnotationDirectionComboBoxBottom, mMapGrid->annotationDirection( Qgis::MapGridBorderSide::Bottom ) );

  mAnnotationFontButton->setDialogTitle( tr( "Grid Annotation Font" ) );
  mAnnotationFontButton->setMode( QgsFontButton::ModeTextRenderer );
  mAnnotationFontButton->setTextFormat( mMapGrid->annotationTextFormat() );

  mAnnotationFormatComboBox->setCurrentIndex( mAnnotationFormatComboBox->findData( QVariant::fromValue( mMapGrid->annotationFormat() ) ) );
  mAnnotationFormatButton->setEnabled( mMapGrid->annotationFormat() == Qgis::MapGridAnnotationFormat::CustomFormat );
  mDistanceToMapFrameSpinBox->setValue( mMapGrid->annotationFrameDistance() );
  mCoordinatePrecisionSpinBox->setValue( mMapGrid->annotationPrecision() );

  //Unit
  mMapGridUnitComboBox->setCurrentIndex( mMapGridUnitComboBox->findData( QVariant::fromValue( mMapGrid->units() ) ) );
  switch ( mMapGrid->units() )
  {
    case Qgis::MapGridUnit::MapUnits:
    case Qgis::MapGridUnit::Millimeters:
    case Qgis::MapGridUnit::Centimeters:
      mIntervalStackedWidget->setCurrentIndex( 0 );
      break;

    case Qgis::MapGridUnit::DynamicPageSizeBased:
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
  mMapGrid->setFrameSideFlag( Qgis::MapGridFrameSideFlag::Left, checked );
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
  mMapGrid->setFrameSideFlag( Qgis::MapGridFrameSideFlag::Right, checked );
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
  mMapGrid->setFrameSideFlag( Qgis::MapGridFrameSideFlag::Top, checked );
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
  mMapGrid->setFrameSideFlag( Qgis::MapGridFrameSideFlag::Bottom, checked );
  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mFrameDivisionsLeftComboBox_currentIndexChanged( int index )
{
  handleChangedFrameDisplay( Qgis::MapGridBorderSide::Left, mFrameDivisionsLeftComboBox->itemData( index ).value< Qgis::MapGridComponentVisibility >() );
}

void QgsLayoutMapGridWidget::mFrameDivisionsRightComboBox_currentIndexChanged( int index )
{
  handleChangedFrameDisplay( Qgis::MapGridBorderSide::Right, mFrameDivisionsRightComboBox->itemData( index ).value< Qgis::MapGridComponentVisibility >() );
}

void QgsLayoutMapGridWidget::mFrameDivisionsTopComboBox_currentIndexChanged( int index )
{
  handleChangedFrameDisplay( Qgis::MapGridBorderSide::Top, mFrameDivisionsTopComboBox->itemData( index ).value< Qgis::MapGridComponentVisibility >() );
}

void QgsLayoutMapGridWidget::mFrameDivisionsBottomComboBox_currentIndexChanged( int index )
{
  handleChangedFrameDisplay( Qgis::MapGridBorderSide::Bottom, mFrameDivisionsBottomComboBox->itemData( index ).value< Qgis::MapGridComponentVisibility >() );
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

  const Qgis::MapGridFrameStyle style = mFrameStyleComboBox->currentData().value< Qgis::MapGridFrameStyle >();
  mMap->beginCommand( tr( "Change Frame Style" ) );
  mMapGrid->setFrameStyle( style );
  switch ( style )
  {
    case Qgis::MapGridFrameStyle::Zebra:
    case Qgis::MapGridFrameStyle::ZebraNautical:
      toggleFrameControls( true, true, true, false );
      break;
    case Qgis::MapGridFrameStyle::InteriorTicks:
    case Qgis::MapGridFrameStyle::ExteriorTicks:
    case Qgis::MapGridFrameStyle::InteriorExteriorTicks:
      toggleFrameControls( true, false, true, true );
      break;
    case Qgis::MapGridFrameStyle::LineBorder:
    case Qgis::MapGridFrameStyle::LineBorderNautical:
      toggleFrameControls( true, false, false, false );
      break;
    case Qgis::MapGridFrameStyle::NoFrame:
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

  const Qgis::MapGridTickLengthMode mode = mRotatedTicksLengthModeComboBox->currentData().value< Qgis::MapGridTickLengthMode >();
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

  const Qgis::MapGridTickLengthMode mode = mRotatedAnnotationsLengthModeComboBox->currentData().value< Qgis::MapGridTickLengthMode >();
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

  const Qgis::MapGridUnit unit = mMapGridUnitComboBox->currentData().value< Qgis::MapGridUnit >();
  switch ( unit )
  {
    case Qgis::MapGridUnit::MapUnits:
    case Qgis::MapGridUnit::Millimeters:
    case Qgis::MapGridUnit::Centimeters:
      mIntervalStackedWidget->setCurrentIndex( 0 );
      break;

    case Qgis::MapGridUnit::DynamicPageSizeBased:
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
  const Qgis::MapGridAnnotationFormat prevFormat = mAnnotationFormatComboBox->currentData().value< Qgis::MapGridAnnotationFormat >();

  mAnnotationFormatComboBox->clear();
  mAnnotationFormatComboBox->addItem( tr( "Decimal" ), QVariant::fromValue( Qgis::MapGridAnnotationFormat::Decimal ) );
  mAnnotationFormatComboBox->addItem( tr( "Decimal with Suffix" ), QVariant::fromValue( Qgis::MapGridAnnotationFormat::DecimalWithSuffix ) );

  // only show degree based options for geographic CRSes
  const QgsCoordinateReferenceSystem crs = mMapGrid->crs().isValid() ? mMapGrid->crs() : mMap->crs();
  switch ( crs.mapUnits() )
  {
    case Qgis::DistanceUnit::Meters:
    case Qgis::DistanceUnit::Kilometers:
    case Qgis::DistanceUnit::Feet:
    case Qgis::DistanceUnit::NauticalMiles:
    case Qgis::DistanceUnit::Yards:
    case Qgis::DistanceUnit::Miles:
    case Qgis::DistanceUnit::Centimeters:
    case Qgis::DistanceUnit::Millimeters:
    case Qgis::DistanceUnit::Inches:
    case Qgis::DistanceUnit::ChainsInternational:
    case Qgis::DistanceUnit::ChainsBritishBenoit1895A:
    case Qgis::DistanceUnit::ChainsBritishBenoit1895B:
    case Qgis::DistanceUnit::ChainsBritishSears1922Truncated:
    case Qgis::DistanceUnit::ChainsBritishSears1922:
    case Qgis::DistanceUnit::ChainsClarkes:
    case Qgis::DistanceUnit::ChainsUSSurvey:
    case Qgis::DistanceUnit::FeetBritish1865:
    case Qgis::DistanceUnit::FeetBritish1936:
    case Qgis::DistanceUnit::FeetBritishBenoit1895A:
    case Qgis::DistanceUnit::FeetBritishBenoit1895B:
    case Qgis::DistanceUnit::FeetBritishSears1922Truncated:
    case Qgis::DistanceUnit::FeetBritishSears1922:
    case Qgis::DistanceUnit::FeetClarkes:
    case Qgis::DistanceUnit::FeetGoldCoast:
    case Qgis::DistanceUnit::FeetIndian:
    case Qgis::DistanceUnit::FeetIndian1937:
    case Qgis::DistanceUnit::FeetIndian1962:
    case Qgis::DistanceUnit::FeetIndian1975:
    case Qgis::DistanceUnit::FeetUSSurvey:
    case Qgis::DistanceUnit::LinksInternational:
    case Qgis::DistanceUnit::LinksBritishBenoit1895A:
    case Qgis::DistanceUnit::LinksBritishBenoit1895B:
    case Qgis::DistanceUnit::LinksBritishSears1922Truncated:
    case Qgis::DistanceUnit::LinksBritishSears1922:
    case Qgis::DistanceUnit::LinksClarkes:
    case Qgis::DistanceUnit::LinksUSSurvey:
    case Qgis::DistanceUnit::YardsBritishBenoit1895A:
    case Qgis::DistanceUnit::YardsBritishBenoit1895B:
    case Qgis::DistanceUnit::YardsBritishSears1922Truncated:
    case Qgis::DistanceUnit::YardsBritishSears1922:
    case Qgis::DistanceUnit::YardsClarkes:
    case Qgis::DistanceUnit::YardsIndian:
    case Qgis::DistanceUnit::YardsIndian1937:
    case Qgis::DistanceUnit::YardsIndian1962:
    case Qgis::DistanceUnit::YardsIndian1975:
    case Qgis::DistanceUnit::MilesUSSurvey:
    case Qgis::DistanceUnit::Fathoms:
    case Qgis::DistanceUnit::MetersGermanLegal:
      break;

    case Qgis::DistanceUnit::Degrees:
    case Qgis::DistanceUnit::Unknown:
      mAnnotationFormatComboBox->addItem( tr( "Degree, Minute" ), QVariant::fromValue( Qgis::MapGridAnnotationFormat::DegreeMinuteNoSuffix ) );
      mAnnotationFormatComboBox->addItem( tr( "Degree, Minute with Suffix" ), QVariant::fromValue( Qgis::MapGridAnnotationFormat::DegreeMinute ) );
      mAnnotationFormatComboBox->addItem( tr( "Degree, Minute Aligned" ), QVariant::fromValue( Qgis::MapGridAnnotationFormat::DegreeMinutePadded ) );
      mAnnotationFormatComboBox->addItem( tr( "Degree, Minute, Second" ), QVariant::fromValue( Qgis::MapGridAnnotationFormat::DegreeMinuteSecondNoSuffix ) );
      mAnnotationFormatComboBox->addItem( tr( "Degree, Minute, Second with Suffix" ), QVariant::fromValue( Qgis::MapGridAnnotationFormat::DegreeMinuteSecond ) );
      mAnnotationFormatComboBox->addItem( tr( "Degree, Minute, Second Aligned" ), QVariant::fromValue( Qgis::MapGridAnnotationFormat::DegreeMinuteSecondPadded ) );
      break;
  }
  mAnnotationFormatComboBox->addItem( tr( "Custom" ), QVariant::fromValue( Qgis::MapGridAnnotationFormat::CustomFormat ) );

  const int prevIndex = mAnnotationFormatComboBox->findData( QVariant::fromValue( prevFormat ) );
  if ( prevIndex >= 0 )
    mAnnotationFormatComboBox->setCurrentIndex( prevIndex );
  else
    mAnnotationFormatComboBox->setCurrentIndex( 0 );
  mBlockAnnotationFormatUpdates--;

  const Qgis::MapGridAnnotationFormat newFormat = mAnnotationFormatComboBox->currentData().value< Qgis::MapGridAnnotationFormat >();
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
  switch ( mGridTypeComboBox->currentData().value< Qgis::MapGridStyle >() )
  {
    case Qgis::MapGridStyle::LineCrosses:
      mMapGrid->setStyle( Qgis::MapGridStyle::LineCrosses );
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

    case Qgis::MapGridStyle::Markers:
      mMapGrid->setStyle( Qgis::MapGridStyle::Markers );
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

    case Qgis::MapGridStyle::Lines:
      mMapGrid->setStyle( Qgis::MapGridStyle::Lines );
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

    case Qgis::MapGridStyle::FrameAndAnnotationsOnly:
      mMapGrid->setStyle( Qgis::MapGridStyle::FrameAndAnnotationsOnly );
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
  expressionContext.setHighlightedFunctions( QStringList() << u"to_dms"_s << u"to_dm"_s );

  QgsExpressionBuilderDialog exprDlg( coverageLayer(), mMapGrid->annotationExpression(), this, u"generic"_s, expressionContext );
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
  handleChangedAnnotationDisplay( Qgis::MapGridBorderSide::Left, mAnnotationDisplayLeftComboBox->currentData().value< Qgis::MapGridComponentVisibility >() );
}

void QgsLayoutMapGridWidget::mAnnotationDisplayRightComboBox_currentIndexChanged( int )
{
  handleChangedAnnotationDisplay( Qgis::MapGridBorderSide::Right, mAnnotationDisplayRightComboBox->currentData().value< Qgis::MapGridComponentVisibility >() );
}

void QgsLayoutMapGridWidget::mAnnotationDisplayTopComboBox_currentIndexChanged( int )
{
  handleChangedAnnotationDisplay( Qgis::MapGridBorderSide::Top, mAnnotationDisplayTopComboBox->currentData().value< Qgis::MapGridComponentVisibility >() );
}

void QgsLayoutMapGridWidget::mAnnotationDisplayBottomComboBox_currentIndexChanged( int )
{
  handleChangedAnnotationDisplay( Qgis::MapGridBorderSide::Bottom, mAnnotationDisplayBottomComboBox->currentData().value< Qgis::MapGridComponentVisibility >() );
}

void QgsLayoutMapGridWidget::mAnnotationPositionLeftComboBox_currentIndexChanged( int )
{
  handleChangedAnnotationPosition( Qgis::MapGridBorderSide::Left, mAnnotationPositionLeftComboBox->currentData().value< Qgis::MapGridAnnotationPosition >() );
}

void QgsLayoutMapGridWidget::mAnnotationPositionRightComboBox_currentIndexChanged( int )
{
  handleChangedAnnotationPosition( Qgis::MapGridBorderSide::Right, mAnnotationPositionRightComboBox->currentData().value< Qgis::MapGridAnnotationPosition >() );
}

void QgsLayoutMapGridWidget::mAnnotationPositionTopComboBox_currentIndexChanged( int )
{
  handleChangedAnnotationPosition( Qgis::MapGridBorderSide::Top, mAnnotationPositionTopComboBox->currentData().value< Qgis::MapGridAnnotationPosition >() );
}

void QgsLayoutMapGridWidget::mAnnotationPositionBottomComboBox_currentIndexChanged( int )
{
  handleChangedAnnotationPosition( Qgis::MapGridBorderSide::Bottom, mAnnotationPositionBottomComboBox->currentData().value< Qgis::MapGridAnnotationPosition >() );
}

void QgsLayoutMapGridWidget::mAnnotationDirectionComboBoxLeft_currentIndexChanged( int index )
{
  handleChangedAnnotationDirection( Qgis::MapGridBorderSide::Left, mAnnotationDirectionComboBoxLeft->itemData( index ).value< Qgis::MapGridAnnotationDirection >() );
}

void QgsLayoutMapGridWidget::mAnnotationDirectionComboBoxRight_currentIndexChanged( int index )
{
  handleChangedAnnotationDirection( Qgis::MapGridBorderSide::Right, mAnnotationDirectionComboBoxRight->itemData( index ).value< Qgis::MapGridAnnotationDirection >() );
}

void QgsLayoutMapGridWidget::mAnnotationDirectionComboBoxTop_currentIndexChanged( int index )
{
  handleChangedAnnotationDirection( Qgis::MapGridBorderSide::Top, mAnnotationDirectionComboBoxTop->itemData( index ).value< Qgis::MapGridAnnotationDirection >() );
}

void QgsLayoutMapGridWidget::mAnnotationDirectionComboBoxBottom_currentIndexChanged( int index )
{
  handleChangedAnnotationDirection( Qgis::MapGridBorderSide::Bottom, mAnnotationDirectionComboBoxBottom->itemData( index ).value< Qgis::MapGridAnnotationDirection >() );
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

  mMapGrid->setAnnotationFormat( mAnnotationFormatComboBox->itemData( index ).value< Qgis::MapGridAnnotationFormat >() );
  mAnnotationFormatButton->setEnabled( mMapGrid->annotationFormat() == Qgis::MapGridAnnotationFormat::CustomFormat );

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
