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
#include "qgsfield.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

#include <QSettings>

QgsDelAttrDialog::QgsDelAttrDialog( const QgsVectorLayer* vl )
    : QDialog()
{
  setupUi( this );
  if ( vl )
  {
    bool canDeleteAttributes = vl->dataProvider()->capabilities() & QgsVectorDataProvider::DeleteAttributes;
    listBox2->clear();
    const QgsFields& layerAttributes = vl->pendingFields();
    for ( int idx = 0; idx < layerAttributes.count(); ++idx )
    {
      QListWidgetItem* item = new QListWidgetItem( layerAttributes[idx].name(), listBox2 );
      switch ( vl->pendingFields().fieldOrigin( idx ) )
      {
        case QgsFields::OriginExpression:
          item->setIcon( QgsApplication::getThemeIcon( "/mIconExpression.svg" ) );
          break;

        case QgsFields::OriginJoin:
          item->setIcon( QgsApplication::getThemeIcon( "/propertyicons/join.png" ) );
          item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
          break;

        default:
          item->setIcon( QgsApplication::getThemeIcon( "/propertyicons/attributes.png" ) );
          if ( !vl->isEditable() || !canDeleteAttributes )
            item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
          break;
      }

      item->setData( Qt::UserRole, idx );
    }

    mEditModeInfo->setVisible( !vl->isEditable() );
    mCanDeleteAttributesInfo->setVisible( !canDeleteAttributes );
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
