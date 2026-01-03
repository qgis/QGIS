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
#include "qgsgui.h"
#include "qgsrelationwidgetregistry.h"

#include "moc_qgsattributewidgetedit.cpp"

QgsAttributeWidgetEdit::QgsAttributeWidgetEdit( const QgsAttributesFormData::AttributeFormItemData &itemData, QWidget *parent )
  : QgsCollapsibleGroupBox( parent )
{
  setupUi( this );
  mHozStretchSpin->setClearValue( 0, tr( "Default" ) );
  mVertStretchSpin->setClearValue( 0, tr( "Default" ) );

  // common configs
  mShowLabelCheckBox->setChecked( itemData.showLabel() );

  mFormLabelFormatWidget->setLabelStyle( itemData.labelStyle() );
  mHozStretchSpin->setValue( itemData.horizontalStretch() );
  mVertStretchSpin->setValue( itemData.verticalStretch() );

  mWidgetSpecificConfigGroupBox->hide();
}

void QgsAttributeWidgetEdit::setRelationSpecificWidget( const QgsAttributesFormData::RelationEditorConfiguration &configuration, const QString &relationId )
{
  QGridLayout *layout = new QGridLayout;
  QgsAttributeWidgetRelationEditWidget *editWidget = new QgsAttributeWidgetRelationEditWidget( this );
  editWidget->setRelationEditorConfiguration( configuration, relationId );
  mSpecificEditWidget = editWidget;
  layout->addWidget( mSpecificEditWidget );
  mWidgetSpecificConfigGroupBox->setLayout( layout );
  mWidgetSpecificConfigGroupBox->setTitle( QgsAttributeWidgetRelationEditWidget::title() );
  mWidgetSpecificConfigGroupBox->show();
}

void QgsAttributeWidgetEdit::updateItemData( QgsAttributesFormData::AttributeFormItemData &itemData ) const
{
  // common configs
  itemData.setShowLabel( mShowLabelCheckBox->isChecked() );
  itemData.setLabelStyle( mFormLabelFormatWidget->labelStyle() );
  itemData.setHorizontalStretch( mHozStretchSpin->value() );
  itemData.setVerticalStretch( mVertStretchSpin->value() );
}

QgsAttributesFormData::RelationEditorConfiguration QgsAttributeWidgetEdit::updatedRelationConfiguration() const
{
  QgsAttributeWidgetRelationEditWidget *editWidget = qobject_cast<QgsAttributeWidgetRelationEditWidget *>( mSpecificEditWidget );
  if ( editWidget )
  {
    return editWidget->relationEditorConfiguration();
  }
  return QVariant().value< QgsAttributesFormData::RelationEditorConfiguration >();
}

void QgsAttributeWidgetEdit::setLabelStyle( const QgsAttributeEditorElement::LabelStyle &labelStyle )
{
  mFormLabelFormatWidget->setLabelStyle( labelStyle );
}

void QgsAttributeWidgetEdit::setShowLabel( bool showLabel )
{
  mShowLabelCheckBox->setChecked( showLabel );
}

void QgsAttributeWidgetEdit::setHorizontalStretch( const int horizontalStretch )
{
  mHozStretchSpin->setValue( horizontalStretch );
}

void QgsAttributeWidgetEdit::setVerticalStretch( const int verticalStretch )
{
  mVertStretchSpin->setValue( verticalStretch );
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

void QgsAttributeWidgetRelationEditWidget::setRelationEditorConfiguration( const QgsAttributesFormData::RelationEditorConfiguration &config, const QString &relationId )
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
      if ( !nmrel.fieldPairs().isEmpty() && nmrel.fieldPairs().at( 0 ).referencingField() != relationFirstFieldPair.referencingField() )
      {
        setCardinalityCombo( u"%1 (%2)"_s.arg( nmrel.referencedLayer()->name(), nmrel.fieldPairs().at( 0 ).referencedField() ), nmrel.id() );
      }
    }
  }

  const int widgetTypeIdx = mWidgetTypeComboBox->findData( config.mRelationWidgetType );
  mWidgetTypeComboBox->setCurrentIndex( widgetTypeIdx >= 0 ? widgetTypeIdx : mWidgetTypeComboBox->findData( QgsGui::relationWidgetRegistry()->defaultWidgetType() ) );

  const QString widgetType = mWidgetTypeComboBox->currentData().toString();
  mConfigWidget = QgsGui::relationWidgetRegistry()->createConfigWidget( widgetType, relation, this );
  mConfigWidget->setConfig( config.mRelationWidgetConfig );
  mWidgetTypePlaceholderLayout->addWidget( mConfigWidget );

  disconnect( mWidgetTypeComboBoxConnection );

  mWidgetTypeComboBoxConnection = connect( mWidgetTypeComboBox, &QComboBox::currentTextChanged, this, [this, relation, config]() {
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

QgsAttributesFormData::RelationEditorConfiguration QgsAttributeWidgetRelationEditWidget::relationEditorConfiguration() const
{
  QgsAttributesFormData::RelationEditorConfiguration relEdCfg;
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
