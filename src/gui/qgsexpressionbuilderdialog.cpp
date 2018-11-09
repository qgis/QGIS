/***************************************************************************
    qgisexpressionbuilderdialog.h - A generic expression string builder dialog.
     --------------------------------------
    Date                 :  29-May-2011
    Copyright            : (C) 2011 by Nathan Woodrow
    Email                : woodrow.nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexpressionbuilderdialog.h"
#include "qgssettings.h"
#include "qgsguiutils.h"
#include "qgsgui.h"

QgsExpressionBuilderDialog::QgsExpressionBuilderDialog( QgsVectorLayer *layer, const QString &startText, QWidget *parent, const QString &key, const QgsExpressionContext &context )
  : QDialog( parent )
  , mRecentKey( key )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( builder, &QgsExpressionBuilderWidget::parserErrorChanged, this, &QgsExpressionBuilderDialog::syncOkButtonEnabledState );
  connect( builder, &QgsExpressionBuilderWidget::evalErrorChanged, this, &QgsExpressionBuilderDialog::syncOkButtonEnabledState );

  builder->setExpressionContext( context );
  builder->setLayer( layer );
  builder->setExpressionText( startText );
  builder->loadFieldNames();
  builder->loadRecent( mRecentKey );

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsExpressionBuilderDialog::showHelp );
}

QgsExpressionBuilderWidget *QgsExpressionBuilderDialog::expressionBuilder()
{
  return builder;
}

void QgsExpressionBuilderDialog::setExpressionText( const QString &text )
{
  builder->setExpressionText( text );
}

QString QgsExpressionBuilderDialog::expressionText()
{
  return builder->expressionText();
}

QString QgsExpressionBuilderDialog::expectedOutputFormat()
{
  return builder->expectedOutputFormat();
}

void QgsExpressionBuilderDialog::setExpectedOutputFormat( const QString &expected )
{
  builder->setExpectedOutputFormat( expected );
}

QgsExpressionContext QgsExpressionBuilderDialog::expressionContext() const
{
  return builder->expressionContext();
}

void QgsExpressionBuilderDialog::setExpressionContext( const QgsExpressionContext &context )
{
  builder->setExpressionContext( context );
}

void QgsExpressionBuilderDialog::done( int r )
{
  QDialog::done( r );
}

void QgsExpressionBuilderDialog::accept()
{
  builder->saveToRecent( mRecentKey );
  QDialog::accept();
}

void QgsExpressionBuilderDialog::setGeomCalculator( const QgsDistanceArea &da )
{
  // Store in child widget only.
  builder->setGeomCalculator( da );
}

bool QgsExpressionBuilderDialog::allowEvalErrors() const
{
  return mAllowEvalErrors;
}

void QgsExpressionBuilderDialog::setAllowEvalErrors( bool allowEvalErrors )
{
  if ( allowEvalErrors == mAllowEvalErrors )
    return;

  mAllowEvalErrors = allowEvalErrors;
  syncOkButtonEnabledState();
  emit allowEvalErrorsChanged();
}

void QgsExpressionBuilderDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_vector/expression.html" ) );
}

void QgsExpressionBuilderDialog::syncOkButtonEnabledState()
{
  QPushButton *okButton = buttonBox->button( QDialogButtonBox::Ok );

  if ( builder->parserError() )
    okButton->setEnabled( false );
  else if ( !builder->evalError() || mAllowEvalErrors )
    okButton->setEnabled( true );
  else
    okButton->setEnabled( true );
}
