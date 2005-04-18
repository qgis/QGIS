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
#include <qcombobox.h>
#include <qlabel.h>
#include <qtextcodec.h>


QgsEncodingFileDialog::QgsEncodingFileDialog(const QString & dirName, const QString& filter, QWidget * parent, const char * name, const QString currentencoding): QFileDialog(dirName, filter, parent, name)
{
    mEncodingComboBox=new QComboBox(this);
    QLabel* l=new QLabel(tr("Encoding:"),this);
    addWidgets(l,mEncodingComboBox,0);
    mEncodingComboBox->insertItem("BIG5"); 
    mEncodingComboBox->insertItem("BIG5-HKSCS"); 
    mEncodingComboBox->insertItem("EUCJP"); 
    mEncodingComboBox->insertItem("EUCKR"); 
    mEncodingComboBox->insertItem("GB2312"); 
    mEncodingComboBox->insertItem("GBK"); 
    mEncodingComboBox->insertItem("GB18030"); 
    mEncodingComboBox->insertItem("JIS7"); 
    mEncodingComboBox->insertItem("SHIFT-JIS"); 
    mEncodingComboBox->insertItem("TSCII"); 
    mEncodingComboBox->insertItem("UTF-8"); 
    mEncodingComboBox->insertItem("UTF-16"); 
    mEncodingComboBox->insertItem("KOI8-R"); 
    mEncodingComboBox->insertItem("KOI8-U"); 
    mEncodingComboBox->insertItem("ISO8859-1"); 
    mEncodingComboBox->insertItem("ISO8859-2");
    mEncodingComboBox->insertItem("ISO8859-3"); 
    mEncodingComboBox->insertItem("ISO8859-4"); 
    mEncodingComboBox->insertItem("ISO8859-5"); 
    mEncodingComboBox->insertItem("ISO8859-6");
    mEncodingComboBox->insertItem("ISO8859-7"); 
    mEncodingComboBox->insertItem("ISO8859-8"); 
    mEncodingComboBox->insertItem("ISO8859-8-I"); 
    mEncodingComboBox->insertItem("ISO8859-9"); 
    mEncodingComboBox->insertItem("ISO8859-10"); 
    mEncodingComboBox->insertItem("ISO8859-13"); 
    mEncodingComboBox->insertItem("ISO8859-14"); 
    mEncodingComboBox->insertItem("ISO8859-15"); 
    mEncodingComboBox->insertItem("IBM 850"); 
    mEncodingComboBox->insertItem("IBM 866"); 
    mEncodingComboBox->insertItem("CP874"); 
    mEncodingComboBox->insertItem("CP1250"); 
    mEncodingComboBox->insertItem("CP1251"); 
    mEncodingComboBox->insertItem("CP1252"); 
    mEncodingComboBox->insertItem("CP1253"); 
    mEncodingComboBox->insertItem("CP1254"); 
    mEncodingComboBox->insertItem("CP1255"); 
    mEncodingComboBox->insertItem("CP1256"); 
    mEncodingComboBox->insertItem("CP1257"); 
    mEncodingComboBox->insertItem("CP1258"); 
    mEncodingComboBox->insertItem("Apple Roman"); 
    mEncodingComboBox->insertItem("TIS-620"); 
    if(currentencoding.isNull()||currentencoding.isEmpty()||currentencoding=="\0")
      {
	mEncodingComboBox->setCurrentText(QString(QTextCodec::codecForLocale()->name()));
      }
    else
      {
	mEncodingComboBox->setCurrentText(currentencoding);
      }
}

QgsEncodingFileDialog::~QgsEncodingFileDialog()
{
    
}

QString QgsEncodingFileDialog::encoding() const
{
    return mEncodingComboBox->currentText();
}
