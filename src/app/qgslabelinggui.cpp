/***************************************************************************
  qgslabelinggui.cpp
  Smart labeling for vector layers
  -------------------
         begin                : June 2009
         copyright            : (C) Martin Dobias
         email                : wonder.sk at gmail.com

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
#include "qgsmapcanvas.h"

#include <QColorDialog>
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

  mRefFont = lblFontPreview->font();
  mPreviewSize = 24;

  connect( btnTextColor, SIGNAL( clicked() ), this, SLOT( changeTextColor() ) );
  connect( btnChangeFont, SIGNAL( clicked() ), this, SLOT( changeTextFont() ) );
  connect( mFontSizeUnitComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updatePreview() ) );
  connect( mFontTranspSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( updatePreview() ) );
  connect( chkBuffer, SIGNAL( toggled( bool ) ), this, SLOT( updatePreview() ) );
  connect( btnBufferColor, SIGNAL( clicked() ), this, SLOT( changeBufferColor() ) );
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
    default:
      Q_ASSERT( 0 && "NOOOO!" );
  }

  //mTabWidget->setEnabled( chkEnableLabeling->isChecked() );
  chkMergeLines->setEnabled( layer->geometryType() == QGis::Line );
  chkAddDirectionSymbol->setEnabled( layer->geometryType() == QGis::Line );
  label_19->setEnabled( layer->geometryType() != QGis::Point );
  mMinSizeSpinBox->setEnabled( layer->geometryType() != QGis::Point );

  // load labeling settings from layer
  QgsPalLayerSettings lyr;
  lyr.readFromLayer( layer );
  populateFieldNames();
  populateDataDefinedCombos( lyr );

  chkEnableLabeling->setChecked( lyr.enabled );
  mTabWidget->setEnabled( lyr.enabled );
  cboFieldName->setEnabled( lyr.enabled );
  btnExpression->setEnabled( lyr.enabled );

  //Add the current expression to the bottom of the list.
  if ( lyr.isExpression && !lyr.fieldName.isEmpty() )
    cboFieldName->addItem( lyr.fieldName );

  // placement
  int distUnitIndex = lyr.distInMapUnits ? 1 : 0;
  switch ( lyr.placement )
  {
    case QgsPalLayerSettings::AroundPoint:
      radAroundPoint->setChecked( true );
      radAroundCentroid->setChecked( true );
      spinDistPoint->setValue( lyr.dist );
      mPointDistanceUnitComboBox->setCurrentIndex( distUnitIndex );
      //spinAngle->setValue(lyr.angle);
      break;
    case QgsPalLayerSettings::OverPoint:
      radOverPoint->setChecked( true );
      radOverCentroid->setChecked( true );
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
  chkMergeLines->setChecked( lyr.mergeLines );
  mMinSizeSpinBox->setValue( lyr.minFeatureSize );
  chkAddDirectionSymbol->setChecked( lyr.addDirectionSymbol );
  wrapCharacterEdit->setText( lyr.wrapChar );
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

  if ( lyr.fontSizeInMapUnits )
  {
    mFontSizeUnitComboBox->setCurrentIndex( 1 );
  }
  else
  {
    mFontSizeUnitComboBox->setCurrentIndex( 0 );
  }

  mRefFont = lyr.textFont;
  mFontSizeSpinBox->setValue( mRefFont.pointSizeF() );
  btnTextColor->setColor( lyr.textColor );
  mFontTranspSpinBox->setValue( lyr.textTransp );

  updateFontViaStyle( lyr.textNamedStyle );
  updateFont( mRefFont );

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
}

QgsLabelingGui::~QgsLabelingGui()
{
}

void QgsLabelingGui::apply()
{
  QgsPalLayerSettings settings = layerSettings();
  settings.writeToLayer( mLayer );
  // trigger refresh
  if ( mMapCanvas )
  {
    mMapCanvas->refresh();
  }
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
  lyr.previewBkgrdColor = mPreviewBackgroundBtn->color();
  lyr.enabled = chkEnableLabeling->isChecked();
  lyr.priority = sliderPriority->value();
  lyr.obstacle = !chkNoObstacle->isChecked();
  lyr.labelPerPart = chkLabelPerFeaturePart->isChecked();
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
  }
  else
  {
    lyr.bufferSize = 0;
  }
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
  if ( chkAddDirectionSymbol->isChecked() )
  {
    lyr.addDirectionSymbol = true;
  }
  else
  {
    lyr.addDirectionSymbol = false;
  }
  lyr.minFeatureSize = mMinSizeSpinBox->value();
  lyr.fontSizeInMapUnits = ( mFontSizeUnitComboBox->currentIndex() == 1 );
  lyr.wrapChar = wrapCharacterEdit->text();
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

  return lyr;
}

void QgsLabelingGui::populateFieldNames()
{
  const QgsFieldMap& fields = mLayer->pendingFields();
  QgsFieldMap::const_iterator it = fields.constBegin();
  for ( ; it != fields.constEnd(); it++ )
  {
    cboFieldName->addItem( it->name() );
  }
}

void QgsLabelingGui::setDataDefinedProperty( const QComboBox* c, QgsPalLayerSettings::DataDefinedProperties p, QgsPalLayerSettings& lyr )
{
  if ( !c )
  {
    return;
  }

  QVariant propertyField = c->itemData( c->currentIndex() );
  if ( propertyField.isValid() )
  {
    lyr.setDataDefinedProperty( p, propertyField.toInt() );
  }
}

void QgsLabelingGui::setCurrentComboValue( QComboBox* c, const QgsPalLayerSettings& s, QgsPalLayerSettings::DataDefinedProperties p )
{
  if ( !c )
  {
    return;
  }

  QMap< QgsPalLayerSettings::DataDefinedProperties, int >::const_iterator it = s.dataDefinedProperties.find( p );
  if ( it == s.dataDefinedProperties.constEnd() )
  {
    c->setCurrentIndex( 0 );
  }
  else
  {
    c->setCurrentIndex( c->findData( it.value() ) );
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

  QList<QComboBox*>::iterator comboIt = comboList.begin();
  for ( ; comboIt != comboList.end(); ++comboIt )
  {
    ( *comboIt )->addItem( "", QVariant() );
  }

  // TODO: don't add field that aren't of appropriate type for the data defined property
  const QgsFieldMap& fields = mLayer->dataProvider()->fields();
  for ( QgsFieldMap::const_iterator it = fields.constBegin(); it != fields.constEnd(); it++ )
  {
    for ( comboIt = comboList.begin(); comboIt != comboList.end(); ++comboIt )
    {
      ( *comboIt )->addItem( it.value().name(), it.key() );
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
}

void QgsLabelingGui::changeTextColor()
{
  QColor color = QColorDialog::getColor( btnTextColor->color(), this );
  if ( !color.isValid() )
    return;

  btnTextColor->setColor( color );
  updatePreview();
}

void QgsLabelingGui::changeTextFont()
{
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
    updateFont( font );
  }
}

void QgsLabelingGui::updateFontViaStyle( const QString & fontstyle )
{
  QFont styledfont;
  bool foundmatch = false;
  if ( !fontstyle.isEmpty() )
  {
    styledfont = mFontDB.font( mRefFont.family(), fontstyle, mRefFont.pointSizeF() );
    if ( QApplication::font().toString() != styledfont.toString() )
    {
      foundmatch = true;
    }
  }
  if ( !foundmatch )
  {
    foreach ( const QString &style, mFontDB.styles( mRefFont.family() ) )
    {
      styledfont = mFontDB.font( mRefFont.family(), style, mRefFont.pointSizeF() );
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
    styledfont.setUnderline( mRefFont.underline() );
    styledfont.setStrikeOut( mRefFont.strikeOut() );
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
    QFont testfont = mFontDB.font( mRefFont.family(), mFontDB.styleString( mRefFont ), mRefFont.pointSizeF() );
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

  blockFontChangeSignals( true );
  populateFontStyleComboBox();
  mFontUnderlineBtn->setChecked( mRefFont.underline() );
  mFontStrikethroughBtn->setChecked( mRefFont.strikeOut() );
  blockFontChangeSignals( false );

  // update font name with font face
//  font.setPixelSize( 18 );
//  lblFontName->setFont( QFont( font ) );

  updatePreview();
}

void QgsLabelingGui::blockFontChangeSignals( bool blk )
{
  mFontStyleComboBox->blockSignals( blk );
  mFontUnderlineBtn->blockSignals( blk );
  mFontStrikethroughBtn->blockSignals( blk );
}

void QgsLabelingGui::updatePreview()
{
  scrollPreview();
  lblFontPreview->setFont( mRefFont );
  QFont previewFont = lblFontPreview->font();
  double fontSize = mFontSizeSpinBox->value();
  double bufferSize = 0.0;
  QString grpboxtitle;

  if ( mFontSizeUnitComboBox->currentIndex() == 1 ) // map units
  {
    // TODO: maybe match current map zoom level instead?
    previewFont.setPointSize( mPreviewSize );
    mPreviewSizeSlider->setEnabled( true );
    grpboxtitle = tr( "Sample @ %1 pts (using map units)" ).arg( mPreviewSize );

    if ( chkBuffer->isChecked() )
    {
      if ( mBufferUnitComboBox->currentIndex() == 1 ) // map units
      {
        bufferSize = ( mPreviewSize / fontSize ) * spinBufferSize->value() / 2.5;
      }
      else // millimeters
      {
        grpboxtitle = tr( "Sample @ %1 pts (using map units, BUFFER IN MILLIMETERS)" ).arg( mPreviewSize );
        bufferSize = spinBufferSize->value();
      }
    }
  }
  else // in points
  {
    previewFont.setPointSize( fontSize );
    mPreviewSizeSlider->setEnabled( false );
    grpboxtitle = tr( "Sample" );

    if ( chkBuffer->isChecked() )
    {
      if ( mBufferUnitComboBox->currentIndex() == 0 ) // millimeters
      {
        bufferSize = spinBufferSize->value();
      }
      else // map units
      {
        grpboxtitle = tr( "Sample (BUFFER NOT SHOWN, in map units)" );
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

void QgsLabelingGui::changeBufferColor()
{
  QColor color = QColorDialog::getColor( btnBufferColor->color(), this );
  if ( !color.isValid() )
    return;

  btnBufferColor->setColor( color );
  updatePreview();
}

void QgsLabelingGui::updateOptions()
{
  if (( stackedPlacement->currentWidget() == pagePoint && radAroundPoint->isChecked() )
      || ( stackedPlacement->currentWidget() == pagePolygon && radAroundCentroid->isChecked() ) )
  {
    stackedOptions->setCurrentWidget( pageOptionsPoint );
  }
  else if (( stackedPlacement->currentWidget() == pageLine && radLineParallel->isChecked() )
           || ( stackedPlacement->currentWidget() == pagePolygon && radPolygonPerimeter->isChecked() )
           || ( stackedPlacement->currentWidget() == pageLine && radLineCurved->isChecked() ) )
  {
    stackedOptions->setCurrentWidget( pageOptionsLine );
  }
  else
  {
    stackedOptions->setCurrentWidget( pageOptionsEmpty );
  }
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

void QgsLabelingGui::on_mBufferUnitComboBox_currentIndexChanged( int index )
{
  double singleStep = ( index == 1 ) ? 1.0 : 0.1 ; //1.0 for map units, 0.1 for mm
  spinBufferSize->setSingleStep( singleStep );
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

void QgsLabelingGui::on_mPreviewBackgroundBtn_clicked()
{
  QColor color = QColorDialog::getColor( mPreviewBackgroundBtn->color(), this );
  if ( !color.isValid() )
    return;

  mPreviewBackgroundBtn->setColor( color );
  setPreviewBackground( color );
}

void QgsLabelingGui::disableDataDefinedAlignment()
{
  mHorizontalAlignmentComboBox->setCurrentIndex( mHorizontalAlignmentComboBox->findText( "" ) );
  mHorizontalAlignmentComboBox->setEnabled( false );
  mVerticalAlignmentComboBox->setCurrentIndex( mVerticalAlignmentComboBox->findText( "" ) );
  mVerticalAlignmentComboBox->setEnabled( false );
  mRotationComboBox->setCurrentIndex( mRotationComboBox->findText( "" ) );
  mRotationComboBox->setEnabled( false );
  chkPreserveRotation->setEnabled( false );
}

void QgsLabelingGui::enableDataDefinedAlignment()
{
  mHorizontalAlignmentComboBox->setEnabled( true );
  mVerticalAlignmentComboBox->setEnabled( true );
  mRotationComboBox->setEnabled( true );
  chkPreserveRotation->setEnabled( true );
}
