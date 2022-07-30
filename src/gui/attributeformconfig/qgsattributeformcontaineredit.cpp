/***************************************************************************
    qgsattributeformcontaineredit.cpp
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

#include "qgsattributeformcontaineredit.h"
#include "ui_qgsattributeformcontaineredit.h"
#include "qgsattributesformproperties.h"


QgsAttributeFormContainerEdit::QgsAttributeFormContainerEdit( QTreeWidgetItem *item, QgsVectorLayer *layer, QWidget *parent )
  : QWidget( parent )
  , mTreeItem( item )
{
  setupUi( this );

  const QgsAttributesFormProperties::DnDTreeItemData itemData = mTreeItem->data( 0, QgsAttributesFormProperties::DnDTreeRole ).value<QgsAttributesFormProperties::DnDTreeItemData>();
  Q_ASSERT( itemData.type() == QgsAttributesFormProperties::DnDTreeItemData::Container );

  if ( item->parent() )
  {
    // a nested container is always a group box, then hide the checkbox
    mShowAsGroupBox->setCheckable( false );
  }
  else
  {
    mShowAsGroupBox->setChecked( itemData.showAsGroupBox() );
  }

  mTitleLineEdit->setText( itemData.name() );
  mShowLabelCheckBox->setChecked( itemData.showLabel() );

  mVisibilityExpressionWidget->setAllowEmptyFieldName( true );
  mVisibilityExpressionWidget->setLayer( layer );
  mVisibilityExpressionWidget->setExpression( itemData.visibilityExpression()->expression() );
  mColumnCountSpinBox->setValue( itemData.columnCount() );
  mBackgroundColorButton->setColor( itemData.backgroundColor() );
  mCollapsedCheckBox->setChecked( itemData.collapsed() );
  mCollapsedExpressionWidget->setAllowEmptyFieldName( true );
  mCollapsedExpressionWidget->setLayer( layer );
  mCollapsedExpressionWidget->setExpression( itemData.collapsedExpression()->expression() );

  mFormLabelFormatWidget->setLabelStyle( itemData.labelStyle() );
}

void QgsAttributeFormContainerEdit::registerExpressionContextGenerator( QgsExpressionContextGenerator *generator )
{
  mVisibilityExpressionWidget->registerExpressionContextGenerator( generator );
  mCollapsedExpressionWidget->registerExpressionContextGenerator( generator );
}

void QgsAttributeFormContainerEdit::updateItemData()
{
  QgsAttributesFormProperties::DnDTreeItemData itemData = mTreeItem->data( 0, QgsAttributesFormProperties::DnDTreeRole ).value<QgsAttributesFormProperties::DnDTreeItemData>();

  itemData.setColumnCount( mColumnCountSpinBox->value() );
  itemData.setShowAsGroupBox( mShowAsGroupBox->isCheckable() ? mShowAsGroupBox->isChecked() : true );
  itemData.setName( mTitleLineEdit->text() );
  itemData.setShowLabel( mShowLabelCheckBox->isChecked() );
  itemData.setBackgroundColor( mBackgroundColorButton->color() );
  itemData.setLabelStyle( mFormLabelFormatWidget->labelStyle() );

  QgsOptionalExpression visibilityExpression;
  visibilityExpression.setData( QgsExpression( mVisibilityExpressionWidget->expression() ) );
  visibilityExpression.setEnabled( !mVisibilityExpressionWidget->currentText().isEmpty() );
  itemData.setVisibilityExpression( visibilityExpression );

  QgsOptionalExpression collapsedExpression;
  collapsedExpression.setData( QgsExpression( mCollapsedExpressionWidget->expression() ) );
  collapsedExpression.setEnabled( itemData.showAsGroupBox() );
  itemData.setCollapsedExpression( collapsedExpression );
  itemData.setCollapsed( mCollapsedCheckBox->isEnabled() ? mCollapsedCheckBox->isChecked() : false );

  mTreeItem->setData( 0, QgsAttributesFormProperties::DnDTreeRole, itemData );
  mTreeItem->setText( 0, itemData.name() );
}
