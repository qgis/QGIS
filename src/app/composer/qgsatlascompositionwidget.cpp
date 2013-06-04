/***************************************************************************
                              qgsatlascompositionwidget.cpp
                              -----------------------------
    begin                : October 2012
    copyright            : (C) 2012 Hugo Mercier
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsatlascompositionwidget.h"
#include "qgsatlascomposition.h"
#include "qgscomposition.h"
#include "qgsmaplayerregistry.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgscomposermap.h"

QgsAtlasCompositionWidget::QgsAtlasCompositionWidget( QWidget* parent, QgsComposition* c ):
    QWidget( parent ), mComposition( c )
{
  setupUi( this );

  // populate the layer list
  mAtlasCoverageLayerComboBox->clear();
  const QMap< QString, QgsMapLayer * >& layers = QgsMapLayerRegistry::instance()->mapLayers();
  int idx = 0;
  for ( QMap<QString, QgsMapLayer*>::const_iterator it = layers.begin(); it != layers.end(); ++it )
  {
    // Only consider vector layers
    if ( dynamic_cast<QgsVectorLayer*>( it.value() ) )
    {
      mAtlasCoverageLayerComboBox->insertItem( idx++, it.value()->name(), /* userdata */ qVariantFromValue(( void* )it.value() ) );
    }
  }
  // update sort columns
  fillSortColumns();

  // Connect to addition / removal of layers
  QgsMapLayerRegistry* layerRegistry = QgsMapLayerRegistry::instance();
  if ( layerRegistry )
  {
    connect( layerRegistry, SIGNAL( layerWillBeRemoved( QString ) ), this, SLOT( onLayerRemoved( QString ) ) );
    connect( layerRegistry, SIGNAL( layerWasAdded( QgsMapLayer* ) ), this, SLOT( onLayerAdded( QgsMapLayer* ) ) );
  }

  // update the composer map combo box
  // populate the map list
  mComposerMapComboBox->clear();
  QList<const QgsComposerMap*> availableMaps = mComposition->composerMapItems();
  QList<const QgsComposerMap*>::const_iterator mapItemIt = availableMaps.constBegin();
  for ( ; mapItemIt != availableMaps.constEnd(); ++mapItemIt )
  {
    mComposerMapComboBox->addItem( tr( "Map %1" ).arg(( *mapItemIt )->id() ), qVariantFromValue(( void* )*mapItemIt ) );
  }

  // Sort direction
  mAtlasSortFeatureDirectionButton->setEnabled( false );

  mAtlasSortFeatureKeyComboBox->setEnabled( false );

  // Connect to addition / removal of maps
  connect( mComposition, SIGNAL( composerMapAdded( QgsComposerMap* ) ), this, SLOT( onComposerMapAdded( QgsComposerMap* ) ) );
  connect( mComposition, SIGNAL( itemRemoved( QgsComposerItem* ) ), this, SLOT( onItemRemoved( QgsComposerItem* ) ) );

  connect( mAtlasMarginRadio, SIGNAL( toggled( bool ) ), mAtlasMarginSpinBox, SLOT( setEnabled( bool ) ) );

  // connect to updates
  connect( &mComposition->atlasComposition(), SIGNAL( parameterChanged() ), this, SLOT( updateGuiElements() ) );

  updateGuiElements();
}

QgsAtlasCompositionWidget::~QgsAtlasCompositionWidget()
{
}

void QgsAtlasCompositionWidget::on_mUseAtlasCheckBox_stateChanged( int state )
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( state == Qt::Checked )
  {
    atlasMap->setEnabled( true );
    mConfigurationGroup->setEnabled( true );
    mVisibilityGroup->setEnabled( true );
    mSortingGroup->setEnabled( true );
    mFilteringGroup->setEnabled( true );
    mScalingGroup->setEnabled( true );
    mOutputGroup->setEnabled( true );
  }
  else
  {
    atlasMap->setEnabled( false );
    mConfigurationGroup->setEnabled( false );
    mVisibilityGroup->setEnabled( false );
    mSortingGroup->setEnabled( false );
    mFilteringGroup->setEnabled( false );
    mScalingGroup->setEnabled( false );
    mOutputGroup->setEnabled( false );
  }
}

void QgsAtlasCompositionWidget::onLayerRemoved( QString layerName )
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  // update the atlas coverage layer combo box
  for ( int i = 0; i < mAtlasCoverageLayerComboBox->count(); ++i )
  {
    const QgsMapLayer* layer = reinterpret_cast<const QgsMapLayer*>( mAtlasCoverageLayerComboBox->itemData( i ).value<void*>() );
    if ( layer->id() == layerName )
    {
      mAtlasCoverageLayerComboBox->removeItem( i );
      break;
    }
  }
  if ( mAtlasCoverageLayerComboBox->count() == 0 )
  {
    atlasMap->setCoverageLayer( 0 );
  }
}

void QgsAtlasCompositionWidget::onLayerAdded( QgsMapLayer* map )
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  // update the atlas coverage layer combo box
  QgsVectorLayer* vectorLayer = dynamic_cast<QgsVectorLayer*>( map );
  if ( vectorLayer )
  {
    mAtlasCoverageLayerComboBox->addItem( map->name(), qVariantFromValue(( void* )map ) );

    if ( mAtlasCoverageLayerComboBox->count() == 1 )
    {
      atlasMap->setCoverageLayer( vectorLayer );
      checkLayerType( vectorLayer );
    }
  }
}

void QgsAtlasCompositionWidget::onComposerMapAdded( QgsComposerMap* map )
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  mComposerMapComboBox->addItem( tr( "Map %1" ).arg( map->id() ), qVariantFromValue(( void* )map ) );
  if ( mComposerMapComboBox->count() == 1 )
  {
    atlasMap->setComposerMap( map );
  }
}

void QgsAtlasCompositionWidget::onItemRemoved( QgsComposerItem* item )
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  QgsComposerMap* map = dynamic_cast<QgsComposerMap*>( item );
  if ( map )
  {
    int idx = mComposerMapComboBox->findData( qVariantFromValue(( void* )map ) );
    if ( idx != -1 )
    {
      mComposerMapComboBox->removeItem( idx );
    }
  }
  if ( mComposerMapComboBox->count() == 0 )
  {
    atlasMap->setComposerMap( 0 );
  }
}

void QgsAtlasCompositionWidget::on_mAtlasCoverageLayerComboBox_currentIndexChanged( int index )
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }
  if ( index == -1 )
  {
    atlasMap->setCoverageLayer( 0 );

    // clean up the sorting columns
    mAtlasSortFeatureKeyComboBox->clear();
  }
  else
  {
    QgsVectorLayer* layer = reinterpret_cast<QgsVectorLayer*>( mAtlasCoverageLayerComboBox->itemData( index ).value<void*>() );

    if ( layer )
    {
      checkLayerType( layer );
      atlasMap->setCoverageLayer( layer );
    }

    // update sorting columns
    fillSortColumns();
  }
}

void QgsAtlasCompositionWidget::checkLayerType( QgsVectorLayer *layer )
{
  // enable or disable fixed scale control based on layer type
  if ( !layer ) return;
  switch ( layer->wkbType() )
  {
    case QGis::WKBPoint:
    case QGis::WKBPoint25D:
    case QGis::WKBMultiPoint:
    case QGis::WKBMultiPoint25D:
      //For point layers buffer setting makes no sense, so set "fixed scale" on and disable margin control
      mAtlasFixedScaleRadio->setChecked( true );
      mAtlasMarginRadio->setEnabled( false );
      break;
    default:
      //Not a point layer, so enable changes to fixed scale control
      mAtlasMarginRadio->setEnabled( true );
  }
}

void QgsAtlasCompositionWidget::on_mComposerMapComboBox_currentIndexChanged( int index )
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }
  if ( index == -1 )
  {
    atlasMap->setComposerMap( 0 );
  }
  else
  {
    QgsComposerMap* map = reinterpret_cast<QgsComposerMap*>( mComposerMapComboBox->itemData( index ).value<void*>() );
    atlasMap->setComposerMap( map );
  }
}

void QgsAtlasCompositionWidget::on_mAtlasFilenamePatternEdit_textChanged( const QString& text )
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }

  atlasMap->setFilenamePattern( text );
}

void QgsAtlasCompositionWidget::on_mAtlasFilenameExpressionButton_clicked()
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap || !atlasMap->coverageLayer() )
  {
    return;
  }

  QgsExpressionBuilderDialog exprDlg( atlasMap->coverageLayer(), mAtlasFilenamePatternEdit->text(), this );
  exprDlg.setWindowTitle( tr( "Expression based filename" ) );
  if ( exprDlg.exec() == QDialog::Accepted )
  {
    QString expression =  exprDlg.expressionText();
    if ( !expression.isEmpty() )
    {
      // will emit a textChanged signal
      mAtlasFilenamePatternEdit->setText( expression );
    }
  }
}

void QgsAtlasCompositionWidget::on_mAtlasHideCoverageCheckBox_stateChanged( int state )
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }
  atlasMap->setHideCoverage( state == Qt::Checked );
}

void QgsAtlasCompositionWidget::on_mAtlasFixedScaleRadio_toggled( bool checked )
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }
  atlasMap->setFixedScale( checked );
}

void QgsAtlasCompositionWidget::on_mAtlasMarginSpinBox_valueChanged( int value )
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  atlasMap->setMargin( value / 100. );
}

void QgsAtlasCompositionWidget::on_mAtlasSingleFileCheckBox_stateChanged( int state )
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }
  if ( state == Qt::Checked )
  {
    mAtlasFilenamePatternEdit->setEnabled( false );
    mAtlasFilenameExpressionButton->setEnabled( false );
  }
  else
  {
    mAtlasFilenamePatternEdit->setEnabled( true );
    mAtlasFilenameExpressionButton->setEnabled( true );
  }
  atlasMap->setSingleFile( state == Qt::Checked );
}

void QgsAtlasCompositionWidget::on_mAtlasSortFeatureCheckBox_stateChanged( int state )
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }

  if ( state == Qt::Checked )
  {
    mAtlasSortFeatureDirectionButton->setEnabled( true );
    mAtlasSortFeatureKeyComboBox->setEnabled( true );
  }
  else
  {
    mAtlasSortFeatureDirectionButton->setEnabled( false );
    mAtlasSortFeatureKeyComboBox->setEnabled( false );
  }
  atlasMap->setSortFeatures( state == Qt::Checked );
}

void QgsAtlasCompositionWidget::on_mAtlasSortFeatureKeyComboBox_currentIndexChanged( int index )
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }

  if ( index != -1 )
  {
    atlasMap->setSortKeyAttributeIndex( index );
  }
}

void QgsAtlasCompositionWidget::on_mAtlasFeatureFilterCheckBox_stateChanged( int state )
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }

  if ( state == Qt::Checked )
  {
    mAtlasFeatureFilterEdit->setEnabled( true );
    mAtlasFeatureFilterButton->setEnabled( true );
  }
  else
  {
    mAtlasFeatureFilterEdit->setEnabled( false );
    mAtlasFeatureFilterButton->setEnabled( false );
  }
  atlasMap->setFilterFeatures( state == Qt::Checked );
}

void QgsAtlasCompositionWidget::on_mAtlasFeatureFilterEdit_textChanged( const QString& text )
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }

  atlasMap->setFeatureFilter( text );
}

void QgsAtlasCompositionWidget::on_mAtlasFeatureFilterButton_clicked()
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap || !atlasMap->coverageLayer() )
  {
    return;
  }

  QgsExpressionBuilderDialog exprDlg( atlasMap->coverageLayer(), mAtlasFeatureFilterEdit->text(), this );
  exprDlg.setWindowTitle( tr( "Expression based filter" ) );
  if ( exprDlg.exec() == QDialog::Accepted )
  {
    QString expression =  exprDlg.expressionText();
    if ( !expression.isEmpty() )
    {
      // will emit a textChanged signal
      mAtlasFeatureFilterEdit->setText( expression );
    }
  }
}

void QgsAtlasCompositionWidget::on_mAtlasSortFeatureDirectionButton_clicked()
{
  Qt::ArrowType at = mAtlasSortFeatureDirectionButton->arrowType();
  at = ( at == Qt::UpArrow ) ? Qt::DownArrow : Qt::UpArrow;
  mAtlasSortFeatureDirectionButton->setArrowType( at );

  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }

  atlasMap->setSortAscending( at == Qt::UpArrow );
}

void QgsAtlasCompositionWidget::fillSortColumns()
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap || !atlasMap->coverageLayer() )
  {
    return;
  }

  mAtlasSortFeatureKeyComboBox->clear();
  // Get fields of the selected coverage layer
  const QgsFields& fields = atlasMap->coverageLayer()->pendingFields();
  for ( int i = 0; i < fields.count(); ++i )
  {
    mAtlasSortFeatureKeyComboBox->insertItem( i, fields.at( i ).name() );
  }
}

void QgsAtlasCompositionWidget::updateGuiElements()
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( atlasMap->enabled() )
  {
    mUseAtlasCheckBox->setCheckState( Qt::Checked );
  }
  else
  {
    mUseAtlasCheckBox->setCheckState( Qt::Unchecked );
  }

  int idx = mAtlasCoverageLayerComboBox->findData( qVariantFromValue(( void* )atlasMap->coverageLayer() ) );
  if ( idx != -1 )
  {
    mAtlasCoverageLayerComboBox->setCurrentIndex( idx );
    checkLayerType( atlasMap->coverageLayer() );
  }
  idx = mComposerMapComboBox->findData( qVariantFromValue(( void* )atlasMap->composerMap() ) );
  if ( idx != -1 )
  {
    mComposerMapComboBox->setCurrentIndex( idx );
  }

  mAtlasMarginSpinBox->setValue( static_cast<int>( atlasMap->margin() * 100 ) );
  mAtlasFilenamePatternEdit->setText( atlasMap->filenamePattern() );
  if ( atlasMap->fixedScale() )
  {
    mAtlasFixedScaleRadio->setChecked( true );
    mAtlasMarginSpinBox->setEnabled( false );
  }
  else
  {
    mAtlasMarginRadio->setChecked( true );
    mAtlasMarginSpinBox->setEnabled( true );
  }
  mAtlasHideCoverageCheckBox->setCheckState( atlasMap->hideCoverage() ? Qt::Checked : Qt::Unchecked );
  mAtlasSingleFileCheckBox->setCheckState( atlasMap->singleFile() ? Qt::Checked : Qt::Unchecked );
  mAtlasSortFeatureCheckBox->setCheckState( atlasMap->sortFeatures() ? Qt::Checked : Qt::Unchecked );
  mAtlasSortFeatureKeyComboBox->setCurrentIndex( atlasMap->sortKeyAttributeIndex() );
  mAtlasSortFeatureDirectionButton->setArrowType( atlasMap->sortAscending() ? Qt::UpArrow : Qt::DownArrow );
  mAtlasFeatureFilterEdit->setText( atlasMap->featureFilter() );
  mAtlasFeatureFilterCheckBox->setCheckState( atlasMap->filterFeatures() ? Qt::Checked : Qt::Unchecked );
}

void QgsAtlasCompositionWidget::blockAllSignals( bool b )
{
  mUseAtlasCheckBox->blockSignals( b );
  mConfigurationGroup->blockSignals( b );
  mVisibilityGroup->blockSignals( b );
  mSortingGroup->blockSignals( b );
  mFilteringGroup->blockSignals( b );
  mScalingGroup->blockSignals( b );
  mOutputGroup->blockSignals( b );
}
