/***************************************************************************
   qgsexpressionlineedit.cpp
    ------------------------
   Date                 : 18.08.2016
   Copyright            : (C) 2016 Nyall Dawson
   Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgsexpressionlineedit.h"
#include "qgsfilterlineedit.h"
#include "qgsexpressioncontext.h"
#include "qgsapplication.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsexpressioncontextgenerator.h"
#include "qgscodeeditorsql.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsexpressioncontextutils.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>


QgsExpressionLineEdit::QgsExpressionLineEdit( QWidget *parent )
  : QWidget( parent )
  , mExpressionDialogTitle( tr( "Expression Dialog" ) )
{
  mButton = new QToolButton();
  mButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  mButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconExpression.svg" ) ) );
  connect( mButton, &QAbstractButton::clicked, this, &QgsExpressionLineEdit::editExpression );

  //sets up layout
  setMultiLine( false );

  mExpressionContext = QgsExpressionContext();
  mExpressionContext << QgsExpressionContextUtils::globalScope()
                     << QgsExpressionContextUtils::projectScope( QgsProject::instance() );
}

QgsExpressionLineEdit::~QgsExpressionLineEdit() = default;

void QgsExpressionLineEdit::setExpressionDialogTitle( const QString &title )
{
  mExpressionDialogTitle = title;
}

void QgsExpressionLineEdit::setMultiLine( bool multiLine )
{
  const QString exp = expression();

  if ( multiLine && !mCodeEditor )
  {
    mCodeEditor = new QgsCodeEditorExpression();
    mCodeEditor->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    delete mLineEdit;
    mLineEdit = nullptr;

    QHBoxLayout *newLayout = new QHBoxLayout();
    newLayout->setContentsMargins( 0, 0, 0, 0 );
    newLayout->addWidget( mCodeEditor );

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addWidget( mButton );
    vLayout->addStretch();
    newLayout->addLayout( vLayout );

    delete layout();
    setLayout( newLayout );

    setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    setFocusProxy( mCodeEditor );
    connect( mCodeEditor, &QsciScintilla::textChanged, this, static_cast < void ( QgsExpressionLineEdit::* )() > ( &QgsExpressionLineEdit::expressionEdited ) );

    setExpression( exp );
  }
  else if ( !multiLine && !mLineEdit )
  {
    delete mCodeEditor;
    mCodeEditor = nullptr;
    mLineEdit = new QgsFilterLineEdit();
    mLineEdit->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );

    QHBoxLayout *newLayout = new QHBoxLayout();
    newLayout->setContentsMargins( 0, 0, 0, 0 );
    newLayout->addWidget( mLineEdit );
    newLayout->addWidget( mButton );

    delete layout();
    setLayout( newLayout );

    setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );

    setFocusProxy( mLineEdit );
    connect( mLineEdit, &QLineEdit::textChanged, this, static_cast < void ( QgsExpressionLineEdit::* )( const QString & ) > ( &QgsExpressionLineEdit::expressionEdited ) );

    setExpression( exp );
  }
}

QString QgsExpressionLineEdit::expectedOutputFormat() const
{
  return mExpectedOutputFormat;
}

void QgsExpressionLineEdit::setExpectedOutputFormat( const QString &expected )
{
  mExpectedOutputFormat = expected;
}

void QgsExpressionLineEdit::setGeomCalculator( const QgsDistanceArea &da )
{
  mDa.reset( new QgsDistanceArea( da ) );
}

void QgsExpressionLineEdit::setLayer( QgsVectorLayer *layer )
{
  if ( !mExpressionContextGenerator || mExpressionContextGenerator == mLayer )
    mExpressionContextGenerator = layer;
  mLayer = layer;
}

QString QgsExpressionLineEdit::expression() const
{
  if ( mLineEdit )
    return mLineEdit->text();
  else if ( mCodeEditor )
    return mCodeEditor->text();

  return QString();
}

bool QgsExpressionLineEdit::isValidExpression( QString *expressionError ) const
{
  QString temp;
  const QgsExpressionContext context = mExpressionContextGenerator ? mExpressionContextGenerator->createExpressionContext() : mExpressionContext;
  return QgsExpression::checkExpression( expression(), &context, expressionError ? *expressionError : temp );
}

void QgsExpressionLineEdit::registerExpressionContextGenerator( const QgsExpressionContextGenerator *generator )
{
  mExpressionContextGenerator = generator;
}

void QgsExpressionLineEdit::setExpression( const QString &newExpression )
{
  if ( mLineEdit )
    mLineEdit->setText( newExpression );
  else if ( mCodeEditor )
    mCodeEditor->setText( newExpression );
}

void QgsExpressionLineEdit::editExpression()
{
  const QString currentExpression = expression();

  const QgsExpressionContext context = mExpressionContextGenerator ? mExpressionContextGenerator->createExpressionContext() : mExpressionContext;

  QgsExpressionBuilderDialog dlg( mLayer, currentExpression, this, QStringLiteral( "generic" ), context );
  dlg.setExpectedOutputFormat( mExpectedOutputFormat );
  if ( mDa )
  {
    dlg.setGeomCalculator( *mDa );
  }
  dlg.setWindowTitle( mExpressionDialogTitle );

  if ( dlg.exec() )
  {
    const QString newExpression = dlg.expressionText();
    setExpression( newExpression );
  }
}

void QgsExpressionLineEdit::expressionEdited()
{
  emit expressionChanged( expression() );
}

void QgsExpressionLineEdit::expressionEdited( const QString &expression )
{
  updateLineEditStyle( expression );
  emit expressionChanged( expression );
}

void QgsExpressionLineEdit::changeEvent( QEvent *event )
{
  if ( event->type() == QEvent::EnabledChange )
  {
    updateLineEditStyle( expression() );
  }
}

void QgsExpressionLineEdit::updateLineEditStyle( const QString &expression )
{
  if ( !mLineEdit )
    return;

  QPalette palette = mLineEdit->palette();
  if ( !isEnabled() )
  {
    palette.setColor( QPalette::Text, Qt::gray );
  }
  else
  {
    bool isValid = true;
    if ( !expression.isEmpty() )
    {
      isValid = isExpressionValid( expression );
    }
    if ( !isValid )
    {
      palette.setColor( QPalette::Text, Qt::red );
    }
    else
    {
      palette.setColor( QPalette::Text, Qt::black );
    }
  }
  mLineEdit->setPalette( palette );
}

bool QgsExpressionLineEdit::isExpressionValid( const QString &expressionStr )
{
  QgsExpression expression( expressionStr );

  const QgsExpressionContext context = mExpressionContextGenerator ? mExpressionContextGenerator->createExpressionContext() : mExpressionContext;
  expression.prepare( &mExpressionContext );
  return !expression.hasParserError();
}
