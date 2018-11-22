/***************************************************************************
                              qgslayoutatlaswidget.cpp
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

#include "qgslayoutatlaswidget.h"
#include "qgsprintlayout.h"
#include "qgslayoutatlas.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgslayoutundostack.h"
#include "qgsmessagebar.h"

QgsLayoutAtlasWidget::QgsLayoutAtlasWidget( QWidget *parent, QgsPrintLayout *layout )
  : QWidget( parent )
  , mLayout( layout )
  , mAtlas( layout->atlas() )
{
  setupUi( this );
  connect( mUseAtlasCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutAtlasWidget::mUseAtlasCheckBox_stateChanged );
  connect( mAtlasFilenamePatternEdit, &QLineEdit::editingFinished, this, &QgsLayoutAtlasWidget::mAtlasFilenamePatternEdit_editingFinished );
  connect( mAtlasFilenameExpressionButton, &QToolButton::clicked, this, &QgsLayoutAtlasWidget::mAtlasFilenameExpressionButton_clicked );
  connect( mAtlasHideCoverageCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutAtlasWidget::mAtlasHideCoverageCheckBox_stateChanged );
  connect( mAtlasSingleFileCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutAtlasWidget::mAtlasSingleFileCheckBox_stateChanged );
  connect( mAtlasSortFeatureCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutAtlasWidget::mAtlasSortFeatureCheckBox_stateChanged );
  connect( mAtlasSortFeatureDirectionButton, &QToolButton::clicked, this, &QgsLayoutAtlasWidget::mAtlasSortFeatureDirectionButton_clicked );
  connect( mAtlasFeatureFilterEdit, &QLineEdit::editingFinished, this, &QgsLayoutAtlasWidget::mAtlasFeatureFilterEdit_editingFinished );
  connect( mAtlasFeatureFilterButton, &QToolButton::clicked, this, &QgsLayoutAtlasWidget::mAtlasFeatureFilterButton_clicked );
  connect( mAtlasFeatureFilterCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutAtlasWidget::mAtlasFeatureFilterCheckBox_stateChanged );

  mAtlasCoverageLayerComboBox->setFilters( QgsMapLayerProxyModel::VectorLayer );

  connect( mAtlasCoverageLayerComboBox, &QgsMapLayerComboBox::layerChanged, mAtlasSortExpressionWidget, &QgsFieldExpressionWidget::setLayer );
  connect( mAtlasCoverageLayerComboBox, &QgsMapLayerComboBox::layerChanged, mPageNameWidget, &QgsFieldExpressionWidget::setLayer );
  connect( mAtlasCoverageLayerComboBox, &QgsMapLayerComboBox::layerChanged, this, &QgsLayoutAtlasWidget::changeCoverageLayer );
  connect( mAtlasSortExpressionWidget, static_cast < void ( QgsFieldExpressionWidget::* )( const QString &, bool ) > ( &QgsFieldExpressionWidget::fieldChanged ), this, &QgsLayoutAtlasWidget::changesSortFeatureExpression );
  connect( mPageNameWidget, static_cast < void ( QgsFieldExpressionWidget::* )( const QString &, bool ) > ( &QgsFieldExpressionWidget::fieldChanged ), this, &QgsLayoutAtlasWidget::pageNameExpressionChanged );

  // Sort direction
  mAtlasSortFeatureDirectionButton->setEnabled( false );
  mAtlasSortExpressionWidget->setEnabled( false );

  // connect to updates
  connect( mAtlas, &QgsLayoutAtlas::changed, this, &QgsLayoutAtlasWidget::updateGuiElements );

  mPageNameWidget->registerExpressionContextGenerator( mLayout );

  QList<QByteArray> formats = QImageWriter::supportedImageFormats();
  for ( int i = 0; i < formats.size(); ++i )
  {
    mAtlasFileFormat->addItem( QString( formats.at( i ) ) );
  }
  connect( mAtlasFileFormat, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [ = ]( int ) { changeFileFormat(); } );

  updateGuiElements();
}

void QgsLayoutAtlasWidget::setMessageBar( QgsMessageBar *bar )
{
  mMessageBar = bar;
}

void QgsLayoutAtlasWidget::mUseAtlasCheckBox_stateChanged( int state )
{
  if ( state == Qt::Checked )
  {
    mAtlas->setEnabled( true );
    mConfigurationGroup->setEnabled( true );
    mOutputGroup->setEnabled( true );
  }
  else
  {
    mAtlas->setEnabled( false );
    mConfigurationGroup->setEnabled( false );
    mOutputGroup->setEnabled( false );
  }
}

void QgsLayoutAtlasWidget::changeCoverageLayer( QgsMapLayer *layer )
{
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer );

  mLayout->undoStack()->beginCommand( mAtlas, tr( "Change Atlas Layer" ) );
  mLayout->reportContext().setLayer( vl );
  if ( !vl )
  {
    mAtlas->setCoverageLayer( nullptr );
  }
  else
  {
    mAtlas->setCoverageLayer( vl );
    updateAtlasFeatures();
  }
  mLayout->undoStack()->endCommand();
}

void QgsLayoutAtlasWidget::mAtlasFilenamePatternEdit_editingFinished()
{
  QString error;
  mLayout->undoStack()->beginCommand( mAtlas, tr( "Change Atlas Filename" ) );
  if ( !mAtlas->setFilenameExpression( mAtlasFilenamePatternEdit->text(), error ) )
  {
    //expression could not be set
    mMessageBar->pushWarning( tr( "Atlas" ),
                              tr( "Could not set filename expression to '%1'.\nParser error:\n%2" )
                              .arg( mAtlasFilenamePatternEdit->text(),
                                    error ) );
  }
  mLayout->undoStack()->endCommand();
}

void QgsLayoutAtlasWidget::mAtlasFilenameExpressionButton_clicked()
{
  if ( !mAtlas || !mAtlas->coverageLayer() )
  {
    return;
  }

  QgsExpressionContext context = mLayout->createExpressionContext();
  QgsExpressionBuilderDialog exprDlg( mAtlas->coverageLayer(), mAtlasFilenamePatternEdit->text(), this, QStringLiteral( "generic" ), context );
  exprDlg.setWindowTitle( tr( "Expression Based Filename" ) );

  if ( exprDlg.exec() == QDialog::Accepted )
  {
    QString expression = exprDlg.expressionText();
    if ( !expression.isEmpty() )
    {
      //set atlas filename expression
      mAtlasFilenamePatternEdit->setText( expression );
      QString error;
      mLayout->undoStack()->beginCommand( mAtlas, tr( "Change Atlas Filename" ) );
      if ( !mAtlas->setFilenameExpression( expression, error ) )
      {
        //expression could not be set
        mMessageBar->pushWarning( tr( "Atlas" ), tr( "Could not set filename expression to '%1'.\nParser error:\n%2" )
                                  .arg( expression,
                                        error ) );
      }
      mLayout->undoStack()->endCommand();
    }
  }
}

void QgsLayoutAtlasWidget::mAtlasHideCoverageCheckBox_stateChanged( int state )
{
  mLayout->undoStack()->beginCommand( mAtlas, tr( "Toggle Atlas Layer" ) );
  mAtlas->setHideCoverage( state == Qt::Checked );
  mLayout->undoStack()->endCommand();
}

void QgsLayoutAtlasWidget::mAtlasSingleFileCheckBox_stateChanged( int state )
{
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

  mLayout->setCustomProperty( QStringLiteral( "singleFile" ), state == Qt::Checked );
}

void QgsLayoutAtlasWidget::mAtlasSortFeatureCheckBox_stateChanged( int state )
{
  if ( state == Qt::Checked )
  {
    mAtlasSortFeatureDirectionButton->setEnabled( true );
    mAtlasSortExpressionWidget->setEnabled( true );
  }
  else
  {
    mAtlasSortFeatureDirectionButton->setEnabled( false );
    mAtlasSortExpressionWidget->setEnabled( false );
  }
  mLayout->undoStack()->beginCommand( mAtlas, tr( "Toggle Atlas Sorting" ) );
  mAtlas->setSortFeatures( state == Qt::Checked );
  mLayout->undoStack()->endCommand();
  updateAtlasFeatures();
}

void QgsLayoutAtlasWidget::changesSortFeatureExpression( const QString &expression, bool )
{
  mLayout->undoStack()->beginCommand( mAtlas, tr( "Change Atlas Sort" ) );
  mAtlas->setSortExpression( expression );
  mLayout->undoStack()->endCommand();
  updateAtlasFeatures();
}

void QgsLayoutAtlasWidget::updateAtlasFeatures()
{
  bool updated = mAtlas->updateFeatures();
  if ( !updated )
  {
    mMessageBar->pushInfo( tr( "Atlas" ),
                           tr( "No matching atlas features found!" ) );

    //Perhaps atlas preview should be disabled now? If so, it may get annoying if user is editing
    //the filter expression and it keeps disabling itself.
  }
}

void QgsLayoutAtlasWidget::mAtlasFeatureFilterCheckBox_stateChanged( int state )
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
  mLayout->undoStack()->beginCommand( mAtlas, tr( "Change Atlas Filter" ) );
  mAtlas->setFilterFeatures( state == Qt::Checked );
  mLayout->undoStack()->endCommand();
  updateAtlasFeatures();
}

void QgsLayoutAtlasWidget::pageNameExpressionChanged( const QString &, bool valid )
{
  QString expression = mPageNameWidget->asExpression();
  if ( !valid && !expression.isEmpty() )
  {
    return;
  }

  mLayout->undoStack()->beginCommand( mAtlas, tr( "Change Atlas Name" ) );
  mAtlas->setPageNameExpression( expression );
  mLayout->undoStack()->endCommand();
}

void QgsLayoutAtlasWidget::mAtlasFeatureFilterEdit_editingFinished()
{
  QString error;
  mLayout->undoStack()->beginCommand( mAtlas, tr( "Change Atlas Filter" ) );

  if ( !mAtlas->setFilterExpression( mAtlasFeatureFilterEdit->text(), error ) )
  {
    //expression could not be set
    mMessageBar->pushWarning( tr( "Atlas" ), tr( "Could not set filter expression to '%1'.\nParser error:\n%2" )
                              .arg( mAtlasFeatureFilterEdit->text(),
                                    error ) );
  }

  mLayout->undoStack()->endCommand();
  updateAtlasFeatures();
}

void QgsLayoutAtlasWidget::mAtlasFeatureFilterButton_clicked()
{
  QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( mAtlasCoverageLayerComboBox->currentLayer() );

  if ( !vl )
  {
    return;
  }

  QgsExpressionContext context = mLayout->createExpressionContext();
  QgsExpressionBuilderDialog exprDlg( vl, mAtlasFeatureFilterEdit->text(), this, QStringLiteral( "generic" ), context );
  exprDlg.setWindowTitle( tr( "Expression Based Filter" ) );

  if ( exprDlg.exec() == QDialog::Accepted )
  {
    QString expression = exprDlg.expressionText();
    if ( !expression.isEmpty() )
    {
      mAtlasFeatureFilterEdit->setText( expression );
      QString error;
      mLayout->undoStack()->beginCommand( mAtlas, tr( "Change Atlas Filter" ) );
      if ( !mAtlas->setFilterExpression( mAtlasFeatureFilterEdit->text(), error ) )
      {
        //expression could not be set
        mMessageBar->pushWarning( tr( "Atlas" ),
                                  tr( "Could not set filter expression to '%1'.\nParser error:\n%2" )
                                  .arg( mAtlasFeatureFilterEdit->text(),
                                        error )
                                );
      }
      mLayout->undoStack()->endCommand();
      updateAtlasFeatures();
    }
  }
}

void QgsLayoutAtlasWidget::mAtlasSortFeatureDirectionButton_clicked()
{
  Qt::ArrowType at = mAtlasSortFeatureDirectionButton->arrowType();
  at = ( at == Qt::UpArrow ) ? Qt::DownArrow : Qt::UpArrow;
  mAtlasSortFeatureDirectionButton->setArrowType( at );

  mLayout->undoStack()->beginCommand( mAtlas, tr( "Change Atlas Sort" ) );
  mAtlas->setSortAscending( at == Qt::UpArrow );
  mLayout->undoStack()->endCommand();
  updateAtlasFeatures();
}

void QgsLayoutAtlasWidget::changeFileFormat()
{
  mLayout->setCustomProperty( QStringLiteral( "atlasRasterFormat" ), mAtlasFileFormat->currentText() );
}

void QgsLayoutAtlasWidget::updateGuiElements()
{
  blockAllSignals( true );
  mUseAtlasCheckBox->setCheckState( mAtlas->enabled() ? Qt::Checked : Qt::Unchecked );
  mConfigurationGroup->setEnabled( mAtlas->enabled() );
  mOutputGroup->setEnabled( mAtlas->enabled() );

  mAtlasCoverageLayerComboBox->setLayer( mAtlas->coverageLayer() );
  mPageNameWidget->setLayer( mAtlas->coverageLayer() );
  mPageNameWidget->setField( mAtlas->pageNameExpression() );

  mAtlasSortExpressionWidget->setLayer( mAtlas->coverageLayer() );
  mAtlasSortExpressionWidget->setField( mAtlas->sortExpression() );

  mAtlasFilenamePatternEdit->setText( mAtlas->filenameExpression() );
  mAtlasHideCoverageCheckBox->setCheckState( mAtlas->hideCoverage() ? Qt::Checked : Qt::Unchecked );

  bool singleFile = mLayout->customProperty( QStringLiteral( "singleFile" ), true ).toBool();
  mAtlasSingleFileCheckBox->setCheckState( singleFile ? Qt::Checked : Qt::Unchecked );
  mAtlasFilenamePatternEdit->setEnabled( !singleFile );
  mAtlasFilenameExpressionButton->setEnabled( !singleFile );

  mAtlasSortFeatureCheckBox->setCheckState( mAtlas->sortFeatures() ? Qt::Checked : Qt::Unchecked );
  mAtlasSortFeatureDirectionButton->setEnabled( mAtlas->sortFeatures() );
  mAtlasSortExpressionWidget->setEnabled( mAtlas->sortFeatures() );

  mAtlasSortFeatureDirectionButton->setArrowType( mAtlas->sortAscending() ? Qt::UpArrow : Qt::DownArrow );
  mAtlasFeatureFilterEdit->setText( mAtlas->filterExpression() );

  mAtlasFeatureFilterCheckBox->setCheckState( mAtlas->filterFeatures() ? Qt::Checked : Qt::Unchecked );
  mAtlasFeatureFilterEdit->setEnabled( mAtlas->filterFeatures() );
  mAtlasFeatureFilterButton->setEnabled( mAtlas->filterFeatures() );

  mAtlasFileFormat->setCurrentIndex( mAtlasFileFormat->findText( mLayout->customProperty( QStringLiteral( "atlasRasterFormat" ), QStringLiteral( "png" ) ).toString() ) );

  blockAllSignals( false );
}

void QgsLayoutAtlasWidget::blockAllSignals( bool b )
{
  mUseAtlasCheckBox->blockSignals( b );
  mConfigurationGroup->blockSignals( b );
  mOutputGroup->blockSignals( b );
  mAtlasCoverageLayerComboBox->blockSignals( b );
  mPageNameWidget->blockSignals( b );
  mAtlasSortExpressionWidget->blockSignals( b );
  mAtlasFilenamePatternEdit->blockSignals( b );
  mAtlasHideCoverageCheckBox->blockSignals( b );
  mAtlasSingleFileCheckBox->blockSignals( b );
  mAtlasSortFeatureCheckBox->blockSignals( b );
  mAtlasSortFeatureDirectionButton->blockSignals( b );
  mAtlasFeatureFilterEdit->blockSignals( b );
  mAtlasFeatureFilterCheckBox->blockSignals( b );
}
