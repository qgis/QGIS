/***************************************************************************
                         qgsdelattrdialog.cpp  -  description
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

#include "qgsdelattrdialog.h"
#include "qgsfield.h"
#include <qheader.h>
#include <qlistbox.h>

QgsDelAttrDialog::QgsDelAttrDialog(QHeader* header)
{
    QObject::connect((QObject*)mOkButton, SIGNAL(clicked()), this, SLOT(accept()));
    QObject::connect((QObject*)mCancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    //insert attribute names into the QListView
    if(header)
    {
	listBox2->clear();
	for(int i=1;i<header->count();++i)
	{
	    listBox2->insertItem(header->label(i));
	}
    }
}

const std::list<QString>* QgsDelAttrDialog::selectedAttributes()
{
    mSelectedItems.clear();
    for(int i=0;i<listBox2->numRows();++i)
    {
	if(listBox2->isSelected(i))
	{
	    mSelectedItems.push_back(listBox2->text(i));
	}
    }
    return &mSelectedItems;
}
