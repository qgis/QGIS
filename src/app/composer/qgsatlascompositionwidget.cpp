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
#include "qgsvectorlayer.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgscomposermap.h"

QgsAtlasCompositionWidget::QgsAtlasCompositionWidget( QWidget* parent, QgsComposition* c ):
    QWidget( parent ), mComposition( c )
{
  setupUi( this );

  // populate the layer list
  mAtlasCoverageLayerComboBox->clear();
  QMap< QString, QgsMapLayer * >& layers = QgsMapLayerRegistry::instance()->mapLayers();
  int idx = 0;
  for ( QMap<QString, QgsMapLayer*>::const_iterator it = layers.begin(); it != layers.end(); ++it )
  {
    // Only consider vector layers
    if ( dynamic_cast<QgsVectorLayer*>( it.value() ) )
    {
      mAtlasCoverageLayerComboBox->insertItem( idx++, it.value()->name(), /* userdata */ qVariantFromValue(( void* )it.value() ) );
    }
  }

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

  // Connect to addition / removal of maps
  connect( mComposition, SIGNAL( composerMapAdded( QgsComposerMap* ) ), this, SLOT( onComposerMapAdded( QgsComposerMap* ) ) );
  connect( mComposition, SIGNAL( itemRemoved( QgsComposerItem* ) ), this, SLOT( onItemRemoved( QgsComposerItem* ) ) );

  // connect to updates
  connect( &mComposition->atlasComposition(), SIGNAL( parameterChanged() ), this, SLOT( updateGuiElements() ) );

  updateGuiElements();
}

QgsAtlasCompositionWidget::~QgsAtlasCompositionWidget()
{
}

void QgsAtlasCompositionWidget::on_mUseAtlasGroupBox_toggled( bool state )
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( state  )
  {
    atlasMap->setEnabled( true );
  }
  else
  {
    atlasMap->setEnabled( false );
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
  }
  if ( mAtlasCoverageLayerComboBox->count() == 1 )
  {
    atlasMap->setCoverageLayer( vectorLayer );
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
  }
  else
  {
    QgsVectorLayer* layer = reinterpret_cast<QgsVectorLayer*>( mAtlasCoverageLayerComboBox->itemData( index ).value<void*>() );
    atlasMap->setCoverageLayer( layer );
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

void QgsAtlasCompositionWidget::on_mAtlasFixedScaleCheckBox_stateChanged( int state )
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }
  atlasMap->setFixedScale( state == Qt::Checked );

  // in fixed scale mode, the margin is meaningless
  if ( state == Qt::Checked )
  {
    mAtlasMarginSpinBox->setEnabled( false );
  }
  else
  {
    mAtlasMarginSpinBox->setEnabled( true );
  }
}

void QgsAtlasCompositionWidget::on_mAtlasSingleFileCheckBox_stateChanged( int state )
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }
  atlasMap->setSingleFile( state == Qt::Checked );
}

void QgsAtlasCompositionWidget::updateGuiElements()
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( atlasMap->enabled() )
  {
    mUseAtlasGroupBox->setChecked( true );
  }
  else
  {
    mUseAtlasGroupBox->setChecked( false );
  }

  int idx = mAtlasCoverageLayerComboBox->findData( qVariantFromValue(( void* )atlasMap->coverageLayer() ) );
  if ( idx != -1 )
  {
    mAtlasCoverageLayerComboBox->setCurrentIndex( idx );
  }
  idx = mComposerMapComboBox->findData( qVariantFromValue(( void* )atlasMap->composerMap() ) );
  if ( idx != -1 )
  {
    mComposerMapComboBox->setCurrentIndex( idx );
  }

  mAtlasMarginSpinBox->setValue( static_cast<int>( atlasMap->margin() * 100 ) );
  mAtlasFilenamePatternEdit->setText( atlasMap->filenamePattern() );
  mAtlasFixedScaleCheckBox->setCheckState( atlasMap->fixedScale() ? Qt::Checked : Qt::Unchecked );
  mAtlasHideCoverageCheckBox->setCheckState( atlasMap->hideCoverage() ? Qt::Checked : Qt::Unchecked );
  mAtlasSingleFileCheckBox->setCheckState( atlasMap->singleFile() ? Qt::Checked : Qt::Unchecked );
}

void QgsAtlasCompositionWidget::blockAllSignals( bool b )
{
  mUseAtlasGroupBox->blockSignals( b );
}
