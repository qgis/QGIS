/***************************************************************************
                         qgsgeomtypedialog.cpp  -  description
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
/* $Id$ */

#include "qgsaddattrdialog.h"
#include "qgsgeomtypedialog.h"
#include <qlistview.h>
#include <qpushbutton.h>
#include <qradiobutton.h>

QgsGeomTypeDialog::QgsGeomTypeDialog(): QgsGeomTypeDialogBase()
{
    QObject::connect((QObject*)mOkButton, SIGNAL(clicked()), this, SLOT(accept()));
    QObject::connect((QObject*)mCancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    QObject::connect((QObject*)mAddAttributeButton, SIGNAL(clicked()), this, SLOT(addAttribute()));
    QObject::connect((QObject*)mRemoveAttributeButton, SIGNAL(clicked()), this, SLOT(removeAttribute()));
    mPointRadioButton->setChecked(true);
    mAttributeView->removeColumn(0);
    mAttributeView->addColumn(tr("Name"));
    mAttributeView->addColumn(tr("Type"));
}

QgsGeomTypeDialog::~QgsGeomTypeDialog()
{

}

QGis::WKBTYPE QgsGeomTypeDialog::selectedType()
{
    if(mPointRadioButton->isChecked())
    {
	return QGis::WKBPoint;
    }
    else if(mLineRadioButton->isChecked())
    {
	return QGis::WKBLineString;
    }
    else if(mPolygonRadioButton->isChecked())
    {
	return QGis::WKBPolygon;
    }

    return QGis::WKBUnknown;
}

void QgsGeomTypeDialog::addAttribute()
{
    std::list<QString> types;
    types.push_back("Real");
    types.push_back("Integer");
    types.push_back("String");
    QgsAddAttrDialog d(types);
    if(d.exec()==QDialog::Accepted)
    {
	QListViewItem* attritem=new QListViewItem(mAttributeView, d.name(), d.type());
    }
    if(mAttributeView->childCount()>0)
    {
	mOkButton->setEnabled(true);
    }
}

void QgsGeomTypeDialog::removeAttribute()
{
    delete(mAttributeView->currentItem());
    if(mAttributeView->childCount()==0)
    {
	mOkButton->setEnabled(false);	
    }
    
}

void QgsGeomTypeDialog::attributes(std::list<std::pair<QString, QString> >& at) const
{
    QListViewItemIterator it(mAttributeView);
    while ( it.current() ) 
    {
	QListViewItem *item = it.current();
	at.push_back(std::make_pair(item->text(0), item->text(1)));
#ifdef QGISDEBUG
	qWarning("appending "+item->text(0)+"//"+item->text(1));
#endif	
	++it;
    }
}
