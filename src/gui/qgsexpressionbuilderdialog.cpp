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

QgsExpressionBuilderDialog::QgsExpressionBuilderDialog( QgsVectorLayer* layer, QString startText, QWidget* parent)
    :QDialog( parent )
{
    setupUi( this );

    QPushButton* okButuon = buttonBox->button(QDialogButtonBox::Ok);
    connect(builder, SIGNAL(expressionParsed(bool)), okButuon, SLOT(setEnabled(bool)));

    builder->setLayer( layer );
    builder->setExpressionString(startText);
    builder->loadFieldNames();

    QSettings settings;
    restoreGeometry( settings.value( "/Windows/ExpressionBuilderDialog/geometry" ).toByteArray() );
}

QgsExpressionBuilderWidget* QgsExpressionBuilderDialog::expressionBuilder()
{
    return builder;
}

void QgsExpressionBuilderDialog::setExpressionText( QString text )
{
    builder->setExpressionString( text );
}

void QgsExpressionBuilderDialog::closeEvent( QCloseEvent *event )
{
    QDialog::closeEvent( event );

    // TODO Work out why this is not working yet.
    QSettings settings;
    settings.setValue( "/Windows/ExpressionBuilderDialog/geometry", saveGeometry() );
}
