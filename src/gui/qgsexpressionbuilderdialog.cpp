/***************************************************************************
    qgisexpressionbuilderdialog.h - A genric expression string builder dialog.
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
#include <QSettings>

QgsExpressionBuilderDialog::QgsExpressionBuilderDialog( QgsVectorLayer* layer, QString startText, QWidget* parent, QString key )
    : QDialog( parent ), mRecentKey( key )
{
  setupUi( this );

  QPushButton* okButton = buttonBox->button( QDialogButtonBox::Ok );
  connect( builder, SIGNAL( expressionParsed( bool ) ), okButton, SLOT( setEnabled( bool ) ) );

  builder->setLayer( layer );
  builder->setExpressionText( startText );
  builder->loadFieldNames();
  builder->loadRecent( mRecentKey );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/ExpressionBuilderDialog/geometry" ).toByteArray() );
}

QgsExpressionBuilderWidget* QgsExpressionBuilderDialog::expressionBuilder()
{
  return builder;
}

void QgsExpressionBuilderDialog::setExpressionText( const QString& text )
{
  builder->setExpressionText( text );
}

QString QgsExpressionBuilderDialog::expressionText()
{
  return builder->expressionText();
}

void QgsExpressionBuilderDialog::done( int r )
{
  QDialog::done( r );

  QSettings settings;
  settings.setValue( "/Windows/ExpressionBuilderDialog/geometry", saveGeometry() );
}

void QgsExpressionBuilderDialog::accept()
{
  builder->saveToRecent( mRecentKey );
  QDialog::accept();
}

void QgsExpressionBuilderDialog::setGeomCalculator( const QgsDistanceArea & da )
{
  // Store in child widget only.
  builder->setGeomCalculator( da );
}
