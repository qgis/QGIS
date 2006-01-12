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
/* $Id$ */
#include "qgsattributedialog.h"
#include <QTableWidgetItem>

QgsAttributeDialog::QgsAttributeDialog(std::vector<QgsFeatureAttribute>* attributes)
: QDialog()
{
    setupUi(this);
    mTable->setRowCount(attributes->size());

    int index=0;
    for(std::vector<QgsFeatureAttribute>::iterator it=attributes->begin();it!=attributes->end();++it)
    {
      QTableWidgetItem * myFieldItem = new QTableWidgetItem((*it).fieldName());
      mTable->setItem(index, 0, myFieldItem);
      QTableWidgetItem * myValueItem = new QTableWidgetItem((*it).fieldValue());
      mTable->setItem(index, 1, myValueItem);
      ++index;
    }
}

QgsAttributeDialog::~QgsAttributeDialog()
{

}

QString QgsAttributeDialog::value(int row)
{
    return mTable->item(row,1)->text();
}
