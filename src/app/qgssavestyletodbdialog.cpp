/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssavestyletodbdialog.h"

QgsSaveStyleToDbDialog::QgsSaveStyleToDbDialog( QWidget *parent ) :
    QDialog( parent )
{
    setupUi( this );
    setWindowTitle( "Save style in Postgres" );
    mDescriptionEdit->setTabChangesFocus( true );
    setTabOrder( mNameEdit, mDescriptionEdit );
    setTabOrder( mDescriptionEdit, mUseAsDefault );
    setTabOrder( mUseAsDefault, buttonBox );
}
QString QgsSaveStyleToDbDialog::getName()
{
    return mNameEdit->text();
}
QString QgsSaveStyleToDbDialog::getDescription()
{
    return mDescriptionEdit->toPlainText();
}
bool QgsSaveStyleToDbDialog::isDefault()
{
    return mUseAsDefault->isChecked();
}
