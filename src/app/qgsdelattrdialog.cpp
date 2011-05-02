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
#include "qgsvectorlayer.h"

QgsDelAttrDialog::QgsDelAttrDialog( const QgsVectorLayer* vl ): QDialog()
{
  setupUi( this );
  if ( vl )
  {
    listBox2->clear();
    const QgsFieldMap layerAttributes = vl->pendingFields();
    QgsFieldMap::const_iterator attIt = layerAttributes.constBegin();
    for ( ; attIt != layerAttributes.constEnd(); ++attIt )
    {
      QListWidgetItem* item = new QListWidgetItem( attIt.value().name(), listBox2 );
      item->setData( Qt::UserRole, attIt.key() );
    }
  }
}

QList<int> QgsDelAttrDialog::selectedAttributes()
{
  QList<int> selectionList;
  QList<QListWidgetItem *> selection = listBox2->selectedItems();
  QList<QListWidgetItem *>::const_iterator itemIter = selection.constBegin();
  for ( ; itemIter != selection.constEnd(); ++itemIter )
  {
    selectionList.push_back(( *itemIter )->data( Qt::UserRole ).toInt() );
  }
  return selectionList;
}
