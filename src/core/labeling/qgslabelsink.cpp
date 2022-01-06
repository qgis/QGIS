/***************************************************************************
                         qgslabelsink.cpp
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

#include "qgslabelsink.h"
#include "qgspallabeling.h"
#include "qgsmapsettings.h"
#include "qgslogger.h"

QgsLabelSinkProvider::QgsLabelSinkProvider( QgsVectorLayer *layer, const QString &providerId, QgsLabelSink *sink, const QgsPalLayerSettings *settings )
  : QgsVectorLayerLabelProvider( layer, providerId, false, settings )
  , mLabelSink( sink )
{
}

void QgsLabelSinkProvider::drawLabel( QgsRenderContext &context, pal::LabelPosition *label ) const
{
  Q_ASSERT( mLabelSink );
  mLabelSink->drawLabel( layerId(), context, label, mSettings );
}

void QgsLabelSinkProvider::drawUnplacedLabel( QgsRenderContext &context, pal::LabelPosition *label ) const
{
  Q_ASSERT( mLabelSink );
  mLabelSink->drawUnplacedLabel( layerId(), context, label, mSettings );
}

QgsRuleBasedLabelSinkProvider::QgsRuleBasedLabelSinkProvider( const QgsRuleBasedLabeling &rules, QgsVectorLayer *layer, QgsLabelSink *sink )
  : QgsRuleBasedLabelProvider( rules, layer, false )
  , mLabelSink( sink )
{
  mRules->rootRule()->createSubProviders( layer, mSubProviders, this );
}

void QgsRuleBasedLabelSinkProvider::reinit( QgsVectorLayer *layer )
{
  mRules->rootRule()->createSubProviders( layer, mSubProviders, this );
}

QgsVectorLayerLabelProvider *QgsRuleBasedLabelSinkProvider::createProvider( QgsVectorLayer *layer, const QString &providerId, bool withFeatureLoop, const QgsPalLayerSettings *settings )
{
  Q_UNUSED( withFeatureLoop )
  return new QgsLabelSinkProvider( layer, providerId, mLabelSink, settings );
}

void QgsRuleBasedLabelSinkProvider::drawLabel( QgsRenderContext &context, pal::LabelPosition *label ) const
{
  Q_ASSERT( mLabelSink );
  mLabelSink->drawLabel( layerId(), context, label, mSettings );
}

void QgsRuleBasedLabelSinkProvider::drawUnplacedLabel( QgsRenderContext &context, pal::LabelPosition *label ) const
{
  Q_ASSERT( mLabelSink );
  mLabelSink->drawUnplacedLabel( layerId(), context, label, mSettings );
}
