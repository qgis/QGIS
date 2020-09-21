/***************************************************************************
                         qgsprocessingfielddefinitionwidgetwrapper.h
                         ----------------------
    begin                : September 2020
    copyright            : (C) 2020 by Ivan Ivanov
    email                : ivan@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingfielddefinitionwidgetwrapper.h"
#include "qgsprocessingparameterfielddefinition.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingmodelparameter.h"
#include "qgsprocessingmodelalgorithm.h"


////
//// QgsProcessingFieldDefinitionParameterDefinitionWidget
////

QgsProcessingFieldDefinitionParameterDefinitionWidget::QgsProcessingFieldDefinitionParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setMargin( 0 );
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Parent layer" ) ) );

  mParentLayerComboBox = new QComboBox();
  mParentLayerComboBox->addItem( tr( "None" ), QVariant() );

  QString initialParent;
  if ( const QgsProcessingParameterFieldDefinition *formParam = dynamic_cast<const QgsProcessingParameterFieldDefinition *>( definition ) )
    initialParent = formParam->parentLayerParameterName();

  if ( widgetContext.model() )
  {
    // populate combo box with other model input choices
    const QMap<QString, QgsProcessingModelParameter> components = widgetContext.model()->parameterComponents();
    for ( auto it = components.constBegin(); it != components.constEnd(); ++it )
    {
      if ( const QgsProcessingParameterFeatureSource *definition = dynamic_cast< const QgsProcessingParameterFeatureSource * >( widgetContext.model()->parameterDefinition( it.value().parameterName() ) ) )
      {
        mParentLayerComboBox-> addItem( definition->description(), definition->name() );
        if ( !initialParent.isEmpty() && initialParent == definition->name() )
        {
          mParentLayerComboBox->setCurrentIndex( mParentLayerComboBox->count() - 1 );
        }
      }
      else if ( const QgsProcessingParameterVectorLayer *definition = dynamic_cast< const QgsProcessingParameterVectorLayer * >( widgetContext.model()->parameterDefinition( it.value().parameterName() ) ) )
      {
        mParentLayerComboBox-> addItem( definition->description(), definition->name() );
        if ( !initialParent.isEmpty() && initialParent == definition->name() )
        {
          mParentLayerComboBox->setCurrentIndex( mParentLayerComboBox->count() - 1 );
        }
      }
    }
  }

  if ( mParentLayerComboBox->count() == 1 && !initialParent.isEmpty() )
  {
    // if no parent candidates found, we just add the existing one as a placeholder
    mParentLayerComboBox->addItem( initialParent, initialParent );
    mParentLayerComboBox->setCurrentIndex( mParentLayerComboBox->count() - 1 );
  }

  vlayout->addWidget( mParentLayerComboBox );
  setLayout( vlayout );
}

QgsProcessingParameterDefinition *QgsProcessingFieldDefinitionParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  auto param = qgis::make_unique< QgsProcessingParameterFieldDefinition >( name, description, mParentLayerComboBox->currentData().toString() );
  param->setFlags( flags );
  return param.release();
}



//
// QgsProcessingFieldDefinitionWidgetWrapper
//

QgsProcessingFieldDefinitionWidgetWrapper::QgsProcessingFieldDefinitionWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{
}

QString QgsProcessingFieldDefinitionWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterFieldDefinition::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingFieldDefinitionWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingFieldDefinitionWidgetWrapper( parameter, type );
}

QWidget *QgsProcessingFieldDefinitionWidgetWrapper::createWidget()
{
  mPanel = new QgsFieldDefinitionWidget();
  mPanel->setToolTip( parameterDefinition()->toolTip() );
  mPanel->addTypes( QList<QVariant::Type>()
                    << QVariant::String
                    << QVariant::Int
                    << QVariant::Double
                    << QVariant::Date );

  connect( mPanel, &QgsFieldDefinitionWidget::changed, this, [ = ]
  {
    emit widgetValueHasChanged( this );
  } );

  return mPanel;
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingFieldDefinitionWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingFieldDefinitionParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}

void QgsProcessingFieldDefinitionWidgetWrapper::postInitialize( const QList<QgsAbstractProcessingParameterWidgetWrapper *> &wrappers )
{
  QgsAbstractProcessingParameterWidgetWrapper::postInitialize( wrappers );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
    {
      for ( const QgsAbstractProcessingParameterWidgetWrapper *wrapper : wrappers )
      {
        if ( wrapper->parameterDefinition()->name() == static_cast< const QgsProcessingParameterFieldDefinition * >( parameterDefinition() )->parentLayerParameterName() )
        {
          setParentLayerWrapperValue( wrapper );
          connect( wrapper, &QgsAbstractProcessingParameterWidgetWrapper::widgetValueHasChanged, this, [ = ]
          {
            setParentLayerWrapperValue( wrapper );
          } );
          break;
        }
      }
      break;
    }

    case QgsProcessingGui::Modeler:
      break;
  }
}

int QgsProcessingFieldDefinitionWidgetWrapper::stretch() const
{
  return 1;
}

void QgsProcessingFieldDefinitionWidgetWrapper::setParentLayerWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *parentWrapper )
{
  // evaluate value to layer
  QgsProcessingContext *context = nullptr;
  std::unique_ptr< QgsProcessingContext > tmpContext;
  if ( mProcessingContextGenerator )
    context = mProcessingContextGenerator->processingContext();

  if ( !context )
  {
    tmpContext = qgis::make_unique< QgsProcessingContext >();
    context = tmpContext.get();
  }

  QgsVectorLayer *layer = QgsProcessingParameters::parameterAsVectorLayer( parentWrapper->parameterDefinition(), parentWrapper->parameterValue(), *context );
  if ( !layer )
  {
    if ( mPanel )
      mPanel->setLayer( nullptr );
    return;
  }

  // need to grab ownership of layer if required - otherwise layer may be deleted when context
  // goes out of scope
  std::unique_ptr< QgsMapLayer > ownedLayer( context->takeResultLayer( layer->id() ) );
  if ( ownedLayer && ownedLayer->type() == QgsMapLayerType::VectorLayer )
  {
    mParentLayer.reset( qobject_cast< QgsVectorLayer * >( ownedLayer.release() ) );
    layer = mParentLayer.get();
  }
  else
  {
    // don't need ownership of this layer - it wasn't owned by context (so e.g. is owned by the project)
  }

  if ( mPanel )
    mPanel->setLayer( layer );
}

void QgsProcessingFieldDefinitionWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext & )
{
  if ( !mPanel )
    return;

  if ( value.type() != QVariant::Map )
    return;

  QgsFields destinationFields;
  QMap<QString, QString> expressions;

  const QVariantMap fieldSettingsMap = value.toMap();

  mPanel->setName( fieldSettingsMap.value( QStringLiteral( "name" ) ).toString() );
  mPanel->setType( static_cast<QVariant::Type>( fieldSettingsMap.value( QStringLiteral( "type" ) ).toInt() ) );
  mPanel->setLength( fieldSettingsMap.value( QStringLiteral( "length" ) ).toUInt() );
  mPanel->setPrecision( fieldSettingsMap.value( QStringLiteral( "precision" ) ).toDouble() );
}

QVariant QgsProcessingFieldDefinitionWidgetWrapper::widgetValue() const
{
  if ( !mPanel )
    return QVariantMap();

  std::unique_ptr< QgsField > field;
  field.reset( mPanel->asField() );

  if ( !field.get() )
    return QVariantMap();

  return QVariantMap(
  {
    {"name", field->name()},
    {"type", field->typeName()},
    {"length", field->length()},
    {"precision", field->precision()},
  } );
}

QStringList QgsProcessingFieldDefinitionWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterFieldDefinition::typeName();
}

QStringList QgsProcessingFieldDefinitionWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList();
}

QString QgsProcessingFieldDefinitionWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "map with 'name', 'type', 'comment' (and optional 'alias', length' and 'precision' values)." );
}

const QgsVectorLayer *QgsProcessingFieldDefinitionWidgetWrapper::linkedVectorLayer() const
{
  if ( mPanel && mPanel->layer() )
    return mPanel->layer();

  return QgsAbstractProcessingParameterWidgetWrapper::linkedVectorLayer();
}

/// @endcond
