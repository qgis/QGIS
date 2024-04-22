/***************************************************************************
                         qgsprocessingmodelconfig.cpp
                         ----------------------
    begin                : April 2024
    copyright            : (C) 2024 by Nyall Dawson
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

#include "qgsprocessingmodelconfig.h"
#include "qgsmaplayerstore.h"

QgsProcessingModelInitialRunConfig::QgsProcessingModelInitialRunConfig() = default;

QgsProcessingModelInitialRunConfig::~QgsProcessingModelInitialRunConfig() = default;

QgsMapLayerStore *QgsProcessingModelInitialRunConfig::previousLayerStore()
{
  return mModelInitialLayerStore.get();
}

std::unique_ptr<QgsMapLayerStore> QgsProcessingModelInitialRunConfig::takePreviousLayerStore()
{
  return std::move( mModelInitialLayerStore );
}

void QgsProcessingModelInitialRunConfig::setPreviousLayerStore( std::unique_ptr<QgsMapLayerStore> store )
{
  if ( store )
  {
    Q_ASSERT_X( !store->thread(), "QgsProcessingModelInitialRunConfig::setPreviousLayerStore", "store must have been pushed to a nullptr thread prior to calling this method" );
  }
  mModelInitialLayerStore = std::move( store );
}
