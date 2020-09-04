/***************************************************************************
                         qgsprocessingfieldformwidgetwrapper.h
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

#include "qgsprocessingfieldformwidgetwrapper.h"
#include "qgsprocessingparameterfieldform.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingmodelparameter.h"
#include "qgsprocessingmodelalgorithm.h"


////
//// QgsProcessingFieldFormParameterDefinitionWidget
////

QgsProcessingFieldFormParameterDefinitionWidget::QgsProcessingFieldFormParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm, QWidget *parent )
  : QgsProcessingAbstractParameterDefinitionWidget( context, widgetContext, definition, algorithm, parent )
{
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setMargin( 0 );
  vlayout->setContentsMargins( 0, 0, 0, 0 );

  vlayout->addWidget( new QLabel( tr( "Parent layer" ) ) );

  mParentLayerComboBox = new QComboBox();
  mParentLayerComboBox->addItem( tr( "None" ), QVariant() );

  QString initialParent;
  if ( const QgsProcessingParameterFieldForm *formParam = dynamic_cast<const QgsProcessingParameterFieldForm *>( definition ) )
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

QgsProcessingParameterDefinition *QgsProcessingFieldFormParameterDefinitionWidget::createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const
{
  auto param = qgis::make_unique< QgsProcessingParameterFieldForm >( name, description, mParentLayerComboBox->currentData().toString() );
  param->setFlags( flags );
  return param.release();
}



//
// QgsProcessingFieldFormWidgetWrapper
//

QgsProcessingFieldFormWidgetWrapper::QgsProcessingFieldFormWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QWidget *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{
}

QString QgsProcessingFieldFormWidgetWrapper::parameterType() const
{
  return QgsProcessingParameterFieldForm::typeName();
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingFieldFormWidgetWrapper::createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  return new QgsProcessingFieldFormWidgetWrapper( parameter, type );
}

QWidget *QgsProcessingFieldFormWidgetWrapper::createWidget()
{
  mPanel = new QgsFieldFormWidget();
  mPanel->setToolTip( parameterDefinition()->toolTip() );

  QgsFieldFormWidget::addStringType( mPanel );
  QgsFieldFormWidget::addIntegerType( mPanel );
  QgsFieldFormWidget::addRealType( mPanel );
  QgsFieldFormWidget::addDateType( mPanel );

  connect( mPanel, &QgsFieldFormWidget::changed, this, [ = ]
  {
    emit widgetValueHasChanged( this );
  } );

  return mPanel;
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingFieldFormWidgetWrapper::createParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  return new QgsProcessingFieldFormParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}

void QgsProcessingFieldFormWidgetWrapper::postInitialize( const QList<QgsAbstractProcessingParameterWidgetWrapper *> &wrappers )
{
  QgsAbstractProcessingParameterWidgetWrapper::postInitialize( wrappers );
  switch ( type() )
  {
    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Batch:
    {
      for ( const QgsAbstractProcessingParameterWidgetWrapper *wrapper : wrappers )
      {
        if ( wrapper->parameterDefinition()->name() == static_cast< const QgsProcessingParameterFieldForm * >( parameterDefinition() )->parentLayerParameterName() )
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

int QgsProcessingFieldFormWidgetWrapper::stretch() const
{
  return 1;
}

void QgsProcessingFieldFormWidgetWrapper::setParentLayerWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *parentWrapper )
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

void QgsProcessingFieldFormWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext & )
{
  if ( !mPanel )
    return;

  if ( value.type() != QVariant::Map )
    return;

  QgsFields destinationFields;
  QMap<QString, QString> expressions;

  const QVariantMap fieldSettingsMap = value.toMap();

  mPanel->setName( fieldSettingsMap.value( QStringLiteral( "name" ) ).toString() );
  mPanel->setType( fieldSettingsMap.value( QStringLiteral( "type" ) ).toString() );
  mPanel->setLength( fieldSettingsMap.value( QStringLiteral( "length" ) ).toUInt() );
  mPanel->setPrecision( fieldSettingsMap.value( QStringLiteral( "precision" ) ).toDouble() );
}

QVariant QgsProcessingFieldFormWidgetWrapper::widgetValue() const
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

QStringList QgsProcessingFieldFormWidgetWrapper::compatibleParameterTypes() const
{
  return QStringList()
         << QgsProcessingParameterFieldForm::typeName();
}

QStringList QgsProcessingFieldFormWidgetWrapper::compatibleOutputTypes() const
{
  return QStringList();
}

QString QgsProcessingFieldFormWidgetWrapper::modelerExpressionFormatString() const
{
  return tr( "map with 'name', 'type', 'comment' (and optional 'alias', length' and 'precision' values)." );
}

const QgsVectorLayer *QgsProcessingFieldFormWidgetWrapper::linkedVectorLayer() const
{
  if ( mPanel && mPanel->layer() )
    return mPanel->layer();

  return QgsAbstractProcessingParameterWidgetWrapper::linkedVectorLayer();
}

/// @endcond
