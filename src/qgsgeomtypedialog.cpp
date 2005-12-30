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

#include "qgsgeomtypedialog.h"
#include "qgsaddattrdialog.h"

QgsGeomTypeDialog::QgsGeomTypeDialog(): QDialog()
{
    setupUi(this);
    connect(mOkButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(mCancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    mPointRadioButton->setChecked(true);
    mAttributeView->removeColumn(0);
    mAttributeView->addColumn(tr("Name"));
    mAttributeView->addColumn(tr("Type"));
    mFileFormatComboBox->insertItem("ESRI Shapefile");
    /*mFileFormatComboBox->insertItem("Comma Separated Value");
    mFileFormatComboBox->insertItem("GML");
    mFileFormatComboBox->insertItem("Mapinfo File");*/
}

QgsGeomTypeDialog::~QgsGeomTypeDialog()
{

}

QGis::WKBTYPE QgsGeomTypeDialog::selectedType() const
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

void QgsGeomTypeDialog::on_mAddAttributeButton_clicked()
{
    std::list<QString> types;
    types.push_back("Real");
    types.push_back("Integer");
    types.push_back("String");
    QgsAddAttrDialog d(types);
    if(d.exec()==QDialog::Accepted)
    {
	Q3ListViewItem* attritem=new Q3ListViewItem(mAttributeView, d.name(), d.type());
    }
    if(mAttributeView->childCount()>0)
    {
	mOkButton->setEnabled(true);
    }
}

void QgsGeomTypeDialog::on_mRemoveAttributeButton_clicked()
{
    delete(mAttributeView->currentItem());
    if(mAttributeView->childCount()==0)
    {
	mOkButton->setEnabled(false);	
    }
    
}

void QgsGeomTypeDialog::attributes(std::list<std::pair<QString, QString> >& at) const
{
    Q3ListViewItemIterator it(mAttributeView);
    while ( it.current() ) 
    {
	Q3ListViewItem *item = it.current();
	at.push_back(std::make_pair(item->text(0), item->text(1)));
#ifdef QGISDEBUG
	qWarning(("appending "+item->text(0)+"//"+item->text(1)).toLocal8Bit().data());
#endif	
	++it;
    }
}

QString QgsGeomTypeDialog::selectedFileFormat() const
{
    return mFileFormatComboBox->currentText();
}
