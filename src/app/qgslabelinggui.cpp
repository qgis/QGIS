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

#include "qgspallabeling.h"
#include "qgslabelengineconfigdialog.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsexpression.h"

#include <QColorDialog>
#include <QFontDialog>
#include <QTextEdit>
#include <QApplication>
#include <QMessageBox>

QgsLabelingGui::QgsLabelingGui( QgsPalLabeling* lbl, QgsVectorLayer* layer, QgsMapCanvas* mapCanvas, QWidget* parent )
    : QDialog( parent ), mLBL( lbl ), mLayer( layer ), mMapCanvas( mapCanvas )
{
  if ( !layer ) return;

  setupUi( this );

  connect( btnTextColor, SIGNAL( clicked() ), this, SLOT( changeTextColor() ) );
  connect( btnChangeFont, SIGNAL( clicked() ), this, SLOT( changeTextFont() ) );
  connect( chkBuffer, SIGNAL( toggled( bool ) ), this, SLOT( updatePreview() ) );
  connect( btnBufferColor, SIGNAL( clicked() ), this, SLOT( changeBufferColor() ) );
  connect( spinBufferSize, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
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
    spinBufferSize->setValue( lyr.bufferSize );

  btnTextColor->setColor( lyr.textColor );
  btnBufferColor->setColor( lyr.bufferColor );

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

  QFont textFont = lyr.textFont;
  updateFont( textFont );
  mFontSizeSpinBox->setValue( textFont.pointSizeF() );
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
  connect( buttonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );
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
  lyr.textFont = lblFontPreview->font();
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

  QList<QComboBox*>::iterator comboIt = comboList.begin();
  for ( ; comboIt != comboList.end(); ++comboIt )
  {
    ( *comboIt )->addItem( "", QVariant() );
  }

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
  QFont font = QFontDialog::getFont( &ok, lblFontPreview->font(), this, QString(), QFontDialog::DontUseNativeDialog );
#else
  QFont font = QFontDialog::getFont( &ok, lblFontPreview->font(), this );
#endif
  if ( ok )
  {
    updateFont( font );
  }
  mFontSizeSpinBox->setValue( font.pointSizeF() );
}

void QgsLabelingGui::updateFont( QFont font )
{
  QString fontSizeUnitString = tr( "pt" );
  if ( mFontSizeUnitComboBox->currentIndex() == 1 )
  {
    fontSizeUnitString = tr( "map units" );
  }
  lblFontName->setText( QString( "%1, %2 %3" ).arg( font.family() ).arg( font.pointSizeF() ).arg( fontSizeUnitString ) );
  lblFontPreview->setFont( font );
  updatePreview();
}

void QgsLabelingGui::updatePreview()
{
  lblFontPreview->setTextColor( btnTextColor->color() );
  if ( chkBuffer->isChecked() )
    lblFontPreview->setBuffer( spinBufferSize->value(), btnBufferColor->color() );
  else
    lblFontPreview->setBuffer( 0, Qt::white );
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
    QString expression =  dlg.getExpressionText();
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

void QgsLabelingGui::on_mFontSizeSpinBox_valueChanged( double d )
{
  QFont font = lblFontPreview->font();
  font.setPointSizeF( d );
  lblFontPreview->setFont( font );
  updateFont( font );
}

void QgsLabelingGui::on_mFontSizeUnitComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  updateFont( lblFontPreview->font() );
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

void QgsLabelingGui::disableDataDefinedAlignment()
{
  mHorizontalAlignmentComboBox->setCurrentIndex( mHorizontalAlignmentComboBox->findText( "" ) );
  mHorizontalAlignmentComboBox->setEnabled( false );
  mVerticalAlignmentComboBox->setCurrentIndex( mVerticalAlignmentComboBox->findText( "" ) );
  mVerticalAlignmentComboBox->setEnabled( false );
  mRotationComboBox->setCurrentIndex( mRotationComboBox->findText( "" ) );
  mRotationComboBox->setEnabled( false );
}

void QgsLabelingGui::enableDataDefinedAlignment()
{
  mHorizontalAlignmentComboBox->setEnabled( true );
  mVerticalAlignmentComboBox->setEnabled( true );
  mRotationComboBox->setEnabled( true );
}
