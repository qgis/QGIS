/***************************************************************************
    qgstextformatwidget.h
    ---------------------
    begin                : June 2009
    copyright            : (C) Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstextformatwidget.h"
#include "qgsmapcanvas.h"
#include "qgscharacterselectordialog.h"
#include "qgslogger.h"
#include "qgsfontutils.h"
#include "qgssymbollayerutils.h"
#include "qgssvgcache.h"
#include "qgssvgselectorwidget.h"
#include "qgssubstitutionlistwidget.h"
#include "qgspallabeling.h" // for enum values
#include "qgspathresolver.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgseffectstack.h"
#include "qgspainteffectregistry.h"

#include <QButtonGroup>

QgsTextFormatWidget::QgsTextFormatWidget( const QgsTextFormat &format, QgsMapCanvas *mapCanvas, QWidget *parent )
  : QWidget( parent )
  , mMapCanvas( mapCanvas )
{
  initWidget();
  setWidgetMode( Text );
  updateWidgetForFormat( format );
}

QgsTextFormatWidget::QgsTextFormatWidget( QgsMapCanvas *mapCanvas, QWidget *parent, Mode mode )
  : QWidget( parent )
  , mWidgetMode( mode )
  , mMapCanvas( mapCanvas )
{
  initWidget();
  setWidgetMode( mode );
}

void QgsTextFormatWidget::initWidget()
{
  setupUi( this );
  connect( mShapeSVGPathLineEdit, &QLineEdit::textChanged, this, &QgsTextFormatWidget::mShapeSVGPathLineEdit_textChanged );
  connect( mFontSizeSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsTextFormatWidget::mFontSizeSpinBox_valueChanged );
  connect( mFontCapitalsComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsTextFormatWidget::mFontCapitalsComboBox_currentIndexChanged );
  connect( mFontFamilyCmbBx, &QFontComboBox::currentFontChanged, this, &QgsTextFormatWidget::mFontFamilyCmbBx_currentFontChanged );
  connect( mFontStyleComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsTextFormatWidget::mFontStyleComboBox_currentIndexChanged );
  connect( mFontUnderlineBtn, &QToolButton::toggled, this, &QgsTextFormatWidget::mFontUnderlineBtn_toggled );
  connect( mFontStrikethroughBtn, &QToolButton::toggled, this, &QgsTextFormatWidget::mFontStrikethroughBtn_toggled );
  connect( mFontWordSpacingSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsTextFormatWidget::mFontWordSpacingSpinBox_valueChanged );
  connect( mFontLetterSpacingSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsTextFormatWidget::mFontLetterSpacingSpinBox_valueChanged );
  connect( mFontSizeUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsTextFormatWidget::mFontSizeUnitWidget_changed );
  connect( mFontMinPixelSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsTextFormatWidget::mFontMinPixelSpinBox_valueChanged );
  connect( mFontMaxPixelSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsTextFormatWidget::mFontMaxPixelSpinBox_valueChanged );
  connect( mBufferUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsTextFormatWidget::mBufferUnitWidget_changed );
  connect( mCoordXDDBtn, &QgsPropertyOverrideButton::activated, this, &QgsTextFormatWidget::mCoordXDDBtn_activated );
  connect( mCoordYDDBtn, &QgsPropertyOverrideButton::activated, this, &QgsTextFormatWidget::mCoordYDDBtn_activated );
  connect( mShapeTypeCmbBx, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsTextFormatWidget::mShapeTypeCmbBx_currentIndexChanged );
  connect( mShapeRotationCmbBx, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsTextFormatWidget::mShapeRotationCmbBx_currentIndexChanged );
  connect( mShapeSVGParamsBtn, &QPushButton::clicked, this, &QgsTextFormatWidget::mShapeSVGParamsBtn_clicked );
  connect( mShapeSVGSelectorBtn, &QPushButton::clicked, this, &QgsTextFormatWidget::mShapeSVGSelectorBtn_clicked );
  connect( mPreviewTextEdit, &QLineEdit::textChanged, this, &QgsTextFormatWidget::mPreviewTextEdit_textChanged );
  connect( mPreviewTextBtn, &QToolButton::clicked, this, &QgsTextFormatWidget::mPreviewTextBtn_clicked );
  connect( mPreviewBackgroundBtn, &QgsColorButton::colorChanged, this, &QgsTextFormatWidget::mPreviewBackgroundBtn_colorChanged );
  connect( mDirectSymbLeftToolBtn, &QToolButton::clicked, this, &QgsTextFormatWidget::mDirectSymbLeftToolBtn_clicked );
  connect( mDirectSymbRightToolBtn, &QToolButton::clicked, this, &QgsTextFormatWidget::mDirectSymbRightToolBtn_clicked );
  connect( mChkNoObstacle, &QCheckBox::toggled, this, &QgsTextFormatWidget::mChkNoObstacle_toggled );
  connect( chkLineOrientationDependent, &QCheckBox::toggled, this, &QgsTextFormatWidget::chkLineOrientationDependent_toggled );
  connect( mToolButtonConfigureSubstitutes, &QToolButton::clicked, this, &QgsTextFormatWidget::mToolButtonConfigureSubstitutes_clicked );

  const int iconSize = QgsGuiUtils::scaleIconSize( 20 );
  mOptionsTab->setIconSize( QSize( iconSize, iconSize ) );
  const int iconSize32 = QgsGuiUtils::scaleIconSize( 32 );
  const int iconSize24 = QgsGuiUtils::scaleIconSize( 24 );
  const int iconSize18 = QgsGuiUtils::scaleIconSize( 18 );
  mPointOffsetAboveLeft->setIconSize( QSize( iconSize32, iconSize18 ) );
  mPointOffsetAbove->setIconSize( QSize( iconSize32, iconSize18 ) );
  mPointOffsetAboveRight->setIconSize( QSize( iconSize32, iconSize18 ) );
  mPointOffsetLeft->setIconSize( QSize( iconSize32, iconSize18 ) );
  mPointOffsetOver ->setIconSize( QSize( iconSize32, iconSize18 ) );
  mPointOffsetRight->setIconSize( QSize( iconSize32, iconSize18 ) );
  mPointOffsetBelowLeft->setIconSize( QSize( iconSize32, iconSize18 ) );
  mPointOffsetBelow->setIconSize( QSize( iconSize32, iconSize18 ) );
  mPointOffsetBelowRight->setIconSize( QSize( iconSize32, iconSize18 ) );
  mLabelMinScale->setPixmap( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomOut.svg" ) ).pixmap( QSize( iconSize24, iconSize24 ) ) );
  mLabelMaxScale->setPixmap( QgsApplication::getThemeIcon( QStringLiteral( "/mActionZoomIn.svg" ) ).pixmap( QSize( iconSize24, iconSize24 ) ) );

  const int buttonSize = QgsGuiUtils::scaleIconSize( 24 );
  mFontUnderlineBtn->setMinimumSize( buttonSize, buttonSize );
  mFontUnderlineBtn->setMaximumSize( buttonSize, buttonSize );
  mFontStrikethroughBtn->setMinimumSize( buttonSize, buttonSize );
  mFontStrikethroughBtn->setMaximumSize( buttonSize, buttonSize );
  mFontBoldBtn->setMinimumSize( buttonSize, buttonSize );
  mFontBoldBtn->setMaximumSize( buttonSize, buttonSize );
  mFontItalicBtn->setMinimumSize( buttonSize, buttonSize );
  mFontItalicBtn->setMaximumSize( buttonSize, buttonSize );

  mPreviewScaleComboBox->setMapCanvas( mMapCanvas );
  mPreviewScaleComboBox->setShowCurrentScaleButton( true );
  connect( mPreviewScaleComboBox, &QgsScaleWidget::scaleChanged, this, &QgsTextFormatWidget::previewScaleChanged );

  Q_FOREACH ( QgsUnitSelectionWidget *unitWidget, findChildren<QgsUnitSelectionWidget *>() )
  {
    unitWidget->setMapCanvas( mMapCanvas );
  }
  mFontSizeUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits
                                 << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderPixels << QgsUnitTypes::RenderInches );
  mBufferUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                               << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mShapeSizeUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                  << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mShapeOffsetUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                    << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mShapeRadiusUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits
                                    << QgsUnitTypes::RenderPixels << QgsUnitTypes::RenderPercentage
                                    << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mShapeStrokeWidthUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                         << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mShadowOffsetUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                     << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mShadowRadiusUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                     << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mPointOffsetUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                    << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mLineDistanceUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                     << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mRepeatDistanceUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                       << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );

  mFontLineHeightSpinBox->setClearValue( 1.0 );
  mShapeRotationDblSpnBx->setClearValue( 0.0 );
  mShapeOffsetXSpnBx->setClearValue( 0.0 );
  mShapeOffsetYSpnBx->setClearValue( 0.0 );
  mPointOffsetXSpinBox->setClearValue( 0.0 );
  mPointOffsetYSpinBox->setClearValue( 0.0 );
  mPointAngleSpinBox->setClearValue( 0.0 );
  mFontLetterSpacingSpinBox->setClearValue( 0.0 );
  mFontWordSpacingSpinBox->setClearValue( 0.0 );
  mZIndexSpinBox->setClearValue( 0.0 );

  mObstacleTypeComboBox->addItem( tr( "Over the feature's interior" ), QgsPalLayerSettings::PolygonInterior );
  mObstacleTypeComboBox->addItem( tr( "Over the feature's boundary" ), QgsPalLayerSettings::PolygonBoundary );

  mOffsetTypeComboBox->addItem( tr( "From point" ), QgsPalLayerSettings::FromPoint );
  mOffsetTypeComboBox->addItem( tr( "From symbol bounds" ), QgsPalLayerSettings::FromSymbolBounds );

  mCharDlg = new QgsCharacterSelectorDialog( this );

  mRefFont = lblFontPreview->font();

  // internal connections
  connect( mShadowOffsetAngleDial, &QAbstractSlider::valueChanged, mShadowOffsetAngleSpnBx, &QSpinBox::setValue );
  connect( mShadowOffsetAngleSpnBx, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), mShadowOffsetAngleDial, &QAbstractSlider::setValue );
  connect( mLimitLabelChkBox, &QAbstractButton::toggled, mLimitLabelSpinBox, &QWidget::setEnabled );
  connect( mCheckBoxSubstituteText, &QAbstractButton::toggled, mToolButtonConfigureSubstitutes, &QWidget::setEnabled );

  //connections to prevent users removing all line placement positions
  connect( chkLineAbove, &QAbstractButton::toggled, this, &QgsTextFormatWidget::updateLinePlacementOptions );
  connect( chkLineBelow, &QAbstractButton::toggled, this, &QgsTextFormatWidget::updateLinePlacementOptions );
  connect( chkLineOn, &QAbstractButton::toggled, this, &QgsTextFormatWidget::updateLinePlacementOptions );

  populateFontCapitalsComboBox();

  // color buttons
  mPreviewBackgroundBtn->setColorDialogTitle( tr( "Select Fill Color" ) );
  mPreviewBackgroundBtn->setContext( QStringLiteral( "labeling" ) );
  mPreviewBackgroundBtn->setColor( QColor( 255, 255, 255 ) );
  btnTextColor->setColorDialogTitle( tr( "Select Text Color" ) );
  btnTextColor->setContext( QStringLiteral( "labeling" ) );
  btnTextColor->setDefaultColor( Qt::black );
  btnBufferColor->setColorDialogTitle( tr( "Select Buffer Color" ) );
  btnBufferColor->setContext( QStringLiteral( "labeling" ) );
  btnBufferColor->setDefaultColor( Qt::white );
  mShapeStrokeColorBtn->setColorDialogTitle( tr( "Select Stroke Color" ) );
  mShapeStrokeColorBtn->setContext( QStringLiteral( "labeling" ) );
  mShapeFillColorBtn->setColorDialogTitle( tr( "Select Fill Color" ) );
  mShapeFillColorBtn->setContext( QStringLiteral( "labeling" ) );
  mShadowColorBtn->setColorDialogTitle( tr( "Select Shadow Color" ) );
  mShadowColorBtn->setContext( QStringLiteral( "labeling" ) );
  mShadowColorBtn->setDefaultColor( Qt::black );

  mFontColorDDBtn->registerLinkedWidget( btnTextColor );
  mBufferColorDDBtn->registerLinkedWidget( btnBufferColor );
  mShapeStrokeColorDDBtn->registerLinkedWidget( mShapeStrokeColorBtn );
  mShapeFillColorDDBtn->registerLinkedWidget( mShapeFillColorBtn );
  mShadowColorDDBtn->registerLinkedWidget( mShadowColorBtn );

  // set up quadrant offset button group
  mQuadrantBtnGrp = new QButtonGroup( this );
  mQuadrantBtnGrp->addButton( mPointOffsetAboveLeft, static_cast<int>( QgsPalLayerSettings::QuadrantAboveLeft ) );
  mQuadrantBtnGrp->addButton( mPointOffsetAbove, static_cast<int>( QgsPalLayerSettings::QuadrantAbove ) );
  mQuadrantBtnGrp->addButton( mPointOffsetAboveRight, static_cast<int>( QgsPalLayerSettings::QuadrantAboveRight ) );
  mQuadrantBtnGrp->addButton( mPointOffsetLeft, static_cast<int>( QgsPalLayerSettings::QuadrantLeft ) );
  mQuadrantBtnGrp->addButton( mPointOffsetOver, static_cast<int>( QgsPalLayerSettings::QuadrantOver ) );
  mQuadrantBtnGrp->addButton( mPointOffsetRight, static_cast<int>( QgsPalLayerSettings::QuadrantRight ) );
  mQuadrantBtnGrp->addButton( mPointOffsetBelowLeft, static_cast<int>( QgsPalLayerSettings::QuadrantBelowLeft ) );
  mQuadrantBtnGrp->addButton( mPointOffsetBelow, static_cast<int>( QgsPalLayerSettings::QuadrantBelow ) );
  mQuadrantBtnGrp->addButton( mPointOffsetBelowRight, static_cast<int>( QgsPalLayerSettings::QuadrantBelowRight ) );
  mQuadrantBtnGrp->setExclusive( true );

  // setup direction symbol(s) button group
  mDirectSymbBtnGrp = new QButtonGroup( this );
  mDirectSymbBtnGrp->addButton( mDirectSymbRadioBtnLR, static_cast<int>( QgsPalLayerSettings::SymbolLeftRight ) );
  mDirectSymbBtnGrp->addButton( mDirectSymbRadioBtnAbove, static_cast<int>( QgsPalLayerSettings::SymbolAbove ) );
  mDirectSymbBtnGrp->addButton( mDirectSymbRadioBtnBelow, static_cast<int>( QgsPalLayerSettings::SymbolBelow ) );
  mDirectSymbBtnGrp->setExclusive( true );

  // upside-down labels button group
  mUpsidedownBtnGrp = new QButtonGroup( this );
  mUpsidedownBtnGrp->addButton( mUpsidedownRadioOff, static_cast<int>( QgsPalLayerSettings::Upright ) );
  mUpsidedownBtnGrp->addButton( mUpsidedownRadioDefined, static_cast<int>( QgsPalLayerSettings::ShowDefined ) );
  mUpsidedownBtnGrp->addButton( mUpsidedownRadioAll, static_cast<int>( QgsPalLayerSettings::ShowAll ) );
  mUpsidedownBtnGrp->setExclusive( true );

  //mShapeCollisionsChkBx->setVisible( false ); // until implemented

  // post updatePlacementWidgets() connections
  connect( chkLineAbove, &QAbstractButton::toggled, this, &QgsTextFormatWidget::updatePlacementWidgets );
  connect( chkLineBelow, &QAbstractButton::toggled, this, &QgsTextFormatWidget::updatePlacementWidgets );

  // setup point placement button group
  mPlacePointBtnGrp = new QButtonGroup( this );
  mPlacePointBtnGrp->addButton( radPredefinedOrder, static_cast<int>( QgsPalLayerSettings::OrderedPositionsAroundPoint ) );
  mPlacePointBtnGrp->addButton( radAroundPoint, static_cast<int>( QgsPalLayerSettings::AroundPoint ) );
  mPlacePointBtnGrp->addButton( radOverPoint, static_cast<int>( QgsPalLayerSettings::OverPoint ) );
  mPlacePointBtnGrp->setExclusive( true );
  connect( mPlacePointBtnGrp, static_cast<void ( QButtonGroup::* )( int )>( &QButtonGroup::buttonClicked ), this, &QgsTextFormatWidget::updatePlacementWidgets );

  // setup line placement button group (assigned enum id currently unused)
  mPlaceLineBtnGrp = new QButtonGroup( this );
  mPlaceLineBtnGrp->addButton( radLineParallel, static_cast<int>( QgsPalLayerSettings::Line ) );
  mPlaceLineBtnGrp->addButton( radLineCurved, static_cast<int>( QgsPalLayerSettings::Curved ) );
  mPlaceLineBtnGrp->addButton( radLineHorizontal, static_cast<int>( QgsPalLayerSettings::Horizontal ) );
  mPlaceLineBtnGrp->setExclusive( true );
  connect( mPlaceLineBtnGrp, static_cast<void ( QButtonGroup::* )( int )>( &QButtonGroup::buttonClicked ), this, &QgsTextFormatWidget::updatePlacementWidgets );

  // setup polygon placement button group (assigned enum id currently unused)
  mPlacePolygonBtnGrp = new QButtonGroup( this );
  mPlacePolygonBtnGrp->addButton( radOverCentroid, static_cast<int>( QgsPalLayerSettings::OverPoint ) );
  mPlacePolygonBtnGrp->addButton( radAroundCentroid, static_cast<int>( QgsPalLayerSettings::AroundPoint ) );
  mPlacePolygonBtnGrp->addButton( radPolygonHorizontal, static_cast<int>( QgsPalLayerSettings::Horizontal ) );
  mPlacePolygonBtnGrp->addButton( radPolygonFree, static_cast<int>( QgsPalLayerSettings::Free ) );
  mPlacePolygonBtnGrp->addButton( radPolygonPerimeter, static_cast<int>( QgsPalLayerSettings::Line ) );
  mPlacePolygonBtnGrp->addButton( radPolygonPerimeterCurved, static_cast<int>( QgsPalLayerSettings::PerimeterCurved ) );
  mPlacePolygonBtnGrp->setExclusive( true );
  connect( mPlacePolygonBtnGrp, static_cast<void ( QButtonGroup::* )( int )>( &QButtonGroup::buttonClicked ), this, &QgsTextFormatWidget::updatePlacementWidgets );

  // Global settings group for groupboxes' saved/restored collapsed state
  // maintains state across different dialogs
  Q_FOREACH ( QgsCollapsibleGroupBox *grpbox, findChildren<QgsCollapsibleGroupBox *>() )
  {
    grpbox->setSettingGroup( QStringLiteral( "mAdvLabelingDlg" ) );
  }

  connect( groupBox_mPreview,
           &QgsCollapsibleGroupBoxBasic::collapsedStateChanged,
           this,
           &QgsTextFormatWidget::collapseSample );

  // get rid of annoying outer focus rect on Mac
  mLabelingOptionsListWidget->setAttribute( Qt::WA_MacShowFocusRect, false );

  QgsSettings settings;

  // reset horiz stretch of left side of options splitter (set to 1 for previewing in Qt Designer)
  QSizePolicy policy( mLabelingOptionsListFrame->sizePolicy() );
  policy.setHorizontalStretch( 0 );
  mLabelingOptionsListFrame->setSizePolicy( policy );
  if ( !settings.contains( QStringLiteral( "/Windows/Labeling/OptionsSplitState" ) ) )
  {
    // set left list widget width on initial showing
    QList<int> splitsizes;
    splitsizes << 115;
    mLabelingOptionsSplitter->setSizes( splitsizes );
  }

  // set up reverse connection from stack to list
  connect( mLabelStackedWidget, &QStackedWidget::currentChanged, this, &QgsTextFormatWidget::optionsStackedWidget_CurrentChanged );

  // restore dialog, splitters and current tab
  mFontPreviewSplitter->restoreState( settings.value( QStringLiteral( "Windows/Labeling/FontPreviewSplitState" ) ).toByteArray() );
  mLabelingOptionsSplitter->restoreState( settings.value( QStringLiteral( "Windows/Labeling/OptionsSplitState" ) ).toByteArray() );

  mLabelingOptionsListWidget->setCurrentRow( settings.value( QStringLiteral( "Windows/Labeling/Tab" ), 0 ).toInt() );

  mBufferEffect.reset( QgsPaintEffectRegistry::defaultStack() );
  connect( mBufferEffectWidget, &QgsEffectStackCompactWidget::changed, this, &QgsTextFormatWidget::updatePreview );
  mBufferEffectWidget->setPaintEffect( mBufferEffect.get() );
  mBackgroundEffect.reset( QgsPaintEffectRegistry::defaultStack() );
  connect( mBackgroundEffectWidget, &QgsEffectStackCompactWidget::changed, this, &QgsTextFormatWidget::updatePreview );
  mBackgroundEffectWidget->setPaintEffect( mBackgroundEffect.get() );

  setDockMode( false );


  QList<QWidget *> widgets;
  widgets << btnBufferColor
          << btnTextColor
          << chkLabelPerFeaturePart
          << chkLineAbove
          << chkLineBelow
          << chkLineOn
          << chkLineOrientationDependent
          << chkMergeLines
          << chkPreserveRotation
          << comboBlendMode
          << comboBufferBlendMode
          << mAlwaysShowDDBtn
          << mBufferBlendModeDDBtn
          << mBufferColorDDBtn
          << mBufferDrawChkBx
          << mBufferDrawDDBtn
          << mBufferJoinStyleComboBox
          << mBufferJoinStyleDDBtn
          << mBufferSizeDDBtn
          << mBufferOpacityDDBtn
          << mBufferTranspFillChbx
          << mBufferOpacityWidget
          << mBufferUnitsDDBtn
          << mCentroidDDBtn
          << mCentroidInsideCheckBox
          << mChkNoObstacle
          << mCoordAlignmentHDDBtn
          << mCoordAlignmentVDDBtn
          << mCoordRotationDDBtn
          << mCoordXDDBtn
          << mCoordYDDBtn
          << mDirectSymbChkBx
          << mDirectSymbDDBtn
          << mDirectSymbLeftDDBtn
          << mDirectSymbLeftLineEdit
          << mDirectSymbPlacementDDBtn
          << mDirectSymbRevChkBx
          << mDirectSymbRevDDBtn
          << mDirectSymbRightDDBtn
          << mDirectSymbRightLineEdit
          << mFitInsidePolygonCheckBox
          << mFontBlendModeDDBtn
          << mFontBoldDDBtn
          << mFontCapitalsComboBox
          << mFontCaseDDBtn
          << mFontColorDDBtn
          << mFontDDBtn
          << mFontItalicDDBtn
          << mFontLetterSpacingDDBtn
          << mFontLetterSpacingSpinBox
          << mFontLimitPixelChkBox
          << mFontLimitPixelDDBtn
          << mFontLineHeightDDBtn
          << mFontLineHeightSpinBox
          << mFontMaxPixelDDBtn
          << mFontMaxPixelSpinBox
          << mFontMinPixelDDBtn
          << mFontMinPixelSpinBox
          << mFontMultiLineAlignComboBox
          << mFontMultiLineAlignDDBtn
          << mFontSizeDDBtn
          << mFontSizeSpinBox
          << mFontStrikeoutDDBtn
          << mFontStyleComboBox
          << mFontStyleDDBtn
          << mFontOpacityDDBtn
          << mTextOpacityWidget
          << mFontUnderlineDDBtn
          << mFontUnitsDDBtn
          << mFontWordSpacingDDBtn
          << mFontWordSpacingSpinBox
          << mFormatNumChkBx
          << mFormatNumDDBtn
          << mFormatNumDecimalsDDBtn
          << mFormatNumDecimalsSpnBx
          << mFormatNumPlusSignChkBx
          << mFormatNumPlusSignDDBtn
          << mIsObstacleDDBtn
          << mLimitLabelChkBox
          << mLimitLabelSpinBox
          << mLineDistanceDDBtn
          << mLineDistanceSpnBx
          << mLineDistanceUnitDDBtn
          << mLineDistanceUnitWidget
          << mMaxCharAngleDDBtn
          << mMaxCharAngleInDSpinBox
          << mMaxCharAngleOutDSpinBox
          << mMinSizeSpinBox
          << mObstacleFactorDDBtn
          << mObstacleFactorSlider
          << mObstacleTypeComboBox
          << mOffsetTypeComboBox
          << mPalShowAllLabelsForLayerChkBx
          << mPointAngleSpinBox
          << mPointOffsetDDBtn
          << mPointOffsetUnitsDDBtn
          << mPointOffsetUnitWidget
          << mPointOffsetXSpinBox
          << mPointOffsetYSpinBox
          << mPointPositionOrderDDBtn
          << mPointQuadOffsetDDBtn
          << mPreviewBackgroundBtn
          << mPreviewTextEdit
          << mPriorityDDBtn
          << mPrioritySlider
          << mRepeatDistanceDDBtn
          << mRepeatDistanceSpinBox
          << mRepeatDistanceUnitDDBtn
          << mRepeatDistanceUnitWidget
          << mScaleBasedVisibilityChkBx
          << mScaleBasedVisibilityDDBtn
          << mScaleBasedVisibilityMaxDDBtn
          << mMaxScaleWidget
          << mScaleBasedVisibilityMinDDBtn
          << mMinScaleWidget
          << mShadowBlendCmbBx
          << mShadowBlendDDBtn
          << mShadowColorBtn
          << mShadowColorDDBtn
          << mShadowDrawChkBx
          << mShadowDrawDDBtn
          << mShadowOffsetAngleDDBtn
          << mShadowOffsetAngleSpnBx
          << mShadowOffsetDDBtn
          << mShadowOffsetGlobalChkBx
          << mShadowOffsetSpnBx
          << mShadowOffsetUnitsDDBtn
          << mShadowOffsetUnitWidget
          << mShadowRadiusAlphaChkBx
          << mShadowRadiusDDBtn
          << mShadowRadiusDblSpnBx
          << mShadowRadiusUnitsDDBtn
          << mShadowRadiusUnitWidget
          << mShadowScaleDDBtn
          << mShadowScaleSpnBx
          << mShadowOpacityDDBtn
          << mShadowOpacityWidget
          << mShadowUnderCmbBx
          << mShadowUnderDDBtn
          << mShapeBlendCmbBx
          << mShapeBlendModeDDBtn
          << mShapeStrokeColorBtn
          << mShapeStrokeColorDDBtn
          << mShapeStrokeUnitsDDBtn
          << mShapeStrokeWidthDDBtn
          << mShapeStrokeWidthSpnBx
          << mShapeStrokeWidthUnitWidget
          << mShapeDrawChkBx
          << mShapeDrawDDBtn
          << mShapeFillColorBtn
          << mShapeFillColorDDBtn
          << mShapeOffsetDDBtn
          << mShapeOffsetUnitsDDBtn
          << mShapeOffsetXSpnBx
          << mShapeOffsetYSpnBx
          << mShapeOffsetUnitWidget
          << mShapePenStyleCmbBx
          << mShapePenStyleDDBtn
          << mShapeRadiusDDBtn
          << mShapeRadiusUnitsDDBtn
          << mShapeRadiusXDbSpnBx
          << mShapeRadiusYDbSpnBx
          << mShapeRotationCmbBx
          << mShapeRotationDDBtn
          << mShapeRotationDblSpnBx
          << mShapeRotationTypeDDBtn
          << mShapeRadiusUnitWidget
          << mShapeSVGPathDDBtn
          << mShapeSVGPathLineEdit
          << mShapeSizeCmbBx
          << mShapeSizeTypeDDBtn
          << mShapeSizeUnitsDDBtn
          << mShapeSizeUnitWidget
          << mShapeSizeXDDBtn
          << mShapeSizeXSpnBx
          << mShapeSizeYDDBtn
          << mShapeSizeYSpnBx
          << mShapeOpacityDDBtn
          << mBackgroundOpacityWidget
          << mShapeTypeCmbBx
          << mShapeTypeDDBtn
          << mShowLabelDDBtn
          << mWrapCharDDBtn
          << mAutoWrapLengthDDBtn
          << mZIndexDDBtn
          << mZIndexSpinBox
          << spinBufferSize
          << wrapCharacterEdit
          << mAutoWrapLengthSpinBox
          << mAutoWrapTypeComboBox
          << mCentroidRadioVisible
          << mCentroidRadioWhole
          << mDirectSymbRadioBtnAbove
          << mDirectSymbRadioBtnBelow
          << mDirectSymbRadioBtnLR
          << mUpsidedownRadioAll
          << mUpsidedownRadioDefined
          << mUpsidedownRadioOff
          << radAroundCentroid
          << radAroundPoint
          << radLineCurved
          << radLineHorizontal
          << radLineParallel
          << radOverCentroid
          << radOverPoint
          << radPolygonFree
          << radPolygonHorizontal
          << radPolygonPerimeter
          << radPolygonPerimeterCurved
          << radPredefinedOrder
          << mFieldExpressionWidget
          << mCheckBoxSubstituteText;
  connectValueChanged( widgets, SLOT( updatePreview() ) );

  connect( mQuadrantBtnGrp, static_cast<void ( QButtonGroup::* )( int )>( &QButtonGroup::buttonClicked ), this, &QgsTextFormatWidget::updatePreview );

  // set correct initial tab to match displayed setting page
  whileBlocking( mOptionsTab )->setCurrentIndex( mLabelStackedWidget->currentIndex() );

  if ( mMapCanvas )
  {
    lblFontPreview->setMapUnits( mMapCanvas->mapSettings().mapUnits() );
    mPreviewScaleComboBox->setScale( mMapCanvas->mapSettings().scale() );
  }
}

void QgsTextFormatWidget::setWidgetMode( QgsTextFormatWidget::Mode mode )
{
  mWidgetMode = mode;
  switch ( mode )
  {
    case Labeling:
      toggleDDButtons( true );
      break;

    case Text:
      toggleDDButtons( false );
      delete mLabelingOptionsListWidget->takeItem( 6 );
      delete mLabelingOptionsListWidget->takeItem( 5 );
      mOptionsTab->removeTab( 6 );
      mOptionsTab->removeTab( 5 );

      frameLabelWith->hide();
      mDirectSymbolsFrame->hide();
      mFormatNumFrame->hide();
      mFormatNumChkBx->hide();
      mFormatNumDDBtn->hide();
      mSubstitutionsFrame->hide();
      mFontBoldBtn->hide();
      mFontItalicBtn->hide();

      break;
  }
}

void QgsTextFormatWidget::toggleDDButtons( bool visible )
{
  Q_FOREACH ( QgsPropertyOverrideButton *button, findChildren< QgsPropertyOverrideButton * >() )
  {
    button->setVisible( visible );
  }
}

void QgsTextFormatWidget::setDockMode( bool enabled )
{
  mOptionsTab->setVisible( enabled );
  mOptionsTab->setTabToolTip( 0, tr( "Text" ) );
  mOptionsTab->setTabToolTip( 1, tr( "Formatting" ) );
  mOptionsTab->setTabToolTip( 2, tr( "Buffer" ) );
  mOptionsTab->setTabToolTip( 3, tr( "Background" ) );
  mOptionsTab->setTabToolTip( 4, tr( "Shadow" ) );
  mOptionsTab->setTabToolTip( 5, tr( "Placement" ) );
  mOptionsTab->setTabToolTip( 6, tr( "Rendering" ) );

  mLabelingOptionsListFrame->setVisible( !enabled );
  groupBox_mPreview->setVisible( !enabled );
  mDockMode = enabled;
}

void QgsTextFormatWidget::connectValueChanged( const QList<QWidget *> &widgets, const char *slot )
{
  Q_FOREACH ( QWidget *widget, widgets )
  {
    if ( QgsPropertyOverrideButton *w = qobject_cast<QgsPropertyOverrideButton *>( widget ) )
    {
      connect( w, SIGNAL( changed() ), this, slot );
    }
    else if ( QgsFieldExpressionWidget *w = qobject_cast< QgsFieldExpressionWidget *>( widget ) )
    {
      connect( w, SIGNAL( fieldChanged( QString ) ), this,  slot );
    }
    else if ( QgsOpacityWidget *w = qobject_cast< QgsOpacityWidget *>( widget ) )
    {
      connect( w, SIGNAL( opacityChanged( double ) ), this,  slot );
    }
    else if ( QgsScaleWidget *w = qobject_cast< QgsScaleWidget *>( widget ) )
    {
      connect( w, SIGNAL( scaleChanged( double ) ), this,  slot );
    }
    else if ( QgsUnitSelectionWidget *w = qobject_cast<QgsUnitSelectionWidget *>( widget ) )
    {
      connect( w, SIGNAL( changed() ), this,  slot );
    }
    else if ( QComboBox *w = qobject_cast<QComboBox *>( widget ) )
    {
      connect( w, SIGNAL( currentIndexChanged( int ) ), this, slot );
    }
    else if ( QSpinBox *w = qobject_cast<QSpinBox *>( widget ) )
    {
      connect( w, SIGNAL( valueChanged( int ) ), this, slot );
    }
    else if ( QDoubleSpinBox *w = qobject_cast<QDoubleSpinBox *>( widget ) )
    {
      connect( w, SIGNAL( valueChanged( double ) ), this, slot );
    }
    else if ( QgsColorButton *w = qobject_cast<QgsColorButton *>( widget ) )
    {
      connect( w, SIGNAL( colorChanged( QColor ) ), this, slot );
    }
    else if ( QCheckBox *w = qobject_cast<QCheckBox *>( widget ) )
    {
      connect( w, SIGNAL( toggled( bool ) ), this, slot );
    }
    else if ( QRadioButton *w = qobject_cast<QRadioButton *>( widget ) )
    {
      connect( w, SIGNAL( toggled( bool ) ), this, slot );
    }
    else if ( QLineEdit *w = qobject_cast<QLineEdit *>( widget ) )
    {
      connect( w, SIGNAL( textEdited( QString ) ), this, slot );
    }
    else if ( QSlider *w = qobject_cast<QSlider *>( widget ) )
    {
      connect( w, SIGNAL( valueChanged( int ) ), this, slot );
    }
    else
    {
      QgsLogger::warning( QStringLiteral( "Could not create connection for widget %1" ).arg( widget->objectName() ) );
    }
  }
}

void QgsTextFormatWidget::updateWidgetForFormat( const QgsTextFormat &format )
{
  QgsTextBufferSettings buffer = format.buffer();
  QgsTextBackgroundSettings background = format.background();
  QgsTextShadowSettings shadow = format.shadow();

  // buffer
  mBufferDrawChkBx->setChecked( buffer.enabled() );
  spinBufferSize->setValue( buffer.size() );
  mBufferUnitWidget->setUnit( buffer.sizeUnit() );
  mBufferUnitWidget->setMapUnitScale( buffer.sizeMapUnitScale() );
  btnBufferColor->setColor( buffer.color() );
  mBufferOpacityWidget->setOpacity( buffer.opacity() );
  mBufferJoinStyleComboBox->setPenJoinStyle( buffer.joinStyle() );
  mBufferTranspFillChbx->setChecked( buffer.fillBufferInterior() );
  comboBufferBlendMode->setBlendMode( buffer.blendMode() );
  if ( buffer.paintEffect() )
    mBufferEffect.reset( buffer.paintEffect()->clone() );
  else
  {
    mBufferEffect.reset( QgsPaintEffectRegistry::defaultStack() );
    mBufferEffect->setEnabled( false );
  }
  mBufferEffectWidget->setPaintEffect( mBufferEffect.get() );

  mFontSizeUnitWidget->setUnit( format.sizeUnit() );
  mFontSizeUnitWidget->setMapUnitScale( format.sizeMapUnitScale() );
  mRefFont = format.font();
  mFontSizeSpinBox->setValue( format.size() );
  btnTextColor->setColor( format.color() );
  mTextOpacityWidget->setOpacity( format.opacity() );
  comboBlendMode->setBlendMode( format.blendMode() );

  mFontWordSpacingSpinBox->setValue( format.font().wordSpacing() );
  mFontLetterSpacingSpinBox->setValue( format.font().letterSpacing() );

  QgsFontUtils::updateFontViaStyle( mRefFont, format.namedStyle() );
  updateFont( mRefFont );

  // show 'font not found' if substitution has occurred (should come after updateFont())
  mFontMissingLabel->setVisible( !format.fontFound() );
  if ( !format.fontFound() )
  {
    QString missingTxt = tr( "%1 not found. Default substituted." );
    QString txtPrepend = tr( "Chosen font" );
    if ( !format.resolvedFontFamily().isEmpty() )
    {
      txtPrepend = QStringLiteral( "'%1'" ).arg( format.resolvedFontFamily() );
    }
    mFontMissingLabel->setText( missingTxt.arg( txtPrepend ) );

    // ensure user is sent to 'Text style' section to see notice
    mLabelingOptionsListWidget->setCurrentRow( 0 );
  }
  mFontLineHeightSpinBox->setValue( format.lineHeight() );

  // shape background
  mShapeDrawChkBx->setChecked( background.enabled() );
  mShapeTypeCmbBx->blockSignals( true );
  mShapeTypeCmbBx->setCurrentIndex( background.type() );
  mShapeTypeCmbBx->blockSignals( false );
  mShapeSVGPathLineEdit->setText( background.svgFile() );

  mShapeSizeCmbBx->setCurrentIndex( background.sizeType() );
  mShapeSizeXSpnBx->setValue( background.size().width() );
  mShapeSizeYSpnBx->setValue( background.size().height() );
  mShapeSizeUnitWidget->setUnit( background.sizeUnit() );
  mShapeSizeUnitWidget->setMapUnitScale( background.sizeMapUnitScale() );
  mShapeRotationCmbBx->setCurrentIndex( background.rotationType() );
  mShapeRotationDblSpnBx->setEnabled( background.rotationType() != QgsTextBackgroundSettings::RotationSync );
  mShapeRotationDDBtn->setEnabled( background.rotationType() != QgsTextBackgroundSettings::RotationSync );
  mShapeRotationDblSpnBx->setValue( background.rotation() );
  mShapeOffsetXSpnBx->setValue( background.offset().x() );
  mShapeOffsetYSpnBx->setValue( background.offset().y() );
  mShapeOffsetUnitWidget->setUnit( background.offsetUnit() );
  mShapeOffsetUnitWidget->setMapUnitScale( background.offsetMapUnitScale() );
  mShapeRadiusXDbSpnBx->setValue( background.radii().width() );
  mShapeRadiusYDbSpnBx->setValue( background.radii().height() );
  mShapeRadiusUnitWidget->setUnit( background.radiiUnit() );
  mShapeRadiusUnitWidget->setMapUnitScale( background.radiiMapUnitScale() );

  mShapeFillColorBtn->setColor( background.fillColor() );
  mShapeStrokeColorBtn->setColor( background.strokeColor() );
  mShapeStrokeWidthSpnBx->setValue( background.strokeWidth() );
  mShapeStrokeWidthUnitWidget->setUnit( background.strokeWidthUnit() );
  mShapeStrokeWidthUnitWidget->setMapUnitScale( background.strokeWidthMapUnitScale() );
  mShapePenStyleCmbBx->setPenJoinStyle( background.joinStyle() );

  mBackgroundOpacityWidget->setOpacity( background.opacity() );
  mShapeBlendCmbBx->setBlendMode( background.blendMode() );

  mLoadSvgParams = false;
  mShapeTypeCmbBx_currentIndexChanged( background.type() ); // force update of shape background gui

  if ( background.paintEffect() )
    mBackgroundEffect.reset( background.paintEffect()->clone() );
  else
  {
    mBackgroundEffect.reset( QgsPaintEffectRegistry::defaultStack() );
    mBackgroundEffect->setEnabled( false );
  }
  mBackgroundEffectWidget->setPaintEffect( mBackgroundEffect.get() );

  // drop shadow
  mShadowDrawChkBx->setChecked( shadow.enabled() );
  mShadowUnderCmbBx->setCurrentIndex( shadow.shadowPlacement() );
  mShadowOffsetAngleSpnBx->setValue( shadow.offsetAngle() );
  mShadowOffsetSpnBx->setValue( shadow.offsetDistance() );
  mShadowOffsetUnitWidget->setUnit( shadow.offsetUnit() );
  mShadowOffsetUnitWidget->setMapUnitScale( shadow.offsetMapUnitScale() );
  mShadowOffsetGlobalChkBx->setChecked( shadow.offsetGlobal() );

  mShadowRadiusDblSpnBx->setValue( shadow.blurRadius() );
  mShadowRadiusUnitWidget->setUnit( shadow.blurRadiusUnit() );
  mShadowRadiusUnitWidget->setMapUnitScale( shadow.blurRadiusMapUnitScale() );
  mShadowRadiusAlphaChkBx->setChecked( shadow.blurAlphaOnly() );
  mShadowOpacityWidget->setOpacity( shadow.opacity() );
  mShadowScaleSpnBx->setValue( shadow.scale() );

  mShadowColorBtn->setColor( shadow.color() );
  mShadowBlendCmbBx->setBlendMode( shadow.blendMode() );

}

QgsTextFormatWidget::~QgsTextFormatWidget()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/Labeling/FontPreviewSplitState" ), mFontPreviewSplitter->saveState() );
  settings.setValue( QStringLiteral( "Windows/Labeling/OptionsSplitState" ), mLabelingOptionsSplitter->saveState() );
  settings.setValue( QStringLiteral( "Windows/Labeling/Tab" ), mLabelingOptionsListWidget->currentRow() );
}

QgsTextFormat QgsTextFormatWidget::format() const
{
  QgsTextFormat format;
  format.setColor( btnTextColor->color() );
  format.setFont( mRefFont );
  format.setSize( mFontSizeSpinBox->value() );
  format.setNamedStyle( mFontStyleComboBox->currentText() );
  format.setOpacity( mTextOpacityWidget->opacity() );
  format.setBlendMode( comboBlendMode->blendMode() );
  format.setSizeUnit( mFontSizeUnitWidget->unit() );
  format.setSizeMapUnitScale( mFontSizeUnitWidget->getMapUnitScale() );
  format.setLineHeight( mFontLineHeightSpinBox->value() );

  // buffer
  QgsTextBufferSettings buffer;
  buffer.setEnabled( mBufferDrawChkBx->isChecked() );
  buffer.setSize( spinBufferSize->value() );
  buffer.setColor( btnBufferColor->color() );
  buffer.setOpacity( mBufferOpacityWidget->opacity() );
  buffer.setSizeUnit( mBufferUnitWidget->unit() );
  buffer.setSizeMapUnitScale( mBufferUnitWidget->getMapUnitScale() );
  buffer.setJoinStyle( mBufferJoinStyleComboBox->penJoinStyle() );
  buffer.setFillBufferInterior( mBufferTranspFillChbx->isChecked() );
  buffer.setBlendMode( comboBufferBlendMode->blendMode() );
  if ( mBufferEffect && !QgsPaintEffectRegistry::isDefaultStack( mBufferEffect.get() ) )
    buffer.setPaintEffect( mBufferEffect->clone() );
  else
    buffer.setPaintEffect( nullptr );
  format.setBuffer( buffer );

  // shape background
  QgsTextBackgroundSettings background;
  background.setEnabled( mShapeDrawChkBx->isChecked() );
  background.setType( ( QgsTextBackgroundSettings::ShapeType )mShapeTypeCmbBx->currentIndex() );
  background.setSvgFile( mShapeSVGPathLineEdit->text() );
  background.setSizeType( ( QgsTextBackgroundSettings::SizeType )mShapeSizeCmbBx->currentIndex() );
  background.setSize( QSizeF( mShapeSizeXSpnBx->value(), mShapeSizeYSpnBx->value() ) );
  background.setSizeUnit( mShapeSizeUnitWidget->unit() );
  background.setSizeMapUnitScale( mShapeSizeUnitWidget->getMapUnitScale() );
  background.setRotationType( ( QgsTextBackgroundSettings::RotationType )( mShapeRotationCmbBx->currentIndex() ) );
  background.setRotation( mShapeRotationDblSpnBx->value() );
  background.setOffset( QPointF( mShapeOffsetXSpnBx->value(), mShapeOffsetYSpnBx->value() ) );
  background.setOffsetUnit( mShapeOffsetUnitWidget->unit() );
  background.setOffsetMapUnitScale( mShapeOffsetUnitWidget->getMapUnitScale() );
  background.setRadii( QSizeF( mShapeRadiusXDbSpnBx->value(), mShapeRadiusYDbSpnBx->value() ) );
  background.setRadiiUnit( mShapeRadiusUnitWidget->unit() );
  background.setRadiiMapUnitScale( mShapeRadiusUnitWidget->getMapUnitScale() );

  background.setFillColor( mShapeFillColorBtn->color() );
  background.setStrokeColor( mShapeStrokeColorBtn->color() );
  background.setStrokeWidth( mShapeStrokeWidthSpnBx->value() );
  background.setStrokeWidthUnit( mShapeStrokeWidthUnitWidget->unit() );
  background.setStrokeWidthMapUnitScale( mShapeStrokeWidthUnitWidget->getMapUnitScale() );
  background.setJoinStyle( mShapePenStyleCmbBx->penJoinStyle() );
  background.setOpacity( mBackgroundOpacityWidget->opacity() );
  background.setBlendMode( mShapeBlendCmbBx->blendMode() );
  if ( mBackgroundEffect && !QgsPaintEffectRegistry::isDefaultStack( mBackgroundEffect.get() ) )
    background.setPaintEffect( mBackgroundEffect->clone() );
  else
    background.setPaintEffect( nullptr );
  format.setBackground( background );

  // drop shadow
  QgsTextShadowSettings shadow;
  shadow.setEnabled( mShadowDrawChkBx->isChecked() );
  shadow.setShadowPlacement( ( QgsTextShadowSettings::ShadowPlacement )mShadowUnderCmbBx->currentIndex() );
  shadow.setOffsetAngle( mShadowOffsetAngleSpnBx->value() );
  shadow.setOffsetDistance( mShadowOffsetSpnBx->value() );
  shadow.setOffsetUnit( mShadowOffsetUnitWidget->unit() );
  shadow.setOffsetMapUnitScale( mShadowOffsetUnitWidget->getMapUnitScale() );
  shadow.setOffsetGlobal( mShadowOffsetGlobalChkBx->isChecked() );
  shadow.setBlurRadius( mShadowRadiusDblSpnBx->value() );
  shadow.setBlurRadiusUnit( mShadowRadiusUnitWidget->unit() );
  shadow.setBlurRadiusMapUnitScale( mShadowRadiusUnitWidget->getMapUnitScale() );
  shadow.setBlurAlphaOnly( mShadowRadiusAlphaChkBx->isChecked() );
  shadow.setOpacity( mShadowOpacityWidget->opacity() );
  shadow.setScale( mShadowScaleSpnBx->value() );
  shadow.setColor( mShadowColorBtn->color() );
  shadow.setBlendMode( mShadowBlendCmbBx->blendMode() );
  format.setShadow( shadow );

  return format;
}

void QgsTextFormatWidget::setFormat( const QgsTextFormat &format )
{
  updateWidgetForFormat( format );
}

void QgsTextFormatWidget::optionsStackedWidget_CurrentChanged( int indx )
{
  mLabelingOptionsListWidget->blockSignals( true );
  mLabelingOptionsListWidget->setCurrentRow( indx );
  mLabelingOptionsListWidget->blockSignals( false );
}

void QgsTextFormatWidget::collapseSample( bool collapse )
{
  if ( collapse )
  {
    QList<int> splitSizes = mFontPreviewSplitter->sizes();
    if ( splitSizes[0] > groupBox_mPreview->height() )
    {
      int delta = splitSizes[0] - groupBox_mPreview->height();
      splitSizes[0] -= delta;
      splitSizes[1] += delta;
      mFontPreviewSplitter->setSizes( splitSizes );
    }
  }
}

void QgsTextFormatWidget::changeTextColor( const QColor &color )
{
  Q_UNUSED( color )
  updatePreview();
}

void QgsTextFormatWidget::updateFont( const QFont &font )
{
  // update background reference font
  if ( font != mRefFont )
  {
    mRefFont = font;
  }

  // test if font is actually available
  // NOTE: QgsFontUtils::fontMatchOnSystem may fail here, just crosscheck family
  mFontMissingLabel->setVisible( !QgsFontUtils::fontFamilyMatchOnSystem( mRefFont.family() ) );

  mDirectSymbLeftLineEdit->setFont( mRefFont );
  mDirectSymbRightLineEdit->setFont( mRefFont );

  blockFontChangeSignals( true );
  mFontFamilyCmbBx->setCurrentFont( mRefFont );
  populateFontStyleComboBox();
  int idx = mFontCapitalsComboBox->findData( QVariant( ( unsigned int ) mRefFont.capitalization() ) );
  mFontCapitalsComboBox->setCurrentIndex( idx == -1 ? 0 : idx );
  mFontUnderlineBtn->setChecked( mRefFont.underline() );
  mFontStrikethroughBtn->setChecked( mRefFont.strikeOut() );
  blockFontChangeSignals( false );

  // update font name with font face
//  font.setPixelSize( 24 );

  updatePreview();
}

void QgsTextFormatWidget::blockFontChangeSignals( bool blk )
{
  mFontFamilyCmbBx->blockSignals( blk );
  mFontStyleComboBox->blockSignals( blk );
  mFontCapitalsComboBox->blockSignals( blk );
  mFontUnderlineBtn->blockSignals( blk );
  mFontStrikethroughBtn->blockSignals( blk );
  mFontWordSpacingSpinBox->blockSignals( blk );
  mFontLetterSpacingSpinBox->blockSignals( blk );
}

void QgsTextFormatWidget::updatePreview()
{
  // In dock mode we don't have a preview we
  // just let stuff know we have changed because
  // there might be live updates connected.
  if ( mDockMode )
  {
    emit widgetChanged();
    return;
  }

  scrollPreview();
  lblFontPreview->setFormat( format() );
}

void QgsTextFormatWidget::scrollPreview()
{
  scrollArea_mPreview->ensureVisible( 0, 0, 0, 0 );
}

void QgsTextFormatWidget::setPreviewBackground( const QColor &color )
{
  scrollArea_mPreview->widget()->setStyleSheet( QStringLiteral( "background: rgb(%1, %2, %3);" ).arg( QString::number( color.red() ),
      QString::number( color.green() ),
      QString::number( color.blue() ) ) );
}

void QgsTextFormatWidget::changeBufferColor( const QColor &color )
{
  Q_UNUSED( color )
  updatePreview();
}

void QgsTextFormatWidget::updatePlacementWidgets()
{
  QWidget *curWdgt = stackedPlacement->currentWidget();

  bool showLineFrame = false;
  bool showCentroidFrame = false;
  bool showQuadrantFrame = false;
  bool showFixedQuadrantFrame = false;
  bool showPlacementPriorityFrame = false;
  bool showOffsetTypeFrame = false;
  bool showOffsetFrame = false;
  bool showDistanceFrame = false;
  bool showRotationFrame = false;
  bool showMaxCharAngleFrame = false;

  bool enableMultiLinesFrame = true;

  if ( ( curWdgt == pagePoint && radAroundPoint->isChecked() )
       || ( curWdgt == pagePolygon && radAroundCentroid->isChecked() ) )
  {
    showCentroidFrame = ( curWdgt == pagePolygon && radAroundCentroid->isChecked() );
    showDistanceFrame = true;
    //showRotationFrame = true; // TODO: uncomment when supported
    if ( curWdgt == pagePoint )
    {
      showQuadrantFrame = true;
    }
  }
  else if ( ( curWdgt == pagePoint && radOverPoint->isChecked() )
            || ( curWdgt == pagePolygon && radOverCentroid->isChecked() ) )
  {
    showCentroidFrame = ( curWdgt == pagePolygon && radOverCentroid->isChecked() );
    showQuadrantFrame = true;
    showFixedQuadrantFrame = true;
    showOffsetFrame = true;
    showRotationFrame = true;
  }
  else if ( curWdgt == pagePoint && radPredefinedOrder->isChecked() )
  {
    showDistanceFrame = true;
    showPlacementPriorityFrame = true;
    showOffsetTypeFrame  = true;
  }
  else if ( ( curWdgt == pageLine && radLineParallel->isChecked() )
            || ( curWdgt == pagePolygon && radPolygonPerimeter->isChecked() )
            || ( curWdgt == pageLine && radLineCurved->isChecked() )
            || ( curWdgt == pagePolygon && radPolygonPerimeterCurved->isChecked() ) )
  {
    showLineFrame = true;
    showDistanceFrame = true;
    //showRotationFrame = true; // TODO: uncomment when supported

    bool offline = chkLineAbove->isChecked() || chkLineBelow->isChecked();
    chkLineOrientationDependent->setEnabled( offline );
    mPlacementDistanceFrame->setEnabled( offline );

    bool isCurved = ( curWdgt == pageLine && radLineCurved->isChecked() )
                    || ( curWdgt == pagePolygon && radPolygonPerimeterCurved->isChecked() );
    showMaxCharAngleFrame = isCurved;
    // TODO: enable mMultiLinesFrame when supported for curved labels
    enableMultiLinesFrame = !isCurved;
  }

  mPlacementLineFrame->setVisible( showLineFrame );
  mPlacementCentroidFrame->setVisible( showCentroidFrame );
  mPlacementQuadrantFrame->setVisible( showQuadrantFrame );
  mPlacementFixedQuadrantFrame->setVisible( showFixedQuadrantFrame );
  mPlacementCartographicFrame->setVisible( showPlacementPriorityFrame );
  mPlacementOffsetFrame->setVisible( showOffsetFrame );
  mPlacementDistanceFrame->setVisible( showDistanceFrame );
  mPlacementOffsetTypeFrame->setVisible( showOffsetTypeFrame );
  mPlacementRotationFrame->setVisible( showRotationFrame );
  mPlacementRepeatDistanceFrame->setVisible( curWdgt == pageLine || ( curWdgt == pagePolygon &&
      ( radPolygonPerimeter->isChecked() || radPolygonPerimeterCurved->isChecked() ) ) );
  mPlacementMaxCharAngleFrame->setVisible( showMaxCharAngleFrame );

  mMultiLinesFrame->setEnabled( enableMultiLinesFrame );
}

void QgsTextFormatWidget::populateFontCapitalsComboBox()
{
  mFontCapitalsComboBox->addItem( tr( "No change" ), QVariant( 0 ) );
  mFontCapitalsComboBox->addItem( tr( "All uppercase" ), QVariant( 1 ) );
  mFontCapitalsComboBox->addItem( tr( "All lowercase" ), QVariant( 2 ) );
  // Small caps doesn't work right with QPainterPath::addText()
  // https://bugreports.qt.io/browse/QTBUG-13965
//  mFontCapitalsComboBox->addItem( tr( "Small caps" ), QVariant( 3 ) );
  mFontCapitalsComboBox->addItem( tr( "Capitalize first letter" ), QVariant( 4 ) );
}

void QgsTextFormatWidget::populateFontStyleComboBox()
{
  mFontStyleComboBox->clear();
  QStringList styles = mFontDB.styles( mRefFont.family() );
  Q_FOREACH ( const QString &style, styles )
  {
    mFontStyleComboBox->addItem( style );
  }

  QString targetStyle = mFontDB.styleString( mRefFont );
  if ( !styles.contains( targetStyle ) )
  {
    QFont f = QFont( mRefFont.family() );
    targetStyle = QFontInfo( f ).styleName();
    mRefFont.setStyleName( targetStyle );
  }
  int curIndx = 0;
  int stylIndx = mFontStyleComboBox->findText( targetStyle );
  if ( stylIndx > -1 )
  {
    curIndx = stylIndx;
  }

  mFontStyleComboBox->setCurrentIndex( curIndx );
}

void QgsTextFormatWidget::mFontSizeSpinBox_valueChanged( double d )
{
  mRefFont.setPointSizeF( d );
  updateFont( mRefFont );
}

void QgsTextFormatWidget::mFontCapitalsComboBox_currentIndexChanged( int index )
{
  int capitalsindex = mFontCapitalsComboBox->itemData( index ).toUInt();
  mRefFont.setCapitalization( ( QFont::Capitalization ) capitalsindex );
  updateFont( mRefFont );
}

void QgsTextFormatWidget::mFontFamilyCmbBx_currentFontChanged( const QFont &f )
{
  mRefFont.setFamily( f.family() );
  updateFont( mRefFont );
}

void QgsTextFormatWidget::mFontStyleComboBox_currentIndexChanged( const QString &text )
{
  QgsFontUtils::updateFontViaStyle( mRefFont, text );
  updateFont( mRefFont );
}

void QgsTextFormatWidget::mFontUnderlineBtn_toggled( bool ckd )
{
  mRefFont.setUnderline( ckd );
  updateFont( mRefFont );
}

void QgsTextFormatWidget::mFontStrikethroughBtn_toggled( bool ckd )
{
  mRefFont.setStrikeOut( ckd );
  updateFont( mRefFont );
}

void QgsTextFormatWidget::mFontWordSpacingSpinBox_valueChanged( double spacing )
{
  mRefFont.setWordSpacing( spacing );
  updateFont( mRefFont );
}

void QgsTextFormatWidget::mFontLetterSpacingSpinBox_valueChanged( double spacing )
{
  mRefFont.setLetterSpacing( QFont::AbsoluteSpacing, spacing );
  updateFont( mRefFont );
}

void QgsTextFormatWidget::mFontSizeUnitWidget_changed()
{
  // disable pixel size limiting for labels defined in points
  if ( mFontSizeUnitWidget->unit() != QgsUnitTypes::RenderMapUnits )
  {
    mFontLimitPixelChkBox->setChecked( false );
  }
  else if ( mMinPixelLimit == 0 )
  {
    // initial minimum trigger value set, turn on pixel size limiting by default
    // for labels defined in map units (ignored after first settings save)
    mFontLimitPixelChkBox->setChecked( true );
  }
  updateFont( mRefFont );
}

void QgsTextFormatWidget::mFontMinPixelSpinBox_valueChanged( int px )
{
  // ensure max font pixel size for map unit labels can't be lower than min
  mFontMaxPixelSpinBox->setMinimum( px );
  mFontMaxPixelSpinBox->update();
}

void QgsTextFormatWidget::mFontMaxPixelSpinBox_valueChanged( int px )
{
  // ensure max font pixel size for map unit labels can't be lower than min
  if ( px < mFontMinPixelSpinBox->value() )
  {
    mFontMaxPixelSpinBox->blockSignals( true );
    mFontMaxPixelSpinBox->setValue( mFontMinPixelSpinBox->value() );
    mFontMaxPixelSpinBox->blockSignals( false );
  }
  mFontMaxPixelSpinBox->setMinimum( mFontMinPixelSpinBox->value() );
}

void QgsTextFormatWidget::mBufferUnitWidget_changed()
{
  updateFont( mRefFont );
}

void QgsTextFormatWidget::mCoordXDDBtn_activated( bool active )
{
  if ( !active ) //no data defined alignment without data defined position
  {
    enableDataDefinedAlignment( false );
  }
  else if ( mCoordYDDBtn->isActive() )
  {
    enableDataDefinedAlignment( true );
  }
}

void QgsTextFormatWidget::mCoordYDDBtn_activated( bool active )
{
  if ( !active ) //no data defined alignment without data defined position
  {
    enableDataDefinedAlignment( false );
  }
  else if ( mCoordXDDBtn->isActive() )
  {
    enableDataDefinedAlignment( true );
  }
}

void QgsTextFormatWidget::mShapeTypeCmbBx_currentIndexChanged( int index )
{
  // shape background
  bool isRect = ( ( QgsTextBackgroundSettings::ShapeType )index == QgsTextBackgroundSettings::ShapeRectangle
                  || ( QgsTextBackgroundSettings::ShapeType )index == QgsTextBackgroundSettings::ShapeSquare );
  bool isSVG = ( ( QgsTextBackgroundSettings::ShapeType )index == QgsTextBackgroundSettings::ShapeSVG );

  showBackgroundPenStyle( isRect );
  showBackgroundRadius( isRect );

  mShapeSVGPathFrame->setVisible( isSVG );
  // symbology SVG renderer only supports size^2 scaling, so we only use the x size spinbox
  mShapeSizeYLabel->setVisible( !isSVG );
  mShapeSizeYSpnBx->setVisible( !isSVG );
  mShapeSizeYDDBtn->setVisible( !isSVG && mWidgetMode == Labeling );
  mShapeSizeXLabel->setText( tr( "Size%1" ).arg( !isSVG ? tr( " X" ) : QString() ) );

  // SVG parameter setting doesn't support color's alpha component yet
  mShapeFillColorBtn->setAllowOpacity( !isSVG );
  mShapeFillColorBtn->setButtonBackground();
  mShapeStrokeColorBtn->setAllowOpacity( !isSVG );
  mShapeStrokeColorBtn->setButtonBackground();

  // configure SVG parameter widgets
  mShapeSVGParamsBtn->setVisible( isSVG );
  if ( isSVG )
  {
    updateSvgWidgets( mShapeSVGPathLineEdit->text() );
  }
  else
  {
    mShapeFillColorLabel->setEnabled( true );
    mShapeFillColorBtn->setEnabled( true );
    mShapeFillColorDDBtn->setEnabled( true );
    mShapeStrokeColorLabel->setEnabled( true );
    mShapeStrokeColorBtn->setEnabled( true );
    mShapeStrokeColorDDBtn->setEnabled( true );
    mShapeStrokeWidthLabel->setEnabled( true );
    mShapeStrokeWidthSpnBx->setEnabled( true );
    mShapeStrokeWidthDDBtn->setEnabled( true );
  }
  // TODO: fix overriding SVG symbol's stroke width units in QgsSvgCache
  // currently broken, fall back to symbol units only
  mShapeStrokeWidthUnitWidget->setVisible( !isSVG );
  mShapeSVGUnitsLabel->setVisible( isSVG );
  mShapeStrokeUnitsDDBtn->setEnabled( !isSVG );
}

void QgsTextFormatWidget::mShapeSVGPathLineEdit_textChanged( const QString &text )
{
  updateSvgWidgets( text );
}

void QgsTextFormatWidget::updateLinePlacementOptions()
{
  int numOptionsChecked = ( chkLineAbove->isChecked() ? 1 : 0 ) +
                          ( chkLineBelow->isChecked() ? 1 : 0 ) +
                          ( chkLineOn->isChecked() ? 1 : 0 );

  if ( numOptionsChecked == 1 )
  {
    //prevent unchecking last option
    chkLineAbove->setEnabled( !chkLineAbove->isChecked() );
    chkLineBelow->setEnabled( !chkLineBelow->isChecked() );
    chkLineOn->setEnabled( !chkLineOn->isChecked() );
  }
  else
  {
    chkLineAbove->setEnabled( true );
    chkLineBelow->setEnabled( true );
    chkLineOn->setEnabled( true );
  }
}

void QgsTextFormatWidget::onSubstitutionsChanged( const QgsStringReplacementCollection &substitutions )
{
  mSubstitutions = substitutions;
  emit widgetChanged();
}

void QgsTextFormatWidget::previewScaleChanged( double scale )
{
  lblFontPreview->setScale( scale );
}

void QgsTextFormatWidget::updateSvgWidgets( const QString &svgPath )
{
  if ( mShapeSVGPathLineEdit->text() != svgPath )
  {
    mShapeSVGPathLineEdit->setText( svgPath );
  }

  QString resolvedPath;
  bool validSVG = true;
  if ( ! svgPath.startsWith( QLatin1String( "base64:" ), Qt::CaseInsensitive ) )
  {
    resolvedPath = QgsSymbolLayerUtils::svgSymbolNameToPath( svgPath, QgsProject::instance()->pathResolver() );
    validSVG = QFileInfo::exists( resolvedPath );
  }
  else
  {
    resolvedPath = svgPath;
    validSVG = true;
  }

  // draw red text for path field if invalid (path can't be resolved)
  mShapeSVGPathLineEdit->setStyleSheet( !validSVG ? QStringLiteral( "QLineEdit{ color: rgb(225, 0, 0); }" ) : QString() );
  mShapeSVGPathLineEdit->setToolTip( !validSVG ? tr( "File not found" ) : resolvedPath );

  QColor fill, stroke;
  double strokeWidth = 0.0;
  bool fillParam = false, strokeParam = false, strokeWidthParam = false;
  if ( validSVG )
  {
    QgsApplication::svgCache()->containsParams( resolvedPath, fillParam, fill, strokeParam, stroke, strokeWidthParam, strokeWidth );
  }

  mShapeSVGParamsBtn->setEnabled( validSVG && ( fillParam || strokeParam || strokeWidthParam ) );

  mShapeFillColorLabel->setEnabled( validSVG && fillParam );
  mShapeFillColorBtn->setEnabled( validSVG && fillParam );
  mShapeFillColorDDBtn->setEnabled( validSVG && fillParam );
  if ( mLoadSvgParams && validSVG && fillParam )
    mShapeFillColorBtn->setColor( fill );

  mShapeStrokeColorLabel->setEnabled( validSVG && strokeParam );
  mShapeStrokeColorBtn->setEnabled( validSVG && strokeParam );
  mShapeStrokeColorDDBtn->setEnabled( validSVG && strokeParam );
  if ( mLoadSvgParams && validSVG && strokeParam )
    mShapeStrokeColorBtn->setColor( stroke );

  mShapeStrokeWidthLabel->setEnabled( validSVG && strokeWidthParam );
  mShapeStrokeWidthSpnBx->setEnabled( validSVG && strokeWidthParam );
  mShapeStrokeWidthDDBtn->setEnabled( validSVG && strokeWidthParam );
  if ( mLoadSvgParams && validSVG && strokeWidthParam )
    mShapeStrokeWidthSpnBx->setValue( strokeWidth );

  // TODO: fix overriding SVG symbol's stroke width units in QgsSvgCache
  // currently broken, fall back to symbol's
  //mShapeStrokeWidthUnitWidget->setEnabled( validSVG && strokeWidthParam );
  //mShapeStrokeUnitsDDBtn->setEnabled( validSVG && strokeWidthParam );
  mShapeSVGUnitsLabel->setEnabled( validSVG && strokeWidthParam );
}

void QgsTextFormatWidget::mShapeSVGSelectorBtn_clicked()
{
  QgsSvgSelectorDialog svgDlg( this );
  svgDlg.setWindowTitle( tr( "Select SVG file" ) );
  svgDlg.svgSelector()->setSvgPath( mShapeSVGPathLineEdit->text().trimmed() );

  if ( svgDlg.exec() == QDialog::Accepted )
  {
    QString svgPath = svgDlg.svgSelector()->currentSvgPath();
    if ( !svgPath.isEmpty() )
    {
      mShapeSVGPathLineEdit->setText( svgPath );
      updatePreview();
    }
  }
}

void QgsTextFormatWidget::mShapeSVGParamsBtn_clicked()
{
  QString svgPath = mShapeSVGPathLineEdit->text();
  mLoadSvgParams = true;
  updateSvgWidgets( svgPath );
  mLoadSvgParams = false;
}

void QgsTextFormatWidget::mShapeRotationCmbBx_currentIndexChanged( int index )
{
  mShapeRotationDblSpnBx->setEnabled( ( QgsTextBackgroundSettings::RotationType )index != QgsTextBackgroundSettings::RotationSync );
  mShapeRotationDDBtn->setEnabled( ( QgsTextBackgroundSettings::RotationType )index != QgsTextBackgroundSettings::RotationSync );
}

void QgsTextFormatWidget::mPreviewTextEdit_textChanged( const QString &text )
{
  lblFontPreview->setText( text );
  updatePreview();
}

void QgsTextFormatWidget::mPreviewTextBtn_clicked()
{
  mPreviewTextEdit->setText( QStringLiteral( "Lorem Ipsum" ) );
  updatePreview();
}

void QgsTextFormatWidget::mPreviewBackgroundBtn_colorChanged( const QColor &color )
{
  setPreviewBackground( color );
}

void QgsTextFormatWidget::mDirectSymbLeftToolBtn_clicked()
{
  bool gotChar = false;
  QChar dirSymb = mCharDlg->selectCharacter( &gotChar, mRefFont, mFontDB.styleString( mRefFont ) );

  if ( !gotChar )
    return;

  if ( !dirSymb.isNull() )
    mDirectSymbLeftLineEdit->setText( QString( dirSymb ) );
}

void QgsTextFormatWidget::mDirectSymbRightToolBtn_clicked()
{
  bool gotChar = false;
  QChar dirSymb = mCharDlg->selectCharacter( &gotChar, mRefFont, mFontDB.styleString( mRefFont ) );

  if ( !gotChar )
    return;

  if ( !dirSymb.isNull() )
    mDirectSymbRightLineEdit->setText( QString( dirSymb ) );
}

void QgsTextFormatWidget::mChkNoObstacle_toggled( bool active )
{
  mPolygonObstacleTypeFrame->setEnabled( active );
  mObstaclePriorityFrame->setEnabled( active );
}

void QgsTextFormatWidget::chkLineOrientationDependent_toggled( bool active )
{
  if ( active )
  {
    chkLineAbove->setText( tr( "Left of line" ) );
    chkLineBelow->setText( tr( "Right of line" ) );
  }
  else
  {
    chkLineAbove->setText( tr( "Above line" ) );
    chkLineBelow->setText( tr( "Below line" ) );
  }
}


void QgsTextFormatWidget::mToolButtonConfigureSubstitutes_clicked()
{
  QgsPanelWidget *panel = QgsPanelWidget::findParentPanel( this );
  if ( panel && panel->dockMode() )
  {
    QgsSubstitutionListWidget *widget = new QgsSubstitutionListWidget( panel );
    widget->setPanelTitle( tr( "Substitutions" ) );
    widget->setSubstitutions( mSubstitutions );
    connect( widget, &QgsSubstitutionListWidget::substitutionsChanged, this, &QgsTextFormatWidget::onSubstitutionsChanged );
    panel->openPanel( widget );
    return;
  }

  QgsSubstitutionListDialog dlg( this );
  dlg.setSubstitutions( mSubstitutions );
  if ( dlg.exec() == QDialog::Accepted )
  {
    mSubstitutions = dlg.substitutions();
    emit widgetChanged();
  }
}

void QgsTextFormatWidget::showBackgroundRadius( bool show )
{
  mShapeRadiusLabel->setVisible( show );
  mShapeRadiusXDbSpnBx->setVisible( show );

  mShapeRadiusYDbSpnBx->setVisible( show );

  mShapeRadiusUnitWidget->setVisible( show );

  mShapeRadiusDDBtn->setVisible( show && mWidgetMode == Labeling );
  mShapeRadiusUnitsDDBtn->setVisible( show  && mWidgetMode == Labeling );
}

void QgsTextFormatWidget::showBackgroundPenStyle( bool show )
{
  mShapePenStyleLabel->setVisible( show );
  mShapePenStyleCmbBx->setVisible( show );

  mShapePenStyleDDBtn->setVisible( show && mWidgetMode == Labeling );
}

void QgsTextFormatWidget::enableDataDefinedAlignment( bool enable )
{
  mCoordAlignmentFrame->setEnabled( enable );
}


//
// QgsTextFormatDialog
//

QgsTextFormatDialog::QgsTextFormatDialog( const QgsTextFormat &format, QgsMapCanvas *mapCanvas, QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setWindowTitle( tr( "Text Settings" ) );

  mFormatWidget = new QgsTextFormatWidget( format, mapCanvas, this );
  mFormatWidget->layout()->setContentsMargins( 0, 0, 0, 0 );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->addWidget( mFormatWidget );

  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this );
  layout->addWidget( buttonBox );

  setLayout( layout );

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/TextFormatDialog/geometry" ) ).toByteArray() );

  connect( buttonBox->button( QDialogButtonBox::Ok ), &QAbstractButton::clicked, this, &QDialog::accept );
  connect( buttonBox->button( QDialogButtonBox::Cancel ), &QAbstractButton::clicked, this, &QDialog::reject );
}

QgsTextFormatDialog::~QgsTextFormatDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/TextFormatDialog/geometry" ), saveGeometry() );
}

QgsTextFormat QgsTextFormatDialog::format() const
{
  return mFormatWidget->format();
}

QgsTextFormatPanelWidget::QgsTextFormatPanelWidget( const QgsTextFormat &format, QgsMapCanvas *mapCanvas, QWidget *parent )
  : QgsPanelWidgetWrapper( new QgsTextFormatWidget( format, mapCanvas ), parent )
{
  mFormatWidget = qobject_cast< QgsTextFormatWidget * >( widget() );
  connect( mFormatWidget, &QgsTextFormatWidget::widgetChanged, this, &QgsPanelWidget::widgetChanged );
}

QgsTextFormat QgsTextFormatPanelWidget::format() const
{
  return mFormatWidget->format();
}

void QgsTextFormatPanelWidget::setDockMode( bool dockMode )
{
  mFormatWidget->setDockMode( dockMode );
  QgsPanelWidgetWrapper::setDockMode( dockMode );
}
