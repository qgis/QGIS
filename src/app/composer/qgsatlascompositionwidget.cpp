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

  // Sort direction
  mAtlasSortFeatureDirectionButton->setEnabled( false );
  mAtlasSortFeatureKeyComboBox->setEnabled( false );

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
    mOutputGroup->setEnabled( true );
  }
  else
  {
    atlasMap->setEnabled( false );
    mConfigurationGroup->setEnabled( false );
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
      updateAtlasFeatures();
    }
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
      atlasMap->setCoverageLayer( layer );
      updateAtlasFeatures();
    }

    // update sorting columns
    fillSortColumns();
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
  updateAtlasFeatures();
}

void QgsAtlasCompositionWidget::updateAtlasFeatures()
{
  //only do this if composer mode is preview
  if ( !mComposition->atlasMode() == QgsComposition::PreviewAtlas )
  {
    return;
  }

  //update atlas features
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }

  bool updated = atlasMap->updateFeatures();
  if ( !updated )
  {
    QMessageBox::warning( 0, tr( "Atlas preview" ),
                          tr( "No matching atlas features found!" ),
                          QMessageBox::Ok,
                          QMessageBox::Ok );

    //Perhaps atlas preview should be disabled now? If so, it may get annoying if user is editing
    //the filter expression and it keeps disabling itself.
    return;
  }
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
  updateAtlasFeatures();
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
  updateAtlasFeatures();
}

void QgsAtlasCompositionWidget::on_mAtlasFeatureFilterEdit_editingFinished()
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }

  atlasMap->setFeatureFilter( mAtlasFeatureFilterEdit->text() );
  updateAtlasFeatures();
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
      mAtlasFeatureFilterEdit->setText( expression );
      atlasMap->setFeatureFilter( mAtlasFeatureFilterEdit->text() );
      updateAtlasFeatures();
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
  updateAtlasFeatures();
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
  }

  mAtlasFilenamePatternEdit->setText( atlasMap->filenamePattern() );
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
  mOutputGroup->blockSignals( b );
}
