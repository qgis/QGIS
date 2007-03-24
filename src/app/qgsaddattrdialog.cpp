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
    connect(mOkButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(mCancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    //fill data types into the combo box
    
    // TODO: [MD]    
    const QSet<QString>& numlist=mDataProvider->numericalTypes();
    const QSet<QString>& anumlist=mDataProvider->nonNumericalTypes();

    for(QSet<QString>::const_iterator it = numlist.constBegin(); it != numlist.constEnd(); ++it)
      {
	mTypeBox->insertItem(*it);
      }
    for(QSet<QString>::const_iterator it = anumlist.constBegin(); it != anumlist.constEnd(); ++it)
      {
	mTypeBox->insertItem(*it);
      }
}

QgsAddAttrDialog::QgsAddAttrDialog(const std::list<QString>& typelist, QWidget *parent, Qt::WFlags fl)
: QDialog(parent, fl), mDataProvider(0)
{
    setupUi(this);
    connect(mOkButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(mCancelButton, SIGNAL(clicked()), this, SLOT(reject()));

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
