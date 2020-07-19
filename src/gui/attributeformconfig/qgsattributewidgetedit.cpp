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

  switch ( itemData.type() )
  {
    case QgsAttributesFormProperties::DnDTreeItemData::Relation:
    {
      showRelationButtons( true );
      mRelationShowLinkCheckBox->setChecked( itemData.relationEditorConfiguration().buttons.testFlag( QgsAttributeEditorRelation::Button::Link ) );
      mRelationShowUnlinkCheckBox->setChecked( itemData.relationEditorConfiguration().buttons.testFlag( QgsAttributeEditorRelation::Button::Unlink ) );
      mRelationShowAddChildCheckBox->setChecked( itemData.relationEditorConfiguration().buttons.testFlag( QgsAttributeEditorRelation::Button::AddChildFeature ) );
      mRelationShowDuplicateChildFeatureCheckBox->setChecked( itemData.relationEditorConfiguration().buttons.testFlag( QgsAttributeEditorRelation::Button::DuplicateChildFeature ) );
      mRelationShowZoomToFeatureCheckBox->setChecked( itemData.relationEditorConfiguration().buttons.testFlag( QgsAttributeEditorRelation::Button::ZoomToChildFeature ) );
      mRelationDeleteChildFeatureCheckBox->setChecked( itemData.relationEditorConfiguration().buttons.testFlag( QgsAttributeEditorRelation::Button::DeleteChildFeature ) );
      mRelationShowSaveChildEditsCheckBox->setChecked( itemData.relationEditorConfiguration().buttons.testFlag( QgsAttributeEditorRelation::Button::SaveChildEdits ) );
    }
    break;

    case QgsAttributesFormProperties::DnDTreeItemData::Field:
    case QgsAttributesFormProperties::DnDTreeItemData::Container:
    case QgsAttributesFormProperties::DnDTreeItemData::QmlWidget:
    case QgsAttributesFormProperties::DnDTreeItemData::HtmlWidget:
    case QgsAttributesFormProperties::DnDTreeItemData::WidgetType:
      showRelationButtons( false );
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
      QgsAttributeEditorRelation::Buttons buttons;
      buttons.setFlag( QgsAttributeEditorRelation::Button::Link, mRelationShowLinkCheckBox->isChecked() );
      buttons.setFlag( QgsAttributeEditorRelation::Button::Unlink, mRelationShowUnlinkCheckBox->isChecked() );
      buttons.setFlag( QgsAttributeEditorRelation::Button::AddChildFeature, mRelationShowAddChildCheckBox->isChecked() );
      buttons.setFlag( QgsAttributeEditorRelation::Button::DuplicateChildFeature, mRelationShowDuplicateChildFeatureCheckBox->isChecked() );
      buttons.setFlag( QgsAttributeEditorRelation::Button::ZoomToChildFeature, mRelationShowZoomToFeatureCheckBox->isChecked() );
      buttons.setFlag( QgsAttributeEditorRelation::Button::DeleteChildFeature, mRelationDeleteChildFeatureCheckBox->isChecked() );
      buttons.setFlag( QgsAttributeEditorRelation::Button::SaveChildEdits, mRelationShowSaveChildEditsCheckBox->isChecked() );
      relEdCfg.buttons = buttons;
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

void QgsAttributeWidgetEdit::showRelationButtons( bool show )
{
  const QList<QAbstractButton *> buttons = mRelationButtonGroup->buttons();
  for ( QAbstractButton *button : buttons )
    button->setVisible( show );
}
