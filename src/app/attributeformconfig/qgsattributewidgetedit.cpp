/***************************************************************************
    qgsattributewidgetedit.cpp
    ---------------------
    begin                : February 2020
    copyright            : (C) 2020 Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributewidgetedit.h"
#include "qgsattributesformproperties.h"


QgsAttributeWidgetEdit::QgsAttributeWidgetEdit( QTreeWidgetItem *item, QWidget *parent )
  : QgsCollapsibleGroupBox( parent )
  , mTreeItem( item )

{
  setupUi( this );

  const QgsAttributesFormProperties::DnDTreeItemData itemData = mTreeItem->data( 0, QgsAttributesFormProperties::DnDTreeRole ).value<QgsAttributesFormProperties::DnDTreeItemData>();

  // common configs
  mShowLabelCheckBox->setChecked( itemData.showLabel() );

  // hide specific configs
  mRelationShowLinkCheckBox->hide();
  mRelationShowUnlinkCheckBox->hide();

  switch ( itemData.type() )
  {
    case QgsAttributesFormProperties::DnDTreeItemData::Relation:
    {
      mRelationShowLinkCheckBox->show();
      mRelationShowUnlinkCheckBox->show();
      mRelationShowLinkCheckBox->setChecked( itemData.relationEditorConfiguration().showLinkButton );
      mRelationShowUnlinkCheckBox->setChecked( itemData.relationEditorConfiguration().showUnlinkButton );
    }
    break;

    case QgsAttributesFormProperties::DnDTreeItemData::Field:
    case QgsAttributesFormProperties::DnDTreeItemData::Container:
    case QgsAttributesFormProperties::DnDTreeItemData::QmlWidget:
    case QgsAttributesFormProperties::DnDTreeItemData::HtmlWidget:
    case QgsAttributesFormProperties::DnDTreeItemData::WidgetType:
      break;
  }


}

void QgsAttributeWidgetEdit::updateItemData()
{
  QgsAttributesFormProperties::DnDTreeItemData itemData = mTreeItem->data( 0, QgsAttributesFormProperties::DnDTreeRole ).value<QgsAttributesFormProperties::DnDTreeItemData>();

  // common configs
  itemData.setShowLabel( mShowLabelCheckBox->isChecked() );

  // specific configs
  switch ( itemData.type() )
  {
    case QgsAttributesFormProperties::DnDTreeItemData::Relation:
    {
      QgsAttributesFormProperties::RelationEditorConfiguration relEdCfg;
      relEdCfg.showLinkButton = mRelationShowLinkCheckBox->isChecked();
      relEdCfg.showUnlinkButton = mRelationShowUnlinkCheckBox->isChecked();
      itemData.setRelationEditorConfiguration( relEdCfg );
    }
    break;

    case QgsAttributesFormProperties::DnDTreeItemData::Field:
    case QgsAttributesFormProperties::DnDTreeItemData::Container:
    case QgsAttributesFormProperties::DnDTreeItemData::QmlWidget:
    case QgsAttributesFormProperties::DnDTreeItemData::HtmlWidget:
    case QgsAttributesFormProperties::DnDTreeItemData::WidgetType:
      break;
  }

  mTreeItem->setData( 0, QgsAttributesFormProperties::DnDTreeRole, itemData );
}
