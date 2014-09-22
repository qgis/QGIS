/***************************************************************************
                         qgscomposerattributetablewidget.cpp
                         -----------------------------------
    begin                : September 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerattributetablewidget.h"
#include "qgscomposerframe.h"
#include "qgsattributeselectiondialog.h"
#include "qgscomposeritemwidget.h"
#include "qgscomposerattributetablev2.h"
#include "qgscomposermultiframecommand.h"
#include "qgscomposertablecolumn.h"
#include "qgscomposermap.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"
#include "qgsexpressionbuilderdialog.h"
#include <QColorDialog>
#include <QFontDialog>

QgsComposerAttributeTableWidget::QgsComposerAttributeTableWidget( QgsComposerAttributeTableV2* table, QgsComposerFrame* frame )
    : QgsComposerItemBaseWidget( 0, table )
    , mComposerTable( table )
    , mFrame( frame )
{
  setupUi( this );
  //add widget for general composer item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, mFrame );
  mainLayout->addWidget( itemPropertiesWidget );

  blockAllSignals( true );

  mResizeModeComboBox->addItem( tr( "Use existing frames" ), QgsComposerMultiFrame::UseExistingFrames );
  mResizeModeComboBox->addItem( tr( "Extend to next page" ), QgsComposerMultiFrame::ExtendToNextPage );
  mResizeModeComboBox->addItem( tr( "Repeat until finished" ), QgsComposerMultiFrame::RepeatUntilFinished );

  bool atlasEnabled = atlasComposition() && atlasComposition()->enabled();
  mSourceComboBox->addItem( tr( "Layer features" ), QgsComposerAttributeTableV2::LayerAttributes );
  if ( atlasEnabled )
  {
    mSourceComboBox->addItem( tr( "Current atlas feature" ), QgsComposerAttributeTableV2::AtlasFeature );
  }

  mLayerComboBox->setFilters( QgsMapLayerProxyModel::VectorLayer );
  connect( mLayerComboBox, SIGNAL( layerChanged( QgsMapLayer* ) ), this, SLOT( changeLayer( QgsMapLayer* ) ) );

  refreshMapComboBox();

  mHeaderFontColorButton->setColorDialogTitle( tr( "Select header font color" ) );
  mHeaderFontColorButton->setAllowAlpha( true );
  mHeaderFontColorButton->setContext( "composer" );
  mContentFontColorButton->setColorDialogTitle( tr( "Select content font color" ) );
  mContentFontColorButton->setAllowAlpha( true );
  mContentFontColorButton->setContext( "composer" );
  mGridColorButton->setColorDialogTitle( tr( "Select grid color" ) );
  mGridColorButton->setAllowAlpha( true );
  mGridColorButton->setContext( "composer" );

  updateGuiElements();
  on_mComposerMapComboBox_activated( mComposerMapComboBox->currentIndex() );

  if ( mComposerTable )
  {
    QObject::connect( mComposerTable, SIGNAL( changed() ), this, SLOT( updateGuiElements() ) );

    QgsAtlasComposition* atlas = atlasComposition();
    if ( atlas )
    {
      // repopulate table source combo box if atlas properties change
//      connect( atlas, SIGNAL( coverageLayerChanged( QgsVectorLayer* ) ),
//               this, SLOT( populateDataDefinedButtons() ) );
      connect( atlas, SIGNAL( toggled( bool ) ), this, SLOT( atlasToggled() ) );
    }
  }
}

QgsComposerAttributeTableWidget::~QgsComposerAttributeTableWidget()
{

}

void QgsComposerAttributeTableWidget::showEvent( QShowEvent* /* event */ )
{
  refreshMapComboBox();
}

void QgsComposerAttributeTableWidget::refreshMapComboBox()
{
  //save the current entry in case it is still present after refresh
  QString saveCurrentComboText = mComposerMapComboBox->currentText();

  mComposerMapComboBox->blockSignals( true );
  mComposerMapComboBox->clear();
  if ( mComposerTable )
  {
    const QgsComposition* tableComposition = mComposerTable->composition();
    if ( tableComposition )
    {
      QList<const QgsComposerMap*> mapList = tableComposition->composerMapItems();
      QList<const QgsComposerMap*>::const_iterator mapIt = mapList.constBegin();
      for ( ; mapIt != mapList.constEnd(); ++mapIt )
      {
        int mapId = ( *mapIt )->id();
        mComposerMapComboBox->addItem( tr( "Map %1" ).arg( mapId ), mapId );
      }
    }
  }
  mComposerMapComboBox->blockSignals( false );

  if ( mComposerMapComboBox->findText( saveCurrentComboText ) == -1 )
  {
    //the former entry is no longer present. Inform the scalebar about the changed composer map
    on_mComposerMapComboBox_activated( mComposerMapComboBox->currentIndex() );
  }
  else
  {
    //the former entry is still present. Make it the current entry again
    mComposerMapComboBox->setCurrentIndex( mComposerMapComboBox->findText( saveCurrentComboText ) );
  }
}

void QgsComposerAttributeTableWidget::on_mRefreshPushButton_clicked()
{
  if ( !mComposerTable )
  {
    return;
  }

  mComposerTable->refreshAttributes();
}

void QgsComposerAttributeTableWidget::on_mAttributesPushButton_clicked()
{
  if ( !mComposerTable )
  {
    return;
  }

  //make deep copy of current columns, so we can restore them in case of cancellation
  QList<QgsComposerTableColumn*> currentColumns;
  QList<QgsComposerTableColumn*>::const_iterator it = mComposerTable->columns()->constBegin();
  for ( ; it != mComposerTable->columns()->constEnd() ; ++it )
  {
    QgsComposerTableColumn* copy = ( *it )->clone();
    currentColumns.append( copy );
  }

  QgsComposition* composition = mComposerTable->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mComposerTable, tr( "Table attribute settings" ) );
  }

  QgsAttributeSelectionDialog d( mComposerTable, mComposerTable->vectorLayer(), 0 );
  if ( d.exec() == QDialog::Accepted )
  {
    mComposerTable->refreshAttributes();
    mComposerTable->update();
    if ( composition )
    {
      composition->endMultiFrameCommand();
    }

    //clear currentColumns to free memory
    qDeleteAll( currentColumns );
    currentColumns.clear();
  }
  else
  {
    //undo changes
    mComposerTable->setColumns( currentColumns );
    if ( composition )
    {
      composition->cancelMultiFrameCommand();
    }
  }
}

void QgsComposerAttributeTableWidget::on_mComposerMapComboBox_activated( int index )
{
  if ( !mComposerTable )
  {
    return;
  }

  QVariant itemData = mComposerMapComboBox->itemData( index );
  if ( itemData.type() == QVariant::Invalid )
  {
    return;
  }

  int mapId = itemData.toInt();
  const QgsComposition* tableComposition = mComposerTable->composition();
  if ( tableComposition )
  {
    QgsComposition* composition = mComposerTable->composition();
    if ( sender() && composition ) //only create command if called from GUI
    {
      composition->beginMultiFrameCommand( mComposerTable, tr( "Table map changed" ) );
    }
    mComposerTable->setComposerMap( tableComposition->getComposerMapById( mapId ) );
    mComposerTable->update();
    if ( sender() && composition )
    {
      composition->endMultiFrameCommand();
    }
  }
}

void QgsComposerAttributeTableWidget::on_mMaximumRowsSpinBox_valueChanged( int i )
{
  if ( !mComposerTable )
  {
    return;
  }

  QgsComposition* composition = mComposerTable->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mComposerTable, tr( "Table maximum columns" ), QgsComposerMultiFrameMergeCommand::TableMaximumFeatures );
  }
  mComposerTable->setMaximumNumberOfFeatures( i );
  mComposerTable->update();
  if ( composition )
  {
    composition->endMultiFrameCommand();
  }
}

void QgsComposerAttributeTableWidget::on_mMarginSpinBox_valueChanged( double d )
{
  if ( !mComposerTable )
  {
    return;
  }

  QgsComposition* composition = mComposerTable->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mComposerTable, tr( "Table margin changed" ), QgsComposerMultiFrameMergeCommand::TableMargin );
  }
  mComposerTable->setCellMargin( d );
  mComposerTable->update();
  if ( composition )
  {
    composition->endMultiFrameCommand();
  }
}

void QgsComposerAttributeTableWidget::on_mHeaderFontPushButton_clicked()
{
  if ( !mComposerTable )
  {
    return;
  }

  bool ok;
#if defined(Q_WS_MAC) && defined(QT_MAC_USE_COCOA)
  // Native Mac dialog works only for Qt Carbon
  QFont newFont = QFontDialog::getFont( &ok, mComposerTable->headerFont(), 0, tr( "Select Font" ), QFontDialog::DontUseNativeDialog );
#else
  QFont newFont = QFontDialog::getFont( &ok, mComposerTable->headerFont(), 0, tr( "Select Font" ) );
#endif
  if ( ok )
  {
    QgsComposition* composition = mComposerTable->composition();
    if ( composition )
    {
      composition->beginMultiFrameCommand( mComposerTable, tr( "Table header font" ) );
    }
    mComposerTable->setHeaderFont( newFont );
    if ( composition )
    {
      composition->endMultiFrameCommand();
    }
  }
}

void QgsComposerAttributeTableWidget::on_mHeaderFontColorButton_colorChanged( const QColor &newColor )
{
  if ( !mComposerTable )
  {
    return;
  }

  QgsComposition* composition = mComposerTable->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mComposerTable, tr( "Table header font color" ) );
  }
  mComposerTable->setHeaderFontColor( newColor );
  mComposerTable->update();
  if ( composition )
  {
    composition->endMultiFrameCommand();
  }
}

void QgsComposerAttributeTableWidget::on_mContentFontPushButton_clicked()
{
  if ( !mComposerTable )
  {
    return;
  }

  bool ok;
#if defined(Q_WS_MAC) && defined(QT_MAC_USE_COCOA)
  // Native Mac dialog works only for Qt Carbon
  QFont newFont = QFontDialog::getFont( &ok, mComposerTable->contentFont(), 0, tr( "Select Font" ), QFontDialog::DontUseNativeDialog );
#else
  QFont newFont = QFontDialog::getFont( &ok, mComposerTable->contentFont(), 0, tr( "Select Font" ) );
#endif
  if ( ok )
  {
    QgsComposition* composition = mComposerTable->composition();
    if ( composition )
    {
      composition->beginMultiFrameCommand( mComposerTable, tr( "Table content font" ) );
    }
    mComposerTable->setContentFont( newFont );
    if ( composition )
    {
      composition->endMultiFrameCommand();
    }
  }
}

void QgsComposerAttributeTableWidget::on_mContentFontColorButton_colorChanged( const QColor &newColor )
{
  if ( !mComposerTable )
  {
    return;
  }

  QgsComposition* composition = mComposerTable->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mComposerTable, tr( "Table content font color" ) );
  }
  mComposerTable->setContentFontColor( newColor );
  mComposerTable->update();
  if ( composition )
  {
    composition->endMultiFrameCommand();
  }
}

void QgsComposerAttributeTableWidget::on_mGridStrokeWidthSpinBox_valueChanged( double d )
{
  if ( !mComposerTable )
  {
    return;
  }

  QgsComposition* composition = mComposerTable->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mComposerTable, tr( "Table grid stroke" ), QgsComposerMultiFrameMergeCommand::TableGridStrokeWidth );
  }
  mComposerTable->setGridStrokeWidth( d );
  mComposerTable->update();
  if ( composition )
  {
    composition->endMultiFrameCommand();
  }
}

void QgsComposerAttributeTableWidget::on_mGridColorButton_colorChanged( const QColor& newColor )
{
  if ( !mComposerTable )
  {
    return;
  }

  QgsComposition* composition = mComposerTable->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mComposerTable, tr( "Table grid color" ) );
  }
  mComposerTable->setGridColor( newColor );
  mComposerTable->update();
  if ( composition )
  {
    composition->endMultiFrameCommand();
  }
}

void QgsComposerAttributeTableWidget::on_mShowGridGroupCheckBox_toggled( bool state )
{
  if ( !mComposerTable )
  {
    return;
  }

  QgsComposition* composition = mComposerTable->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mComposerTable, tr( "Table grid toggled" ) );
  }
  mComposerTable->setShowGrid( state );
  mComposerTable->update();
  if ( composition )
  {
    composition->endMultiFrameCommand();
  }
}


void QgsComposerAttributeTableWidget::updateGuiElements()
{
  if ( !mComposerTable )
  {
    return;
  }

  blockAllSignals( true );

  mSourceComboBox->setCurrentIndex( mSourceComboBox->findData( mComposerTable->source() ) );

  //layer combo box
  if ( mComposerTable->vectorLayer() )
  {
    mLayerComboBox->setLayer( mComposerTable->vectorLayer() );
    if ( mComposerTable->vectorLayer()->geometryType() == QGis::NoGeometry )
    {
      //layer has no geometry, so uncheck & disable controls which require geometry
      mShowOnlyVisibleFeaturesCheckBox->setChecked( false );
      mShowOnlyVisibleFeaturesCheckBox->setEnabled( false );
    }
    else
    {
      mShowOnlyVisibleFeaturesCheckBox->setEnabled( true );
    }
  }

  //map combo box
  const QgsComposerMap* cm = mComposerTable->composerMap();
  if ( cm )
  {
    int mapIndex = mComposerMapComboBox->findText( tr( "Map %1" ).arg( cm->id() ) );
    if ( mapIndex != -1 )
    {
      mComposerMapComboBox->setCurrentIndex( mapIndex );
    }
  }
  mMaximumRowsSpinBox->setValue( mComposerTable->maximumNumberOfFeatures() );
  mMarginSpinBox->setValue( mComposerTable->cellMargin() );
  mGridStrokeWidthSpinBox->setValue( mComposerTable->gridStrokeWidth() );
  mGridColorButton->setColor( mComposerTable->gridColor() );
  if ( mComposerTable->showGrid() )
  {
    mShowGridGroupCheckBox->setChecked( true );
  }
  else
  {
    mShowGridGroupCheckBox->setChecked( false );
  }

  mHeaderFontColorButton->setColor( mComposerTable->headerFontColor() );
  mContentFontColorButton->setColor( mComposerTable->contentFontColor() );

  if ( mComposerTable->displayOnlyVisibleFeatures() && mShowOnlyVisibleFeaturesCheckBox->isEnabled() )
  {
    mShowOnlyVisibleFeaturesCheckBox->setCheckState( Qt::Checked );
    mComposerMapComboBox->setEnabled( true );
    mComposerMapLabel->setEnabled( true );
  }
  else
  {
    mShowOnlyVisibleFeaturesCheckBox->setCheckState( Qt::Unchecked );
    mComposerMapComboBox->setEnabled( false );
    mComposerMapLabel->setEnabled( false );
  }

  mFeatureFilterEdit->setText( mComposerTable->featureFilter() );
  mFeatureFilterCheckBox->setCheckState( mComposerTable->filterFeatures() ? Qt::Checked : Qt::Unchecked );
  mFeatureFilterEdit->setEnabled( mComposerTable->filterFeatures() );
  mFeatureFilterButton->setEnabled( mComposerTable->filterFeatures() );

  mHeaderHAlignmentComboBox->setCurrentIndex(( int )mComposerTable->headerHAlignment() );
  mHeaderModeComboBox->setCurrentIndex(( int )mComposerTable->headerMode() );

  mResizeModeComboBox->setCurrentIndex( mResizeModeComboBox->findData( mComposerTable->resizeMode() ) );
  mAddFramePushButton->setEnabled( mComposerTable->resizeMode() == QgsComposerMultiFrame::UseExistingFrames );

  toggleSourceControls();

  blockAllSignals( false );
}

void QgsComposerAttributeTableWidget::atlasToggled()
{
  //display/hide atlas options in source combobox depending on atlas status
  bool atlasEnabled = atlasComposition() && atlasComposition()->enabled();
  if ( !atlasEnabled )
  {
    if ( mComposerTable->source() == QgsComposerAttributeTableV2::AtlasFeature )
    {
      mComposerTable->setSource( QgsComposerAttributeTableV2::LayerAttributes );
    }
    mSourceComboBox->removeItem( mSourceComboBox->findData( QgsComposerAttributeTableV2::AtlasFeature ) );
  }
  else
  {
    if ( mSourceComboBox->findData( QgsComposerAttributeTableV2::AtlasFeature ) == -1 )
    {
      //add missing atlasfeature option to combobox
      mSourceComboBox->addItem( tr( "Current atlas feature" ), QgsComposerAttributeTableV2::AtlasFeature );
    }
  }
  mSourceComboBox->blockSignals( true );
  mSourceComboBox->setCurrentIndex( mSourceComboBox->findData( mComposerTable->source() ) );
  mSourceComboBox->blockSignals( false );
}

void QgsComposerAttributeTableWidget::blockAllSignals( bool b )
{
  mSourceComboBox->blockSignals( b );
  mLayerComboBox->blockSignals( b );
  mComposerMapComboBox->blockSignals( b );
  mMaximumRowsSpinBox->blockSignals( b );
  mMarginSpinBox->blockSignals( b );
  mGridColorButton->blockSignals( b );
  mGridStrokeWidthSpinBox->blockSignals( b );
  mShowGridGroupCheckBox->blockSignals( b );
  mShowOnlyVisibleFeaturesCheckBox->blockSignals( b );
  mFeatureFilterEdit->blockSignals( b );
  mFeatureFilterCheckBox->blockSignals( b );
  mHeaderHAlignmentComboBox->blockSignals( b );
  mHeaderModeComboBox->blockSignals( b );
  mHeaderFontColorButton->blockSignals( b );
  mContentFontColorButton->blockSignals( b );
  mResizeModeComboBox->blockSignals( b );
}

void QgsComposerAttributeTableWidget::setMaximumNumberOfFeatures( int n )
{
  mMaximumRowsSpinBox->blockSignals( true );
  mMaximumRowsSpinBox->setValue( n );
  mMaximumRowsSpinBox->blockSignals( false );
}

void QgsComposerAttributeTableWidget::on_mShowOnlyVisibleFeaturesCheckBox_stateChanged( int state )
{
  if ( !mComposerTable )
  {
    return;
  }

  QgsComposition* composition = mComposerTable->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mComposerTable, tr( "Table visible only toggled" ) );
  }
  bool showOnlyVisibleFeatures = ( state == Qt::Checked );
  mComposerTable->setDisplayOnlyVisibleFeatures( showOnlyVisibleFeatures );
  mComposerTable->update();
  if ( composition )
  {
    composition->endMultiFrameCommand();
  }

  //enable/disable map combobox based on state of checkbox
  mComposerMapComboBox->setEnabled( state == Qt::Checked );
  mComposerMapLabel->setEnabled( state == Qt::Checked );
}

void QgsComposerAttributeTableWidget::on_mFeatureFilterCheckBox_stateChanged( int state )
{
  if ( !mComposerTable )
  {
    return;
  }

  if ( state == Qt::Checked )
  {
    mFeatureFilterEdit->setEnabled( true );
    mFeatureFilterButton->setEnabled( true );
  }
  else
  {
    mFeatureFilterEdit->setEnabled( false );
    mFeatureFilterButton->setEnabled( false );
  }

  QgsComposition* composition = mComposerTable->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mComposerTable, tr( "Table feature filter toggled" ) );
  }
  mComposerTable->setFilterFeatures( state == Qt::Checked );
  mComposerTable->update();
  if ( composition )
  {
    composition->endMultiFrameCommand();
  }
}

void QgsComposerAttributeTableWidget::on_mFeatureFilterEdit_editingFinished()
{
  if ( !mComposerTable )
  {
    return;
  }

  QgsComposition* composition = mComposerTable->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mComposerTable, tr( "Table feature filter modified" ) );
  }
  mComposerTable->setFeatureFilter( mFeatureFilterEdit->text() );
  mComposerTable->update();
  if ( composition )
  {
    composition->endMultiFrameCommand();
  }
}

void QgsComposerAttributeTableWidget::on_mFeatureFilterButton_clicked()
{
  if ( !mComposerTable )
  {
    return;
  }

  QgsExpressionBuilderDialog exprDlg( mComposerTable->vectorLayer(), mFeatureFilterEdit->text(), this );
  exprDlg.setWindowTitle( tr( "Expression based filter" ) );
  if ( exprDlg.exec() == QDialog::Accepted )
  {
    QString expression =  exprDlg.expressionText();
    if ( !expression.isEmpty() )
    {
      mFeatureFilterEdit->setText( expression );
      QgsComposition* composition = mComposerTable->composition();
      if ( composition )
      {
        composition->beginMultiFrameCommand( mComposerTable, tr( "Table feature filter modified" ) );
      }
      mComposerTable->setFeatureFilter( mFeatureFilterEdit->text() );
      mComposerTable->update();
      if ( composition )
      {
        composition->endMultiFrameCommand();
      }
    }
  }
}

void QgsComposerAttributeTableWidget::on_mHeaderHAlignmentComboBox_currentIndexChanged( int index )
{
  if ( !mComposerTable )
  {
    return;
  }

  QgsComposition* composition = mComposerTable->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mComposerTable, tr( "Table header alignment changed" ) );
  }
  mComposerTable->setHeaderHAlignment(( QgsComposerTableV2::HeaderHAlignment )index );
  if ( composition )
  {
    composition->endMultiFrameCommand();
  }
}

void QgsComposerAttributeTableWidget::on_mHeaderModeComboBox_currentIndexChanged( int index )
{
  if ( !mComposerTable )
  {
    return;
  }

  QgsComposition* composition = mComposerTable->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mComposerTable, tr( "Table header mode changed" ) );
  }
  mComposerTable->setHeaderMode(( QgsComposerTableV2::HeaderMode )index );
  if ( composition )
  {
    composition->endMultiFrameCommand();
  }
}

void QgsComposerAttributeTableWidget::changeLayer( QgsMapLayer *layer )
{
  if ( !mComposerTable )
  {
    return;
  }

  QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( layer );
  if ( !vl )
  {
    return;
  }

  QgsComposition* composition = mComposerTable->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mComposerTable, tr( "Table layer changed" ) );
  }
  mComposerTable->setVectorLayer( vl );
  mComposerTable->update();
  if ( composition )
  {
    composition->endMultiFrameCommand();
  }

  if ( vl->geometryType() == QGis::NoGeometry )
  {
    //layer has no geometry, so uncheck & disable controls which require geometry
    mShowOnlyVisibleFeaturesCheckBox->setChecked( false );
    mShowOnlyVisibleFeaturesCheckBox->setEnabled( false );
  }
  else
  {
    mShowOnlyVisibleFeaturesCheckBox->setEnabled( true );
  }
}

void QgsComposerAttributeTableWidget::on_mAddFramePushButton_clicked()
{
  if ( !mComposerTable || !mFrame )
  {
    return;
  }

  //create a new frame based on the current frame
  QPointF pos = mFrame->pos();
  //shift new frame so that it sits 10 units below current frame
  pos.ry() += mFrame->rect().height() + 10;

  QgsComposerFrame * newFrame = mComposerTable->createNewFrame( mFrame, pos, mFrame->rect().size() );
  mComposerTable->recalculateFrameSizes();

  //set new frame as selection
  QgsComposition* composition = mComposerTable->composition();
  if ( composition )
  {
    composition->setSelectedItem( newFrame );
  }
}

void QgsComposerAttributeTableWidget::on_mResizeModeComboBox_currentIndexChanged( int index )
{
  if ( !mComposerTable )
  {
    return;
  }

  QgsComposition* composition = mComposerTable->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mComposerTable, tr( "Change resize mode" ) );
    mComposerTable->setResizeMode(( QgsComposerMultiFrame::ResizeMode )mResizeModeComboBox->itemData( index ).toInt() );
    composition->endMultiFrameCommand();
  }

  mAddFramePushButton->setEnabled( mComposerTable->resizeMode() == QgsComposerMultiFrame::UseExistingFrames );
}

void QgsComposerAttributeTableWidget::on_mSourceComboBox_currentIndexChanged( int index )
{
  if ( !mComposerTable )
  {
    return;
  }

  QgsComposition* composition = mComposerTable->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mComposerTable, tr( "Change table source" ) );
    mComposerTable->setSource(( QgsComposerAttributeTableV2::ContentSource )mSourceComboBox->itemData( index ).toInt() );
    composition->endMultiFrameCommand();
  }

  toggleSourceControls();
}

void QgsComposerAttributeTableWidget::toggleSourceControls()
{
  switch ( mComposerTable->source() )
  {
    case QgsComposerAttributeTableV2::LayerAttributes:
      mLayerComboBox->setEnabled( true );
      mMaximumRowsSpinBox->setEnabled( true );
      mMaxNumFeaturesLabel->setEnabled( true );
      mShowOnlyVisibleFeaturesCheckBox->setEnabled( true );
      mComposerMapComboBox->setEnabled( mComposerTable->displayOnlyVisibleFeatures() );
      mComposerMapLabel->setEnabled( mComposerTable->displayOnlyVisibleFeatures() );
      break;
    case QgsComposerAttributeTableV2::AtlasFeature:
      mLayerComboBox->setEnabled( false );
      mMaximumRowsSpinBox->setEnabled( false );
      mMaxNumFeaturesLabel->setEnabled( false );
      mShowOnlyVisibleFeaturesCheckBox->setEnabled( false );
      mComposerMapComboBox->setEnabled( false );
      mComposerMapLabel->setEnabled( false );
      break;
    default:
      break;
  }
}
