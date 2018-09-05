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

QgsLayoutMapGridWidget::QgsLayoutMapGridWidget( QgsLayoutItemMapGrid *mapGrid, QgsLayoutItemMap *map )
  : QgsLayoutItemBaseWidget( nullptr, mapGrid )
  , mMap( map )
  , mMapGrid( mapGrid )
{
  setupUi( this );
  connect( mIntervalXSpinBox, &QgsDoubleSpinBox::editingFinished, this, &QgsLayoutMapGridWidget::mIntervalXSpinBox_editingFinished );
  connect( mIntervalYSpinBox, &QgsDoubleSpinBox::editingFinished, this, &QgsLayoutMapGridWidget::mIntervalYSpinBox_editingFinished );
  connect( mOffsetXSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mOffsetXSpinBox_valueChanged );
  connect( mOffsetYSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mOffsetYSpinBox_valueChanged );
  connect( mCrossWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mCrossWidthSpinBox_valueChanged );
  connect( mFrameWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mFrameWidthSpinBox_valueChanged );
  connect( mFrameStyleComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mFrameStyleComboBox_currentIndexChanged );
  connect( mGridFramePenSizeSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mGridFramePenSizeSpinBox_valueChanged );
  connect( mGridFramePenColorButton, &QgsColorButton::colorChanged, this, &QgsLayoutMapGridWidget::mGridFramePenColorButton_colorChanged );
  connect( mGridFrameFill1ColorButton, &QgsColorButton::colorChanged, this, &QgsLayoutMapGridWidget::mGridFrameFill1ColorButton_colorChanged );
  connect( mGridFrameFill2ColorButton, &QgsColorButton::colorChanged, this, &QgsLayoutMapGridWidget::mGridFrameFill2ColorButton_colorChanged );
  connect( mGridTypeComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mGridTypeComboBox_currentIndexChanged );
  connect( mMapGridCRSButton, &QPushButton::clicked, this, &QgsLayoutMapGridWidget::mMapGridCRSButton_clicked );
  connect( mMapGridUnitComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mMapGridUnitComboBox_currentIndexChanged );
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
  connect( mAnnotationDisplayLeftComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationDisplayLeftComboBox_currentIndexChanged );
  connect( mAnnotationDisplayRightComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationDisplayRightComboBox_currentIndexChanged );
  connect( mAnnotationDisplayTopComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationDisplayTopComboBox_currentIndexChanged );
  connect( mAnnotationDisplayBottomComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationDisplayBottomComboBox_currentIndexChanged );
  connect( mAnnotationPositionLeftComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationPositionLeftComboBox_currentIndexChanged );
  connect( mAnnotationPositionRightComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationPositionRightComboBox_currentIndexChanged );
  connect( mAnnotationPositionTopComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationPositionTopComboBox_currentIndexChanged );
  connect( mAnnotationPositionBottomComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationPositionBottomComboBox_currentIndexChanged );
  connect( mAnnotationDirectionComboBoxLeft, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationDirectionComboBoxLeft_currentIndexChanged );
  connect( mAnnotationDirectionComboBoxRight, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationDirectionComboBoxRight_currentIndexChanged );
  connect( mAnnotationDirectionComboBoxTop, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationDirectionComboBoxTop_currentIndexChanged );
  connect( mAnnotationDirectionComboBoxBottom, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationDirectionComboBoxBottom_currentIndexChanged );
  connect( mAnnotationFormatComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutMapGridWidget::mAnnotationFormatComboBox_currentIndexChanged );
  connect( mCoordinatePrecisionSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mCoordinatePrecisionSpinBox_valueChanged );
  connect( mDistanceToMapFrameSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutMapGridWidget::mDistanceToMapFrameSpinBox_valueChanged );
  connect( mAnnotationFontColorButton, &QgsColorButton::colorChanged, this, &QgsLayoutMapGridWidget::mAnnotationFontColorButton_colorChanged );
  setPanelTitle( tr( "Map Grid Properties" ) );

  mAnnotationFontButton->setMode( QgsFontButton::ModeQFont );

  blockAllSignals( true );

  mGridTypeComboBox->insertItem( 0, tr( "Solid" ) );
  mGridTypeComboBox->insertItem( 1, tr( "Cross" ) );
  mGridTypeComboBox->insertItem( 2, tr( "Markers" ) );
  mGridTypeComboBox->insertItem( 3, tr( "Frame and annotations only" ) );

  insertFrameDisplayEntries( mFrameDivisionsLeftComboBox );
  insertFrameDisplayEntries( mFrameDivisionsRightComboBox );
  insertFrameDisplayEntries( mFrameDivisionsTopComboBox );
  insertFrameDisplayEntries( mFrameDivisionsBottomComboBox );

  mAnnotationFormatComboBox->addItem( tr( "Decimal" ), QgsLayoutItemMapGrid::Decimal );
  mAnnotationFormatComboBox->addItem( tr( "Decimal with suffix" ), QgsLayoutItemMapGrid::DecimalWithSuffix );
  mAnnotationFormatComboBox->addItem( tr( "Degree, minute" ), QgsLayoutItemMapGrid::DegreeMinuteNoSuffix );
  mAnnotationFormatComboBox->addItem( tr( "Degree, minute with suffix" ), QgsLayoutItemMapGrid::DegreeMinute );
  mAnnotationFormatComboBox->addItem( tr( "Degree, minute aligned" ), QgsLayoutItemMapGrid::DegreeMinutePadded );
  mAnnotationFormatComboBox->addItem( tr( "Degree, minute, second" ), QgsLayoutItemMapGrid::DegreeMinuteSecondNoSuffix );
  mAnnotationFormatComboBox->addItem( tr( "Degree, minute, second with suffix" ), QgsLayoutItemMapGrid::DegreeMinuteSecond );
  mAnnotationFormatComboBox->addItem( tr( "Degree, minute, second aligned" ), QgsLayoutItemMapGrid::DegreeMinuteSecondPadded );
  mAnnotationFormatComboBox->addItem( tr( "Custom" ), QgsLayoutItemMapGrid::CustomFormat );

  mAnnotationFontColorButton->setColorDialogTitle( tr( "Select Font Color" ) );
  mAnnotationFontColorButton->setAllowOpacity( true );
  mAnnotationFontColorButton->setContext( QStringLiteral( "composer" ) );

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
  mGridFramePenColorButton->setNoColorString( tr( "Transparent frame" ) );
  mGridFramePenColorButton->setShowNoColor( true );

  mGridFrameFill1ColorButton->setColorDialogTitle( tr( "Select Grid Frame Fill Color" ) );
  mGridFrameFill1ColorButton->setAllowOpacity( true );
  mGridFrameFill1ColorButton->setContext( QStringLiteral( "composer" ) );
  mGridFrameFill1ColorButton->setNoColorString( tr( "Transparent fill" ) );
  mGridFrameFill1ColorButton->setShowNoColor( true );

  mGridFrameFill2ColorButton->setColorDialogTitle( tr( "Select Grid Frame Fill Color" ) );
  mGridFrameFill2ColorButton->setAllowOpacity( true );
  mGridFrameFill2ColorButton->setContext( QStringLiteral( "composer" ) );
  mGridFrameFill2ColorButton->setNoColorString( tr( "Transparent fill" ) );
  mGridFrameFill2ColorButton->setShowNoColor( true );

  mGridLineStyleButton->setSymbolType( QgsSymbol::Line );
  mGridMarkerStyleButton->setSymbolType( QgsSymbol::Marker );

  //set initial state of frame style controls
  toggleFrameControls( false, false, false );

  updateGuiElements();

  blockAllSignals( false );
  connect( mAnnotationFontButton, &QgsFontButton::changed, this, &QgsLayoutMapGridWidget::annotationFontChanged );
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
  }

}

void QgsLayoutMapGridWidget::populateDataDefinedButtons()
{
  // none for now
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
  mGridTypeComboBox->blockSignals( block );
  mIntervalXSpinBox->blockSignals( block );
  mIntervalYSpinBox->blockSignals( block );
  mOffsetXSpinBox->blockSignals( block );
  mOffsetYSpinBox->blockSignals( block );
  mCrossWidthSpinBox->blockSignals( block );
  mFrameStyleComboBox->blockSignals( block );
  mFrameWidthSpinBox->blockSignals( block );
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
  mAnnotationFontColorButton->blockSignals( block );
  mAnnotationFontButton->blockSignals( block );
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

void QgsLayoutMapGridWidget::handleChangedAnnotationDisplay( QgsLayoutItemMapGrid::BorderSide border, const QString &text )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Annotation Format" ) );
  if ( text == tr( "Show all" ) )
  {
    mMapGrid->setAnnotationDisplay( QgsLayoutItemMapGrid::ShowAll, border );
  }
  else if ( text == tr( "Show latitude only" ) )
  {
    mMapGrid->setAnnotationDisplay( QgsLayoutItemMapGrid::LatitudeOnly, border );
  }
  else if ( text == tr( "Show longitude only" ) )
  {
    mMapGrid->setAnnotationDisplay( QgsLayoutItemMapGrid::LongitudeOnly, border );
  }
  else //disabled
  {
    mMapGrid->setAnnotationDisplay( QgsLayoutItemMapGrid::HideAll, border );
  }

  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::toggleFrameControls( bool frameEnabled, bool frameFillEnabled, bool frameSizeEnabled )
{
  //set status of frame controls
  mFrameWidthSpinBox->setEnabled( frameSizeEnabled );
  mGridFramePenSizeSpinBox->setEnabled( frameEnabled );
  mGridFramePenColorButton->setEnabled( frameEnabled );
  mGridFrameFill1ColorButton->setEnabled( frameFillEnabled );
  mGridFrameFill2ColorButton->setEnabled( frameFillEnabled );
  mFrameWidthLabel->setEnabled( frameSizeEnabled );
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
}

void QgsLayoutMapGridWidget::insertAnnotationPositionEntries( QComboBox *c )
{
  c->insertItem( 0, tr( "Inside frame" ) );
  c->insertItem( 1, tr( "Outside frame" ) );
}

void QgsLayoutMapGridWidget::insertAnnotationDirectionEntries( QComboBox *c )
{
  c->addItem( tr( "Horizontal" ), QgsLayoutItemMapGrid::Horizontal );
  c->addItem( tr( "Vertical ascending" ), QgsLayoutItemMapGrid::Vertical );
  c->addItem( tr( "Vertical descending" ), QgsLayoutItemMapGrid::VerticalDescending );
}

void QgsLayoutMapGridWidget::initFrameDisplayBox( QComboBox *c, QgsLayoutItemMapGrid::DisplayMode display )
{
  if ( !c )
  {
    return;
  }
  c->setCurrentIndex( c->findData( display ) );
}

void QgsLayoutMapGridWidget::initAnnotationDisplayBox( QComboBox *c, QgsLayoutItemMapGrid::DisplayMode display )
{
  if ( !c )
  {
    return;
  }

  if ( display == QgsLayoutItemMapGrid::ShowAll )
  {
    c->setCurrentIndex( c->findText( tr( "Show all" ) ) );
  }
  else if ( display == QgsLayoutItemMapGrid::LatitudeOnly )
  {
    c->setCurrentIndex( c->findText( tr( "Show latitude only" ) ) );
  }
  else if ( display == QgsLayoutItemMapGrid::LongitudeOnly )
  {
    c->setCurrentIndex( c->findText( tr( "Show longitude only" ) ) );
  }
  else
  {
    c->setCurrentIndex( c->findText( tr( "Disabled" ) ) );
  }
}

void QgsLayoutMapGridWidget::handleChangedAnnotationPosition( QgsLayoutItemMapGrid::BorderSide border, const QString &text )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Annotation Position" ) );
  if ( text == tr( "Inside frame" ) )
  {
    mMapGrid->setAnnotationPosition( QgsLayoutItemMapGrid::InsideMapFrame, border );
  }
  else //Outside frame
  {
    mMapGrid->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, border );
  }

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
  c->addItem( tr( "Latitude/Y only" ), QgsLayoutItemMapGrid::LatitudeOnly );
  c->addItem( tr( "Longitude/X only" ), QgsLayoutItemMapGrid::LongitudeOnly );
}

void QgsLayoutMapGridWidget::insertAnnotationDisplayEntries( QComboBox *c )
{
  c->insertItem( 0, tr( "Show all" ) );
  c->insertItem( 1, tr( "Show latitude only" ) );
  c->insertItem( 2, tr( "Show longitude only" ) );
  c->insertItem( 3, tr( "Disabled" ) );
}

void QgsLayoutMapGridWidget::initAnnotationPositionBox( QComboBox *c, QgsLayoutItemMapGrid::AnnotationPosition pos )
{
  if ( !c )
  {
    return;
  }

  if ( pos == QgsLayoutItemMapGrid::InsideMapFrame )
  {
    c->setCurrentIndex( c->findText( tr( "Inside frame" ) ) );
  }
  else
  {
    c->setCurrentIndex( c->findText( tr( "Outside frame" ) ) );
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
  QStringList scales( QgsProject::instance()->readListEntry( QStringLiteral( "Scales" ), QStringLiteral( "/ScalesList" ) ) );
  bool hasProjectScales( QgsProject::instance()->readBoolEntry( QStringLiteral( "Scales" ), QStringLiteral( "/useProjectScales" ) ) );
  if ( !hasProjectScales || scales.isEmpty() )
  {
    // default to global map tool scales
    QgsSettings settings;
    QString scalesStr( settings.value( QStringLiteral( "Map/scales" ), PROJECT_SCALES ).toString() );
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

  mIntervalXSpinBox->setValue( mMapGrid->intervalX() );
  mIntervalYSpinBox->setValue( mMapGrid->intervalY() );
  mOffsetXSpinBox->setValue( mMapGrid->offsetX() );
  mOffsetYSpinBox->setValue( mMapGrid->offsetY() );
  mCrossWidthSpinBox->setValue( mMapGrid->crossLength() );
  mFrameWidthSpinBox->setValue( mMapGrid->frameWidth() );
  mGridFramePenSizeSpinBox->setValue( mMapGrid->framePenSize() );
  mGridFramePenColorButton->setColor( mMapGrid->framePenColor() );
  mGridFrameFill1ColorButton->setColor( mMapGrid->frameFillColor1() );
  mGridFrameFill2ColorButton->setColor( mMapGrid->frameFillColor2() );

  QgsLayoutItemMapGrid::GridStyle gridStyle = mMapGrid->style();
  switch ( gridStyle )
  {
    case QgsLayoutItemMapGrid::Cross:
      mGridTypeComboBox->setCurrentIndex( mGridTypeComboBox->findText( tr( "Cross" ) ) );
      mCrossWidthSpinBox->setVisible( true );
      mCrossWidthLabel->setVisible( true );
      mGridLineStyleButton->setVisible( true );
      mLineStyleLabel->setVisible( true );
      mGridMarkerStyleButton->setVisible( false );
      mMarkerStyleLabel->setVisible( false );
      mGridBlendComboBox->setVisible( true );
      mGridBlendLabel->setVisible( true );
      break;
    case QgsLayoutItemMapGrid::Markers:
      mGridTypeComboBox->setCurrentIndex( mGridTypeComboBox->findText( tr( "Markers" ) ) );
      mCrossWidthSpinBox->setVisible( false );
      mCrossWidthLabel->setVisible( false );
      mGridLineStyleButton->setVisible( false );
      mLineStyleLabel->setVisible( false );
      mGridMarkerStyleButton->setVisible( true );
      mMarkerStyleLabel->setVisible( true );
      mGridBlendComboBox->setVisible( true );
      mGridBlendLabel->setVisible( true );
      break;
    case QgsLayoutItemMapGrid::Solid:
      mGridTypeComboBox->setCurrentIndex( mGridTypeComboBox->findText( tr( "Solid" ) ) );
      mCrossWidthSpinBox->setVisible( false );
      mCrossWidthLabel->setVisible( false );
      mGridLineStyleButton->setVisible( true );
      mLineStyleLabel->setVisible( true );
      mGridMarkerStyleButton->setVisible( false );
      mMarkerStyleLabel->setVisible( false );
      mGridBlendComboBox->setVisible( true );
      mGridBlendLabel->setVisible( true );
      break;
    case QgsLayoutItemMapGrid::FrameAnnotationsOnly:
      mGridTypeComboBox->setCurrentIndex( mGridTypeComboBox->findText( tr( "Frame and annotations only" ) ) );
      mCrossWidthSpinBox->setVisible( false );
      mCrossWidthLabel->setVisible( false );
      mGridLineStyleButton->setVisible( false );
      mLineStyleLabel->setVisible( false );
      mGridMarkerStyleButton->setVisible( false );
      mMarkerStyleLabel->setVisible( false );
      mGridBlendComboBox->setVisible( false );
      mGridBlendLabel->setVisible( false );
      break;
  }

  //grid frame
  mFrameWidthSpinBox->setValue( mMapGrid->frameWidth() );
  QgsLayoutItemMapGrid::FrameStyle gridFrameStyle = mMapGrid->frameStyle();
  switch ( gridFrameStyle )
  {
    case QgsLayoutItemMapGrid::Zebra:
      mFrameStyleComboBox->setCurrentIndex( 1 );
      toggleFrameControls( true, true, true );
      break;
    case QgsLayoutItemMapGrid::InteriorTicks:
      mFrameStyleComboBox->setCurrentIndex( 2 );
      toggleFrameControls( true, false, true );
      break;
    case QgsLayoutItemMapGrid::ExteriorTicks:
      mFrameStyleComboBox->setCurrentIndex( 3 );
      toggleFrameControls( true, false, true );
      break;
    case QgsLayoutItemMapGrid::InteriorExteriorTicks:
      mFrameStyleComboBox->setCurrentIndex( 4 );
      toggleFrameControls( true, false, true );
      break;
    case QgsLayoutItemMapGrid::LineBorder:
      mFrameStyleComboBox->setCurrentIndex( 5 );
      toggleFrameControls( true, false, false );
      break;
    default:
      mFrameStyleComboBox->setCurrentIndex( 0 );
      toggleFrameControls( false, false, false );
      break;
  }

  mCheckGridLeftSide->setChecked( mMapGrid->testFrameSideFlag( QgsLayoutItemMapGrid::FrameLeft ) );
  mCheckGridRightSide->setChecked( mMapGrid->testFrameSideFlag( QgsLayoutItemMapGrid::FrameRight ) );
  mCheckGridTopSide->setChecked( mMapGrid->testFrameSideFlag( QgsLayoutItemMapGrid::FrameTop ) );
  mCheckGridBottomSide->setChecked( mMapGrid->testFrameSideFlag( QgsLayoutItemMapGrid::FrameBottom ) );

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
  initAnnotationDisplayBox( mAnnotationDisplayLeftComboBox, mMapGrid->annotationDisplay( QgsLayoutItemMapGrid::Left ) );
  initAnnotationDisplayBox( mAnnotationDisplayRightComboBox, mMapGrid->annotationDisplay( QgsLayoutItemMapGrid::Right ) );
  initAnnotationDisplayBox( mAnnotationDisplayTopComboBox, mMapGrid->annotationDisplay( QgsLayoutItemMapGrid::Top ) );
  initAnnotationDisplayBox( mAnnotationDisplayBottomComboBox, mMapGrid->annotationDisplay( QgsLayoutItemMapGrid::Bottom ) );

  initAnnotationPositionBox( mAnnotationPositionLeftComboBox, mMapGrid->annotationPosition( QgsLayoutItemMapGrid::Left ) );
  initAnnotationPositionBox( mAnnotationPositionRightComboBox, mMapGrid->annotationPosition( QgsLayoutItemMapGrid::Right ) );
  initAnnotationPositionBox( mAnnotationPositionTopComboBox, mMapGrid->annotationPosition( QgsLayoutItemMapGrid::Top ) );
  initAnnotationPositionBox( mAnnotationPositionBottomComboBox, mMapGrid->annotationPosition( QgsLayoutItemMapGrid::Bottom ) );

  initAnnotationDirectionBox( mAnnotationDirectionComboBoxLeft, mMapGrid->annotationDirection( QgsLayoutItemMapGrid::Left ) );
  initAnnotationDirectionBox( mAnnotationDirectionComboBoxRight, mMapGrid->annotationDirection( QgsLayoutItemMapGrid::Right ) );
  initAnnotationDirectionBox( mAnnotationDirectionComboBoxTop, mMapGrid->annotationDirection( QgsLayoutItemMapGrid::Top ) );
  initAnnotationDirectionBox( mAnnotationDirectionComboBoxBottom, mMapGrid->annotationDirection( QgsLayoutItemMapGrid::Bottom ) );

  mAnnotationFontColorButton->setColor( mMapGrid->annotationFontColor() );
  mAnnotationFontButton->setCurrentFont( mMapGrid->annotationFont() );

  mAnnotationFormatComboBox->setCurrentIndex( mAnnotationFormatComboBox->findData( mMapGrid->annotationFormat() ) );
  mAnnotationFormatButton->setEnabled( mMapGrid->annotationFormat() == QgsLayoutItemMapGrid::CustomFormat );
  mDistanceToMapFrameSpinBox->setValue( mMapGrid->annotationFrameDistance() );
  mCoordinatePrecisionSpinBox->setValue( mMapGrid->annotationPrecision() );

  //Unit
  QgsLayoutItemMapGrid::GridUnit gridUnit = mMapGrid->units();
  if ( gridUnit == QgsLayoutItemMapGrid::MapUnit )
  {
    mMapGridUnitComboBox->setCurrentIndex( mMapGridUnitComboBox->findText( tr( "Map unit" ) ) );
  }
  else if ( gridUnit == QgsLayoutItemMapGrid::MM )
  {
    mMapGridUnitComboBox->setCurrentIndex( mMapGridUnitComboBox->findText( tr( "Millimeter" ) ) );
  }
  else if ( gridUnit == QgsLayoutItemMapGrid::CM )
  {
    mMapGridUnitComboBox->setCurrentIndex( mMapGridUnitComboBox->findText( tr( "Centimeter" ) ) );
  }

  //CRS button
  QgsCoordinateReferenceSystem gridCrs = mMapGrid->crs();
  QString crsButtonText = gridCrs.isValid() ? gridCrs.authid() : tr( "Change…" );
  mMapGridCRSButton->setText( crsButtonText );
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
  handleChangedFrameDisplay( QgsLayoutItemMapGrid::Left, ( QgsLayoutItemMapGrid::DisplayMode ) mFrameDivisionsLeftComboBox->itemData( index ).toInt() );
}

void QgsLayoutMapGridWidget::mFrameDivisionsRightComboBox_currentIndexChanged( int index )
{
  handleChangedFrameDisplay( QgsLayoutItemMapGrid::Right, ( QgsLayoutItemMapGrid::DisplayMode ) mFrameDivisionsRightComboBox->itemData( index ).toInt() );
}

void QgsLayoutMapGridWidget::mFrameDivisionsTopComboBox_currentIndexChanged( int index )
{
  handleChangedFrameDisplay( QgsLayoutItemMapGrid::Top, ( QgsLayoutItemMapGrid::DisplayMode ) mFrameDivisionsTopComboBox->itemData( index ).toInt() );
}

void QgsLayoutMapGridWidget::mFrameDivisionsBottomComboBox_currentIndexChanged( int index )
{
  handleChangedFrameDisplay( QgsLayoutItemMapGrid::Bottom, ( QgsLayoutItemMapGrid::DisplayMode ) mFrameDivisionsBottomComboBox->itemData( index ).toInt() );
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

void QgsLayoutMapGridWidget::mFrameStyleComboBox_currentIndexChanged( const QString &text )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Frame Style" ) );
  if ( text == tr( "Zebra" ) )
  {
    mMapGrid->setFrameStyle( QgsLayoutItemMapGrid::Zebra );
    toggleFrameControls( true, true, true );
  }
  else if ( text == tr( "Interior ticks" ) )
  {
    mMapGrid->setFrameStyle( QgsLayoutItemMapGrid::InteriorTicks );
    toggleFrameControls( true, false, true );
  }
  else if ( text == tr( "Exterior ticks" ) )
  {
    mMapGrid->setFrameStyle( QgsLayoutItemMapGrid::ExteriorTicks );
    toggleFrameControls( true, false, true );
  }
  else if ( text == tr( "Interior and exterior ticks" ) )
  {
    mMapGrid->setFrameStyle( QgsLayoutItemMapGrid::InteriorExteriorTicks );
    toggleFrameControls( true, false, true );
  }
  else if ( text == tr( "Line border" ) )
  {
    mMapGrid->setFrameStyle( QgsLayoutItemMapGrid::LineBorder );
    toggleFrameControls( true, false, false );
  }
  else //no frame
  {
    mMapGrid->setFrameStyle( QgsLayoutItemMapGrid::NoFrame );
    toggleFrameControls( false, false, false );
  }
  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mMapGridUnitComboBox_currentIndexChanged( const QString &text )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Grid Unit" ) );
  if ( text == tr( "Map unit" ) )
  {
    mMapGrid->setUnits( QgsLayoutItemMapGrid::MapUnit );
  }
  else if ( text == tr( "Millimeter" ) )
  {
    mMapGrid->setUnits( QgsLayoutItemMapGrid::MM );
  }
  else if ( text == tr( "Centimeter" ) )
  {
    mMapGrid->setUnits( QgsLayoutItemMapGrid::CM );
  }
  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mGridBlendComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  if ( mMapGrid )
  {
    mMap->beginCommand( tr( "Change Grid Blend Mode" ) );
    mMapGrid->setBlendMode( mGridBlendComboBox->blendMode() );
    mMap->update();
    mMap->endCommand();
  }

}

void QgsLayoutMapGridWidget::mGridTypeComboBox_currentIndexChanged( const QString &text )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Grid Type" ) );
  if ( text == tr( "Cross" ) )
  {
    mMapGrid->setStyle( QgsLayoutItemMapGrid::Cross );
    mCrossWidthSpinBox->setVisible( true );
    mCrossWidthLabel->setVisible( true );
    mGridLineStyleButton->setVisible( true );
    mLineStyleLabel->setVisible( true );
    mGridMarkerStyleButton->setVisible( false );
    mMarkerStyleLabel->setVisible( false );
    mGridBlendComboBox->setVisible( true );
    mGridBlendLabel->setVisible( true );
  }
  else if ( text == tr( "Markers" ) )
  {
    mMapGrid->setStyle( QgsLayoutItemMapGrid::Markers );
    mCrossWidthSpinBox->setVisible( false );
    mCrossWidthLabel->setVisible( false );
    mGridLineStyleButton->setVisible( false );
    mLineStyleLabel->setVisible( false );
    mGridMarkerStyleButton->setVisible( true );
    mMarkerStyleLabel->setVisible( true );
    mGridBlendComboBox->setVisible( true );
    mGridBlendLabel->setVisible( true );
  }
  else if ( text == tr( "Solid" ) )
  {
    mMapGrid->setStyle( QgsLayoutItemMapGrid::Solid );
    mCrossWidthSpinBox->setVisible( false );
    mCrossWidthLabel->setVisible( false );
    mGridLineStyleButton->setVisible( true );
    mLineStyleLabel->setVisible( true );
    mGridMarkerStyleButton->setVisible( false );
    mMarkerStyleLabel->setVisible( false );
    mGridBlendComboBox->setVisible( true );
    mGridBlendLabel->setVisible( true );
  }
  else
  {
    mMapGrid->setStyle( QgsLayoutItemMapGrid::FrameAnnotationsOnly );
    mCrossWidthSpinBox->setVisible( false );
    mCrossWidthLabel->setVisible( false );
    mGridLineStyleButton->setVisible( false );
    mLineStyleLabel->setVisible( false );
    mGridMarkerStyleButton->setVisible( false );
    mMarkerStyleLabel->setVisible( false );
    mGridBlendComboBox->setVisible( false );
    mGridBlendLabel->setVisible( false );
  }

  mMap->updateBoundingRect();
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mMapGridCRSButton_clicked()
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  QgsProjectionSelectionDialog crsDialog( this );
  QgsCoordinateReferenceSystem crs = mMapGrid->crs();
  crsDialog.setCrs( crs.isValid() ? crs : mMap->crs() );

  if ( crsDialog.exec() == QDialog::Accepted )
  {
    mMap->beginCommand( tr( "Change Grid CRS" ) );
    mMapGrid->setCrs( crsDialog.crs() );
    mMap->updateBoundingRect();
    mMapGridCRSButton->setText( crsDialog.crs().authid() );
    mMap->endCommand();
  }
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

  QgsExpressionBuilderDialog exprDlg( nullptr, mMapGrid->annotationExpression(), this, QStringLiteral( "generic" ), expressionContext );
  exprDlg.setWindowTitle( tr( "Expression Based Annotation" ) );

  if ( exprDlg.exec() == QDialog::Accepted )
  {
    QString expression = exprDlg.expressionText();
    mMap->beginCommand( tr( "Change Annotation Format" ) );
    mMapGrid->setAnnotationExpression( expression );
    mMap->updateBoundingRect();
    mMap->update();
    mMap->endCommand();
  }
}

void QgsLayoutMapGridWidget::mAnnotationDisplayLeftComboBox_currentIndexChanged( const QString &text )
{
  handleChangedAnnotationDisplay( QgsLayoutItemMapGrid::Left, text );
}

void QgsLayoutMapGridWidget::mAnnotationDisplayRightComboBox_currentIndexChanged( const QString &text )
{
  handleChangedAnnotationDisplay( QgsLayoutItemMapGrid::Right, text );
}

void QgsLayoutMapGridWidget::mAnnotationDisplayTopComboBox_currentIndexChanged( const QString &text )
{
  handleChangedAnnotationDisplay( QgsLayoutItemMapGrid::Top, text );
}

void QgsLayoutMapGridWidget::mAnnotationDisplayBottomComboBox_currentIndexChanged( const QString &text )
{
  handleChangedAnnotationDisplay( QgsLayoutItemMapGrid::Bottom, text );
}

void QgsLayoutMapGridWidget::mAnnotationPositionLeftComboBox_currentIndexChanged( const QString &text )
{
  handleChangedAnnotationPosition( QgsLayoutItemMapGrid::Left, text );
}

void QgsLayoutMapGridWidget::mAnnotationPositionRightComboBox_currentIndexChanged( const QString &text )
{
  handleChangedAnnotationPosition( QgsLayoutItemMapGrid::Right, text );
}

void QgsLayoutMapGridWidget::mAnnotationPositionTopComboBox_currentIndexChanged( const QString &text )
{
  handleChangedAnnotationPosition( QgsLayoutItemMapGrid::Top, text );
}

void QgsLayoutMapGridWidget::mAnnotationPositionBottomComboBox_currentIndexChanged( const QString &text )
{
  handleChangedAnnotationPosition( QgsLayoutItemMapGrid::Bottom, text );
}

void QgsLayoutMapGridWidget::mAnnotationDirectionComboBoxLeft_currentIndexChanged( int index )
{
  handleChangedAnnotationDirection( QgsLayoutItemMapGrid::Left, ( QgsLayoutItemMapGrid::AnnotationDirection ) mAnnotationDirectionComboBoxLeft->itemData( index ).toInt() );
}

void QgsLayoutMapGridWidget::mAnnotationDirectionComboBoxRight_currentIndexChanged( int index )
{
  handleChangedAnnotationDirection( QgsLayoutItemMapGrid::Right, ( QgsLayoutItemMapGrid::AnnotationDirection ) mAnnotationDirectionComboBoxRight->itemData( index ).toInt() );
}

void QgsLayoutMapGridWidget::mAnnotationDirectionComboBoxTop_currentIndexChanged( int index )
{
  handleChangedAnnotationDirection( QgsLayoutItemMapGrid::Top, ( QgsLayoutItemMapGrid::AnnotationDirection ) mAnnotationDirectionComboBoxTop->itemData( index ).toInt() );
}

void QgsLayoutMapGridWidget::mAnnotationDirectionComboBoxBottom_currentIndexChanged( int index )
{
  handleChangedAnnotationDirection( QgsLayoutItemMapGrid::Bottom, ( QgsLayoutItemMapGrid::AnnotationDirection ) mAnnotationDirectionComboBoxBottom->itemData( index ).toInt() );
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

void QgsLayoutMapGridWidget::annotationFontChanged()
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Annotation Font" ) );
  mMapGrid->setAnnotationFont( mAnnotationFontButton->currentFont() );
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

void QgsLayoutMapGridWidget::mAnnotationFontColorButton_colorChanged( const QColor &color )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Annotation Color" ), QgsLayoutItem::UndoMapGridAnnotationFontColor );
  mMapGrid->setAnnotationFontColor( color );
  mMap->update();
  mMap->endCommand();
}

void QgsLayoutMapGridWidget::mAnnotationFormatComboBox_currentIndexChanged( int index )
{
  if ( !mMapGrid || !mMap )
  {
    return;
  }

  mMap->beginCommand( tr( "Change Annotation Format" ) );

  mMapGrid->setAnnotationFormat( ( QgsLayoutItemMapGrid::AnnotationFormat )mAnnotationFormatComboBox->itemData( index ).toInt() );
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
