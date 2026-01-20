/***************************************************************************
                         qgsprocessingrastercalculatorexpressionlineedit.cpp
                         ---------------------
    begin                : July 2023
    copyright            : (C) 2023 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingrastercalculatorexpressionlineedit.h"

#include "qgsapplication.h"
#include "qgsfilterlineedit.h"
#include "qgsgui.h"
#include "qgsmaplayer.h"

#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QToolButton>

#include "moc_qgsprocessingrastercalculatorexpressionlineedit.cpp"

/// @cond PRIVATE

QgsProcessingRasterCalculatorExpressionLineEdit::QgsProcessingRasterCalculatorExpressionLineEdit( QWidget *parent )
  : QWidget( parent )
{
  mLineEdit = new QgsFilterLineEdit();
  mLineEdit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );

  mButton = new QToolButton();
  mButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  mButton->setIcon( QgsApplication::getThemeIcon( u"/mIconExpression.svg"_s ) );
  connect( mButton, &QAbstractButton::clicked, this, &QgsProcessingRasterCalculatorExpressionLineEdit::editExpression );

  QHBoxLayout *layout = new QHBoxLayout();
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->addWidget( mLineEdit );
  layout->addWidget( mButton );
  setLayout( layout );

  setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
  setFocusProxy( mLineEdit );
  connect( mLineEdit, &QLineEdit::textChanged, this, static_cast<void ( QgsProcessingRasterCalculatorExpressionLineEdit::* )( const QString & )>( &QgsProcessingRasterCalculatorExpressionLineEdit::expressionEdited ) );

  setExpression( expression() );
}

QgsProcessingRasterCalculatorExpressionLineEdit::~QgsProcessingRasterCalculatorExpressionLineEdit() = default;

void QgsProcessingRasterCalculatorExpressionLineEdit::setLayers( const QVariantList &layers )
{
  mLayers = layers;
}

QString QgsProcessingRasterCalculatorExpressionLineEdit::expression() const
{
  if ( mLineEdit )
    return mLineEdit->text();

  return QString();
}

void QgsProcessingRasterCalculatorExpressionLineEdit::setExpression( const QString &newExpression )
{
  if ( mLineEdit )
    mLineEdit->setText( newExpression );
}

void QgsProcessingRasterCalculatorExpressionLineEdit::editExpression()
{
  const QString currentExpression = expression();
  QgsProcessingRasterCalculatorExpressionDialog dlg( mLayers );
  dlg.setExpression( currentExpression );

  if ( dlg.exec() )
  {
    const QString newExpression = dlg.expression();
    setExpression( newExpression );
  }
}

void QgsProcessingRasterCalculatorExpressionLineEdit::expressionEdited()
{
  emit expressionChanged( expression() );
}

void QgsProcessingRasterCalculatorExpressionLineEdit::expressionEdited( const QString &expression )
{
  emit expressionChanged( expression );
}


QgsProcessingRasterCalculatorExpressionDialog::QgsProcessingRasterCalculatorExpressionDialog( const QVariantList &layers, const QString &startExpression, QWidget *parent )
  : QDialog( parent )
  , mLayers( layers )
  , mInitialText( startExpression )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  populateLayers();

  connect( mLayersList, &QListWidget::itemDoubleClicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mLayersList_itemDoubleClicked );

  connect( mBtnPlus, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnPlus_clicked );
  connect( mBtnMinus, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnMinus_clicked );
  connect( mBtnMultiply, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnMultiply_clicked );
  connect( mBtnDivide, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnDivide_clicked );
  connect( mBtnPower, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnPower_clicked );
  connect( mBtnSqrt, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnSqrt_clicked );
  connect( mBtnOpenBracket, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnOpenBracket_clicked );
  connect( mBtnCloseBracket, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnCloseBracket_clicked );
  connect( mBtnGreater, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnGreater_clicked );
  connect( mBtnGreaterEqual, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnGreaterEqual_clicked );
  connect( mBtnLess, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnLess_clicked );
  connect( mBtnLessEqual, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnLessEqual_clicked );
  connect( mBtnEqual, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnEqual_clicked );
  connect( mBtnNotEqual, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnNotEqual_clicked );
  connect( mBtnAnd, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnAnd_clicked );
  connect( mBtnOr, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnOr_clicked );
  connect( mBtnIf, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnIf_clicked );
  connect( mBtnMin, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnMin_clicked );
  connect( mBtnMax, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnMax_clicked );
  connect( mBtnAbs, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnAbs_clicked );
  connect( mBtnSin, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnSin_clicked );
  connect( mBtnCos, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnCos_clicked );
  connect( mBtnTan, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnTan_clicked );
  connect( mBtnLog, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnLog_clicked );
  connect( mBtnAsin, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnAsin_clicked );
  connect( mBtnAcos, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnAcos_clicked );
  connect( mBtnAtan, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnAtan_clicked );
  connect( mBtnLn, &QPushButton::clicked, this, &QgsProcessingRasterCalculatorExpressionDialog::mBtnLn_clicked );

  mExpressionTextEdit->setPlainText( mInitialText );
}

void QgsProcessingRasterCalculatorExpressionDialog::setExpression( const QString &text )
{
  mExpressionTextEdit->setPlainText( text );
}

QString QgsProcessingRasterCalculatorExpressionDialog::expression()
{
  return mExpressionTextEdit->toPlainText();
}

void QgsProcessingRasterCalculatorExpressionDialog::populateLayers()
{
  if ( mLayers.isEmpty() )
  {
    return;
  }

  for ( const QVariant &layer : mLayers )
  {
    QListWidgetItem *item = new QListWidgetItem( layer.toString(), mLayersList );
    mLayersList->addItem( item );
  }
}

QString QgsProcessingRasterCalculatorExpressionDialog::quoteBandEntry( const QString &layerName )
{
  // '"' -> '\\"'
  QString quotedName = layerName;
  quotedName.replace( '\"', "\\\""_L1 );
  quotedName.append( '\"' );
  quotedName.prepend( '\"' );
  return quotedName;
}

void QgsProcessingRasterCalculatorExpressionDialog::mLayersList_itemDoubleClicked( QListWidgetItem *item )
{
  mExpressionTextEdit->insertPlainText( quoteBandEntry( u"%1@1"_s.arg( item->text() ) ) );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnPlus_clicked()
{
  mExpressionTextEdit->insertPlainText( u" + "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnMinus_clicked()
{
  mExpressionTextEdit->insertPlainText( u" - "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnMultiply_clicked()
{
  mExpressionTextEdit->insertPlainText( u" * "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnDivide_clicked()
{
  mExpressionTextEdit->insertPlainText( u" / "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnPower_clicked()
{
  mExpressionTextEdit->insertPlainText( u" ^ "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnSqrt_clicked()
{
  mExpressionTextEdit->insertPlainText( u" sqrt ( "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnOpenBracket_clicked()
{
  mExpressionTextEdit->insertPlainText( u" ( "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnCloseBracket_clicked()
{
  mExpressionTextEdit->insertPlainText( u" ) "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnGreater_clicked()
{
  mExpressionTextEdit->insertPlainText( u" > "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnGreaterEqual_clicked()
{
  mExpressionTextEdit->insertPlainText( u" >= "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnLess_clicked()
{
  mExpressionTextEdit->insertPlainText( u" < "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnLessEqual_clicked()
{
  mExpressionTextEdit->insertPlainText( u" <= "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnEqual_clicked()
{
  mExpressionTextEdit->insertPlainText( u" = "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnNotEqual_clicked()
{
  mExpressionTextEdit->insertPlainText( u" != "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnAnd_clicked()
{
  mExpressionTextEdit->insertPlainText( u" AND "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnOr_clicked()
{
  mExpressionTextEdit->insertPlainText( u" OR "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnIf_clicked()
{
  mExpressionTextEdit->insertPlainText( u" if ( "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnMin_clicked()
{
  mExpressionTextEdit->insertPlainText( u" MIN ( "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnMax_clicked()
{
  mExpressionTextEdit->insertPlainText( u" MAX ( "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnAbs_clicked()
{
  mExpressionTextEdit->insertPlainText( u" ABS ( "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnSin_clicked()
{
  mExpressionTextEdit->insertPlainText( u" sin ( "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnCos_clicked()
{
  mExpressionTextEdit->insertPlainText( u" cos ( "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnTan_clicked()
{
  mExpressionTextEdit->insertPlainText( u" tan ( "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnLog_clicked()
{
  mExpressionTextEdit->insertPlainText( u" log10 ( "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnAsin_clicked()
{
  mExpressionTextEdit->insertPlainText( u" asin ( "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnAcos_clicked()
{
  mExpressionTextEdit->insertPlainText( u" acos ( "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnAtan_clicked()
{
  mExpressionTextEdit->insertPlainText( u" atan ( "_s );
}

void QgsProcessingRasterCalculatorExpressionDialog::mBtnLn_clicked()
{
  mExpressionTextEdit->insertPlainText( u" ln ( "_s );
}

///@endcond
