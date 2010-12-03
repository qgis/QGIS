/***************************************************************************
                         qgscomposertablewidget.cpp
                         --------------------------
    begin                : January 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposertablewidget.h"
#include "qgsattributeselectiondialog.h"
#include "qgscomposeritemwidget.h"
#include "qgscomposerattributetable.h"
#include "qgscomposermap.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"
#include <QColorDialog>
#include <QFontDialog>

QgsComposerTableWidget::QgsComposerTableWidget( QgsComposerAttributeTable* table ): QWidget( 0 ), mComposerTable( table )
{
  setupUi( this );
  //add widget for general composer item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, mComposerTable );
  mToolBox->addItem( itemPropertiesWidget, tr( "General options" ) );

  blockAllSignals( true );

  //insert vector layers into combo
  QMap<QString, QgsMapLayer*> layerMap =  QgsMapLayerRegistry::instance()->mapLayers();
  QMap<QString, QgsMapLayer*>::const_iterator mapIt = layerMap.constBegin();

  for ( ; mapIt != layerMap.constEnd(); ++mapIt )
  {
    QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( mapIt.value() );
    if ( vl )
    {
      mLayerComboBox->addItem( vl->name(), mapIt.key() );
    }
  }

  //insert composer maps into combo
  mLayerComboBox->blockSignals( true );
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
  mLayerComboBox->blockSignals( false );

  updateGuiElements();
  on_mComposerMapComboBox_activated( mComposerMapComboBox->currentIndex() );

  if ( mComposerTable )
  {
    QObject::connect( mComposerTable, SIGNAL( maximumNumerOfFeaturesChanged( int ) ), this, SLOT( setMaximumNumberOfFeatures( int ) ) );
    QObject::connect( mComposerTable, SIGNAL( itemChanged ), this, SLOT( updateGuiElements() ) );
  }
}

QgsComposerTableWidget::~QgsComposerTableWidget()
{

}

void QgsComposerTableWidget::on_mLayerComboBox_currentIndexChanged( int index )
{
  if ( !mComposerTable )
  {
    return;
  }

  //set new layer to table item
  QVariant itemData = mLayerComboBox->itemData( index );
  if ( itemData.type() == QVariant::Invalid )
  {
    return;
  }

  QString layerId = itemData.toString();
  QgsMapLayer* ml = QgsMapLayerRegistry::instance()->mapLayer( layerId );
  if ( ml )
  {
    QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( ml );
    if ( vl )
    {
      mComposerTable->beginCommand( tr( "Table layer changed" ) );
      mComposerTable->setVectorLayer( vl );
      mComposerTable->update();
      mComposerTable->endCommand();
    }
  }
}

void QgsComposerTableWidget::on_mAttributesPushButton_clicked()
{
  if ( !mComposerTable )
  {
    return;
  }

  QgsAttributeSelectionDialog d( mComposerTable->vectorLayer(), mComposerTable->displayAttributes(), mComposerTable->fieldAliasMap(), mComposerTable->sortAttributes(), 0 );
  if ( d.exec() == QDialog::Accepted )
  {
    //change displayAttributes and aliases
    mComposerTable->beginCommand( tr( "Table attribute settings" ) );
    mComposerTable->setDisplayAttributes( d.enabledAttributes() );
    mComposerTable->setFieldAliasMap( d.aliasMap() );
    mComposerTable->setSortAttributes( d.attributeSorting() );
    mComposerTable->update();
    mComposerTable->endCommand();
  }
}

void QgsComposerTableWidget::on_mComposerMapComboBox_activated( int index )
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
    if ( sender() ) //only create command if called from GUI
    {
      mComposerTable->beginCommand( tr( "Table map changed" ) );
    }
    mComposerTable->setComposerMap( tableComposition->getComposerMapById( mapId ) );
    mComposerTable->update();
    if ( sender() )
    {
      mComposerTable->endCommand();
    }
  }
}

void QgsComposerTableWidget::on_mMaximumColumnsSpinBox_valueChanged( int i )
{
  if ( !mComposerTable )
  {
    return;
  }

  mComposerTable->beginCommand( tr( "Table maximum columns" ), QgsComposerMergeCommand::TableMaximumFeatures );
  mComposerTable->setMaximumNumberOfFeatures( i );
  mComposerTable->update();
  mComposerTable->endCommand();
}

void QgsComposerTableWidget::on_mMarginSpinBox_valueChanged( double d )
{
  if ( !mComposerTable )
  {
    return;
  }

  mComposerTable->beginCommand( tr( "Table maximum columns" ), QgsComposerMergeCommand::TableMargin );
  mComposerTable->setLineTextDistance( d );
  mComposerTable->update();
  mComposerTable->endCommand();
}

void QgsComposerTableWidget::on_mHeaderFontPushButton_clicked()
{
  if ( !mComposerTable )
  {
    return;
  }

  bool ok;
  QFont newFont = QFontDialog::getFont( &ok, mComposerTable->headerFont(), 0, tr( "Select Font" ) );
  if ( ok )
  {
    mComposerTable->beginCommand( tr( "Table header font" ) );
    mComposerTable->setHeaderFont( newFont );
    mComposerTable->endCommand();
  }
}

void QgsComposerTableWidget::on_mContentFontPushButton_clicked()
{
  if ( !mComposerTable )
  {
    return;
  }

  bool ok;
  QFont newFont = QFontDialog::getFont( &ok, mComposerTable->contentFont(), 0, tr( "Select Font" ) );
  if ( ok )
  {
    mComposerTable->beginCommand( tr( "Table content font" ) );
    mComposerTable->setContentFont( newFont );
    mComposerTable->endCommand();
  }
}

void QgsComposerTableWidget::on_mGridStrokeWidthSpinBox_valueChanged( double d )
{
  if ( !mComposerTable )
  {
    return;
  }
  mComposerTable->beginCommand( tr( "Table grid stroke" ), QgsComposerMergeCommand::TableGridStrokeWidth );
  mComposerTable->setGridStrokeWidth( d );
  mComposerTable->update();
  mComposerTable->endCommand();
}

void QgsComposerTableWidget::on_mGridColorButton_clicked()
{
  if ( !mComposerTable )
  {
    return;
  }

#if QT_VERSION >= 0x040500
  QColor newColor = QColorDialog::getColor( mComposerTable->gridColor(), 0, tr( "Select grid color" ) );
#else
  QColor newColor = QColorDialog::getColor( mComposerTable->gridColor(), 0 );
#endif
  if ( !newColor.isValid() )
  {
    return;
  }
  mComposerTable->beginCommand( tr( "Table grid color" ) );
  mGridColorButton->setColor( newColor );
  mComposerTable->setGridColor( newColor );
  mComposerTable->update();
  mComposerTable->endCommand();
}

void QgsComposerTableWidget::on_mShowGridCheckBox_stateChanged( int state )
{
  if ( !mComposerTable )
  {
    return;
  }

  bool showGrid = false;
  if ( state == Qt::Checked )
  {
    showGrid = true;
  }
  mComposerTable->beginCommand( tr( "Table grid toggled" ) );
  mComposerTable->setShowGrid( showGrid );
  mComposerTable->update();
  mComposerTable->endCommand();
}


void QgsComposerTableWidget::updateGuiElements()
{
  if ( !mComposerTable )
  {
    return;
  }

  blockAllSignals( true );

  //layer combo box
  const QgsVectorLayer* vl = mComposerTable->vectorLayer();
  if ( vl )
  {
    int layerIndex = mLayerComboBox->findText( vl->name() );
    if ( layerIndex != -1 )
    {
      mLayerComboBox->setCurrentIndex( layerIndex );
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
  mMaximumColumnsSpinBox->setValue( mComposerTable->maximumNumberOfFeatures() );
  mMarginSpinBox->setValue( mComposerTable->lineTextDistance() );
  mGridStrokeWidthSpinBox->setValue( mComposerTable->gridStrokeWidth() );
  mGridColorButton->setColor( mComposerTable->gridColor() );
  if ( mComposerTable->showGrid() )
  {
    mShowGridCheckBox->setCheckState( Qt::Checked );
  }
  else
  {
    mShowGridCheckBox->setCheckState( Qt::Unchecked );
  }

  if ( mComposerTable->displayOnlyVisibleFeatures() )
  {
    mShowOnlyVisibleFeaturesCheckBox->setCheckState( Qt::Checked );
  }
  else
  {
    mShowOnlyVisibleFeaturesCheckBox->setCheckState( Qt::Unchecked );
  }
  blockAllSignals( false );
}

void QgsComposerTableWidget::blockAllSignals( bool b )
{
  mLayerComboBox->blockSignals( b );
  mComposerMapComboBox->blockSignals( b );
  mMaximumColumnsSpinBox->blockSignals( b );
  mMarginSpinBox->blockSignals( b );
  mGridColorButton->blockSignals( b );
  mGridStrokeWidthSpinBox->blockSignals( b );
  mShowGridCheckBox->blockSignals( b );
  mShowOnlyVisibleFeaturesCheckBox->blockSignals( b );
}

void QgsComposerTableWidget::setMaximumNumberOfFeatures( int n )
{
  mMaximumColumnsSpinBox->blockSignals( true );
  mMaximumColumnsSpinBox->setValue( n );
  mMaximumColumnsSpinBox->blockSignals( false );
}

void QgsComposerTableWidget::on_mShowOnlyVisibleFeaturesCheckBox_stateChanged( int state )
{
  if ( !mComposerTable )
  {
    return;
  }

  mComposerTable->beginCommand( tr( "Table visible only toggled" ) );
  bool showOnlyVisibleFeatures = ( state == Qt::Checked );
  mComposerTable->setDisplayOnlyVisibleFeatures( showOnlyVisibleFeatures );
  mComposerTable->update();
  mComposerTable->endCommand();
}


