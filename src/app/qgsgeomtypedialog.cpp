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

QgsGeomTypeDialog::QgsGeomTypeDialog(QWidget *parent, Qt::WFlags fl)
: QDialog(parent, fl)
{
  setupUi(this);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  mPointRadioButton->setChecked(true);
  mFileFormatComboBox->insertItem("ESRI Shapefile");
  /*mFileFormatComboBox->insertItem("Comma Separated Value");
  mFileFormatComboBox->insertItem("GML");
  mFileFormatComboBox->insertItem("Mapinfo File");*/
  mOkButton = buttonBox->button(QDialogButtonBox::Ok);
  mOkButton->setEnabled(false);
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
  QgsAddAttrDialog d(types, this);
  if(d.exec()==QDialog::Accepted)
  {
    mAttributeView->addTopLevelItem(new QTreeWidgetItem(QStringList() << d.name() << d.type()));
  }
  if(mAttributeView->topLevelItemCount()>0)
  {
    mOkButton->setEnabled(true);
  }
}

void QgsGeomTypeDialog::on_mRemoveAttributeButton_clicked()
{
  delete(mAttributeView->currentItem());
  if(mAttributeView->topLevelItemCount()==0)
  {
    mOkButton->setEnabled(false);	
  }
}

void QgsGeomTypeDialog::on_buttonBox_helpRequested()
{
  QgsContextHelp::run(context_id);
}

void QgsGeomTypeDialog::attributes(std::list<std::pair<QString, QString> >& at) const
{
  QTreeWidgetItemIterator it(mAttributeView);
  while ( *it ) 
  {
    QTreeWidgetItem *item = *it;
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
