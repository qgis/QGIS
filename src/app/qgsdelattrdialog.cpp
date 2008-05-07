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
#include <QHeaderView>

QgsDelAttrDialog::QgsDelAttrDialog(QHeaderView* header): QDialog()
{
  setupUi(this);
  QObject::connect(mOkButton, SIGNAL(clicked(bool)), this, SLOT(accept()));
  QObject::connect(mCancelButton, SIGNAL(clicked(bool)), this, SLOT(reject()));

  //insert attribute names into the QListView
  if(header)
    {
      listBox2->clear();
      QAbstractItemModel *model = header->model();
      for(int i=1;i<header->count();++i)
      {
        listBox2->addItem(model->headerData(i, Qt::Horizontal).toString());
      }
    }
}

const std::list<QString>* QgsDelAttrDialog::selectedAttributes()
{
    mSelectedItems.clear();
    QListIterator<QListWidgetItem *> selection(listBox2->selectedItems());
    while (selection.hasNext())
    {
      mSelectedItems.push_back(selection.next()->text());
    }
    return &mSelectedItems;
}
