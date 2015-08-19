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
#include "qgsapplication.h"
#include "qgsmapcanvas.h"
#include "qgscomposermapgrid.h"
#include "qgscomposermapoverview.h"
#include "qgscomposermapwidget.h"
#include "qgscomposeritemwidget.h"
#include "qgscomposition.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmaprenderer.h"
#include "qgsstylev2.h"
#include "qgssymbolv2.h"
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
#include "qgsvisibilitypresetcollection.h"
#include "qgisgui.h"

#include <QMessageBox>

QgsComposerMapWidget::QgsComposerMapWidget( QgsComposerMap* composerMap )
    : QgsComposerItemBaseWidget( 0, composerMap )
    , mComposerMap( composerMap )
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
  mGridTypeComboBox->insertItem( 3, tr( "Frame and annotations only" ) );

  insertFrameDisplayEntries( mFrameDivisionsLeftComboBox );
  insertFrameDisplayEntries( mFrameDivisionsRightComboBox );
  insertFrameDisplayEntries( mFrameDivisionsTopComboBox );
  insertFrameDisplayEntries( mFrameDivisionsBottomComboBox );

  mAnnotationFormatComboBox->insertItem( 0, tr( "Decimal" ) );
  mAnnotationFormatComboBox->insertItem( 1, tr( "Decimal with suffix" ) );
  mAnnotationFormatComboBox->insertItem( 2, tr( "Degree, minute" ) );
  mAnnotationFormatComboBox->insertItem( 3, tr( "Degree, minute with suffix" ) );
  mAnnotationFormatComboBox->insertItem( 4, tr( "Degree, minute aligned" ) );
  mAnnotationFormatComboBox->insertItem( 5, tr( "Degree, minute, second" ) );
  mAnnotationFormatComboBox->insertItem( 6, tr( "Degree, minute, second with suffix" ) );
  mAnnotationFormatComboBox->insertItem( 7, tr( "Degree, minute, second aligned" ) );

  mAnnotationFontColorButton->setColorDialogTitle( tr( "Select font color" ) );
  mAnnotationFontColorButton->setAllowAlpha( true );
  mAnnotationFontColorButton->setContext( "composer" );

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

  mGridFramePenColorButton->setColorDialogTitle( tr( "Select grid frame color" ) );
  mGridFramePenColorButton->setAllowAlpha( true );
  mGridFramePenColorButton->setContext( "composer" );
  mGridFramePenColorButton->setNoColorString( tr( "Transparent frame" ) );
  mGridFramePenColorButton->setShowNoColor( true );

  mGridFrameFill1ColorButton->setColorDialogTitle( tr( "Select grid frame fill color" ) );
  mGridFrameFill1ColorButton->setAllowAlpha( true );
  mGridFrameFill1ColorButton->setContext( "composer" );
  mGridFrameFill1ColorButton->setNoColorString( tr( "Transparent fill" ) );
  mGridFrameFill1ColorButton->setShowNoColor( true );

  mGridFrameFill2ColorButton->setColorDialogTitle( tr( "Select grid frame fill color" ) );
  mGridFrameFill2ColorButton->setAllowAlpha( true );
  mGridFrameFill2ColorButton->setContext( "composer" );
  mGridFrameFill2ColorButton->setNoColorString( tr( "Transparent fill" ) );
  mGridFrameFill2ColorButton->setShowNoColor( true );

  //set initial state of frame style controls
  toggleFrameControls( false, false, false );

  QMenu* m = new QMenu( this );
  mLayerListFromPresetButton->setMenu( m );
  mLayerListFromPresetButton->setIcon( QgsApplication::getThemeIcon( "/mActionShowAllLayers.png" ) );
  mLayerListFromPresetButton->setToolTip( tr( "Set layer list from a visibility preset" ) );

  connect( m, SIGNAL( aboutToShow() ), this, SLOT( aboutToShowVisibilityPresetsMenu() ) );

  if ( composerMap )
  {
    mLabel->setText( tr( "Map %1" ).arg( composerMap->id() ) );

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
  connect( mScaleDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mScaleDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );

  connect( mMapRotationDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mMapRotationDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );

  connect( mXMinDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mXMinDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );

  connect( mYMinDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mYMinDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );

  connect( mXMaxDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mXMaxDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );

  connect( mYMaxDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mYMaxDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );

  connect( mAtlasMarginDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mAtlasMarginDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );

  connect( mLayersDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mLayersDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );

  connect( mStylePresetsDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mStylePresetsDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );

  updateGuiElements();
  loadGridEntries();
  loadOverviewEntries();
  blockAllSignals( false );
}

QgsComposerMapWidget::~QgsComposerMapWidget()
{
}

static QgsExpressionContext _getExpressionContext( const void* context )
{
  const QgsComposerObject* composerObject = ( const QgsComposerObject* ) context;
  if ( !composerObject )
  {
    return QgsExpressionContext();
  }

  QScopedPointer< QgsExpressionContext > expContext( composerObject->createExpressionContext() );
  return QgsExpressionContext( *expContext );
}

void QgsComposerMapWidget::populateDataDefinedButtons()
{
  QgsVectorLayer* vl = atlasCoverageLayer();

  Q_FOREACH ( QgsDataDefinedButton* button, findChildren< QgsDataDefinedButton* >() )
  {
    button->blockSignals( true );
    button->registerGetExpressionContextCallback( &_getExpressionContext, mComposerMap );
  }

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
  mAtlasMarginDDBtn->init( vl, mComposerMap->dataDefinedProperty( QgsComposerObject::MapAtlasMargin ),
                           QgsDataDefinedButton::AnyType, QgsDataDefinedButton::doubleDesc() );
  mStylePresetsDDBtn->init( vl, mComposerMap->dataDefinedProperty( QgsComposerObject::MapStylePreset ),
                            QgsDataDefinedButton::String, tr( "string matching a style preset name" ) );
  mLayersDDBtn->init( vl, mComposerMap->dataDefinedProperty( QgsComposerObject::MapLayers ),
                      QgsDataDefinedButton::String, tr( "list of map layer names separated by | characters" ) );

  Q_FOREACH ( QgsDataDefinedButton* button, findChildren< QgsDataDefinedButton* >() )
  {
    button->blockSignals( false );
  }
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
  else if ( widget == mAtlasMarginDDBtn )
  {
    return QgsComposerObject::MapAtlasMargin;
  }
  else if ( widget == mStylePresetsDDBtn )
  {
    return QgsComposerObject::MapStylePreset;
  }
  else if ( widget == mLayersDDBtn )
  {
    return QgsComposerObject::MapLayers;
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

void QgsComposerMapWidget::aboutToShowVisibilityPresetsMenu()
{
  QMenu* menu = qobject_cast<QMenu*>( sender() );
  if ( !menu )
    return;

  menu->clear();
  foreach ( QString presetName, QgsProject::instance()->visibilityPresetCollection()->presets() )
  {
    QAction* a = menu->addAction( presetName, this, SLOT( visibilityPresetSelected() ) );
    a->setCheckable( true );
    QStringList layers = QgsProject::instance()->visibilityPresetCollection()->presetVisibleLayers( presetName );
    QMap<QString, QString> styles = QgsProject::instance()->visibilityPresetCollection()->presetStyleOverrides( presetName );
    if ( layers == mComposerMap->layerSet() && styles == mComposerMap->layerStyleOverrides() )
      a->setChecked( true );
  }

  if ( menu->actions().isEmpty() )
    menu->addAction( tr( "No presets defined" ) )->setEnabled( false );
}

void QgsComposerMapWidget::visibilityPresetSelected()
{
  QAction* action = qobject_cast<QAction*>( sender() );
  if ( !action )
    return;

  QString presetName = action->text();
  QStringList lst = QgsProject::instance()->visibilityPresetCollection()->presetVisibleLayers( presetName );
  if ( mComposerMap )
  {
    mKeepLayerListCheckBox->setChecked( true );
    mComposerMap->setLayerSet( lst );

    mKeepLayerStylesCheckBox->setChecked( true );

    mComposerMap->setLayerStyleOverrides( QgsProject::instance()->visibilityPresetCollection()->presetStyleOverrides( presetName ) );

    mComposerMap->cache();
    mComposerMap->update();
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

  if ( mComposerMap->atlasDriven() )
  {
    //update atlas based extent for map
    QgsAtlasComposition* atlas = &composition->atlasComposition();
    //prepareMap causes a redraw
    atlas->prepareMap( mComposerMap );
  }
  else
  {
    //redraw map
    mComposerMap->cache();
    mComposerMap->update();
  }
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

  mComposerMap->beginCommand( tr( "Map extent changed" ) );
  mComposerMap->zoomToExtent( newExtent );
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

  mKeepLayerStylesCheckBox->setEnabled( mComposerMap->keepLayerSet() );
  mKeepLayerStylesCheckBox->setCheckState( mComposerMap->keepLayerStyles() ? Qt::Checked : Qt::Unchecked );

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
  mAtlasMarginSpinBox->setValue( static_cast<int>( mComposerMap->atlasMargin( QgsComposerObject::OriginalValue ) * 100 ) );

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
  mKeepLayerStylesCheckBox->blockSignals( b );
  mSetToMapCanvasExtentButton->blockSignals( b );
  mUpdatePreviewButton->blockSignals( b );

  blockGridItemsSignals( b );
  blockOverviewItemsSignals( b );
}

void QgsComposerMapWidget::handleChangedFrameDisplay( QgsComposerMapGrid::BorderSide border, const QgsComposerMapGrid::DisplayMode mode )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Frame divisions changed" ) );
  grid->setFrameDivisions( mode, border );
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::handleChangedAnnotationDisplay( QgsComposerMapGrid::BorderSide border, const QString &text )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Annotation display changed" ) );
  if ( text == tr( "Show all" ) )
  {
    grid->setAnnotationDisplay( QgsComposerMapGrid::ShowAll, border );
  }
  else if ( text == tr( "Show latitude only" ) )
  {
    grid->setAnnotationDisplay( QgsComposerMapGrid::LatitudeOnly, border );
  }
  else if ( text == tr( "Show longitude only" ) )
  {
    grid->setAnnotationDisplay( QgsComposerMapGrid::LongitudeOnly, border );
  }
  else //disabled
  {
    grid->setAnnotationDisplay( QgsComposerMapGrid::HideAll, border );
  }

  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::toggleFrameControls( bool frameEnabled, bool frameFillEnabled, bool frameSizeEnabled )
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

    mKeepLayerStylesCheckBox->setChecked( Qt::Unchecked );
  }

  mKeepLayerStylesCheckBox->setEnabled( state == Qt::Checked );
}

void QgsComposerMapWidget::on_mKeepLayerStylesCheckBox_stateChanged( int state )
{
  if ( !mComposerMap )
  {
    return;
  }

  if ( state == Qt::Checked )
  {
    mComposerMap->storeCurrentLayerStyles();
    mComposerMap->setKeepLayerStyles( true );
  }
  else
  {
    mComposerMap->setLayerStyleOverrides( QMap<QString, QString>() );
    mComposerMap->setKeepLayerStyles( false );
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
}

void QgsComposerMapWidget::insertAnnotationDirectionEntries( QComboBox* c )
{
  c->addItem( tr( "Horizontal" ), QgsComposerMapGrid::Horizontal );
  c->addItem( tr( "Vertical ascending" ), QgsComposerMapGrid::Vertical );
  c->addItem( tr( "Vertical descending" ), QgsComposerMapGrid::VerticalDescending );
}

void QgsComposerMapWidget::initFrameDisplayBox( QComboBox *c, QgsComposerMapGrid::DisplayMode display )
{
  if ( !c )
  {
    return;
  }
  c->setCurrentIndex( c->findData( display ) );
}

void QgsComposerMapWidget::initAnnotationDisplayBox( QComboBox *c, QgsComposerMapGrid::DisplayMode display )
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

void QgsComposerMapWidget::handleChangedAnnotationPosition( QgsComposerMapGrid::BorderSide border, const QString& text )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Annotation position changed" ) );
  if ( text == tr( "Inside frame" ) )
  {
    grid->setAnnotationPosition( QgsComposerMapGrid::InsideMapFrame, border );
  }
  else if ( text == tr( "Disabled" ) )
  {
    grid->setAnnotationPosition( QgsComposerMapGrid::Disabled, border );
  }
  else //Outside frame
  {
    grid->setAnnotationPosition( QgsComposerMapGrid::OutsideMapFrame, border );
  }

  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::handleChangedAnnotationDirection( QgsComposerMapGrid::BorderSide border, const QgsComposerMapGrid::AnnotationDirection& direction )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Changed annotation direction" ) );
  grid->setAnnotationDirection( direction, border );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::insertFrameDisplayEntries( QComboBox *c )
{
  c->addItem( tr( "All" ), QgsComposerMapGrid::ShowAll );
  c->addItem( tr( "Latitude/Y only" ), QgsComposerMapGrid::LatitudeOnly );
  c->addItem( tr( "Longitude/X only" ), QgsComposerMapGrid::LongitudeOnly );
}

void QgsComposerMapWidget::insertAnnotationDisplayEntries( QComboBox *c )
{
  c->insertItem( 0, tr( "Show all" ) );
  c->insertItem( 1, tr( "Show latitude only" ) );
  c->insertItem( 2, tr( "Show longitude only" ) );
  c->insertItem( 3, tr( "Disabled" ) );
}

void QgsComposerMapWidget::initAnnotationPositionBox( QComboBox* c, QgsComposerMapGrid::AnnotationPosition pos )
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

void QgsComposerMapWidget::initAnnotationDirectionBox( QComboBox* c, QgsComposerMapGrid::AnnotationDirection dir )
{
  if ( !c )
  {
    return;
  }
  c->setCurrentIndex( c->findData( dir ) );
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

  QString itemName = tr( "Grid %1" ).arg( mComposerMap->grids()->size() + 1 );
  QgsComposerMapGrid* grid = new QgsComposerMapGrid( itemName, mComposerMap );
  mComposerMap->beginCommand( tr( "Add map grid" ) );
  mComposerMap->grids()->addGrid( grid );
  mComposerMap->endCommand();
  mComposerMap->updateBoundingRect();
  mComposerMap->update();

  addGridListItem( grid->id(), grid->name() );
  mGridListWidget->setCurrentRow( 0 );
  on_mGridListWidget_currentItemChanged( mGridListWidget->currentItem(), 0 );
}

void QgsComposerMapWidget::on_mRemoveGridPushButton_clicked()
{
  QListWidgetItem* item = mGridListWidget->currentItem();
  if ( !item )
  {
    return;
  }

  mComposerMap->grids()->removeGrid( item->data( Qt::UserRole ).toString() );
  QListWidgetItem* delItem = mGridListWidget->takeItem( mGridListWidget->row( item ) );
  delete delItem;
  mComposerMap->updateBoundingRect();
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
  mComposerMap->grids()->moveGridUp( item->data( Qt::UserRole ).toString() );
  mComposerMap->update();
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
  mComposerMap->grids()->moveGridDown( item->data( Qt::UserRole ).toString() );
  mComposerMap->update();
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

  return mComposerMap->grids()->grid( item->data( Qt::UserRole ).toString() );
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
  setGridItems( mComposerMap->grids()->constGrid( current->data( Qt::UserRole ).toString() ) );
}

void QgsComposerMapWidget::on_mGridListWidget_itemChanged( QListWidgetItem* item )
{
  if ( !mComposerMap )
  {
    return;
  }

  QgsComposerMapGrid* grid = mComposerMap->grids()->grid( item->data( Qt::UserRole ).toString() );
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
  mFrameDivisionsLeftComboBox->setEnabled( enabled );
  mFrameDivisionsRightComboBox->setEnabled( enabled );
  mFrameDivisionsTopComboBox->setEnabled( enabled );
  mFrameDivisionsBottomComboBox->setEnabled( enabled );
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

void QgsComposerMapWidget::setGridItems( const QgsComposerMapGrid* grid )
{
  if ( !grid )
  {
    return;
  }

  blockGridItemsSignals( true );

  mGridCheckBox->setTitle( QString( tr( "Draw \"%1\" grid" ) ).arg( grid->name() ) );
  mGridCheckBox->setChecked( grid->enabled() );
  mIntervalXSpinBox->setValue( grid->intervalX() );
  mIntervalYSpinBox->setValue( grid->intervalY() );
  mOffsetXSpinBox->setValue( grid->offsetX() );
  mOffsetYSpinBox->setValue( grid->offsetY() );
  mCrossWidthSpinBox->setValue( grid->crossLength() );
  mFrameWidthSpinBox->setValue( grid->frameWidth() );
  mGridFramePenSizeSpinBox->setValue( grid->framePenSize() );
  mGridFramePenColorButton->setColor( grid->framePenColor() );
  mGridFrameFill1ColorButton->setColor( grid->frameFillColor1() );
  mGridFrameFill2ColorButton->setColor( grid->frameFillColor2() );

  QgsComposerMapGrid::GridStyle gridStyle = grid->style();
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
  mFrameWidthSpinBox->setValue( grid->frameWidth() );
  QgsComposerMapGrid::FrameStyle gridFrameStyle = grid->frameStyle();
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

  mCheckGridLeftSide->setChecked( grid->testFrameSideFlag( QgsComposerMapGrid::FrameLeft ) );
  mCheckGridRightSide->setChecked( grid->testFrameSideFlag( QgsComposerMapGrid::FrameRight ) );
  mCheckGridTopSide->setChecked( grid->testFrameSideFlag( QgsComposerMapGrid::FrameTop ) );
  mCheckGridBottomSide->setChecked( grid->testFrameSideFlag( QgsComposerMapGrid::FrameBottom ) );

  initFrameDisplayBox( mFrameDivisionsLeftComboBox, grid->frameDivisions( QgsComposerMapGrid::Left ) );
  initFrameDisplayBox( mFrameDivisionsRightComboBox, grid->frameDivisions( QgsComposerMapGrid::Right ) );
  initFrameDisplayBox( mFrameDivisionsTopComboBox, grid->frameDivisions( QgsComposerMapGrid::Top ) );
  initFrameDisplayBox( mFrameDivisionsBottomComboBox, grid->frameDivisions( QgsComposerMapGrid::Bottom ) );

  //line style
  updateGridLineSymbolMarker( grid );
  //marker style
  updateGridMarkerSymbolMarker( grid );

  mGridBlendComboBox->setBlendMode( grid->blendMode() );

  mDrawAnnotationGroupBox->setChecked( grid->annotationEnabled() );
  initAnnotationDisplayBox( mAnnotationDisplayLeftComboBox, grid->annotationDisplay( QgsComposerMapGrid::Left ) );
  initAnnotationDisplayBox( mAnnotationDisplayRightComboBox, grid->annotationDisplay( QgsComposerMapGrid::Right ) );
  initAnnotationDisplayBox( mAnnotationDisplayTopComboBox, grid->annotationDisplay( QgsComposerMapGrid::Top ) );
  initAnnotationDisplayBox( mAnnotationDisplayBottomComboBox, grid->annotationDisplay( QgsComposerMapGrid::Bottom ) );

  initAnnotationPositionBox( mAnnotationPositionLeftComboBox, grid->annotationPosition( QgsComposerMapGrid::Left ) );
  initAnnotationPositionBox( mAnnotationPositionRightComboBox, grid->annotationPosition( QgsComposerMapGrid::Right ) );
  initAnnotationPositionBox( mAnnotationPositionTopComboBox, grid->annotationPosition( QgsComposerMapGrid::Top ) );
  initAnnotationPositionBox( mAnnotationPositionBottomComboBox, grid->annotationPosition( QgsComposerMapGrid::Bottom ) );

  initAnnotationDirectionBox( mAnnotationDirectionComboBoxLeft, grid->annotationDirection( QgsComposerMapGrid::Left ) );
  initAnnotationDirectionBox( mAnnotationDirectionComboBoxRight, grid->annotationDirection( QgsComposerMapGrid::Right ) );
  initAnnotationDirectionBox( mAnnotationDirectionComboBoxTop, grid->annotationDirection( QgsComposerMapGrid::Top ) );
  initAnnotationDirectionBox( mAnnotationDirectionComboBoxBottom, grid->annotationDirection( QgsComposerMapGrid::Bottom ) );

  mAnnotationFontColorButton->setColor( grid->annotationFontColor() );

  //mAnnotationFormatComboBox
  switch ( grid->annotationFormat() )
  {
    case QgsComposerMapGrid::Decimal:
      mAnnotationFormatComboBox->setCurrentIndex( 0 );
      break;
    case QgsComposerMapGrid::DegreeMinute:
      mAnnotationFormatComboBox->setCurrentIndex( 3 );
      break;
    case QgsComposerMapGrid::DegreeMinuteSecond:
      mAnnotationFormatComboBox->setCurrentIndex( 6 );
      break;
    case QgsComposerMapGrid::DecimalWithSuffix:
      mAnnotationFormatComboBox->setCurrentIndex( 1 );
      break;
    case QgsComposerMapGrid::DegreeMinuteNoSuffix:
      mAnnotationFormatComboBox->setCurrentIndex( 2 );
      break;
    case QgsComposerMapGrid::DegreeMinutePadded:
      mAnnotationFormatComboBox->setCurrentIndex( 4 );
      break;
    case QgsComposerMapGrid::DegreeMinuteSecondNoSuffix:
      mAnnotationFormatComboBox->setCurrentIndex( 5 );
      break;
    case QgsComposerMapGrid::DegreeMinuteSecondPadded:
      mAnnotationFormatComboBox->setCurrentIndex( 7 );
      break;
  }
  mDistanceToMapFrameSpinBox->setValue( grid->annotationFrameDistance() );
  mCoordinatePrecisionSpinBox->setValue( grid->annotationPrecision() );

  //Unit
  QgsComposerMapGrid::GridUnit gridUnit = grid->units();
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
    QgsLineSymbolV2* nonConstSymbol = const_cast<QgsLineSymbolV2*>( grid->lineSymbol() ); //bad
    QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( nonConstSymbol, mGridLineStyleButton->iconSize() );
    mGridLineStyleButton->setIcon( icon );
  }
}

void QgsComposerMapWidget::updateGridMarkerSymbolMarker( const QgsComposerMapGrid *grid )
{
  if ( grid )
  {
    QgsMarkerSymbolV2* nonConstSymbol = const_cast<QgsMarkerSymbolV2*>( grid->markerSymbol() ); //bad
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

  QgsLineSymbolV2* newSymbol = dynamic_cast<QgsLineSymbolV2*>( grid->lineSymbol()->clone() );
  QgsSymbolV2SelectorDialog d( newSymbol, QgsStyleV2::defaultStyle(), 0, this );

  if ( d.exec() == QDialog::Accepted )
  {
    mComposerMap->beginCommand( tr( "Grid line style changed" ) );
    grid->setLineSymbol( newSymbol );
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

  QgsMarkerSymbolV2* newSymbol = dynamic_cast<QgsMarkerSymbolV2*>( grid->markerSymbol()->clone() );
  QgsSymbolV2SelectorDialog d( newSymbol, QgsStyleV2::defaultStyle(), 0, this );

  if ( d.exec() == QDialog::Accepted )
  {
    mComposerMap->beginCommand( tr( "Grid markers style changed" ) );
    grid->setMarkerSymbol( newSymbol );
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
  grid->setIntervalX( mIntervalXSpinBox->value() );
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
  grid->setIntervalY( mIntervalYSpinBox->value() );
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
  grid->setOffsetX( value );
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
  grid->setOffsetY( value );
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
  grid->setFrameWidth( val );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mCheckGridLeftSide_toggled( bool checked )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Frame left side changed" ) );
  grid->setFrameSideFlag( QgsComposerMapGrid::FrameLeft, checked );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mCheckGridRightSide_toggled( bool checked )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Frame right side changed" ) );
  grid->setFrameSideFlag( QgsComposerMapGrid::FrameRight, checked );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mCheckGridTopSide_toggled( bool checked )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Frame top side changed" ) );
  grid->setFrameSideFlag( QgsComposerMapGrid::FrameTop, checked );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mCheckGridBottomSide_toggled( bool checked )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Frame bottom side changed" ) );
  grid->setFrameSideFlag( QgsComposerMapGrid::FrameBottom, checked );
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mFrameDivisionsLeftComboBox_currentIndexChanged( int index )
{
  handleChangedFrameDisplay( QgsComposerMapGrid::Left, ( QgsComposerMapGrid::DisplayMode ) mFrameDivisionsLeftComboBox->itemData( index ).toInt() );
}

void QgsComposerMapWidget::on_mFrameDivisionsRightComboBox_currentIndexChanged( int index )
{
  handleChangedFrameDisplay( QgsComposerMapGrid::Right, ( QgsComposerMapGrid::DisplayMode ) mFrameDivisionsRightComboBox->itemData( index ).toInt() );
}

void QgsComposerMapWidget::on_mFrameDivisionsTopComboBox_currentIndexChanged( int index )
{
  handleChangedFrameDisplay( QgsComposerMapGrid::Top, ( QgsComposerMapGrid::DisplayMode ) mFrameDivisionsTopComboBox->itemData( index ).toInt() );
}

void QgsComposerMapWidget::on_mFrameDivisionsBottomComboBox_currentIndexChanged( int index )
{
  handleChangedFrameDisplay( QgsComposerMapGrid::Bottom, ( QgsComposerMapGrid::DisplayMode ) mFrameDivisionsBottomComboBox->itemData( index ).toInt() );
}

void QgsComposerMapWidget::on_mGridFramePenSizeSpinBox_valueChanged( double d )
{
  QgsComposerMapGrid* grid = currentGrid();
  if ( !grid || !mComposerMap )
  {
    return;
  }

  mComposerMap->beginCommand( tr( "Changed grid frame line thickness" ) );
  grid->setFramePenSize( d );
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
  grid->setFramePenColor( newColor );
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
  grid->setFrameFillColor1( newColor );
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
  grid->setFrameFillColor2( newColor );
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
    grid->setFrameStyle( QgsComposerMapGrid::Zebra );
    toggleFrameControls( true, true, true );
  }
  else if ( text == tr( "Interior ticks" ) )
  {
    grid->setFrameStyle( QgsComposerMapGrid::InteriorTicks );
    toggleFrameControls( true, false, true );
  }
  else if ( text == tr( "Exterior ticks" ) )
  {
    grid->setFrameStyle( QgsComposerMapGrid::ExteriorTicks );
    toggleFrameControls( true, false, true );
  }
  else if ( text == tr( "Interior and exterior ticks" ) )
  {
    grid->setFrameStyle( QgsComposerMapGrid::InteriorExteriorTicks );
    toggleFrameControls( true, false, true );
  }
  else if ( text == tr( "Line border" ) )
  {
    grid->setFrameStyle( QgsComposerMapGrid::LineBorder );
    toggleFrameControls( true, false, false );
  }
  else //no frame
  {
    grid->setFrameStyle( QgsComposerMapGrid::NoFrame );
    toggleFrameControls( false, false, false );
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
    grid->setUnits( QgsComposerMapGrid::MapUnit );
  }
  else if ( text == tr( "Millimeter" ) )
  {
    grid->setUnits( QgsComposerMapGrid::MM );
  }
  else if ( text == tr( "Centimeter" ) )
  {
    grid->setUnits( QgsComposerMapGrid::CM );
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
    grid->setStyle( QgsComposerMapGrid::Cross );
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
    grid->setStyle( QgsComposerMapGrid::Markers );
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
    grid->setStyle( QgsComposerMapGrid::Solid );
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
    grid->setStyle( QgsComposerMapGrid::FrameAnnotationsOnly );
    mCrossWidthSpinBox->setVisible( false );
    mCrossWidthLabel->setVisible( false );
    mGridLineStyleButton->setVisible( false );
    mLineStyleLabel->setVisible( false );
    mGridMarkerStyleButton->setVisible( false );
    mMarkerStyleLabel->setVisible( false );
    mGridBlendComboBox->setVisible( false );
    mGridBlendLabel->setVisible( false );
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
  grid->setAnnotationEnabled( state );
  mComposerMap->updateBoundingRect();
  mComposerMap->update();
  mComposerMap->endCommand();
}

void QgsComposerMapWidget::on_mAnnotationDisplayLeftComboBox_currentIndexChanged( const QString &text )
{
  handleChangedAnnotationDisplay( QgsComposerMapGrid::Left, text );
}

void QgsComposerMapWidget::on_mAnnotationDisplayRightComboBox_currentIndexChanged( const QString &text )
{
  handleChangedAnnotationDisplay( QgsComposerMapGrid::Right, text );
}

void QgsComposerMapWidget::on_mAnnotationDisplayTopComboBox_currentIndexChanged( const QString &text )
{
  handleChangedAnnotationDisplay( QgsComposerMapGrid::Top, text );
}

void QgsComposerMapWidget::on_mAnnotationDisplayBottomComboBox_currentIndexChanged( const QString &text )
{
  handleChangedAnnotationDisplay( QgsComposerMapGrid::Bottom, text );
}

void QgsComposerMapWidget::on_mAnnotationPositionLeftComboBox_currentIndexChanged( const QString& text )
{
  handleChangedAnnotationPosition( QgsComposerMapGrid::Left, text );
}

void QgsComposerMapWidget::on_mAnnotationPositionRightComboBox_currentIndexChanged( const QString& text )
{
  handleChangedAnnotationPosition( QgsComposerMapGrid::Right, text );
}

void QgsComposerMapWidget::on_mAnnotationPositionTopComboBox_currentIndexChanged( const QString& text )
{
  handleChangedAnnotationPosition( QgsComposerMapGrid::Top, text );
}

void QgsComposerMapWidget::on_mAnnotationPositionBottomComboBox_currentIndexChanged( const QString& text )
{
  handleChangedAnnotationPosition( QgsComposerMapGrid::Bottom, text );
}

void QgsComposerMapWidget::on_mAnnotationDirectionComboBoxLeft_currentIndexChanged( int index )
{
  handleChangedAnnotationDirection( QgsComposerMapGrid::Left, ( QgsComposerMapGrid::AnnotationDirection ) mAnnotationDirectionComboBoxLeft->itemData( index ).toInt() );
}

void QgsComposerMapWidget::on_mAnnotationDirectionComboBoxRight_currentIndexChanged( int index )
{
  handleChangedAnnotationDirection( QgsComposerMapGrid::Right, ( QgsComposerMapGrid::AnnotationDirection ) mAnnotationDirectionComboBoxRight->itemData( index ).toInt() );
}

void QgsComposerMapWidget::on_mAnnotationDirectionComboBoxTop_currentIndexChanged( int index )
{
  handleChangedAnnotationDirection( QgsComposerMapGrid::Top, ( QgsComposerMapGrid::AnnotationDirection ) mAnnotationDirectionComboBoxTop->itemData( index ).toInt() );
}

void QgsComposerMapWidget::on_mAnnotationDirectionComboBoxBottom_currentIndexChanged( int index )
{
  handleChangedAnnotationDirection( QgsComposerMapGrid::Bottom, ( QgsComposerMapGrid::AnnotationDirection ) mAnnotationDirectionComboBoxBottom->itemData( index ).toInt() );
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
  QFont newFont = QgisGui::getFont( ok, grid->annotationFont() );
  if ( ok )
  {
    mComposerMap->beginCommand( tr( "Annotation font changed" ) );
    grid->setAnnotationFont( newFont );
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
  grid->setAnnotationFontColor( color );
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

  switch ( index )
  {
    case 0:
      grid->setAnnotationFormat( QgsComposerMapGrid::Decimal );
      break;
    case 3:
      grid->setAnnotationFormat( QgsComposerMapGrid::DegreeMinute );
      break;
    case 6:
      grid->setAnnotationFormat( QgsComposerMapGrid::DegreeMinuteSecond );
      break;
    case 1:
      grid->setAnnotationFormat( QgsComposerMapGrid::DecimalWithSuffix );
      break;
    case 2:
      grid->setAnnotationFormat( QgsComposerMapGrid::DegreeMinuteNoSuffix );
      break;
    case 4:
      grid->setAnnotationFormat( QgsComposerMapGrid::DegreeMinutePadded );
      break;
    case 5:
      grid->setAnnotationFormat( QgsComposerMapGrid::DegreeMinuteSecondNoSuffix );
      break;
    case 7:
      grid->setAnnotationFormat( QgsComposerMapGrid::DegreeMinuteSecondPadded );
      break;
  }

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
  grid->setAnnotationPrecision( value );
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
  QList< QgsComposerMapGrid* > grids = mComposerMap->grids()->asList();
  QList< QgsComposerMapGrid* >::const_iterator gridIt = grids.constBegin();
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
    grid->setEnabled( true );
  }
  else
  {
    grid->setEnabled( false );
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

  QString itemName = tr( "Overview %1" ).arg( mComposerMap->overviews()->size() + 1 );
  QgsComposerMapOverview* overview = new QgsComposerMapOverview( itemName, mComposerMap );
  mComposerMap->beginCommand( tr( "Add map overview" ) );
  mComposerMap->overviews()->addOverview( overview );
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

  mComposerMap->overviews()->removeOverview( item->data( Qt::UserRole ).toString() );
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
  mComposerMap->overviews()->moveOverviewUp( item->data( Qt::UserRole ).toString() );
  mComposerMap->update();
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
  mComposerMap->overviews()->moveOverviewDown( item->data( Qt::UserRole ).toString() );
  mComposerMap->update();
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

  return mComposerMap->overviews()->overview( item->data( Qt::UserRole ).toString() );
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
  setOverviewItems( mComposerMap->overviews()->constOverview( current->data( Qt::UserRole ).toString() ) );
}

void QgsComposerMapWidget::on_mOverviewListWidget_itemChanged( QListWidgetItem* item )
{
  if ( !mComposerMap )
  {
    return;
  }

  QgsComposerMapOverview* overview = mComposerMap->overviews()->overview( item->data( Qt::UserRole ).toString() );
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
  QList< QgsComposerMapOverview* > overviews = mComposerMap->overviews()->asList();
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
  QgsSymbolV2SelectorDialog d( newSymbol, QgsStyleV2::defaultStyle(), 0, this );

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
