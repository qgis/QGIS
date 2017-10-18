/***************************************************************************
                         qgscomposermapgridwidget.cpp
                         ----------------------------
    begin                : October 2016
    copyright            : (C) 2016 by Nyall Dawson
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

#include "qgscomposermapgridwidget.h"
#include "qgssymbolselectordialog.h"
#include "qgssymbol.h"
#include "qgscomposermap.h"
#include "qgsproject.h"
#include "qgssymbollayerutils.h"
#include "qgsstyle.h"
#include "qgsprojectionselectiondialog.h"
#include "qgscomposition.h"
#include "qgsmapsettings.h"
#include "qgsexpressionbuilderdialog.h"

QgsComposerMapGridWidget::QgsComposerMapGridWidget( QgsComposerMapGrid *mapGrid, QgsComposerMap *composerMap )
  : QgsComposerItemBaseWidget( nullptr, mapGrid )
  , mComposerMap( composerMap )
  , mComposerMapGrid( mapGrid )
{
  setupUi( this );
  connect( mGridLineStyleButton, &QPushButton::clicked, this, &QgsComposerMapGridWidget::mGridLineStyleButton_clicked );
  connect( mGridMarkerStyleButton, &QPushButton::clicked, this, &QgsComposerMapGridWidget::mGridMarkerStyleButton_clicked );
  connect( mIntervalXSpinBox, &QgsDoubleSpinBox::editingFinished, this, &QgsComposerMapGridWidget::mIntervalXSpinBox_editingFinished );
  connect( mIntervalYSpinBox, &QgsDoubleSpinBox::editingFinished, this, &QgsComposerMapGridWidget::mIntervalYSpinBox_editingFinished );
  connect( mOffsetXSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerMapGridWidget::mOffsetXSpinBox_valueChanged );
  connect( mOffsetYSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerMapGridWidget::mOffsetYSpinBox_valueChanged );
  connect( mCrossWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerMapGridWidget::mCrossWidthSpinBox_valueChanged );
  connect( mFrameWidthSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerMapGridWidget::mFrameWidthSpinBox_valueChanged );
  connect( mFrameStyleComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsComposerMapGridWidget::mFrameStyleComboBox_currentIndexChanged );
  connect( mGridFramePenSizeSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerMapGridWidget::mGridFramePenSizeSpinBox_valueChanged );
  connect( mGridFramePenColorButton, &QgsColorButton::colorChanged, this, &QgsComposerMapGridWidget::mGridFramePenColorButton_colorChanged );
  connect( mGridFrameFill1ColorButton, &QgsColorButton::colorChanged, this, &QgsComposerMapGridWidget::mGridFrameFill1ColorButton_colorChanged );
  connect( mGridFrameFill2ColorButton, &QgsColorButton::colorChanged, this, &QgsComposerMapGridWidget::mGridFrameFill2ColorButton_colorChanged );
  connect( mGridTypeComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsComposerMapGridWidget::mGridTypeComboBox_currentIndexChanged );
  connect( mMapGridCRSButton, &QPushButton::clicked, this, &QgsComposerMapGridWidget::mMapGridCRSButton_clicked );
  connect( mMapGridUnitComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsComposerMapGridWidget::mMapGridUnitComboBox_currentIndexChanged );
  connect( mGridBlendComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerMapGridWidget::mGridBlendComboBox_currentIndexChanged );
  connect( mCheckGridLeftSide, &QCheckBox::toggled, this, &QgsComposerMapGridWidget::mCheckGridLeftSide_toggled );
  connect( mCheckGridRightSide, &QCheckBox::toggled, this, &QgsComposerMapGridWidget::mCheckGridRightSide_toggled );
  connect( mCheckGridTopSide, &QCheckBox::toggled, this, &QgsComposerMapGridWidget::mCheckGridTopSide_toggled );
  connect( mCheckGridBottomSide, &QCheckBox::toggled, this, &QgsComposerMapGridWidget::mCheckGridBottomSide_toggled );
  connect( mFrameDivisionsLeftComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerMapGridWidget::mFrameDivisionsLeftComboBox_currentIndexChanged );
  connect( mFrameDivisionsRightComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerMapGridWidget::mFrameDivisionsRightComboBox_currentIndexChanged );
  connect( mFrameDivisionsTopComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerMapGridWidget::mFrameDivisionsTopComboBox_currentIndexChanged );
  connect( mFrameDivisionsBottomComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerMapGridWidget::mFrameDivisionsBottomComboBox_currentIndexChanged );
  connect( mDrawAnnotationGroupBox, &QgsCollapsibleGroupBoxBasic::toggled, this, &QgsComposerMapGridWidget::mDrawAnnotationGroupBox_toggled );
  connect( mAnnotationFormatButton, &QToolButton::clicked, this, &QgsComposerMapGridWidget::mAnnotationFormatButton_clicked );
  connect( mAnnotationDisplayLeftComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsComposerMapGridWidget::mAnnotationDisplayLeftComboBox_currentIndexChanged );
  connect( mAnnotationDisplayRightComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsComposerMapGridWidget::mAnnotationDisplayRightComboBox_currentIndexChanged );
  connect( mAnnotationDisplayTopComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsComposerMapGridWidget::mAnnotationDisplayTopComboBox_currentIndexChanged );
  connect( mAnnotationDisplayBottomComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsComposerMapGridWidget::mAnnotationDisplayBottomComboBox_currentIndexChanged );
  connect( mAnnotationPositionLeftComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsComposerMapGridWidget::mAnnotationPositionLeftComboBox_currentIndexChanged );
  connect( mAnnotationPositionRightComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsComposerMapGridWidget::mAnnotationPositionRightComboBox_currentIndexChanged );
  connect( mAnnotationPositionTopComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsComposerMapGridWidget::mAnnotationPositionTopComboBox_currentIndexChanged );
  connect( mAnnotationPositionBottomComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsComposerMapGridWidget::mAnnotationPositionBottomComboBox_currentIndexChanged );
  connect( mAnnotationDirectionComboBoxLeft, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerMapGridWidget::mAnnotationDirectionComboBoxLeft_currentIndexChanged );
  connect( mAnnotationDirectionComboBoxRight, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerMapGridWidget::mAnnotationDirectionComboBoxRight_currentIndexChanged );
  connect( mAnnotationDirectionComboBoxTop, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerMapGridWidget::mAnnotationDirectionComboBoxTop_currentIndexChanged );
  connect( mAnnotationDirectionComboBoxBottom, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerMapGridWidget::mAnnotationDirectionComboBoxBottom_currentIndexChanged );
  connect( mAnnotationFormatComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerMapGridWidget::mAnnotationFormatComboBox_currentIndexChanged );
  connect( mCoordinatePrecisionSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsComposerMapGridWidget::mCoordinatePrecisionSpinBox_valueChanged );
  connect( mDistanceToMapFrameSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerMapGridWidget::mDistanceToMapFrameSpinBox_valueChanged );
  connect( mAnnotationFontColorButton, &QgsColorButton::colorChanged, this, &QgsComposerMapGridWidget::mAnnotationFontColorButton_colorChanged );
  setPanelTitle( tr( "Map grid properties" ) );

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

  mAnnotationFormatComboBox->addItem( tr( "Decimal" ), QgsComposerMapGrid::Decimal );
  mAnnotationFormatComboBox->addItem( tr( "Decimal with suffix" ), QgsComposerMapGrid::DecimalWithSuffix );
  mAnnotationFormatComboBox->addItem( tr( "Degree, minute" ), QgsComposerMapGrid::DegreeMinuteNoSuffix );
  mAnnotationFormatComboBox->addItem( tr( "Degree, minute with suffix" ), QgsComposerMapGrid::DegreeMinute );
  mAnnotationFormatComboBox->addItem( tr( "Degree, minute aligned" ), QgsComposerMapGrid::DegreeMinutePadded );
  mAnnotationFormatComboBox->addItem( tr( "Degree, minute, second" ), QgsComposerMapGrid::DegreeMinuteSecondNoSuffix );
  mAnnotationFormatComboBox->addItem( tr( "Degree, minute, second with suffix" ), QgsComposerMapGrid::DegreeMinuteSecond );
  mAnnotationFormatComboBox->addItem( tr( "Degree, minute, second aligned" ), QgsComposerMapGrid::DegreeMinuteSecondPadded );
  mAnnotationFormatComboBox->addItem( tr( "Custom" ), QgsComposerMapGrid::CustomFormat );

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

  //set initial state of frame style controls
  toggleFrameControls( false, false, false );

  updateGuiElements();

  blockAllSignals( false );
  connect( mAnnotationFontButton, &QgsFontButton::changed, this, &QgsComposerMapGridWidget::annotationFontChanged );
}

void QgsComposerMapGridWidget::populateDataDefinedButtons()
{
  // none for now
}


void QgsComposerMapGridWidget::updateGridLineStyleFromWidget()
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  QgsSymbolSelectorWidget *w = qobject_cast<QgsSymbolSelectorWidget *>( sender() );
  mComposerMapGrid->setLineSymbol( dynamic_cast< QgsLineSymbol * >( w->symbol()->clone() ) );
  mComposerMap->update();
}

void QgsComposerMapGridWidget::cleanUpGridLineStyleSelector( QgsPanelWidget *container )
{
  QgsSymbolSelectorWidget *w = qobject_cast<QgsSymbolSelectorWidget *>( container );
  if ( !w )
    return;

  delete w->symbol();

  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  updateGridLineSymbolMarker();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::updateGridMarkerStyleFromWidget()
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  QgsSymbolSelectorWidget *w = qobject_cast<QgsSymbolSelectorWidget *>( sender() );
  mComposerMapGrid->setMarkerSymbol( dynamic_cast< QgsMarkerSymbol * >( w->symbol()->clone() ) );
  mComposerMap->update();
}

void QgsComposerMapGridWidget::cleanUpGridMarkerStyleSelector( QgsPanelWidget *container )
{
  QgsSymbolSelectorWidget *w = qobject_cast<QgsSymbolSelectorWidget *>( container );
  if ( !w )
    return;

  delete w->symbol();

  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  updateGridMarkerSymbolMarker();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::setGuiElementValues()
{
  updateGuiElements();
}

void QgsComposerMapGridWidget::updateGuiElements()
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  blockAllSignals( true );
  populateDataDefinedButtons();
  setGridItems();
  blockAllSignals( false );
}

void QgsComposerMapGridWidget::blockAllSignals( bool block )
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

void QgsComposerMapGridWidget::handleChangedFrameDisplay( QgsComposerMapGrid::BorderSide border, const QgsComposerMapGrid::DisplayMode mode )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Frame divisions changed" ) );
  mComposerMapGrid->setFrameDivisions( mode, border );
  mComposerMap->endCommand();
  mComposerMap->updateBoundingRect();
}

void QgsComposerMapGridWidget::handleChangedAnnotationDisplay( QgsComposerMapGrid::BorderSide border, const QString &text )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Annotation display changed" ) );
  if ( text == tr( "Show all" ) )
  {
    mComposerMapGrid->setAnnotationDisplay( QgsComposerMapGrid::ShowAll, border );
  }
  else if ( text == tr( "Show latitude only" ) )
  {
    mComposerMapGrid->setAnnotationDisplay( QgsComposerMapGrid::LatitudeOnly, border );
  }
  else if ( text == tr( "Show longitude only" ) )
  {
    mComposerMapGrid->setAnnotationDisplay( QgsComposerMapGrid::LongitudeOnly, border );
  }
  else //disabled
  {
    mComposerMapGrid->setAnnotationDisplay( QgsComposerMapGrid::HideAll, border );
  }

  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::toggleFrameControls( bool frameEnabled, bool frameFillEnabled, bool frameSizeEnabled )
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

void QgsComposerMapGridWidget::insertAnnotationPositionEntries( QComboBox *c )
{
  c->insertItem( 0, tr( "Inside frame" ) );
  c->insertItem( 1, tr( "Outside frame" ) );
}

void QgsComposerMapGridWidget::insertAnnotationDirectionEntries( QComboBox *c )
{
  c->addItem( tr( "Horizontal" ), QgsComposerMapGrid::Horizontal );
  c->addItem( tr( "Vertical ascending" ), QgsComposerMapGrid::Vertical );
  c->addItem( tr( "Vertical descending" ), QgsComposerMapGrid::VerticalDescending );
}

void QgsComposerMapGridWidget::initFrameDisplayBox( QComboBox *c, QgsComposerMapGrid::DisplayMode display )
{
  if ( !c )
  {
    return;
  }
  c->setCurrentIndex( c->findData( display ) );
}

void QgsComposerMapGridWidget::initAnnotationDisplayBox( QComboBox *c, QgsComposerMapGrid::DisplayMode display )
{
  if ( !c )
  {
    return;
  }

  if ( display == QgsComposerMapGrid::ShowAll )
  {
    c->setCurrentIndex( c->findText( tr( "Show all" ) ) );
  }
  else if ( display == QgsComposerMapGrid::LatitudeOnly )
  {
    c->setCurrentIndex( c->findText( tr( "Show latitude only" ) ) );
  }
  else if ( display == QgsComposerMapGrid::LongitudeOnly )
  {
    c->setCurrentIndex( c->findText( tr( "Show longitude only" ) ) );
  }
  else
  {
    c->setCurrentIndex( c->findText( tr( "Disabled" ) ) );
  }
}

void QgsComposerMapGridWidget::handleChangedAnnotationPosition( QgsComposerMapGrid::BorderSide border, const QString &text )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Annotation position changed" ) );
  if ( text == tr( "Inside frame" ) )
  {
    mComposerMapGrid->setAnnotationPosition( QgsComposerMapGrid::InsideMapFrame, border );
  }
  else //Outside frame
  {
    mComposerMapGrid->setAnnotationPosition( QgsComposerMapGrid::OutsideMapFrame, border );
  }

  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::handleChangedAnnotationDirection( QgsComposerMapGrid::BorderSide border, QgsComposerMapGrid::AnnotationDirection direction )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Changed annotation direction" ) );
  mComposerMapGrid->setAnnotationDirection( direction, border );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::insertFrameDisplayEntries( QComboBox *c )
{
  c->addItem( tr( "All" ), QgsComposerMapGrid::ShowAll );
  c->addItem( tr( "Latitude/Y only" ), QgsComposerMapGrid::LatitudeOnly );
  c->addItem( tr( "Longitude/X only" ), QgsComposerMapGrid::LongitudeOnly );
}

void QgsComposerMapGridWidget::insertAnnotationDisplayEntries( QComboBox *c )
{
  c->insertItem( 0, tr( "Show all" ) );
  c->insertItem( 1, tr( "Show latitude only" ) );
  c->insertItem( 2, tr( "Show longitude only" ) );
  c->insertItem( 3, tr( "Disabled" ) );
}

void QgsComposerMapGridWidget::initAnnotationPositionBox( QComboBox *c, QgsComposerMapGrid::AnnotationPosition pos )
{
  if ( !c )
  {
    return;
  }

  if ( pos == QgsComposerMapGrid::InsideMapFrame )
  {
    c->setCurrentIndex( c->findText( tr( "Inside frame" ) ) );
  }
  else
  {
    c->setCurrentIndex( c->findText( tr( "Outside frame" ) ) );
  }
}

void QgsComposerMapGridWidget::initAnnotationDirectionBox( QComboBox *c, QgsComposerMapGrid::AnnotationDirection dir )
{
  if ( !c )
  {
    return;
  }
  c->setCurrentIndex( c->findData( dir ) );
}

bool QgsComposerMapGridWidget::hasPredefinedScales() const
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

void QgsComposerMapGridWidget::setGridItems()
{
  if ( !mComposerMapGrid )
  {
    return;
  }

  mIntervalXSpinBox->setValue( mComposerMapGrid->intervalX() );
  mIntervalYSpinBox->setValue( mComposerMapGrid->intervalY() );
  mOffsetXSpinBox->setValue( mComposerMapGrid->offsetX() );
  mOffsetYSpinBox->setValue( mComposerMapGrid->offsetY() );
  mCrossWidthSpinBox->setValue( mComposerMapGrid->crossLength() );
  mFrameWidthSpinBox->setValue( mComposerMapGrid->frameWidth() );
  mGridFramePenSizeSpinBox->setValue( mComposerMapGrid->framePenSize() );
  mGridFramePenColorButton->setColor( mComposerMapGrid->framePenColor() );
  mGridFrameFill1ColorButton->setColor( mComposerMapGrid->frameFillColor1() );
  mGridFrameFill2ColorButton->setColor( mComposerMapGrid->frameFillColor2() );

  QgsComposerMapGrid::GridStyle gridStyle = mComposerMapGrid->style();
  switch ( gridStyle )
  {
    case QgsComposerMapGrid::Cross:
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
    case QgsComposerMapGrid::Markers:
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
    case QgsComposerMapGrid::Solid:
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
    case QgsComposerMapGrid::FrameAnnotationsOnly:
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
  mFrameWidthSpinBox->setValue( mComposerMapGrid->frameWidth() );
  QgsComposerMapGrid::FrameStyle gridFrameStyle = mComposerMapGrid->frameStyle();
  switch ( gridFrameStyle )
  {
    case QgsComposerMapGrid::Zebra:
      mFrameStyleComboBox->setCurrentIndex( 1 );
      toggleFrameControls( true, true, true );
      break;
    case QgsComposerMapGrid::InteriorTicks:
      mFrameStyleComboBox->setCurrentIndex( 2 );
      toggleFrameControls( true, false, true );
      break;
    case QgsComposerMapGrid::ExteriorTicks:
      mFrameStyleComboBox->setCurrentIndex( 3 );
      toggleFrameControls( true, false, true );
      break;
    case QgsComposerMapGrid::InteriorExteriorTicks:
      mFrameStyleComboBox->setCurrentIndex( 4 );
      toggleFrameControls( true, false, true );
      break;
    case QgsComposerMapGrid::LineBorder:
      mFrameStyleComboBox->setCurrentIndex( 5 );
      toggleFrameControls( true, false, false );
      break;
    default:
      mFrameStyleComboBox->setCurrentIndex( 0 );
      toggleFrameControls( false, false, false );
      break;
  }

  mCheckGridLeftSide->setChecked( mComposerMapGrid->testFrameSideFlag( QgsComposerMapGrid::FrameLeft ) );
  mCheckGridRightSide->setChecked( mComposerMapGrid->testFrameSideFlag( QgsComposerMapGrid::FrameRight ) );
  mCheckGridTopSide->setChecked( mComposerMapGrid->testFrameSideFlag( QgsComposerMapGrid::FrameTop ) );
  mCheckGridBottomSide->setChecked( mComposerMapGrid->testFrameSideFlag( QgsComposerMapGrid::FrameBottom ) );

  initFrameDisplayBox( mFrameDivisionsLeftComboBox, mComposerMapGrid->frameDivisions( QgsComposerMapGrid::Left ) );
  initFrameDisplayBox( mFrameDivisionsRightComboBox, mComposerMapGrid->frameDivisions( QgsComposerMapGrid::Right ) );
  initFrameDisplayBox( mFrameDivisionsTopComboBox, mComposerMapGrid->frameDivisions( QgsComposerMapGrid::Top ) );
  initFrameDisplayBox( mFrameDivisionsBottomComboBox, mComposerMapGrid->frameDivisions( QgsComposerMapGrid::Bottom ) );

  //line style
  updateGridLineSymbolMarker();
  //marker style
  updateGridMarkerSymbolMarker();

  mGridBlendComboBox->setBlendMode( mComposerMapGrid->blendMode() );

  mDrawAnnotationGroupBox->setChecked( mComposerMapGrid->annotationEnabled() );
  initAnnotationDisplayBox( mAnnotationDisplayLeftComboBox, mComposerMapGrid->annotationDisplay( QgsComposerMapGrid::Left ) );
  initAnnotationDisplayBox( mAnnotationDisplayRightComboBox, mComposerMapGrid->annotationDisplay( QgsComposerMapGrid::Right ) );
  initAnnotationDisplayBox( mAnnotationDisplayTopComboBox, mComposerMapGrid->annotationDisplay( QgsComposerMapGrid::Top ) );
  initAnnotationDisplayBox( mAnnotationDisplayBottomComboBox, mComposerMapGrid->annotationDisplay( QgsComposerMapGrid::Bottom ) );

  initAnnotationPositionBox( mAnnotationPositionLeftComboBox, mComposerMapGrid->annotationPosition( QgsComposerMapGrid::Left ) );
  initAnnotationPositionBox( mAnnotationPositionRightComboBox, mComposerMapGrid->annotationPosition( QgsComposerMapGrid::Right ) );
  initAnnotationPositionBox( mAnnotationPositionTopComboBox, mComposerMapGrid->annotationPosition( QgsComposerMapGrid::Top ) );
  initAnnotationPositionBox( mAnnotationPositionBottomComboBox, mComposerMapGrid->annotationPosition( QgsComposerMapGrid::Bottom ) );

  initAnnotationDirectionBox( mAnnotationDirectionComboBoxLeft, mComposerMapGrid->annotationDirection( QgsComposerMapGrid::Left ) );
  initAnnotationDirectionBox( mAnnotationDirectionComboBoxRight, mComposerMapGrid->annotationDirection( QgsComposerMapGrid::Right ) );
  initAnnotationDirectionBox( mAnnotationDirectionComboBoxTop, mComposerMapGrid->annotationDirection( QgsComposerMapGrid::Top ) );
  initAnnotationDirectionBox( mAnnotationDirectionComboBoxBottom, mComposerMapGrid->annotationDirection( QgsComposerMapGrid::Bottom ) );

  mAnnotationFontColorButton->setColor( mComposerMapGrid->annotationFontColor() );
  mAnnotationFontButton->setCurrentFont( mComposerMapGrid->annotationFont() );

  mAnnotationFormatComboBox->setCurrentIndex( mAnnotationFormatComboBox->findData( mComposerMapGrid->annotationFormat() ) );
  mAnnotationFormatButton->setEnabled( mComposerMapGrid->annotationFormat() == QgsComposerMapGrid::CustomFormat );
  mDistanceToMapFrameSpinBox->setValue( mComposerMapGrid->annotationFrameDistance() );
  mCoordinatePrecisionSpinBox->setValue( mComposerMapGrid->annotationPrecision() );

  //Unit
  QgsComposerMapGrid::GridUnit gridUnit = mComposerMapGrid->units();
  if ( gridUnit == QgsComposerMapGrid::MapUnit )
  {
    mMapGridUnitComboBox->setCurrentIndex( mMapGridUnitComboBox->findText( tr( "Map unit" ) ) );
  }
  else if ( gridUnit == QgsComposerMapGrid::MM )
  {
    mMapGridUnitComboBox->setCurrentIndex( mMapGridUnitComboBox->findText( tr( "Millimeter" ) ) );
  }
  else if ( gridUnit == QgsComposerMapGrid::CM )
  {
    mMapGridUnitComboBox->setCurrentIndex( mMapGridUnitComboBox->findText( tr( "Centimeter" ) ) );
  }

  //CRS button
  QgsCoordinateReferenceSystem gridCrs = mComposerMapGrid->crs();
  QString crsButtonText = gridCrs.isValid() ? gridCrs.authid() : tr( "change..." );
  mMapGridCRSButton->setText( crsButtonText );
}

void QgsComposerMapGridWidget::updateGridLineSymbolMarker()
{
  if ( mComposerMapGrid )
  {
    QgsLineSymbol *nonConstSymbol = const_cast<QgsLineSymbol *>( mComposerMapGrid->lineSymbol() ); //bad
    QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( nonConstSymbol, mGridLineStyleButton->iconSize() );
    mGridLineStyleButton->setIcon( icon );
  }
}

void QgsComposerMapGridWidget::updateGridMarkerSymbolMarker()
{
  if ( mComposerMapGrid )
  {
    QgsMarkerSymbol *nonConstSymbol = const_cast<QgsMarkerSymbol *>( mComposerMapGrid->markerSymbol() ); //bad
    QIcon icon = QgsSymbolLayerUtils::symbolPreviewIcon( nonConstSymbol, mGridMarkerStyleButton->iconSize() );
    mGridMarkerStyleButton->setIcon( icon );
  }
}

void QgsComposerMapGridWidget::mGridLineStyleButton_clicked()
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  // use the atlas coverage layer, if any
  QgsVectorLayer *coverageLayer = atlasCoverageLayer();

  QgsLineSymbol *newSymbol = static_cast<QgsLineSymbol *>( mComposerMapGrid->lineSymbol()->clone() );
  QgsExpressionContext context = mComposerMap->createExpressionContext();

  QgsSymbolSelectorWidget *d = new QgsSymbolSelectorWidget( newSymbol, QgsStyle::defaultStyle(), coverageLayer, nullptr );
  QgsSymbolWidgetContext symbolContext;
  symbolContext.setExpressionContext( &context );
  d->setContext( symbolContext );

  connect( d, &QgsPanelWidget::widgetChanged, this, &QgsComposerMapGridWidget::updateGridLineStyleFromWidget );
  connect( d, &QgsPanelWidget::panelAccepted, this, &QgsComposerMapGridWidget::cleanUpGridLineStyleSelector );
  openPanel( d );
  mComposerMap->beginCommand( tr( "Grid line style changed" ) );
}

void QgsComposerMapGridWidget::mGridMarkerStyleButton_clicked()
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  // use the atlas coverage layer, if any
  QgsVectorLayer *coverageLayer = atlasCoverageLayer();

  QgsMarkerSymbol *newSymbol = static_cast<QgsMarkerSymbol *>( mComposerMapGrid->markerSymbol()->clone() );
  QgsExpressionContext context = mComposerMap->createExpressionContext();

  QgsSymbolSelectorWidget *d = new QgsSymbolSelectorWidget( newSymbol, QgsStyle::defaultStyle(), coverageLayer, nullptr );
  QgsSymbolWidgetContext symbolContext;
  symbolContext.setExpressionContext( &context );
  d->setContext( symbolContext );

  connect( d, &QgsPanelWidget::widgetChanged, this, &QgsComposerMapGridWidget::updateGridMarkerStyleFromWidget );
  connect( d, &QgsPanelWidget::panelAccepted, this, &QgsComposerMapGridWidget::cleanUpGridMarkerStyleSelector );
  openPanel( d );
  mComposerMap->beginCommand( tr( "Grid markers style changed" ) );
}

void QgsComposerMapGridWidget::mIntervalXSpinBox_editingFinished()
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid interval changed" ) );
  mComposerMapGrid->setIntervalX( mIntervalXSpinBox->value() );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::mIntervalYSpinBox_editingFinished()
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid interval changed" ) );
  mComposerMapGrid->setIntervalY( mIntervalYSpinBox->value() );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::mOffsetXSpinBox_valueChanged( double value )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid offset changed" ) );
  mComposerMapGrid->setOffsetX( value );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::mOffsetYSpinBox_valueChanged( double value )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid offset changed" ) );
  mComposerMapGrid->setOffsetY( value );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::mCrossWidthSpinBox_valueChanged( double val )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Cross width changed" ) );
  mComposerMapGrid->setCrossLength( val );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::mFrameWidthSpinBox_valueChanged( double val )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Frame width changed" ) );
  mComposerMapGrid->setFrameWidth( val );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::mCheckGridLeftSide_toggled( bool checked )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Frame left side changed" ) );
  mComposerMapGrid->setFrameSideFlag( QgsComposerMapGrid::FrameLeft, checked );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::mCheckGridRightSide_toggled( bool checked )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Frame right side changed" ) );
  mComposerMapGrid->setFrameSideFlag( QgsComposerMapGrid::FrameRight, checked );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::mCheckGridTopSide_toggled( bool checked )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Frame top side changed" ) );
  mComposerMapGrid->setFrameSideFlag( QgsComposerMapGrid::FrameTop, checked );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::mCheckGridBottomSide_toggled( bool checked )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Frame bottom side changed" ) );
  mComposerMapGrid->setFrameSideFlag( QgsComposerMapGrid::FrameBottom, checked );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::mFrameDivisionsLeftComboBox_currentIndexChanged( int index )
{
  handleChangedFrameDisplay( QgsComposerMapGrid::Left, ( QgsComposerMapGrid::DisplayMode ) mFrameDivisionsLeftComboBox->itemData( index ).toInt() );
}

void QgsComposerMapGridWidget::mFrameDivisionsRightComboBox_currentIndexChanged( int index )
{
  handleChangedFrameDisplay( QgsComposerMapGrid::Right, ( QgsComposerMapGrid::DisplayMode ) mFrameDivisionsRightComboBox->itemData( index ).toInt() );
}

void QgsComposerMapGridWidget::mFrameDivisionsTopComboBox_currentIndexChanged( int index )
{
  handleChangedFrameDisplay( QgsComposerMapGrid::Top, ( QgsComposerMapGrid::DisplayMode ) mFrameDivisionsTopComboBox->itemData( index ).toInt() );
}

void QgsComposerMapGridWidget::mFrameDivisionsBottomComboBox_currentIndexChanged( int index )
{
  handleChangedFrameDisplay( QgsComposerMapGrid::Bottom, ( QgsComposerMapGrid::DisplayMode ) mFrameDivisionsBottomComboBox->itemData( index ).toInt() );
}

void QgsComposerMapGridWidget::mGridFramePenSizeSpinBox_valueChanged( double d )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Changed grid frame line thickness" ) );
  mComposerMapGrid->setFramePenSize( d );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::mGridFramePenColorButton_colorChanged( const QColor &newColor )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid frame color changed" ), QgsComposerMergeCommand::ComposerMapGridFramePenColor );
  mComposerMapGrid->setFramePenColor( newColor );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::mGridFrameFill1ColorButton_colorChanged( const QColor &newColor )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid frame first fill color changed" ), QgsComposerMergeCommand::ComposerMapGridFrameFill1Color );
  mComposerMapGrid->setFrameFillColor1( newColor );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::mGridFrameFill2ColorButton_colorChanged( const QColor &newColor )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid frame second fill color changed" ), QgsComposerMergeCommand::ComposerMapGridFrameFill2Color );
  mComposerMapGrid->setFrameFillColor2( newColor );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::mFrameStyleComboBox_currentIndexChanged( const QString &text )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Changed grid frame style" ) );
  if ( text == tr( "Zebra" ) )
  {
    mComposerMapGrid->setFrameStyle( QgsComposerMapGrid::Zebra );
    toggleFrameControls( true, true, true );
  }
  else if ( text == tr( "Interior ticks" ) )
  {
    mComposerMapGrid->setFrameStyle( QgsComposerMapGrid::InteriorTicks );
    toggleFrameControls( true, false, true );
  }
  else if ( text == tr( "Exterior ticks" ) )
  {
    mComposerMapGrid->setFrameStyle( QgsComposerMapGrid::ExteriorTicks );
    toggleFrameControls( true, false, true );
  }
  else if ( text == tr( "Interior and exterior ticks" ) )
  {
    mComposerMapGrid->setFrameStyle( QgsComposerMapGrid::InteriorExteriorTicks );
    toggleFrameControls( true, false, true );
  }
  else if ( text == tr( "Line border" ) )
  {
    mComposerMapGrid->setFrameStyle( QgsComposerMapGrid::LineBorder );
    toggleFrameControls( true, false, false );
  }
  else //no frame
  {
    mComposerMapGrid->setFrameStyle( QgsComposerMapGrid::NoFrame );
    toggleFrameControls( false, false, false );
  }
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::mMapGridUnitComboBox_currentIndexChanged( const QString &text )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Changed grid unit" ) );
  if ( text == tr( "Map unit" ) )
  {
    mComposerMapGrid->setUnits( QgsComposerMapGrid::MapUnit );
  }
  else if ( text == tr( "Millimeter" ) )
  {
    mComposerMapGrid->setUnits( QgsComposerMapGrid::MM );
  }
  else if ( text == tr( "Centimeter" ) )
  {
    mComposerMapGrid->setUnits( QgsComposerMapGrid::CM );
  }
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::mGridBlendComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  if ( mComposerMapGrid )
  {
    mComposerMap->beginCommand( tr( "Grid blend mode changed" ) );
    mComposerMapGrid->setBlendMode( mGridBlendComboBox->blendMode() );
    mComposerMap->update();
    mComposerMap->endCommand();
  }

}

void QgsComposerMapGridWidget::mGridTypeComboBox_currentIndexChanged( const QString &text )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid type changed" ) );
  if ( text == tr( "Cross" ) )
  {
    mComposerMapGrid->setStyle( QgsComposerMapGrid::Cross );
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
    mComposerMapGrid->setStyle( QgsComposerMapGrid::Markers );
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
    mComposerMapGrid->setStyle( QgsComposerMapGrid::Solid );
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
    mComposerMapGrid->setStyle( QgsComposerMapGrid::FrameAnnotationsOnly );
    mCrossWidthSpinBox->setVisible( false );
    mCrossWidthLabel->setVisible( false );
    mGridLineStyleButton->setVisible( false );
    mLineStyleLabel->setVisible( false );
    mGridMarkerStyleButton->setVisible( false );
    mMarkerStyleLabel->setVisible( false );
    mGridBlendComboBox->setVisible( false );
    mGridBlendLabel->setVisible( false );
  }

  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::mMapGridCRSButton_clicked()
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  QgsProjectionSelectionDialog crsDialog( this );
  QgsCoordinateReferenceSystem crs = mComposerMapGrid->crs();
  crsDialog.setCrs( crs.isValid() ? crs : mComposerMap->crs() );

  if ( crsDialog.exec() == QDialog::Accepted )
  {
    mComposerMap->beginCommand( tr( "Grid CRS changed" ) );
    mComposerMapGrid->setCrs( crsDialog.crs() );
    mComposerMap->updateBoundingRect();
    mMapGridCRSButton->setText( crsDialog.crs().authid() );
    mComposerMap->endCommand();
  }
}

void QgsComposerMapGridWidget::mDrawAnnotationGroupBox_toggled( bool state )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Annotation toggled" ) );
  mComposerMapGrid->setAnnotationEnabled( state );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::mAnnotationFormatButton_clicked()
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  QgsExpressionContext expressionContext = mComposerMapGrid->createExpressionContext();

  QgsExpressionBuilderDialog exprDlg( nullptr, mComposerMapGrid->annotationExpression(), this, QStringLiteral( "generic" ), expressionContext );
  exprDlg.setWindowTitle( tr( "Expression Based Annotation" ) );

  if ( exprDlg.exec() == QDialog::Accepted )
  {
    QString expression = exprDlg.expressionText();
    mComposerMap->beginCommand( tr( "Annotation format changed" ) );
    mComposerMapGrid->setAnnotationExpression( expression );
    mComposerMap->updateBoundingRect();
    mComposerMap->update();
    mComposerMap->endCommand();
  }
}

void QgsComposerMapGridWidget::mAnnotationDisplayLeftComboBox_currentIndexChanged( const QString &text )
{
  handleChangedAnnotationDisplay( QgsComposerMapGrid::Left, text );
}

void QgsComposerMapGridWidget::mAnnotationDisplayRightComboBox_currentIndexChanged( const QString &text )
{
  handleChangedAnnotationDisplay( QgsComposerMapGrid::Right, text );
}

void QgsComposerMapGridWidget::mAnnotationDisplayTopComboBox_currentIndexChanged( const QString &text )
{
  handleChangedAnnotationDisplay( QgsComposerMapGrid::Top, text );
}

void QgsComposerMapGridWidget::mAnnotationDisplayBottomComboBox_currentIndexChanged( const QString &text )
{
  handleChangedAnnotationDisplay( QgsComposerMapGrid::Bottom, text );
}

void QgsComposerMapGridWidget::mAnnotationPositionLeftComboBox_currentIndexChanged( const QString &text )
{
  handleChangedAnnotationPosition( QgsComposerMapGrid::Left, text );
}

void QgsComposerMapGridWidget::mAnnotationPositionRightComboBox_currentIndexChanged( const QString &text )
{
  handleChangedAnnotationPosition( QgsComposerMapGrid::Right, text );
}

void QgsComposerMapGridWidget::mAnnotationPositionTopComboBox_currentIndexChanged( const QString &text )
{
  handleChangedAnnotationPosition( QgsComposerMapGrid::Top, text );
}

void QgsComposerMapGridWidget::mAnnotationPositionBottomComboBox_currentIndexChanged( const QString &text )
{
  handleChangedAnnotationPosition( QgsComposerMapGrid::Bottom, text );
}

void QgsComposerMapGridWidget::mAnnotationDirectionComboBoxLeft_currentIndexChanged( int index )
{
  handleChangedAnnotationDirection( QgsComposerMapGrid::Left, ( QgsComposerMapGrid::AnnotationDirection ) mAnnotationDirectionComboBoxLeft->itemData( index ).toInt() );
}

void QgsComposerMapGridWidget::mAnnotationDirectionComboBoxRight_currentIndexChanged( int index )
{
  handleChangedAnnotationDirection( QgsComposerMapGrid::Right, ( QgsComposerMapGrid::AnnotationDirection ) mAnnotationDirectionComboBoxRight->itemData( index ).toInt() );
}

void QgsComposerMapGridWidget::mAnnotationDirectionComboBoxTop_currentIndexChanged( int index )
{
  handleChangedAnnotationDirection( QgsComposerMapGrid::Top, ( QgsComposerMapGrid::AnnotationDirection ) mAnnotationDirectionComboBoxTop->itemData( index ).toInt() );
}

void QgsComposerMapGridWidget::mAnnotationDirectionComboBoxBottom_currentIndexChanged( int index )
{
  handleChangedAnnotationDirection( QgsComposerMapGrid::Bottom, ( QgsComposerMapGrid::AnnotationDirection ) mAnnotationDirectionComboBoxBottom->itemData( index ).toInt() );
}

void QgsComposerMapGridWidget::mDistanceToMapFrameSpinBox_valueChanged( double d )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Annotation distance changed" ), QgsComposerMergeCommand::ComposerMapAnnotationDistance );
  mComposerMapGrid->setAnnotationFrameDistance( d );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::annotationFontChanged()
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Annotation font changed" ) );
  mComposerMapGrid->setAnnotationFont( mAnnotationFontButton->currentFont() );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::mAnnotationFontColorButton_colorChanged( const QColor &color )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Annotation color changed" ), QgsComposerMergeCommand::ComposerMapGridAnnotationFontColor );
  mComposerMapGrid->setAnnotationFontColor( color );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::mAnnotationFormatComboBox_currentIndexChanged( int index )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Annotation format changed" ) );

  mComposerMapGrid->setAnnotationFormat( ( QgsComposerMapGrid::AnnotationFormat )mAnnotationFormatComboBox->itemData( index ).toInt() );
  mAnnotationFormatButton->setEnabled( mComposerMapGrid->annotationFormat() == QgsComposerMapGrid::CustomFormat );

  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapGridWidget::mCoordinatePrecisionSpinBox_valueChanged( int value )
{
  if ( !mComposerMapGrid || !mComposerMap )
  {
    return;
  }
  mComposerMap->beginCommand( tr( "Changed annotation precision" ) );
  mComposerMapGrid->setAnnotationPrecision( value );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}
