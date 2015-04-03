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
#include "qgscomposertablecolumn.h"
#include "qgscomposermap.h"
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgisgui.h"

QgsComposerTableWidget::QgsComposerTableWidget( QgsComposerAttributeTable* table ): QgsComposerItemBaseWidget( 0, table ), mComposerTable( table )
{
  setupUi( this );
  //add widget for general composer item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, mComposerTable );
  mainLayout->addWidget( itemPropertiesWidget );

  blockAllSignals( true );
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
    QObject::connect( mComposerTable, SIGNAL( itemChanged() ), this, SLOT( updateGuiElements() ) );
  }
}

QgsComposerTableWidget::~QgsComposerTableWidget()
{

}

void QgsComposerTableWidget::showEvent( QShowEvent* /* event */ )
{
  refreshMapComboBox();
}

void QgsComposerTableWidget::refreshMapComboBox()
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

void QgsComposerTableWidget::on_mRefreshPushButton_clicked()
{
  if ( !mComposerTable )
  {
    return;
  }

  mComposerTable->refreshAttributes();
}

void QgsComposerTableWidget::on_mAttributesPushButton_clicked()
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

  mComposerTable->beginCommand( tr( "Table attribute settings" ) );

  QgsAttributeSelectionDialog d( mComposerTable, mComposerTable->vectorLayer(), this );
  if ( d.exec() == QDialog::Accepted )
  {
    mComposerTable->refreshAttributes();
    mComposerTable->update();
    mComposerTable->endCommand();

    //clear currentColumns to free memory
    qDeleteAll( currentColumns );
    currentColumns.clear();
  }
  else
  {
    //undo changes
    mComposerTable->setColumns( currentColumns );
    mComposerTable->cancelCommand();
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
  QFont newFont = QgisGui::getFont( ok, mComposerTable->headerFont(), tr( "Select Font" ) );
  if ( ok )
  {
    mComposerTable->beginCommand( tr( "Table header font" ) );
    mComposerTable->setHeaderFont( newFont );
    mComposerTable->endCommand();
  }
}

void QgsComposerTableWidget::on_mHeaderFontColorButton_colorChanged( const QColor &newColor )
{
  if ( !mComposerTable )
  {
    return;
  }

  mComposerTable->beginCommand( tr( "Table header font color" ) );
  mComposerTable->setHeaderFontColor( newColor );
  mComposerTable->update();
  mComposerTable->endCommand();
}

void QgsComposerTableWidget::on_mContentFontPushButton_clicked()
{
  if ( !mComposerTable )
  {
    return;
  }

  bool ok;
  QFont newFont = QgisGui::getFont( ok, mComposerTable->contentFont(), tr( "Select Font" ) );
  if ( ok )
  {
    mComposerTable->beginCommand( tr( "Table content font" ) );
    mComposerTable->setContentFont( newFont );
    mComposerTable->endCommand();
  }
}

void QgsComposerTableWidget::on_mContentFontColorButton_colorChanged( const QColor &newColor )
{
  if ( !mComposerTable )
  {
    return;
  }

  mComposerTable->beginCommand( tr( "Table content font color" ) );
  mComposerTable->setContentFontColor( newColor );
  mComposerTable->update();
  mComposerTable->endCommand();
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

void QgsComposerTableWidget::on_mGridColorButton_colorChanged( const QColor& newColor )
{
  if ( !mComposerTable )
  {
    return;
  }

  mComposerTable->beginCommand( tr( "Table grid color" ) );
  mComposerTable->setGridColor( newColor );
  mComposerTable->update();
  mComposerTable->endCommand();
}

void QgsComposerTableWidget::on_mShowGridGroupCheckBox_toggled( bool state )
{
  if ( !mComposerTable )
  {
    return;
  }

  mComposerTable->beginCommand( tr( "Table grid toggled" ) );
  mComposerTable->setShowGrid( state );
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
  mMaximumColumnsSpinBox->setValue( mComposerTable->maximumNumberOfFeatures() );
  mMarginSpinBox->setValue( mComposerTable->lineTextDistance() );
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
  mShowGridGroupCheckBox->blockSignals( b );
  mShowOnlyVisibleFeaturesCheckBox->blockSignals( b );
  mFeatureFilterEdit->blockSignals( b );
  mFeatureFilterCheckBox->blockSignals( b );
  mHeaderHAlignmentComboBox->blockSignals( b );
  mHeaderFontColorButton->blockSignals( b );
  mContentFontColorButton->blockSignals( b );
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

  //enable/disable map combobox based on state of checkbox
  mComposerMapComboBox->setEnabled( state == Qt::Checked );
  mComposerMapLabel->setEnabled( state == Qt::Checked );
}

void QgsComposerTableWidget::on_mFeatureFilterCheckBox_stateChanged( int state )
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
  mComposerTable->beginCommand( tr( "Table feature filter toggled" ) );
  mComposerTable->setFilterFeatures( state == Qt::Checked );
  mComposerTable->update();
  mComposerTable->endCommand();
}

void QgsComposerTableWidget::on_mFeatureFilterEdit_editingFinished()
{
  if ( !mComposerTable )
  {
    return;
  }

  mComposerTable->beginCommand( tr( "Table feature filter modified" ) );
  mComposerTable->setFeatureFilter( mFeatureFilterEdit->text() );
  mComposerTable->update();
  mComposerTable->endCommand();
}

void QgsComposerTableWidget::on_mFeatureFilterButton_clicked()
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
      mComposerTable->beginCommand( tr( "Table feature filter modified" ) );
      mComposerTable->setFeatureFilter( mFeatureFilterEdit->text() );
      mComposerTable->update();
      mComposerTable->endCommand();
    }
  }
}

void QgsComposerTableWidget::on_mHeaderHAlignmentComboBox_currentIndexChanged( int index )
{
  if ( !mComposerTable )
  {
    return;
  }

  mComposerTable->beginCommand( tr( "Table header alignment changed" ) );
  mComposerTable->setHeaderHAlignment(( QgsComposerTable::HeaderHAlignment )index );
  mComposerTable->endCommand();
}

void QgsComposerTableWidget::changeLayer( QgsMapLayer *layer )
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

  mComposerTable->beginCommand( tr( "Table layer changed" ) );
  mComposerTable->setVectorLayer( vl );
  mComposerTable->update();
  mComposerTable->endCommand();

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
