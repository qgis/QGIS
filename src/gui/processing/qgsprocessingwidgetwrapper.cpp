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

QString QgsProcessingParameterWidgetContext::modelChildAlgorithmId() const
{
  return mModelChildAlgorithmId;
}

void QgsProcessingParameterWidgetContext::setModelChildAlgorithmId( const QString &modelChildAlgorithmId )
{
  mModelChildAlgorithmId = modelChildAlgorithmId;
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
  if ( mType != QgsProcessingGui::Batch && mParameterDefinition->isDynamic() )
  {
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setMargin( 0 );
    hLayout->setContentsMargins( 0, 0, 0, 0 );
    hLayout->addWidget( mWidget, 1 );
    mPropertyButton = new QgsPropertyOverrideButton();
    hLayout->addWidget( mPropertyButton );
    mPropertyButton->init( 0, QgsProperty(), mParameterDefinition->dynamicPropertyDefinition() );
    mPropertyButton->registerEnabledWidget( mWidget, false );

    wrappedWidget = new QWidget();
    wrappedWidget->setLayout( hLayout );
  }

  setWidgetValue( mParameterDefinition->defaultValue(), context );

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

void QgsAbstractProcessingParameterWidgetWrapper::registerProcessingContextGenerator( QgsProcessingContextGenerator *generator )
{
  mProcessingContextGenerator = generator;
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
      std::unique_ptr< QLabel > label = qgis::make_unique< QLabel >( description );
      label->setToolTip( mParameterDefinition->toolTip() );
      return label.release();
    }
  }
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
            setDynamicParentLayerParameter( wrapper->parameterValue() );
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

void QgsAbstractProcessingParameterWidgetWrapper::parentLayerChanged( QgsAbstractProcessingParameterWidgetWrapper *wrapper )
{
  if ( wrapper )
  {
    setDynamicParentLayerParameter( wrapper->parameterValue() );
  }
}

void QgsAbstractProcessingParameterWidgetWrapper::setDynamicParentLayerParameter( const QVariant &value )
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
      tmpContext = qgis::make_unique< QgsProcessingContext >();
      context = tmpContext.get();
    }

    QgsVectorLayer *layer = QgsProcessingParameters::parameterAsVectorLayer( parameterDefinition(), value, *context );
    if ( !layer )
    {
      mPropertyButton->setVectorLayer( nullptr );
      return;
    }

    // need to grab ownership of layer if required - otherwise layer may be deleted when context
    // goes out of scope
    std::unique_ptr< QgsMapLayer > ownedLayer( context->takeResultLayer( layer->id() ) );
    if ( ownedLayer && ownedLayer->type() == QgsMapLayer::VectorLayer )
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
  std::unique_ptr< QgsProcessingModelerParameterWidget > widget = qgis::make_unique< QgsProcessingModelerParameterWidget >( model, childId, parameter, context );
  widget->populateSources( compatibleParameterTypes(), compatibleOutputTypes(), compatibleDataTypes() );
  widget->setExpressionHelpText( modelerExpressionFormatString() );
  return widget.release();
}

QString QgsProcessingParameterWidgetFactoryInterface::modelerExpressionFormatString() const
{
  return QString();
}

