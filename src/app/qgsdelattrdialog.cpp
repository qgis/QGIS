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

#include "qgsapplication.h"
#include "qgsdelattrdialog.h"
#include "moc_qgsdelattrdialog.cpp"
#include "qgsfields.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsgui.h"

QgsDelAttrDialog::QgsDelAttrDialog( const QgsVectorLayer *vl )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  if ( vl )
  {
    const bool canDeleteAttributes = vl->dataProvider()->capabilities() & Qgis::VectorProviderCapability::DeleteAttributes;
    listBox2->clear();
    const QgsFields layerAttributes = vl->fields();
    for ( int idx = 0; idx < layerAttributes.count(); ++idx )
    {
      QListWidgetItem *item = new QListWidgetItem( layerAttributes.at( idx ).name(), listBox2 );
      item->setIcon( layerAttributes.iconForField( idx ) );
      switch ( layerAttributes.fieldOrigin( idx ) )
      {
        case Qgis::FieldOrigin::Expression:
          break;

        case Qgis::FieldOrigin::Join:
          item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
          break;

        default:
          if ( !vl->isEditable() || !canDeleteAttributes )
            item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
          break;
      }

      item->setData( Qt::UserRole, idx );
    }

    mEditModeInfo->setVisible( !vl->isEditable() );
    mCanDeleteAttributesInfo->setVisible( !canDeleteAttributes );
  }
}

QList<int> QgsDelAttrDialog::selectedAttributes()
{
  QList<int> selectionList;
  const QList<QListWidgetItem *> selection = listBox2->selectedItems();
  QList<QListWidgetItem *>::const_iterator itemIter = selection.constBegin();
  for ( ; itemIter != selection.constEnd(); ++itemIter )
  {
    selectionList.push_back( ( *itemIter )->data( Qt::UserRole ).toInt() );
  }
  return selectionList;
}
