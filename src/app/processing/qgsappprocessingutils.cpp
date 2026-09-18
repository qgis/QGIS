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
#include "qgsexpressioncontextutils.h"
#include "qgsmapcanvas.h"
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
// QgsAppProcessingContextFactory
//

QgsAppProcessingContextFactory::QgsAppProcessingContextFactory( QgisApp *app )
  : mQgisApp( app )
{}

QgsProcessingContext *QgsAppProcessingContextFactory::createContext( QgsProcessingFeedback *feedback )
{
  auto context = std::make_unique< QgsProcessingContext >();

  context->setProject( QgsProject::instance() );
  context->setFeedback( feedback );

  QgsSettings settings;

  bool ok = false;
  const int invalid = settings.value( "/Processing/Configuration/FILTER_INVALID_GEOMETRIES" ).toInt( &ok );
  if ( ok )
  {
    switch ( invalid )
    {
      case 0:
        context->setInvalidGeometryCheck( Qgis::InvalidGeometryCheck::NoCheck );
        break;
      case 1:
        context->setInvalidGeometryCheck( Qgis::InvalidGeometryCheck::SkipInvalid );
        break;
      case 2:
      default:
        context->setInvalidGeometryCheck( Qgis::InvalidGeometryCheck::AbortOnInvalid );
        break;
    }
  }
  else
  {
    context->setInvalidGeometryCheck( Qgis::InvalidGeometryCheck::AbortOnInvalid );
  }

  context->setDefaultEncoding( QgsProcessingUtils::resolveDefaultEncoding( settings.value( "/Processing/encoding" ).toString() ) );

  context->setExpressionContext( createExpressionContext() );

  if ( QgsMapCanvas *canvas = mQgisApp->mapCanvas() )
  {
    if ( canvas->mapSettings().isTemporal() )
    {
      context->setCurrentTimeRange( canvas->mapSettings().temporalRange() );
    }
  }

  return context.release();
}

QgsExpressionContext QgsAppProcessingContextFactory::createExpressionContext() const
{
  QgsExpressionContext context;
  context.appendScope( QgsExpressionContextUtils::globalScope() );
  context.appendScope( QgsExpressionContextUtils::projectScope( QgsProject::instance() ) );

  if ( QgsMapCanvas *canvas = mQgisApp->mapCanvas() )
  {
    if ( canvas->mapSettings().isTemporal() )
    {
      context.appendScope( QgsExpressionContextUtils::mapSettingsScope( canvas->mapSettings() ) );
    }
  }

  auto processingScope = new QgsExpressionContextScope();
  if ( QgsMapCanvas *canvas = mQgisApp->mapCanvas() )
  {
    const QgsRectangle extent = canvas->fullExtent();
    processingScope->setVariable( "fullextent_minx", extent.xMinimum() );
    processingScope->setVariable( "fullextent_miny", extent.yMinimum() );
    processingScope->setVariable( "fullextent_maxx", extent.xMaximum() );
    processingScope->setVariable( "fullextent_maxy", extent.yMaximum() );
  }
  context.appendScope( processingScope );

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
