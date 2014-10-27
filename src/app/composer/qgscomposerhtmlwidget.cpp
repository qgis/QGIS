/***************************************************************************
    qgscomposerhtmlwidget.cpp
    ---------------------
    begin                : July 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgscomposerhtmlwidget.h"
#include "qgscomposerframe.h"
#include "qgscomposeritemwidget.h"
#include "qgscomposermultiframecommand.h"
#include "qgscomposerhtml.h"
#include "qgscomposition.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgscodeeditorhtml.h"
#include "qgscodeeditorcss.h"
#include <QFileDialog>
#include <QSettings>


QgsComposerHtmlWidget::QgsComposerHtmlWidget( QgsComposerHtml* html, QgsComposerFrame* frame )
    : QgsComposerItemBaseWidget( 0, html )
    , mHtml( html )
    , mFrame( frame )
{
  setupUi( this );

  //setup html editor
  mHtmlEditor = new QgsCodeEditorHTML( this );
  connect( mHtmlEditor, SIGNAL( textChanged() ), this, SLOT( htmlEditorChanged() ) );
  htmlEditorLayout->addWidget( mHtmlEditor );

  //setup stylesheet editor
  mStylesheetEditor = new QgsCodeEditorCSS( this );
  connect( mStylesheetEditor, SIGNAL( textChanged() ), this, SLOT( stylesheetEditorChanged() ) );
  stylesheetEditorLayout->addWidget( mStylesheetEditor );

  blockSignals( true );
  mResizeModeComboBox->addItem( tr( "Use existing frames" ), QgsComposerMultiFrame::UseExistingFrames );
  mResizeModeComboBox->addItem( tr( "Extend to next page" ), QgsComposerMultiFrame::ExtendToNextPage );
  mResizeModeComboBox->addItem( tr( "Repeat on every page" ), QgsComposerMultiFrame::RepeatOnEveryPage );
  mResizeModeComboBox->addItem( tr( "Repeat until finished" ), QgsComposerMultiFrame::RepeatUntilFinished );
  blockSignals( false );
  setGuiElementValues();

  if ( mHtml )
  {
    QObject::connect( mHtml, SIGNAL( changed() ), this, SLOT( setGuiElementValues() ) );

    QgsAtlasComposition* atlas = atlasComposition();
    if ( atlas )
    {
      // repopulate data defined buttons if atlas layer changes
      connect( atlas, SIGNAL( coverageLayerChanged( QgsVectorLayer* ) ),
               this, SLOT( populateDataDefinedButtons() ) );
      connect( atlas, SIGNAL( toggled( bool ) ), this, SLOT( populateDataDefinedButtons() ) );
    }
  }

  //embed widget for general options
  if ( mFrame )
  {
    //add widget for general composer item properties
    QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, mFrame );
    mainLayout->addWidget( itemPropertiesWidget );
  }

  //connections for data defined buttons
  connect( mUrlDDBtn, SIGNAL( dataDefinedChanged( const QString& ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mUrlDDBtn, SIGNAL( dataDefinedActivated( bool ) ), this, SLOT( updateDataDefinedProperty() ) );
  connect( mUrlDDBtn, SIGNAL( dataDefinedActivated( bool ) ), mUrlLineEdit, SLOT( setDisabled( bool ) ) );

}

QgsComposerHtmlWidget::QgsComposerHtmlWidget(): QgsComposerItemBaseWidget( 0, 0 )
{
}

QgsComposerHtmlWidget::~QgsComposerHtmlWidget()
{
}

void QgsComposerHtmlWidget::blockSignals( bool block )
{
  mUrlLineEdit->blockSignals( block );
  mFileToolButton->blockSignals( block );
  mResizeModeComboBox->blockSignals( block );
  mUseSmartBreaksCheckBox->blockSignals( block );
  mMaxDistanceSpinBox->blockSignals( block );
  mHtmlEditor->blockSignals( block );
  mStylesheetEditor->blockSignals( block );
  mUserStylesheetCheckBox->blockSignals( block );
  mRadioManualSource->blockSignals( block );
  mRadioUrlSource->blockSignals( block );
  mEvaluateExpressionsCheckbox->blockSignals( block );
  mEmptyFrameCheckBox->blockSignals( block );
  mHideEmptyBgCheckBox->blockSignals( block );
}

void QgsComposerHtmlWidget::on_mUrlLineEdit_editingFinished()
{
  if ( mHtml )
  {
    QUrl newUrl( mUrlLineEdit->text() );
    if ( newUrl == mHtml->url() )
    {
      return;
    }

    QgsComposition* composition = mHtml->composition();
    if ( composition )
    {
      composition->beginMultiFrameCommand( mHtml, tr( "Change html url" ) );
      mHtml->setUrl( newUrl );
      mHtml->update();
      composition->endMultiFrameCommand();
    }
  }
}

void QgsComposerHtmlWidget::on_mFileToolButton_clicked()
{
  QSettings s;
  QString lastDir = s.value( "/UI/lastHtmlDir", "" ).toString();
  QString file = QFileDialog::getOpenFileName( this, tr( "Select HTML document" ), lastDir, "HTML (*.html *.htm);;All files (*.*)" );
  if ( !file.isEmpty() )
  {
    QUrl url = QUrl::fromLocalFile( file );
    mUrlLineEdit->setText( url.toString() );
    on_mUrlLineEdit_editingFinished();
    mHtml->update();
    s.setValue( "/UI/lastHtmlDir", QFileInfo( file ).absolutePath() );
  }
}

void QgsComposerHtmlWidget::on_mResizeModeComboBox_currentIndexChanged( int index )
{
  if ( !mHtml )
  {
    return;
  }

  QgsComposition* composition = mHtml->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mHtml, tr( "Change resize mode" ) );
    mHtml->setResizeMode(( QgsComposerMultiFrame::ResizeMode )mResizeModeComboBox->itemData( index ).toInt() );
    composition->endMultiFrameCommand();
  }

  mAddFramePushButton->setEnabled( mHtml->resizeMode() == QgsComposerMultiFrame::UseExistingFrames );
}

void QgsComposerHtmlWidget::on_mEvaluateExpressionsCheckbox_toggled( bool checked )
{
  if ( !mHtml )
  {
    return;
  }

  QgsComposition* composition = mHtml->composition();
  if ( composition )
  {
    blockSignals( true );
    composition->beginMultiFrameCommand( mHtml, tr( "Evaluate expressions changed" ) );
    mHtml->setEvaluateExpressions( checked );
    composition->endMultiFrameCommand();
    blockSignals( false );
  }
}

void QgsComposerHtmlWidget::on_mUseSmartBreaksCheckBox_toggled( bool checked )
{
  if ( !mHtml )
  {
    return;
  }

  QgsComposition* composition = mHtml->composition();
  if ( composition )
  {
    blockSignals( true );
    composition->beginMultiFrameCommand( mHtml, tr( "Use smart breaks changed" ) );
    mHtml->setUseSmartBreaks( checked );
    composition->endMultiFrameCommand();
    blockSignals( false );
  }
}

void QgsComposerHtmlWidget::on_mMaxDistanceSpinBox_valueChanged( double val )
{
  if ( !mHtml )
  {
    return;
  }

  QgsComposition* composition = mHtml->composition();
  if ( composition )
  {
    blockSignals( true );
    composition->beginMultiFrameCommand( mHtml, tr( "Page break distance changed" ), QgsComposerMultiFrameMergeCommand::HtmlBreakDistance );
    mHtml->setMaxBreakDistance( val );
    composition->endMultiFrameCommand();
    blockSignals( false );
  }
}

void QgsComposerHtmlWidget::htmlEditorChanged()
{
  if ( !mHtml )
  {
    return;
  }

  QgsComposition* composition = mHtml->composition();
  if ( composition )
  {
    blockSignals( true );
    composition->beginMultiFrameCommand( mHtml, tr( "HTML changed" ), QgsComposerMultiFrameMergeCommand::HtmlSource );
    mHtml->setHtml( mHtmlEditor->text() );
    composition->endMultiFrameCommand();
    blockSignals( false );
  }

}

void QgsComposerHtmlWidget::stylesheetEditorChanged()
{
  if ( !mHtml )
  {
    return;
  }

  QgsComposition* composition = mHtml->composition();
  if ( composition )
  {
    blockSignals( true );
    composition->beginMultiFrameCommand( mHtml, tr( "User stylesheet changed" ), QgsComposerMultiFrameMergeCommand::HtmlStylesheet );
    mHtml->setUserStylesheet( mStylesheetEditor->text() );
    composition->endMultiFrameCommand();
    blockSignals( false );
  }
}

void QgsComposerHtmlWidget::on_mUserStylesheetCheckBox_toggled( bool checked )
{
  if ( !mHtml )
  {
    return;
  }

  QgsComposition* composition = mHtml->composition();
  if ( composition )
  {
    blockSignals( true );
    composition->beginMultiFrameCommand( mHtml, tr( "User stylesheet toggled" ) );
    mHtml->setUserStylesheetEnabled( checked );
    composition->endMultiFrameCommand();
    blockSignals( false );
  }
}

void QgsComposerHtmlWidget::on_mEmptyFrameCheckBox_toggled( bool checked )
{
  if ( !mFrame )
  {
    return;
  }

  mFrame->beginCommand( tr( "Empty frame mode toggled" ) );
  mFrame->setHidePageIfEmpty( checked );
  mFrame->endCommand();
}

void QgsComposerHtmlWidget::on_mHideEmptyBgCheckBox_toggled( bool checked )
{
  if ( !mFrame )
  {
    return;
  }

  mFrame->beginCommand( tr( "Hide background if empty toggled" ) );
  mFrame->setHideBackgroundIfEmpty( checked );
  mFrame->endCommand();
}

void QgsComposerHtmlWidget::on_mRadioManualSource_clicked( bool checked )
{
  if ( !mHtml )
  {
    return;
  }

  QgsComposition* composition = mHtml->composition();
  if ( composition )
  {
    blockSignals( true );
    composition->beginMultiFrameCommand( mHtml, tr( "HTML source changed" ) );
    mHtml->setContentMode( checked ? QgsComposerHtml::ManualHtml : QgsComposerHtml::Url );
    composition->endMultiFrameCommand();
    blockSignals( false );
  }
  mHtmlEditor->setEnabled( checked );
  mInsertExpressionButton->setEnabled( checked );
  mUrlLineEdit->setEnabled( !checked );
  mFileToolButton->setEnabled( !checked );

  mHtml->loadHtml();
}

void QgsComposerHtmlWidget::on_mRadioUrlSource_clicked( bool checked )
{
  if ( !mHtml )
  {
    return;
  }

  QgsComposition* composition = mHtml->composition();
  if ( composition )
  {
    blockSignals( true );
    composition->beginMultiFrameCommand( mHtml, tr( "HTML source changed" ) );
    mHtml->setContentMode( checked ? QgsComposerHtml::Url : QgsComposerHtml::ManualHtml );
    composition->endMultiFrameCommand();
    blockSignals( false );
  }
  mHtmlEditor->setEnabled( !checked );
  mInsertExpressionButton->setEnabled( !checked );
  mUrlLineEdit->setEnabled( checked );
  mFileToolButton->setEnabled( checked );

  mHtml->loadHtml();
}

void QgsComposerHtmlWidget::on_mInsertExpressionButton_clicked()
{
  if ( !mHtml )
  {
    return;
  }

  int line = 0;
  int index = 0;
  QString selText;
  if ( mHtmlEditor->hasSelectedText() )
  {
    selText = mHtmlEditor->selectedText();

    // edit the selected expression if there's one
    if ( selText.startsWith( "[%" ) && selText.endsWith( "%]" ) )
      selText = selText.mid( 2, selText.size() - 4 );
  }
  else
  {
    mHtmlEditor->getCursorPosition( &line, &index );
  }

  // use the atlas coverage layer, if any
  QgsVectorLayer* coverageLayer = atlasCoverageLayer();
  QgsExpressionBuilderDialog exprDlg( coverageLayer, selText, this );
  exprDlg.setWindowTitle( tr( "Insert expression" ) );
  if ( exprDlg.exec() == QDialog::Accepted )
  {
    QString expression =  exprDlg.expressionText();
    QgsComposition* composition = mHtml->composition();
    if ( !expression.isEmpty() && composition )
    {
      blockSignals( true );
      composition->beginMultiFrameCommand( mHtml, tr( "HTML source changed" ) );
      if ( mHtmlEditor->hasSelectedText() )
      {
        mHtmlEditor->replaceSelectedText( "[%" + expression + "%]" );
      }
      else
      {
        mHtmlEditor->insertAt( "[%" + expression + "%]", line, index );
      }
      composition->endMultiFrameCommand();
      blockSignals( false );
    }
  }

}

void QgsComposerHtmlWidget::on_mReloadPushButton_clicked()
{
  if ( !mHtml )
  {
    return;
  }

  mHtml->loadHtml();
}

void QgsComposerHtmlWidget::on_mReloadPushButton2_clicked()
{
  if ( !mHtml )
  {
    return;
  }

  mHtml->loadHtml();
}

void QgsComposerHtmlWidget::on_mAddFramePushButton_clicked()
{
  if ( !mHtml || !mFrame )
  {
    return;
  }

  //create a new frame based on the current frame
  QPointF pos = mFrame->pos();
  //shift new frame so that it sits 10 units below current frame
  pos.ry() += mFrame->rect().height() + 10;

  QgsComposerFrame * newFrame = mHtml->createNewFrame( mFrame, pos, mFrame->rect().size() );
  mHtml->recalculateFrameSizes();

  //set new frame as selection
  QgsComposition* composition = mHtml->composition();
  if ( composition )
  {
    composition->setSelectedItem( newFrame );
  }
}

void QgsComposerHtmlWidget::setGuiElementValues()
{
  if ( !mHtml )
  {
    return;
  }

  blockSignals( true );
  mUrlLineEdit->setText( mHtml->url().toString() );
  mResizeModeComboBox->setCurrentIndex( mResizeModeComboBox->findData( mHtml->resizeMode() ) );
  mEvaluateExpressionsCheckbox->setChecked( mHtml->evaluateExpressions() );
  mUseSmartBreaksCheckBox->setChecked( mHtml->useSmartBreaks() );
  mMaxDistanceSpinBox->setValue( mHtml->maxBreakDistance() );

  mAddFramePushButton->setEnabled( mHtml->resizeMode() == QgsComposerMultiFrame::UseExistingFrames );
  mHtmlEditor->setText( mHtml->html() );

  mRadioUrlSource->setChecked( mHtml->contentMode() == QgsComposerHtml::Url );
  mUrlLineEdit->setEnabled( mHtml->contentMode() == QgsComposerHtml::Url );
  mFileToolButton->setEnabled( mHtml->contentMode() == QgsComposerHtml::Url );
  mRadioManualSource->setChecked( mHtml->contentMode() == QgsComposerHtml::ManualHtml );
  mHtmlEditor->setEnabled( mHtml->contentMode() == QgsComposerHtml::ManualHtml );
  mInsertExpressionButton->setEnabled( mHtml->contentMode() == QgsComposerHtml::ManualHtml );

  mUserStylesheetCheckBox->setChecked( mHtml->userStylesheetEnabled() );
  mStylesheetEditor->setText( mHtml->userStylesheet() );

  mEmptyFrameCheckBox->setChecked( mFrame->hidePageIfEmpty() );
  mHideEmptyBgCheckBox->setChecked( mFrame->hideBackgroundIfEmpty() );

  populateDataDefinedButtons();

  blockSignals( false );
}

QgsComposerItem::DataDefinedProperty QgsComposerHtmlWidget::ddPropertyForWidget( QgsDataDefinedButton *widget )
{
  if ( widget == mUrlDDBtn )
  {
    return QgsComposerItem::SourceUrl;
  }
  return QgsComposerItem::NoProperty;
}

void QgsComposerHtmlWidget::populateDataDefinedButtons()
{
  QgsVectorLayer* vl = atlasCoverageLayer();

  //block signals from data defined buttons
  mUrlDDBtn->blockSignals( true );

  //initialise buttons to use atlas coverage layer
  mUrlDDBtn->init( vl, mHtml->dataDefinedProperty( QgsComposerItem::SourceUrl ),
                   QgsDataDefinedButton::AnyType, tr( "url string" ) );

  //initial state of controls - disable related controls when dd buttons are active
  mUrlLineEdit->setEnabled( !mUrlDDBtn->isActive() );

  //unblock signals from data defined buttons
  mUrlDDBtn->blockSignals( false );
}
