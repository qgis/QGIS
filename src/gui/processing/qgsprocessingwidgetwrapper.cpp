/***************************************************************************
                         qgsprocessingwidgetwrapper.cpp
                         ---------------------
    begin                : August 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsprocessingwidgetwrapper.h"
#include "qgsprocessingparameters.h"
#include "qgsprocessingmodelerparameterwidget.h"
#include "qgspropertyoverridebutton.h"
#include "qgsexpressioncontext.h"
#include "models/qgsprocessingmodelalgorithm.h"
#include "qgsexpressioncontextutils.h"
#include "qgsprocessingwidgetwrapperimpl.h"
#include <QLabel>
#include <QHBoxLayout>

//
// QgsProcessingParameterWidgetContext
//

void QgsProcessingParameterWidgetContext::setMapCanvas( QgsMapCanvas *canvas )
{
  mMapCanvas = canvas;
}

QgsMapCanvas *QgsProcessingParameterWidgetContext::mapCanvas() const
{
  return mMapCanvas;
}

void QgsProcessingParameterWidgetContext::setMessageBar( QgsMessageBar *bar )
{
  mMessageBar = bar;
}

QgsMessageBar *QgsProcessingParameterWidgetContext::messageBar() const
{
  return mMessageBar;
}

void QgsProcessingParameterWidgetContext::setBrowserModel( QgsBrowserGuiModel *model )
{
  mBrowserModel = model;
}

QgsBrowserGuiModel *QgsProcessingParameterWidgetContext::browserModel() const
{
  return mBrowserModel;
}

void QgsProcessingParameterWidgetContext::setProject( QgsProject *project )
{
  mProject = project;
}

QgsProject *QgsProcessingParameterWidgetContext::project() const
{
  return mProject;
}

QString QgsProcessingParameterWidgetContext::modelChildAlgorithmId() const
{
  return mModelChildAlgorithmId;
}

void QgsProcessingParameterWidgetContext::setModelChildAlgorithmId( const QString &modelChildAlgorithmId )
{
  mModelChildAlgorithmId = modelChildAlgorithmId;
}

QgsMapLayer *QgsProcessingParameterWidgetContext::activeLayer() const
{
  return mActiveLayer;
}

void QgsProcessingParameterWidgetContext::setActiveLayer( QgsMapLayer *activeLayer )
{
  mActiveLayer = activeLayer;
}

QgsProcessingModelAlgorithm *QgsProcessingParameterWidgetContext::model() const
{
  return mModel;
}

void QgsProcessingParameterWidgetContext::setModel( QgsProcessingModelAlgorithm *model )
{
  mModel = model;
}


//
// QgsAbstractProcessingParameterWidgetWrapper
//

QgsAbstractProcessingParameterWidgetWrapper::QgsAbstractProcessingParameterWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QObject *parent )
  : QObject( parent )
  , mType( type )
  , mParameterDefinition( parameter )
{
}

QgsProcessingGui::WidgetType QgsAbstractProcessingParameterWidgetWrapper::type() const
{
  return mType;
}

void QgsAbstractProcessingParameterWidgetWrapper::setWidgetContext( const QgsProcessingParameterWidgetContext &context )
{
  mWidgetContext = context;
}

const QgsProcessingParameterWidgetContext &QgsAbstractProcessingParameterWidgetWrapper::widgetContext() const
{
  return mWidgetContext;
}

QWidget *QgsAbstractProcessingParameterWidgetWrapper::createWrappedWidget( QgsProcessingContext &context )
{
  if ( mWidget )
    return mWidget;

  mWidget = createWidget();
  QWidget *wrappedWidget = mWidget;
  if ( mParameterDefinition->isDynamic() )
  {
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins( 0, 0, 0, 0 );
    hLayout->addWidget( mWidget, 1 );
    mPropertyButton = new QgsPropertyOverrideButton();
    hLayout->addWidget( mPropertyButton );
    mPropertyButton->init( 0, QgsProperty(), mParameterDefinition->dynamicPropertyDefinition() );
    mPropertyButton->registerEnabledWidget( mWidget, false );
    mPropertyButton->registerExpressionContextGenerator( this );

    wrappedWidget = new QWidget();
    wrappedWidget->setLayout( hLayout );
  }

  if ( !dynamic_cast<const QgsProcessingDestinationParameter * >( mParameterDefinition ) )
  {
    // an exception -- output widgets handle this themselves
    setWidgetValue( mParameterDefinition->defaultValueForGui(), context );
  }

  return wrappedWidget;
}

QLabel *QgsAbstractProcessingParameterWidgetWrapper::createWrappedLabel()
{
  if ( mLabel )
    return mLabel;

  mLabel = createLabel();
  return mLabel;
}

QWidget *QgsAbstractProcessingParameterWidgetWrapper::wrappedWidget()
{
  return mWidget;
}

QLabel *QgsAbstractProcessingParameterWidgetWrapper::wrappedLabel()
{
  return mLabel;
}

const QgsProcessingParameterDefinition *QgsAbstractProcessingParameterWidgetWrapper::parameterDefinition() const
{
  return mParameterDefinition;
}

void QgsAbstractProcessingParameterWidgetWrapper::setParameterValue( const QVariant &value, QgsProcessingContext &context )
{
  if ( mPropertyButton && value.canConvert< QgsProperty >() )
  {
    mPropertyButton->setToProperty( value.value< QgsProperty >() );
  }
  else
  {
    if ( mPropertyButton )
      mPropertyButton->setToProperty( QgsProperty() );

    setWidgetValue( value, context );
  }
}

QVariant QgsAbstractProcessingParameterWidgetWrapper::parameterValue() const
{
  if ( mPropertyButton && mPropertyButton->isActive() )
    return mPropertyButton->toProperty();
  else
    return widgetValue();
}

QVariantMap QgsAbstractProcessingParameterWidgetWrapper::customProperties() const
{
  return QVariantMap();
}

void QgsAbstractProcessingParameterWidgetWrapper::registerProcessingContextGenerator( QgsProcessingContextGenerator *generator )
{
  mProcessingContextGenerator = generator;
}

void QgsAbstractProcessingParameterWidgetWrapper::registerProcessingParametersGenerator( QgsProcessingParametersGenerator *generator )
{
  mParametersGenerator = generator;
}

QLabel *QgsAbstractProcessingParameterWidgetWrapper::createLabel()
{
  switch ( mType )
  {
    case QgsProcessingGui::Batch:
      return nullptr;

    case QgsProcessingGui::Standard:
    case QgsProcessingGui::Modeler:
    {
      QString description = mParameterDefinition->description();
      if ( parameterDefinition()->flags() & QgsProcessingParameterDefinition::FlagOptional )
        description = QObject::tr( "%1 [optional]" ).arg( description );
      std::unique_ptr< QLabel > label = std::make_unique< QLabel >( description );
      label->setToolTip( mParameterDefinition->toolTip() );
      return label.release();
    }
  }
  return nullptr;
}

const QgsVectorLayer *QgsAbstractProcessingParameterWidgetWrapper::linkedVectorLayer() const
{
  if ( mPropertyButton )
    return mPropertyButton->vectorLayer();
  return nullptr;
}

void QgsAbstractProcessingParameterWidgetWrapper::postInitialize( const QList<QgsAbstractProcessingParameterWidgetWrapper *> &wrappers )
{
  switch ( mType )
  {
    case QgsProcessingGui::Batch:
    case QgsProcessingGui::Standard:
    {
      if ( parameterDefinition()->isDynamic() )
      {
        for ( const QgsAbstractProcessingParameterWidgetWrapper *wrapper : wrappers )
        {
          if ( wrapper->parameterDefinition()->name() == parameterDefinition()->dynamicLayerParameterName() )
          {
            setDynamicParentLayerParameter( wrapper );
            connect( wrapper, &QgsAbstractProcessingParameterWidgetWrapper::widgetValueHasChanged, this, &QgsAbstractProcessingParameterWidgetWrapper::parentLayerChanged );
            break;
          }
        }
      }
      break;
    }

    case QgsProcessingGui::Modeler:
      break;
  }
}

int QgsAbstractProcessingParameterWidgetWrapper::stretch() const
{
  return 0;
}

QgsExpressionContext QgsAbstractProcessingParameterWidgetWrapper::createExpressionContext() const
{
  QgsExpressionContext context = QgsProcessingGuiUtils::createExpressionContext( mProcessingContextGenerator, mWidgetContext, mParameterDefinition ? mParameterDefinition->algorithm() : nullptr, linkedVectorLayer() );
  if ( mParameterDefinition && !mParameterDefinition->additionalExpressionContextVariables().isEmpty() )
  {
    std::unique_ptr< QgsExpressionContextScope > paramScope = std::make_unique< QgsExpressionContextScope >();
    const QStringList additional = mParameterDefinition->additionalExpressionContextVariables();
    for ( const QString &var : additional )
    {
      paramScope->setVariable( var, QVariant() );
    }
    context.appendScope( paramScope.release() );

    // we always highlight additional variables for visibility
    QStringList highlighted = context.highlightedVariables();
    highlighted.append( additional );
    context.setHighlightedVariables( highlighted );
  }
  return context;
}

void QgsAbstractProcessingParameterWidgetWrapper::setDialog( QDialog * )
{

}

void QgsAbstractProcessingParameterWidgetWrapper::parentLayerChanged( QgsAbstractProcessingParameterWidgetWrapper *wrapper )
{
  if ( wrapper )
  {
    setDynamicParentLayerParameter( wrapper );
  }
}

void QgsAbstractProcessingParameterWidgetWrapper::setDynamicParentLayerParameter( const QgsAbstractProcessingParameterWidgetWrapper *parentWrapper )
{
  if ( mPropertyButton )
  {
    // evaluate value to layer
    QgsProcessingContext *context = nullptr;
    std::unique_ptr< QgsProcessingContext > tmpContext;
    if ( mProcessingContextGenerator )
      context = mProcessingContextGenerator->processingContext();

    if ( !context )
    {
      tmpContext = std::make_unique< QgsProcessingContext >();
      context = tmpContext.get();
    }

    QVariant val = parentWrapper->parameterValue();
    if ( val.canConvert<QgsProcessingFeatureSourceDefinition>() )
    {
      // input is a QgsProcessingFeatureSourceDefinition - get extra properties from it
      const QgsProcessingFeatureSourceDefinition fromVar = qvariant_cast<QgsProcessingFeatureSourceDefinition>( val );
      val = fromVar.source;
    }

    QgsVectorLayer *layer = QgsProcessingParameters::parameterAsVectorLayer( parentWrapper->parameterDefinition(), val, *context );
    if ( !layer )
    {
      mPropertyButton->setVectorLayer( nullptr );
      return;
    }

    // need to grab ownership of layer if required - otherwise layer may be deleted when context
    // goes out of scope
    std::unique_ptr< QgsMapLayer > ownedLayer( context->takeResultLayer( layer->id() ) );
    if ( ownedLayer && ownedLayer->type() == QgsMapLayerType::VectorLayer )
    {
      mDynamicLayer.reset( qobject_cast< QgsVectorLayer * >( ownedLayer.release() ) );
      layer = mDynamicLayer.get();
    }
    else
    {
      // don't need ownership of this layer - it wasn't owned by context (so e.g. is owned by the project)
    }

    mPropertyButton->setVectorLayer( layer );
  }
}

QgsProcessingModelerParameterWidget *QgsProcessingParameterWidgetFactoryInterface::createModelerWidgetWrapper( QgsProcessingModelAlgorithm *model, const QString &childId, const QgsProcessingParameterDefinition *parameter, QgsProcessingContext &context )
{
  std::unique_ptr< QgsProcessingModelerParameterWidget > widget = std::make_unique< QgsProcessingModelerParameterWidget >( model, childId, parameter, context );
  widget->populateSources( compatibleParameterTypes(), compatibleOutputTypes(), compatibleDataTypes( parameter ) );
  widget->setExpressionHelpText( modelerExpressionFormatString() );

  if ( parameter->isDestination() )
    widget->setSourceType( QgsProcessingModelChildParameterSource::ModelOutput );
  else
    widget->setSourceType( defaultModelSource( parameter ) );

  return widget.release();
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingParameterWidgetFactoryInterface::createParameterDefinitionWidget( QgsProcessingContext &,
    const QgsProcessingParameterWidgetContext &, const QgsProcessingParameterDefinition *,
    const QgsProcessingAlgorithm * )
{
  return nullptr;
}

QList<int> QgsProcessingParameterWidgetFactoryInterface::compatibleDataTypes( const QgsProcessingParameterDefinition * ) const
{
  return QList< int >();
}

QString QgsProcessingParameterWidgetFactoryInterface::modelerExpressionFormatString() const
{
  return QString();
}

QgsProcessingModelChildParameterSource::Source QgsProcessingParameterWidgetFactoryInterface::defaultModelSource( const QgsProcessingParameterDefinition * ) const
{
  return QgsProcessingModelChildParameterSource::StaticValue;
}

//
// QgsProcessingGuiUtils
//

///@cond PRIVATE
QgsExpressionContext QgsProcessingGuiUtils::createExpressionContext( QgsProcessingContextGenerator *processingContextGenerator, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingAlgorithm *algorithm, const QgsVectorLayer *linkedLayer )
{
  // Get a processing context to start with
  QgsProcessingContext *context = nullptr;
  std::unique_ptr< QgsProcessingContext > tmpContext;
  if ( processingContextGenerator )
    context = processingContextGenerator->processingContext();

  if ( !context )
  {
    tmpContext = std::make_unique< QgsProcessingContext >();
    context = tmpContext.get();
  }

  QgsExpressionContext c = context->expressionContext();

  if ( auto *lModel = widgetContext.model() )
  {
    c << QgsExpressionContextUtils::processingModelAlgorithmScope( lModel, QVariantMap(), *context );

    const QgsProcessingAlgorithm *alg = nullptr;
    if ( lModel->childAlgorithms().contains( widgetContext.modelChildAlgorithmId() ) )
      alg = lModel->childAlgorithm( widgetContext.modelChildAlgorithmId() ).algorithm();

    QgsExpressionContextScope *algorithmScope = QgsExpressionContextUtils::processingAlgorithmScope( alg ? alg : algorithm, QVariantMap(), *context );
    c << algorithmScope;
    QgsExpressionContextScope *childScope = lModel->createExpressionContextScopeForChildAlgorithm( widgetContext.modelChildAlgorithmId(), *context, QVariantMap(), QVariantMap() );
    c << childScope;

    QStringList highlightedVariables = childScope->variableNames();
    QStringList highlightedFunctions = childScope->functionNames();
    highlightedVariables += algorithmScope->variableNames();
    highlightedVariables += lModel->variables().keys();
    highlightedFunctions += algorithmScope->functionNames();
    c.setHighlightedVariables( highlightedVariables );
    c.setHighlightedFunctions( highlightedFunctions );
  }
  else
  {
    if ( algorithm )
      c << QgsExpressionContextUtils::processingAlgorithmScope( algorithm, QVariantMap(), *context );
  }

  if ( linkedLayer )
    c << QgsExpressionContextUtils::layerScope( linkedLayer );

  return c;
}
///@endcond

QgsProcessingHiddenWidgetWrapper::QgsProcessingHiddenWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type, QObject *parent )
  : QgsAbstractProcessingParameterWidgetWrapper( parameter, type, parent )
{

}

void QgsProcessingHiddenWidgetWrapper::setWidgetValue( const QVariant &value, QgsProcessingContext & )
{
  if ( mValue == value )
    return;

  mValue = value;
  emit widgetValueHasChanged( this );
}

QVariant QgsProcessingHiddenWidgetWrapper::widgetValue() const
{
  return mValue;
}

const QgsVectorLayer *QgsProcessingHiddenWidgetWrapper::linkedVectorLayer() const
{
  return mLayer;
}

void QgsProcessingHiddenWidgetWrapper::setLinkedVectorLayer( const QgsVectorLayer *layer )
{
  mLayer = layer;
}

QWidget *QgsProcessingHiddenWidgetWrapper::createWidget()
{
  return nullptr;

}

QLabel *QgsProcessingHiddenWidgetWrapper::createLabel()
{
  return nullptr;
}
