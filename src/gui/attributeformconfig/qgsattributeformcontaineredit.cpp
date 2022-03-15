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
    // only top level items can be tabs
    // i.e. it's always a group box if it's a nested container
    mShowAsGroupBox->hide();
    mShowAsGroupBox->setEnabled( false );
  }

  mTitleLineEdit->setText( itemData.name() );
  mShowLabelCheckBox->setChecked( itemData.showLabel() );
  mShowLabelCheckBox->setEnabled( itemData.showAsGroupBox() ); // show label makes sense for group box, not for tabs
  mShowAsGroupBox->setChecked( itemData.showAsGroupBox() );

  mControlVisibilityGroupBox->setChecked( itemData.visibilityExpression().enabled() );
  mVisibilityExpressionWidget->setLayer( layer );
  mVisibilityExpressionWidget->setExpression( itemData.visibilityExpression()->expression() );
  mColumnCountSpinBox->setValue( itemData.columnCount() );
  mBackgroundColorButton->setColor( itemData.backgroundColor() );
  mCollapsedCheckBox->setChecked( itemData.collapsed() );
  mCollapsedCheckBox->setEnabled( itemData.showAsGroupBox() );
  mControlCollapsedGroupBox->setChecked( itemData.collapsedExpression().enabled() );
  mControlCollapsedGroupBox->setEnabled( itemData.showAsGroupBox() );
  mCollapsedExpressionWidget->setExpression( itemData.collapsedExpression()->expression() );

  // show label makes sense for group box, not for tabs
  connect( mShowAsGroupBox, &QCheckBox::stateChanged, mShowLabelCheckBox, &QCheckBox::setEnabled );
  connect( mShowAsGroupBox, &QCheckBox::stateChanged, mCollapsedCheckBox, &QCheckBox::setEnabled );
  connect( mShowAsGroupBox, &QCheckBox::stateChanged, mControlCollapsedGroupBox, &QCheckBox::setEnabled );
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
  itemData.setShowAsGroupBox( mShowAsGroupBox->isEnabled() ? mShowAsGroupBox->isChecked() : false );
  itemData.setName( mTitleLineEdit->text() );
  itemData.setShowLabel( mShowLabelCheckBox->isChecked() );
  itemData.setBackgroundColor( mBackgroundColorButton->color() );

  QgsOptionalExpression visibilityExpression;
  visibilityExpression.setData( QgsExpression( mVisibilityExpressionWidget->expression() ) );
  visibilityExpression.setEnabled( mControlVisibilityGroupBox->isChecked() );
  itemData.setVisibilityExpression( visibilityExpression );

  QgsOptionalExpression collapsedExpression;
  collapsedExpression.setData( QgsExpression( mCollapsedExpressionWidget->expression() ) );
  collapsedExpression.setEnabled( mControlCollapsedGroupBox->isChecked() );
  itemData.setCollapsedExpression( collapsedExpression );
  itemData.setCollapsed( mCollapsedCheckBox->isEnabled() ? mCollapsedCheckBox->isChecked() : false );

  mTreeItem->setData( 0, QgsAttributesFormProperties::DnDTreeRole, itemData );
  mTreeItem->setText( 0, itemData.name() );
}
