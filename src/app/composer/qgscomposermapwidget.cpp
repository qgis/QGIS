/***************************************************************************
                         qgscomposermapwidget.cpp
                         ------------------------
    begin                : May 26, 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgisapp.h"
#include "qgsmapcanvas.h"
#include "qgscomposermapwidget.h"
#include "qgscomposeritemwidget.h"
#include "qgscomposition.h"
#include "qgsmaprenderer.h"
#include "qgsstylev2.h"
#include "qgssymbolv2.h"
//#include "qgssymbolv2propertiesdialog.h"
#include "qgssymbolv2selectordialog.h"
#include "qgssymbollayerv2utils.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsmaplayerregistry.h"
#include "qgscomposershape.h"
#include "qgspaperitem.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsproject.h"
#include <QColorDialog>
#include <QFontDialog>
#include <QMessageBox>

QgsComposerMapWidget::QgsComposerMapWidget( QgsComposerMap* composerMap ): QWidget(), mComposerMap( composerMap )
{
  setupUi( this );

  //add widget for general composer item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, composerMap );
  mainLayout->addWidget( itemPropertiesWidget );

  mScaleLineEdit->setValidator( new QDoubleValidator( mScaleLineEdit ) );

  mXMinLineEdit->setValidator( new QDoubleValidator( mXMinLineEdit ) );
  mXMaxLineEdit->setValidator( new QDoubleValidator( mXMaxLineEdit ) );
  mYMinLineEdit->setValidator( new QDoubleValidator( mYMinLineEdit ) );
  mYMaxLineEdit->setValidator( new QDoubleValidator( mYMaxLineEdit ) );

  blockAllSignals( true );
  mPreviewModeComboBox->insertItem( 0, tr( "Cache" ) );
  mPreviewModeComboBox->insertItem( 1, tr( "Render" ) );
  mPreviewModeComboBox->insertItem( 2, tr( "Rectangle" ) );

  mGridTypeComboBox->insertItem( 0, tr( "Solid" ) );
  mGridTypeComboBox->insertItem( 1, tr( "Cross" ) );

  mAnnotationFormatComboBox->insertItem( 0, tr( "Decimal" ) );
  mAnnotationFormatComboBox->insertItem( 1, tr( "DegreeMinute" ) );
  mAnnotationFormatComboBox->insertItem( 2, tr( "DegreeMinuteSecond" ) );

  mAnnotationFontColorButton->setColorDialogTitle( tr( "Select font color" ) );
  mAnnotationFontColorButton->setColorDialogOptions( QColorDialog::ShowAlphaChannel );

  insertAnnotationPositionEntries( mAnnotationPositionLeftComboBox );
  insertAnnotationPositionEntries( mAnnotationPositionRightComboBox );
  insertAnnotationPositionEntries( mAnnotationPositionTopComboBox );
  insertAnnotationPositionEntries( mAnnotationPositionBottomComboBox );

  insertAnnotationDirectionEntries( mAnnotationDirectionComboBoxLeft );
  insertAnnotationDirectionEntries( mAnnotationDirectionComboBoxRight );
  insertAnnotationDirectionEntries( mAnnotationDirectionComboBoxTop );
  insertAnnotationDirectionEntries( mAnnotationDirectionComboBoxBottom );

  mFrameStyleComboBox->insertItem( 0, tr( "No frame" ) );
  mFrameStyleComboBox->insertItem( 1, tr( "Zebra" ) );

  mGridFramePenColorButton->setColorDialogTitle( tr( "Select grid frame color" ) );
  mGridFramePenColorButton->setColorDialogOptions( QColorDialog::ShowAlphaChannel );
  mGridFrameFill1ColorButton->setColorDialogTitle( tr( "Select grid frame fill color" ) );
  mGridFrameFill1ColorButton->setColorDialogOptions( QColorDialog::ShowAlphaChannel );
  mGridFrameFill2ColorButton->setColorDialogTitle( tr( "Select grid frame fill color" ) );
  mGridFrameFill2ColorButton->setColorDialogOptions( QColorDialog::ShowAlphaChannel );

  //set initial state of frame style controls
  toggleFrameControls( false );

  connect( mGridCheckBox, SIGNAL( toggled( bool ) ),
           mDrawAnnotationCheckableGroupBox, SLOT( setEnabled( bool ) ) );

  if ( composerMap )
  {
    connect( composerMap, SIGNAL( itemChanged() ), this, SLOT( setGuiElementValues() ) );

    //get composition
    QgsComposition* composition = mComposerMap->composition();
    if ( composition )
    {
      QgsAtlasComposition* atlas = &composition->atlasComposition();
      connect( atlas, SIGNAL( coverageLayerChanged( QgsVectorLayer* ) ),
               this, SLOT( atlasLayerChanged( QgsVectorLayer* ) ) );
      connect( atlas, SIGNAL( toggled( bool ) ), this, SLOT( compositionAtlasToggled( bool ) ) );
    }
  }

  updateOverviewSymbolMarker();
  updateLineSymbolMarker();

  updateGuiElements();
  blockAllSignals( false );
}

QgsComposerMapWidget::~QgsComposerMapWidget()
{
}

void QgsComposerMapWidget::compositionAtlasToggled( bool atlasEnabled )
{
  if ( atlasEnabled )
  {
    mAtlasCheckBox->setEnabled( true );
  }
  else
  {
    mAtlasCheckBox->setEnabled( false );
    mAtlasCheckBox->setChecked( false );
  }
}

void QgsComposerMapWidget::on_mAtlasCheckBox_toggled( bool checked )
{
  if ( !mComposerMap )
  {
    return;
  }

  mAtlasFixedScaleRadio->setEnabled( checked );
  mAtlasMarginRadio->setEnabled( checked );

  if ( mAtlasMarginRadio->isEnabled() && mAtlasMarginRadio->isChecked() )
  {
    mAtlasMarginSpinBox->setEnabled( true );
  }
  else
  {
    mAtlasMarginSpinBox->setEnabled( false );
  }

  mAtlasPredefinedScaleRadio->setEnabled( checked );

  if ( checked )
  {
    //check atlas coverage layer type
    QgsComposition* composition = mComposerMap->composition();
    if ( composition )
    {
      toggleAtlasScalingOptionsByLayerType();
    }
  }

  // disable predefined scales if none are defined
  if ( !hasPredefinedScales() )
  {
    mAtlasPredefinedScaleRadio->setEnabled( false );
  }

  mComposerMap->setAtlasDriven( checked );
  updateMapForAtlas();
}

void QgsComposerMapWidget::updateMapForAtlas()
{
  //update map if in atlas preview mode
  QgsComposition* composition = mComposerMap->composition();
  if ( !composition )
  {
    return;
  }
  if ( composition->atlasMode() == QgsComposition::AtlasOff )
  {
    return;
  }

  //update atlas based extent for map
  QgsAtlasComposition* atlas = &composition->atlasComposition();
  atlas->prepareMap( mComposerMap );

  //redraw map
  mComposerMap->cache();
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mAtlasMarginRadio_toggled( bool checked )
{
  mAtlasMarginSpinBox->setEnabled( checked );

  if (checked && mComposerMap)
  {
    mComposerMap->setAtlasScalingMode( QgsComposerMap::Auto );
    updateMapForAtlas();
  }
}

void QgsComposerMapWidget::on_mAtlasMarginSpinBox_valueChanged( int value )
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->setAtlasMargin( value / 100. );
  updateMapForAtlas();
}

void QgsComposerMapWidget::on_mAtlasFixedScaleRadio_toggled( bool checked )
{
  if ( !mComposerMap )
  {
    return;
  }

  if (checked)
  {
    mComposerMap->setAtlasScalingMode( QgsComposerMap::Fixed );
    updateMapForAtlas();
  }
}

void QgsComposerMapWidget::on_mAtlasPredefinedScaleRadio_toggled( bool checked )
{
  if ( !mComposerMap )
  {
    return;
  }

  if ( hasPredefinedScales() )
  {
    if ( checked )
    {
      mComposerMap->setAtlasScalingMode( QgsComposerMap::Predefined );
      updateMapForAtlas();
    }
  }
  else
  {
    // restore to fixed scale if no predefined scales exist
    mAtlasFixedScaleRadio->blockSignals( true );
    mAtlasFixedScaleRadio->setChecked( Qt::Checked );
    mAtlasFixedScaleRadio->blockSignals( false );
    mComposerMap->setAtlasScalingMode( QgsComposerMap::Fixed );
  }
}

void QgsComposerMapWidget::on_mPreviewModeComboBox_activated( int i )
{
  Q_UNUSED( i );

  if ( !mComposerMap )
  {
    return;
  }

  if ( mComposerMap->isDrawing() )
  {
    return;
  }

  QString comboText = mPreviewModeComboBox->currentText();
  if ( comboText == tr( "Cache" ) )
  {
    mComposerMap->setPreviewMode( QgsComposerMap::Cache );
    mUpdatePreviewButton->setEnabled( true );
  }
  else if ( comboText == tr( "Render" ) )
  {
    mComposerMap->setPreviewMode( QgsComposerMap::Render );
    mUpdatePreviewButton->setEnabled( true );
  }
  else if ( comboText == tr( "Rectangle" ) )
  {
    mComposerMap->setPreviewMode( QgsComposerMap::Rectangle );
    mUpdatePreviewButton->setEnabled( false );
  }

  mComposerMap->cache();
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mScaleLineEdit_editingFinished()
{
  if ( !mComposerMap )
  {
    return;
  }

  bool conversionSuccess;
  double scaleDenominator = mScaleLineEdit->text().toDouble( &conversionSuccess );

  if ( !conversionSuccess )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Map scale changed" ) );
  mComposerMap->setNewScale( scaleDenominator );
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mMapRotationSpinBox_valueChanged( double value )
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Map rotation changed" ), QgsComposerMergeCommand::ComposerMapRotation );
  mComposerMap->setMapRotation( value );
  mComposerMap->endCommand();
  mComposerMap->cache();
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mSetToMapCanvasExtentButton_clicked()
{
  if ( mComposerMap )
  {
    QgsRectangle newExtent = mComposerMap->composition()->mapSettings().visibleExtent();

    //Make sure the width/height ratio is the same as in current composer map extent.
    //This is to keep the map item frame and the page layout fixed
    QgsRectangle currentMapExtent = *( mComposerMap->currentMapExtent() );
    double currentWidthHeightRatio = currentMapExtent.width() / currentMapExtent.height();
    double newWidthHeightRatio = newExtent.width() / newExtent.height();

    if ( currentWidthHeightRatio < newWidthHeightRatio )
    {
      //enlarge height of new extent, ensuring the map center stays the same
      double newHeight = newExtent.width() / currentWidthHeightRatio;
      double deltaHeight = newHeight - newExtent.height();
      newExtent.setYMinimum( newExtent.yMinimum() - deltaHeight / 2 );
      newExtent.setYMaximum( newExtent.yMaximum() + deltaHeight / 2 );
    }
    else
    {
      //enlarge width of new extent, ensuring the map center stays the same
      double newWidth = currentWidthHeightRatio * newExtent.height();
      double deltaWidth = newWidth - newExtent.width();
      newExtent.setXMinimum( newExtent.xMinimum() - deltaWidth / 2 );
      newExtent.setXMaximum( newExtent.xMaximum() + deltaWidth / 2 );
    }

    //fill text into line edits
    mXMinLineEdit->setText( QString::number( newExtent.xMinimum() ) );
    mXMaxLineEdit->setText( QString::number( newExtent.xMaximum() ) );
    mYMinLineEdit->setText( QString::number( newExtent.yMinimum() ) );
    mYMaxLineEdit->setText( QString::number( newExtent.yMaximum() ) );

    mComposerMap->beginCommand( tr( "Map extent changed" ) );
    mComposerMap->setNewExtent( newExtent );
    mComposerMap->endCommand();
  }
}

void QgsComposerMapWidget::on_mViewExtentInCanvasButton_clicked()
{
  if ( !mComposerMap )
  {
    return;
  }

  QgsRectangle currentMapExtent = *( mComposerMap->currentMapExtent() );

  if ( !currentMapExtent.isEmpty() )
  {
    QgisApp::instance()->mapCanvas()->setExtent( currentMapExtent );
    QgisApp::instance()->mapCanvas()->refresh();
  }
}

void QgsComposerMapWidget::on_mXMinLineEdit_editingFinished()
{
  updateComposerExtentFromGui();
}

void QgsComposerMapWidget::on_mXMaxLineEdit_editingFinished()
{
  updateComposerExtentFromGui();
}

void QgsComposerMapWidget::on_mYMinLineEdit_editingFinished()
{
  updateComposerExtentFromGui();
}

void QgsComposerMapWidget::on_mYMaxLineEdit_editingFinished()
{
  updateComposerExtentFromGui();
}

void QgsComposerMapWidget::setGuiElementValues()
{
  mScaleLineEdit->blockSignals( true );
  mPreviewModeComboBox->blockSignals( true );

  updateGuiElements();

  mScaleLineEdit->blockSignals( false );
  mPreviewModeComboBox->blockSignals( false );
}

void QgsComposerMapWidget::updateGuiElements()
{
  if ( mComposerMap )
  {
    blockAllSignals( true );

    //width, height, scale
//    QRectF composerMapRect = mComposerMap->rect();
    mScaleLineEdit->setText( QString::number( mComposerMap->scale(), 'f', 0 ) );

    //preview mode
    QgsComposerMap::PreviewMode previewMode = mComposerMap->previewMode();
    int index = -1;
    if ( previewMode == QgsComposerMap::Cache )
    {
      index = mPreviewModeComboBox->findText( tr( "Cache" ) );
      mUpdatePreviewButton->setEnabled( true );
    }
    else if ( previewMode == QgsComposerMap::Render )
    {
      index = mPreviewModeComboBox->findText( tr( "Render" ) );
      mUpdatePreviewButton->setEnabled( true );
    }
    else if ( previewMode == QgsComposerMap::Rectangle )
    {
      index = mPreviewModeComboBox->findText( tr( "Rectangle" ) );
      mUpdatePreviewButton->setEnabled( false );
    }
    if ( index != -1 )
    {
      mPreviewModeComboBox->setCurrentIndex( index );
    }

    //composer map extent
    QgsRectangle composerMapExtent = *( mComposerMap->currentMapExtent() );
    mXMinLineEdit->setText( QString::number( composerMapExtent.xMinimum(), 'f', 3 ) );
    mXMaxLineEdit->setText( QString::number( composerMapExtent.xMaximum(), 'f', 3 ) );
    mYMinLineEdit->setText( QString::number( composerMapExtent.yMinimum(), 'f', 3 ) );
    mYMaxLineEdit->setText( QString::number( composerMapExtent.yMaximum(), 'f', 3 ) );

    mMapRotationSpinBox->setValue( mComposerMap->mapRotation() );

    //keep layer list check box
    if ( mComposerMap->keepLayerSet() )
    {
      mKeepLayerListCheckBox->setCheckState( Qt::Checked );
    }
    else
    {
      mKeepLayerListCheckBox->setCheckState( Qt::Unchecked );
    }

    //draw canvas items
    if ( mComposerMap->drawCanvasItems() )
    {
      mDrawCanvasItemsCheckBox->setCheckState( Qt::Checked );
    }
    else
    {
      mDrawCanvasItemsCheckBox->setCheckState( Qt::Unchecked );
    }

    //overview frame
    int overviewMapFrameId = mComposerMap->overviewFrameMapId();
    mOverviewFrameMapComboBox->setCurrentIndex( mOverviewFrameMapComboBox->findData( overviewMapFrameId ) );
    //overview frame blending mode
    mOverviewBlendModeComboBox->setBlendMode( mComposerMap->overviewBlendMode() );
    //overview inverted
    mOverviewInvertCheckbox->setChecked( mComposerMap->overviewInverted() );
    //center overview
    mOverviewCenterCheckbox->setChecked( mComposerMap->overviewCentered() );

    //grid
    if ( mComposerMap->gridEnabled() )
    {
      mGridCheckBox->setChecked( true );
    }
    else
    {
      mGridCheckBox->setChecked( false );
    }

    mIntervalXSpinBox->setValue( mComposerMap->gridIntervalX() );
    mIntervalYSpinBox->setValue( mComposerMap->gridIntervalY() );
    mOffsetXSpinBox->setValue( mComposerMap->gridOffsetX() );
    mOffsetYSpinBox->setValue( mComposerMap->gridOffsetY() );

    QgsComposerMap::GridStyle gridStyle = mComposerMap->gridStyle();
    if ( gridStyle == QgsComposerMap::Cross )
    {
      mGridTypeComboBox->setCurrentIndex( mGridTypeComboBox->findText( tr( "Cross" ) ) );
    }
    else
    {
      mGridTypeComboBox->setCurrentIndex( mGridTypeComboBox->findText( tr( "Solid" ) ) );
    }

    mCrossWidthSpinBox->setValue( mComposerMap->crossLength() );

    //grid frame
    mFrameWidthSpinBox->setValue( mComposerMap->gridFrameWidth() );
    mGridFramePenSizeSpinBox->setValue( mComposerMap->gridFramePenSize() );
    mGridFramePenColorButton->setColor( mComposerMap->gridFramePenColor() );
    mGridFrameFill1ColorButton->setColor( mComposerMap->gridFrameFillColor1() );
    mGridFrameFill2ColorButton->setColor( mComposerMap->gridFrameFillColor2() );
    QgsComposerMap::GridFrameStyle gridFrameStyle = mComposerMap->gridFrameStyle();
    if ( gridFrameStyle == QgsComposerMap::Zebra )
    {
      mFrameStyleComboBox->setCurrentIndex( mFrameStyleComboBox->findText( tr( "Zebra" ) ) );
      toggleFrameControls( true );
    }
    else //NoGridFrame
    {
      mFrameStyleComboBox->setCurrentIndex( mFrameStyleComboBox->findText( tr( "No frame" ) ) );
      toggleFrameControls( false );
    }

    //grid blend mode
    mGridBlendComboBox->setBlendMode( mComposerMap->gridBlendMode() );

    //grid annotation format
    QgsComposerMap::GridAnnotationFormat gf = mComposerMap->gridAnnotationFormat();
    mAnnotationFormatComboBox->setCurrentIndex(( int )gf );

    //grid annotation position
    initAnnotationPositionBox( mAnnotationPositionLeftComboBox, mComposerMap->gridAnnotationPosition( QgsComposerMap::Left ) );
    initAnnotationPositionBox( mAnnotationPositionRightComboBox, mComposerMap->gridAnnotationPosition( QgsComposerMap::Right ) );
    initAnnotationPositionBox( mAnnotationPositionTopComboBox, mComposerMap->gridAnnotationPosition( QgsComposerMap::Top ) );
    initAnnotationPositionBox( mAnnotationPositionBottomComboBox, mComposerMap->gridAnnotationPosition( QgsComposerMap::Bottom ) );

    //grid annotation direction
    initAnnotationDirectionBox( mAnnotationDirectionComboBoxLeft, mComposerMap->gridAnnotationDirection( QgsComposerMap::Left ) );
    initAnnotationDirectionBox( mAnnotationDirectionComboBoxRight, mComposerMap->gridAnnotationDirection( QgsComposerMap::Right ) );
    initAnnotationDirectionBox( mAnnotationDirectionComboBoxTop, mComposerMap->gridAnnotationDirection( QgsComposerMap::Top ) );
    initAnnotationDirectionBox( mAnnotationDirectionComboBoxBottom, mComposerMap->gridAnnotationDirection( QgsComposerMap::Bottom ) );

    mAnnotationFontColorButton->setColor( mComposerMap->annotationFontColor() );

    mDistanceToMapFrameSpinBox->setValue( mComposerMap->annotationFrameDistance() );

    if ( mComposerMap->showGridAnnotation() )
    {
      mDrawAnnotationCheckableGroupBox->setChecked( true );
    }
    else
    {
      mDrawAnnotationCheckableGroupBox->setChecked( false );
    }

    mCoordinatePrecisionSpinBox->setValue( mComposerMap->gridAnnotationPrecision() );

    //atlas controls
    mAtlasCheckBox->setChecked( mComposerMap->atlasDriven() );
    mAtlasMarginSpinBox->setValue( static_cast<int>( mComposerMap->atlasMargin() * 100 ) );

    mAtlasFixedScaleRadio->setEnabled( mComposerMap->atlasDriven() );
    mAtlasFixedScaleRadio->setChecked( mComposerMap->atlasScalingMode() == QgsComposerMap::Fixed );
    mAtlasMarginSpinBox->setEnabled( mComposerMap->atlasScalingMode() == QgsComposerMap::Auto );
    mAtlasMarginRadio->setEnabled( mComposerMap->atlasDriven() );
    mAtlasMarginRadio->setChecked( mComposerMap->atlasScalingMode() == QgsComposerMap::Auto );
    mAtlasPredefinedScaleRadio->setEnabled( mComposerMap->atlasDriven() );
    mAtlasPredefinedScaleRadio->setChecked( mComposerMap->atlasScalingMode() == QgsComposerMap::Predefined );

    if ( mComposerMap->atlasDriven() )
    {
      toggleAtlasScalingOptionsByLayerType();
    }
    // disable predefined scales if none are defined
    if ( !hasPredefinedScales() )
    {
      mAtlasPredefinedScaleRadio->setEnabled( false );
    }

    blockAllSignals( false );
  }
}

void QgsComposerMapWidget::toggleAtlasScalingOptionsByLayerType()
{
  if ( !mComposerMap )
  {
    return;
  }

  //get composition
  QgsComposition* composition = mComposerMap->composition();
  if ( !composition )
  {
    return;
  }

  QgsAtlasComposition* atlas = &composition->atlasComposition();

  QgsVectorLayer* coverageLayer = atlas->coverageLayer();
  if ( !coverageLayer )
  {
    return;
  }

  switch ( atlas->coverageLayer()->wkbType() )
  {
    case QGis::WKBPoint:
    case QGis::WKBPoint25D:
    case QGis::WKBMultiPoint:
    case QGis::WKBMultiPoint25D:
      //For point layers buffer setting makes no sense, so set "fixed scale" on and disable margin control
      mAtlasFixedScaleRadio->setChecked( true );
      mAtlasMarginRadio->setEnabled( false );
      mAtlasPredefinedScaleRadio->setEnabled( false );
      break;
    default:
      //Not a point layer, so enable changes to fixed scale control
      mAtlasMarginRadio->setEnabled( true );
      mAtlasPredefinedScaleRadio->setEnabled( true );
  }
}

void QgsComposerMapWidget::updateComposerExtentFromGui()
{
  if ( !mComposerMap )
  {
    return;
  }

  double xmin, ymin, xmax, ymax;
  bool conversionSuccess;

  xmin = mXMinLineEdit->text().toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    return;
  xmax = mXMaxLineEdit->text().toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    return;
  ymin = mYMinLineEdit->text().toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    return;
  ymax = mYMaxLineEdit->text().toDouble( &conversionSuccess );
  if ( !conversionSuccess )
    return;

  QgsRectangle newExtent( xmin, ymin, xmax, ymax );
  mComposerMap->beginCommand( tr( "Map extent changed" ) );
  mComposerMap->setNewExtent( newExtent );
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::blockAllSignals( bool b )
{
  mScaleLineEdit->blockSignals( b );
  mXMinLineEdit->blockSignals( b );
  mXMaxLineEdit->blockSignals( b );
  mYMinLineEdit->blockSignals( b );
  mYMaxLineEdit->blockSignals( b );
  mIntervalXSpinBox->blockSignals( b );
  mIntervalYSpinBox->blockSignals( b );
  mOffsetXSpinBox->blockSignals( b );
  mOffsetYSpinBox->blockSignals( b );
  mGridTypeComboBox->blockSignals( b );
  mCrossWidthSpinBox->blockSignals( b );
  mPreviewModeComboBox->blockSignals( b );
  mKeepLayerListCheckBox->blockSignals( b );
  mSetToMapCanvasExtentButton->blockSignals( b );
  mUpdatePreviewButton->blockSignals( b );
  mGridLineStyleButton->blockSignals( b );
  mGridBlendComboBox->blockSignals( b );
  mDrawAnnotationCheckableGroupBox->blockSignals( b );
  mAnnotationFontButton->blockSignals( b );
  mAnnotationFormatComboBox->blockSignals( b );
  mAnnotationPositionLeftComboBox->blockSignals( b );
  mAnnotationPositionRightComboBox->blockSignals( b );
  mAnnotationPositionTopComboBox->blockSignals( b );
  mAnnotationPositionBottomComboBox->blockSignals( b );
  mDistanceToMapFrameSpinBox->blockSignals( b );
  mAnnotationDirectionComboBoxLeft->blockSignals( b );
  mAnnotationDirectionComboBoxRight->blockSignals( b );
  mAnnotationDirectionComboBoxTop->blockSignals( b );
  mAnnotationDirectionComboBoxBottom->blockSignals( b );
  mCoordinatePrecisionSpinBox->blockSignals( b );
  mAnnotationFontColorButton->blockSignals( b );
  mDrawCanvasItemsCheckBox->blockSignals( b );
  mFrameStyleComboBox->blockSignals( b );
  mFrameWidthSpinBox->blockSignals( b );
  mGridFramePenSizeSpinBox->blockSignals( b );
  mGridFramePenColorButton->blockSignals( b );
  mGridFrameFill1ColorButton->blockSignals( b );
  mGridFrameFill2ColorButton->blockSignals( b );
  mOverviewFrameMapComboBox->blockSignals( b );
  mOverviewFrameStyleButton->blockSignals( b );
  mOverviewBlendModeComboBox->blockSignals( b );
  mOverviewInvertCheckbox->blockSignals( b );
  mOverviewCenterCheckbox->blockSignals( b );
  mAtlasCheckBox->blockSignals( b );
  mAtlasMarginSpinBox->blockSignals( b );
  mAtlasFixedScaleRadio->blockSignals( b );
  mAtlasMarginRadio->blockSignals( b );
}

void QgsComposerMapWidget::on_mUpdatePreviewButton_clicked()
{
  if ( !mComposerMap )
  {
    return;
  }

  if ( mComposerMap->isDrawing() )
  {
    return;
  }

  mUpdatePreviewButton->setEnabled( false ); //prevent crashes because of many button clicks

  mComposerMap->setCacheUpdated( false );
  mComposerMap->cache();
  mComposerMap->update();

  mUpdatePreviewButton->setEnabled( true );
}

void QgsComposerMapWidget::on_mKeepLayerListCheckBox_stateChanged( int state )
{
  if ( !mComposerMap )
  {
    return;
  }

  if ( state == Qt::Checked )
  {
    mComposerMap->storeCurrentLayerSet();
    mComposerMap->setKeepLayerSet( true );
  }
  else
  {
    QStringList emptyLayerSet;
    mComposerMap->setLayerSet( emptyLayerSet );
    mComposerMap->setKeepLayerSet( false );
  }
}

void QgsComposerMapWidget::on_mDrawCanvasItemsCheckBox_stateChanged( int state )
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Canvas items toggled" ) );
  mComposerMap->setDrawCanvasItems( state == Qt::Checked );
  mUpdatePreviewButton->setEnabled( false ); //prevent crashes because of many button clicks
  mComposerMap->setCacheUpdated( false );
  mComposerMap->cache();
  mComposerMap->update();
  mUpdatePreviewButton->setEnabled( true );
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mOverviewFrameMapComboBox_currentIndexChanged( const QString& text )
{
  if ( !mComposerMap )
  {
    return;
  }

  if ( text == tr( "None" ) )
  {
    mComposerMap->setOverviewFrameMap( -1 );
  }

  //get composition
  const QgsComposition* composition = mComposerMap->composition();
  if ( !composition )
  {
    return;
  }

  //extract id
  int id;
  bool conversionOk;
  QStringList textSplit = text.split( " " );
  if ( textSplit.size() < 1 )
  {
    return;
  }

  QString idString = textSplit.at( textSplit.size() - 1 );
  id = idString.toInt( &conversionOk );

  if ( !conversionOk )
  {
    return;
  }

  const QgsComposerMap* composerMap = composition->getComposerMapById( id );
  if ( !composerMap )
  {
    return;
  }

  mComposerMap->setOverviewFrameMap( id );
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mOverviewFrameStyleButton_clicked()
{
  if ( !mComposerMap )
  {
    return;
  }

  QgsFillSymbolV2* newSymbol = dynamic_cast<QgsFillSymbolV2*>( mComposerMap->overviewFrameMapSymbol()->clone() );
  QgsSymbolV2SelectorDialog d( newSymbol, QgsStyleV2::defaultStyle(), 0 );

  //QgsSymbolV2PropertiesDialog d( mComposerMap->overviewFrameMapSymbol(), 0, this );
  if ( d.exec() == QDialog::Accepted )
  {
    mComposerMap->beginCommand( tr( "Overview frame style changed" ) );
    mComposerMap->setOverviewFrameMapSymbol( newSymbol );
    updateOverviewSymbolMarker();
    mComposerMap->endCommand();
  }
  else
  {
    delete newSymbol;
  }
}

void QgsComposerMapWidget::on_mOverviewBlendModeComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  if ( mComposerMap )
  {
    mComposerMap->setOverviewBlendMode( mOverviewBlendModeComboBox->blendMode() );
  }

}
void QgsComposerMapWidget::on_mOverviewInvertCheckbox_toggled( bool state )
{
  if ( mComposerMap )
  {
    mComposerMap->setOverviewInverted( state );
  }
}

void QgsComposerMapWidget::on_mOverviewCenterCheckbox_toggled( bool state )
{
  if ( mComposerMap )
  {
    mComposerMap->setOverviewCentered( state );
  }
  mComposerMap->beginCommand( tr( "Overview centering mode changed" ) );
  mComposerMap->cache();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mGridCheckBox_toggled( bool state )
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid checkbox toggled" ) );
  if ( state )
  {
    mComposerMap->setGridEnabled( true );
  }
  else
  {
    mComposerMap->setGridEnabled( false );
  }
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mIntervalXSpinBox_editingFinished()
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid interval changed" ) );
  mComposerMap->setGridIntervalX( mIntervalXSpinBox->value() );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mIntervalYSpinBox_editingFinished()
{
  if ( !mComposerMap )
  {
    return;
  }
  mComposerMap->beginCommand( tr( "Grid interval changed" ) );
  mComposerMap->setGridIntervalY( mIntervalYSpinBox->value() );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mOffsetXSpinBox_editingFinished()
{
  if ( !mComposerMap )
  {
    return;
  }
  mComposerMap->beginCommand( tr( "Grid offset changed" ) );
  mComposerMap->setGridOffsetX( mOffsetXSpinBox->value() );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mOffsetYSpinBox_editingFinished()
{
  if ( !mComposerMap )
  {
    return;
  }
  mComposerMap->beginCommand( tr( "Grid offset changed" ) );
  mComposerMap->setGridOffsetY( mOffsetYSpinBox->value() );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mGridLineStyleButton_clicked()
{
  if ( !mComposerMap )
  {
    return;
  }

  QgsLineSymbolV2* newSymbol = dynamic_cast<QgsLineSymbolV2*>( mComposerMap->gridLineSymbol()->clone() );
  QgsSymbolV2SelectorDialog d( newSymbol, QgsStyleV2::defaultStyle(), 0 );

  if ( d.exec() == QDialog::Accepted )
  {
    mComposerMap->beginCommand( tr( "Grid line style changed" ) );
    mComposerMap->setGridLineSymbol( newSymbol );
    updateLineSymbolMarker();
    mComposerMap->endCommand();
    mComposerMap->update();
  }
  else
  {
    delete newSymbol;
  }
}

void QgsComposerMapWidget::on_mGridTypeComboBox_currentIndexChanged( const QString& text )
{
  if ( !mComposerMap )
  {
    return;
  }

  if ( text == tr( "Cross" ) )
  {
    mComposerMap->setGridStyle( QgsComposerMap::Cross );
  }
  else
  {
    mComposerMap->setGridStyle( QgsComposerMap::Solid );
  }
  mComposerMap->beginCommand( tr( "Grid type changed" ) );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mCrossWidthSpinBox_valueChanged( double d )
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid cross width changed" ) );
  mComposerMap->setCrossLength( d );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mGridBlendComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  if ( mComposerMap )
  {
    mComposerMap->setGridBlendMode( mGridBlendComboBox->blendMode() );
  }

}

void QgsComposerMapWidget::on_mAnnotationFontButton_clicked()
{
  if ( !mComposerMap )
  {
    return;
  }

  bool ok;
#if defined(Q_WS_MAC) && defined(QT_MAC_USE_COCOA)
  // Native Mac dialog works only for Qt Carbon
  QFont newFont = QFontDialog::getFont( &ok, mComposerMap->gridAnnotationFont(), 0, QString(), QFontDialog::DontUseNativeDialog );
#else
  QFont newFont = QFontDialog::getFont( &ok, mComposerMap->gridAnnotationFont() );
#endif
  if ( ok )
  {
    mComposerMap->beginCommand( tr( "Annotation font changed" ) );
    mComposerMap->setGridAnnotationFont( newFont );
    mComposerMap->updateBoundingRect();
    mComposerMap->update();
    mComposerMap->endCommand();
  }
}

void QgsComposerMapWidget::on_mAnnotationFontColorButton_colorChanged( const QColor& newFontColor )
{
  if ( !mComposerMap )
  {
    return;
  }
  mComposerMap->beginCommand( tr( "Label font changed" ) );
  mComposerMap->setAnnotationFontColor( newFontColor );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mDistanceToMapFrameSpinBox_valueChanged( double d )
{
  if ( !mComposerMap )
  {
    return;
  }
  mComposerMap->beginCommand( tr( "Annotation distance changed" ), QgsComposerMergeCommand::ComposerMapAnnotationDistance );
  mComposerMap->setAnnotationFrameDistance( d );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mAnnotationFormatComboBox_currentIndexChanged( int index )
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Annotation format changed" ) );
  mComposerMap->setGridAnnotationFormat(( QgsComposerMap::GridAnnotationFormat )index );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mAnnotationPositionLeftComboBox_currentIndexChanged( const QString& text )
{
  handleChangedAnnotationPosition( QgsComposerMap::Left, text );
}

void QgsComposerMapWidget::on_mAnnotationPositionRightComboBox_currentIndexChanged( const QString& text )
{
  handleChangedAnnotationPosition( QgsComposerMap::Right, text );
}

void QgsComposerMapWidget::on_mAnnotationPositionTopComboBox_currentIndexChanged( const QString& text )
{
  handleChangedAnnotationPosition( QgsComposerMap::Top, text );
}

void QgsComposerMapWidget::on_mAnnotationPositionBottomComboBox_currentIndexChanged( const QString& text )
{
  handleChangedAnnotationPosition( QgsComposerMap::Bottom, text );
}

void QgsComposerMapWidget::on_mDrawAnnotationCheckableGroupBox_toggled( bool state )
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Annotation toggled" ) );
  if ( state )
  {
    mComposerMap->setShowGridAnnotation( true );
  }
  else
  {
    mComposerMap->setShowGridAnnotation( false );
  }
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mAnnotationDirectionComboBoxLeft_currentIndexChanged( const QString& text )
{
  handleChangedAnnotationDirection( QgsComposerMap::Left, text );
}

void QgsComposerMapWidget::on_mAnnotationDirectionComboBoxRight_currentIndexChanged( const QString& text )
{
  handleChangedAnnotationDirection( QgsComposerMap::Right, text );
}

void QgsComposerMapWidget::on_mAnnotationDirectionComboBoxTop_currentIndexChanged( const QString& text )
{
  handleChangedAnnotationDirection( QgsComposerMap::Top, text );
}

void QgsComposerMapWidget::on_mAnnotationDirectionComboBoxBottom_currentIndexChanged( const QString& text )
{
  handleChangedAnnotationDirection( QgsComposerMap::Bottom, text );
}

void QgsComposerMapWidget::on_mCoordinatePrecisionSpinBox_valueChanged( int value )
{
  if ( !mComposerMap )
  {
    return;
  }
  mComposerMap->beginCommand( tr( "Changed annotation precision" ) );
  mComposerMap->setGridAnnotationPrecision( value );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::toggleFrameControls( bool frameEnabled )
{
  //set status of frame controls
  mFrameWidthSpinBox->setEnabled( frameEnabled );
  mGridFramePenSizeSpinBox->setEnabled( frameEnabled );
  mGridFramePenColorButton->setEnabled( frameEnabled );
  mGridFrameFill1ColorButton->setEnabled( frameEnabled );
  mGridFrameFill2ColorButton->setEnabled( frameEnabled );
  mFrameWidthLabel->setEnabled( frameEnabled );
  mFramePenLabel->setEnabled( frameEnabled );
  mFrameFillLabel->setEnabled( frameEnabled );
}

void QgsComposerMapWidget::on_mFrameStyleComboBox_currentIndexChanged( const QString& text )
{
  toggleFrameControls( text !=  tr( "No frame" ) );

  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Changed grid frame style" ) );
  if ( text == tr( "Zebra" ) )
  {
    mComposerMap->setGridFrameStyle( QgsComposerMap::Zebra );
  }
  else //no frame
  {
    mComposerMap->setGridFrameStyle( QgsComposerMap::NoGridFrame );
  }
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mFrameWidthSpinBox_valueChanged( double d )
{
  if ( mComposerMap )
  {
    mComposerMap->beginCommand( tr( "Changed grid frame width" ) );
    mComposerMap->setGridFrameWidth( d );
    mComposerMap->updateBoundingRect();
    mComposerMap->update();
    mComposerMap->endCommand();
  }
}

void QgsComposerMapWidget::on_mGridFramePenSizeSpinBox_valueChanged( double d )
{
  if ( mComposerMap )
  {
    mComposerMap->beginCommand( tr( "Changed grid frame line thickness" ) );
    mComposerMap->setGridFramePenSize( d );
    mComposerMap->updateBoundingRect();
    mComposerMap->update();
    mComposerMap->endCommand();
  }
}

void QgsComposerMapWidget::on_mGridFramePenColorButton_colorChanged( const QColor& newColor )
{
  if ( !mComposerMap )
  {
    return;
  }
  mComposerMap->beginCommand( tr( "Grid frame color changed" ) );
  mComposerMap->setGridFramePenColor( newColor );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mGridFrameFill1ColorButton_colorChanged( const QColor& newColor )
{
  if ( !mComposerMap )
  {
    return;
  }
  mComposerMap->beginCommand( tr( "Grid frame first fill color changed" ) );
  mComposerMap->setGridFrameFillColor1( newColor );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mGridFrameFill2ColorButton_colorChanged( const QColor& newColor )
{
  if ( !mComposerMap )
  {
    return;
  }
  mComposerMap->beginCommand( tr( "Grid frame second fill color changed" ) );
  mComposerMap->setGridFrameFillColor2( newColor );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::showEvent( QShowEvent * event )
{
  refreshMapComboBox();
  QWidget::showEvent( event );
}

void QgsComposerMapWidget::addPageToToolbox( QWidget* widget, const QString& name )
{
  Q_UNUSED( name );
  //TODO : wrap the widget in a collapsibleGroupBox to be more consistent with previous implementation
  mainLayout->addWidget( widget );
}

void QgsComposerMapWidget::insertAnnotationPositionEntries( QComboBox* c )
{
  c->insertItem( 0, tr( "Inside frame" ) );
  c->insertItem( 1, tr( "Outside frame" ) );
  c->insertItem( 2, tr( "Disabled" ) );
}

void QgsComposerMapWidget::insertAnnotationDirectionEntries( QComboBox* c )
{
  c->insertItem( 0, tr( "Horizontal" ) );
  c->insertItem( 1, tr( "Vertical" ) );
}

void QgsComposerMapWidget::handleChangedAnnotationPosition( QgsComposerMap::Border border, const QString& text )
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Annotation position changed" ) );
  if ( text == tr( "Inside frame" ) )
  {
    mComposerMap->setGridAnnotationPosition( QgsComposerMap::InsideMapFrame, border );
  }
  else if ( text == tr( "Disabled" ) )
  {
    mComposerMap->setGridAnnotationPosition( QgsComposerMap::Disabled, border );
  }
  else //Outside frame
  {
    mComposerMap->setGridAnnotationPosition( QgsComposerMap::OutsideMapFrame, border );
  }

  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::handleChangedAnnotationDirection( QgsComposerMap::Border border, const QString& text )
{
  if ( !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Changed annotation direction" ) );
  if ( text == tr( "Horizontal" ) )
  {
    mComposerMap->setGridAnnotationDirection( QgsComposerMap::Horizontal, border );
  }
  else //Vertical
  {
    mComposerMap->setGridAnnotationDirection( QgsComposerMap::Vertical, border );
  }
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::initAnnotationPositionBox( QComboBox* c, QgsComposerMap::GridAnnotationPosition pos )
{
  if ( !c )
  {
    return;
  }

  if ( pos == QgsComposerMap::InsideMapFrame )
  {
    c->setCurrentIndex( c->findText( tr( "Inside frame" ) ) );
  }
  else if ( pos == QgsComposerMap::OutsideMapFrame )
  {
    c->setCurrentIndex( c->findText( tr( "Outside frame" ) ) );
  }
  else //disabled
  {
    c->setCurrentIndex( c->findText( tr( "Disabled" ) ) );
  }
}

void QgsComposerMapWidget::initAnnotationDirectionBox( QComboBox* c, QgsComposerMap::GridAnnotationDirection dir )
{
  if ( !c )
  {
    return;
  }

  if ( dir == QgsComposerMap::Vertical )
  {
    c->setCurrentIndex( c->findText( tr( "Vertical" ) ) );
  }
  else //horizontal
  {
    c->setCurrentIndex( c->findText( tr( "Horizontal" ) ) );
  }
}

void QgsComposerMapWidget::updateOverviewSymbolMarker()
{
  if ( mComposerMap )
  {
    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mComposerMap->overviewFrameMapSymbol(), mOverviewFrameStyleButton->iconSize() );
    mOverviewFrameStyleButton->setIcon( icon );
  }
}

void QgsComposerMapWidget::updateLineSymbolMarker()
{
  if ( mComposerMap )
  {
    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mComposerMap->gridLineSymbol(), mGridLineStyleButton->iconSize() );
    mGridLineStyleButton->setIcon( icon );
  }
}

void QgsComposerMapWidget::refreshMapComboBox()
{
  if ( !mComposerMap )
  {
    return;
  }

  mOverviewFrameMapComboBox->blockSignals( true );

  //save the current entry in case it is still present after refresh
  QString saveComboText = mOverviewFrameMapComboBox->currentText();

  mOverviewFrameMapComboBox->clear();
  mOverviewFrameMapComboBox->addItem( tr( "None" ), -1 );
  const QgsComposition* composition = mComposerMap->composition();
  if ( !composition )
  {
    return;
  }

  QList<const QgsComposerMap*> availableMaps = composition->composerMapItems();
  QList<const QgsComposerMap*>::const_iterator mapItemIt = availableMaps.constBegin();
  for ( ; mapItemIt != availableMaps.constEnd(); ++mapItemIt )
  {
    if (( *mapItemIt )->id() != mComposerMap->id() )
    {
      mOverviewFrameMapComboBox->addItem( tr( "Map %1" ).arg(( *mapItemIt )->id() ), ( *mapItemIt )->id() );
    }
  }


  if ( !saveComboText.isEmpty() )
  {
    int saveTextIndex = mOverviewFrameMapComboBox->findText( saveComboText );
    if ( saveTextIndex == -1 )
    {
      //entry is no longer present
      mOverviewFrameMapComboBox->setCurrentIndex( mOverviewFrameMapComboBox->findText( tr( "None" ) ) );
    }
    else
    {
      mOverviewFrameMapComboBox->setCurrentIndex( saveTextIndex );
    }
  }

  mOverviewFrameMapComboBox->blockSignals( false );
}

void QgsComposerMapWidget::atlasLayerChanged( QgsVectorLayer* layer )
{
  // enable or disable fixed scale control based on layer type
  if ( !layer || !mAtlasCheckBox->isChecked() )
  {
    return;
  }

  toggleAtlasScalingOptionsByLayerType();
}

bool QgsComposerMapWidget::hasPredefinedScales() const
{
  // first look at project's scales
  QStringList scales( QgsProject::instance()->readListEntry( "Scales", "/ScalesList" ) );
  bool hasProjectScales( QgsProject::instance()->readBoolEntry( "Scales", "/useProjectScales" ) );
  if ( !hasProjectScales || scales.isEmpty() )
  {
    // default to global map tool scales
    QSettings settings;
    QString scalesStr( settings.value( "Map/scales", PROJECT_SCALES ).toString() );
    QStringList myScalesList = scalesStr.split( "," );
    return myScalesList.size() > 0 && myScalesList[0] != "";
  }
  return true;
}
