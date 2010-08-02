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

#include <qgsvectorlayer.h>
#include <qgsvectordataprovider.h>
#include <qgsmaplayerregistry.h>

#include "qgspallabeling.h"
#include "qgslabelengineconfigdialog.h"

#include <QColorDialog>
#include <QFontDialog>

#include <iostream>
#include <QApplication>



QgsLabelingGui::QgsLabelingGui( QgsPalLabeling* lbl, QgsVectorLayer* layer, QWidget* parent )
    : QDialog( parent ), mLBL( lbl ), mLayer( layer )
{
  setupUi( this );

  connect( btnTextColor, SIGNAL( clicked() ), this, SLOT( changeTextColor() ) );
  connect( btnChangeFont, SIGNAL( clicked() ), this, SLOT( changeTextFont() ) );
  connect( chkBuffer, SIGNAL( toggled( bool ) ), this, SLOT( updatePreview() ) );
  connect( btnBufferColor, SIGNAL( clicked() ), this, SLOT( changeBufferColor() ) );
  connect( spinBufferSize, SIGNAL( valueChanged( double ) ), this, SLOT( updatePreview() ) );
  connect( btnEngineSettings, SIGNAL( clicked() ), this, SLOT( showEngineConfigDialog() ) );

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

  chkMergeLines->setEnabled( layer->geometryType() == QGis::Line );
  label_19->setEnabled( layer->geometryType() != QGis::Point );
  mMinSizeSpinBox->setEnabled( layer->geometryType() != QGis::Point );

  populateFieldNames();

  // load labeling settings from layer
  QgsPalLayerSettings lyr;
  lyr.readFromLayer( layer );

  populateDataDefinedCombos( lyr );

  // placement
  switch ( lyr.placement )
  {
    case QgsPalLayerSettings::AroundPoint:
      radAroundPoint->setChecked( true );
      radAroundCentroid->setChecked( true );
      spinDistPoint->setValue( lyr.dist );
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
    chkLineAbove->setChecked( lyr.placementFlags & QgsPalLayerSettings::AboveLine );
    chkLineBelow->setChecked( lyr.placementFlags & QgsPalLayerSettings::BelowLine );
    chkLineOn->setChecked( lyr.placementFlags & QgsPalLayerSettings::OnLine );
    if ( lyr.placementFlags & QgsPalLayerSettings::MapOrientation )
      radOrientationMap->setChecked( true );
    else
      radOrientationLine->setChecked( true );
  }

  cboFieldName->setCurrentIndex( cboFieldName->findText( lyr.fieldName ) );
  chkEnableLabeling->setChecked( lyr.enabled );
  sliderPriority->setValue( lyr.priority );
  chkNoObstacle->setChecked( !lyr.obstacle );
  chkLabelPerFeaturePart->setChecked( lyr.labelPerPart );
  chkMergeLines->setChecked( lyr.mergeLines );
  chkMultiLine->setChecked( lyr.multiLineLabels );
  mMinSizeSpinBox->setValue( lyr.minFeatureSize );

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
  updateFont( lyr.textFont );
  updateUi();

  updateOptions();

  connect( chkBuffer, SIGNAL( toggled( bool ) ), this, SLOT( updateUi() ) );
  connect( chkScaleBasedVisibility, SIGNAL( toggled( bool ) ), this, SLOT( updateUi() ) );

  // setup connection to changes in the placement
  QRadioButton* placementRadios[] =
  {
    radAroundPoint, radOverPoint, // point
    radLineParallel, radLineCurved, radLineHorizontal, // line
    radAroundCentroid, radPolygonHorizontal, radPolygonFree, radPolygonPerimeter // polygon
  };
  for ( unsigned int i = 0; i < sizeof( placementRadios ) / sizeof( QRadioButton* ); i++ )
    connect( placementRadios[i], SIGNAL( toggled( bool ) ), this, SLOT( updateOptions() ) );
}

QgsLabelingGui::~QgsLabelingGui()
{
}

QgsPalLayerSettings QgsLabelingGui::layerSettings()
{
  QgsPalLayerSettings lyr;
  lyr.fieldName = cboFieldName->currentText();

  lyr.dist = 0;
  lyr.placementFlags = 0;

  if (( stackedPlacement->currentWidget() == pagePoint && radAroundPoint->isChecked() )
      || ( stackedPlacement->currentWidget() == pagePolygon && radAroundCentroid->isChecked() ) )
  {
    lyr.placement = QgsPalLayerSettings::AroundPoint;
    lyr.dist = spinDistPoint->value();
    //lyr.angle = spinAngle->value();
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
    if ( chkLineAbove->isChecked() )
      lyr.placementFlags |= QgsPalLayerSettings::AboveLine;
    if ( chkLineBelow->isChecked() )
      lyr.placementFlags |= QgsPalLayerSettings::BelowLine;
    if ( chkLineOn->isChecked() )
      lyr.placementFlags |= QgsPalLayerSettings::OnLine;

    if ( radOrientationMap->isChecked() )
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
  lyr.multiLineLabels = chkMultiLine->isChecked();
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
  lyr.minFeatureSize = mMinSizeSpinBox->value();

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

  return lyr;
}


void QgsLabelingGui::populateFieldNames()
{
  QgsFieldMap fields = mLayer->dataProvider()->fields();
  for ( QgsFieldMap::iterator it = fields.begin(); it != fields.end(); it++ )
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
  QFont font = QFontDialog::getFont( &ok, lblFontPreview->font(), this );
  if ( ok )
    updateFont( font );
}

void QgsLabelingGui::updateFont( QFont font )
{
  lblFontName->setText( QString( "%1, %2 %3" ).arg( font.family() ).arg( font.pointSize() ).arg( tr( "pt" ) ) );
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

void QgsLabelingGui::updateUi()
{
  // enable/disable scale-based, buffer
  bool buf = chkBuffer->isChecked();
  spinBufferSize->setEnabled( buf );
  btnBufferColor->setEnabled( buf );

  bool scale = chkScaleBasedVisibility->isChecked();
  spinScaleMin->setEnabled( scale );
  spinScaleMax->setEnabled( scale );
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
