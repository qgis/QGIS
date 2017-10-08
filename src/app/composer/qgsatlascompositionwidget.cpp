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

#include <QComboBox>
#include <QImageWriter>
#include <QMessageBox>

#include "qgsatlascompositionwidget.h"
#include "qgsatlascomposition.h"
#include "qgscomposition.h"
#include "qgsfieldmodel.h"
#include "qgsmaplayerproxymodel.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgscomposermap.h"
#include "qgsvectorlayer.h"

QgsAtlasCompositionWidget::QgsAtlasCompositionWidget( QWidget *parent, QgsComposition *c ):
  QWidget( parent ), mComposition( c )
{
  setupUi( this );
  connect( mUseAtlasCheckBox, &QCheckBox::stateChanged, this, &QgsAtlasCompositionWidget::mUseAtlasCheckBox_stateChanged );
  connect( mAtlasFilenamePatternEdit, &QLineEdit::editingFinished, this, &QgsAtlasCompositionWidget::mAtlasFilenamePatternEdit_editingFinished );
  connect( mAtlasFilenameExpressionButton, &QToolButton::clicked, this, &QgsAtlasCompositionWidget::mAtlasFilenameExpressionButton_clicked );
  connect( mAtlasHideCoverageCheckBox, &QCheckBox::stateChanged, this, &QgsAtlasCompositionWidget::mAtlasHideCoverageCheckBox_stateChanged );
  connect( mAtlasSingleFileCheckBox, &QCheckBox::stateChanged, this, &QgsAtlasCompositionWidget::mAtlasSingleFileCheckBox_stateChanged );
  connect( mAtlasSortFeatureCheckBox, &QCheckBox::stateChanged, this, &QgsAtlasCompositionWidget::mAtlasSortFeatureCheckBox_stateChanged );
  connect( mAtlasSortFeatureDirectionButton, &QToolButton::clicked, this, &QgsAtlasCompositionWidget::mAtlasSortFeatureDirectionButton_clicked );
  connect( mAtlasFeatureFilterEdit, &QLineEdit::editingFinished, this, &QgsAtlasCompositionWidget::mAtlasFeatureFilterEdit_editingFinished );
  connect( mAtlasFeatureFilterButton, &QToolButton::clicked, this, &QgsAtlasCompositionWidget::mAtlasFeatureFilterButton_clicked );
  connect( mAtlasFeatureFilterCheckBox, &QCheckBox::stateChanged, this, &QgsAtlasCompositionWidget::mAtlasFeatureFilterCheckBox_stateChanged );

  mAtlasCoverageLayerComboBox->setFilters( QgsMapLayerProxyModel::VectorLayer );

  connect( mAtlasCoverageLayerComboBox, &QgsMapLayerComboBox::layerChanged, mAtlasSortFeatureKeyComboBox, &QgsFieldComboBox::setLayer );
  connect( mAtlasCoverageLayerComboBox, &QgsMapLayerComboBox::layerChanged, mPageNameWidget, &QgsFieldExpressionWidget::setLayer );
  connect( mAtlasCoverageLayerComboBox, &QgsMapLayerComboBox::layerChanged, this, &QgsAtlasCompositionWidget::changeCoverageLayer );
  connect( mAtlasSortFeatureKeyComboBox, &QgsFieldComboBox::fieldChanged, this, &QgsAtlasCompositionWidget::changesSortFeatureField );
  connect( mPageNameWidget, static_cast < void ( QgsFieldExpressionWidget::* )( const QString &, bool ) > ( &QgsFieldExpressionWidget::fieldChanged ), this, &QgsAtlasCompositionWidget::pageNameExpressionChanged );

  // Sort direction
  mAtlasSortFeatureDirectionButton->setEnabled( false );
  mAtlasSortFeatureKeyComboBox->setEnabled( false );

  // connect to updates
  connect( &mComposition->atlasComposition(), &QgsAtlasComposition::parameterChanged, this, &QgsAtlasCompositionWidget::updateGuiElements );

  mPageNameWidget->registerExpressionContextGenerator( mComposition );

  QList<QByteArray> formats = QImageWriter::supportedImageFormats();
  for ( int i = 0; i < formats.size(); ++i )
  {
    mAtlasFileFormat->addItem( QString( formats.at( i ) ) );
  }
  connect( mAtlasFileFormat, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [ = ]( int ) { changeFileFormat(); } );

  updateGuiElements();
}

void QgsAtlasCompositionWidget::mUseAtlasCheckBox_stateChanged( int state )
{
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
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

void QgsAtlasCompositionWidget::changeCoverageLayer( QgsMapLayer *layer )
{
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }

  QgsVectorLayer *vl = dynamic_cast<QgsVectorLayer *>( layer );

  if ( !vl )
  {
    atlasMap->setCoverageLayer( nullptr );
  }
  else
  {
    atlasMap->setCoverageLayer( vl );
    updateAtlasFeatures();
  }
}

void QgsAtlasCompositionWidget::mAtlasFilenamePatternEdit_editingFinished()
{
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
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
                          .arg( mAtlasFilenamePatternEdit->text(),
                                atlasMap->filenamePatternErrorString() )
                        );
  }
}

void QgsAtlasCompositionWidget::mAtlasFilenameExpressionButton_clicked()
{
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap || !atlasMap->coverageLayer() )
  {
    return;
  }

  QgsExpressionContext context = mComposition->createExpressionContext();
  QgsExpressionBuilderDialog exprDlg( atlasMap->coverageLayer(), mAtlasFilenamePatternEdit->text(), this, QStringLiteral( "generic" ), context );
  exprDlg.setWindowTitle( tr( "Expression Based Filename" ) );

  if ( exprDlg.exec() == QDialog::Accepted )
  {
    QString expression = exprDlg.expressionText();
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
                              .arg( expression,
                                    atlasMap->filenamePatternErrorString() )
                            );
      }
    }
  }
}

void QgsAtlasCompositionWidget::mAtlasHideCoverageCheckBox_stateChanged( int state )
{
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }
  atlasMap->setHideCoverage( state == Qt::Checked );
}

void QgsAtlasCompositionWidget::mAtlasSingleFileCheckBox_stateChanged( int state )
{
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
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

void QgsAtlasCompositionWidget::mAtlasSortFeatureCheckBox_stateChanged( int state )
{
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
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
  if ( !( mComposition->atlasMode() == QgsComposition::PreviewAtlas ) )
  {
    return;
  }

  //update atlas features
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }

  bool updated = atlasMap->updateFeatures();
  if ( !updated )
  {
    QMessageBox::warning( nullptr, tr( "Atlas preview" ),
                          tr( "No matching atlas features found!" ),
                          QMessageBox::Ok,
                          QMessageBox::Ok );

    //Perhaps atlas preview should be disabled now? If so, it may get annoying if user is editing
    //the filter expression and it keeps disabling itself.
    return;
  }
}

void QgsAtlasCompositionWidget::changesSortFeatureField( const QString &fieldName )
{
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }
  atlasMap->setSortKeyAttributeName( fieldName );
  updateAtlasFeatures();
}

void QgsAtlasCompositionWidget::mAtlasFeatureFilterCheckBox_stateChanged( int state )
{
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
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

void QgsAtlasCompositionWidget::pageNameExpressionChanged( const QString &, bool valid )
{
  QString expression = mPageNameWidget->asExpression();
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap || ( !valid && !expression.isEmpty() ) )
  {
    return;
  }

  atlasMap->setPageNameExpression( expression );
}

void QgsAtlasCompositionWidget::mAtlasFeatureFilterEdit_editingFinished()
{
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }

  atlasMap->setFeatureFilter( mAtlasFeatureFilterEdit->text() );
  updateAtlasFeatures();
}

void QgsAtlasCompositionWidget::mAtlasFeatureFilterButton_clicked()
{
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
  QgsVectorLayer *vl = dynamic_cast<QgsVectorLayer *>( mAtlasCoverageLayerComboBox->currentLayer() );

  if ( !atlasMap || !vl )
  {
    return;
  }

  QgsExpressionContext context = mComposition->createExpressionContext();
  QgsExpressionBuilderDialog exprDlg( vl, mAtlasFeatureFilterEdit->text(), this, QStringLiteral( "generic" ), context );
  exprDlg.setWindowTitle( tr( "Expression Based Filter" ) );

  if ( exprDlg.exec() == QDialog::Accepted )
  {
    QString expression = exprDlg.expressionText();
    if ( !expression.isEmpty() )
    {
      mAtlasFeatureFilterEdit->setText( expression );
      atlasMap->setFeatureFilter( mAtlasFeatureFilterEdit->text() );
      updateAtlasFeatures();
    }
  }
}

void QgsAtlasCompositionWidget::mAtlasSortFeatureDirectionButton_clicked()
{
  Qt::ArrowType at = mAtlasSortFeatureDirectionButton->arrowType();
  at = ( at == Qt::UpArrow ) ? Qt::DownArrow : Qt::UpArrow;
  mAtlasSortFeatureDirectionButton->setArrowType( at );

  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
  if ( !atlasMap )
  {
    return;
  }

  atlasMap->setSortAscending( at == Qt::UpArrow );
  updateAtlasFeatures();
}

void QgsAtlasCompositionWidget::changeFileFormat()
{
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();
  atlasMap->setFileFormat( mAtlasFileFormat->currentText() );
}
void QgsAtlasCompositionWidget::updateGuiElements()
{
  blockAllSignals( true );
  QgsAtlasComposition *atlasMap = &mComposition->atlasComposition();

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

  mAtlasFileFormat->setCurrentIndex( mAtlasFileFormat->findText( atlasMap->fileFormat() ) );

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
