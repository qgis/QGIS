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

#include <QMessageBox>

#include "qgsatlascompositionwidget.h"
#include "qgsatlascomposition.h"
#include "qgscomposition.h"
#include "qgsfieldmodel.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaplayerproxymodel.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgscomposermap.h"

static QgsExpressionContext _getExpressionContext( const void* context )
{
  const QgsComposition* composition = ( const QgsComposition* ) context;
  if ( !composition )
  {
    return QgsExpressionContext();
  }

  QScopedPointer< QgsExpressionContext > expContext( composition->createExpressionContext() );
  return QgsExpressionContext( *expContext );
}

QgsAtlasCompositionWidget::QgsAtlasCompositionWidget( QWidget* parent, QgsComposition* c ):
    QWidget( parent ), mComposition( c )
{
  setupUi( this );

  mAtlasCoverageLayerComboBox->setFilters( QgsMapLayerProxyModel::VectorLayer );

  connect( mAtlasCoverageLayerComboBox, SIGNAL( layerChanged( QgsMapLayer* ) ), mAtlasSortFeatureKeyComboBox, SLOT( setLayer( QgsMapLayer* ) ) );
  connect( mAtlasCoverageLayerComboBox, SIGNAL( layerChanged( QgsMapLayer* ) ), mPageNameWidget, SLOT( setLayer( QgsMapLayer* ) ) );

  // Sort direction
  mAtlasSortFeatureDirectionButton->setEnabled( false );
  mAtlasSortFeatureKeyComboBox->setEnabled( false );

  // connect to updates
  connect( &mComposition->atlasComposition(), SIGNAL( parameterChanged() ), this, SLOT( updateGuiElements() ) );

  mPageNameWidget->registerGetExpressionContextCallback( &_getExpressionContext, mComposition );

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

void QgsAtlasCompositionWidget::on_mAtlasFilenamePatternEdit_editingFinished()
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }

  if ( ! atlasMap->setFilenamePattern( mAtlasFilenamePatternEdit->text() ) )
  {
    //expression could not be set
    QMessageBox::warning( this
                          , tr( "Could not evaluate filename pattern" )
                          , tr( "Could not set filename pattern as '%1'.\nParser error:\n%2" )
                          .arg( mAtlasFilenamePatternEdit->text() )
                          .arg( atlasMap->filenamePatternErrorString() )
                        );
  }
}

void QgsAtlasCompositionWidget::on_mAtlasFilenameExpressionButton_clicked()
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap || !atlasMap->coverageLayer() )
  {
    return;
  }

  QScopedPointer<QgsExpressionContext> context( mComposition->createExpressionContext() );
  QgsExpressionBuilderDialog exprDlg( atlasMap->coverageLayer(), mAtlasFilenamePatternEdit->text(), this, "generic", *context );
  exprDlg.setWindowTitle( tr( "Expression based filename" ) );

  if ( exprDlg.exec() == QDialog::Accepted )
  {
    QString expression =  exprDlg.expressionText();
    if ( !expression.isEmpty() )
    {
      //set atlas filename expression
      mAtlasFilenamePatternEdit->setText( expression );
      if ( ! atlasMap->setFilenamePattern( expression ) )
      {
        //expression could not be set
        QMessageBox::warning( this
                              , tr( "Could not evaluate filename pattern" )
                              , tr( "Could not set filename pattern as '%1'.\nParser error:\n%2" )
                              .arg( expression )
                              .arg( atlasMap->filenamePatternErrorString() )
                            );
      }
    }
  }
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
}

void QgsAtlasCompositionWidget::updateAtlasFeatures()
{
  //only do this if composer mode is preview
  if ( !( mComposition->atlasMode() == QgsComposition::PreviewAtlas ) )
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

    return;
  }
}

void QgsAtlasCompositionWidget::on_mAtlasFeatureFilterCheckBox_stateChanged( int state )
{
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
}

void QgsAtlasCompositionWidget::on_mApplyConfigButton_clicked()
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }

  QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( mAtlasCoverageLayerComboBox->currentLayer() );
  if ( !vl )
  {
    atlasMap->setCoverageLayer( 0 );
  }
  else
  {
    atlasMap->setCoverageLayer( vl );
  }

  atlasMap->setSortFeatures( mAtlasSortFeatureCheckBox->isChecked() );
  atlasMap->setSortKeyAttributeName( mAtlasSortFeatureKeyComboBox->currentField() );
  Qt::ArrowType at = mAtlasSortFeatureDirectionButton->arrowType();
  at = ( at == Qt::UpArrow ) ? Qt::DownArrow : Qt::UpArrow;
  atlasMap->setSortAscending( at == Qt::UpArrow );
  atlasMap->setFilterFeatures( mAtlasFeatureFilterCheckBox->isChecked() );
  atlasMap->setFeatureFilter( mAtlasFeatureFilterEdit->text() );
  atlasMap->setPageNameExpression( mPageNameWidget->currentField() );
  atlasMap->setHideCoverage( mAtlasHideCoverageCheckBox->isChecked() );

  updateAtlasFeatures();
}

void QgsAtlasCompositionWidget::on_mAtlasFeatureFilterButton_clicked()
{
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();
  QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( mAtlasCoverageLayerComboBox->currentLayer() );

  if ( !atlasMap || !vl )
  {
    return;
  }

  QScopedPointer<QgsExpressionContext> context( mComposition->createExpressionContext() );
  QgsExpressionBuilderDialog exprDlg( vl, mAtlasFeatureFilterEdit->text(), this, "generic", *context );
  exprDlg.setWindowTitle( tr( "Expression based filter" ) );

  if ( exprDlg.exec() == QDialog::Accepted )
  {
    QString expression =  exprDlg.expressionText();
    if ( !expression.isEmpty() )
    {
      mAtlasFeatureFilterEdit->setText( expression );
    }
  }
}

void QgsAtlasCompositionWidget::on_mAtlasSortFeatureDirectionButton_clicked()
{
  Qt::ArrowType at = mAtlasSortFeatureDirectionButton->arrowType();
  at = ( at == Qt::UpArrow ) ? Qt::DownArrow : Qt::UpArrow;
  mAtlasSortFeatureDirectionButton->setArrowType( at );
}

void QgsAtlasCompositionWidget::updateGuiElements()
{
  blockAllSignals( true );
  QgsAtlasComposition* atlasMap = &mComposition->atlasComposition();

  mUseAtlasCheckBox->setCheckState( atlasMap->enabled() ? Qt::Checked : Qt::Unchecked );
  mConfigurationGroup->setEnabled( atlasMap->enabled() );
  mOutputGroup->setEnabled( atlasMap->enabled() );

  mAtlasCoverageLayerComboBox->setLayer( atlasMap->coverageLayer() );
  mPageNameWidget->setLayer( atlasMap->coverageLayer() );
  mPageNameWidget->setField( atlasMap->pageNameExpression() );

  mAtlasSortFeatureKeyComboBox->setLayer( atlasMap->coverageLayer() );
  mAtlasSortFeatureKeyComboBox->setField( atlasMap->sortKeyAttributeName() );

  mAtlasFilenamePatternEdit->setText( atlasMap->filenamePattern() );
  mAtlasHideCoverageCheckBox->setCheckState( atlasMap->hideCoverage() ? Qt::Checked : Qt::Unchecked );

  mAtlasSingleFileCheckBox->setCheckState( atlasMap->singleFile() ? Qt::Checked : Qt::Unchecked );
  mAtlasFilenamePatternEdit->setEnabled( !atlasMap->singleFile() );
  mAtlasFilenameExpressionButton->setEnabled( !atlasMap->singleFile() );

  mAtlasSortFeatureCheckBox->setCheckState( atlasMap->sortFeatures() ? Qt::Checked : Qt::Unchecked );
  mAtlasSortFeatureDirectionButton->setEnabled( atlasMap->sortFeatures() );
  mAtlasSortFeatureKeyComboBox->setEnabled( atlasMap->sortFeatures() );

  mAtlasSortFeatureDirectionButton->setArrowType( atlasMap->sortAscending() ? Qt::UpArrow : Qt::DownArrow );
  mAtlasFeatureFilterEdit->setText( atlasMap->featureFilter() );

  mAtlasFeatureFilterCheckBox->setCheckState( atlasMap->filterFeatures() ? Qt::Checked : Qt::Unchecked );
  mAtlasFeatureFilterEdit->setEnabled( atlasMap->filterFeatures() );
  mAtlasFeatureFilterButton->setEnabled( atlasMap->filterFeatures() );

  blockAllSignals( false );
}

void QgsAtlasCompositionWidget::blockAllSignals( bool b )
{
  mUseAtlasCheckBox->blockSignals( b );
  mConfigurationGroup->blockSignals( b );
  mOutputGroup->blockSignals( b );
  mAtlasCoverageLayerComboBox->blockSignals( b );
  mPageNameWidget->blockSignals( b );
  mAtlasSortFeatureKeyComboBox->blockSignals( b );
  mAtlasFilenamePatternEdit->blockSignals( b );
  mAtlasHideCoverageCheckBox->blockSignals( b );
  mAtlasSingleFileCheckBox->blockSignals( b );
  mAtlasSortFeatureCheckBox->blockSignals( b );
  mAtlasSortFeatureDirectionButton->blockSignals( b );
  mAtlasFeatureFilterEdit->blockSignals( b );
  mAtlasFeatureFilterCheckBox->blockSignals( b );
}
