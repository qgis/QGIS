/***************************************************************************
    qgslayouthtmlwidget.cpp
    -----------------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslayouthtmlwidget.h"
#include "qgslayoutframe.h"
#include "qgslayoutitemhtml.h"
#include "qgslayout.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgscodeeditorhtml.h"
#include "qgscodeeditorcss.h"
#include "qgssettings.h"
#include "qgslayoutundostack.h"

#include <QFileDialog>
#include <QUrl>

QgsLayoutHtmlWidget::QgsLayoutHtmlWidget( QgsLayoutFrame *frame )
  : QgsLayoutItemBaseWidget( nullptr, frame ? qobject_cast< QgsLayoutItemHtml* >( frame->multiFrame() ) : nullptr )
  , mHtml( frame ? qobject_cast< QgsLayoutItemHtml* >( frame->multiFrame() ) : nullptr )
  , mFrame( frame )
{
  setupUi( this );
  connect( mUrlLineEdit, &QLineEdit::editingFinished, this, &QgsLayoutHtmlWidget::mUrlLineEdit_editingFinished );
  connect( mFileToolButton, &QToolButton::clicked, this, &QgsLayoutHtmlWidget::mFileToolButton_clicked );
  connect( mResizeModeComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsLayoutHtmlWidget::mResizeModeComboBox_currentIndexChanged );
  connect( mEvaluateExpressionsCheckbox, &QCheckBox::toggled, this, &QgsLayoutHtmlWidget::mEvaluateExpressionsCheckbox_toggled );
  connect( mUseSmartBreaksCheckBox, &QgsCollapsibleGroupBoxBasic::toggled, this, &QgsLayoutHtmlWidget::mUseSmartBreaksCheckBox_toggled );
  connect( mMaxDistanceSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutHtmlWidget::mMaxDistanceSpinBox_valueChanged );
  connect( mUserStylesheetCheckBox, &QgsCollapsibleGroupBoxBasic::toggled, this, &QgsLayoutHtmlWidget::mUserStylesheetCheckBox_toggled );
  connect( mRadioManualSource, &QRadioButton::clicked, this, &QgsLayoutHtmlWidget::mRadioManualSource_clicked );
  connect( mRadioUrlSource, &QRadioButton::clicked, this, &QgsLayoutHtmlWidget::mRadioUrlSource_clicked );
  connect( mInsertExpressionButton, &QPushButton::clicked, this, &QgsLayoutHtmlWidget::mInsertExpressionButton_clicked );
  connect( mReloadPushButton, &QPushButton::clicked, this, &QgsLayoutHtmlWidget::mReloadPushButton_clicked );
  connect( mReloadPushButton2, &QPushButton::clicked, this, &QgsLayoutHtmlWidget::mReloadPushButton_clicked );
  connect( mAddFramePushButton, &QPushButton::clicked, this, &QgsLayoutHtmlWidget::mAddFramePushButton_clicked );
  connect( mEmptyFrameCheckBox, &QCheckBox::toggled, this, &QgsLayoutHtmlWidget::mEmptyFrameCheckBox_toggled );
  connect( mHideEmptyBgCheckBox, &QCheckBox::toggled, this, &QgsLayoutHtmlWidget::mHideEmptyBgCheckBox_toggled );
  setPanelTitle( tr( "HTML Properties" ) );

  //setup html editor
  mHtmlEditor = new QgsCodeEditorHTML( this );
  connect( mHtmlEditor, &QsciScintilla::textChanged, this, &QgsLayoutHtmlWidget::htmlEditorChanged );
  htmlEditorLayout->addWidget( mHtmlEditor );

  //setup stylesheet editor
  mStylesheetEditor = new QgsCodeEditorCSS( this );
  connect( mStylesheetEditor, &QsciScintilla::textChanged, this, &QgsLayoutHtmlWidget::stylesheetEditorChanged );
  stylesheetEditorLayout->addWidget( mStylesheetEditor );

  blockSignals( true );
  mResizeModeComboBox->addItem( tr( "Use Existing Frames" ), QgsLayoutMultiFrame::UseExistingFrames );
  mResizeModeComboBox->addItem( tr( "Extend to Next Page" ), QgsLayoutMultiFrame::ExtendToNextPage );
  mResizeModeComboBox->addItem( tr( "Repeat on Every Page" ), QgsLayoutMultiFrame::RepeatOnEveryPage );
  mResizeModeComboBox->addItem( tr( "Repeat Until Finished" ), QgsLayoutMultiFrame::RepeatUntilFinished );
  blockSignals( false );
  setGuiElementValues();

  if ( mHtml )
  {
    connect( mHtml, &QgsLayoutMultiFrame::changed, this, &QgsLayoutHtmlWidget::setGuiElementValues );
  }

  //embed widget for general options
  if ( mFrame )
  {
    //add widget for general composer item properties
    mItemPropertiesWidget = new QgsLayoutItemPropertiesWidget( this, mFrame );
    mainLayout->addWidget( mItemPropertiesWidget );
  }

  //connections for data defined buttons
  connect( mUrlDDBtn, &QgsPropertyOverrideButton::activated, mUrlLineEdit, &QLineEdit::setDisabled );
  registerDataDefinedButton( mUrlDDBtn, QgsLayoutObject::SourceUrl );
}

void QgsLayoutHtmlWidget::setMasterLayout( QgsMasterLayoutInterface *masterLayout )
{
  if ( mItemPropertiesWidget )
    mItemPropertiesWidget->setMasterLayout( masterLayout );
}

bool QgsLayoutHtmlWidget::setNewItem( QgsLayoutItem *item )
{
  QgsLayoutFrame *frame = qobject_cast< QgsLayoutFrame * >( item );
  if ( !frame )
    return false;

  QgsLayoutMultiFrame *multiFrame = frame->multiFrame();
  if ( !multiFrame )
    return false;

  if ( multiFrame->type() != QgsLayoutItemRegistry::LayoutHtml )
    return false;

  if ( mHtml )
  {
    disconnect( mHtml, &QgsLayoutObject::changed, this, &QgsLayoutHtmlWidget::setGuiElementValues );
  }

  mHtml = qobject_cast< QgsLayoutItemHtml * >( multiFrame );
  mFrame = frame;
  mItemPropertiesWidget->setItem( frame );

  if ( mHtml )
  {
    connect( mHtml, &QgsLayoutObject::changed, this, &QgsLayoutHtmlWidget::setGuiElementValues );
  }

  setGuiElementValues();

  return true;
}

void QgsLayoutHtmlWidget::blockSignals( bool block )
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

void QgsLayoutHtmlWidget::mUrlLineEdit_editingFinished()
{
  if ( mHtml )
  {
    const QUrl newUrl( mUrlLineEdit->text() );
    if ( newUrl == mHtml->url() )
    {
      return;
    }

    mHtml->beginCommand( tr( "Change HTML Url" ) );
    mHtml->setUrl( newUrl );
    mHtml->update();
    mHtml->endCommand();
  }
}

void QgsLayoutHtmlWidget::mFileToolButton_clicked()
{
  QgsSettings s;
  const QString lastDir = s.value( QStringLiteral( "/UI/lastHtmlDir" ), QDir::homePath() ).toString();
  const QString file = QFileDialog::getOpenFileName( this, tr( "Select HTML document" ), lastDir, QStringLiteral( "HTML (*.html *.htm);;All files (*.*)" ) );
  if ( !file.isEmpty() )
  {
    const QUrl url = QUrl::fromLocalFile( file );
    mUrlLineEdit->setText( url.toString() );
    mUrlLineEdit_editingFinished();
    mHtml->update();
    s.setValue( QStringLiteral( "/UI/lastHtmlDir" ), QFileInfo( file ).absolutePath() );
  }
}

void QgsLayoutHtmlWidget::mResizeModeComboBox_currentIndexChanged( int index )
{
  if ( !mHtml )
  {
    return;
  }

  mHtml->beginCommand( tr( "Change Resize Mode" ) );
  mHtml->setResizeMode( static_cast< QgsLayoutMultiFrame::ResizeMode >( mResizeModeComboBox->itemData( index ).toInt() ) );
  mHtml->endCommand();

  mAddFramePushButton->setEnabled( mHtml->resizeMode() == QgsLayoutMultiFrame::UseExistingFrames );
}

void QgsLayoutHtmlWidget::mEvaluateExpressionsCheckbox_toggled( bool checked )
{
  if ( !mHtml )
  {
    return;
  }

  blockSignals( true );
  mHtml->beginCommand( tr( "Change Evaluate Expressions" ) );
  mHtml->setEvaluateExpressions( checked );
  mHtml->endCommand();
  blockSignals( false );
}

void QgsLayoutHtmlWidget::mUseSmartBreaksCheckBox_toggled( bool checked )
{
  if ( !mHtml )
  {
    return;
  }

  blockSignals( true );
  mHtml->beginCommand( tr( "Change Smart Breaks" ) );
  mHtml->setUseSmartBreaks( checked );
  mHtml->endCommand();
  blockSignals( false );
}

void QgsLayoutHtmlWidget::mMaxDistanceSpinBox_valueChanged( double val )
{
  if ( !mHtml )
  {
    return;
  }

  blockSignals( true );
  mHtml->beginCommand( tr( "Change Page Break Distance" ), QgsLayoutMultiFrame::UndoHtmlBreakDistance );
  mHtml->setMaxBreakDistance( val );
  mHtml->endCommand();
  blockSignals( false );
}

void QgsLayoutHtmlWidget::htmlEditorChanged()
{
  if ( !mHtml )
  {
    return;
  }

  blockSignals( true );
  mHtml->beginCommand( tr( "Change HTML" ), QgsLayoutMultiFrame::UndoHtmlSource );
  mHtml->setHtml( mHtmlEditor->text() );
  mHtml->endCommand();
  blockSignals( false );
}

void QgsLayoutHtmlWidget::stylesheetEditorChanged()
{
  if ( !mHtml )
  {
    return;
  }

  blockSignals( true );
  mHtml->beginCommand( tr( "Change User Stylesheet" ), QgsLayoutMultiFrame::UndoHtmlStylesheet );
  mHtml->setUserStylesheet( mStylesheetEditor->text() );
  mHtml->endCommand();
  blockSignals( false );
}

void QgsLayoutHtmlWidget::mUserStylesheetCheckBox_toggled( bool checked )
{
  if ( !mHtml )
  {
    return;
  }

  blockSignals( true );
  mHtml->beginCommand( tr( "Toggle User Stylesheet" ) );
  mHtml->setUserStylesheetEnabled( checked );
  mHtml->endCommand();
  blockSignals( false );
}

void QgsLayoutHtmlWidget::mEmptyFrameCheckBox_toggled( bool checked )
{
  if ( !mFrame )
  {
    return;
  }

  mFrame->beginCommand( tr( "Toggle Empty Frame Mode" ) );
  mFrame->setHidePageIfEmpty( checked );
  mFrame->endCommand();
}

void QgsLayoutHtmlWidget::mHideEmptyBgCheckBox_toggled( bool checked )
{
  if ( !mFrame )
  {
    return;
  }

  mFrame->beginCommand( tr( "Toggle Hide Background" ) );
  mFrame->setHideBackgroundIfEmpty( checked );
  mFrame->endCommand();
}

void QgsLayoutHtmlWidget::mRadioManualSource_clicked( bool checked )
{
  if ( !mHtml )
  {
    return;
  }

  blockSignals( true );
  mHtml->beginCommand( tr( "Change HTML Source" ) );
  mHtml->setContentMode( checked ? QgsLayoutItemHtml::ManualHtml : QgsLayoutItemHtml::Url );
  blockSignals( false );

  mHtmlEditor->setEnabled( checked );
  mInsertExpressionButton->setEnabled( checked );
  mUrlLineEdit->setEnabled( !checked );
  mFileToolButton->setEnabled( !checked );

  mHtml->loadHtml();
  mHtml->endCommand();
}

void QgsLayoutHtmlWidget::mRadioUrlSource_clicked( bool checked )
{
  if ( !mHtml )
  {
    return;
  }

  blockSignals( true );
  mHtml->beginCommand( tr( "Change HTML Source" ) );
  mHtml->setContentMode( checked ? QgsLayoutItemHtml::Url : QgsLayoutItemHtml::ManualHtml );
  blockSignals( false );

  mHtmlEditor->setEnabled( !checked );
  mInsertExpressionButton->setEnabled( !checked );
  mUrlLineEdit->setEnabled( checked );
  mFileToolButton->setEnabled( checked );

  mHtml->loadHtml();
  mHtml->endCommand();
}

void QgsLayoutHtmlWidget::mInsertExpressionButton_clicked()
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
  QgsVectorLayer *layer = coverageLayer();

  const QgsExpressionContext context = mHtml->createExpressionContext();
  QgsExpressionBuilderDialog exprDlg( layer, selText, this, QStringLiteral( "generic" ), context );
  exprDlg.setWindowTitle( tr( "Insert Expression" ) );
  if ( exprDlg.exec() == QDialog::Accepted )
  {
    const QString expression = exprDlg.expressionText();
    if ( !expression.isEmpty() )
    {
      blockSignals( true );
      mHtml->beginCommand( tr( "Change HTML Source" ) );
      if ( mHtmlEditor->hasSelectedText() )
      {
        mHtmlEditor->replaceSelectedText( "[%" + expression + "%]" );
      }
      else
      {
        mHtmlEditor->insertAt( "[%" + expression + "%]", line, index );
      }
      mHtml->setHtml( mHtmlEditor->text() );
      mHtml->endCommand();
      blockSignals( false );
    }
  }

}

void QgsLayoutHtmlWidget::mReloadPushButton_clicked()
{
  if ( !mHtml )
  {
    return;
  }

  if ( mHtml->layout() )
    mHtml->layout()->undoStack()->blockCommands( true );
  mHtml->loadHtml();
  if ( mHtml->layout() )
    mHtml->layout()->undoStack()->blockCommands( false );
}

void QgsLayoutHtmlWidget::mAddFramePushButton_clicked()
{
  if ( !mHtml || !mFrame )
  {
    return;
  }

  //create a new frame based on the current frame
  QPointF pos = mFrame->pos();
  //shift new frame so that it sits 10 units below current frame
  pos.ry() += mFrame->rect().height() + 10;

  QgsLayoutFrame *newFrame = mHtml->createNewFrame( mFrame, pos, mFrame->rect().size() );
  mHtml->recalculateFrameSizes();

  //set new frame as selection
  if ( QgsLayout *layout = mHtml->layout() )
  {
    layout->setSelectedItem( newFrame );
  }
}

void QgsLayoutHtmlWidget::setGuiElementValues()
{
  if ( !mHtml || !mFrame )
  {
    return;
  }

  blockSignals( true );
  mUrlLineEdit->setText( mHtml->url().toString() );
  mResizeModeComboBox->setCurrentIndex( mResizeModeComboBox->findData( mHtml->resizeMode() ) );
  mEvaluateExpressionsCheckbox->setChecked( mHtml->evaluateExpressions() );
  mUseSmartBreaksCheckBox->setChecked( mHtml->useSmartBreaks() );
  mMaxDistanceSpinBox->setValue( mHtml->maxBreakDistance() );

  mAddFramePushButton->setEnabled( mHtml->resizeMode() == QgsLayoutMultiFrame::UseExistingFrames );
  mHtmlEditor->setText( mHtml->html() );

  mRadioUrlSource->setChecked( mHtml->contentMode() == QgsLayoutItemHtml::Url );
  mUrlLineEdit->setEnabled( mHtml->contentMode() == QgsLayoutItemHtml::Url );
  mFileToolButton->setEnabled( mHtml->contentMode() == QgsLayoutItemHtml::Url );
  mRadioManualSource->setChecked( mHtml->contentMode() == QgsLayoutItemHtml::ManualHtml );
  mHtmlEditor->setEnabled( mHtml->contentMode() == QgsLayoutItemHtml::ManualHtml );
  mInsertExpressionButton->setEnabled( mHtml->contentMode() == QgsLayoutItemHtml::ManualHtml );

  mUserStylesheetCheckBox->setChecked( mHtml->userStylesheetEnabled() );
  mStylesheetEditor->setText( mHtml->userStylesheet() );

  mEmptyFrameCheckBox->setChecked( mFrame->hidePageIfEmpty() );
  mHideEmptyBgCheckBox->setChecked( mFrame->hideBackgroundIfEmpty() );

  populateDataDefinedButtons();

  blockSignals( false );
}

void QgsLayoutHtmlWidget::populateDataDefinedButtons()
{
  updateDataDefinedButton( mUrlDDBtn );

  //initial state of controls - disable related controls when dd buttons are active
  mUrlLineEdit->setEnabled( !mUrlDDBtn->isActive() );
}
