/***************************************************************************
  qgslabelinggui.cpp
  Smart labeling for vector layers
  -------------------
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

#include "qgslabelinggui.h"

#include <qgsmapcanvas.h>
#include <qgsvectorlayer.h>
#include <qgsvectordataprovider.h>
#include <qgsmaplayerregistry.h>

#include "qgslabelengineconfigdialog.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsexpression.h"
#include "qgisapp.h"
#include "qgsmaprenderer.h"
#include "qgsproject.h"
#include "qgssvgcache.h"
#include "qgscharacterselectdialog.h"
#include "qgssvgselectorwidget.h"

#include <QColorDialog>
#include <QFileDialog>
#include <QFontDialog>
#include <QTextEdit>
#include <QApplication>
#include <QMessageBox>
#include <QSettings>


QgsLabelingGui::QgsLabelingGui( QgsPalLabeling* lbl, QgsVectorLayer* layer, QgsMapCanvas* mapCanvas, QWidget* parent )
    : QWidget( parent ), mLBL( lbl ), mLayer( layer ), mMapCanvas( mapCanvas )
{
  if ( !layer ) return;

  setupUi( this );
  mCharDlg = new QgsCharacterSelectorDialog( this );

  mRefFont = lblFontPreview->font();
  mPreviewSize = 24;

  connect( btnTextColor, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( changeTextColor( const QColor& ) ) );
  connect( btnChangeFont, SIGNAL( clicked() ), this, SLOT( changeTextFont() ) );
  connect( mFontTranspSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( updatePreview() ) );
  connect( chkBuffer, SIGNAL( toggled( bool ) ), this, SLOT( updatePreview() ) );
  connect( btnBufferColor, SIGNAL( colorChanged( const QColor& ) ), this, SLOT( changeBufferColor( const QColor& ) ) );
  connect( spinBufferSize, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  connect( mBufferTranspSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( updatePreview() ) );
  connect( mBufferJoinStyleComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updatePreview() ) );
  connect( mBufferTranspFillChbx, SIGNAL( toggled( bool ) ), this, SLOT( updatePreview() ) );
  connect( btnEngineSettings, SIGNAL( clicked() ), this, SLOT( showEngineConfigDialog() ) );
  connect( btnExpression, SIGNAL( clicked() ), this, SLOT( showExpressionDialog() ) );

  // set placement methods page based on geometry type
  switch ( layer->geometryType() )
  {
    case QGis::Point:
      stackedPlacement->setCurrentWidget( pagePoint );
      break;
    case QGis::Line:
      stackedPlacement->setCurrentWidget( pageLine );
      break;
    case QGis::Polygon:
      stackedPlacement->setCurrentWidget( pagePolygon );
      break;
    case QGis::NoGeometry:
      break;
    default:
      Q_ASSERT( 0 && "NOOOO!" );
  }

  //mTabWidget->setEnabled( chkEnableLabeling->isChecked() );
  chkMergeLines->setEnabled( layer->geometryType() == QGis::Line );
  mDirectSymbGroupBox->setEnabled( layer->geometryType() == QGis::Line );
  label_19->setEnabled( layer->geometryType() != QGis::Point );
  mMinSizeSpinBox->setEnabled( layer->geometryType() != QGis::Point );

  // load labeling settings from layer
  QgsPalLayerSettings lyr;
  lyr.readFromLayer( layer );
  populateFieldNames();
  populateDataDefinedCombos( lyr );
  populateFontCapitalsComboBox();

  chkEnableLabeling->setChecked( lyr.enabled );
  mTabWidget->setEnabled( lyr.enabled );
  cboFieldName->setEnabled( lyr.enabled );
  btnExpression->setEnabled( lyr.enabled );

  //Add the current expression to the bottom of the list.
  if ( lyr.isExpression && !lyr.fieldName.isEmpty() )
    cboFieldName->addItem( lyr.fieldName );

  // placement
  int distUnitIndex = lyr.distInMapUnits ? 1 : 0;
  mXQuadOffset = lyr.xQuadOffset;
  mYQuadOffset = lyr.yQuadOffset;
  mCentroidRadioWhole->setChecked( lyr.centroidWhole );
  mCentroidFrame->setVisible( false );
  switch ( lyr.placement )
  {
    case QgsPalLayerSettings::AroundPoint:
      radAroundPoint->setChecked( true );
      radAroundCentroid->setChecked( true );
      spinDistPoint->setValue( lyr.dist );
      mPointDistanceUnitComboBox->setCurrentIndex( distUnitIndex );
      mCentroidFrame->setVisible( layer->geometryType() == QGis::Polygon );

      //spinAngle->setValue( lyr.angle );
      break;
    case QgsPalLayerSettings::OverPoint:
      radOverPoint->setChecked( true );
      radOverCentroid->setChecked( true );

      mPointOffsetRadioAboveLeft->setChecked( mXQuadOffset == -1 && mYQuadOffset == 1 );
      mPointOffsetRadioAbove->setChecked( mXQuadOffset == 0 && mYQuadOffset == 1 );
      mPointOffsetRadioAboveRight->setChecked( mXQuadOffset == 1 && mYQuadOffset == 1 );
      mPointOffsetRadioLeft->setChecked( mXQuadOffset == -1 && mYQuadOffset == 0 );
      mPointOffsetRadioOver->setChecked( mXQuadOffset == 0 && mYQuadOffset == 0 );
      mPointOffsetRadioRight->setChecked( mXQuadOffset == 1 && mYQuadOffset == 0 );
      mPointOffsetRadioBelowLeft->setChecked( mXQuadOffset == -1 && mYQuadOffset == -1 );
      mPointOffsetRadioBelow->setChecked( mXQuadOffset == 0 && mYQuadOffset == -1 );
      mPointOffsetRadioBelowRight->setChecked( mXQuadOffset == 1 && mYQuadOffset == -1 );

      mPointOffsetXOffsetSpinBox->setValue( lyr.xOffset );
      mPointOffsetYOffsetSpinBox->setValue( lyr.yOffset );
      mPointOffsetUnitsComboBox->setCurrentIndex( lyr.labelOffsetInMapUnits ? 1 : 0 );
      mPointOffsetAngleSpinBox->setValue( lyr.angleOffset );
      mCentroidFrame->setVisible( layer->geometryType() == QGis::Polygon );

      break;
    case QgsPalLayerSettings::Line:
      radLineParallel->setChecked( true );
      radPolygonPerimeter->setChecked( true );
      break;
    case QgsPalLayerSettings::Curved:
      radLineCurved->setChecked( true );
      break;
    case QgsPalLayerSettings::Horizontal:
      radPolygonHorizontal->setChecked( true );
      radLineHorizontal->setChecked( true );
      break;
    case QgsPalLayerSettings::Free:
      radPolygonFree->setChecked( true );
      break;
    default:
      Q_ASSERT( 0 && "NOOO!" );
  }

  if ( lyr.placement == QgsPalLayerSettings::Line || lyr.placement == QgsPalLayerSettings::Curved )
  {
    spinDistLine->setValue( lyr.dist );
    mLineDistanceUnitComboBox->setCurrentIndex( distUnitIndex );
    chkLineAbove->setChecked( lyr.placementFlags & QgsPalLayerSettings::AboveLine );
    chkLineBelow->setChecked( lyr.placementFlags & QgsPalLayerSettings::BelowLine );
    chkLineOn->setChecked( lyr.placementFlags & QgsPalLayerSettings::OnLine );
    if ( !( lyr.placementFlags & QgsPalLayerSettings::MapOrientation ) )
      chkLineOrientationDependent->setChecked( true );
  }

  cboFieldName->setCurrentIndex( cboFieldName->findText( lyr.fieldName ) );
  chkEnableLabeling->setChecked( lyr.enabled );
  sliderPriority->setValue( lyr.priority );
  chkNoObstacle->setChecked( !lyr.obstacle );
  chkLabelPerFeaturePart->setChecked( lyr.labelPerPart );
  mPalShowAllLabelsForLayerChkBx->setChecked( lyr.displayAll );
  chkMergeLines->setChecked( lyr.mergeLines );
  mMinSizeSpinBox->setValue( lyr.minFeatureSize );
  mLimitLabelChkBox->setChecked( lyr.limitNumLabels );
  mLimitLabelSpinBox->setValue( lyr.maxNumLabels );
  mDirectSymbGroupBox->setChecked( lyr.addDirectionSymbol );
  mDirectSymbLeftLineEdit->setText( lyr.leftDirectionSymbol );
  mDirectSymbRightLineEdit->setText( lyr.rightDirectionSymbol );
  mDirectSymbRevChkBx->setChecked( lyr.reverseDirectionSymbol );
  switch ( lyr.placeDirectionSymbol )
  {
    case QgsPalLayerSettings::SymbolLeftRight:
      mDirectSymbRadioBtnLR->setChecked( true );
      break;
    case QgsPalLayerSettings::SymbolAbove:
      mDirectSymbRadioBtnAbove->setChecked( true );
      break;
    case QgsPalLayerSettings::SymbolBelow:
      mDirectSymbRadioBtnBelow->setChecked( true );
      break;
    default:
      mDirectSymbRadioBtnLR->setChecked( true );
      break;
  }

  // upside-down labels
  switch ( lyr.upsidedownLabels )
  {
    case QgsPalLayerSettings::Upright:
      mUpsidedownRadioOff->setChecked( true );
      break;
    case QgsPalLayerSettings::ShowDefined:
      mUpsidedownRadioDefined->setChecked( true );
      break;
    case QgsPalLayerSettings::ShowAll:
      mUpsidedownRadioAll->setChecked( true );
      break;
    default:
      mUpsidedownRadioOff->setChecked( true );
      break;
  }
  mMaxCharAngleInDSpinBox->setValue( lyr.maxCurvedCharAngleIn );
  // lyr.maxCurvedCharAngleOut must be negative, but it is shown as positive spinbox in GUI
  mMaxCharAngleOutDSpinBox->setValue( qAbs( lyr.maxCurvedCharAngleOut ) );

  wrapCharacterEdit->setText( lyr.wrapChar );
  mFontLineHeightSpinBox->setValue( lyr.multilineHeight );
  mFontMultiLineComboBox->setCurrentIndex(( unsigned int ) lyr.multilineAlign );
  chkPreserveRotation->setChecked( lyr.preserveRotation );

  mPreviewBackgroundBtn->setColor( lyr.previewBkgrdColor );
  setPreviewBackground( lyr.previewBkgrdColor );

  bool scaleBased = ( lyr.scaleMin != 0 && lyr.scaleMax != 0 );
  chkScaleBasedVisibility->setChecked( scaleBased );
  if ( scaleBased )
  {
    spinScaleMin->setValue( lyr.scaleMin );
    spinScaleMax->setValue( lyr.scaleMax );
  }

  bool buffer = ( lyr.bufferSize != 0 );
  chkBuffer->setChecked( buffer );
  if ( buffer )
  {
    spinBufferSize->setValue( lyr.bufferSize );
    if ( lyr.bufferSizeInMapUnits )
    {
      mBufferUnitComboBox->setCurrentIndex( 1 );
    }
    else
    {
      mBufferUnitComboBox->setCurrentIndex( 0 );
    }
    btnBufferColor->setColor( lyr.bufferColor );
    mBufferTranspSpinBox->setValue( lyr.bufferTransp );
    mBufferJoinStyleComboBox->setPenJoinStyle( lyr.bufferJoinStyle );
    mBufferTranspFillChbx->setChecked( !lyr.bufferNoFill );
    comboBufferBlendMode->setBlendMode( lyr.bufferBlendMode );
  }
  else
  {
    // default color
    // TODO: remove after moving to persistent PAL settings?
    btnBufferColor->setColor( Qt::white );
  }

  bool formattedNumbers = lyr.formatNumbers;
  bool plusSign = lyr.plusSign;

  chkFormattedNumbers->setChecked( formattedNumbers );
  if ( formattedNumbers )
  {
    spinDecimals->setValue( lyr.decimals );
  }
  if ( plusSign )
  {
    chkPlusSign->setChecked( plusSign );
  }

  // set pixel size limiting checked state before unit choice so limiting can be
  // turned on as a default for map units, if minimum trigger value of 0 is used
  mFontLimitPixelGroupBox->setChecked( lyr.fontLimitPixelSize );
  mMinPixelLimit = lyr.fontMinPixelSize; // ignored after first settings save
  mFontMinPixelSpinBox->setValue( lyr.fontMinPixelSize == 0 ? 3 : lyr.fontMinPixelSize );
  mFontMaxPixelSpinBox->setValue( lyr.fontMaxPixelSize );
  if ( lyr.fontSizeInMapUnits )
  {
    mFontSizeUnitComboBox->setCurrentIndex( 1 );
  }
  else
  {
    mFontSizeUnitComboBox->setCurrentIndex( 0 );
  }

  mRefFont = lyr.textFont;
  mFontSizeSpinBox->setValue( lyr.textFont.pointSizeF() );
  btnTextColor->setColor( lyr.textColor );
  mFontTranspSpinBox->setValue( lyr.textTransp );
  comboBlendMode->setBlendMode( lyr.blendMode );

  mFontWordSpacingSpinBox->setValue( lyr.textFont.wordSpacing() );
  mFontLetterSpacingSpinBox->setValue( lyr.textFont.letterSpacing() );

  updateFontViaStyle( lyr.textNamedStyle );
  updateFont( mRefFont );

  // shape background
  mShapeBackgroundGrpBx->setChecked( lyr.shapeDraw );
  mShapeTypeCmbBx->blockSignals( true );
  mShapeTypeCmbBx->setCurrentIndex( lyr.shapeType );
  mShapeTypeCmbBx->blockSignals( false );
  // set up SVG preview
  mSvgSelector = new QgsSvgSelectorWidget( this );
  mSVGSelectGrpBx->layout()->addWidget( mSvgSelector );
  mSvgSelector->setSvgPath( lyr.shapeSVGFile );

  mShapeSizeCmbBx->setCurrentIndex( lyr.shapeSizeType );
  mShapeSizeXSpnBx->setValue( lyr.shapeSize.x() );
  mShapeSizeYSpnBx->setValue( lyr.shapeSize.y() );
  mShapeSizeUnitsCmbBx->setCurrentIndex( lyr.shapeSizeUnits - 1 );
  mShapeRotationCmbBx->setCurrentIndex( lyr.shapeRotationType );
  mShapeRotationDblSpnBx->setEnabled( lyr.shapeRotationType != QgsPalLayerSettings::RotationSync );
  mShapeRotationDblSpnBx->setValue( lyr.shapeRotation );
  mShapeOffsetXSpnBx->setValue( lyr.shapeOffset.x() );
  mShapeOffsetYSpnBx->setValue( lyr.shapeOffset.y() );
  mShapeOffsetUnitsCmbBx->setCurrentIndex( lyr.shapeOffsetUnits - 1 );
  mShapeRadiusXDbSpnBx->setValue( lyr.shapeRadii.x() );
  mShapeRadiusYDbSpnBx->setValue( lyr.shapeRadii.y() );
  mShapeRadiusUnitsCmbBx->setCurrentIndex( lyr.shapeRadiiUnits - 1 );

  mShapeFillColorBtn->setColor( lyr.shapeFillColor );
  mShapeBorderColorBtn->setColor( lyr.shapeBorderColor );
  mShapeBorderWidthSpnBx->setValue( lyr.shapeBorderWidth );
  mShapeBorderWidthUnitsCmbBx->setCurrentIndex( lyr.shapeBorderWidthUnits - 1 );
  mShapePenStyleCmbBx->setPenJoinStyle( lyr.shapeJoinStyle );

  connect( mShapeTranspSlider, SIGNAL( valueChanged( int ) ), mShapeTranspSpinBox, SLOT( setValue( int ) ) );
  connect( mShapeTranspSpinBox, SIGNAL( valueChanged( int ) ), mShapeTranspSlider, SLOT( setValue( int ) ) );
  mShapeTranspSpinBox->setValue( lyr.shapeTransparency );
  mShapeBlendCmbBx->setBlendMode( QgsMapRenderer::getBlendModeEnum( lyr.shapeBlendMode ) );

  mLoadSvgParams = false;
  on_mShapeTypeCmbBx_currentIndexChanged( lyr.shapeType ); // force update of shape background gui

  connect( mSvgSelector, SIGNAL( svgSelected( const QString& ) ), this, SLOT( updateSvgWidgets( const QString& ) ) );

  mShapeCollisionsChkBx->setVisible( false ); // until implemented

  // drop shadow
  mShadowGrpBx->setVisible( false ); // until implemented


  updateUi();

  updateOptions();

  connect( chkBuffer, SIGNAL( toggled( bool ) ), this, SLOT( updateUi() ) );
  connect( chkScaleBasedVisibility, SIGNAL( toggled( bool ) ), this, SLOT( updateUi() ) );
  connect( chkFormattedNumbers, SIGNAL( toggled( bool ) ), this, SLOT( updateUi() ) );
  connect( chkLineAbove, SIGNAL( toggled( bool ) ), this, SLOT( updateUi() ) );
  connect( chkLineBelow, SIGNAL( toggled( bool ) ), this, SLOT( updateUi() ) );

  // setup connection to changes in the placement
  QRadioButton* placementRadios[] =
  {
    radAroundPoint, radOverPoint, // point
    radLineParallel, radLineCurved, radLineHorizontal, // line
    radAroundCentroid, radPolygonHorizontal, radPolygonFree, radPolygonPerimeter // polygon
  };
  for ( unsigned int i = 0; i < sizeof( placementRadios ) / sizeof( QRadioButton* ); i++ )
  {
    connect( placementRadios[i], SIGNAL( toggled( bool ) ), this, SLOT( updateOptions() ) );
  }

  // setup connections for label quadrant offsets
  QRadioButton* quadrantRadios[] =
  {
    mPointOffsetRadioAboveLeft, mPointOffsetRadioAbove, mPointOffsetRadioAboveRight,
    mPointOffsetRadioLeft, mPointOffsetRadioOver, mPointOffsetRadioRight,
    mPointOffsetRadioBelowLeft, mPointOffsetRadioBelow, mPointOffsetRadioBelowRight
  };
  for ( unsigned int i = 0; i < sizeof( quadrantRadios ) / sizeof( QRadioButton* ); i++ )
  {
    connect( quadrantRadios[i], SIGNAL( toggled( bool ) ), this, SLOT( updateQuadrant() ) );
  }

  // Global settings group for groupboxes' saved/retored collapsed state
  // maintains state across different dialogs
  foreach ( QgsCollapsibleGroupBox *grpbox, findChildren<QgsCollapsibleGroupBox*>() )
  {
    grpbox->setSettingGroup( QString( "mAdvLabelingDlg" ) );
  }

  connect( groupBox_mPreview,
           SIGNAL( collapsedStateChanged( bool ) ),
           this,
           SLOT( collapseSample( bool ) ) );
}

QgsLabelingGui::~QgsLabelingGui()
{
}

void QgsLabelingGui::collapseSample( bool collapse )
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

void QgsLabelingGui::apply()
{
  writeSettingsToLayer();
  QgisApp::instance()->markDirty();
  // trigger refresh
  if ( mMapCanvas )
  {
    mMapCanvas->refresh();
  }
}

void QgsLabelingGui::writeSettingsToLayer()
{
  QgsPalLayerSettings settings = layerSettings();
  settings.writeToLayer( mLayer );
}

QgsPalLayerSettings QgsLabelingGui::layerSettings()
{
  QgsPalLayerSettings lyr;
  lyr.fieldName = cboFieldName->currentText();
  // Check if we are an expression. Also treats expressions with just a column name as non expressions,
  // this saves time later so we don't have to parse the expression tree.
  lyr.isExpression = mLayer->fieldNameIndex( lyr.fieldName ) == -1 && !lyr.fieldName.isEmpty();

  lyr.dist = 0;
  lyr.placementFlags = 0;

  lyr.centroidWhole = mCentroidRadioWhole->isChecked();
  if (( stackedPlacement->currentWidget() == pagePoint && radAroundPoint->isChecked() )
      || ( stackedPlacement->currentWidget() == pagePolygon && radAroundCentroid->isChecked() ) )
  {
    lyr.placement = QgsPalLayerSettings::AroundPoint;
    lyr.dist = spinDistPoint->value();
    lyr.distInMapUnits = ( mPointDistanceUnitComboBox->currentIndex() == 1 );
  }
  else if (( stackedPlacement->currentWidget() == pagePoint && radOverPoint->isChecked() )
           || ( stackedPlacement->currentWidget() == pagePolygon && radOverCentroid->isChecked() ) )
  {
    lyr.placement = QgsPalLayerSettings::OverPoint;

    lyr.xQuadOffset = mXQuadOffset;
    lyr.yQuadOffset = mYQuadOffset;
    lyr.xOffset = mPointOffsetXOffsetSpinBox->value();
    lyr.yOffset = mPointOffsetYOffsetSpinBox->value();
    lyr.labelOffsetInMapUnits = ( mPointOffsetUnitsComboBox->currentIndex() == 1 );
    lyr.angleOffset = mPointOffsetAngleSpinBox->value();
  }
  else if (( stackedPlacement->currentWidget() == pageLine && radLineParallel->isChecked() )
           || ( stackedPlacement->currentWidget() == pagePolygon && radPolygonPerimeter->isChecked() )
           || ( stackedPlacement->currentWidget() == pageLine && radLineCurved->isChecked() ) )
  {
    bool curved = ( stackedPlacement->currentWidget() == pageLine && radLineCurved->isChecked() );
    lyr.placement = ( curved ? QgsPalLayerSettings::Curved : QgsPalLayerSettings::Line );
    lyr.dist = spinDistLine->value();
    lyr.distInMapUnits = ( mLineDistanceUnitComboBox->currentIndex() == 1 );
    if ( chkLineAbove->isChecked() )
      lyr.placementFlags |= QgsPalLayerSettings::AboveLine;
    if ( chkLineBelow->isChecked() )
      lyr.placementFlags |= QgsPalLayerSettings::BelowLine;
    if ( chkLineOn->isChecked() )
      lyr.placementFlags |= QgsPalLayerSettings::OnLine;

    if ( ! chkLineOrientationDependent->isChecked() )
      lyr.placementFlags |= QgsPalLayerSettings::MapOrientation;
  }
  else if (( stackedPlacement->currentWidget() == pageLine && radLineHorizontal->isChecked() )
           || ( stackedPlacement->currentWidget() == pagePolygon && radPolygonHorizontal->isChecked() ) )
  {
    lyr.placement = QgsPalLayerSettings::Horizontal;
  }
  else if ( radPolygonFree->isChecked() )
  {
    lyr.placement = QgsPalLayerSettings::Free;
  }
  else
    Q_ASSERT( 0 && "NOOO!" );


  lyr.textColor = btnTextColor->color();
  lyr.textFont = mRefFont;
  lyr.textNamedStyle = mFontStyleComboBox->currentText();
  lyr.textTransp = mFontTranspSpinBox->value();
  lyr.blendMode = QgsMapRenderer::getCompositionMode(( QgsMapRenderer::BlendMode )comboBlendMode->blendMode() );
  lyr.previewBkgrdColor = mPreviewBackgroundBtn->color();
  lyr.enabled = chkEnableLabeling->isChecked();
  lyr.priority = sliderPriority->value();
  lyr.obstacle = !chkNoObstacle->isChecked();
  lyr.labelPerPart = chkLabelPerFeaturePart->isChecked();
  lyr.displayAll = mPalShowAllLabelsForLayerChkBx->isChecked();
  lyr.mergeLines = chkMergeLines->isChecked();
  if ( chkScaleBasedVisibility->isChecked() )
  {
    lyr.scaleMin = spinScaleMin->value();
    lyr.scaleMax = spinScaleMax->value();
  }
  else
  {
    lyr.scaleMin = lyr.scaleMax = 0;
  }
  if ( chkBuffer->isChecked() )
  {
    lyr.bufferSize = spinBufferSize->value();
    lyr.bufferColor = btnBufferColor->color();
    lyr.bufferTransp = mBufferTranspSpinBox->value();
    lyr.bufferSizeInMapUnits = ( mBufferUnitComboBox->currentIndex() == 1 );
    lyr.bufferJoinStyle = mBufferJoinStyleComboBox->penJoinStyle();
    lyr.bufferNoFill = !mBufferTranspFillChbx->isChecked();
    lyr.bufferBlendMode = QgsMapRenderer::getCompositionMode(( QgsMapRenderer::BlendMode )comboBufferBlendMode->blendMode() );
  }
  else
  {
    lyr.bufferSize = 0;
  }

  // shape background
  lyr.shapeDraw = mShapeBackgroundGrpBx->isChecked();
  lyr.shapeType = ( QgsPalLayerSettings::ShapeType )mShapeTypeCmbBx->currentIndex();
  lyr.shapeSVGFile = mSvgSelector->currentSvgPath();

  lyr.shapeSizeType = ( QgsPalLayerSettings::SizeType )mShapeSizeCmbBx->currentIndex();
  lyr.shapeSize = QPointF( mShapeSizeXSpnBx->value(), mShapeSizeYSpnBx->value() );
  lyr.shapeSizeUnits = ( QgsPalLayerSettings::SizeUnit )( mShapeSizeUnitsCmbBx->currentIndex() + 1 );
  lyr.shapeRotationType = ( QgsPalLayerSettings::RotationType )( mShapeRotationCmbBx->currentIndex() );
  lyr.shapeRotation = mShapeRotationDblSpnBx->value();
  lyr.shapeOffset = QPointF( mShapeOffsetXSpnBx->value(), mShapeOffsetYSpnBx->value() );
  lyr.shapeOffsetUnits = ( QgsPalLayerSettings::SizeUnit )( mShapeOffsetUnitsCmbBx->currentIndex() + 1 );
  lyr.shapeRadii = QPointF( mShapeRadiusXDbSpnBx->value(), mShapeRadiusYDbSpnBx->value() );
  lyr.shapeRadiiUnits = ( QgsPalLayerSettings::SizeUnit )( mShapeRadiusUnitsCmbBx->currentIndex() + 1 );

  lyr.shapeFillColor = mShapeFillColorBtn->color();
  lyr.shapeBorderColor = mShapeBorderColorBtn->color();
  lyr.shapeBorderWidth = mShapeBorderWidthSpnBx->value();
  lyr.shapeBorderWidthUnits = ( QgsPalLayerSettings::SizeUnit )( mShapeBorderWidthUnitsCmbBx->currentIndex() + 1 );
  lyr.shapeJoinStyle = mShapePenStyleCmbBx->penJoinStyle();
  lyr.shapeTransparency = mShapeTranspSpinBox->value();
  lyr.shapeBlendMode = QgsMapRenderer::getCompositionMode(( QgsMapRenderer::BlendMode )mShapeBlendCmbBx->blendMode() );


  if ( chkFormattedNumbers->isChecked() )
  {
    lyr.formatNumbers = true;
    lyr.decimals = spinDecimals->value();
    lyr.plusSign = chkPlusSign->isChecked();
  }
  else
  {
    lyr.formatNumbers = false;
    lyr.decimals = spinDecimals->value();
    lyr.plusSign = true;
  }

  lyr.addDirectionSymbol = mDirectSymbGroupBox->isChecked();
  lyr.leftDirectionSymbol = mDirectSymbLeftLineEdit->text();
  lyr.rightDirectionSymbol = mDirectSymbRightLineEdit->text();
  lyr.reverseDirectionSymbol = mDirectSymbRevChkBx->isChecked();
  if ( mDirectSymbRadioBtnLR->isChecked() )
  {
    lyr.placeDirectionSymbol = QgsPalLayerSettings::SymbolLeftRight;
  }
  else if ( mDirectSymbRadioBtnAbove->isChecked() )
  {
    lyr.placeDirectionSymbol = QgsPalLayerSettings::SymbolAbove;
  }
  else if ( mDirectSymbRadioBtnBelow->isChecked() )
  {
    lyr.placeDirectionSymbol = QgsPalLayerSettings::SymbolBelow;
  }

  if ( mUpsidedownRadioOff->isChecked() )
  {
    lyr.upsidedownLabels = QgsPalLayerSettings::Upright;
  }
  else if ( mUpsidedownRadioDefined->isChecked() )
  {
    lyr.upsidedownLabels = QgsPalLayerSettings::ShowDefined;
  }
  else if ( mUpsidedownRadioAll->isChecked() )
  {
    lyr.upsidedownLabels = QgsPalLayerSettings::ShowAll;
  }
  lyr.maxCurvedCharAngleIn = mMaxCharAngleInDSpinBox->value();
  // lyr.maxCurvedCharAngleOut must be negative, but it is shown as positive spinbox in GUI
  lyr.maxCurvedCharAngleOut = -mMaxCharAngleOutDSpinBox->value();

  lyr.minFeatureSize = mMinSizeSpinBox->value();
  lyr.limitNumLabels = mLimitLabelChkBox->isChecked();
  lyr.maxNumLabels = mLimitLabelSpinBox->value();
  lyr.fontSizeInMapUnits = ( mFontSizeUnitComboBox->currentIndex() == 1 );
  lyr.fontLimitPixelSize = mFontLimitPixelGroupBox->isChecked();
  lyr.fontMinPixelSize = mFontMinPixelSpinBox->value();
  lyr.fontMaxPixelSize = mFontMaxPixelSpinBox->value();
  lyr.wrapChar = wrapCharacterEdit->text();
  lyr.multilineHeight = mFontLineHeightSpinBox->value();
  lyr.multilineAlign = ( QgsPalLayerSettings::MultiLineAlign ) mFontMultiLineComboBox->currentIndex();
  if ( chkPreserveRotation->isChecked() )
  {
    lyr.preserveRotation = true;
  }
  else
  {
    lyr.preserveRotation = false;
  }

  //data defined labeling
  setDataDefinedProperty( mSizeAttributeComboBox, QgsPalLayerSettings::Size, lyr );
  setDataDefinedProperty( mColorAttributeComboBox, QgsPalLayerSettings::Color, lyr );
  setDataDefinedProperty( mBoldAttributeComboBox, QgsPalLayerSettings::Bold, lyr );
  setDataDefinedProperty( mItalicAttributeComboBox, QgsPalLayerSettings::Italic, lyr );
  setDataDefinedProperty( mUnderlineAttributeComboBox, QgsPalLayerSettings::Underline, lyr );
  setDataDefinedProperty( mStrikeoutAttributeComboBox, QgsPalLayerSettings::Strikeout, lyr );
  setDataDefinedProperty( mFontFamilyAttributeComboBox, QgsPalLayerSettings::Family, lyr );
  setDataDefinedProperty( mBufferSizeAttributeComboBox, QgsPalLayerSettings:: BufferSize, lyr );
  setDataDefinedProperty( mBufferColorAttributeComboBox, QgsPalLayerSettings::BufferColor, lyr );
  setDataDefinedProperty( mXCoordinateComboBox, QgsPalLayerSettings::PositionX, lyr );
  setDataDefinedProperty( mYCoordinateComboBox, QgsPalLayerSettings::PositionY, lyr );
  setDataDefinedProperty( mHorizontalAlignmentComboBox, QgsPalLayerSettings::Hali, lyr );
  setDataDefinedProperty( mVerticalAlignmentComboBox, QgsPalLayerSettings::Vali, lyr );
  setDataDefinedProperty( mLabelDistanceComboBox, QgsPalLayerSettings::LabelDistance, lyr );
  setDataDefinedProperty( mRotationComboBox, QgsPalLayerSettings::Rotation, lyr );
  setDataDefinedProperty( mShowLabelAttributeComboBox, QgsPalLayerSettings::Show, lyr );
  setDataDefinedProperty( mMinScaleAttributeComboBox, QgsPalLayerSettings::MinScale, lyr );
  setDataDefinedProperty( mMaxScaleAttributeComboBox, QgsPalLayerSettings::MaxScale, lyr );
  setDataDefinedProperty( mTranspAttributeComboBox, QgsPalLayerSettings::FontTransp, lyr );
  setDataDefinedProperty( mBufferTranspAttributeComboBox, QgsPalLayerSettings::BufferTransp, lyr );
  setDataDefinedProperty( mAlwaysShowAttributeComboBox, QgsPalLayerSettings::AlwaysShow, lyr );

  return lyr;
}

void QgsLabelingGui::populateFieldNames()
{
  const QgsFields& fields = mLayer->pendingFields();
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    cboFieldName->addItem( fields[idx].name() );
  }
}

void QgsLabelingGui::setDataDefinedProperty( const QComboBox* c, QgsPalLayerSettings::DataDefinedProperties p, QgsPalLayerSettings& lyr )
{
  if ( !c )
  {
    return;
  }
  lyr.setDataDefinedProperty( p, c->currentText() ); // "" value effectively clears setting
}

void QgsLabelingGui::setCurrentComboValue( QComboBox* c, const QgsPalLayerSettings& s, QgsPalLayerSettings::DataDefinedProperties p )
{
  if ( !c )
  {
    return;
  }

  QMap< QgsPalLayerSettings::DataDefinedProperties, QString >::const_iterator it = s.dataDefinedProperties.find( p );
  if ( it == s.dataDefinedProperties.constEnd() || it.value().isEmpty() )
  {
    c->setCurrentIndex( 0 );
  }
  else
  {
    c->setCurrentIndex( c->findText( it.value() ) );
  }
}

void QgsLabelingGui::populateDataDefinedCombos( QgsPalLayerSettings& s )
{
  QList<QComboBox*> comboList;
  comboList << mSizeAttributeComboBox;
  comboList << mColorAttributeComboBox;
  comboList << mBoldAttributeComboBox;
  comboList << mItalicAttributeComboBox;
  comboList << mUnderlineAttributeComboBox;
  comboList << mStrikeoutAttributeComboBox;
  comboList << mFontFamilyAttributeComboBox;
  comboList << mBufferSizeAttributeComboBox;
  comboList << mBufferColorAttributeComboBox;
  comboList << mXCoordinateComboBox;
  comboList << mYCoordinateComboBox;
  comboList << mHorizontalAlignmentComboBox;
  comboList << mVerticalAlignmentComboBox;
  comboList << mLabelDistanceComboBox;
  comboList << mRotationComboBox;
  comboList << mShowLabelAttributeComboBox;
  comboList << mMinScaleAttributeComboBox;
  comboList << mMaxScaleAttributeComboBox;
  comboList << mTranspAttributeComboBox;
  comboList << mBufferTranspAttributeComboBox;
  comboList << mAlwaysShowAttributeComboBox;

  QList<QComboBox*>::iterator comboIt = comboList.begin();
  for ( ; comboIt != comboList.end(); ++comboIt )
  {
    ( *comboIt )->addItem( "", QVariant() );
  }

  // TODO: don't add field that aren't of appropriate type for the data defined property
  const QgsFields& fields = mLayer->dataProvider()->fields();
  for ( int idx = 0; idx < fields.count(); ++idx )
  {
    for ( comboIt = comboList.begin(); comboIt != comboList.end(); ++comboIt )
    {
      ( *comboIt )->addItem( fields[idx].name(), idx );
    }

  }

  //set current combo boxes to already existing indices
  setCurrentComboValue( mSizeAttributeComboBox, s, QgsPalLayerSettings::Size );
  setCurrentComboValue( mColorAttributeComboBox, s, QgsPalLayerSettings::Color );
  setCurrentComboValue( mBoldAttributeComboBox, s, QgsPalLayerSettings::Bold );
  setCurrentComboValue( mItalicAttributeComboBox, s, QgsPalLayerSettings::Italic );
  setCurrentComboValue( mUnderlineAttributeComboBox, s, QgsPalLayerSettings::Underline );
  setCurrentComboValue( mStrikeoutAttributeComboBox, s, QgsPalLayerSettings::Strikeout );
  setCurrentComboValue( mFontFamilyAttributeComboBox, s, QgsPalLayerSettings::Family );
  setCurrentComboValue( mBufferSizeAttributeComboBox, s , QgsPalLayerSettings::BufferSize );
  setCurrentComboValue( mBufferColorAttributeComboBox, s, QgsPalLayerSettings::BufferColor );
  setCurrentComboValue( mXCoordinateComboBox, s, QgsPalLayerSettings::PositionX );
  setCurrentComboValue( mYCoordinateComboBox, s, QgsPalLayerSettings::PositionY );
  setCurrentComboValue( mHorizontalAlignmentComboBox, s, QgsPalLayerSettings::Hali );
  setCurrentComboValue( mVerticalAlignmentComboBox, s, QgsPalLayerSettings::Vali );
  setCurrentComboValue( mLabelDistanceComboBox, s, QgsPalLayerSettings::LabelDistance );
  setCurrentComboValue( mRotationComboBox, s, QgsPalLayerSettings::Rotation );
  setCurrentComboValue( mShowLabelAttributeComboBox, s, QgsPalLayerSettings::Show );
  setCurrentComboValue( mMinScaleAttributeComboBox, s, QgsPalLayerSettings::MinScale );
  setCurrentComboValue( mMaxScaleAttributeComboBox, s, QgsPalLayerSettings::MaxScale );
  setCurrentComboValue( mTranspAttributeComboBox, s, QgsPalLayerSettings::FontTransp );
  setCurrentComboValue( mBufferTranspAttributeComboBox, s, QgsPalLayerSettings::BufferTransp );
  setCurrentComboValue( mAlwaysShowAttributeComboBox, s, QgsPalLayerSettings::AlwaysShow );
}

void QgsLabelingGui::changeTextColor( const QColor &color )
{
  Q_UNUSED( color )
  updatePreview();
}

void QgsLabelingGui::changeTextFont()
{
  // store properties of QFont that might be stripped by font dialog
  QFont::Capitalization captials = mRefFont.capitalization();
  double wordspacing = mRefFont.wordSpacing();
  double letterspacing = mRefFont.letterSpacing();

  bool ok;
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
  // Native Mac dialog works only for Qt Carbon
  QFont font = QFontDialog::getFont( &ok, mRefFont, 0, QString(), QFontDialog::DontUseNativeDialog );
#else
  QFont font = QFontDialog::getFont( &ok, mRefFont );
#endif
  if ( ok )
  {
    if ( mFontSizeUnitComboBox->currentIndex() == 1 )
    {
      // don't override map units size with selected size from font dialog
      font.setPointSizeF( mFontSizeSpinBox->value() );
    }
    else
    {
      mFontSizeSpinBox->setValue( font.pointSizeF() );
    }

    // reassign possibly stripped QFont properties
    font.setCapitalization( captials );
    font.setWordSpacing( wordspacing );
    font.setLetterSpacing( QFont::AbsoluteSpacing, letterspacing );

    updateFont( font );
  }
}

void QgsLabelingGui::updateFontViaStyle( const QString & fontstyle )
{
  QFont styledfont;
  bool foundmatch = false;
  int fontSize = 12; // QFontDatabase::font() needs an integer for size
  if ( !fontstyle.isEmpty() )
  {
    styledfont = mFontDB.font( mRefFont.family(), fontstyle, fontSize );
    styledfont.setPointSizeF( mRefFont.pointSizeF() );
    if ( QApplication::font().toString() != styledfont.toString() )
    {
      foundmatch = true;
    }
  }
  if ( !foundmatch )
  {
    foreach ( const QString &style, mFontDB.styles( mRefFont.family() ) )
    {
      styledfont = mFontDB.font( mRefFont.family(), style, fontSize );
      styledfont.setPointSizeF( mRefFont.pointSizeF() );
      styledfont = styledfont.resolve( mRefFont );
      if ( mRefFont.toString() == styledfont.toString() )
      {
        foundmatch = true;
        break;
      }
    }
  }
  if ( foundmatch )
  {
//    styledfont.setPointSizeF( mRefFont.pointSizeF() );
    styledfont.setCapitalization( mRefFont.capitalization() );
    styledfont.setUnderline( mRefFont.underline() );
    styledfont.setStrikeOut( mRefFont.strikeOut() );
    styledfont.setWordSpacing( mRefFont.wordSpacing() );
    styledfont.setLetterSpacing( QFont::AbsoluteSpacing, mRefFont.letterSpacing() );
    mRefFont = styledfont;
  }
  // if no match, style combobox will be left blank, which should not affect engine labeling
}

void QgsLabelingGui::updateFont( QFont font )
{
  // update background reference font
  if ( font != mRefFont )
  {
    mRefFont = font;
  }

  // test if font is actually available
  QString missingtxt = QString( "" );
  bool missing = false;
  if ( QApplication::font().toString() != mRefFont.toString() )
  {
    QFont testfont = mFontDB.font( mRefFont.family(), mFontDB.styleString( mRefFont ), 12 );
    if ( QApplication::font().toString() == testfont.toString() )
    {
      missing = true;
    }
  }
  if ( missing )
  {
    missingtxt = tr( " (not found!)" );
    lblFontName->setStyleSheet( "color: #990000;" );
  }
  else
  {
    lblFontName->setStyleSheet( "color: #000000;" );
  }

  lblFontName->setText( QString( "%1%2" ).arg( mRefFont.family() ).arg( missingtxt ) );
  mDirectSymbLeftLineEdit->setFont( mRefFont );
  mDirectSymbRightLineEdit->setFont( mRefFont );

  blockFontChangeSignals( true );
  populateFontStyleComboBox();
  int idx = mFontCapitalsComboBox->findData( QVariant(( unsigned int ) mRefFont.capitalization() ) );
  mFontCapitalsComboBox->setCurrentIndex( idx == -1 ? 0 : idx );
  mFontUnderlineBtn->setChecked( mRefFont.underline() );
  mFontStrikethroughBtn->setChecked( mRefFont.strikeOut() );
  blockFontChangeSignals( false );

  // update font name with font face
//  font.setPixelSize( 24 );
//  lblFontName->setFont( QFont( font ) );

  updatePreview();
}

void QgsLabelingGui::blockFontChangeSignals( bool blk )
{
  mFontStyleComboBox->blockSignals( blk );
  mFontCapitalsComboBox->blockSignals( blk );
  mFontUnderlineBtn->blockSignals( blk );
  mFontStrikethroughBtn->blockSignals( blk );
  mFontWordSpacingSpinBox->blockSignals( blk );
  mFontLetterSpacingSpinBox->blockSignals( blk );
}

void QgsLabelingGui::updatePreview()
{
  scrollPreview();
  lblFontPreview->setFont( mRefFont );
  QFont previewFont = lblFontPreview->font();
  double fontSize = mFontSizeSpinBox->value();
  double previewRatio = mPreviewSize / fontSize;
  double bufferSize = 0.0;
  QString grpboxtitle;
  QString sampleTxt = tr( "Text/Buffer sample" );

  if ( mFontSizeUnitComboBox->currentIndex() == 1 ) // map units
  {
    // TODO: maybe match current map zoom level instead?
    previewFont.setPointSize( mPreviewSize );
    mPreviewSizeSlider->setEnabled( true );
    grpboxtitle = sampleTxt + tr( " @ %1 pts (using map units)" ).arg( mPreviewSize );

    previewFont.setWordSpacing( previewRatio * mFontWordSpacingSpinBox->value() );
    previewFont.setLetterSpacing( QFont::AbsoluteSpacing, previewRatio * mFontLetterSpacingSpinBox->value() );

    if ( chkBuffer->isChecked() )
    {
      if ( mBufferUnitComboBox->currentIndex() == 1 ) // map units
      {
        bufferSize = previewRatio * spinBufferSize->value() / 3.527;
      }
      else // millimeters
      {
        grpboxtitle = sampleTxt + tr( " @ %1 pts (using map units, BUFFER IN MILLIMETERS)" ).arg( mPreviewSize );
        bufferSize = spinBufferSize->value();
      }
    }
  }
  else // in points
  {
    previewFont.setPointSize( fontSize );
    mPreviewSizeSlider->setEnabled( false );
    grpboxtitle = sampleTxt;

    if ( chkBuffer->isChecked() )
    {
      if ( mBufferUnitComboBox->currentIndex() == 0 ) // millimeters
      {
        bufferSize = spinBufferSize->value();
      }
      else // map units
      {
        grpboxtitle = sampleTxt + tr( " (BUFFER NOT SHOWN, in map units)" );
      }
    }
  }

  lblFontPreview->setFont( previewFont );
  groupBox_mPreview->setTitle( grpboxtitle );

  QColor prevColor = btnTextColor->color();
  prevColor.setAlphaF(( 100.0 - ( double )( mFontTranspSpinBox->value() ) ) / 100.0 );
  lblFontPreview->setTextColor( prevColor );

  bool bufferNoFill = false;
  if ( chkBuffer->isChecked() && bufferSize != 0.0 )
  {
    QColor buffColor = btnBufferColor->color();
    buffColor.setAlphaF(( 100.0 - ( double )( mBufferTranspSpinBox->value() ) ) / 100.0 );

    bufferNoFill = !mBufferTranspFillChbx->isChecked();
    lblFontPreview->setBuffer( bufferSize, buffColor, mBufferJoinStyleComboBox->penJoinStyle(), bufferNoFill );
  }
  else
  {
    lblFontPreview->setBuffer( 0, Qt::white, Qt::BevelJoin, bufferNoFill );
  }
}

void QgsLabelingGui::scrollPreview()
{
  scrollArea_mPreview->ensureVisible( 0, 0, 0, 0 );
}

void QgsLabelingGui::setPreviewBackground( QColor color )
{
  scrollArea_mPreview->widget()->setStyleSheet( QString( "background: rgb(%1, %2, %3);" ).arg( QString::number( color.red() ),
      QString::number( color.green() ),
      QString::number( color.blue() ) ) );
}

void QgsLabelingGui::showEngineConfigDialog()
{
  QgsLabelEngineConfigDialog dlg( mLBL, this );
  dlg.exec();
}

void QgsLabelingGui::showExpressionDialog()
{
  QgsExpressionBuilderDialog dlg( mLayer, cboFieldName->currentText() , this );
  dlg.setWindowTitle( tr( "Expression based label" ) );

  QgsDistanceArea myDa;
  myDa.setSourceCrs( mLayer->crs().srsid() );
  myDa.setEllipsoidalMode( QgisApp::instance()->mapCanvas()->mapRenderer()->hasCrsTransformEnabled() );
  myDa.setEllipsoid( QgsProject::instance()->readEntry( "Measure", "/Ellipsoid", GEO_NONE ) );
  dlg.setGeomCalculator( myDa );

  if ( dlg.exec() == QDialog::Accepted )
  {
    QString expression =  dlg.expressionText();
    //Only add the expression if the user has entered some text.
    if ( !expression.isEmpty() )
    {
      cboFieldName->addItem( expression );
      cboFieldName->setCurrentIndex( cboFieldName->count() - 1 );
    }
  }
}

void QgsLabelingGui::updateUi()
{
  // enable/disable scale-based, buffer, decimals
  bool buf = chkBuffer->isChecked();
  spinBufferSize->setEnabled( buf );
  btnBufferColor->setEnabled( buf );
  mBufferUnitComboBox->setEnabled( buf );
  mBufferTranspSlider->setEnabled( buf );
  mBufferTranspSpinBox->setEnabled( buf );

  bool scale = chkScaleBasedVisibility->isChecked();
  spinScaleMin->setEnabled( scale );
  spinScaleMax->setEnabled( scale );

  spinDecimals->setEnabled( chkFormattedNumbers->isChecked() );

  bool offline = chkLineAbove->isChecked() || chkLineBelow->isChecked();
  offlineOptions->setEnabled( offline );
}

void QgsLabelingGui::changeBufferColor( const QColor &color )
{
  Q_UNUSED( color )
  updatePreview();
}

void QgsLabelingGui::updateOptions()
{
  mCentroidFrame->setVisible( false );
  if (( stackedPlacement->currentWidget() == pagePoint && radAroundPoint->isChecked() )
      || ( stackedPlacement->currentWidget() == pagePolygon && radAroundCentroid->isChecked() ) )
  {
    stackedOptions->setCurrentWidget( pageOptionsPoint );
    mCentroidFrame->setVisible( stackedPlacement->currentWidget() == pagePolygon
                                && radAroundCentroid->isChecked() );
  }
  else if (( stackedPlacement->currentWidget() == pagePoint && radOverPoint->isChecked() )
           || ( stackedPlacement->currentWidget() == pagePolygon && radOverCentroid->isChecked() ) )
  {
    stackedOptions->setCurrentWidget( pageOptionsPointOffset );
    mCentroidFrame->setVisible( stackedPlacement->currentWidget() == pagePolygon
                                && radOverCentroid->isChecked() );
  }
  else if (( stackedPlacement->currentWidget() == pageLine && radLineParallel->isChecked() )
           || ( stackedPlacement->currentWidget() == pagePolygon && radPolygonPerimeter->isChecked() )
           || ( stackedPlacement->currentWidget() == pageLine && radLineCurved->isChecked() ) )
  {
    stackedOptions->setCurrentWidget( pageOptionsLine );
    mMaxCharAngleFrame->setVisible(( stackedPlacement->currentWidget() == pageLine
                                     && radLineCurved->isChecked() ) );
  }
  else
  {
    stackedOptions->setCurrentWidget( pageOptionsEmpty );
  }
}

void QgsLabelingGui::updateQuadrant()
{
  if ( mPointOffsetRadioAboveLeft->isChecked() ) { mXQuadOffset = -1; mYQuadOffset = 1; };
  if ( mPointOffsetRadioAbove->isChecked() ) { mXQuadOffset = 0; mYQuadOffset = 1; };
  if ( mPointOffsetRadioAboveRight->isChecked() ) { mXQuadOffset = 1; mYQuadOffset = 1; };

  if ( mPointOffsetRadioLeft->isChecked() ) { mXQuadOffset = -1; mYQuadOffset = 0; };
  if ( mPointOffsetRadioOver->isChecked() ) { mXQuadOffset = 0; mYQuadOffset = 0; };
  if ( mPointOffsetRadioRight->isChecked() ) { mXQuadOffset = 1; mYQuadOffset = 0; };

  if ( mPointOffsetRadioBelowLeft->isChecked() ) { mXQuadOffset = -1; mYQuadOffset = -1; };
  if ( mPointOffsetRadioBelow->isChecked() ) { mXQuadOffset = 0; mYQuadOffset = -1; };
  if ( mPointOffsetRadioBelowRight->isChecked() ) { mXQuadOffset = 1; mYQuadOffset = -1; };
}

void QgsLabelingGui::populateFontCapitalsComboBox()
{
  mFontCapitalsComboBox->addItem( tr( "Mixed Case" ), QVariant( 0 ) );
  mFontCapitalsComboBox->addItem( tr( "All Uppercase" ), QVariant( 1 ) );
  mFontCapitalsComboBox->addItem( tr( "All Lowercase" ), QVariant( 2 ) );
  // Small caps doesn't work right with QPainterPath::addText()
  // https://bugreports.qt-project.org/browse/QTBUG-13965
//  mFontCapitalsComboBox->addItem( tr( "Small Caps" ), QVariant( 3 ) );
  mFontCapitalsComboBox->addItem( tr( "Title Case" ), QVariant( 4 ) );
}

void QgsLabelingGui::populateFontStyleComboBox()
{
  mFontStyleComboBox->clear();
  foreach ( const QString &style, mFontDB.styles( mRefFont.family() ) )
  {
    mFontStyleComboBox->addItem( style );
  }
  mFontStyleComboBox->setCurrentIndex( mFontStyleComboBox->findText( mFontDB.styleString( mRefFont ) ) );
}

void QgsLabelingGui::on_mPreviewSizeSlider_valueChanged( int i )
{
  mPreviewSize = i;
  updatePreview();
}

void QgsLabelingGui::on_mFontSizeSpinBox_valueChanged( double d )
{
  mRefFont.setPointSizeF( d );
  updateFont( mRefFont );
}

void QgsLabelingGui::on_mFontCapitalsComboBox_currentIndexChanged( int index )
{
  int capitalsindex = mFontCapitalsComboBox->itemData( index ).toUInt();
  mRefFont.setCapitalization(( QFont::Capitalization ) capitalsindex );
  updateFont( mRefFont );
}

void QgsLabelingGui::on_mFontStyleComboBox_currentIndexChanged( const QString & text )
{
  updateFontViaStyle( text );
  updateFont( mRefFont );
}

void QgsLabelingGui::on_mFontUnderlineBtn_toggled( bool ckd )
{
  mRefFont.setUnderline( ckd );
  updateFont( mRefFont );
}

void QgsLabelingGui::on_mFontStrikethroughBtn_toggled( bool ckd )
{
  mRefFont.setStrikeOut( ckd );
  updateFont( mRefFont );
}

void QgsLabelingGui::on_mFontWordSpacingSpinBox_valueChanged( double spacing )
{
  mRefFont.setWordSpacing( spacing );
  updateFont( mRefFont );
}

void QgsLabelingGui::on_mFontLetterSpacingSpinBox_valueChanged( double spacing )
{
  mRefFont.setLetterSpacing( QFont::AbsoluteSpacing, spacing );
  updateFont( mRefFont );
}

void QgsLabelingGui::on_mFontSizeUnitComboBox_currentIndexChanged( int index )
{
  // disable pixel size limiting for labels defined in points
  if ( index == 0 )
  {
    mFontLimitPixelGroupBox->setChecked( false );
  }
  else if ( index == 1 && mMinPixelLimit == 0 )
  {
    // initial minimum trigger value set, turn on pixel size limiting by default
    // for labels defined in map units (ignored after first settings save)
    mFontLimitPixelGroupBox->setChecked( true );
  }
  updateFont( mRefFont );
}

void QgsLabelingGui::on_mFontMinPixelSpinBox_valueChanged( int px )
{
  // ensure max font pixel size for map unit labels can't be lower than min
  mFontMaxPixelSpinBox->setMinimum( px );
  mFontMaxPixelSpinBox->update();
}

void QgsLabelingGui::on_mFontMaxPixelSpinBox_valueChanged( int px )
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

void QgsLabelingGui::on_mBufferUnitComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  updateFont( mRefFont );
}

void QgsLabelingGui::on_mXCoordinateComboBox_currentIndexChanged( const QString & text )
{
  if ( text.isEmpty() ) //no data defined alignment without data defined position
  {
    disableDataDefinedAlignment();
  }
  else if ( !mYCoordinateComboBox->currentText().isEmpty() )
  {
    enableDataDefinedAlignment();
  }
}

void QgsLabelingGui::on_mYCoordinateComboBox_currentIndexChanged( const QString & text )
{
  if ( text.isEmpty() ) //no data defined alignment without data defined position
  {
    disableDataDefinedAlignment();
  }
  else if ( !mXCoordinateComboBox->currentText().isEmpty() )
  {
    enableDataDefinedAlignment();
  }
}

void QgsLabelingGui::on_mShapeTypeCmbBx_currentIndexChanged( int index )
{
  // shape background
  bool isRect = (( QgsPalLayerSettings::ShapeType )index == QgsPalLayerSettings::ShapeRectangle
                 || ( QgsPalLayerSettings::ShapeType )index == QgsPalLayerSettings::ShapeSquare );
  bool isSVG = (( QgsPalLayerSettings::ShapeType )index == QgsPalLayerSettings::ShapeSVG );

  mShapePenStyleLine->setVisible( isRect );
  mShapePenStyleLabel->setVisible( isRect );
  mShapePenStyleCmbBx->setVisible( isRect );
  mShapeRadiusLabel->setVisible( isRect );
  mShapeRadiusFrame->setVisible( isRect );

  mSvgSelector->setMinimumHeight( isSVG ? 240 : 0 );
  mSVGSelectGrpBx->setVisible( isSVG );
//  mSVGSelectGrpBx->setCollapsed( !isSVG );
  // symbology SVG renderer only supports size^2 scaling, so we only use the x size spinbox
  mShapeSizeYSpnBx->setVisible( !isSVG );

  // SVG parameter setting doesn't support color's alpha component yet
  mShapeFillColorBtn->setColorDialogOptions( isSVG ? QColorDialog::ColorDialogOptions( 0 ) : QColorDialog::ShowAlphaChannel );
  mShapeFillColorBtn->setButtonBackground();
  mShapeBorderColorBtn->setColorDialogOptions( isSVG ? QColorDialog::ColorDialogOptions( 0 ) : QColorDialog::ShowAlphaChannel );
  mShapeBorderColorBtn->setButtonBackground();

  // configure SVG parameter widgets
  mShapeSVGParamsBtn->setVisible( isSVG );
  QString svgPath = mSvgSelector->currentSvgPath();
  if ( isSVG )
  {
    mShapePenStyleLine->setVisible( true );
    updateSvgWidgets( svgPath );
  }
  else
  {
    mShapeFillColorLabel->setEnabled( true );
    mShapeFillColorBtn->setEnabled( true );
    mShapeBorderColorLabel->setEnabled( true );
    mShapeBorderColorBtn->setEnabled( true );
    mShapeBorderWidthLabel->setEnabled( true );
    mShapeBorderWidthSpnBx->setEnabled( true );
  }
  // TODO: fix overriding SVG symbol's border width units in QgsSvgCache
  // currently broken, fall back to symbol's
  mShapeBorderWidthUnitsCmbBx->setVisible( !isSVG );
  mShapeSVGUnitsLabel->setVisible( isSVG );
}

void QgsLabelingGui::updateSvgWidgets( const QString& svgPath )
{
  bool validSVG = false;
  QFileInfo finfo( svgPath );
  validSVG = finfo.exists();

  if ( !validSVG )
    validSVG = ( QUrl( svgPath ).isValid() );

  QString grpbxTitle = tr( "Select SVG symbol" );
  mSVGSelectGrpBx->setTitle( validSVG ? grpbxTitle + ": " + finfo.fileName() : grpbxTitle );

  QColor fill, outline;
  double outlineWidth = 0.0;
  bool fillParam = false, outlineParam = false, outlineWidthParam = false;
  if ( validSVG )
  {
    QgsSvgCache::instance()->containsParams( svgPath, fillParam, fill, outlineParam, outline, outlineWidthParam, outlineWidth );
  }

  mShapeSVGParamsBtn->setEnabled( validSVG && ( fillParam || outlineParam || outlineWidthParam ) );

  mShapeFillColorLabel->setEnabled( validSVG && fillParam );
  mShapeFillColorBtn->setEnabled( validSVG && fillParam );
  if ( mLoadSvgParams && validSVG && fillParam )
    mShapeFillColorBtn->setColor( fill );

  mShapeBorderColorLabel->setEnabled( validSVG && outlineParam );
  mShapeBorderColorBtn->setEnabled( validSVG && outlineParam );
  if ( mLoadSvgParams && validSVG && outlineParam )
    mShapeBorderColorBtn->setColor( outline );

  mShapeBorderWidthLabel->setEnabled( validSVG && outlineWidthParam );
  mShapeBorderWidthSpnBx->setEnabled( validSVG && outlineWidthParam );
  if ( mLoadSvgParams && validSVG && outlineWidthParam )
    mShapeBorderWidthSpnBx->setValue( outlineWidth );

  // TODO: fix overriding SVG symbol's border width units in QgsSvgCache
  // currently broken, fall back to symbol's
  //mShapeBorderWidthUnitsCmbBx->setEnabled( validSVG && outlineWidthParam );
  mShapeSVGUnitsLabel->setEnabled( validSVG && outlineWidthParam );
}

void QgsLabelingGui::on_mShapeSVGParamsBtn_clicked()
{
  QString svgPath = mSvgSelector->currentSvgPath();
  mLoadSvgParams = true;
  updateSvgWidgets( svgPath );
  mLoadSvgParams = false;
}

void QgsLabelingGui::on_mShapeRotationCmbBx_currentIndexChanged( int index )
{
  mShapeRotationDblSpnBx->setEnabled(( QgsPalLayerSettings::RotationType )index != QgsPalLayerSettings::RotationSync );
}

void QgsLabelingGui::on_mPreviewTextEdit_textChanged( const QString & text )
{
  lblFontPreview->setText( text );
  updatePreview();
}

void QgsLabelingGui::on_mPreviewTextBtn_clicked()
{
  mPreviewTextEdit->setText( QString( "Lorem Ipsum" ) );
  updatePreview();
}

void QgsLabelingGui::on_mPreviewBackgroundBtn_colorChanged( const QColor &color )
{
  setPreviewBackground( color );
}

void QgsLabelingGui::on_mDirectSymbLeftToolBtn_clicked()
{
  bool gotChar = false;
  QChar dirSymb = QChar();

  dirSymb = mCharDlg->selectCharacter( &gotChar, mRefFont, mFontDB.styleString( mRefFont ) );

  if ( !gotChar )
    return;

  if ( !dirSymb.isNull() )
    mDirectSymbLeftLineEdit->setText( QString( dirSymb ) );
}

void QgsLabelingGui::on_mDirectSymbRightToolBtn_clicked()
{
  bool gotChar = false;
  QChar dirSymb = QChar();

  dirSymb = mCharDlg->selectCharacter( &gotChar, mRefFont, mFontDB.styleString( mRefFont ) );

  if ( !gotChar )
    return;

  if ( !dirSymb.isNull() )
    mDirectSymbRightLineEdit->setText( QString( dirSymb ) );
}

void QgsLabelingGui::disableDataDefinedAlignment()
{
  mHorizontalAlignmentComboBox->setCurrentIndex( mHorizontalAlignmentComboBox->findText( "" ) );
  mHorizontalAlignmentComboBox->setEnabled( false );
  mVerticalAlignmentComboBox->setCurrentIndex( mVerticalAlignmentComboBox->findText( "" ) );
  mVerticalAlignmentComboBox->setEnabled( false );
}

void QgsLabelingGui::enableDataDefinedAlignment()
{
  mHorizontalAlignmentComboBox->setEnabled( true );
  mVerticalAlignmentComboBox->setEnabled( true );
}
