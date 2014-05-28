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

#include <QSettings>

QgsDelAttrDialog::QgsDelAttrDialog( const QgsVectorLayer* vl ): QDialog()
{
  setupUi( this );
  if ( vl )
  {
    listBox2->clear();
    const QgsFields& layerAttributes = vl->pendingFields();
    for ( int idx = 0; idx < layerAttributes.count(); ++idx )
    {
      QListWidgetItem* item = new QListWidgetItem( layerAttributes[idx].name(), listBox2 );
      item->setData( Qt::UserRole, idx );
    }
  }

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/QgsDelAttrDialog/geometry" ).toByteArray() );
}

QgsDelAttrDialog::~QgsDelAttrDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/QgsDelAttrDialog/geometry", saveGeometry() );
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
