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
#include "qgsrelationwidgetregistry.h"


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
      mWidgetSpecificConfigGroupBox->setTitle( QgsAttributeWidgetRelationEditWidget::title() );

    }
    break;

    case QgsAttributesFormProperties::DnDTreeItemData::Field:
    case QgsAttributesFormProperties::DnDTreeItemData::Action:
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

    case QgsAttributesFormProperties::DnDTreeItemData::Action:
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

  QMapIterator<QString, QgsAbstractRelationEditorWidgetFactory *> it( QgsGui::relationWidgetRegistry()->factories() );

  while ( it.hasNext() )
  {
    it.next();
    mWidgetTypeComboBox->addItem( it.value()->name(), it.key() );
  }

  connect( mRelationCardinalityCombo, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsAttributeWidgetRelationEditWidget::relationCardinalityComboCurrentIndexChanged );
}

void QgsAttributeWidgetRelationEditWidget::setRelationEditorConfiguration( const QgsAttributesFormProperties::RelationEditorConfiguration &config, const QString &relationId )
{
  //load the combo mRelationCardinalityCombo
  mRelationCardinalityCombo->clear();
  setCardinalityCombo( tr( "Many to one relation" ) );

  const QgsRelation relation = QgsProject::instance()->relationManager()->relation( relationId );
  const QList<QgsRelation> relations = QgsProject::instance()->relationManager()->referencingRelations( relation.referencingLayer() );
  if ( !relation.fieldPairs().isEmpty() )
  {
    const QgsRelation::FieldPair relationFirstFieldPair = relation.fieldPairs().at( 0 );
    for ( const QgsRelation &nmrel : relations )
    {
      if ( !nmrel.fieldPairs().isEmpty() &&
           nmrel.fieldPairs().at( 0 ).referencingField() != relationFirstFieldPair.referencingField() )
      {
        setCardinalityCombo( QStringLiteral( "%1 (%2)" ).arg( nmrel.referencedLayer()->name(), nmrel.fieldPairs().at( 0 ).referencedField() ), nmrel.id() );
      }
    }
  }

  const int widgetTypeIdx = mWidgetTypeComboBox->findData( config.mRelationWidgetType );
  mWidgetTypeComboBox->setCurrentIndex( widgetTypeIdx >= 0
                                        ? widgetTypeIdx
                                        : mWidgetTypeComboBox->findData( QgsGui::relationWidgetRegistry()->defaultWidgetType() ) );

  const QString widgetType = mWidgetTypeComboBox->currentData().toString();
  mConfigWidget = QgsGui::relationWidgetRegistry()->createConfigWidget( widgetType, relation, this );
  mConfigWidget->setConfig( config.mRelationWidgetConfig );
  mWidgetTypePlaceholderLayout->addWidget( mConfigWidget );

  disconnect( mWidgetTypeComboBoxConnection );

  mWidgetTypeComboBoxConnection = connect( mWidgetTypeComboBox, &QComboBox::currentTextChanged, this, [ = ]()
  {
    const QString widgetId = mWidgetTypeComboBox->currentData().toString();

    mWidgetTypePlaceholderLayout->removeWidget( mConfigWidget );
    mConfigWidget->deleteLater();
    mConfigWidget = QgsGui::relationWidgetRegistry()->createConfigWidget( widgetId, relation, this );
    mConfigWidget->setConfig( config.mRelationWidgetConfig );
    mWidgetTypePlaceholderLayout->addWidget( mConfigWidget );
    update();
  } );

  mRelationCardinalityCombo->setToolTip( tr( "For a many to many (N:M) relation, the direct link has to be selected. The in-between table will be hidden." ) );
  setNmRelationId( config.nmRelationId );

  mRelationLabelEdit->setText( config.label );

  mRelationForceSuppressFormPopupCheckBox->setChecked( config.forceSuppressFormPopup );
}

QgsAttributesFormProperties::RelationEditorConfiguration QgsAttributeWidgetRelationEditWidget::relationEditorConfiguration() const
{
  QgsAttributesFormProperties::RelationEditorConfiguration relEdCfg;
  relEdCfg.mRelationWidgetType = mWidgetTypeComboBox->currentData().toString();
  relEdCfg.mRelationWidgetConfig = mConfigWidget->config();
  relEdCfg.nmRelationId = mRelationCardinalityCombo->currentData();
  relEdCfg.forceSuppressFormPopup = mRelationForceSuppressFormPopupCheckBox->isChecked();
  relEdCfg.label = mRelationLabelEdit->text();
  return relEdCfg;
}

void QgsAttributeWidgetRelationEditWidget::relationCardinalityComboCurrentIndexChanged( int index )
{
  if ( index < 0 )
    return;

  if ( !mConfigWidget )
    return;

  const QgsRelation nmRelation = QgsProject::instance()->relationManager()->relation( mRelationCardinalityCombo->currentData().toString() );
  mConfigWidget->setNmRelation( nmRelation );
}

void QgsAttributeWidgetRelationEditWidget::setCardinalityCombo( const QString &cardinalityComboItem, const QVariant &auserData )
{
  mRelationCardinalityCombo->addItem( cardinalityComboItem, auserData );
}

void QgsAttributeWidgetRelationEditWidget::setNmRelationId( const QVariant &auserData )
{
  const int idx = mRelationCardinalityCombo->findData( auserData );

  if ( idx != -1 )
    mRelationCardinalityCombo->setCurrentIndex( idx );
}
