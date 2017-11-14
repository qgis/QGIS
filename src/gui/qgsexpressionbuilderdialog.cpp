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

QgsExpressionBuilderDialog::QgsExpressionBuilderDialog( QgsVectorLayer *layer, const QString &startText, QWidget *parent, const QString &key, const QgsExpressionContext &context )
  : QDialog( parent )
  , mRecentKey( key )
{
  setupUi( this );

  QPushButton *okButton = buttonBox->button( QDialogButtonBox::Ok );
  connect( builder, &QgsExpressionBuilderWidget::expressionParsed, okButton, &QWidget::setEnabled );

  builder->setExpressionContext( context );
  builder->setLayer( layer );
  builder->setExpressionText( startText );
  builder->loadFieldNames();
  builder->loadRecent( mRecentKey );

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/ExpressionBuilderDialog/geometry" ) ).toByteArray() );

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

  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/ExpressionBuilderDialog/geometry" ), saveGeometry() );
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

void QgsExpressionBuilderDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_vector/expression.html" ) );
}
