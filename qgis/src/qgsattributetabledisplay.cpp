/***************************************************************************
                          QgsAttributeTableDisplay.cpp  -  description
                             -------------------
    begin                : Sat Nov 23 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc dot com
       Romans 3:23=>Romans 6:23=>Romans 5:8=>Romans 10:9,10=>Romans 12
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
#include "qgsattributetable.h"
#include "qgsattributetabledisplay.h"
#include "qgsdelattrdialog.h"
#include "qgsvectorlayer.h"
#include <qlayout.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qpushbutton.h> 

QgsAttributeTableDisplay::QgsAttributeTableDisplay(QgsVectorLayer* layer):QgsAttributeTableBase(), mLayer(layer)
{
    //insert editing popup
    mMenuBar = new QMenuBar(this, "mMenuBar");
    mMenuBar->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)0, 0, 0, mMenuBar->sizePolicy().hasHeightForWidth() ) );
    mMenuBar->setMinimumSize( QSize( 0, 40 ) );

    QgsAttributeTableBaseLayout->addMultiCellWidget( mMenuBar, 0, 0, 0, 4 );

    QPopupMenu* edit = new QPopupMenu(this);
    edit->insertItem(tr("&Add Attribute..."), this, SLOT(addAttribute()), CTRL+Key_A);
    edit->insertItem(tr("&Delete Attributes..."), this, SLOT(deleteAttributes()), CTRL+Key_D);
    mMenuBar->insertItem(tr("&Edit"), edit);
    mMenuBar->setEnabled(false);

    btnStopEditing->setEnabled(false);
    if(!layer->getDataProvider()->supportsAttributeEditing())
    {
	btnStartEditing->setEnabled(false);
    }
    
    QObject::connect(btnStartEditing, SIGNAL(clicked()), this, SLOT(startEditing()));
    QObject::connect(btnStopEditing, SIGNAL(clicked()), this, SLOT(stopEditing()));
}

QgsAttributeTableDisplay::~QgsAttributeTableDisplay()
{
}
QgsAttributeTable *QgsAttributeTableDisplay::table()
{
  return tblAttributes;
}

void QgsAttributeTableDisplay::setTitle(QString title)
{
  setCaption(title);
}

void QgsAttributeTableDisplay::deleteAttributes()
{
    QgsDelAttrDialog dialog(table()->horizontalHeader());
	if(dialog.exec()==QDialog::Accepted)
	{
	    const std::list<QString>* attlist=dialog.selectedAttributes();
	    for(std::list<QString>::const_iterator iter=attlist->begin();iter!=attlist->end();++iter)
	    {
		table()->deleteAttribute(*iter);
	    }
	}
}

void QgsAttributeTableDisplay::addAttribute()
{
    QgsAddAttrDialog dialog(mLayer->getDataProvider());
    if(dialog.exec()==QDialog::Accepted)
    {
	if(!table()->addAttribute(dialog.name(),dialog.type()))
	{
	    QMessageBox::information(0,"Name conflict","The attribute could not be inserted. The name already exists in the table",QMessageBox::Ok);
	}
    }
}

void QgsAttributeTableDisplay::startEditing()
{
    btnStartEditing->setEnabled(false);
    btnStopEditing->setEnabled(true);
    btnClose->setEnabled(false);
    mMenuBar->setEnabled(true);
    table()->setReadOnly(false);
    table()->setColumnReadOnly(0,true);//id column is not editable
}

void QgsAttributeTableDisplay::stopEditing()
{
    if(table()->edited())
    {
	//commit or roll back?
	int commit=QMessageBox::information(0,"Stop editing","Do you want to save the changes?",QMessageBox::Yes,QMessageBox::No);
	if(commit==QMessageBox::Yes)
	{
	    table()->commitChanges(mLayer);
	}
	else
	{
	    table()->rollBack(mLayer);
	}
    }
    btnStartEditing->setEnabled(true);
    btnStopEditing->setEnabled(false);
    btnClose->setEnabled(true); 
    mMenuBar->setEnabled(false);
    table()->setReadOnly(true);
}
