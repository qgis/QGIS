/***************************************************************************
    qgslabelsettingswidgetbase.h
    ----------------------
    begin                : December 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgslabelsettingswidgetbase.h"
#include "qgsexpressioncontextutils.h"
#include "qgsnewauxiliaryfielddialog.h"
#include "qgsnewauxiliarylayerdialog.h"
#include "qgspropertyoverridebutton.h"
#include "qgsauxiliarystorage.h"
#include "qgsgui.h"


QgsLabelSettingsWidgetBase::QgsLabelSettingsWidgetBase( QWidget *parent, QgsVectorLayer *vl )
  : QgsPanelWidget( parent )
  , mVectorLayer( vl )
{
}

void QgsLabelSettingsWidgetBase::setContext( const QgsSymbolWidgetContext &context )
{
  mContext = context;
}

QgsSymbolWidgetContext QgsLabelSettingsWidgetBase::context() const
{
  return mContext;
}

void QgsLabelSettingsWidgetBase::setGeometryType( QgsWkbTypes::GeometryType )
{

}

QgsExpressionContext QgsLabelSettingsWidgetBase::createExpressionContext() const
{
  if ( auto *lExpressionContext = mContext.expressionContext() )
    return *lExpressionContext;

  QgsExpressionContext expContext( mContext.globalProjectAtlasMapLayerScopes( mVectorLayer ) );
  QgsExpressionContextScope *symbolScope = QgsExpressionContextUtils::updateSymbolScope( nullptr, new QgsExpressionContextScope() );
  symbolScope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_SYMBOL_COLOR, QColor(), true ) );
  expContext << symbolScope;

  // additional scopes
  const auto constAdditionalExpressionContextScopes = mContext.additionalExpressionContextScopes();
  for ( const QgsExpressionContextScope &scope : constAdditionalExpressionContextScopes )
  {
    expContext.appendScope( new QgsExpressionContextScope( scope ) );
  }

  //TODO - show actual value
  expContext.setOriginalValueVariable( QVariant() );

  expContext.setHighlightedVariables( QStringList() << QgsExpressionContext::EXPR_ORIGINAL_VALUE << QgsExpressionContext::EXPR_SYMBOL_COLOR );

  return expContext;
}

void QgsLabelSettingsWidgetBase::createAuxiliaryField()
{
  // try to create an auxiliary layer if not yet created
  if ( !mVectorLayer->auxiliaryLayer() )
  {
    QgsNewAuxiliaryLayerDialog dlg( mVectorLayer, this );
    dlg.exec();
  }

  // return if still not exists
  if ( !mVectorLayer->auxiliaryLayer() )
    return;

  QgsPropertyOverrideButton *button = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  const QgsPalLayerSettings::Property key = static_cast<  QgsPalLayerSettings::Property >( button->propertyKey() );
  QgsPropertyDefinition def = QgsPalLayerSettings::propertyDefinitions()[key];

  // create property in auxiliary storage if necessary
  if ( !mVectorLayer->auxiliaryLayer()->exists( def ) )
  {
    QgsNewAuxiliaryFieldDialog dlg( def, mVectorLayer, true, this );
    if ( dlg.exec() == QDialog::Accepted )
      def = dlg.propertyDefinition();
  }

  // return if still not exist
  if ( !mVectorLayer->auxiliaryLayer()->exists( def ) )
    return;

  // update property with join field name from auxiliary storage
  QgsProperty property = button->toProperty();
  property.setField( QgsAuxiliaryLayer::nameFromProperty( def, true ) );
  property.setActive( true );
  button->updateFieldLists();
  button->setToProperty( property );

  mDataDefinedProperties.setProperty( key, button->toProperty() );

  emit auxiliaryFieldCreated();

  emit changed();
}

void QgsLabelSettingsWidgetBase::updateDataDefinedProperty()
{
  QgsPropertyOverrideButton *button = qobject_cast<QgsPropertyOverrideButton *>( sender() );
  const QgsPalLayerSettings::Property key = static_cast<  QgsPalLayerSettings::Property >( button->propertyKey() );
  mDataDefinedProperties.setProperty( key, button->toProperty() );
  emit changed();
}

QgsPropertyCollection QgsLabelSettingsWidgetBase::dataDefinedProperties() const
{
  return mDataDefinedProperties;
}

void QgsLabelSettingsWidgetBase::setDataDefinedProperties( const QgsPropertyCollection &dataDefinedProperties )
{
  mDataDefinedProperties = dataDefinedProperties;

  const auto overrideButtons = findChildren<QgsPropertyOverrideButton *>();
  for ( QgsPropertyOverrideButton *button : overrideButtons )
  {
    const QgsPalLayerSettings::Property key = static_cast<  QgsPalLayerSettings::Property >( button->propertyKey() );
    button->setToProperty( mDataDefinedProperties.property( key ) );
  }
}

void QgsLabelSettingsWidgetBase::updateDataDefinedProperties( QgsPropertyCollection & )
{

}

void QgsLabelSettingsWidgetBase::registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsPalLayerSettings::Property key )
{
  button->init( key, mDataDefinedProperties, QgsPalLayerSettings::propertyDefinitions(), mVectorLayer, true );
  connect( button, &QgsPropertyOverrideButton::changed, this, &QgsLabelSettingsWidgetBase::updateDataDefinedProperty );
  connect( button, &QgsPropertyOverrideButton::createAuxiliaryField, this, &QgsLabelSettingsWidgetBase::createAuxiliaryField );

  button->registerExpressionContextGenerator( this );
}


//
// QgsLabelSettingsWidgetDialog
//

QgsLabelSettingsWidgetDialog::QgsLabelSettingsWidgetDialog( QgsLabelSettingsWidgetBase *widget, QWidget *parent )
  : QDialog( parent )
{
  setWindowTitle( widget->windowTitle() );
  QVBoxLayout *vLayout = new QVBoxLayout();
  vLayout->addWidget( widget );
  mButtonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  vLayout->addWidget( mButtonBox );
  setLayout( vLayout );

  setObjectName( QStringLiteral( "QgsLabelSettingsWidgetDialog" ) );
  QgsGui::enableAutoGeometryRestore( this );
}

QDialogButtonBox *QgsLabelSettingsWidgetDialog::buttonBox()
{
  return mButtonBox;
}
