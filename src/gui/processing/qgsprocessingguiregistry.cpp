/***************************************************************************
                         qgsprocessingguiregistry.cpp
                         ---------------------
    begin                : April 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingguiregistry.h"
#include "qgsprocessingalgorithmconfigurationwidget.h"
#include "qgsprocessingconfigurationwidgets.h"
#include "qgsprocessingwidgetwrapperimpl.h"
#include "qgsprocessingparameters.h"
#include "qgis.h"
#include "qgslogger.h"

QgsProcessingGuiRegistry::QgsProcessingGuiRegistry()
{
  addAlgorithmConfigurationWidgetFactory( new QgsFilterAlgorithmConfigurationWidgetFactory() );

  addParameterWidgetFactory( new QgsProcessingBooleanWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingCrsWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingStringWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingNumericWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingDistanceWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingRangeWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingAuthConfigWidgetWrapper() );
}

QgsProcessingGuiRegistry::~QgsProcessingGuiRegistry()
{
  const QList< QgsProcessingAlgorithmConfigurationWidgetFactory * > factories = mAlgorithmConfigurationWidgetFactories;
  for ( QgsProcessingAlgorithmConfigurationWidgetFactory *factory : factories )
    removeAlgorithmConfigurationWidgetFactory( factory );
  const QMap< QString, QgsProcessingParameterWidgetFactoryInterface * > paramFactories = mParameterWidgetFactories;
  for ( auto it = paramFactories.constBegin(); it != paramFactories.constEnd(); ++it )
    removeParameterWidgetFactory( it.value() );
}

void QgsProcessingGuiRegistry::addAlgorithmConfigurationWidgetFactory( QgsProcessingAlgorithmConfigurationWidgetFactory *factory )
{
  mAlgorithmConfigurationWidgetFactories.append( factory );
}

void QgsProcessingGuiRegistry::removeAlgorithmConfigurationWidgetFactory( QgsProcessingAlgorithmConfigurationWidgetFactory *factory )
{
  mAlgorithmConfigurationWidgetFactories.removeAll( factory );
  delete factory;
}

QgsProcessingAlgorithmConfigurationWidget *QgsProcessingGuiRegistry::algorithmConfigurationWidget( const QgsProcessingAlgorithm *algorithm ) const
{
  for ( const auto *factory : mAlgorithmConfigurationWidgetFactories )
  {
    if ( factory->canCreateFor( algorithm ) )
    {
      return factory->create( algorithm );
    }
  }

  return nullptr;
}

bool QgsProcessingGuiRegistry::addParameterWidgetFactory( QgsProcessingParameterWidgetFactoryInterface *factory )
{
  if ( !factory )
    return false;

  if ( mParameterWidgetFactories.contains( factory->parameterType() ) )
  {
    QgsLogger::warning( QStringLiteral( "Duplicate parameter factory for %1 registered" ).arg( factory->parameterType() ) );
    return false;
  }

  mParameterWidgetFactories.insert( factory->parameterType(), factory );
  return true;
}

void QgsProcessingGuiRegistry::removeParameterWidgetFactory( QgsProcessingParameterWidgetFactoryInterface *factory )
{
  if ( !factory )
    return;

  mParameterWidgetFactories.remove( factory->parameterType() );
  delete factory;
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingGuiRegistry::createParameterWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  if ( !parameter )
    return nullptr;

  const QString parameterType = parameter->type();
  if ( !mParameterWidgetFactories.contains( parameterType ) )
    return nullptr;

  return mParameterWidgetFactories.value( parameterType )->createWidgetWrapper( parameter, type );
}

QgsProcessingModelerParameterWidget *QgsProcessingGuiRegistry::createModelerParameterWidget( QgsProcessingModelAlgorithm *model, const QString &childId, const QgsProcessingParameterDefinition *parameter, QgsProcessingContext &context )
{
  if ( !parameter )
    return nullptr;

  const QString parameterType = parameter->type();
  if ( !mParameterWidgetFactories.contains( parameterType ) )
    return nullptr;

  return mParameterWidgetFactories.value( parameterType )->createModelerWidgetWrapper( model, childId, parameter, context );
}

