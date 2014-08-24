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
#include "qgscomposermapgrid.h"
#include "qgscomposermapoverview.h"
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
#include "qgsgenericprojectionselector.h"
#include "qgsproject.h"
#include <QColorDialog>
#include <QFontDialog>
#include <QMessageBox>

QgsComposerMapWidget::QgsComposerMapWidget( QgsComposerMap* composerMap ): QgsComposerItemBaseWidget( 0, composerMap ), mComposerMap( composerMap )
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
  mGridTypeComboBox->insertItem( 2, tr( "Markers" ) );

  mAnnotationFormatComboBox->insertItem( 0, tr( "Decimal" ) );
  mAnnotationFormatComboBox->insertItem( 1, tr( "DegreeMinute" ) );
  mAnnotationFormatComboBox->insertItem( 2, tr( "DegreeMinuteSecond" ) );

  mAnnotationFontColorButton->setColorDialogTitle( tr( "Select font color" ) );
  mAnnotationFontColorButton->setColorDialogOptions( QColorDialog::ShowAlphaChannel );
  mAnnotationFontColorButton->setContext( "composer" );

  insertAnnotationPositionEntries( mAnnotationPositionLeftComboBox );
  insertAnnotationPositionEntries( mAnnotationPositionRightComboBox );
  insertAnnotationPositionEntries( mAnnotationPositionTopComboBox );
  insertAnnotationPositionEntries( mAnnotationPositionBottomComboBox );

  insertAnnotationDirectionEntries( mAnnotationDirectionComboBoxLeft );
  insertAnnotationDirectionEntries( mAnnotationDirectionComboBoxRight );
  insertAnnotationDirectionEntries( mAnnotationDirectionComboBoxTop );
  insertAnnotationDirectionEntries( mAnnotationDirectionComboBoxBottom );

  mGridFramePenColorButton->setColorDialogTitle( tr( "Select grid frame color" ) );
  mGridFramePenColorButton->setColorDialogOptions( QColorDialog::ShowAlphaChannel );
  mGridFramePenColorButton->setContext( "composer" );
  mGridFramePenColorButton->setNoColorString( tr( "Transparent frame" ) );
  mGridFramePenColorButton->setShowNoColor( true );

  mGridFrameFill1ColorButton->setColorDialogTitle( tr( "Select grid frame fill color" ) );
  mGridFrameFill1ColorButton->setColorDialogOptions( QColorDialog::ShowAlphaChannel );
  mGridFrameFill1ColorButton->setContext( "composer" );
  mGridFrameFill1ColorButton->setNoColorString( tr( "Transparent fill" ) );
  mGridFrameFill1ColorButton->setShowNoColor( true );

  mGridFrameFill2ColorButton->setColorDialogTitle( tr( "Select grid frame fill color" ) );
  mGridFrameFill2ColorButton->setColorDialogOptions( QColorDialog::ShowAlphaChannel );
  mGridFrameFill2ColorButton->setContext( "composer" );
  mGridFrameFill2ColorButton->setNoColorString( tr( "Transparent fill" ) );
  mGridFrameFill2ColorButton->setShowNoColor( true );

  //set initial state of frame style controls
  toggleFrameControls( false );

  if ( composerMap )
  {
    connect( composerMap, SIGNAL( itemChanged() ), this, SLOT( setGuiElementValues() ) );

    QgsAtlasComposition* atlas = atlasComposition();
    if ( atlas )
    {
      connect( atlas, SIGNAL( coverageLayerChanged( QgsVectorLayer* ) ),
               this, SLOT( atlasLayerChanged( QgsVectorLayer* ) ) );
      connect( atlas, SIGNAL( toggled( bool ) ), this, SLOT( compositionAtlasToggled( bool ) ) );

      // repopulate data defined buttons if atlas layer changes
      connect( atlas, SIGNAL( coverageLayerChanged( QgsVectorLayer* ) ),
               this, SLOT( populateDataDefinedButtons() ) );
      connect( atlas, SIGNAL( toggled( bool ) ), this, SLOT( populateDataDefinedButtons() ) );
    }
  }

  //connections for data defined buttons
  connect( mScaleDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty( ) ) );
  connect( mScaleDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty( ) ) );
  connect( mScaleDDBtn, SIGNAL( dataDefinedActivated( bool ) ), mScaleLineEdit, SLOT( setDisabled( bool ) ) );

  connect( mMapRotationDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty( ) ) );
  connect( mMapRotationDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty( ) ) );
  connect( mMapRotationDDBtn, SIGNAL( dataDefinedActivated( bool ) ), mMapRotationSpinBox, SLOT( setDisabled( bool ) ) );

  connect( mXMinDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty( ) ) );
  connect( mXMinDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty( ) ) );
  connect( mXMinDDBtn, SIGNAL( dataDefinedActivated( bool ) ), mXMinLineEdit, SLOT( setDisabled( bool ) ) );

  connect( mYMinDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty( ) ) );
  connect( mYMinDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty( ) ) );
  connect( mYMinDDBtn, SIGNAL( dataDefinedActivated( bool ) ), mYMinLineEdit, SLOT( setDisabled( bool ) ) );

  connect( mXMaxDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty( ) ) );
  connect( mXMaxDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty( ) ) );
  connect( mXMaxDDBtn, SIGNAL( dataDefinedActivated( bool ) ), mXMaxLineEdit, SLOT( setDisabled( bool ) ) );

  connect( mYMaxDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty( ) ) );
  connect( mYMaxDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty( ) ) );
  connect( mYMaxDDBtn, SIGNAL( dataDefinedActivated( bool ) ), mYMaxLineEdit, SLOT( setDisabled( bool ) ) );

  updateGuiElements();
  loadGridEntries();
  loadOverviewEntries();
  blockAllSignals( false );
}

QgsComposerMapWidget::~QgsComposerMapWidget()
{
}

void QgsComposerMapWidget::populateDataDefinedButtons()
{
  QgsVectorLayer* vl = atlasCoverageLayer();

  //block signals from data defined buttons
  mScaleDDBtn->blockSignals( true );
  mMapRotationDDBtn->blockSignals( true );
  mXMinDDBtn->blockSignals( true );
  mYMinDDBtn->blockSignals( true );
  mXMaxDDBtn->blockSignals( true );
  mYMaxDDBtn->blockSignals( true );

  //initialise buttons to use atlas coverage layer
  mScaleDDBtn->init( vl, mComposerMap->dataDefinedProperty( QgsComposerObject::MapScale ),
                     QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
  mMapRotationDDBtn->init( vl, mComposerMap->dataDefinedProperty( QgsComposerObject::MapRotation ),
                           QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
  mXMinDDBtn->init( vl, mComposerMap->dataDefinedProperty( QgsComposerObject::MapXMin ),
                    QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
  mYMinDDBtn->init( vl, mComposerMap->dataDefinedProperty( QgsComposerObject::MapYMin ),
                    QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
  mXMaxDDBtn->init( vl, mComposerMap->dataDefinedProperty( QgsComposerObject::MapXMax ),
                    QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
  mYMaxDDBtn->init( vl, mComposerMap->dataDefinedProperty( QgsComposerObject::MapYMax ),
                    QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );

  //initial state of controls - disable related controls when dd buttons are active
  mScaleLineEdit->setEnabled( !mScaleDDBtn->isActive() );
  mMapRotationSpinBox->setEnabled( !mMapRotationDDBtn->isActive() );
  mXMinLineEdit->setEnabled( !mXMinDDBtn->isActive() );
  mYMinLineEdit->setEnabled( !mYMinDDBtn->isActive() );
  mXMaxLineEdit->setEnabled( !mXMaxDDBtn->isActive() );
  mYMaxLineEdit->setEnabled( !mYMaxDDBtn->isActive() );

  //unblock signals from data defined buttons
  mScaleDDBtn->blockSignals( false );
  mMapRotationDDBtn->blockSignals( false );
  mXMinDDBtn->blockSignals( false );
  mYMinDDBtn->blockSignals( false );
  mXMaxDDBtn->blockSignals( false );
  mYMaxDDBtn->blockSignals( false );
}

QgsComposerObject::DataDefinedProperty QgsComposerMapWidget::ddPropertyForWidget( QgsDataDefinedButton* widget )
{
  if ( widget == mScaleDDBtn )
  {
    return QgsComposerObject::MapScale;
  }
  else if ( widget == mMapRotationDDBtn )
  {
    return QgsComposerObject::MapRotation;
  }
  else if ( widget == mXMinDDBtn )
  {
    return QgsComposerObject::MapXMin;
  }
  else if ( widget == mYMinDDBtn )
  {
    return QgsComposerObject::MapYMin;
  }
  else if ( widget == mXMaxDDBtn )
  {
    return QgsComposerObject::MapXMax;
  }
  else if ( widget == mYMaxDDBtn )
  {
    return QgsComposerObject::MapYMax;
  }

  return QgsComposerObject::NoProperty;
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

  if ( checked && mComposerMap )
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

  if ( checked )
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
    mAtlasFixedScaleRadio->setChecked( true );
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

  if ( qRound( scaleDenominator ) == qRound( mComposerMap->scale() ) )
    return;

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
  if ( !mComposerMap )
  {
    return;
  }

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
  if ( !mComposerMap )
  {
    return;
  }

  blockAllSignals( true );

  //width, height, scale
  double scale = mComposerMap->scale();

  //round scale to an appropriate number of decimal places
  if ( scale >= 10 )
  {
    //round scale to integer if it's greater than 10
    mScaleLineEdit->setText( QString::number( mComposerMap->scale(), 'f', 0 ) );
  }
  else if ( scale >= 1 )
  {
    //don't round scale if it's less than 10, instead use 4 decimal places
    mScaleLineEdit->setText( QString::number( mComposerMap->scale(), 'f', 4 ) );
  }
  else
  {
    //if scale < 1 then use 10 decimal places
    mScaleLineEdit->setText( QString::number( mComposerMap->scale(), 'f', 10 ) );
  }

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

  mMapRotationSpinBox->setValue( mComposerMap->mapRotation( QgsComposerObject::OriginalValue ) );

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

  populateDataDefinedButtons();
  loadGridEntries();
  loadOverviewEntries();
  blockAllSignals( false );
}

void QgsComposerMapWidget::toggleAtlasScalingOptionsByLayerType()
{
  if ( !mComposerMap )
  {
    return;
  }

  //get atlas coverage layer
  QgsVectorLayer* coverageLayer = atlasCoverageLayer();
  if ( !coverageLayer )
  {
    return;
  }

  switch ( coverageLayer->wkbType() )
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
  mDrawCanvasItemsCheckBox->blockSignals( b );
  mOverviewFrameMapComboBox->blockSignals( b );
  mOverviewFrameStyleButton->blockSignals( b );
  mOverviewBlendModeComboBox->blockSignals( b );
  mOverviewInvertCheckbox->blockSignals( b );
  mOverviewCenterCheckbox->blockSignals( b );
  mAtlasCheckBox->blockSignals( b );
  mAtlasMarginSpinBox->blockSignals( b );
  mAtlasFixedScaleRadio->blockSignals( b );
  mAtlasMarginRadio->blockSignals( b );
  mKeepLayerListCheckBox->blockSignals( b );
  mSetToMapCanvasExtentButton->blockSignals( b );
  mUpdatePreviewButton->blockSignals( b );

  blockGridItemsSignals( b );
  blockOverviewItemsSignals( b );
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
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Annotation position changed" ) );
  if ( text == tr( "Inside frame" ) )
  {
    grid->setGridAnnotationPosition( QgsComposerMap::InsideMapFrame, border );
  }
  else if ( text == tr( "Disabled" ) )
  {
    grid->setGridAnnotationPosition( QgsComposerMap::Disabled, border );
  }
  else //Outside frame
  {
    grid->setGridAnnotationPosition( QgsComposerMap::OutsideMapFrame, border );
  }

  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::handleChangedAnnotationDirection( QgsComposerMap::Border border, const QString& text )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Changed annotation direction" ) );
  if ( text == tr( "Horizontal" ) )
  {
    grid->setGridAnnotationDirection( QgsComposerMap::Horizontal, border );
  }
  else //Vertical
  {
    grid->setGridAnnotationDirection( QgsComposerMap::Vertical, border );
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

void QgsComposerMapWidget::on_mAddGridPushButton_clicked()
{
  if ( !mComposerMap )
  {
    return;
  }

  QString itemName = tr( "Grid %1" ).arg( mComposerMap->gridCount() + 1 );
  QgsComposerMapGrid* grid = new QgsComposerMapGrid( itemName, mComposerMap );
  mComposerMap->beginCommand( tr( "Add map grid" ) );
  mComposerMap->addGrid( grid );
  mComposerMap->endCommand();
  mComposerMap->update();

  addGridListItem( grid->id(), grid->name() );
  mGridListWidget->setCurrentRow( 0 );
}

void QgsComposerMapWidget::on_mRemoveGridPushButton_clicked()
{
  QListWidgetItem* item = mGridListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  mComposerMap->removeGrid( item->text() );
  QListWidgetItem* delItem = mGridListWidget->takeItem( mGridListWidget->row( item ) );
  delete delItem;
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mGridUpButton_clicked()
{
  QListWidgetItem* item = mGridListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  int row = mGridListWidget->row( item );
  if ( row < 1 )
  {
    return;
  }
  mGridListWidget->takeItem( row );
  mGridListWidget->insertItem( row - 1, item );
  mGridListWidget->setCurrentItem( item );
  mComposerMap->moveGridUp( item->text() );
}

void QgsComposerMapWidget::on_mGridDownButton_clicked()
{
  QListWidgetItem* item = mGridListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  int row = mGridListWidget->row( item );
  if ( mGridListWidget->count() <= row )
  {
    return;
  }
  mGridListWidget->takeItem( row );
  mGridListWidget->insertItem( row + 1, item );
  mGridListWidget->setCurrentItem( item );
  mComposerMap->moveGridDown( item->text() );
}

QgsComposerMapGrid* QgsComposerMapWidget::currentGrid()
{
  if ( !mComposerMap )
  {
    return 0;
  }

  QListWidgetItem* item = mGridListWidget->currentItem();
  if ( !item )
  {
    return 0;
  }

  return mComposerMap->mapGrid( item->data( Qt::UserRole ).toString() );
}

void QgsComposerMapWidget::on_mGridListWidget_currentItemChanged( QListWidgetItem* current, QListWidgetItem* previous )
{
  Q_UNUSED( previous );
  if ( !current )
  {
    mGridCheckBox->setEnabled( false );
    return;
  }

  mGridCheckBox->setEnabled( true );
  setGridItems( mComposerMap->constMapGrid( current->data( Qt::UserRole ).toString() ) );
}

void QgsComposerMapWidget::on_mGridListWidget_itemChanged( QListWidgetItem* item )
{
  if ( !mComposerMap )
  {
    return;
  }

  QgsComposerMapGrid* grid = mComposerMap->mapGrid( item->data( Qt::UserRole ).toString() );
  if ( !grid )
  {
    return;
  }

  grid->setName( item->text() );
  if ( item->isSelected() )
  {
    //update check box title if item is current item
    mGridCheckBox->setTitle( QString( tr( "Draw \"%1\" grid" ) ).arg( grid->name() ) );
  }
}

void QgsComposerMapWidget::setGridItemsEnabled( bool enabled )
{
  mGridTypeComboBox->setEnabled( enabled );
  mIntervalXSpinBox->setEnabled( enabled );
  mIntervalYSpinBox->setEnabled( enabled );
  mOffsetXSpinBox->setEnabled( enabled );
  mOffsetYSpinBox->setEnabled( enabled );
  mCrossWidthSpinBox->setEnabled( enabled );
  mFrameStyleComboBox->setEnabled( enabled );
  mFrameWidthSpinBox->setEnabled( enabled );
  mGridLineStyleButton->setEnabled( enabled );
  mGridFramePenSizeSpinBox->setEnabled( enabled );
  mGridFramePenColorButton->setEnabled( enabled );
  mGridFrameFill1ColorButton->setEnabled( enabled );
  mGridFrameFill2ColorButton->setEnabled( enabled );
}

void QgsComposerMapWidget::blockGridItemsSignals( bool block )
{
  //grid
  mGridCheckBox->blockSignals( block );
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

  //grid annotation
  mDrawAnnotationGroupBox->blockSignals( block );
  mAnnotationFormatComboBox->blockSignals( block );
  mAnnotationPositionLeftComboBox->blockSignals( block );
  mAnnotationDirectionComboBoxLeft->blockSignals( block );
  mAnnotationPositionRightComboBox->blockSignals( block );
  mAnnotationDirectionComboBoxRight->blockSignals( block );
  mAnnotationPositionTopComboBox->blockSignals( block );
  mAnnotationDirectionComboBoxTop->blockSignals( block );
  mAnnotationPositionBottomComboBox->blockSignals( block );
  mAnnotationDirectionComboBoxBottom->blockSignals( block );
  mDistanceToMapFrameSpinBox->blockSignals( block );
  mCoordinatePrecisionSpinBox->blockSignals( block );
  mAnnotationFontColorButton->blockSignals( block );
  mAnnotationFontButton->blockSignals( block );
}

void QgsComposerMapWidget::setGridItems( const QgsComposerMapGrid* grid )
{
  if ( !grid )
  {
    return;
  }

  blockGridItemsSignals( true );

  mGridCheckBox->setTitle( QString( tr( "Draw \"%1\" grid" ) ).arg( grid->name() ) );
  mGridCheckBox->setChecked( grid->gridEnabled() );
  mIntervalXSpinBox->setValue( grid->gridIntervalX() );
  mIntervalYSpinBox->setValue( grid->gridIntervalY() );
  mOffsetXSpinBox->setValue( grid->gridOffsetX() );
  mOffsetYSpinBox->setValue( grid->gridOffsetY() );
  mCrossWidthSpinBox->setValue( grid->crossLength() );
  mFrameWidthSpinBox->setValue( grid->gridFrameWidth() );
  mGridFramePenSizeSpinBox->setValue( grid->gridFramePenSize() );
  mGridFramePenColorButton->setColor( grid->gridFramePenColor() );
  mGridFrameFill1ColorButton->setColor( grid->gridFrameFillColor1() );
  mGridFrameFill2ColorButton->setColor( grid->gridFrameFillColor2() );

  QgsComposerMap::GridStyle gridStyle = grid->gridStyle();
  switch ( gridStyle )
  {
    case QgsComposerMap::Cross:
      mGridTypeComboBox->setCurrentIndex( mGridTypeComboBox->findText( tr( "Cross" ) ) );
      mCrossWidthSpinBox->setVisible( true );
      mCrossWidthLabel->setVisible( true );
      mGridLineStyleButton->setVisible( true );
      mLineStyleLabel->setVisible( true );
      mGridMarkerStyleButton->setVisible( false );
      mMarkerStyleLabel->setVisible( false );
      break;
    case QgsComposerMap::Markers:
      mGridTypeComboBox->setCurrentIndex( mGridTypeComboBox->findText( tr( "Markers" ) ) );
      mCrossWidthSpinBox->setVisible( false );
      mCrossWidthLabel->setVisible( false );
      mGridLineStyleButton->setVisible( false );
      mLineStyleLabel->setVisible( false );
      mGridMarkerStyleButton->setVisible( true );
      mMarkerStyleLabel->setVisible( true );
      break;
    case QgsComposerMap::Solid:
      mGridTypeComboBox->setCurrentIndex( mGridTypeComboBox->findText( tr( "Solid" ) ) );
      mCrossWidthSpinBox->setVisible( false );
      mCrossWidthLabel->setVisible( false );
      mGridLineStyleButton->setVisible( true );
      mLineStyleLabel->setVisible( true );
      mGridMarkerStyleButton->setVisible( false );
      mMarkerStyleLabel->setVisible( false );
      break;
  }

  //grid frame
  mFrameWidthSpinBox->setValue( grid->gridFrameWidth() );
  QgsComposerMap::GridFrameStyle gridFrameStyle = grid->gridFrameStyle();
  if ( gridFrameStyle == QgsComposerMap::Zebra )
  {
    mFrameStyleComboBox->setCurrentIndex( 1 );
    toggleFrameControls( true );
  }
  else //NoGridFrame
  {
    mFrameStyleComboBox->setCurrentIndex( 0 );
    toggleFrameControls( false );
  }

  //line style
  updateGridLineSymbolMarker( grid );
  //marker style
  updateGridMarkerSymbolMarker( grid );

  mGridBlendComboBox->setBlendMode( grid->blendMode() );

  mDrawAnnotationGroupBox->setChecked( grid->showGridAnnotation() );
  initAnnotationPositionBox( mAnnotationPositionLeftComboBox, grid->gridAnnotationPosition( QgsComposerMap::Left ) );
  initAnnotationPositionBox( mAnnotationPositionRightComboBox, grid->gridAnnotationPosition( QgsComposerMap::Right ) );
  initAnnotationPositionBox( mAnnotationPositionTopComboBox, grid->gridAnnotationPosition( QgsComposerMap::Top ) );
  initAnnotationPositionBox( mAnnotationPositionBottomComboBox, grid->gridAnnotationPosition( QgsComposerMap::Bottom ) );

  initAnnotationDirectionBox( mAnnotationDirectionComboBoxLeft, grid->gridAnnotationDirection( QgsComposerMap::Left ) );
  initAnnotationDirectionBox( mAnnotationDirectionComboBoxRight, grid->gridAnnotationDirection( QgsComposerMap::Right ) );
  initAnnotationDirectionBox( mAnnotationDirectionComboBoxTop, grid->gridAnnotationDirection( QgsComposerMap::Top ) );
  initAnnotationDirectionBox( mAnnotationDirectionComboBoxBottom, grid->gridAnnotationDirection( QgsComposerMap::Bottom ) );

  mAnnotationFontColorButton->setColor( grid->gridAnnotationFontColor() );

  //mAnnotationFormatComboBox
  QgsComposerMap::GridAnnotationFormat gf = grid->gridAnnotationFormat();
  mAnnotationFormatComboBox->setCurrentIndex(( int )gf );
  mDistanceToMapFrameSpinBox->setValue( grid->annotationFrameDistance() );
  mCoordinatePrecisionSpinBox->setValue( grid->gridAnnotationPrecision() );

  //Unit
  QgsComposerMapGrid::GridUnit gridUnit = grid->gridUnit();
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
  QgsCoordinateReferenceSystem gridCrs = grid->crs();
  QString crsButtonText = gridCrs.isValid() ? gridCrs.authid() : tr( "change..." );
  mMapGridCRSButton->setText( crsButtonText );

  blockGridItemsSignals( false );
}

void QgsComposerMapWidget::updateGridLineSymbolMarker( const QgsComposerMapGrid* grid )
{
  if ( grid )
  {
    QgsLineSymbolV2* nonConstSymbol = const_cast<QgsLineSymbolV2*>( grid->gridLineSymbol() ); //bad
    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( nonConstSymbol, mGridLineStyleButton->iconSize() );
    mGridLineStyleButton->setIcon( icon );
  }
}

void QgsComposerMapWidget::updateGridMarkerSymbolMarker( const QgsComposerMapGrid *grid )
{
  if ( grid )
  {
    QgsMarkerSymbolV2* nonConstSymbol = const_cast<QgsMarkerSymbolV2*>( grid->gridMarkerSymbol() ); //bad
    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( nonConstSymbol, mGridMarkerStyleButton->iconSize() );
    mGridMarkerStyleButton->setIcon( icon );
  }
}

void QgsComposerMapWidget::on_mGridLineStyleButton_clicked()
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  QgsLineSymbolV2* newSymbol = dynamic_cast<QgsLineSymbolV2*>( grid->gridLineSymbol()->clone() );
  QgsSymbolV2SelectorDialog d( newSymbol, QgsStyleV2::defaultStyle(), 0 );

  if ( d.exec() == QDialog::Accepted )
  {
    mComposerMap->beginCommand( tr( "Grid line style changed" ) );
    grid->setGridLineSymbol( newSymbol );
    updateGridLineSymbolMarker( grid );
    mComposerMap->endCommand();
    mComposerMap->update();
  }
  else
  {
    delete newSymbol;
  }
}

void QgsComposerMapWidget::on_mGridMarkerStyleButton_clicked()
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  QgsMarkerSymbolV2* newSymbol = dynamic_cast<QgsMarkerSymbolV2*>( grid->gridMarkerSymbol()->clone() );
  QgsSymbolV2SelectorDialog d( newSymbol, QgsStyleV2::defaultStyle(), 0 );

  if ( d.exec() == QDialog::Accepted )
  {
    mComposerMap->beginCommand( tr( "Grid markers style changed" ) );
    grid->setGridMarkerSymbol( newSymbol );
    updateGridMarkerSymbolMarker( grid );
    mComposerMap->endCommand();
    mComposerMap->update();
  }
  else
  {
    delete newSymbol;
  }
}

void QgsComposerMapWidget::on_mIntervalXSpinBox_editingFinished()
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid interval changed" ) );
  grid->setGridIntervalX( mIntervalXSpinBox->value() );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mIntervalYSpinBox_editingFinished()
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid interval changed" ) );
  grid->setGridIntervalY( mIntervalYSpinBox->value() );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mOffsetXSpinBox_valueChanged( double value )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid offset changed" ) );
  grid->setGridOffsetX( value );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mOffsetYSpinBox_valueChanged( double value )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid offset changed" ) );
  grid->setGridOffsetY( value );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mCrossWidthSpinBox_valueChanged( double val )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Cross width changed" ) );
  grid->setCrossLength( val );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mFrameWidthSpinBox_valueChanged( double val )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Frame width changed" ) );
  grid->setGridFrameWidth( val );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mGridFramePenSizeSpinBox_valueChanged( double d )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Changed grid frame line thickness" ) );
  grid->setGridFramePenSize( d );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mGridFramePenColorButton_colorChanged( const QColor& newColor )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid frame color changed" ) );
  grid->setGridFramePenColor( newColor );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mGridFrameFill1ColorButton_colorChanged( const QColor& newColor )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid frame first fill color changed" ) );
  grid->setGridFrameFillColor1( newColor );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mGridFrameFill2ColorButton_colorChanged( const QColor& newColor )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid frame second fill color changed" ) );
  grid->setGridFrameFillColor2( newColor );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mFrameStyleComboBox_currentIndexChanged( const QString& text )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Changed grid frame style" ) );
  if ( text == tr( "Zebra" ) )
  {
    grid->setGridFrameStyle( QgsComposerMap::Zebra );
    toggleFrameControls( true );
  }
  else //no frame
  {
    grid->setGridFrameStyle( QgsComposerMap::NoGridFrame );
    toggleFrameControls( false );
  }
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mMapGridUnitComboBox_currentIndexChanged( const QString& text )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Changed grid unit" ) );
  if ( text == tr( "Map unit" ) )
  {
    grid->setGridUnit( QgsComposerMapGrid::MapUnit );
  }
  else if ( text == tr( "Millimeter" ) )
  {
    grid->setGridUnit( QgsComposerMapGrid::MM );
  }
  else if ( text == tr( "Centimeter" ) )
  {
    grid->setGridUnit( QgsComposerMapGrid::CM );
  }
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mGridBlendComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  QgsComposerMapGrid* grid = currentGrid();
  if ( grid )
  {
    mComposerMap->beginCommand( tr( "Grid blend mode changed" ) );
    grid->setBlendMode( mGridBlendComboBox->blendMode() );
    mComposerMap->update();
    mComposerMap->endCommand();
  }

}

void QgsComposerMapWidget::on_mGridTypeComboBox_currentIndexChanged( const QString& text )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid type changed" ) );
  if ( text == tr( "Cross" ) )
  {
    grid->setGridStyle( QgsComposerMap::Cross );
    mCrossWidthSpinBox->setVisible( true );
    mCrossWidthLabel->setVisible( true );
    mGridLineStyleButton->setVisible( true );
    mLineStyleLabel->setVisible( true );
    mGridMarkerStyleButton->setVisible( false );
    mMarkerStyleLabel->setVisible( false );
  }
  else if ( text == tr( "Markers" ) )
  {
    grid->setGridStyle( QgsComposerMap::Markers );
    mCrossWidthSpinBox->setVisible( false );
    mCrossWidthLabel->setVisible( false );
    mGridLineStyleButton->setVisible( false );
    mLineStyleLabel->setVisible( false );
    mGridMarkerStyleButton->setVisible( true );
    mMarkerStyleLabel->setVisible( true );
  }
  else
  {
    grid->setGridStyle( QgsComposerMap::Solid );
    mCrossWidthSpinBox->setVisible( false );
    mCrossWidthLabel->setVisible( false );
    mGridLineStyleButton->setVisible( true );
    mLineStyleLabel->setVisible( true );
    mGridMarkerStyleButton->setVisible( false );
    mMarkerStyleLabel->setVisible( false );
  }
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mMapGridCRSButton_clicked()
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid || !mComposerMap )
  {
    return;
  }

  QgsGenericProjectionSelector crsDialog( this );
  QgsCoordinateReferenceSystem crs = grid->crs();
  QString currentAuthId = crs.isValid() ? crs.authid() : mComposerMap->composition()->mapSettings().destinationCrs().authid();
  crsDialog.setSelectedAuthId( currentAuthId );

  if ( crsDialog.exec() == QDialog::Accepted )
  {
    mComposerMap->beginCommand( tr( "Grid CRS changed" ) );
    QString selectedAuthId = crsDialog.selectedAuthId();
    grid->setCrs( QgsCoordinateReferenceSystem( selectedAuthId ) );
    mMapGridCRSButton->setText( selectedAuthId );
    mComposerMap->endCommand();
  }
}

void QgsComposerMapWidget::on_mDrawAnnotationGroupBox_toggled( bool state )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Annotation toggled" ) );
  grid->setShowGridAnnotation( state );
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

void QgsComposerMapWidget::on_mDistanceToMapFrameSpinBox_valueChanged( double d )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Annotation distance changed" ), QgsComposerMergeCommand::ComposerMapAnnotationDistance );
  grid->setAnnotationFrameDistance( d );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mAnnotationFontButton_clicked()
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  bool ok;
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
  // Native Mac dialog works only for Qt Carbon
  QFont newFont = QFontDialog::getFont( &ok, grid->gridAnnotationFont(), 0, QString(), QFontDialog::DontUseNativeDialog );
#else
  QFont newFont = QFontDialog::getFont( &ok, grid->gridAnnotationFont() );
#endif
  if ( ok )
  {
    mComposerMap->beginCommand( tr( "Annotation font changed" ) );
    grid->setGridAnnotationFont( newFont );
    mComposerMap->updateBoundingRect();
    mComposerMap->update();
    mComposerMap->endCommand();
  }
}

void QgsComposerMapWidget::on_mAnnotationFontColorButton_colorChanged( const QColor& color )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Annotation color changed" ) );
  grid->setGridAnnotationFontColor( color );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mAnnotationFormatComboBox_currentIndexChanged( int index )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Annotation format changed" ) );
  grid->setGridAnnotationFormat(( QgsComposerMap::GridAnnotationFormat )index );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}



void QgsComposerMapWidget::on_mCoordinatePrecisionSpinBox_valueChanged( int value )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }
  mComposerMap->beginCommand( tr( "Changed annotation precision" ) );
  grid->setGridAnnotationPrecision( value );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

QListWidgetItem* QgsComposerMapWidget::addGridListItem( const QString& id, const QString& name )
{
  QListWidgetItem* item = new QListWidgetItem( name, 0 );
  item->setData( Qt::UserRole, id );
  item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
  mGridListWidget->insertItem( 0, item );
  return item;
}

void QgsComposerMapWidget::loadGridEntries()
{
  //save selection
  QSet<QString> selectedIds;
  QList<QListWidgetItem*> itemSelection = mGridListWidget->selectedItems();
  QList<QListWidgetItem*>::const_iterator sIt = itemSelection.constBegin();
  for ( ; sIt != itemSelection.constEnd(); ++sIt )
  {
    selectedIds.insert(( *sIt )->data( Qt::UserRole ).toString() );
  }

  mGridListWidget->clear();
  if ( !mComposerMap )
  {
    return;
  }

  //load all composer grids into list widget
  QList< const QgsComposerMapGrid* > grids = mComposerMap->mapGrids();
  QList< const QgsComposerMapGrid* >::const_iterator gridIt = grids.constBegin();
  for ( ; gridIt != grids.constEnd(); ++gridIt )
  {
    QListWidgetItem* item = addGridListItem(( *gridIt )->id(), ( *gridIt )->name() );
    if ( selectedIds.contains(( *gridIt )->id() ) )
    {
      item->setSelected( true );
      mGridListWidget->setCurrentItem( item );
    }
  }

  if ( mGridListWidget->currentItem() )
  {
    on_mGridListWidget_currentItemChanged( mGridListWidget->currentItem(), 0 );
  }
  else
  {
    on_mGridListWidget_currentItemChanged( 0, 0 );
  }
}

void QgsComposerMapWidget::on_mGridCheckBox_toggled( bool state )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Grid checkbox toggled" ) );
  if ( state )
  {
    grid->setGridEnabled( true );
  }
  else
  {
    grid->setGridEnabled( false );
  }
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mAddOverviewPushButton_clicked()
{
  if ( !mComposerMap )
  {
    return;
  }

  QString itemName = tr( "Overview %1" ).arg( mComposerMap->overviewCount() + 1 );
  QgsComposerMapOverview* overview = new QgsComposerMapOverview( itemName, mComposerMap );
  mComposerMap->beginCommand( tr( "Add map overview" ) );
  mComposerMap->addOverview( overview );
  mComposerMap->endCommand();
  mComposerMap->update();

  addOverviewListItem( overview->id(), overview->name() );
  mOverviewListWidget->setCurrentRow( 0 );
}

void QgsComposerMapWidget::on_mRemoveOverviewPushButton_clicked()
{
  QListWidgetItem* item = mOverviewListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  mComposerMap->removeOverview( item->text() );
  QListWidgetItem* delItem = mOverviewListWidget->takeItem( mOverviewListWidget->row( item ) );
  delete delItem;
  mComposerMap->update();
}

void QgsComposerMapWidget::on_mOverviewUpButton_clicked()
{
  QListWidgetItem* item = mOverviewListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  int row = mOverviewListWidget->row( item );
  if ( row < 1 )
  {
    return;
  }
  mOverviewListWidget->takeItem( row );
  mOverviewListWidget->insertItem( row - 1, item );
  mOverviewListWidget->setCurrentItem( item );
  mComposerMap->moveOverviewUp( item->text() );
}

void QgsComposerMapWidget::on_mOverviewDownButton_clicked()
{
  QListWidgetItem* item = mOverviewListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  int row = mOverviewListWidget->row( item );
  if ( mOverviewListWidget->count() <= row )
  {
    return;
  }
  mOverviewListWidget->takeItem( row );
  mOverviewListWidget->insertItem( row + 1, item );
  mOverviewListWidget->setCurrentItem( item );
  mComposerMap->moveOverviewDown( item->text() );
}

QgsComposerMapOverview* QgsComposerMapWidget::currentOverview()
{
  if ( !mComposerMap )
  {
    return 0;
  }

  QListWidgetItem* item = mOverviewListWidget->currentItem();
  if ( !item )
  {
    return 0;
  }

  return mComposerMap->mapOverview( item->data( Qt::UserRole ).toString() );
}

void QgsComposerMapWidget::on_mOverviewListWidget_currentItemChanged( QListWidgetItem* current, QListWidgetItem* previous )
{
  Q_UNUSED( previous );
  if ( !current )
  {
    mOverviewCheckBox->setEnabled( false );
    return;
  }

  mOverviewCheckBox->setEnabled( true );
  setOverviewItems( mComposerMap->constMapOverview( current->data( Qt::UserRole ).toString() ) );
}

void QgsComposerMapWidget::on_mOverviewListWidget_itemChanged( QListWidgetItem* item )
{
  if ( !mComposerMap )
  {
    return;
  }

  QgsComposerMapOverview* overview = mComposerMap->mapOverview( item->data( Qt::UserRole ).toString() );
  if ( !overview )
  {
    return;
  }

  overview->setName( item->text() );
  if ( item->isSelected() )
  {
    //update check box title if item is current item
    mOverviewCheckBox->setTitle( QString( tr( "Draw \"%1\" overview" ) ).arg( overview->name() ) );
  }
}

void QgsComposerMapWidget::setOverviewItemsEnabled( bool enabled )
{
  mOverviewFrameMapLabel->setEnabled( enabled );
  mOverviewFrameMapComboBox->setEnabled( enabled );
  mOverviewFrameStyleLabel->setEnabled( enabled );
  mOverviewFrameStyleButton->setEnabled( enabled );
  mOverviewBlendModeLabel->setEnabled( enabled );
  mOverviewBlendModeComboBox->setEnabled( enabled );
  mOverviewInvertCheckbox->setEnabled( enabled );
  mOverviewCenterCheckbox->setEnabled( enabled );
}

void QgsComposerMapWidget::blockOverviewItemsSignals( bool block )
{
  //grid
  mOverviewFrameMapComboBox->blockSignals( block );
  mOverviewFrameStyleButton->blockSignals( block );
  mOverviewBlendModeComboBox->blockSignals( block );
  mOverviewInvertCheckbox->blockSignals( block );
  mOverviewCenterCheckbox->blockSignals( block );
}

void QgsComposerMapWidget::setOverviewItems( const QgsComposerMapOverview* overview )
{
  if ( !overview )
  {
    return;
  }

  blockOverviewItemsSignals( true );

  mOverviewCheckBox->setTitle( QString( tr( "Draw \"%1\" overview" ) ).arg( overview->name() ) );
  mOverviewCheckBox->setChecked( overview->enabled() );

  //overview frame
  refreshMapComboBox();
  int overviewMapFrameId = overview->frameMapId();
  mOverviewFrameMapComboBox->setCurrentIndex( mOverviewFrameMapComboBox->findData( overviewMapFrameId ) );
  //overview frame blending mode
  mOverviewBlendModeComboBox->setBlendMode( overview->blendMode() );
  //overview inverted
  mOverviewInvertCheckbox->setChecked( overview->inverted() );
  //center overview
  mOverviewCenterCheckbox->setChecked( overview->centered() );

  //frame style
  updateOverviewFrameSymbolMarker( overview );

  blockOverviewItemsSignals( false );
}

void QgsComposerMapWidget::updateOverviewFrameSymbolMarker( const QgsComposerMapOverview* overview )
{
  if ( overview )
  {
    QgsFillSymbolV2* nonConstSymbol = const_cast<QgsFillSymbolV2*>( overview->frameSymbol() ); //bad
    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( nonConstSymbol, mOverviewFrameStyleButton->iconSize() );
    mOverviewFrameStyleButton->setIcon( icon );
  }
}

QListWidgetItem* QgsComposerMapWidget::addOverviewListItem( const QString& id, const QString& name )
{
  QListWidgetItem* item = new QListWidgetItem( name, 0 );
  item->setData( Qt::UserRole, id );
  item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable );
  mOverviewListWidget->insertItem( 0, item );
  return item;
}

void QgsComposerMapWidget::loadOverviewEntries()
{
  //save selection
  QSet<QString> selectedIds;
  QList<QListWidgetItem*> itemSelection = mOverviewListWidget->selectedItems();
  QList<QListWidgetItem*>::const_iterator sIt = itemSelection.constBegin();
  for ( ; sIt != itemSelection.constEnd(); ++sIt )
  {
    selectedIds.insert(( *sIt )->data( Qt::UserRole ).toString() );
  }

  mOverviewListWidget->clear();
  if ( !mComposerMap )
  {
    return;
  }

  //load all composer overviews into list widget
  QList< QgsComposerMapOverview* > overviews = mComposerMap->mapOverviews();
  QList< QgsComposerMapOverview* >::const_iterator overviewIt = overviews.constBegin();
  for ( ; overviewIt != overviews.constEnd(); ++overviewIt )
  {
    QListWidgetItem* item = addOverviewListItem(( *overviewIt )->id(), ( *overviewIt )->name() );
    if ( selectedIds.contains(( *overviewIt )->id() ) )
    {
      item->setSelected( true );
      mOverviewListWidget->setCurrentItem( item );
    }
  }

  if ( mOverviewListWidget->currentItem() )
  {
    on_mOverviewListWidget_currentItemChanged( mOverviewListWidget->currentItem(), 0 );
  }
  else
  {
    on_mOverviewListWidget_currentItemChanged( 0, 0 );
  }
}

void QgsComposerMapWidget::on_mOverviewCheckBox_toggled( bool state )
{
  QgsComposerMapOverview* overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Overview checkbox toggled" ) );
  if ( state )
  {
    overview->setEnabled( true );
  }
  else
  {
    overview->setEnabled( false );
  }
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mOverviewFrameMapComboBox_currentIndexChanged( const QString& text )
{
  QgsComposerMapOverview* overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  int id;

  if ( text == tr( "None" ) )
  {
    id = -1;
  }
  else
  {

    //get composition
    const QgsComposition* composition = mComposerMap->composition();
    if ( !composition )
    {
      return;
    }

    //extract id
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
  }

  mComposerMap->beginCommand( tr( "Overview map changed" ) );
  overview->setFrameMap( id );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mOverviewFrameStyleButton_clicked()
{
  QgsComposerMapOverview* overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  QgsFillSymbolV2* newSymbol = dynamic_cast<QgsFillSymbolV2*>( overview->frameSymbol()->clone() );
  QgsSymbolV2SelectorDialog d( newSymbol, QgsStyleV2::defaultStyle(), 0 );

  if ( d.exec() == QDialog::Accepted )
  {
    mComposerMap->beginCommand( tr( "Overview frame style changed" ) );
    overview->setFrameSymbol( newSymbol );
    updateOverviewFrameSymbolMarker( overview );
    mComposerMap->endCommand();
    mComposerMap->update();
  }
  else
  {
    delete newSymbol;
  }
}

void QgsComposerMapWidget::on_mOverviewBlendModeComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );
  QgsComposerMapOverview* overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Overview blend mode changed" ) );
  overview->setBlendMode( mOverviewBlendModeComboBox->blendMode() );
  mComposerMap->update();
  mComposerMap->endCommand();
}
void QgsComposerMapWidget::on_mOverviewInvertCheckbox_toggled( bool state )
{
  QgsComposerMapOverview* overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Overview inverted toggled" ) );
  overview->setInverted( state );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mOverviewCenterCheckbox_toggled( bool state )
{
  QgsComposerMapOverview* overview = currentOverview();
  if ( !overview )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Overview centered toggled" ) );
  overview->setCentered( state );
  mComposerMap->update();
  mComposerMap->endCommand();
}
