/***************************************************************************
                         qgslayoutlabelwidget.cpp
                         ------------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutlabelwidget.h"
#include "qgslayoutitemlabel.h"
#include "qgslayout.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsguiutils.h"

#include <QColorDialog>
#include <QFontDialog>
#include <QWidget>

QgsLayoutLabelWidget::QgsLayoutLabelWidget( QgsLayoutItemLabel *label )
  : QgsLayoutItemBaseWidget( nullptr, label )
  , mLabel( label )
{
  Q_ASSERT( mLabel );

  setupUi( this );
  connect( mHtmlCheckBox, &QCheckBox::stateChanged, this, &QgsLayoutLabelWidget::mHtmlCheckBox_stateChanged );
  connect( mTextEdit, &QPlainTextEdit::textChanged, this, &QgsLayoutLabelWidget::mTextEdit_textChanged );
  connect( mInsertExpressionButton, &QPushButton::clicked, this, &QgsLayoutLabelWidget::mInsertExpressionButton_clicked );
  connect( mMarginXDoubleSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLabelWidget::mMarginXDoubleSpinBox_valueChanged );
  connect( mMarginYDoubleSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsLayoutLabelWidget::mMarginYDoubleSpinBox_valueChanged );
  connect( mFontColorButton, &QgsColorButton::colorChanged, this, &QgsLayoutLabelWidget::mFontColorButton_colorChanged );
  connect( mCenterRadioButton, &QRadioButton::clicked, this, &QgsLayoutLabelWidget::mCenterRadioButton_clicked );
  connect( mLeftRadioButton, &QRadioButton::clicked, this, &QgsLayoutLabelWidget::mLeftRadioButton_clicked );
  connect( mRightRadioButton, &QRadioButton::clicked, this, &QgsLayoutLabelWidget::mRightRadioButton_clicked );
  connect( mTopRadioButton, &QRadioButton::clicked, this, &QgsLayoutLabelWidget::mTopRadioButton_clicked );
  connect( mBottomRadioButton, &QRadioButton::clicked, this, &QgsLayoutLabelWidget::mBottomRadioButton_clicked );
  connect( mMiddleRadioButton, &QRadioButton::clicked, this, &QgsLayoutLabelWidget::mMiddleRadioButton_clicked );
  setPanelTitle( tr( "Label Properties" ) );

  mFontButton->setMode( QgsFontButton::ModeQFont );

  //add widget for general composer item properties
  mItemPropertiesWidget = new QgsLayoutItemPropertiesWidget( this, label );
  mainLayout->addWidget( mItemPropertiesWidget );

  mFontColorButton->setColorDialogTitle( tr( "Select Font Color" ) );
  mFontColorButton->setContext( QStringLiteral( "composer" ) );
  mFontColorButton->setAllowOpacity( true );

  mMarginXDoubleSpinBox->setClearValue( 0.0 );
  mMarginYDoubleSpinBox->setClearValue( 0.0 );

  setGuiElementValues();
  connect( mLabel, &QgsLayoutObject::changed, this, &QgsLayoutLabelWidget::setGuiElementValues );

  connect( mFontButton, &QgsFontButton::changed, this, &QgsLayoutLabelWidget::fontChanged );
  connect( mJustifyRadioButton, &QRadioButton::clicked, this, &QgsLayoutLabelWidget::justifyClicked );
}

void QgsLayoutLabelWidget::setMasterLayout( QgsMasterLayoutInterface *masterLayout )
{
  if ( mItemPropertiesWidget )
    mItemPropertiesWidget->setMasterLayout( masterLayout );
}

bool QgsLayoutLabelWidget::setNewItem( QgsLayoutItem *item )
{
  if ( item->type() != QgsLayoutItemRegistry::LayoutLabel )
    return false;

  if ( mLabel )
  {
    disconnect( mLabel, &QgsLayoutObject::changed, this, &QgsLayoutLabelWidget::setGuiElementValues );
  }

  mLabel = qobject_cast< QgsLayoutItemLabel * >( item );
  mItemPropertiesWidget->setItem( mLabel );

  if ( mLabel )
  {
    connect( mLabel, &QgsLayoutObject::changed, this, &QgsLayoutLabelWidget::setGuiElementValues );
  }

  setGuiElementValues();

  return true;
}

void QgsLayoutLabelWidget::mHtmlCheckBox_stateChanged( int state )
{
  if ( mLabel )
  {
    mVerticalAlignementLabel->setDisabled( state );
    mTopRadioButton->setDisabled( state );
    mMiddleRadioButton->setDisabled( state );
    mBottomRadioButton->setDisabled( state );

    mLabel->beginCommand( tr( "Change Label Mode" ) );
    mLabel->blockSignals( true );
    mLabel->setMode( state ? QgsLayoutItemLabel::ModeHtml : QgsLayoutItemLabel::ModeFont );
    mLabel->setText( mTextEdit->toPlainText() );
    mLabel->update();
    mLabel->blockSignals( false );
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::mTextEdit_textChanged()
{
  if ( mLabel )
  {
    mLabel->beginCommand( tr( "Change Label Text" ), QgsLayoutItem::UndoLabelText );
    mLabel->blockSignals( true );
    mLabel->setText( mTextEdit->toPlainText() );
    mLabel->update();
    mLabel->blockSignals( false );
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::fontChanged()
{
  if ( mLabel )
  {
    QFont newFont = mFontButton->currentFont();
    mLabel->beginCommand( tr( "Change Label Font" ), QgsLayoutItem::UndoLabelFont );
    mLabel->setFont( newFont );
    mLabel->update();
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::justifyClicked()
{
  if ( mLabel )
  {
    mLabel->beginCommand( tr( "Change Label Alignment" ) );
    mLabel->setHAlign( Qt::AlignJustify );
    mLabel->update();
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::mMarginXDoubleSpinBox_valueChanged( double d )
{
  if ( mLabel )
  {
    mLabel->beginCommand( tr( "Change Label Margin" ), QgsLayoutItem::UndoLabelMargin );
    mLabel->setMarginX( d );
    mLabel->update();
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::mMarginYDoubleSpinBox_valueChanged( double d )
{
  if ( mLabel )
  {
    mLabel->beginCommand( tr( "Change Label Margin" ), QgsLayoutItem::UndoLabelMargin );
    mLabel->setMarginY( d );
    mLabel->update();
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::mFontColorButton_colorChanged( const QColor &newLabelColor )
{
  if ( !mLabel )
  {
    return;
  }

  mLabel->beginCommand( tr( "Change Label Color" ), QgsLayoutItem::UndoLabelFontColor );
  mLabel->setFontColor( newLabelColor );
  mLabel->update();
  mLabel->endCommand();
}

void QgsLayoutLabelWidget::mInsertExpressionButton_clicked()
{
  if ( !mLabel )
  {
    return;
  }

  QString selText = mTextEdit->textCursor().selectedText();

  // html editor replaces newlines with Paragraph Separator characters - see https://github.com/qgis/QGIS/issues/27568
  selText = selText.replace( QChar( 0x2029 ), QChar( '\n' ) );

  // edit the selected expression if there's one
  if ( selText.startsWith( QLatin1String( "[%" ) ) && selText.endsWith( QLatin1String( "%]" ) ) )
    selText = selText.mid( 2, selText.size() - 4 );

  // use the atlas coverage layer, if any
  QgsVectorLayer *layer = coverageLayer();

  QgsExpressionContext context = mLabel->createExpressionContext();
  QgsExpressionBuilderDialog exprDlg( layer, selText, this, QStringLiteral( "generic" ), context );

  exprDlg.setWindowTitle( tr( "Insert Expression" ) );
  if ( exprDlg.exec() == QDialog::Accepted )
  {
    QString expression = exprDlg.expressionText();
    if ( !expression.isEmpty() )
    {
      mLabel->beginCommand( tr( "Insert expression" ) );
      mTextEdit->insertPlainText( "[%" + expression + "%]" );
      mLabel->endCommand();
    }
  }
}

void QgsLayoutLabelWidget::mCenterRadioButton_clicked()
{
  if ( mLabel )
  {
    mLabel->beginCommand( tr( "Change Label Alignment" ) );
    mLabel->setHAlign( Qt::AlignHCenter );
    mLabel->update();
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::mRightRadioButton_clicked()
{
  if ( mLabel )
  {
    mLabel->beginCommand( tr( "Change Label Alignment" ) );
    mLabel->setHAlign( Qt::AlignRight );
    mLabel->update();
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::mLeftRadioButton_clicked()
{
  if ( mLabel )
  {
    mLabel->beginCommand( tr( "Change Label Alignment" ) );
    mLabel->setHAlign( Qt::AlignLeft );
    mLabel->update();
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::mTopRadioButton_clicked()
{
  if ( mLabel )
  {
    mLabel->beginCommand( tr( "Change Label Alignment" ) );
    mLabel->setVAlign( Qt::AlignTop );
    mLabel->update();
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::mBottomRadioButton_clicked()
{
  if ( mLabel )
  {
    mLabel->beginCommand( tr( "Change Label Alignment" ) );
    mLabel->setVAlign( Qt::AlignBottom );
    mLabel->update();
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::mMiddleRadioButton_clicked()
{
  if ( mLabel )
  {
    mLabel->beginCommand( tr( "Change Label Alignment" ) );
    mLabel->setVAlign( Qt::AlignVCenter );
    mLabel->update();
    mLabel->endCommand();
  }
}

void QgsLayoutLabelWidget::setGuiElementValues()
{
  blockAllSignals( true );
  mTextEdit->setPlainText( mLabel->text() );
  mTextEdit->moveCursor( QTextCursor::End, QTextCursor::MoveAnchor );
  mMarginXDoubleSpinBox->setValue( mLabel->marginX() );
  mMarginYDoubleSpinBox->setValue( mLabel->marginY() );
  mHtmlCheckBox->setChecked( mLabel->mode() == QgsLayoutItemLabel::ModeHtml );
  mTopRadioButton->setChecked( mLabel->vAlign() == Qt::AlignTop );
  mMiddleRadioButton->setChecked( mLabel->vAlign() == Qt::AlignVCenter );
  mBottomRadioButton->setChecked( mLabel->vAlign() == Qt::AlignBottom );
  mLeftRadioButton->setChecked( mLabel->hAlign() == Qt::AlignLeft );
  mJustifyRadioButton->setChecked( mLabel->hAlign() == Qt::AlignJustify );
  mCenterRadioButton->setChecked( mLabel->hAlign() == Qt::AlignHCenter );
  mRightRadioButton->setChecked( mLabel->hAlign() == Qt::AlignRight );
  mFontColorButton->setColor( mLabel->fontColor() );
  mFontButton->setCurrentFont( mLabel->font() );
  mVerticalAlignementLabel->setDisabled( mLabel->mode() == QgsLayoutItemLabel::ModeHtml );
  mTopRadioButton->setDisabled( mLabel->mode() == QgsLayoutItemLabel::ModeHtml );
  mMiddleRadioButton->setDisabled( mLabel->mode() == QgsLayoutItemLabel::ModeHtml );
  mBottomRadioButton->setDisabled( mLabel->mode() == QgsLayoutItemLabel::ModeHtml );

  blockAllSignals( false );
}

void QgsLayoutLabelWidget::blockAllSignals( bool block )
{
  mTextEdit->blockSignals( block );
  mHtmlCheckBox->blockSignals( block );
  mMarginXDoubleSpinBox->blockSignals( block );
  mMarginYDoubleSpinBox->blockSignals( block );
  mTopRadioButton->blockSignals( block );
  mMiddleRadioButton->blockSignals( block );
  mBottomRadioButton->blockSignals( block );
  mLeftRadioButton->blockSignals( block );
  mCenterRadioButton->blockSignals( block );
  mRightRadioButton->blockSignals( block );
  mJustifyRadioButton->blockSignals( block );
  mFontColorButton->blockSignals( block );
  mFontButton->blockSignals( block );
}
