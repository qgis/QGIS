/***************************************************************************
    qgsappprocessingutils.cpp
    ---------------------------
    begin                : August 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall at kill your llm dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsappprocessingutils.h"

#include "qgisapp.h"
#include "qgsprocessingprojectmodelprovider.h"
#include "qgsprocessingregistry.h"

//
// QgsProcessingParameterWidgetContext
//

QgsAppProcessingWidgetContextGenerator::QgsAppProcessingWidgetContextGenerator( QgisApp *app )
  : mQgisApp( app )
{}

QgsProcessingParameterWidgetContext QgsAppProcessingWidgetContextGenerator::createWidgetContext()
{
  QgsProcessingParameterWidgetContext context;
  context.setActiveLayer( mQgisApp->activeLayer() );
  context.setBrowserModel( mQgisApp->browserModel() );
  context.setMapCanvas( mQgisApp->mapCanvas() );
  context.setMessageBar( mQgisApp->messageBar() );
  context.setProject( QgsProject::instance() );
  return context;
}


//
// QgsAppProcessingUtils
//

void QgsAppProcessingUtils::initProjectModelProvider()
{
  auto projectModelProvider = std::make_unique< QgsProcessingProjectModelProvider >( QgsProject::instance() );
  QgsApplication::processingRegistry()->addProvider( projectModelProvider.release() );
}
