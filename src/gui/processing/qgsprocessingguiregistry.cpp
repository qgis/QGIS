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
#include "qgis.h"

QgsProcessingGuiRegistry::QgsProcessingGuiRegistry()
{
  addAlgorithmConfigurationWidgetFactory( new QgsFilterAlgorithmConfigurationWidgetFactory() );
}

QgsProcessingGuiRegistry::~QgsProcessingGuiRegistry()
{
  for ( QgsProcessingAlgorithmConfigurationWidgetFactory *factory : qgis::as_const( mAlgorithmConfigurationWidgetFactories ) )
    removeAlgorithmConfigurationWidgetFactory( factory );
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

QgsProcessingAlgorithmConfigurationWidget *QgsProcessingGuiRegistry::algorithmConfigurationWidget( QgsProcessingAlgorithm *algorithm ) const
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
