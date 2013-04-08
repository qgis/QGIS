/***************************************************************************
                         qgscomposerlabelwidget.cpp
                         --------------------------
    begin                : June 10, 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposerlabelwidget.h"
#include "qgscomposerlabel.h"
#include "qgscomposeritemwidget.h"
#include "qgscomposition.h"
#include "qgsexpressionbuilderdialog.h"

#include <QColorDialog>
#include <QFontDialog>
#include <QWidget>

QgsComposerLabelWidget::QgsComposerLabelWidget( QgsComposerLabel* label ): QWidget(), mComposerLabel( label )
{
  setupUi( this );

  //add widget for general composer item properties
  QgsComposerItemWidget* itemPropertiesWidget = new QgsComposerItemWidget( this, label );
  mainLayout->addWidget( itemPropertiesWidget );


  if ( mComposerLabel )
  {
    setGuiElementValues();
    connect( mComposerLabel, SIGNAL( itemChanged() ), this, SLOT( setGuiElementValues() ) );
  }
}

void QgsComposerLabelWidget::on_mHtmlCheckBox_stateChanged( int state )
{
  if ( mComposerLabel )
  {
    if ( state )
    {
      mFontButton->setEnabled( false );
      mFontColorButton->setEnabled( false );
      mAlignementGroup->setEnabled( false );
    }
    else
    {
      mFontButton->setEnabled( true );
      mFontColorButton->setEnabled( true );
      mAlignementGroup->setEnabled( true );
    }

    mComposerLabel->beginCommand( tr( "Label text HTML state changed" ), QgsComposerMergeCommand::ComposerLabelSetText );
    mComposerLabel->blockSignals( true );
    mComposerLabel->setHtmlSate( state );
    mComposerLabel->setText( mTextEdit->toPlainText() );
    mComposerLabel->update();
    mComposerLabel->blockSignals( false );
    mComposerLabel->endCommand();
  }
}

void QgsComposerLabelWidget::on_mTextEdit_textChanged()
{
  if ( mComposerLabel )
  {
    mComposerLabel->beginCommand( tr( "Label text changed" ), QgsComposerMergeCommand::ComposerLabelSetText );
    mComposerLabel->blockSignals( true );
    mComposerLabel->setText( mTextEdit->toPlainText() );
    mComposerLabel->update();
    mComposerLabel->blockSignals( false );
    mComposerLabel->endCommand();
  }
}

void QgsComposerLabelWidget::on_mFontButton_clicked()
{
  if ( mComposerLabel )
  {
    bool ok;
#if defined(Q_WS_MAC) && QT_VERSION >= 0x040500 && defined(QT_MAC_USE_COCOA)
    // Native Mac dialog works only for Qt Carbon
    QFont newFont = QFontDialog::getFont( &ok, mComposerLabel->font(), 0, QString(), QFontDialog::DontUseNativeDialog );
#else
    QFont newFont = QFontDialog::getFont( &ok, mComposerLabel->font() );
#endif
    if ( ok )
    {
      mComposerLabel->beginCommand( tr( "Label font changed" ) );
      mComposerLabel->setFont( newFont );
      mComposerLabel->update();
      mComposerLabel->endCommand();
    }
  }
}

void QgsComposerLabelWidget::on_mMarginDoubleSpinBox_valueChanged( double d )
{
  if ( mComposerLabel )
  {
    mComposerLabel->beginCommand( tr( "Label margin changed" ) );
    mComposerLabel->setMargin( d );
    mComposerLabel->update();
    mComposerLabel->endCommand();
  }
}

void QgsComposerLabelWidget::on_mFontColorButton_clicked()
{
  if ( !mComposerLabel )
  {
    return;
  }
  QColor newColor = QColorDialog::getColor( mComposerLabel->fontColor() );
  if ( !newColor.isValid() )
  {
    return;
  }
  mComposerLabel->beginCommand( tr( "Label font changed" ) );
  mComposerLabel->setFontColor( newColor );
  mComposerLabel->endCommand();
}

void QgsComposerLabelWidget::on_mInsertExpressionButton_clicked()
{
  if ( !mComposerLabel )
  {
    return;
  }

  QString selText = mTextEdit->textCursor().selectedText();

  // edit the selected expression if there's one
  if ( selText.startsWith( "[%" ) && selText.endsWith( "%]" ) )
    selText = selText.mid( 2, selText.size() - 4 );

  QgsVectorLayer* coverageLayer = 0;
  // use the atlas coverage layer, if any
  if ( mComposerLabel->composition()->atlasComposition().enabled() )
  {
    coverageLayer = mComposerLabel->composition()->atlasComposition().coverageLayer();
  }
  QgsExpressionBuilderDialog exprDlg( coverageLayer, selText, this );
  exprDlg.setWindowTitle( tr( "Insert expression" ) );
  if ( exprDlg.exec() == QDialog::Accepted )
  {
    QString expression =  exprDlg.expressionText();
    if ( !expression.isEmpty() )
    {
      mComposerLabel->beginCommand( tr( "Insert expression" ) );
      mTextEdit->insertPlainText( "[%" + expression + "%]" );
      mComposerLabel->endCommand();
    }
  }
}

void QgsComposerLabelWidget::on_mCenterRadioButton_clicked()
{
  if ( mComposerLabel )
  {
    mComposerLabel->beginCommand( tr( "Label alignment changed" ) );
    mComposerLabel->setHAlign( Qt::AlignHCenter );
    mComposerLabel->update();
    mComposerLabel->endCommand();
  }
}

void QgsComposerLabelWidget::on_mRightRadioButton_clicked()
{
  if ( mComposerLabel )
  {
    mComposerLabel->beginCommand( tr( "Label alignment changed" ) );
    mComposerLabel->setHAlign( Qt::AlignRight );
    mComposerLabel->update();
    mComposerLabel->endCommand();
  }
}

void QgsComposerLabelWidget::on_mLeftRadioButton_clicked()
{
  if ( mComposerLabel )
  {
    mComposerLabel->beginCommand( tr( "Label alignment changed" ) );
    mComposerLabel->setHAlign( Qt::AlignLeft );
    mComposerLabel->update();
    mComposerLabel->endCommand();
  }
}

void QgsComposerLabelWidget::on_mTopRadioButton_clicked()
{
  if ( mComposerLabel )
  {
    mComposerLabel->beginCommand( tr( "Label alignment changed" ) );
    mComposerLabel->setVAlign( Qt::AlignTop );
    mComposerLabel->update();
    mComposerLabel->endCommand();
  }
}

void QgsComposerLabelWidget::on_mBottomRadioButton_clicked()
{
  if ( mComposerLabel )
  {
    mComposerLabel->beginCommand( tr( "Label alignment changed" ) );
    mComposerLabel->setVAlign( Qt::AlignBottom );
    mComposerLabel->update();
    mComposerLabel->endCommand();
  }
}

void QgsComposerLabelWidget::on_mMiddleRadioButton_clicked()
{
  if ( mComposerLabel )
  {
    mComposerLabel->beginCommand( tr( "Label alignment changed" ) );
    mComposerLabel->setVAlign( Qt::AlignVCenter );
    mComposerLabel->update();
    mComposerLabel->endCommand();
  }
}

void QgsComposerLabelWidget::on_mRotationSpinBox_valueChanged( double v )
{
  if ( mComposerLabel )
  {
    mComposerLabel->beginCommand( tr( "Label rotation changed" ), QgsComposerMergeCommand::ComposerLabelRotation );
    mComposerLabel->setRotation( v );
    mComposerLabel->update();
    mComposerLabel->endCommand();
  }
}

void QgsComposerLabelWidget::setGuiElementValues()
{
  blockAllSignals( true );
  mTextEdit->setPlainText( mComposerLabel->text() );
  mTextEdit->moveCursor( QTextCursor::End, QTextCursor::MoveAnchor );
  mMarginDoubleSpinBox->setValue( mComposerLabel->margin() );
  mHtmlCheckBox->setChecked( mComposerLabel->htmlSate() );
  mTopRadioButton->setChecked( mComposerLabel->vAlign() == Qt::AlignTop );
  mMiddleRadioButton->setChecked( mComposerLabel->vAlign() == Qt::AlignVCenter );
  mBottomRadioButton->setChecked( mComposerLabel->vAlign() == Qt::AlignBottom );
  mLeftRadioButton->setChecked( mComposerLabel->hAlign() == Qt::AlignLeft );
  mCenterRadioButton->setChecked( mComposerLabel->hAlign() == Qt::AlignHCenter );
  mRightRadioButton->setChecked( mComposerLabel->hAlign() == Qt::AlignRight );
  mRotationSpinBox->setValue( mComposerLabel->rotation() );
  blockAllSignals( false );
}

void QgsComposerLabelWidget::blockAllSignals( bool block )
{
  mTextEdit->blockSignals( block );
  mHtmlCheckBox->blockSignals( block );
  mMarginDoubleSpinBox->blockSignals( block );
  mTopRadioButton->blockSignals( block );
  mMiddleRadioButton->blockSignals( block );
  mBottomRadioButton->blockSignals( block );
  mLeftRadioButton->blockSignals( block );
  mCenterRadioButton->blockSignals( block );
  mRightRadioButton->blockSignals( block );
  mRotationSpinBox->blockSignals( block );
}
