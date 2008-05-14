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

QgsAddAttrDialog::QgsAddAttrDialog(QgsVectorDataProvider* provider, QWidget *parent, Qt::WFlags fl)
: QDialog(parent, fl), mDataProvider(provider)
{
    setupUi(this);

    //fill data types into the combo box    
    const QSet<QString>& typelist=mDataProvider->supportedNativeTypes();

    for(QSet<QString>::const_iterator it = typelist.constBegin(); it != typelist.constEnd(); ++it)
      {
	mTypeBox->insertItem(*it);
      }
}

QgsAddAttrDialog::QgsAddAttrDialog(const std::list<QString>& typelist, QWidget *parent, Qt::WFlags fl)
: QDialog(parent, fl), mDataProvider(0)
{
    setupUi(this);

    for(std::list<QString>::const_iterator iter=typelist.begin();iter!=typelist.end();++iter)
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
