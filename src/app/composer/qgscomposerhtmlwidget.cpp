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
#include "qgssettings.h"

#include <QFileDialog>


QgsComposerHtmlWidget::QgsComposerHtmlWidget( QgsComposerHtml *html, QgsComposerFrame *frame )
  : QgsComposerItemBaseWidget( nullptr, html )
  , mHtml( html )
  , mFrame( frame )
{
  setupUi( this );
  connect( mUrlLineEdit, &QLineEdit::editingFinished, this, &QgsComposerHtmlWidget::mUrlLineEdit_editingFinished );
  connect( mFileToolButton, &QToolButton::clicked, this, &QgsComposerHtmlWidget::mFileToolButton_clicked );
  connect( mResizeModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsComposerHtmlWidget::mResizeModeComboBox_currentIndexChanged );
  connect( mEvaluateExpressionsCheckbox, &QCheckBox::toggled, this, &QgsComposerHtmlWidget::mEvaluateExpressionsCheckbox_toggled );
  connect( mUseSmartBreaksCheckBox, &QgsCollapsibleGroupBoxBasic::toggled, this, &QgsComposerHtmlWidget::mUseSmartBreaksCheckBox_toggled );
  connect( mMaxDistanceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsComposerHtmlWidget::mMaxDistanceSpinBox_valueChanged );
  connect( mUserStylesheetCheckBox, &QgsCollapsibleGroupBoxBasic::toggled, this, &QgsComposerHtmlWidget::mUserStylesheetCheckBox_toggled );
  connect( mRadioManualSource, &QRadioButton::clicked, this, &QgsComposerHtmlWidget::mRadioManualSource_clicked );
  connect( mRadioUrlSource, &QRadioButton::clicked, this, &QgsComposerHtmlWidget::mRadioUrlSource_clicked );
  connect( mInsertExpressionButton, &QPushButton::clicked, this, &QgsComposerHtmlWidget::mInsertExpressionButton_clicked );
  connect( mReloadPushButton, &QPushButton::clicked, this, &QgsComposerHtmlWidget::mReloadPushButton_clicked );
  connect( mReloadPushButton2, &QPushButton::clicked, this, &QgsComposerHtmlWidget::mReloadPushButton2_clicked );
  connect( mAddFramePushButton, &QPushButton::clicked, this, &QgsComposerHtmlWidget::mAddFramePushButton_clicked );
  connect( mEmptyFrameCheckBox, &QCheckBox::toggled, this, &QgsComposerHtmlWidget::mEmptyFrameCheckBox_toggled );
  connect( mHideEmptyBgCheckBox, &QCheckBox::toggled, this, &QgsComposerHtmlWidget::mHideEmptyBgCheckBox_toggled );
  setPanelTitle( tr( "HTML properties" ) );

  //setup html editor
  mHtmlEditor = new QgsCodeEditorHTML( this );
  connect( mHtmlEditor, &QsciScintilla::textChanged, this, &QgsComposerHtmlWidget::htmlEditorChanged );
  htmlEditorLayout->addWidget( mHtmlEditor );

  //setup stylesheet editor
  mStylesheetEditor = new QgsCodeEditorCSS( this );
  connect( mStylesheetEditor, &QsciScintilla::textChanged, this, &QgsComposerHtmlWidget::stylesheetEditorChanged );
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
    connect( mHtml, &QgsComposerMultiFrame::changed, this, &QgsComposerHtmlWidget::setGuiElementValues );
  }

  //embed widget for general options
  if ( mFrame )
  {
    //add widget for general composer item properties
    QgsComposerItemWidget *itemPropertiesWidget = new QgsComposerItemWidget( this, mFrame );
    mainLayout->addWidget( itemPropertiesWidget );
  }

  //connections for data defined buttons
  connect( mUrlDDBtn, &QgsPropertyOverrideButton::activated, mUrlLineEdit, &QLineEdit::setDisabled );
  registerDataDefinedButton( mUrlDDBtn, QgsComposerObject::SourceUrl );
}

QgsComposerHtmlWidget::QgsComposerHtmlWidget()
  : QgsComposerItemBaseWidget( nullptr, nullptr )

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

void QgsComposerHtmlWidget::mUrlLineEdit_editingFinished()
{
  if ( mHtml )
  {
    QUrl newUrl( mUrlLineEdit->text() );
    if ( newUrl == mHtml->url() )
    {
      return;
    }

    QgsComposition *composition = mHtml->composition();
    if ( composition )
    {
      composition->beginMultiFrameCommand( mHtml, tr( "Change HTML url" ) );
      mHtml->setUrl( newUrl );
      mHtml->update();
      composition->endMultiFrameCommand();
    }
  }
}

void QgsComposerHtmlWidget::mFileToolButton_clicked()
{
  QgsSettings s;
  QString lastDir = s.value( QStringLiteral( "/UI/lastHtmlDir" ), QDir::homePath() ).toString();
  QString file = QFileDialog::getOpenFileName( this, tr( "Select HTML document" ), lastDir, QStringLiteral( "HTML (*.html *.htm);;All files (*.*)" ) );
  if ( !file.isEmpty() )
  {
    QUrl url = QUrl::fromLocalFile( file );
    mUrlLineEdit->setText( url.toString() );
    mUrlLineEdit_editingFinished();
    mHtml->update();
    s.setValue( QStringLiteral( "/UI/lastHtmlDir" ), QFileInfo( file ).absolutePath() );
  }
}

void QgsComposerHtmlWidget::mResizeModeComboBox_currentIndexChanged( int index )
{
  if ( !mHtml )
  {
    return;
  }

  QgsComposition *composition = mHtml->composition();
  if ( composition )
  {
    composition->beginMultiFrameCommand( mHtml, tr( "Change resize mode" ) );
    mHtml->setResizeMode( ( QgsComposerMultiFrame::ResizeMode )mResizeModeComboBox->itemData( index ).toInt() );
    composition->endMultiFrameCommand();
  }

  mAddFramePushButton->setEnabled( mHtml->resizeMode() == QgsComposerMultiFrame::UseExistingFrames );
}

void QgsComposerHtmlWidget::mEvaluateExpressionsCheckbox_toggled( bool checked )
{
  if ( !mHtml )
  {
    return;
  }

  QgsComposition *composition = mHtml->composition();
  if ( composition )
  {
    blockSignals( true );
    composition->beginMultiFrameCommand( mHtml, tr( "Evaluate expressions changed" ) );
    mHtml->setEvaluateExpressions( checked );
    composition->endMultiFrameCommand();
    blockSignals( false );
  }
}

void QgsComposerHtmlWidget::mUseSmartBreaksCheckBox_toggled( bool checked )
{
  if ( !mHtml )
  {
    return;
  }

  QgsComposition *composition = mHtml->composition();
  if ( composition )
  {
    blockSignals( true );
    composition->beginMultiFrameCommand( mHtml, tr( "Use smart breaks changed" ) );
    mHtml->setUseSmartBreaks( checked );
    composition->endMultiFrameCommand();
    blockSignals( false );
  }
}

void QgsComposerHtmlWidget::mMaxDistanceSpinBox_valueChanged( double val )
{
  if ( !mHtml )
  {
    return;
  }

  QgsComposition *composition = mHtml->composition();
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

  QgsComposition *composition = mHtml->composition();
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

  QgsComposition *composition = mHtml->composition();
  if ( composition )
  {
    blockSignals( true );
    composition->beginMultiFrameCommand( mHtml, tr( "User stylesheet changed" ), QgsComposerMultiFrameMergeCommand::HtmlStylesheet );
    mHtml->setUserStylesheet( mStylesheetEditor->text() );
    composition->endMultiFrameCommand();
    blockSignals( false );
  }
}

void QgsComposerHtmlWidget::mUserStylesheetCheckBox_toggled( bool checked )
{
  if ( !mHtml )
  {
    return;
  }

  QgsComposition *composition = mHtml->composition();
  if ( composition )
  {
    blockSignals( true );
    composition->beginMultiFrameCommand( mHtml, tr( "User stylesheet toggled" ) );
    mHtml->setUserStylesheetEnabled( checked );
    composition->endMultiFrameCommand();
    blockSignals( false );
  }
}

void QgsComposerHtmlWidget::mEmptyFrameCheckBox_toggled( bool checked )
{
  if ( !mFrame )
  {
    return;
  }

  mFrame->beginCommand( tr( "Empty frame mode toggled" ) );
  mFrame->setHidePageIfEmpty( checked );
  mFrame->endCommand();
}

void QgsComposerHtmlWidget::mHideEmptyBgCheckBox_toggled( bool checked )
{
  if ( !mFrame )
  {
    return;
  }

  mFrame->beginCommand( tr( "Hide background if empty toggled" ) );
  mFrame->setHideBackgroundIfEmpty( checked );
  mFrame->endCommand();
}

void QgsComposerHtmlWidget::mRadioManualSource_clicked( bool checked )
{
  if ( !mHtml )
  {
    return;
  }

  QgsComposition *composition = mHtml->composition();
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

void QgsComposerHtmlWidget::mRadioUrlSource_clicked( bool checked )
{
  if ( !mHtml )
  {
    return;
  }

  QgsComposition *composition = mHtml->composition();
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

void QgsComposerHtmlWidget::mInsertExpressionButton_clicked()
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
    if ( selText.startsWith( QLatin1String( "[%" ) ) && selText.endsWith( QLatin1String( "%]" ) ) )
      selText = selText.mid( 2, selText.size() - 4 );
  }
  else
  {
    mHtmlEditor->getCursorPosition( &line, &index );
  }

  // use the atlas coverage layer, if any
  QgsVectorLayer *coverageLayer = atlasCoverageLayer();
  QgsExpressionContext context = mHtml->createExpressionContext();
  QgsExpressionBuilderDialog exprDlg( coverageLayer, selText, this, QStringLiteral( "generic" ), context );
  exprDlg.setWindowTitle( tr( "Insert Expression" ) );
  if ( exprDlg.exec() == QDialog::Accepted )
  {
    QString expression = exprDlg.expressionText();
    QgsComposition *composition = mHtml->composition();
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
      mHtml->setHtml( mHtmlEditor->text() );
      composition->endMultiFrameCommand();
      blockSignals( false );
    }
  }

}

void QgsComposerHtmlWidget::mReloadPushButton_clicked()
{
  if ( !mHtml )
  {
    return;
  }

  mHtml->loadHtml();
}

void QgsComposerHtmlWidget::mReloadPushButton2_clicked()
{
  if ( !mHtml )
  {
    return;
  }

  mHtml->loadHtml();
}

void QgsComposerHtmlWidget::mAddFramePushButton_clicked()
{
  if ( !mHtml || !mFrame )
  {
    return;
  }

  //create a new frame based on the current frame
  QPointF pos = mFrame->pos();
  //shift new frame so that it sits 10 units below current frame
  pos.ry() += mFrame->rect().height() + 10;

  QgsComposerFrame *newFrame = mHtml->createNewFrame( mFrame, pos, mFrame->rect().size() );
  mHtml->recalculateFrameSizes();

  //set new frame as selection
  QgsComposition *composition = mHtml->composition();
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

void QgsComposerHtmlWidget::populateDataDefinedButtons()
{
  updateDataDefinedButton( mUrlDDBtn );

  //initial state of controls - disable related controls when dd buttons are active
  mUrlLineEdit->setEnabled( !mUrlDDBtn->isActive() );
}
