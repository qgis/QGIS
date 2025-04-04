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

QgsAttributeFormContainerEdit::QgsAttributeFormContainerEdit( const QgsAttributesFormTreeData::DnDTreeNodeData &nodeData, QgsVectorLayer *layer, QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  mHozStretchSpin->setClearValue( 0, tr( "Default" ) );
  mVertStretchSpin->setClearValue( 0, tr( "Default" ) );

  mShowLabelCheckBox->setChecked( nodeData.showLabel() );

  mControlVisibilityGroupBox->setChecked( nodeData.visibilityExpression().enabled() );
  mVisibilityExpressionWidget->setLayer( layer );
  mVisibilityExpressionWidget->setExpression( nodeData.visibilityExpression()->expression() );
  mColumnCountSpinBox->setValue( nodeData.columnCount() );
  mBackgroundColorButton->setShowNull( true );
  mBackgroundColorButton->setColor( nodeData.backgroundColor() );
  mCollapsedCheckBox->setChecked( nodeData.collapsed() );
  mControlCollapsedGroupBox->setChecked( nodeData.collapsedExpression().enabled() );
  mCollapsedExpressionWidget->setExpression( nodeData.collapsedExpression()->expression() );

  mHozStretchSpin->setValue( nodeData.horizontalStretch() );
  mVertStretchSpin->setValue( nodeData.verticalStretch() );

  mFormLabelFormatWidget->setLabelStyle( nodeData.labelStyle() );
}

void QgsAttributeFormContainerEdit::setTitle( const QString &containerName )
{
  mTitleLineEdit->setText( containerName );
}

void QgsAttributeFormContainerEdit::setUpContainerTypeComboBox( bool isTopLevelContainer, const Qgis::AttributeEditorContainerType containerType )
{
  if ( isTopLevelContainer )
  {
    // only top level nodes can be tabs
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

void QgsAttributeFormContainerEdit::updateNodeData( QgsAttributesFormTreeData::DnDTreeNodeData &nodeData, QString &containerName )
{
  nodeData.setColumnCount( mColumnCountSpinBox->value() );
  nodeData.setContainerType( mTypeCombo->currentData().value<Qgis::AttributeEditorContainerType>() );
  containerName = mTitleLineEdit->text();
  nodeData.setShowLabel( mShowLabelCheckBox->isChecked() );
  nodeData.setBackgroundColor( mBackgroundColorButton->color() );
  nodeData.setLabelStyle( mFormLabelFormatWidget->labelStyle() );
  nodeData.setHorizontalStretch( mHozStretchSpin->value() );
  nodeData.setVerticalStretch( mVertStretchSpin->value() );

  QgsOptionalExpression visibilityExpression;
  visibilityExpression.setData( QgsExpression( mVisibilityExpressionWidget->expression() ) );
  visibilityExpression.setEnabled( mControlVisibilityGroupBox->isChecked() );
  nodeData.setVisibilityExpression( visibilityExpression );

  QgsOptionalExpression collapsedExpression;
  collapsedExpression.setData( QgsExpression( mCollapsedExpressionWidget->expression() ) );
  collapsedExpression.setEnabled( mControlCollapsedGroupBox->isChecked() );
  nodeData.setCollapsedExpression( collapsedExpression );
  nodeData.setCollapsed( mCollapsedCheckBox->isEnabled() ? mCollapsedCheckBox->isChecked() : false );
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
