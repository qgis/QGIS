/***************************************************************************
    qgsencodingfiledialog.cpp - File dialog which queries the encoding type
     --------------------------------------
    Date                 : 16-Feb-2005
    Copyright            : (C) 2005 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsencodingfiledialog.h"
#include <qlabel.h>

QgsEncodingFileDialog::QgsEncodingFileDialog(const QString & dirName, const QString& filter, QWidget * parent, const char * name): QFileDialog(dirName, filter, parent, name)
{
    mEncodingComboBox=new QComboBox(this);
    QLabel* l=new QLabel(tr("Encoding:"),this);
    addWidgets(l,mEncodingComboBox,0);
    mEncodingComboBox->insertItem("Ascii",0);
    mEncodingComboBox->insertItem("Latin1",1);
    mEncodingComboBox->insertItem("Local8Bit",2);
    mEncodingComboBox->insertItem("Utf8",3);
    mEncodingComboBox->setCurrentItem(3);//make Utf8 the default
}

QgsEncodingFileDialog::~QgsEncodingFileDialog()
{
    
}

QgsVectorDataProvider::Encoding QgsEncodingFileDialog::encoding() const
{
    switch(mEncodingComboBox->currentItem())
    {
	case 0:
	    return QgsVectorDataProvider::Ascii;
	case 1:
	    return QgsVectorDataProvider::Latin1;
	case 2:
	    return QgsVectorDataProvider::Local8Bit;
	case 3:
	    return QgsVectorDataProvider::Utf8;
    }
}
