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
      QGridLayout *layout = new QGridLayout;
      QgsAttributeWidgetRelationEditWidget *editWidget = new QgsAttributeWidgetRelationEditWidget( this );
      editWidget->setRelationEditorConfiguration( itemData.relationEditorConfiguration(), itemData.name() );
      mSpecificEditWidget = editWidget;
      layout->addWidget( mSpecificEditWidget );
      mWidgetSpecificConfigGroupBox->setLayout( layout );
      mWidgetSpecificConfigGroupBox->setTitle( editWidget->title() );

    }
    break;

    case QgsAttributesFormProperties::DnDTreeItemData::Field:
    case QgsAttributesFormProperties::DnDTreeItemData::Container:
    case QgsAttributesFormProperties::DnDTreeItemData::QmlWidget:
    case QgsAttributesFormProperties::DnDTreeItemData::HtmlWidget:
    case QgsAttributesFormProperties::DnDTreeItemData::WidgetType:
      mWidgetSpecificConfigGroupBox->hide();
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
      QgsAttributeWidgetRelationEditWidget *editWidget = qobject_cast<QgsAttributeWidgetRelationEditWidget *>( mSpecificEditWidget );
      if ( editWidget )
      {
        itemData.setRelationEditorConfiguration( editWidget->relationEditorConfiguration() );
      }
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

// Relation Widget Specific Edit

QgsAttributeWidgetRelationEditWidget::QgsAttributeWidgetRelationEditWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );
}

void QgsAttributeWidgetRelationEditWidget::setRelationEditorConfiguration( const QgsAttributesFormProperties::RelationEditorConfiguration &config, const QString &relationId )
{
  mRelationShowLinkCheckBox->setChecked( config.buttons.testFlag( QgsAttributeEditorRelation::Button::Link ) );
  mRelationShowUnlinkCheckBox->setChecked( config.buttons.testFlag( QgsAttributeEditorRelation::Button::Unlink ) );
  mRelationShowAddChildCheckBox->setChecked( config.buttons.testFlag( QgsAttributeEditorRelation::Button::AddChildFeature ) );
  mRelationShowDuplicateChildFeatureCheckBox->setChecked( config.buttons.testFlag( QgsAttributeEditorRelation::Button::DuplicateChildFeature ) );
  mRelationShowZoomToFeatureCheckBox->setChecked( config.buttons.testFlag( QgsAttributeEditorRelation::Button::ZoomToChildFeature ) );
  mRelationDeleteChildFeatureCheckBox->setChecked( config.buttons.testFlag( QgsAttributeEditorRelation::Button::DeleteChildFeature ) );
  mRelationShowSaveChildEditsCheckBox->setChecked( config.buttons.testFlag( QgsAttributeEditorRelation::Button::SaveChildEdits ) );

  //load the combo mRelationCardinalityCombo
  setCardinalityCombo( tr( "Many to one relation" ) );

  QgsRelation relation = QgsProject::instance()->relationManager()->relation( relationId );
  const QList<QgsRelation> relations = QgsProject::instance()->relationManager()->referencingRelations( relation.referencingLayer() );
  for ( const QgsRelation &nmrel : relations )
  {
    if ( nmrel.fieldPairs().at( 0 ).referencingField() != relation.fieldPairs().at( 0 ).referencingField() )
      setCardinalityCombo( QStringLiteral( "%1 (%2)" ).arg( nmrel.referencedLayer()->name(), nmrel.fieldPairs().at( 0 ).referencedField() ), nmrel.id() );
  }

  mRelationCardinalityCombo->setToolTip( tr( "For a many to many (N:M) relation, the direct link has to be selected. The in-between table will be hidden." ) );
  setNmRelationId( config.nmRelationId );

  mRelationLabelEdit->setText( config.label );

  mRelationForceSuppressFormPopupCheckBox->setChecked( config.forceSuppressFormPopup );
}

QgsAttributesFormProperties::RelationEditorConfiguration QgsAttributeWidgetRelationEditWidget::relationEditorConfiguration() const
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
  relEdCfg.nmRelationId = mRelationCardinalityCombo->currentData();
  relEdCfg.forceSuppressFormPopup = mRelationForceSuppressFormPopupCheckBox->isChecked();
  relEdCfg.label = mRelationLabelEdit->text();
  return relEdCfg;
}

void QgsAttributeWidgetRelationEditWidget::setCardinalityCombo( const QString &cardinalityComboItem, const QVariant &auserData )
{
  mRelationCardinalityCombo->addItem( cardinalityComboItem, auserData );
}

void QgsAttributeWidgetRelationEditWidget::setNmRelationId( const QVariant &auserData )
{
  int idx = mRelationCardinalityCombo->findData( auserData );

  if ( idx != -1 )
    mRelationCardinalityCombo->setCurrentIndex( idx );
}
