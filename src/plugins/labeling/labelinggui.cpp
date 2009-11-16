/***************************************************************************
  labelinggui.cpp
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

#include "labelinggui.h"

#include <qgsvectorlayer.h>
#include <qgsvectordataprovider.h>
#include <qgsmaplayerregistry.h>

#include "pallabeling.h"
#include "engineconfigdialog.h"

#include <QColorDialog>
#include <QFontDialog>

#include <iostream>
#include <QApplication>



LabelingGui::LabelingGui( PalLabeling* lbl, QgsVectorLayer* layer, QWidget* parent )
    : QDialog( parent ), mLBL( lbl ), mLayer( layer )
{
  setupUi( this );

  connect( btnTextColor, SIGNAL( clicked() ), this, SLOT( changeTextColor() ) );
  connect( btnChangeFont, SIGNAL( clicked() ), this, SLOT( changeTextFont() ) );
  connect( chkBuffer, SIGNAL( toggled( bool ) ), this, SLOT( updatePreview() ) );
  connect( btnBufferColor, SIGNAL( clicked() ), this, SLOT( changeBufferColor() ) );
  connect( spinBufferSize, SIGNAL( valueChanged( int ) ), this, SLOT( updatePreview() ) );
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

  populateFieldNames();

  // load labeling settings from layer
  LayerSettings lyr;
  lyr.readFromLayer( layer );

  // placement
  switch ( lyr.placement )
  {
    case LayerSettings::AroundPoint:
      radAroundPoint->setChecked( true );
      radAroundCentroid->setChecked( true );
      spinDistPoint->setValue( lyr.dist );
      //spinAngle->setValue(lyr.angle);
      break;
    case LayerSettings::OverPoint:
      radOverPoint->setChecked( true );
      radOverCentroid->setChecked( true );
      break;
    case LayerSettings::Line:
      radLineParallel->setChecked( true );
      radPolygonPerimeter->setChecked( true );
      break;
    case LayerSettings::Curved:
      radLineCurved->setChecked( true );
      break;
    case LayerSettings::Horizontal:
      radPolygonHorizontal->setChecked( true );
      radLineHorizontal->setChecked( true );
      break;
    case LayerSettings::Free:
      radPolygonFree->setChecked( true );
      break;
    default:
      Q_ASSERT( 0 && "NOOO!" );
  }

  if ( lyr.placement == LayerSettings::Line || lyr.placement == LayerSettings::Curved )
  {
    spinDistLine->setValue( lyr.dist );
    chkLineAbove->setChecked( lyr.placementFlags & LayerSettings::AboveLine );
    chkLineBelow->setChecked( lyr.placementFlags & LayerSettings::BelowLine );
    chkLineOn->setChecked( lyr.placementFlags & LayerSettings::OnLine );
    if ( lyr.placementFlags & LayerSettings::MapOrientation )
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
  for ( int i = 0; i < sizeof( placementRadios ) / sizeof( QRadioButton* ); i++ )
    connect( placementRadios[i], SIGNAL( toggled( bool ) ), this, SLOT( updateOptions() ) );
}

LabelingGui::~LabelingGui()
{
}

LayerSettings LabelingGui::layerSettings()
{
  LayerSettings lyr;
  lyr.fieldName = cboFieldName->currentText();

  lyr.dist = 0;
  lyr.placementFlags = 0;

  if (( stackedPlacement->currentWidget() == pagePoint && radAroundPoint->isChecked() )
      || ( stackedPlacement->currentWidget() == pagePolygon && radAroundCentroid->isChecked() ) )
  {
    lyr.placement = LayerSettings::AroundPoint;
    lyr.dist = spinDistPoint->value();
    //lyr.angle = spinAngle->value();
  }
  else if (( stackedPlacement->currentWidget() == pagePoint && radOverPoint->isChecked() )
           || ( stackedPlacement->currentWidget() == pagePolygon && radOverCentroid->isChecked() ) )
  {
    lyr.placement = LayerSettings::OverPoint;
  }
  else if (( stackedPlacement->currentWidget() == pageLine && radLineParallel->isChecked() )
           || ( stackedPlacement->currentWidget() == pagePolygon && radPolygonPerimeter->isChecked() )
           || ( stackedPlacement->currentWidget() == pageLine && radLineCurved->isChecked() ) )
  {
    bool curved = ( stackedPlacement->currentWidget() == pageLine && radLineCurved->isChecked() );
    lyr.placement = ( curved ? LayerSettings::Curved : LayerSettings::Line );
    lyr.dist = spinDistLine->value();
    if ( chkLineAbove->isChecked() )
      lyr.placementFlags |= LayerSettings::AboveLine;
    if ( chkLineBelow->isChecked() )
      lyr.placementFlags |= LayerSettings::BelowLine;
    if ( chkLineOn->isChecked() )
      lyr.placementFlags |= LayerSettings::OnLine;

    if ( radOrientationMap->isChecked() )
      lyr.placementFlags |= LayerSettings::MapOrientation;
  }
  else if (( stackedPlacement->currentWidget() == pageLine && radLineHorizontal->isChecked() )
           || ( stackedPlacement->currentWidget() == pagePolygon && radPolygonHorizontal->isChecked() ) )
  {
    lyr.placement = LayerSettings::Horizontal;
  }
  else if ( radPolygonFree->isChecked() )
  {
    lyr.placement = LayerSettings::Free;
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

  return lyr;
}


void LabelingGui::populateFieldNames()
{
  QgsFieldMap fields = mLayer->dataProvider()->fields();
  for ( QgsFieldMap::iterator it = fields.begin(); it != fields.end(); it++ )
  {
    cboFieldName->addItem( it->name() );
  }
}

void LabelingGui::changeTextColor()
{
  QColor color = QColorDialog::getColor( btnTextColor->color(), this );
  if ( !color.isValid() )
    return;

  btnTextColor->setColor( color );
  updatePreview();
}

void LabelingGui::changeTextFont()
{
  bool ok;
  QFont font = QFontDialog::getFont( &ok, lblFontPreview->font(), this );
  if ( ok )
    updateFont( font );
}

void LabelingGui::updateFont( QFont font )
{
  lblFontName->setText( QString( "%1, %2" ).arg( font.family() ).arg( font.pointSize() ) );
  lblFontPreview->setFont( font );

  updatePreview();
}

void LabelingGui::updatePreview()
{
  lblFontPreview->setTextColor( btnTextColor->color() );
  if ( chkBuffer->isChecked() )
    lblFontPreview->setBuffer( spinBufferSize->value(), btnBufferColor->color() );
  else
    lblFontPreview->setBuffer( 0, Qt::white );
}

void LabelingGui::showEngineConfigDialog()
{
  EngineConfigDialog dlg( mLBL, this );
  dlg.exec();
}

void LabelingGui::updateUi()
{
  // enable/disable scale-based, buffer
  bool buf = chkBuffer->isChecked();
  spinBufferSize->setEnabled( buf );
  btnBufferColor->setEnabled( buf );

  bool scale = chkScaleBasedVisibility->isChecked();
  spinScaleMin->setEnabled( scale );
  spinScaleMax->setEnabled( scale );
}

void LabelingGui::changeBufferColor()
{
  QColor color = QColorDialog::getColor( btnBufferColor->color(), this );
  if ( !color.isValid() )
    return;

  btnBufferColor->setColor( color );
  updatePreview();
}

void LabelingGui::updateOptions()
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
