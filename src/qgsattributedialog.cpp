/***************************************************************************
                         qgsattributedialog.cpp  -  description
                             -------------------
    begin                : October 2004
    copyright            : (C) 2004 by Marco Hugentobler
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

#include "qgsattributedialog.h"
#include <qheader.h>
#include <qtable.h>

QgsAttributeDialog::QgsAttributeDialog(std::vector<QgsFeatureAttribute>* attributes): QgsAttributeDialogBase()
{
    QHeader* header=mTable->horizontalHeader();
    header->setLabel(0,"field");
    header->setLabel(1,"value");
    mTable->setNumRows(attributes->size());

    int index=0;
    for(std::vector<QgsFeatureAttribute>::iterator it=attributes->begin();it!=attributes->end();++it)
    {
	mTable->setText(index,0,(*it).fieldName());
	mTable->setText(index,1,(*it).fieldValue());
	++index;
    }

    QObject::connect((QObject*)mOkButton, SIGNAL(clicked()), this, SLOT(accept()));//why is this cast necessary????
    QObject::connect((QObject*)mCancelButton, SIGNAL(clicked()), this, SLOT(reject()));//why is this cast necessary????
}

QgsAttributeDialog::~QgsAttributeDialog()
{

}

QString QgsAttributeDialog::value(int row)
{
    return mTable->text(row,1);
}
