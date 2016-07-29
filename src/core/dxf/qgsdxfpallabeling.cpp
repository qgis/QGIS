/***************************************************************************
                         qgsdxfpallabeling.cpp
                         ---------------------
    begin                : January 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdxfpallabeling.h"
#include "qgsdxfexport.h"
#include "qgspallabeling.h"
#include "qgsmapsettings.h"
#include "qgslogger.h"


QgsDxfLabelProvider::QgsDxfLabelProvider( QgsVectorLayer* layer, const QString& providerId, QgsDxfExport* dxf, const QgsPalLayerSettings *settings )
    : QgsVectorLayerLabelProvider( layer, providerId, false, settings )
    , mDxfExport( dxf )
{
}

void QgsDxfLabelProvider::drawLabel( QgsRenderContext& context, pal::LabelPosition* label ) const
{
  Q_ASSERT( mDxfExport );
  mDxfExport->drawLabel( layerId(), context, label, mSettings );
}

void QgsDxfLabelProvider::registerDxfFeature( QgsFeature& feature, QgsRenderContext& context, const QString& dxfLayerName )
{
  registerFeature( feature, context );
  mDxfExport->registerDxfLayer( layerId(), feature.id(), dxfLayerName );
}

QgsDxfRuleBasedLabelProvider::QgsDxfRuleBasedLabelProvider( const QgsRuleBasedLabeling &rules, QgsVectorLayer* layer, QgsDxfExport* dxf )
    : QgsRuleBasedLabelProvider( rules, layer, false )
    , mDxfExport( dxf )
{
}

void QgsDxfRuleBasedLabelProvider::reinit( QgsVectorLayer* layer )
{
  QgsDebugMsg( "Entering." );
  mRules.rootRule()->createSubProviders( layer, mSubProviders, this );
}

QgsVectorLayerLabelProvider *QgsDxfRuleBasedLabelProvider::createProvider( QgsVectorLayer *layer, const QString& providerId, bool withFeatureLoop, const QgsPalLayerSettings *settings )
{
  QgsDebugMsg( "Entering." );
  Q_UNUSED( withFeatureLoop );
  return new QgsDxfLabelProvider( layer, providerId, mDxfExport, settings );
}

void QgsDxfRuleBasedLabelProvider::drawLabel( QgsRenderContext &context, pal::LabelPosition *label ) const
{
  QgsDebugMsg( "Entering." );
  Q_ASSERT( mDxfExport );
  mDxfExport->drawLabel( layerId(), context, label, mSettings );
}

void QgsDxfRuleBasedLabelProvider::registerDxfFeature( QgsFeature &feature, QgsRenderContext &context, const QString &dxfLayerName )
{
  registerFeature( feature, context );
  mDxfExport->registerDxfLayer( layerId(), feature.id(), dxfLayerName );
}
