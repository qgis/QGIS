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
#include "moc_qgsattributeformcontaineredit.cpp"
#include "ui_qgsattributeformcontaineredit.h"
#include "qgsattributesformproperties.h"
#include "qgsattributesformmodel.h"
#include "qgsvectorlayer.h"

QgsAttributeFormContainerEdit::QgsAttributeFormContainerEdit( const QgsAttributesFormData::AttributeFormItemData &itemData, QgsVectorLayer *layer, QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mHozStretchSpin->setClearValue( 0, tr( "Default" ) );
  mVertStretchSpin->setClearValue( 0, tr( "Default" ) );

  mShowLabelCheckBox->setChecked( itemData.showLabel() );

  mControlVisibilityGroupBox->setChecked( itemData.visibilityExpression().enabled() );
  mVisibilityExpressionWidget->setLayer( layer );
  mVisibilityExpressionWidget->setExpression( itemData.visibilityExpression()->expression() );
  mColumnCountSpinBox->setValue( itemData.columnCount() );
  mBackgroundColorButton->setShowNull( true );
  mBackgroundColorButton->setColor( itemData.backgroundColor() );
  mCollapsedCheckBox->setChecked( itemData.collapsed() );
  mControlCollapsedGroupBox->setChecked( itemData.collapsedExpression().enabled() );
  mCollapsedExpressionWidget->setExpression( itemData.collapsedExpression()->expression() );

  mHozStretchSpin->setValue( itemData.horizontalStretch() );
  mVertStretchSpin->setValue( itemData.verticalStretch() );

  mFormLabelFormatWidget->setLabelStyle( itemData.labelStyle() );
}

void QgsAttributeFormContainerEdit::setTitle( const QString &title )
{
  mTitleLineEdit->setText( title );
}

void QgsAttributeFormContainerEdit::setUpContainerTypeComboBox( bool isTopLevelContainer, const Qgis::AttributeEditorContainerType containerType )
{
  if ( isTopLevelContainer )
  {
    // only top level items can be tabs
    mTypeCombo->addItem( tr( "Tab" ), QVariant::fromValue( Qgis::AttributeEditorContainerType::Tab ) );
  }
  mTypeCombo->addItem( tr( "Group Box" ), QVariant::fromValue( Qgis::AttributeEditorContainerType::GroupBox ) );
  mTypeCombo->addItem( tr( "Row" ), QVariant::fromValue( Qgis::AttributeEditorContainerType::Row ) );

  mTypeCombo->setCurrentIndex( mTypeCombo->findData( QVariant::fromValue( containerType ) ) );
  if ( mTypeCombo->currentIndex() < 0 )
    mTypeCombo->setCurrentIndex( 0 );

  connect( mTypeCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsAttributeFormContainerEdit::containerTypeChanged );
  containerTypeChanged();
}

void QgsAttributeFormContainerEdit::registerExpressionContextGenerator( QgsExpressionContextGenerator *generator )
{
  mVisibilityExpressionWidget->registerExpressionContextGenerator( generator );
  mCollapsedExpressionWidget->registerExpressionContextGenerator( generator );
}

void QgsAttributeFormContainerEdit::updateItemData( QgsAttributesFormData::AttributeFormItemData &itemData, QString &title )
{
  itemData.setColumnCount( mColumnCountSpinBox->value() );
  itemData.setContainerType( mTypeCombo->currentData().value<Qgis::AttributeEditorContainerType>() );
  title = mTitleLineEdit->text();
  itemData.setShowLabel( mShowLabelCheckBox->isChecked() );
  itemData.setBackgroundColor( mBackgroundColorButton->color() );
  itemData.setLabelStyle( mFormLabelFormatWidget->labelStyle() );
  itemData.setHorizontalStretch( mHozStretchSpin->value() );
  itemData.setVerticalStretch( mVertStretchSpin->value() );

  QgsOptionalExpression visibilityExpression;
  visibilityExpression.setData( QgsExpression( mVisibilityExpressionWidget->expression() ) );
  visibilityExpression.setEnabled( mControlVisibilityGroupBox->isChecked() );
  itemData.setVisibilityExpression( visibilityExpression );

  QgsOptionalExpression collapsedExpression;
  collapsedExpression.setData( QgsExpression( mCollapsedExpressionWidget->expression() ) );
  collapsedExpression.setEnabled( mControlCollapsedGroupBox->isChecked() );
  itemData.setCollapsedExpression( collapsedExpression );
  itemData.setCollapsed( mCollapsedCheckBox->isEnabled() ? mCollapsedCheckBox->isChecked() : false );
}

void QgsAttributeFormContainerEdit::containerTypeChanged()
{
  // show label makes sense for group box, not for tabs
  const Qgis::AttributeEditorContainerType type = mTypeCombo->currentData().value<Qgis::AttributeEditorContainerType>();
  switch ( type )
  {
    case Qgis::AttributeEditorContainerType::GroupBox:
      mShowLabelCheckBox->setEnabled( true );
      mCollapsedCheckBox->setEnabled( true );
      mControlCollapsedGroupBox->setEnabled( true );
      mColumnsLabel->show();
      mColumnCountSpinBox->show();
      break;
    case Qgis::AttributeEditorContainerType::Tab:
      mShowLabelCheckBox->setEnabled( false );
      mShowLabelCheckBox->setChecked( true );
      mCollapsedCheckBox->setEnabled( false );
      mCollapsedCheckBox->setChecked( false );
      mControlCollapsedGroupBox->setEnabled( false );
      mColumnsLabel->show();
      mColumnCountSpinBox->show();
      break;
    case Qgis::AttributeEditorContainerType::Row:
      mShowLabelCheckBox->setEnabled( false );
      mShowLabelCheckBox->setChecked( false );
      mCollapsedCheckBox->setEnabled( false );
      mCollapsedCheckBox->setChecked( false );
      mControlCollapsedGroupBox->setEnabled( false );
      mColumnsLabel->hide();
      mColumnCountSpinBox->hide();
      break;
  }
}
