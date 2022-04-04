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
#include "qgsstylesavedialog.h"
#include "qgsexpressioncontextutils.h"
#include "qgsgui.h"
#include "qgsvectorlayer.h"
#include "qgsauxiliarystorage.h"
#include "qgsnewauxiliarylayerdialog.h"
#include "qgshelp.h"
#include "qgsmarkersymbol.h"
#include "qgsfillsymbol.h"
#include "qgsiconutils.h"
#include "qgssymbollayerreference.h"
#include "qgsconfig.h"

#include <QButtonGroup>
#include <QMessageBox>

QgsTextFormatWidget::QgsTextFormatWidget( const QgsTextFormat &format, QgsMapCanvas *mapCanvas, QWidget *parent, QgsVectorLayer *layer )
  : QWidget( parent )
  , mMapCanvas( mapCanvas )
  , mLayer( layer )
{
  initWidget();
  setWidgetMode( Text );
  populateDataDefinedButtons();
  updateWidgetForFormat( format.isValid() ? format : QgsStyle::defaultStyle()->defaultTextFormat() );
}

QgsTextFormatWidget::QgsTextFormatWidget( QgsMapCanvas *mapCanvas, QWidget *parent, Mode mode, QgsVectorLayer *layer )
  : QWidget( parent )
  , mMapCanvas( mapCanvas )
  , mLayer( layer )
  , mWidgetMode( mode )
{
  initWidget();
  if ( mode == Text )
    populateDataDefinedButtons();
  setWidgetMode( mode );
}

void QgsTextFormatWidget::initWidget()
{
  setupUi( this );

  mGeometryGeneratorGroupBox->setCollapsed( true );

  connect( mShapeSVGPathLineEdit, &QLineEdit::textChanged, this, &QgsTextFormatWidget::mShapeSVGPathLineEdit_textChanged );
  connect( mFontSizeSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsTextFormatWidget::mFontSizeSpinBox_valueChanged );
  connect( mFontFamilyCmbBx, &QFontComboBox::currentFontChanged, this, &QgsTextFormatWidget::mFontFamilyCmbBx_currentFontChanged );
  connect( mFontStyleComboBox, &QComboBox::currentTextChanged, this, &QgsTextFormatWidget::mFontStyleComboBox_currentIndexChanged );
  connect( mFontUnderlineBtn, &QToolButton::toggled, this, &QgsTextFormatWidget::mFontUnderlineBtn_toggled );
  connect( mFontStrikethroughBtn, &QToolButton::toggled, this, &QgsTextFormatWidget::mFontStrikethroughBtn_toggled );
  connect( mFontWordSpacingSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsTextFormatWidget::mFontWordSpacingSpinBox_valueChanged );
  connect( mFontLetterSpacingSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsTextFormatWidget::mFontLetterSpacingSpinBox_valueChanged );
  connect( mFontSizeUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsTextFormatWidget::mFontSizeUnitWidget_changed );
  connect( mFontMinPixelSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsTextFormatWidget::mFontMinPixelSpinBox_valueChanged );
  connect( mFontMaxPixelSpinBox, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsTextFormatWidget::mFontMaxPixelSpinBox_valueChanged );
  connect( mBufferUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsTextFormatWidget::mBufferUnitWidget_changed );
  connect( mMaskBufferUnitWidget, &QgsUnitSelectionWidget::changed, this, &QgsTextFormatWidget::mMaskBufferUnitWidget_changed );
  connect( mCoordXDDBtn, &QgsPropertyOverrideButton::changed, this, &QgsTextFormatWidget::mCoordXDDBtn_changed );
  connect( mCoordXDDBtn, &QgsPropertyOverrideButton::activated, this, &QgsTextFormatWidget::mCoordXDDBtn_activated );
  connect( mCoordYDDBtn, &QgsPropertyOverrideButton::changed, this, &QgsTextFormatWidget::mCoordYDDBtn_changed );
  connect( mCoordYDDBtn, &QgsPropertyOverrideButton::activated, this, &QgsTextFormatWidget::mCoordYDDBtn_activated );
  connect( mCoordPointDDBtn, &QgsPropertyOverrideButton::changed, this, &QgsTextFormatWidget::mCoordPointDDBtn_changed );
  connect( mCoordPointDDBtn, &QgsPropertyOverrideButton::activated, this, &QgsTextFormatWidget::mCoordPointDDBtn_activated );
  connect( mShapeTypeCmbBx, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsTextFormatWidget::mShapeTypeCmbBx_currentIndexChanged );
  connect( mShapeRotationCmbBx, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsTextFormatWidget::mShapeRotationCmbBx_currentIndexChanged );
  connect( mShapeSVGParamsBtn, &QPushButton::clicked, this, &QgsTextFormatWidget::mShapeSVGParamsBtn_clicked );
  connect( mShapeSVGSelectorBtn, &QPushButton::clicked, this, &QgsTextFormatWidget::mShapeSVGSelectorBtn_clicked );
  connect( mPreviewTextEdit, &QLineEdit::textChanged, this, &QgsTextFormatWidget::mPreviewTextEdit_textChanged );
  connect( mPreviewTextBtn, &QToolButton::clicked, this, &QgsTextFormatWidget::mPreviewTextBtn_clicked );
  connect( mPreviewBackgroundBtn, &QgsColorButton::colorChanged, this, &QgsTextFormatWidget::mPreviewBackgroundBtn_colorChanged );
  connect( mDirectSymbLeftToolBtn, &QToolButton::clicked, this, &QgsTextFormatWidget::mDirectSymbLeftToolBtn_clicked );
  connect( mDirectSymbRightToolBtn, &QToolButton::clicked, this, &QgsTextFormatWidget::mDirectSymbRightToolBtn_clicked );
  connect( chkLineOrientationDependent, &QCheckBox::toggled, this, &QgsTextFormatWidget::chkLineOrientationDependent_toggled );
  connect( mToolButtonConfigureSubstitutes, &QToolButton::clicked, this, &QgsTextFormatWidget::mToolButtonConfigureSubstitutes_clicked );
  connect( mKerningCheckBox, &QCheckBox::toggled, this, &QgsTextFormatWidget::kerningToggled );

  const int iconSize = QgsGuiUtils::scaleIconSize( 20 );
  mOptionsTab->setIconSize( QSize( iconSize, iconSize ) );
  mLabelingOptionsListWidget->setIconSize( QSize( iconSize, iconSize ) ) ;
  const int iconSize32 = QgsGuiUtils::scaleIconSize( 32 );
  const int iconSize24 = QgsGuiUtils::scaleIconSize( 24 );
  const int iconSize18 = QgsGuiUtils::scaleIconSize( 18 );
  const int iconSize16 = QgsGuiUtils::scaleIconSize( 16 );

  mPreviewTextBtn->setIconSize( QSize( iconSize16, iconSize16 ) );
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

  const auto unitWidgets = findChildren<QgsUnitSelectionWidget *>();
  for ( QgsUnitSelectionWidget *unitWidget : unitWidgets )
  {
    unitWidget->setMapCanvas( mMapCanvas );
  }
  mFontSizeUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits
                                 << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderPixels << QgsUnitTypes::RenderInches );
  mBufferUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                               << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches << QgsUnitTypes::RenderPercentage );
  mMaskBufferUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                   << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches  << QgsUnitTypes::RenderPercentage );
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
                                     << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches << QgsUnitTypes::RenderPercentage );
  mShadowRadiusUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                     << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches  << QgsUnitTypes::RenderPercentage );
  mPointOffsetUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                    << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mLineDistanceUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                     << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mRepeatDistanceUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
                                       << QgsUnitTypes::RenderPoints << QgsUnitTypes::RenderInches );
  mOverrunDistanceUnitWidget->setUnits( QgsUnitTypes::RenderUnitList() << QgsUnitTypes::RenderMillimeters << QgsUnitTypes::RenderMetersInMapUnits << QgsUnitTypes::RenderMapUnits << QgsUnitTypes::RenderPixels
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
  mLineDistanceSpnBx->setClearValue( 0.0 );
  mSpinStretch->setClearValue( 100 );

  mOffsetTypeComboBox->addItem( tr( "From Point" ), QgsPalLayerSettings::FromPoint );
  mOffsetTypeComboBox->addItem( tr( "From Symbol Bounds" ), QgsPalLayerSettings::FromSymbolBounds );

  mShapeTypeCmbBx->addItem( tr( "Rectangle" ), QgsTextBackgroundSettings::ShapeRectangle );
  mShapeTypeCmbBx->addItem( tr( "Square" ), QgsTextBackgroundSettings::ShapeSquare );
  mShapeTypeCmbBx->addItem( tr( "Ellipse" ), QgsTextBackgroundSettings::ShapeEllipse );
  mShapeTypeCmbBx->addItem( tr( "Circle" ), QgsTextBackgroundSettings::ShapeCircle );
  mShapeTypeCmbBx->addItem( tr( "SVG" ), QgsTextBackgroundSettings::ShapeSVG );
  mShapeTypeCmbBx->addItem( tr( "Marker Symbol" ), QgsTextBackgroundSettings::ShapeMarkerSymbol );

  updateAvailableShadowPositions();

  mBackgroundMarkerSymbolButton->setSymbolType( Qgis::SymbolType::Marker );
  mBackgroundMarkerSymbolButton->setDialogTitle( tr( "Background Symbol" ) );
  mBackgroundMarkerSymbolButton->registerExpressionContextGenerator( this );
  mBackgroundMarkerSymbolButton->setMapCanvas( mMapCanvas );
  mBackgroundFillSymbolButton->setSymbolType( Qgis::SymbolType::Fill );
  mBackgroundFillSymbolButton->setDialogTitle( tr( "Background Symbol" ) );
  mBackgroundFillSymbolButton->registerExpressionContextGenerator( this );
  mBackgroundFillSymbolButton->setMapCanvas( mMapCanvas );

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

  mTextOrientationComboBox->addItem( tr( "Horizontal" ), QgsTextFormat::HorizontalOrientation );
  mTextOrientationComboBox->addItem( tr( "Vertical" ), QgsTextFormat::VerticalOrientation );

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
  mDirectSymbBtnGrp->addButton( mDirectSymbRadioBtnLR, static_cast<int>( QgsLabelLineSettings::DirectionSymbolPlacement::SymbolLeftRight ) );
  mDirectSymbBtnGrp->addButton( mDirectSymbRadioBtnAbove, static_cast<int>( QgsLabelLineSettings::DirectionSymbolPlacement::SymbolAbove ) );
  mDirectSymbBtnGrp->addButton( mDirectSymbRadioBtnBelow, static_cast<int>( QgsLabelLineSettings::DirectionSymbolPlacement::SymbolBelow ) );
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
  connect( mCheckAllowLabelsOutsidePolygons, &QAbstractButton::toggled, this, &QgsTextFormatWidget::updatePlacementWidgets );
  connect( mAllowOutsidePolygonsDDBtn, &QgsPropertyOverrideButton::changed, this, &QgsTextFormatWidget::updatePlacementWidgets );

  connect( mPlacementModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsTextFormatWidget::updatePlacementWidgets );

  // Global settings group for groupboxes' saved/restored collapsed state
  // maintains state across different dialogs
  const auto groupBoxes = findChildren<QgsCollapsibleGroupBox *>();
  for ( QgsCollapsibleGroupBox *grpbox : groupBoxes )
  {
    grpbox->setSettingGroup( QStringLiteral( "mAdvLabelingDlg" ) );
  }

  connect( groupBox_mPreview, &QgsCollapsibleGroupBoxBasic::collapsedStateChanged, this, &QgsTextFormatWidget::collapseSample );

  // get rid of annoying outer focus rect on Mac
  mLabelingOptionsListWidget->setAttribute( Qt::WA_MacShowFocusRect, false );

  const QgsSettings settings;

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

  mMaskEffect.reset( QgsPaintEffectRegistry::defaultStack() );
  connect( mMaskEffectWidget, &QgsEffectStackCompactWidget::changed, this, &QgsTextFormatWidget::updatePreview );
  mMaskEffectWidget->setPaintEffect( mMaskEffect.get() );

  mBackgroundEffect.reset( QgsPaintEffectRegistry::defaultStack() );
  connect( mBackgroundEffectWidget, &QgsEffectStackCompactWidget::changed, this, &QgsTextFormatWidget::updatePreview );
  mBackgroundEffectWidget->setPaintEffect( mBackgroundEffect.get() );

#ifndef HAS_KDE_QT5_FONT_STRETCH_FIX
  mLabelStretch->hide();
  mSpinStretch->hide();
  mFontStretchDDBtn->hide();
#endif

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
          << mBufferDrawChkBx
          << mBufferJoinStyleComboBox
          << mBufferTranspFillChbx
          << mBufferOpacityWidget
          << mCentroidInsideCheckBox
          << mChkNoObstacle
          << mCoordRotationUnitComboBox
          << mDirectSymbChkBx
          << mDirectSymbLeftLineEdit
          << mDirectSymbRevChkBx
          << mDirectSymbRightLineEdit
          << mFitInsidePolygonCheckBox
          << mFontCapitalsComboBox
          << mFontLetterSpacingSpinBox
          << mFontLimitPixelChkBox
          << mFontLineHeightSpinBox
          << mFontMaxPixelSpinBox
          << mFontMinPixelSpinBox
          << mFontMultiLineAlignComboBox
          << mFontSizeSpinBox
          << mFontStyleComboBox
          << mTextOrientationComboBox
          << mTextOpacityWidget
          << mSpinStretch
          << mFontWordSpacingSpinBox
          << mFormatNumChkBx
          << mFormatNumDecimalsSpnBx
          << mFormatNumPlusSignChkBx
          << mLimitLabelChkBox
          << mLimitLabelSpinBox
          << mLineDistanceSpnBx
          << mLineDistanceUnitWidget
          << mMaxCharAngleInDSpinBox
          << mMaxCharAngleOutDSpinBox
          << mMinSizeSpinBox
          << mOffsetTypeComboBox
          << mPalShowAllLabelsForLayerChkBx
          << mPointAngleSpinBox
          << mPointOffsetUnitWidget
          << mPointOffsetXSpinBox
          << mPointOffsetYSpinBox
          << mPreviewBackgroundBtn
          << mPreviewTextEdit
          << mPrioritySlider
          << mRepeatDistanceSpinBox
          << mRepeatDistanceUnitWidget
          << mOverrunDistanceSpinBox
          << mOverrunDistanceUnitWidget
          << mScaleBasedVisibilityChkBx
          << mMaxScaleWidget
          << mMinScaleWidget
          << mShadowBlendCmbBx
          << mShadowColorBtn
          << mShadowDrawChkBx
          << mShadowOffsetAngleSpnBx
          << mShadowOffsetGlobalChkBx
          << mShadowOffsetSpnBx
          << mShadowOffsetUnitWidget
          << mShadowRadiusAlphaChkBx
          << mShadowRadiusDblSpnBx
          << mShadowRadiusUnitWidget
          << mShadowScaleSpnBx
          << mShadowOpacityWidget
          << mShadowUnderCmbBx
          << mShapeBlendCmbBx
          << mShapeStrokeColorBtn
          << mShapeStrokeWidthSpnBx
          << mShapeStrokeWidthUnitWidget
          << mShapeDrawChkBx
          << mShapeFillColorBtn
          << mShapeOffsetXSpnBx
          << mShapeOffsetYSpnBx
          << mShapeOffsetUnitWidget
          << mShapeRadiusXDbSpnBx
          << mShapeRadiusYDbSpnBx
          << mShapeRotationCmbBx
          << mShapeRotationDblSpnBx
          << mShapeRadiusUnitWidget
          << mShapeSVGPathLineEdit
          << mShapeSizeCmbBx
          << mShapeSizeUnitWidget
          << mShapeSizeXSpnBx
          << mShapeSizeYSpnBx
          << mBackgroundOpacityWidget
          << mShapeTypeCmbBx
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
          << mPlacementModeComboBox
          << mFieldExpressionWidget
          << mCheckBoxSubstituteText
          << mGeometryGeneratorGroupBox
          << mGeometryGenerator
          << mGeometryGeneratorType
          << mBackgroundMarkerSymbolButton
          << mBackgroundFillSymbolButton
          << mCalloutsDrawCheckBox
          << mCalloutStyleComboBox
          << mKerningCheckBox
          << mEnableMaskChkBx
          << mMaskJoinStyleComboBox
          << mMaskBufferSizeSpinBox
          << mMaskOpacityWidget
          << mCheckAllowLabelsOutsidePolygons
          << mHtmlFormattingCheckBox;

  connectValueChanged( widgets, SLOT( updatePreview() ) );

  connect( mQuadrantBtnGrp, qOverload< QAbstractButton * >( &QButtonGroup::buttonClicked ), this, &QgsTextFormatWidget::updatePreview );

  connect( mBufferDrawDDBtn, &QgsPropertyOverrideButton::activated, this, &QgsTextFormatWidget::updateBufferFrameStatus );
  connect( mBufferDrawChkBx, &QCheckBox::stateChanged, this, [ = ]( int )
  {
    updateBufferFrameStatus();
  } );
  connect( mShapeDrawDDBtn, &QgsPropertyOverrideButton::activated, this, &QgsTextFormatWidget::updateShapeFrameStatus );
  connect( mShapeDrawChkBx, &QCheckBox::stateChanged, this, [ = ]( int )
  {
    updateShapeFrameStatus();
  } );
  connect( mShadowDrawDDBtn, &QgsPropertyOverrideButton::activated, this, &QgsTextFormatWidget::updateShadowFrameStatus );
  connect( mShadowDrawChkBx, &QCheckBox::stateChanged, this, [ = ]( int )
  {
    updateShadowFrameStatus();
  } );
  connect( mCalloutDrawDDBtn, &QgsPropertyOverrideButton::changed, this, &QgsTextFormatWidget::updateCalloutFrameStatus );
  connect( mCalloutsDrawCheckBox, &QCheckBox::stateChanged, this, [ = ]( int )
  {
    updateCalloutFrameStatus();
  } );

  mGeometryGeneratorType->addItem( QgsIconUtils::iconForWkbType( QgsWkbTypes::Polygon ), tr( "Polygon / MultiPolygon" ), QgsWkbTypes::GeometryType::PolygonGeometry );
  mGeometryGeneratorType->addItem( QgsIconUtils::iconForWkbType( QgsWkbTypes::LineString ), tr( "LineString / MultiLineString" ), QgsWkbTypes::GeometryType::LineGeometry );
  mGeometryGeneratorType->addItem( QgsIconUtils::iconForWkbType( QgsWkbTypes::Point ), tr( "Point / MultiPoint" ), QgsWkbTypes::GeometryType::PointGeometry );

  // set correct initial tab to match displayed setting page
  whileBlocking( mOptionsTab )->setCurrentIndex( mLabelStackedWidget->currentIndex() );
  mOptionsTab->tabBar()->setUsesScrollButtons( true );


  if ( mMapCanvas )
  {
    lblFontPreview->setMapUnits( mMapCanvas->mapSettings().mapUnits() );
    mPreviewScaleComboBox->setScale( mMapCanvas->mapSettings().scale() );
  }

  mTextFormatsListWidget->setStyle( QgsStyle::defaultStyle() );
  mTextFormatsListWidget->setEntityType( QgsStyle::TextFormatEntity );
  connect( mTextFormatsListWidget, &QgsStyleItemsListWidget::selectionChanged, this, &QgsTextFormatWidget::setFormatFromStyle );
  connect( mTextFormatsListWidget, &QgsStyleItemsListWidget::saveEntity, this, &QgsTextFormatWidget::saveFormat );
}

void QgsTextFormatWidget::setWidgetMode( QgsTextFormatWidget::Mode mode )
{
  mWidgetMode = mode;
  switch ( mode )
  {
    case Labeling:
      toggleDDButtons( true );
      mTextFormatsListWidget->setEntityTypes( QList< QgsStyle::StyleEntity >() << QgsStyle::TextFormatEntity << QgsStyle::LabelSettingsEntity );
      mTextOrientationComboBox->addItem( tr( "Rotation-based" ), QgsTextFormat::RotationBasedOrientation );
      break;

    case Text:
    {
      const int prevIndex = mOptionsTab->currentIndex();
      toggleDDButtons( true );
      delete mLabelingOptionsListWidget->takeItem( 8 ); // rendering
      delete mLabelingOptionsListWidget->takeItem( 7 ); // placement
      delete mLabelingOptionsListWidget->takeItem( 6 ); // callouts
      delete mLabelingOptionsListWidget->takeItem( 3 ); // mask
      mOptionsTab->removeTab( 8 );
      mOptionsTab->removeTab( 7 );
      mOptionsTab->removeTab( 6 );
      mOptionsTab->removeTab( 3 );
      mLabelStackedWidget->removeWidget( mLabelPage_Rendering );
      mLabelStackedWidget->removeWidget( mLabelPage_Callouts );
      mLabelStackedWidget->removeWidget( mLabelPage_Mask );
      mLabelStackedWidget->removeWidget( mLabelPage_Placement );
      switch ( prevIndex )
      {
        case 0:
        case 1:
        case 2:
          break;

        case 4: // background - account for removed mask tab
        case 5: // shadow
          mLabelStackedWidget->setCurrentIndex( prevIndex - 1 );
          mOptionsTab->setCurrentIndex( prevIndex - 1 );
          break;

        case 3: // mask
        case 6: // callouts
        case 7: // placement
        case 8: // rendering
          mLabelStackedWidget->setCurrentIndex( 0 );
          mOptionsTab->setCurrentIndex( 0 );
          break;
      }

      frameLabelWith->hide();
      mDirectSymbolsFrame->hide();
      mFormatNumFrame->hide();
      mFormatNumChkBx->hide();
      mFormatNumDDBtn->hide();
      mCheckBoxSubstituteText->hide();
      mToolButtonConfigureSubstitutes->hide();
      mLabelWrapOnCharacter->hide();
      wrapCharacterEdit->hide();
      mWrapCharDDBtn->hide();
      mLabelWrapLinesTo->hide();
      mAutoWrapLengthSpinBox->hide();
      mAutoWrapLengthDDBtn->hide();
      mAutoWrapTypeComboBox->hide();
      mFontMultiLineLabel->hide();
      mFontMultiLineAlignComboBox->hide();
      mFontMultiLineAlignDDBtn->hide();

      mTextOrientationComboBox->removeItem( mTextOrientationComboBox->findData( QgsTextFormat::RotationBasedOrientation ) );
      break;
    }
  }
}

void QgsTextFormatWidget::toggleDDButtons( bool visible )
{
  const auto buttons = findChildren< QgsPropertyOverrideButton * >();
  for ( QgsPropertyOverrideButton *button : buttons )
  {
#ifndef HAS_KDE_QT5_FONT_STRETCH_FIX
    if ( button == mFontStretchDDBtn )
      continue; // always hidden
#endif
    button->setVisible( visible );
  }
}

void QgsTextFormatWidget::setDockMode( bool enabled )
{
  mOptionsTab->setVisible( enabled );
  mOptionsTab->setTabToolTip( 0, tr( "Text" ) );
  mOptionsTab->setTabToolTip( 1, tr( "Formatting" ) );
  mOptionsTab->setTabToolTip( 2, tr( "Buffer" ) );
  mOptionsTab->setTabToolTip( 3, tr( "Mask" ) );
  mOptionsTab->setTabToolTip( 4, tr( "Background" ) );
  mOptionsTab->setTabToolTip( 5, tr( "Shadow" ) );
  mOptionsTab->setTabToolTip( 6, tr( "Callouts" ) );
  mOptionsTab->setTabToolTip( 7, tr( "Placement" ) );
  mOptionsTab->setTabToolTip( 8, tr( "Rendering" ) );

  mLabelingOptionsListFrame->setVisible( !enabled );
  groupBox_mPreview->setVisible( !enabled );
  mDockMode = enabled;
}

void QgsTextFormatWidget::connectValueChanged( const QList<QWidget *> &widgets, const char *slot )
{
  const auto constWidgets = widgets;
  for ( QWidget *widget : constWidgets )
  {
    if ( QgsSymbolButton *w = qobject_cast<QgsSymbolButton *>( widget ) )
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
    else if ( QGroupBox *w = qobject_cast<QGroupBox *>( widget ) )
    {
      connect( w, SIGNAL( toggled( bool ) ), this, slot );
    }
    else if ( QgsCodeEditorExpression *w = qobject_cast<QgsCodeEditorExpression *>( widget ) )
    {
      connect( w, SIGNAL( textChanged() ), this, slot );
    }
    else
    {
      QgsLogger::warning( QStringLiteral( "Could not create connection for widget %1" ).arg( widget->objectName() ) );
    }
  }
}

void QgsTextFormatWidget::populateDataDefinedButtons()
{
  // text style
  registerDataDefinedButton( mFontDDBtn, QgsPalLayerSettings::Family );
  registerDataDefinedButton( mFontStyleDDBtn, QgsPalLayerSettings::FontStyle );
  registerDataDefinedButton( mFontUnderlineDDBtn, QgsPalLayerSettings::Underline );
  registerDataDefinedButton( mFontStrikeoutDDBtn, QgsPalLayerSettings::Strikeout );
  registerDataDefinedButton( mFontBoldDDBtn, QgsPalLayerSettings::Bold );
  registerDataDefinedButton( mFontItalicDDBtn, QgsPalLayerSettings::Italic );
  registerDataDefinedButton( mFontSizeDDBtn, QgsPalLayerSettings::Size );
  registerDataDefinedButton( mFontUnitsDDBtn, QgsPalLayerSettings::FontSizeUnit );
  registerDataDefinedButton( mFontColorDDBtn, QgsPalLayerSettings::Color );
  registerDataDefinedButton( mFontOpacityDDBtn, QgsPalLayerSettings::FontOpacity );
  registerDataDefinedButton( mFontCaseDDBtn, QgsPalLayerSettings::FontCase );
  registerDataDefinedButton( mFontLetterSpacingDDBtn, QgsPalLayerSettings::FontLetterSpacing );
  registerDataDefinedButton( mFontWordSpacingDDBtn, QgsPalLayerSettings::FontWordSpacing );
  registerDataDefinedButton( mFontBlendModeDDBtn, QgsPalLayerSettings::FontBlendMode );
  registerDataDefinedButton( mFontStretchDDBtn, QgsPalLayerSettings::FontStretchFactor );

  // text formatting
  registerDataDefinedButton( mWrapCharDDBtn, QgsPalLayerSettings::MultiLineWrapChar );
  registerDataDefinedButton( mAutoWrapLengthDDBtn, QgsPalLayerSettings::AutoWrapLength );
  registerDataDefinedButton( mFontLineHeightDDBtn, QgsPalLayerSettings::MultiLineHeight );
  registerDataDefinedButton( mFontMultiLineAlignDDBtn, QgsPalLayerSettings::MultiLineAlignment );
  registerDataDefinedButton( mTextOrientationDDBtn, QgsPalLayerSettings::TextOrientation );

  registerDataDefinedButton( mDirectSymbDDBtn, QgsPalLayerSettings::DirSymbDraw );
  mDirectSymbDDBtn->registerCheckedWidget( mDirectSymbChkBx );
  registerDataDefinedButton( mDirectSymbLeftDDBtn, QgsPalLayerSettings::DirSymbLeft );
  registerDataDefinedButton( mDirectSymbRightDDBtn, QgsPalLayerSettings::DirSymbRight );

  registerDataDefinedButton( mDirectSymbPlacementDDBtn, QgsPalLayerSettings::DirSymbPlacement );
  registerDataDefinedButton( mDirectSymbRevDDBtn, QgsPalLayerSettings::DirSymbReverse );

  registerDataDefinedButton( mFormatNumDDBtn, QgsPalLayerSettings::NumFormat );
  mFormatNumDDBtn->registerCheckedWidget( mFormatNumChkBx );
  registerDataDefinedButton( mFormatNumDecimalsDDBtn, QgsPalLayerSettings::NumDecimals );
  registerDataDefinedButton( mFormatNumPlusSignDDBtn, QgsPalLayerSettings::NumPlusSign );

  // text buffer
  registerDataDefinedButton( mBufferDrawDDBtn, QgsPalLayerSettings::BufferDraw );
  registerDataDefinedButton( mBufferSizeDDBtn, QgsPalLayerSettings::BufferSize );
  registerDataDefinedButton( mBufferUnitsDDBtn, QgsPalLayerSettings::BufferUnit );
  registerDataDefinedButton( mBufferColorDDBtn, QgsPalLayerSettings::BufferColor );
  registerDataDefinedButton( mBufferOpacityDDBtn, QgsPalLayerSettings::BufferOpacity );
  registerDataDefinedButton( mBufferJoinStyleDDBtn, QgsPalLayerSettings::BufferJoinStyle );
  registerDataDefinedButton( mBufferBlendModeDDBtn, QgsPalLayerSettings::BufferBlendMode );

  // mask
  registerDataDefinedButton( mEnableMaskDDBtn, QgsPalLayerSettings::MaskEnabled );
  mEnableMaskDDBtn->registerCheckedWidget( mEnableMaskChkBx );
  registerDataDefinedButton( mMaskBufferSizeDDBtn, QgsPalLayerSettings::MaskBufferSize );
  registerDataDefinedButton( mMaskBufferUnitsDDBtn, QgsPalLayerSettings::MaskBufferUnit );
  registerDataDefinedButton( mMaskOpacityDDBtn, QgsPalLayerSettings::MaskOpacity );
  registerDataDefinedButton( mMaskJoinStyleDDBtn, QgsPalLayerSettings::MaskJoinStyle );

  // background
  registerDataDefinedButton( mShapeDrawDDBtn, QgsPalLayerSettings::ShapeDraw );
  mShapeDrawDDBtn->registerCheckedWidget( mShapeDrawChkBx );
  registerDataDefinedButton( mShapeTypeDDBtn, QgsPalLayerSettings::ShapeKind );
  registerDataDefinedButton( mShapeSVGPathDDBtn, QgsPalLayerSettings::ShapeSVGFile );
  registerDataDefinedButton( mShapeSizeTypeDDBtn, QgsPalLayerSettings::ShapeSizeType );
  registerDataDefinedButton( mShapeSizeXDDBtn, QgsPalLayerSettings::ShapeSizeX );
  registerDataDefinedButton( mShapeSizeYDDBtn, QgsPalLayerSettings::ShapeSizeY );
  registerDataDefinedButton( mShapeSizeUnitsDDBtn, QgsPalLayerSettings::ShapeSizeUnits );
  registerDataDefinedButton( mShapeRotationTypeDDBtn, QgsPalLayerSettings::ShapeRotationType );
  registerDataDefinedButton( mShapeRotationDDBtn, QgsPalLayerSettings::ShapeRotation );
  registerDataDefinedButton( mShapeOffsetDDBtn, QgsPalLayerSettings::ShapeOffset );
  registerDataDefinedButton( mShapeOffsetUnitsDDBtn, QgsPalLayerSettings::ShapeOffsetUnits );
  registerDataDefinedButton( mShapeRadiusDDBtn, QgsPalLayerSettings::ShapeRadii );
  registerDataDefinedButton( mShapeRadiusUnitsDDBtn, QgsPalLayerSettings::ShapeRadiiUnits );
  registerDataDefinedButton( mShapeOpacityDDBtn, QgsPalLayerSettings::ShapeOpacity );
  registerDataDefinedButton( mShapeBlendModeDDBtn, QgsPalLayerSettings::ShapeBlendMode );
  registerDataDefinedButton( mShapeFillColorDDBtn, QgsPalLayerSettings::ShapeFillColor );
  registerDataDefinedButton( mShapeStrokeColorDDBtn, QgsPalLayerSettings::ShapeStrokeColor );
  registerDataDefinedButton( mShapeStrokeWidthDDBtn, QgsPalLayerSettings::ShapeStrokeWidth );
  registerDataDefinedButton( mShapeStrokeUnitsDDBtn, QgsPalLayerSettings::ShapeStrokeWidthUnits );

  // drop shadows
  registerDataDefinedButton( mShadowDrawDDBtn, QgsPalLayerSettings::ShadowDraw );
  mShadowDrawDDBtn->registerCheckedWidget( mShadowDrawChkBx );
  registerDataDefinedButton( mShadowUnderDDBtn, QgsPalLayerSettings::ShadowUnder );
  registerDataDefinedButton( mShadowOffsetAngleDDBtn, QgsPalLayerSettings::ShadowOffsetAngle );
  registerDataDefinedButton( mShadowOffsetDDBtn, QgsPalLayerSettings::ShadowOffsetDist );
  registerDataDefinedButton( mShadowOffsetUnitsDDBtn, QgsPalLayerSettings::ShadowOffsetUnits );
  registerDataDefinedButton( mShadowRadiusDDBtn, QgsPalLayerSettings::ShadowRadius );
  registerDataDefinedButton( mShadowRadiusUnitsDDBtn, QgsPalLayerSettings::ShadowRadiusUnits );
  registerDataDefinedButton( mShadowOpacityDDBtn, QgsPalLayerSettings::ShadowOpacity );
  registerDataDefinedButton( mShadowScaleDDBtn, QgsPalLayerSettings::ShadowScale );
  registerDataDefinedButton( mShadowColorDDBtn, QgsPalLayerSettings::ShadowColor );
  registerDataDefinedButton( mShadowBlendDDBtn, QgsPalLayerSettings::ShadowBlendMode );

  // placement
  registerDataDefinedButton( mCentroidDDBtn, QgsPalLayerSettings::CentroidWhole );
  registerDataDefinedButton( mPointQuadOffsetDDBtn, QgsPalLayerSettings::OffsetQuad );
  registerDataDefinedButton( mPointPositionOrderDDBtn, QgsPalLayerSettings::PredefinedPositionOrder );
  registerDataDefinedButton( mLinePlacementFlagsDDBtn, QgsPalLayerSettings::LinePlacementOptions );
  registerDataDefinedButton( mPointOffsetDDBtn, QgsPalLayerSettings::OffsetXY );
  registerDataDefinedButton( mPointOffsetUnitsDDBtn, QgsPalLayerSettings::OffsetUnits );
  registerDataDefinedButton( mLineDistanceDDBtn, QgsPalLayerSettings::LabelDistance );
  registerDataDefinedButton( mLineDistanceUnitDDBtn, QgsPalLayerSettings::DistanceUnits );
  registerDataDefinedButton( mPriorityDDBtn, QgsPalLayerSettings::Priority );
  registerDataDefinedButton( mAllowOutsidePolygonsDDBtn, QgsPalLayerSettings::PolygonLabelOutside );

  // TODO: is this necessary? maybe just use the data defined-only rotation?
  //mPointAngleDDBtn, QgsPalLayerSettings::OffsetRotation,
  //                        QgsPropertyOverrideButton::AnyType, QgsPropertyOverrideButton::double180RotDesc() );
  registerDataDefinedButton( mMaxCharAngleDDBtn, QgsPalLayerSettings::CurvedCharAngleInOut );
  registerDataDefinedButton( mRepeatDistanceDDBtn, QgsPalLayerSettings::RepeatDistance );
  registerDataDefinedButton( mRepeatDistanceUnitDDBtn, QgsPalLayerSettings::RepeatDistanceUnit );
  registerDataDefinedButton( mOverrunDistanceDDBtn, QgsPalLayerSettings::OverrunDistance );

  // data defined-only
  registerDataDefinedButton( mCoordXDDBtn, QgsPalLayerSettings::PositionX );
  registerDataDefinedButton( mCoordYDDBtn, QgsPalLayerSettings::PositionY );
  registerDataDefinedButton( mCoordPointDDBtn, QgsPalLayerSettings::PositionPoint );
  registerDataDefinedButton( mCoordAlignmentHDDBtn, QgsPalLayerSettings::Hali );
  registerDataDefinedButton( mCoordAlignmentVDDBtn, QgsPalLayerSettings::Vali );
  registerDataDefinedButton( mCoordRotationDDBtn, QgsPalLayerSettings::LabelRotation );

  updateDataDefinedAlignment();

  // rendering
  const QString ddScaleVisInfo = tr( "Value &lt; 0 represents a scale closer than 1:1, e.g. -10 = 10:1<br>"
                                     "Value of 0 disables the specific limit." );
  registerDataDefinedButton( mScaleBasedVisibilityDDBtn, QgsPalLayerSettings::ScaleVisibility );
  mScaleBasedVisibilityDDBtn->registerCheckedWidget( mScaleBasedVisibilityChkBx );
  registerDataDefinedButton( mScaleBasedVisibilityMinDDBtn, QgsPalLayerSettings::MinimumScale );
  mScaleBasedVisibilityMinDDBtn->setUsageInfo( ddScaleVisInfo );
  registerDataDefinedButton( mScaleBasedVisibilityMaxDDBtn, QgsPalLayerSettings::MaximumScale );
  mScaleBasedVisibilityMaxDDBtn->setUsageInfo( ddScaleVisInfo );

  registerDataDefinedButton( mFontLimitPixelDDBtn, QgsPalLayerSettings::FontLimitPixel );
  mFontLimitPixelDDBtn->registerCheckedWidget( mFontLimitPixelChkBox );
  registerDataDefinedButton( mFontMinPixelDDBtn, QgsPalLayerSettings::FontMinPixel );
  registerDataDefinedButton( mFontMaxPixelDDBtn, QgsPalLayerSettings::FontMaxPixel );

  registerDataDefinedButton( mShowLabelDDBtn, QgsPalLayerSettings::Show );

  registerDataDefinedButton( mAlwaysShowDDBtn, QgsPalLayerSettings::AlwaysShow );

  registerDataDefinedButton( mIsObstacleDDBtn, QgsPalLayerSettings::IsObstacle );
  registerDataDefinedButton( mZIndexDDBtn, QgsPalLayerSettings::ZIndex );

  registerDataDefinedButton( mCalloutDrawDDBtn, QgsPalLayerSettings::CalloutDraw );

  registerDataDefinedButton( mLabelAllPartsDDBtn, QgsPalLayerSettings::LabelAllParts );
}

void QgsTextFormatWidget::registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsPalLayerSettings::Property key )
{
  button->init( key, mDataDefinedProperties, QgsPalLayerSettings::propertyDefinitions(), mLayer, true );
  if ( !mButtons.contains( key ) )
  {
    connect( button, &QgsPropertyOverrideButton::changed, this, &QgsTextFormatWidget::updateProperty );
    connect( button, &QgsPropertyOverrideButton::createAuxiliaryField, this, &QgsTextFormatWidget::createAuxiliaryField );
    button->registerExpressionContextGenerator( this );
    mButtons[key] = button;
  }
}

void QgsTextFormatWidget::updateWidgetForFormat( const QgsTextFormat &format )
{
  const QgsTextBufferSettings buffer = format.buffer();
  const QgsTextMaskSettings mask = format.mask();
  const QgsTextBackgroundSettings background = format.background();
  const QgsTextShadowSettings shadow = format.shadow();

  if ( mWidgetMode != Labeling )
  {
    mDataDefinedProperties = format.dataDefinedProperties();
  }

  // buffer
  mBufferDrawChkBx->setChecked( buffer.enabled() );
  mBufferFrame->setEnabled( buffer.enabled() );
  spinBufferSize->setValue( buffer.size() );
  mBufferUnitWidget->setUnit( buffer.sizeUnit() );
  mBufferUnitWidget->setMapUnitScale( buffer.sizeMapUnitScale() );
  btnBufferColor->setColor( buffer.color() );
  mBufferOpacityWidget->setOpacity( buffer.opacity() );
  mBufferJoinStyleComboBox->setPenJoinStyle( buffer.joinStyle() );
  mBufferTranspFillChbx->setChecked( buffer.fillBufferInterior() );
  comboBufferBlendMode->setBlendMode( buffer.blendMode() );
  if ( auto *lPaintEffect = buffer.paintEffect() )
    mBufferEffect.reset( lPaintEffect->clone() );
  else
  {
    mBufferEffect.reset( QgsPaintEffectRegistry::defaultStack() );
    mBufferEffect->setEnabled( false );
  }
  mBufferEffectWidget->setPaintEffect( mBufferEffect.get() );

  // mask
  mMaskedSymbolLayers = mask.maskedSymbolLayers();
  mEnableMaskChkBx->setChecked( mask.enabled() );
  mMaskBufferSizeSpinBox->setValue( mask.size() );
  mMaskBufferUnitWidget->setUnit( mask.sizeUnit() );
  mMaskBufferUnitWidget->setMapUnitScale( mask.sizeMapUnitScale() );
  mMaskOpacityWidget->setOpacity( mask.opacity() );
  mMaskJoinStyleComboBox->setPenJoinStyle( mask.joinStyle() );
  if ( auto *lPaintEffect = mask.paintEffect() )
    mMaskEffect.reset( lPaintEffect->clone() );
  else
  {
    mMaskEffect.reset( QgsPaintEffectRegistry::defaultStack() );
    mMaskEffect->setEnabled( false );
  }
  mMaskEffectWidget->setPaintEffect( mMaskEffect.get() );

  mFontSizeUnitWidget->setUnit( format.sizeUnit() );
  mFontSizeUnitWidget->setMapUnitScale( format.sizeMapUnitScale() );
  mRefFont = format.font();
  mFontSizeSpinBox->setValue( format.size() );
  btnTextColor->setColor( format.color() );
  whileBlocking( mSpinStretch )->setValue( format.stretchFactor() );
  mTextOpacityWidget->setOpacity( format.opacity() );
  comboBlendMode->setBlendMode( format.blendMode() );
  mTextOrientationComboBox->setCurrentIndex( mTextOrientationComboBox->findData( format.orientation() ) );
  mHtmlFormattingCheckBox->setChecked( format.allowHtmlFormatting() );

  mFontWordSpacingSpinBox->setValue( format.font().wordSpacing() );
  mFontLetterSpacingSpinBox->setValue( format.font().letterSpacing() );
  whileBlocking( mKerningCheckBox )->setChecked( format.font().kerning() );

  whileBlocking( mFontCapitalsComboBox )->setCurrentIndex( mFontCapitalsComboBox->findData( static_cast< int >( format.capitalization() ) ) );
  QgsFontUtils::updateFontViaStyle( mRefFont, format.namedStyle() );
  updateFont( mRefFont );

  // show 'font not found' if substitution has occurred (should come after updateFont())
  mFontMissingLabel->setVisible( !format.fontFound() );
  if ( !format.fontFound() )
  {
    const QString missingTxt = tr( "%1 not found. Default substituted." );
    QString txtPrepend = tr( "Chosen font" );
    if ( !format.resolvedFontFamily().isEmpty() )
    {
      txtPrepend = QStringLiteral( "'%1'" ).arg( format.resolvedFontFamily() );
    }
    mFontMissingLabel->setText( missingTxt.arg( txtPrepend ) );

    // ensure user is sent to 'Text style' section to see notice
    mLabelingOptionsListWidget->setCurrentRow( 0 );
    whileBlocking( mOptionsTab )->setCurrentIndex( 0 );
  }
  mFontLineHeightSpinBox->setValue( format.lineHeight() );

  // shape background
  mShapeDrawChkBx->setChecked( background.enabled() );
  mShapeFrame->setEnabled( background.enabled() );
  mShapeTypeCmbBx->blockSignals( true );
  mShapeTypeCmbBx->setCurrentIndex( mShapeTypeCmbBx->findData( background.type() ) );
  mShapeTypeCmbBx->blockSignals( false );
  updateAvailableShadowPositions();
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

  mBackgroundOpacityWidget->setOpacity( background.opacity() );
  mShapeBlendCmbBx->setBlendMode( background.blendMode() );

  mLoadSvgParams = false;
  mShapeTypeCmbBx_currentIndexChanged( background.type() ); // force update of shape background gui

  if ( auto *lPaintEffect = background.paintEffect() )
    mBackgroundEffect.reset( lPaintEffect->clone() );
  else
  {
    mBackgroundEffect.reset( QgsPaintEffectRegistry::defaultStack() );
    mBackgroundEffect->setEnabled( false );
  }
  mBackgroundEffectWidget->setPaintEffect( mBackgroundEffect.get() );

  mBackgroundMarkerSymbolButton->setSymbol( background.markerSymbol() ? background.markerSymbol()->clone() : QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  mBackgroundFillSymbolButton->setSymbol( background.fillSymbol() ? background.fillSymbol()->clone() : QgsSymbol::defaultSymbol( QgsWkbTypes::PolygonGeometry ) );

  // drop shadow
  mShadowDrawChkBx->setChecked( shadow.enabled() );
  mShadowFrame->setEnabled( shadow.enabled() );
  mShadowUnderCmbBx->setCurrentIndex( mShadowUnderCmbBx->findData( shadow.shadowPlacement() ) );
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

  mPreviewBackgroundBtn->setColor( format.previewBackgroundColor() );
  mPreviewBackgroundBtn->setDefaultColor( format.previewBackgroundColor() );
  setPreviewBackground( format.previewBackgroundColor() );

  populateDataDefinedButtons();
}

QgsTextFormatWidget::~QgsTextFormatWidget()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/Labeling/FontPreviewSplitState" ), mFontPreviewSplitter->saveState() );
  settings.setValue( QStringLiteral( "Windows/Labeling/OptionsSplitState" ), mLabelingOptionsSplitter->saveState() );

  int prevIndex = mLabelingOptionsListWidget->currentRow();
  if ( mWidgetMode == Text )
  {
    switch ( prevIndex )
    {
      case 3: // background - account for removed mask tab
      case 4: // shadow - account for removed mask tab
        prevIndex++;
        break;
    }
  }

  settings.setValue( QStringLiteral( "Windows/Labeling/Tab" ), prevIndex );
}

QgsTextFormat QgsTextFormatWidget::format( bool includeDataDefinedProperties ) const
{
  QgsTextFormat format;
  format.setColor( btnTextColor->color() );
  format.setFont( mRefFont );
  format.setSize( mFontSizeSpinBox->value() );
  format.setNamedStyle( mFontStyleComboBox->currentText() );
  format.setOpacity( mTextOpacityWidget->opacity() );
  format.setStretchFactor( mSpinStretch->value() );
  format.setBlendMode( comboBlendMode->blendMode() );
  format.setSizeUnit( mFontSizeUnitWidget->unit() );
  format.setSizeMapUnitScale( mFontSizeUnitWidget->getMapUnitScale() );
  format.setLineHeight( mFontLineHeightSpinBox->value() );
  format.setPreviewBackgroundColor( mPreviewBackgroundColor );
  format.setOrientation( static_cast< QgsTextFormat::TextOrientation >( mTextOrientationComboBox->currentData().toInt() ) );
  format.setAllowHtmlFormatting( mHtmlFormattingCheckBox->isChecked( ) );
  format.setCapitalization( static_cast< Qgis::Capitalization >( mFontCapitalsComboBox->currentData().toInt() ) );

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
  if ( mBufferEffect && ( !QgsPaintEffectRegistry::isDefaultStack( mBufferEffect.get() ) || mBufferEffect->enabled() ) )
    buffer.setPaintEffect( mBufferEffect->clone() );
  else
    buffer.setPaintEffect( nullptr );
  format.setBuffer( buffer );

  // mask
  QgsTextMaskSettings mask;
  mask.setEnabled( mEnableMaskChkBx->isChecked() );
  mask.setSize( mMaskBufferSizeSpinBox->value() );
  mask.setOpacity( mMaskOpacityWidget->opacity() );
  mask.setSizeUnit( mMaskBufferUnitWidget->unit() );
  mask.setSizeMapUnitScale( mMaskBufferUnitWidget->getMapUnitScale() );
  mask.setJoinStyle( mMaskJoinStyleComboBox->penJoinStyle() );
  if ( mMaskEffect && ( !QgsPaintEffectRegistry::isDefaultStack( mMaskEffect.get() ) || mMaskEffect->enabled() ) )
    mask.setPaintEffect( mMaskEffect->clone() );
  else
    mask.setPaintEffect( nullptr );
  mask.setMaskedSymbolLayers( mMaskedSymbolLayers );
  format.setMask( mask );

  // shape background
  QgsTextBackgroundSettings background;
  background.setEnabled( mShapeDrawChkBx->isChecked() );
  background.setType( static_cast< QgsTextBackgroundSettings::ShapeType >( mShapeTypeCmbBx->currentData().toInt() ) );
  background.setSvgFile( mShapeSVGPathLineEdit->text() );
  background.setSizeType( static_cast< QgsTextBackgroundSettings::SizeType >( mShapeSizeCmbBx->currentIndex() ) );
  background.setSize( QSizeF( mShapeSizeXSpnBx->value(), mShapeSizeYSpnBx->value() ) );
  background.setSizeUnit( mShapeSizeUnitWidget->unit() );
  background.setSizeMapUnitScale( mShapeSizeUnitWidget->getMapUnitScale() );
  background.setRotationType( static_cast< QgsTextBackgroundSettings::RotationType >( mShapeRotationCmbBx->currentIndex() ) );
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
  background.setOpacity( mBackgroundOpacityWidget->opacity() );
  background.setBlendMode( mShapeBlendCmbBx->blendMode() );
  if ( mBackgroundEffect && ( !QgsPaintEffectRegistry::isDefaultStack( mBackgroundEffect.get() ) || mBackgroundEffect->enabled() ) )
    background.setPaintEffect( mBackgroundEffect->clone() );
  else
    background.setPaintEffect( nullptr );
  background.setMarkerSymbol( mBackgroundMarkerSymbolButton->clonedSymbol< QgsMarkerSymbol >() );
  background.setFillSymbol( mBackgroundFillSymbolButton->clonedSymbol< QgsFillSymbol >() );
  format.setBackground( background );

  // drop shadow
  QgsTextShadowSettings shadow;
  shadow.setEnabled( mShadowDrawChkBx->isChecked() );
  shadow.setShadowPlacement( static_cast< QgsTextShadowSettings::ShadowPlacement >( mShadowUnderCmbBx->currentData().toInt() ) );
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

  if ( includeDataDefinedProperties )
    format.setDataDefinedProperties( mDataDefinedProperties );

  return format;
}

void QgsTextFormatWidget::setFormat( const QgsTextFormat &format )
{
  if ( mWidgetMode != Labeling )
  {
    // we need to combine any data defined properties from the text format with existing ones from the label settings
    const QgsPropertyCollection formatProps = format.dataDefinedProperties();
    for ( const int key : formatProps.propertyKeys() )
    {
      if ( formatProps.isActive( key ) )
      {
        mDataDefinedProperties.setProperty( key, formatProps.property( key ) );
      }
    }
  }

  updateWidgetForFormat( format );
}

QgsSymbolWidgetContext QgsTextFormatWidget::context() const
{
  return mContext;
}

void QgsTextFormatWidget::deactivateField( QgsPalLayerSettings::Property key )
{
  if ( mButtons.contains( key ) )
  {
    QgsPropertyOverrideButton *button = mButtons[ key ];
    button->updateFieldLists();
    button->setToProperty( QgsProperty() );
    mDataDefinedProperties.setProperty( key, QgsProperty() );
  }
}

void QgsTextFormatWidget::optionsStackedWidget_CurrentChanged( int indx )
{
  mLabelingOptionsListWidget->blockSignals( true );
  mLabelingOptionsListWidget->setCurrentRow( indx );
  mLabelingOptionsListWidget->blockSignals( false );
}

void QgsTextFormatWidget::setContext( const QgsSymbolWidgetContext &context )
{
  mContext = context;

  if ( auto *lExpressionContext = mContext.expressionContext() )
  {
    mPreviewExpressionContext = *lExpressionContext;
    if ( mLayer )
      mPreviewExpressionContext.appendScope( QgsExpressionContextUtils::layerScope( mLayer ) );
  }

  const auto symbolButtonWidgets = findChildren<QgsSymbolButton *>();
  for ( QgsSymbolButton *symbolWidget : symbolButtonWidgets )
  {
    symbolWidget->setMapCanvas( mContext.mapCanvas() );
    symbolWidget->setMessageBar( mContext.messageBar() );
  }
}

void QgsTextFormatWidget::collapseSample( bool collapse )
{
  if ( collapse )
  {
    QList<int> splitSizes = mFontPreviewSplitter->sizes();
    if ( splitSizes[0] > groupBox_mPreview->height() )
    {
      const int delta = splitSizes[0] - groupBox_mPreview->height();
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
  mFontUnderlineBtn->setChecked( mRefFont.underline() );
  mFontStrikethroughBtn->setChecked( mRefFont.strikeOut() );
  mKerningCheckBox->setChecked( mRefFont.kerning() );
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
  mKerningCheckBox->blockSignals( blk );
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
  mPreviewBackgroundColor = color;

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
  const QgsWkbTypes::GeometryType currentGeometryType = labelGeometryType();
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

  const QgsPalLayerSettings::Placement currentPlacement = static_cast< QgsPalLayerSettings::Placement >( mPlacementModeComboBox->currentData().toInt() );
  const bool showPolygonPlacementOptions = ( currentGeometryType == QgsWkbTypes::PolygonGeometry && currentPlacement != QgsPalLayerSettings::Line && currentPlacement != QgsPalLayerSettings::PerimeterCurved && currentPlacement != QgsPalLayerSettings::OutsidePolygons );

  bool enableMultiLinesFrame = true;

  if ( currentPlacement == QgsPalLayerSettings::AroundPoint
       && ( currentGeometryType == QgsWkbTypes::PointGeometry || currentGeometryType == QgsWkbTypes::PolygonGeometry ) )
  {
    showCentroidFrame = currentGeometryType == QgsWkbTypes::PolygonGeometry;
    showDistanceFrame = true;
    //showRotationFrame = true; // TODO: uncomment when supported
    showQuadrantFrame = currentGeometryType == QgsWkbTypes::PointGeometry;
  }
  else if ( currentPlacement == QgsPalLayerSettings::OverPoint
            && ( currentGeometryType == QgsWkbTypes::PointGeometry || currentGeometryType == QgsWkbTypes::PolygonGeometry ) )
  {
    showCentroidFrame = currentGeometryType == QgsWkbTypes::PolygonGeometry;
    showQuadrantFrame = true;
    showFixedQuadrantFrame = true;
    showOffsetFrame = true;
    showRotationFrame = true;
  }
  else if ( currentGeometryType == QgsWkbTypes::PointGeometry && currentPlacement == QgsPalLayerSettings::OrderedPositionsAroundPoint )
  {
    showDistanceFrame = true;
    showPlacementPriorityFrame = true;
    showOffsetTypeFrame  = true;
  }
  else if ( ( currentGeometryType == QgsWkbTypes::LineGeometry && currentPlacement == QgsPalLayerSettings::Line )
            || ( currentGeometryType == QgsWkbTypes::PolygonGeometry && currentPlacement == QgsPalLayerSettings::Line )
            || ( currentGeometryType == QgsWkbTypes::LineGeometry && currentPlacement == QgsPalLayerSettings::Curved )
            || ( currentGeometryType == QgsWkbTypes::PolygonGeometry && currentPlacement == QgsPalLayerSettings::PerimeterCurved ) )
  {
    showLineFrame = true;
    showDistanceFrame = true;
    //showRotationFrame = true; // TODO: uncomment when supported

    const bool offline = chkLineAbove->isChecked() || chkLineBelow->isChecked();
    chkLineOrientationDependent->setEnabled( offline );
    mPlacementDistanceFrame->setEnabled( offline );

    const bool isCurved = ( currentGeometryType == QgsWkbTypes::LineGeometry && currentPlacement == QgsPalLayerSettings::Curved )
                          || ( currentGeometryType == QgsWkbTypes::PolygonGeometry && currentPlacement == QgsPalLayerSettings::PerimeterCurved );
    showMaxCharAngleFrame = isCurved;
    // TODO: enable mMultiLinesFrame when supported for curved labels
    enableMultiLinesFrame = !isCurved;
  }
  else if ( currentGeometryType == QgsWkbTypes::PolygonGeometry
            && ( currentPlacement == QgsPalLayerSettings::OutsidePolygons || mCheckAllowLabelsOutsidePolygons->isChecked() || mAllowOutsidePolygonsDDBtn->isActive() ) )
  {
    showDistanceFrame = true;
  }

  mPlacementLineFrame->setVisible( showLineFrame );
  mPlacementPolygonFrame->setVisible( showPolygonPlacementOptions );
  mPlacementCentroidFrame->setVisible( showCentroidFrame );
  mPlacementQuadrantFrame->setVisible( showQuadrantFrame );
  mPlacementFixedQuadrantFrame->setVisible( showFixedQuadrantFrame );
  mPlacementCartographicFrame->setVisible( showPlacementPriorityFrame );
  mPlacementOffsetFrame->setVisible( showOffsetFrame );
  mPlacementDistanceFrame->setVisible( showDistanceFrame );
  mPlacementOffsetTypeFrame->setVisible( showOffsetTypeFrame );
  mPlacementRotationFrame->setVisible( showRotationFrame );
  mPlacementRepeatGroupBox->setVisible( currentGeometryType == QgsWkbTypes::LineGeometry || ( currentGeometryType == QgsWkbTypes::PolygonGeometry &&
                                        ( currentPlacement == QgsPalLayerSettings::Line || currentPlacement == QgsPalLayerSettings::PerimeterCurved ) ) );
  mPlacementOverrunGroupBox->setVisible( currentGeometryType == QgsWkbTypes::LineGeometry && currentPlacement != QgsPalLayerSettings::Horizontal );
  mLineAnchorGroupBox->setVisible( currentGeometryType == QgsWkbTypes::LineGeometry );
  mPlacementMaxCharAngleFrame->setVisible( showMaxCharAngleFrame );

  mMultiLinesFrame->setEnabled( enableMultiLinesFrame );


  QString helperText;
  switch ( currentPlacement )
  {
    case QgsPalLayerSettings::AroundPoint:
      if ( currentGeometryType == QgsWkbTypes::PointGeometry )
        helperText = tr( "Arranges label candidates in a clockwise circle around the feature, preferring placements to the top-right of the feature." );
      else if ( currentGeometryType == QgsWkbTypes::PolygonGeometry )
        helperText = tr( "Arranges label candidates in a cluster around the feature's centroid, preferring placements directly over the centroid." );
      break;
    case QgsPalLayerSettings::OverPoint:
      if ( currentGeometryType == QgsWkbTypes::PointGeometry )
        helperText = tr( "Arranges label candidates directly over the feature or at a preset offset from the feature." );
      else if ( currentGeometryType == QgsWkbTypes::PolygonGeometry )
        helperText = tr( "Arranges label candidates directly over the feature's centroid, or at a preset offset from the centroid." );
      break;
    case QgsPalLayerSettings::Line:
      if ( currentGeometryType == QgsWkbTypes::LineGeometry )
        helperText = tr( "Arranges label candidates parallel to a generalised line representing the feature. Placements which fall over straighter portions of the line are preferred." );
      else if ( currentGeometryType == QgsWkbTypes::PolygonGeometry )
        helperText = tr( "Arranges label candidates parallel to a generalised line representing the polygon's perimeter. Placements which fall over straighter portions of the perimeter are preferred." );
      break;
    case QgsPalLayerSettings::Curved:
      if ( currentGeometryType == QgsWkbTypes::LineGeometry )
        helperText = tr( "Arranges candidates following the curvature of a line feature. Placements which fall over straighter portions of the line are preferred." );
      break;
    case QgsPalLayerSettings::Horizontal:
      if ( currentGeometryType == QgsWkbTypes::PolygonGeometry )
        helperText = tr( "Arranges label candidates scattered throughout the polygon. Labels will always be placed horizontally, with placements further from the edges of the polygon preferred." );
      else if ( currentGeometryType == QgsWkbTypes::LineGeometry )
        helperText = tr( "Label candidates are arranged horizontally along the length of the feature." );
      break;
    case QgsPalLayerSettings::Free:
      if ( currentGeometryType == QgsWkbTypes::PolygonGeometry )
        helperText = tr( "Arranges label candidates scattered throughout the polygon. Labels are rotated to respect the polygon's orientation, with placements further from the edges of the polygon preferred." );
      break;
    case QgsPalLayerSettings::OrderedPositionsAroundPoint:
      if ( currentGeometryType == QgsWkbTypes::PointGeometry )
        helperText = tr( "Label candidates are placed in predefined positions around the features. Preference is given to positions with greatest cartographic appeal, e.g., top right and bottom right of the feature." );
      break;
    case QgsPalLayerSettings::PerimeterCurved:
      if ( currentGeometryType == QgsWkbTypes::PolygonGeometry )
        helperText = tr( "Arranges candidates following the curvature of the feature's perimeter. Placements which fall over straighter portions of the perimeter are preferred." );
      break;
    case QgsPalLayerSettings::OutsidePolygons:
      if ( currentGeometryType == QgsWkbTypes::PolygonGeometry )
        helperText = tr( "Label candidates are placed outside of the features, preferring placements which give greatest visual association between the label and the feature." );
      break;
  }
  mPlacementModeDescriptionLabel->setText( QStringLiteral( "<i>%1</i>" ).arg( helperText ) );
}

void QgsTextFormatWidget::populateFontCapitalsComboBox()
{
  mFontCapitalsComboBox->addItem( tr( "No Change" ), static_cast< int >( Qgis::Capitalization::MixedCase ) );
  mFontCapitalsComboBox->addItem( tr( "All Uppercase" ), static_cast< int >( Qgis::Capitalization::AllUppercase ) );
  mFontCapitalsComboBox->addItem( tr( "All Lowercase" ), static_cast< int >( Qgis::Capitalization::AllLowercase ) );
#if defined(HAS_KDE_QT5_SMALL_CAPS_FIX) || QT_VERSION >= QT_VERSION_CHECK(6, 3, 0)
  // Requires new enough build due to
  // https://bugreports.qt.io/browse/QTBUG-13965
  mFontCapitalsComboBox->addItem( tr( "Small Caps" ), static_cast< int >( Qgis::Capitalization::SmallCaps ) );
  mFontCapitalsComboBox->addItem( tr( "All Small Caps" ), static_cast< int >( Qgis::Capitalization::AllSmallCaps ) );
#endif
  mFontCapitalsComboBox->addItem( tr( "Title Case" ), static_cast< int >( Qgis::Capitalization::TitleCase ) );
  mFontCapitalsComboBox->addItem( tr( "Force First Letter to Capital" ), static_cast< int >( Qgis::Capitalization::ForceFirstLetterToCapital ) );
}

void QgsTextFormatWidget::populateFontStyleComboBox()
{
  mFontStyleComboBox->clear();
  const QStringList styles = mFontDB.styles( mRefFont.family() );
  const auto constStyles = styles;
  for ( const QString &style : constStyles )
  {
    mFontStyleComboBox->addItem( style );
  }

  QString targetStyle = mFontDB.styleString( mRefFont );
  if ( !styles.contains( targetStyle ) )
  {
    const QFont f = QFont( mRefFont.family() );
    targetStyle = QFontInfo( f ).styleName();
    mRefFont.setStyleName( targetStyle );
  }
  int curIndx = 0;
  const int stylIndx = mFontStyleComboBox->findText( targetStyle );
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

void QgsTextFormatWidget::kerningToggled( bool checked )
{
  mRefFont.setKerning( checked );
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

void QgsTextFormatWidget::mMaskBufferUnitWidget_changed()
{
  updateFont( mRefFont );
}

void QgsTextFormatWidget::mCoordXDDBtn_changed()
{
  updateDataDefinedAlignment();
}

void QgsTextFormatWidget::mCoordXDDBtn_activated( bool isActive )
{
  if ( !isActive )
    return;

  mCoordPointDDBtn->setActive( false );
}

void QgsTextFormatWidget::mCoordYDDBtn_changed()
{
  updateDataDefinedAlignment();
}

void QgsTextFormatWidget::mCoordYDDBtn_activated( bool isActive )
{
  if ( !isActive )
    return;

  mCoordPointDDBtn->setActive( false );
}

void QgsTextFormatWidget::mCoordPointDDBtn_changed()
{
  updateDataDefinedAlignment();
}

void QgsTextFormatWidget::mCoordPointDDBtn_activated( bool isActive )
{
  if ( !isActive )
    return;

  mCoordXDDBtn->setActive( false );
  mCoordYDDBtn->setActive( false );
}

void QgsTextFormatWidget::mShapeTypeCmbBx_currentIndexChanged( int )
{
  // shape background
  const QgsTextBackgroundSettings::ShapeType type = static_cast< QgsTextBackgroundSettings::ShapeType >( mShapeTypeCmbBx->currentData().toInt() );
  const bool isRect = type == QgsTextBackgroundSettings::ShapeRectangle || type == QgsTextBackgroundSettings::ShapeSquare;
  const bool isSVG = type == QgsTextBackgroundSettings::ShapeSVG;
  const bool isMarker = type == QgsTextBackgroundSettings::ShapeMarkerSymbol;

  showBackgroundRadius( isRect );

  mShapeSVGPathFrame->setVisible( isSVG );
  mBackgroundMarkerSymbolButton->setVisible( isMarker );
  mBackgroundFillSymbolButton->setVisible( !isSVG && !isMarker );

  // symbology SVG and marker renderers only support size^2 scaling,
  // so we only use the x size spinbox
  mShapeSizeYLabel->setVisible( !isSVG && !isMarker );
  mShapeSizeYSpnBx->setVisible( !isSVG && !isMarker );
  mShapeSizeYDDBtn->setVisible( !isSVG && !isMarker );
  mShapeSizeXLabel->setText( tr( "Size%1" ).arg( !isSVG && !isMarker ? tr( " X" ) : QString() ) );

  // SVG parameter setting doesn't support color's alpha component yet
  mShapeFillColorBtn->setAllowOpacity( !isSVG );
  mShapeFillColorBtn->setButtonBackground();
  mShapeStrokeColorBtn->setAllowOpacity( !isSVG );
  mShapeStrokeColorBtn->setButtonBackground();

  // Hide parameter widgets not used by marker symbol
  mShapeFillColorLabel->setVisible( isSVG );
  mShapeFillColorLabel->setEnabled( isSVG );
  mShapeFillColorBtn->setVisible( isSVG );
  mShapeFillColorBtn->setEnabled( isSVG );
  mShapeFillColorDDBtn->setVisible( isSVG );
  mShapeFillColorDDBtn->setEnabled( isSVG );
  mShapeStrokeColorLabel->setVisible( isSVG );
  mShapeStrokeColorLabel->setEnabled( isSVG );
  mShapeStrokeColorBtn->setVisible( isSVG );
  mShapeStrokeColorBtn->setEnabled( isSVG );
  mShapeStrokeColorDDBtn->setVisible( isSVG );
  mShapeStrokeColorDDBtn->setEnabled( isSVG );
  mShapeStrokeWidthLabel->setVisible( isSVG );
  mShapeStrokeWidthLabel->setEnabled( isSVG );
  mShapeStrokeWidthSpnBx->setVisible( isSVG );
  mShapeStrokeWidthSpnBx->setEnabled( isSVG );
  mShapeStrokeWidthDDBtn->setVisible( isSVG );
  mShapeStrokeWidthDDBtn->setEnabled( isSVG );

  // configure SVG parameter widgets
  mShapeSVGParamsBtn->setVisible( isSVG );
  if ( isSVG )
  {
    updateSvgWidgets( mShapeSVGPathLineEdit->text() );
  }
  // TODO: fix overriding SVG symbol's stroke width units in QgsSvgCache
  // currently broken, fall back to symbol units only
  mShapeSVGUnitsLabel->setVisible( isSVG );
  mShapeStrokeWidthUnitWidget->setVisible( false );
  mShapeStrokeUnitsDDBtn->setVisible( false );
  mShapeStrokeUnitsDDBtn->setEnabled( false );

  updateAvailableShadowPositions();
}

void QgsTextFormatWidget::mShapeSVGPathLineEdit_textChanged( const QString &text )
{
  updateSvgWidgets( text );
}

void QgsTextFormatWidget::updateLinePlacementOptions()
{
  const int numOptionsChecked = ( chkLineAbove->isChecked() ? 1 : 0 ) +
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

void QgsTextFormatWidget::updateAvailableShadowPositions()
{
  if ( mShadowUnderCmbBx->count() == 0
       || ( mShadowUnderCmbBx->findData( QgsTextShadowSettings::ShadowShape ) > -1 && mShapeTypeCmbBx->currentData().toInt() == QgsTextBackgroundSettings::ShapeMarkerSymbol )
       || ( mShadowUnderCmbBx->findData( QgsTextShadowSettings::ShadowShape ) == -1 && mShapeTypeCmbBx->currentData().toInt() != QgsTextBackgroundSettings::ShapeMarkerSymbol ) )
  {
    // showing invalid choices, have to rebuild the list
    const QgsTextShadowSettings::ShadowPlacement currentPlacement = static_cast< QgsTextShadowSettings::ShadowPlacement >( mShadowUnderCmbBx->currentData().toInt() );
    mShadowUnderCmbBx->clear();

    mShadowUnderCmbBx->addItem( tr( "Lowest Label Component" ), QgsTextShadowSettings::ShadowLowest );
    mShadowUnderCmbBx->addItem( tr( "Text" ), QgsTextShadowSettings::ShadowText );
    mShadowUnderCmbBx->addItem( tr( "Buffer" ), QgsTextShadowSettings::ShadowBuffer );
    if ( mShapeTypeCmbBx->currentData().toInt() != QgsTextBackgroundSettings::ShapeMarkerSymbol )
      mShadowUnderCmbBx->addItem( tr( "Background" ), QgsTextShadowSettings::ShadowShape ); // not supported for marker symbol background shapes

    mShadowUnderCmbBx->setCurrentIndex( mShadowUnderCmbBx->findData( currentPlacement ) );
    if ( mShadowUnderCmbBx->currentIndex() == -1 )
      mShadowUnderCmbBx->setCurrentIndex( 0 );
  }
}

void QgsTextFormatWidget::updateProperty()
{
  QgsPropertyOverrideButton *button = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  const QgsPalLayerSettings::Property key = static_cast< QgsPalLayerSettings::Property >( button->propertyKey() );
  mDataDefinedProperties.setProperty( key, button->toProperty() );
  updatePreview();
}

void QgsTextFormatWidget::createAuxiliaryField()
{
  if ( !mLayer )
    return;

  // try to create an auxiliary layer if not yet created
  if ( !mLayer->auxiliaryLayer() )
  {
    QgsNewAuxiliaryLayerDialog dlg( mLayer, this );
    dlg.exec();
  }

  // return if still not exists
  if ( !mLayer->auxiliaryLayer() )
    return;

  QgsPropertyOverrideButton *button = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  const QgsPalLayerSettings::Property key = static_cast< QgsPalLayerSettings::Property >( button->propertyKey() );
  const QgsPropertyDefinition def = QgsPalLayerSettings::propertyDefinitions()[key];

  // create property in auxiliary storage if necessary
  if ( !mLayer->auxiliaryLayer()->exists( def ) )
    mLayer->auxiliaryLayer()->addAuxiliaryField( def );

  // update property with join field name from auxiliary storage
  QgsProperty property = button->toProperty();
  property.setField( QgsAuxiliaryLayer::nameFromProperty( def, true ) );
  property.setActive( true );
  button->updateFieldLists();
  button->setToProperty( property );
  mDataDefinedProperties.setProperty( key, button->toProperty() );
  updatePreview();
  emit auxiliaryFieldCreated();
}


void QgsTextFormatWidget::updateShapeFrameStatus()
{
  mShapeFrame->setEnabled( mShapeDrawDDBtn->isActive() || mShapeDrawChkBx->isChecked() );
}

void QgsTextFormatWidget::updateBufferFrameStatus()
{
  mBufferFrame->setEnabled( mBufferDrawDDBtn->isActive() || mBufferDrawChkBx->isChecked() );
}

void QgsTextFormatWidget::updateShadowFrameStatus()
{
  mShadowFrame->setEnabled( mShadowDrawDDBtn->isActive() || mShadowDrawChkBx->isChecked() );
}

void QgsTextFormatWidget::updateCalloutFrameStatus()
{
  mCalloutFrame->setEnabled( mCalloutDrawDDBtn->isActive() || mCalloutsDrawCheckBox->isChecked() );
}

void QgsTextFormatWidget::updateDataDefinedAlignment()
{
  // no data defined alignment without data defined position
  mCoordAlignmentFrame->setEnabled( ( mCoordXDDBtn->isActive() && mCoordYDDBtn->isActive() )
                                    || mCoordPointDDBtn->isActive() );
}

void QgsTextFormatWidget::setFormatFromStyle( const QString &name, QgsStyle::StyleEntity type )
{
  switch ( type )
  {
    case QgsStyle::SymbolEntity:
    case QgsStyle::ColorrampEntity:
    case QgsStyle::TagEntity:
    case QgsStyle::SmartgroupEntity:
    case QgsStyle::LegendPatchShapeEntity:
    case QgsStyle::Symbol3DEntity:
      return;

    case QgsStyle::TextFormatEntity:
    {
      if ( !QgsStyle::defaultStyle()->textFormatNames().contains( name ) )
        return;

      const QgsTextFormat newFormat = QgsStyle::defaultStyle()->textFormat( name );
      setFormat( newFormat );
      break;
    }

    case QgsStyle::LabelSettingsEntity:
    {
      if ( !QgsStyle::defaultStyle()->labelSettingsNames().contains( name ) )
        return;

      const QgsTextFormat newFormat = QgsStyle::defaultStyle()->labelSettings( name ).format();
      setFormat( newFormat );
      break;
    }
  }
}

void QgsTextFormatWidget::saveFormat()
{
  QgsStyle *style = QgsStyle::defaultStyle();
  if ( !style )
    return;

  QgsStyleSaveDialog saveDlg( this, QgsStyle::TextFormatEntity );
  saveDlg.setDefaultTags( mTextFormatsListWidget->currentTagFilter() );
  if ( !saveDlg.exec() )
    return;

  if ( saveDlg.name().isEmpty() )
    return;

  // check if there is no format with same name
  if ( style->textFormatNames().contains( saveDlg.name() ) )
  {
    const int res = QMessageBox::warning( this, tr( "Save Text Format" ),
                                          tr( "Format with name '%1' already exists. Overwrite?" )
                                          .arg( saveDlg.name() ),
                                          QMessageBox::Yes | QMessageBox::No );
    if ( res != QMessageBox::Yes )
    {
      return;
    }
    style->removeTextFormat( saveDlg.name() );
  }

  const QStringList symbolTags = saveDlg.tags().split( ',' );

  const QgsTextFormat newFormat = format();
  style->addTextFormat( saveDlg.name(), newFormat );
  style->saveTextFormat( saveDlg.name(), newFormat, saveDlg.isFavorite(), symbolTags );
}

void QgsTextFormatWidget::mShapeSVGSelectorBtn_clicked()
{
  QgsSvgSelectorDialog svgDlg( this );
  svgDlg.setWindowTitle( tr( "Select SVG file" ) );
  svgDlg.svgSelector()->setSvgPath( mShapeSVGPathLineEdit->text().trimmed() );

  if ( svgDlg.exec() == QDialog::Accepted )
  {
    const QString svgPath = svgDlg.svgSelector()->currentSvgPath();
    if ( !svgPath.isEmpty() )
    {
      mShapeSVGPathLineEdit->setText( svgPath );
      updatePreview();
    }
  }
}

void QgsTextFormatWidget::mShapeSVGParamsBtn_clicked()
{
  const QString svgPath = mShapeSVGPathLineEdit->text();
  mLoadSvgParams = true;
  updateSvgWidgets( svgPath );
  mLoadSvgParams = false;
}

void QgsTextFormatWidget::mShapeRotationCmbBx_currentIndexChanged( int index )
{
  mShapeRotationDblSpnBx->setEnabled( static_cast< QgsTextBackgroundSettings::RotationType >( index ) != QgsTextBackgroundSettings::RotationSync );
  mShapeRotationDDBtn->setEnabled( static_cast< QgsTextBackgroundSettings::RotationType >( index ) != QgsTextBackgroundSettings::RotationSync );
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

  const QChar initial = !mDirectSymbLeftLineEdit->text().isEmpty() ? mDirectSymbLeftLineEdit->text().at( 0 ) : QChar();
  const QChar dirSymb = mCharDlg->selectCharacter( &gotChar, mRefFont, mFontDB.styleString( mRefFont ), initial );

  if ( !gotChar )
    return;

  if ( !dirSymb.isNull() )
    mDirectSymbLeftLineEdit->setText( QString( dirSymb ) );
}

void QgsTextFormatWidget::mDirectSymbRightToolBtn_clicked()
{
  bool gotChar = false;
  const QChar initial = !mDirectSymbRightLineEdit->text().isEmpty() ? mDirectSymbRightLineEdit->text().at( 0 ) : QChar();
  const QChar dirSymb = mCharDlg->selectCharacter( &gotChar, mRefFont, mFontDB.styleString( mRefFont ), initial );

  if ( !gotChar )
    return;

  if ( !dirSymb.isNull() )
    mDirectSymbRightLineEdit->setText( QString( dirSymb ) );
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

  mShapeRadiusDDBtn->setVisible( show );
  mShapeRadiusUnitsDDBtn->setVisible( show );
}

QgsExpressionContext QgsTextFormatWidget::createExpressionContext() const
{
  if ( auto *lExpressionContext = mContext.expressionContext() )
    return *lExpressionContext;

  QgsExpressionContext expContext;
  expContext << QgsExpressionContextUtils::globalScope()
             << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
             << QgsExpressionContextUtils::atlasScope( nullptr );
  if ( mMapCanvas )
    expContext << QgsExpressionContextUtils::mapSettingsScope( mMapCanvas->mapSettings() );

  if ( mLayer )
    expContext << QgsExpressionContextUtils::layerScope( mLayer );

  //TODO - show actual value
  expContext.setOriginalValueVariable( QVariant() );
  expContext.setHighlightedVariables( QStringList() << QgsExpressionContext::EXPR_ORIGINAL_VALUE );

  return expContext;
}

QgsWkbTypes::GeometryType QgsTextFormatWidget::labelGeometryType() const
{
  if ( mGeometryGeneratorGroupBox->isChecked() )
    return mGeometryGeneratorType->currentData().value<QgsWkbTypes::GeometryType>();
  else if ( mLayer )
    return mLayer->geometryType();
  else
    return mGeomType;
}


//
// QgsTextFormatDialog
//

QgsTextFormatDialog::QgsTextFormatDialog( const QgsTextFormat &format, QgsMapCanvas *mapCanvas, QWidget *parent, Qt::WindowFlags fl, QgsVectorLayer *layer )
  : QDialog( parent, fl )
{
  setWindowTitle( tr( "Text Settings" ) );

  mFormatWidget = new QgsTextFormatWidget( format, mapCanvas, this, layer );
  mFormatWidget->layout()->setContentsMargins( 0, 0, 0, 0 );

  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->addWidget( mFormatWidget );

  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, Qt::Horizontal, this );
  layout->addWidget( mButtonBox );

  setLayout( layout );
  QgsGui::enableAutoGeometryRestore( this );

  connect( mButtonBox->button( QDialogButtonBox::Ok ), &QAbstractButton::clicked, this, &QDialog::accept );
  connect( mButtonBox->button( QDialogButtonBox::Cancel ), &QAbstractButton::clicked, this, &QDialog::reject );
  connect( mButtonBox->button( QDialogButtonBox::Help ), &QAbstractButton::clicked, this, &QgsTextFormatDialog::showHelp );
}

QgsTextFormat QgsTextFormatDialog::format() const
{
  return mFormatWidget->format();
}

void QgsTextFormatDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "style_library/label_settings.html#formatting-the-label-text" ) );
}

void QgsTextFormatDialog::setContext( const QgsSymbolWidgetContext &context )
{
  mFormatWidget->setContext( context );
}

QDialogButtonBox *QgsTextFormatDialog::buttonBox() const
{
  return mButtonBox;
}

QgsTextFormatPanelWidget::QgsTextFormatPanelWidget( const QgsTextFormat &format, QgsMapCanvas *mapCanvas, QWidget *parent, QgsVectorLayer *layer )
  : QgsPanelWidgetWrapper( new QgsTextFormatWidget( format, mapCanvas, nullptr, layer ), parent )
{
  mFormatWidget = qobject_cast< QgsTextFormatWidget * >( widget() );
  connect( mFormatWidget, &QgsTextFormatWidget::widgetChanged, this, [ = ]
  {
    if ( !mBlockSignals )
      emit widgetChanged();
  } );
}

QgsTextFormat QgsTextFormatPanelWidget::format() const
{
  return mFormatWidget->format();
}

void QgsTextFormatPanelWidget::setFormat( const QgsTextFormat &format )
{
  mBlockSignals = true;
  mFormatWidget->setFormat( format );
  mBlockSignals = false;
}

void QgsTextFormatPanelWidget::setContext( const QgsSymbolWidgetContext &context )
{
  mFormatWidget->setContext( context );
}

void QgsTextFormatPanelWidget::setDockMode( bool dockMode )
{
  mFormatWidget->setDockMode( dockMode );
  QgsPanelWidgetWrapper::setDockMode( dockMode );
}
