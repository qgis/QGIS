/***************************************************************************
                         qgsaddattrdialog.h  -  description
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaddattrdialog.h"
#include "qgsvectordataprovider.h"
#include <qcombobox.h>
#include <qlineedit.h>

QgsAddAttrDialog::QgsAddAttrDialog(QgsVectorDataProvider* provider): QgsAddAttrDialogBase(), mDataProvider(provider)
{
    QObject::connect((QObject*)mOkButton, SIGNAL(clicked()), this, SLOT(accept()));
    QObject::connect((QObject*)mCancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    //fill data types into the combo box
    const std::list<QString>& numlist=mDataProvider->numericalTypes();
    const std::list<QString>& anumlist=mDataProvider->nonNumericalTypes();

    for(std::list<QString>::const_iterator iter=numlist.begin();iter!=numlist.end();++iter)
    {
	mTypeBox->insertItem(*iter);
    }
    for(std::list<QString>::const_iterator iter=anumlist.begin();iter!=anumlist.end();++iter)
    {
	mTypeBox->insertItem(*iter);
    }
}

QString QgsAddAttrDialog::name() const
{
    return mNameEdit->text();
}

QString QgsAddAttrDialog::type() const
{
    return mTypeBox->currentText();
}
